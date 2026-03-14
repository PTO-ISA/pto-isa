# -*- coding: utf-8 -*-
# --------------------------------------------------------------------------------
# Copyright (c) 2025 Huawei Technologies Co., Ltd.
# This program is free software, you can redistribute it and/or modify it under the terms and conditions of
# CANN Open Software License Agreement Version 2.0 (the "License").
# Please refer to the License for details. You may not use this file except in compliance with the License.
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
# See LICENSE in the root of the software repository for the full text of the License.
# --------------------------------------------------------------------------------

"""
MkDocs build-time generator for PTO Tile Lib.

We intentionally keep MkDocs config under `docs/mkdocs/` and generate a *mirror*
of repository markdown into `docs/mkdocs/src/` so the site can browse markdown
across the entire repo (README files under kernels/, tests/, scripts/, etc.).

Key property:
- Generated pages preserve original repository paths, so existing repo-relative
  links like `docs/...` or `kernels/...` keep working in the site.
"""

from __future__ import annotations

import json
import posixpath
import re
from pathlib import Path

import mkdocs_gen_files


repoRoot = Path(__file__).resolve().parents[2]

skipPrefixes = (
    ".git/",
    ".github/",
    ".gitcode/",
    ".venv/",
    ".venv-mkdocs/",
    "site/",
    "site_zh/",
    "build/",
    "build_tests/",
    ".idea/",
    ".vscode/",
)

skipContains = (
    "/__pycache__/",
    "/CMakeFiles/",
)

assetExts = {
    ".svg",
    ".png",
    ".jpg",
    ".jpeg",
    ".gif",
    ".webp",
    ".bnf",
}

# Directory names whose README is the canonical index page.
# Used by enUrlToZhUrl to map e.g. /docs/isa/ -> /docs/isa/README_zh/.
readmeDirs = {
    "coding", "isa", "machine", "assembly", "docs", "kernels",
    "tests", "demos", "scripts", "include", "cmake", "reference",
    "tutorials", "script", "package", "custom", "baseline", "add",
    "gemm_basic", "flash_atten", "gemm_performance", "a2a3", "a5",
    "kirin9030", "npu", "pto",
}


def shouldSkip(relPosix: str) -> bool:
    if relPosix.startswith("docs/mkdocs/"):
        return True
    if relPosix.endswith("/mkdocs.yml"):
        return True
    if relPosix == "docs/menu_ops_development.md":
        return True
    if relPosix.startswith(".venv"):
        return True
    if "site-packages/" in relPosix:
        return True
    if any(relPosix.startswith(p) for p in skipPrefixes):
        return True
    if any(s in relPosix for s in skipContains):
        return True
    if relPosix.endswith((".pyc",)):
        return True
    return False


absLinkRe = re.compile(r'\]\(/((?!http)[^)]+)\)')
relImgRe = re.compile(r'(<img\b[^>]*\bsrc=["\'])((?!http|/|data:)[^"\'>]+)(["\'])')


def rewriteRelImgsForBuild(text: str, srcRel: str) -> str:
    """Rewrite relative <img src="..."> paths so they resolve correctly from
    the MkDocs virtual page URL.

    MkDocs serves foo/bar.md at /foo/bar/, so a relative image path that
    works when browsing the repo (relative to foo/) needs to be adjusted
    to be relative to /foo/bar/ instead.

    Example:
      srcRel = "docs/getting-started.md"
      imgPath = "figures/pto_logo.svg"  (relative to docs/)
      resolved repo path = docs/figures/pto_logo.svg
      MkDocs page URL   = /docs/getting-started/
      correct rel path  = ../figures/pto_logo.svg
    """
    # Directory containing the source file (repo-relative, posix).
    srcDir = Path(srcRel).parent.as_posix()  # e.g. "docs"

    # Virtual page directory (where MkDocs serves the page).
    # For foo/bar.md -> /foo/bar/  so pageDir = "foo/bar"
    pageDir = Path(srcRel).with_suffix('').as_posix()  # e.g. "docs/getting-started"

    def replace(m: re.Match) -> str:
        prefix, imgPath, suffix = m.group(1), m.group(2), m.group(3)
        # Resolve image to repo-relative path.
        repoImg = (srcDir + '/' + imgPath) if (srcDir and srcDir != '.') else imgPath
        # Normalize (handle any ../ in original imgPath).
        repoImg = posixpath.normpath(repoImg)
        # Compute relative path from pageDir to repoImg.
        rel = posixpath.relpath(repoImg, pageDir)  # e.g. ../figures/pto_logo.svg
        return f'{prefix}{rel}{suffix}'

    return relImgRe.sub(replace, text)


# Matches a relative markdown link that starts with one or more "../" components.
# Group 1: the "../" prefix (one or more), Group 2: the rest of the target.
relLinkRe = re.compile(r'\]\((\.\./+)([^)]+)\)')

# Matches repo-relative links into docs/mkdocs/src/ (e.g. mkdocs/src/manual/foo.md).
# These appear in repo-level docs so they resolve correctly during static browsing,
# but must be rewritten to root-absolute paths at MkDocs build time.
mkdocsSrcLinkRe = re.compile(r'\]\(mkdocs/src/([^)]+)\)')

# Strip stale "<!-- Generated from ... -->" header lines that accumulate on
# repeated builds when docs_dir is the same directory as the source files.
_generatedHeaderRe = re.compile(
    r'^(?:<!-- Generated from `[^`]*` -->\s*\n\n?)+', re.MULTILINE
)


def stripGeneratedHeader(text: str) -> str:
    """Remove any leading '<!-- Generated from ... -->' comment blocks."""
    return _generatedHeaderRe.sub('', text)


def rewriteLinksForBuild(text: str, virtualPath: str) -> str:
    """Rewrite links in a hand-written docs/mkdocs/src/ file so they resolve
    correctly from the MkDocs virtual page URL.

    Two kinds of links are rewritten:

    1. Root-absolute links like /docs/isa/TADD.md  ->  ../docs/isa/TADD.md
       These are written with a leading '/' so they work when browsing the
       repo on GitHub/Gitee; at build time they need to be relative.

    2. Relative links whose "../" depth is wrong for the virtual path.
       Example: hand-written file lives at
         docs/mkdocs/src/manual/appendix-d.md  (repo path)
       so the author wrote  ../../docs/isa/TADD.md  (2 levels up from
       docs/mkdocs/src/manual/ to reach the repo root docs/).
       But the virtual path is  manual/appendix-d.md  (depth=1), so
       MkDocs needs  ../docs/isa/TADD.md  (only 1 level up).

       The hand-written source sits at depth
         srcDepth = len(Path("docs/mkdocs/src") / virtualPath).parent.parts
                  = len(("docs","mkdocs","src","manual")) = 4
       and the virtual path sits at depth
         virtDepth = len(Path(virtualPath).parent.parts)
                   = len(("manual",)) = 1
       so each "../" in the original link corresponds to climbing one level
       in the repo tree.  After stripping the src prefix the correct number
       of "../" is virtDepth.

    Args:
        text:        Markdown source text.
        virtualPath: Virtual path of the file (relative to docs_dir), e.g.
                     "manual/appendix-d-instruction-family-matrix.md".
    """
    # Depth of virtual page's parent directory.
    virtDepth = len(Path(virtualPath).parent.parts)  # e.g. 1 for "manual/foo.md"

    # Depth of the source file inside docs/mkdocs/src/.
    srcDepth = len((Path("docs") / "mkdocs" / "src" / virtualPath).parent.parts)

    # --- Pass 1: root-absolute links /foo/bar  ->  (../)*virtDepth foo/bar ---
    prefixAbs = '../' * virtDepth if virtDepth else ''

    def replaceAbs(m: re.Match) -> str:
        return f']({prefixAbs}{m.group(1)})'

    text = absLinkRe.sub(replaceAbs, text)

    # --- Pass 2: relative links with wrong ../ depth ---
    # The author wrote the link relative to the *repo* source file location
    # (srcDepth levels deep).  We need it relative to the virtual page
    # (virtDepth levels deep).  We only touch links whose leading "../"
    # count equals srcDepth (exactly what the author would write to reach
    # the repo root from the source file).
    if srcDepth != virtDepth:
        newUps = '../' * virtDepth if virtDepth else ''

        def replaceRel(m: re.Match) -> str:
            ups, rest = m.group(1), m.group(2)
            if ups.count('../') == srcDepth:
                return f']({newUps}{rest})'
            return m.group(0)  # leave unchanged

        text = relLinkRe.sub(replaceRel, text)

    return text


# ---------------------------------------------------------------------------
# Nav order from mkdocs.yml (used for prev/next generation)
# ---------------------------------------------------------------------------

navPagesEn = [
    "index.md",
    "docs/getting-started.md",
    "docs/PTO-Virtual-ISA-Manual.md",
    "manual/index.md",
    "manual/01-overview.md",
    "manual/02-machine-model.md",
    "manual/03-state-and-types.md",
    "manual/04-tiles-and-globaltensor.md",
    "manual/05-synchronization.md",
    "manual/06-assembly.md",
    "manual/07-instructions.md",
    "manual/08-programming.md",
    "manual/09-virtual-isa-and-ir.md",
    "manual/10-bytecode-and-toolchain.md",
    "manual/11-memory-ordering-and-consistency.md",
    "manual/12-backend-profiles-and-conformance.md",
    "manual/appendix-a-glossary.md",
    "manual/appendix-b-instruction-contract-template.md",
    "manual/appendix-c-diagnostics-taxonomy.md",
    "manual/appendix-d-instruction-family-matrix.md",
    "docs/coding/README.md",
    "docs/coding/ProgrammingModel.md",
    "docs/coding/Tile.md",
    "docs/coding/GlobalTensor.md",
    "docs/coding/Scalar.md",
    "docs/coding/Event.md",
    "docs/coding/tutorial.md",
    "docs/coding/tutorials/README.md",
    "docs/coding/tutorials/vec-add.md",
    "docs/coding/tutorials/row-softmax.md",
    "docs/coding/tutorials/gemm.md",
    "docs/coding/opt.md",
    "docs/coding/debug.md",
    "docs/machine/abstract-machine.md",
    "docs/machine/README.md",
    "docs/isa/README.md",
    "docs/isa/conventions.md",
    "docs/assembly/README.md",
    "docs/assembly/PTO-AS.md",
    "docs/assembly/conventions.md",
    "docs/assembly/nonisa-ops.md",
    "docs/assembly/elementwise-ops.md",
    "docs/assembly/tile-scalar-ops.md",
    "docs/assembly/axis-ops.md",
    "docs/assembly/memory-ops.md",
    "docs/assembly/matrix-ops.md",
    "docs/assembly/data-movement-ops.md",
    "docs/assembly/complex-ops.md",
    "docs/assembly/manual-binding-ops.md",
    "docs/assembly/scalar-arith-ops.md",
    "docs/assembly/control-flow-ops.md",
    "docs/PTOISA.md",
    "docs/isa/TSYNC.md",
    "docs/isa/TASSIGN.md",
    "docs/isa/TSETFMATRIX.md",
    "docs/isa/TSET_IMG2COL_RPT.md",
    "docs/isa/TSET_IMG2COL_PADDING.md",
    "docs/isa/TADD.md", "docs/isa/TABS.md", "docs/isa/TAND.md",
    "docs/isa/TOR.md", "docs/isa/TSUB.md", "docs/isa/TMUL.md",
    "docs/isa/TMIN.md", "docs/isa/TMAX.md", "docs/isa/TCMP.md",
    "docs/isa/TDIV.md", "docs/isa/TSHL.md", "docs/isa/TSHR.md",
    "docs/isa/TXOR.md", "docs/isa/TLOG.md", "docs/isa/TRECIP.md",
    "docs/isa/TPRELU.md", "docs/isa/TADDC.md", "docs/isa/TSUBC.md",
    "docs/isa/TCVT.md", "docs/isa/TSEL.md", "docs/isa/TRSQRT.md",
    "docs/isa/TSQRT.md", "docs/isa/TEXP.md", "docs/isa/TNOT.md",
    "docs/isa/TRELU.md", "docs/isa/TNEG.md", "docs/isa/TREM.md",
    "docs/isa/TFMOD.md",
    "docs/isa/TEXPANDS.md", "docs/isa/TCMPS.md", "docs/isa/TSELS.md",
    "docs/isa/TMINS.md", "docs/isa/TADDS.md", "docs/isa/TSUBS.md",
    "docs/isa/TDIVS.md", "docs/isa/TMULS.md", "docs/isa/TFMODS.md",
    "docs/isa/TREMS.md", "docs/isa/TMAXS.md", "docs/isa/TANDS.md",
    "docs/isa/TORS.md", "docs/isa/TSHLS.md", "docs/isa/TSHRS.md",
    "docs/isa/TXORS.md", "docs/isa/TLRELU.md", "docs/isa/TADDSC.md",
    "docs/isa/TSUBSC.md",
    "docs/isa/TROWSUM.md", "docs/isa/TROWPROD.md", "docs/isa/TCOLSUM.md",
    "docs/isa/TCOLPROD.md", "docs/isa/TCOLMAX.md", "docs/isa/TROWMAX.md",
    "docs/isa/TROWMIN.md", "docs/isa/TCOLMIN.md", "docs/isa/TROWEXPAND.md",
    "docs/isa/TROWEXPANDDIV.md", "docs/isa/TROWEXPANDMUL.md",
    "docs/isa/TROWEXPANDSUB.md", "docs/isa/TROWEXPANDADD.md",
    "docs/isa/TROWEXPANDMAX.md", "docs/isa/TROWEXPANDMIN.md",
    "docs/isa/TROWEXPANDEXPDIF.md", "docs/isa/TCOLEXPAND.md",
    "docs/isa/TCOLEXPANDDIV.md", "docs/isa/TCOLEXPANDMUL.md",
    "docs/isa/TCOLEXPANDADD.md", "docs/isa/TCOLEXPANDMAX.md",
    "docs/isa/TCOLEXPANDMIN.md", "docs/isa/TCOLEXPANDSUB.md",
    "docs/isa/TCOLEXPANDEXPDIF.md",
    "docs/isa/TLOAD.md", "docs/isa/TPREFETCH.md", "docs/isa/TSTORE.md",
    "docs/isa/TSTORE_FP.md", "docs/isa/MGATHER.md", "docs/isa/MSCATTER.md",
    "docs/isa/TMATMUL.md", "docs/isa/TMATMUL_ACC.md", "docs/isa/TMATMUL_BIAS.md",
    "docs/isa/TMATMUL_MX.md", "docs/isa/TGEMV.md", "docs/isa/TGEMV_ACC.md",
    "docs/isa/TGEMV_BIAS.md", "docs/isa/TGEMV_MX.md",
    "docs/isa/TMOV.md", "docs/isa/TMOV_FP.md", "docs/isa/TEXTRACT.md",
    "docs/isa/TEXTRACT_FP.md", "docs/isa/TINSERT.md", "docs/isa/TINSERT_FP.md",
    "docs/isa/TFILLPAD.md", "docs/isa/TFILLPAD_INPLACE.md",
    "docs/isa/TFILLPAD_EXPAND.md", "docs/isa/TRESHAPE.md",
    "docs/isa/TTRANS.md", "docs/isa/TIMG2COL.md",
    "docs/isa/TGATHER.md", "docs/isa/TGATHERB.md", "docs/isa/TSCATTER.md",
    "docs/isa/TCI.md", "docs/isa/TTRI.md", "docs/isa/TPARTADD.md",
    "docs/isa/TPARTMUL.md", "docs/isa/TPARTMAX.md", "docs/isa/TPARTMIN.md",
    "docs/isa/TSORT32.md", "docs/isa/TMRGSORT.md", "docs/isa/TQUANT.md",
    "docs/isa/TPRINT.md",
    "docs/reference/pto-intrinsics-header.md",
    "manual/isa-reference.md",
    # Examples & Kernels
    "kernels/README.md",
    "kernels/manual/a2a3/gemm_performance/README.md",
    "kernels/manual/common/flash_atten/README.md",
    "demos/baseline/add/README.md",
    "demos/baseline/gemm_basic/README.md",
    "tests/README.md",
    "tests/script/README.md",
    # Documentation
    "docs/README.md",
    "docs/website.md",
    # Full index
    "all-pages.md",
]


def mdToUrl(mdPath: str) -> str:
    """Convert a virtual .md path to the MkDocs site URL path.

    MkDocs converts:
      - ``foo/index.md``  -> ``/foo/``
      - ``foo/README.md`` -> ``/foo/``   (README treated as directory index)
      - ``index.md``      -> ``/``
      - ``README.md``     -> ``/``
      - ``foo/bar.md``    -> ``/foo/bar/``
    """
    p = Path(mdPath)
    if p.name in ("index.md", "README.md"):
        parent = p.parent.as_posix().lstrip("./")
        url = "/" + parent + "/" if parent else "/"
    else:
        url = "/" + p.with_suffix("").as_posix().lstrip("./") + "/"
    # normalise double-slash at root
    if url == "//":
        url = "/"
    return url


def enUrlToZhUrl(enUrl: str) -> str | None:
    """Best-effort mapping: English URL -> Chinese URL.

    Returns None if we cannot determine the zh counterpart.
    """
    # root index: / -> /index_zh/
    if enUrl == "/":
        return "/index_zh/"
    # strip trailing slash for manipulation
    base = enUrl.rstrip("/")
    # manual index: /manual -> /manual/index_zh
    if base == "/manual":
        return "/manual/index_zh/"
    # README pages: last segment is a known directory name
    last = base.rsplit("/", 1)[-1]
    if last in readmeDirs:
        return enUrl.rstrip("/") + "/README_zh/"
    # general page: append _zh
    return base + "_zh/"


def generateLangMap(navPages: list[str]) -> dict:
    """Build a mapping dict for use by the language switcher JS.

    Structure::

        {
          "en_to_zh": { "/manual/01-overview/": "/manual/01-overview_zh/", ... },
          "zh_to_en": { "/manual/01-overview_zh/": "/manual/01-overview/", ... },
          "nav": [
            { "en": "/manual/01-overview/", "zh": "/manual/01-overview_zh/",
              "prev_en": "/manual/", "prev_zh": "/manual/index_zh/",
              "next_en": "/manual/02-machine-model/",
              "next_zh": "/manual/02-machine-model_zh/" },
            ...
          ]
        }
    """
    enUrls = [mdToUrl(p) for p in navPages]
    enToZh: dict[str, str] = {}
    zhToEn: dict[str, str] = {}

    for en in enUrls:
        zh = enUrlToZhUrl(en)
        if zh:
            enToZh[en] = zh
            zhToEn[zh] = en

    navEntries = []
    for i, en in enumerate(enUrls):
        zh = enToZh.get(en)
        prevEn = enUrls[i - 1] if i > 0 else None
        nextEn = enUrls[i + 1] if i < len(enUrls) - 1 else None
        entry = {
            "en": en,
            "zh": zh,
            "prev_en": prevEn,
            "prev_zh": enToZh.get(prevEn) if prevEn else None,
            "next_en": nextEn,
            "next_zh": enToZh.get(nextEn) if nextEn else None,
        }
        navEntries.append(entry)

    return {"en_to_zh": enToZh, "zh_to_en": zhToEn, "nav": navEntries}


# ---------------------------------------------------------------------------
# Helpers used by main()
# ---------------------------------------------------------------------------

def extractFirstHeading(mdPath: Path) -> str:
    """Return the text of the first Markdown heading in *mdPath*, or the stem."""
    try:
        text = mdPath.read_text(encoding="utf-8-sig", errors="replace")
    except OSError:
        return mdPath.stem
    for line in text.splitlines():
        if line.startswith("#"):
            return line.lstrip("#").strip()
    return mdPath.stem


def writeIsaReferenceIndex(
    outPath: str,
    isaPages: list[tuple[str, str]],
    heading: str,
    preamble: str,
    sectionHeading: str,
    emptyMsg: str,
) -> None:
    """Write a generated ISA reference index page to *outPath*."""
    with mkdocs_gen_files.open(outPath, "w") as f:
        f.write(f"{heading}\n\n")
        f.write(preamble)
        if not isaPages:
            f.write(emptyMsg)
        else:
            f.write(f"{sectionHeading}\n\n")
            for instr, title in isaPages:
                link = f"../docs/isa/{instr}.md"
                suffix = "" if title.strip() == instr else f" — {title}"
                f.write(f"- [{instr}]({link}){suffix}\n")
            f.write("\n")


def writeAllPagesIndex(
    outPath: str,
    sections: dict[str, list[str]],
    heading: str,
    preamble: str,
    emptyMsg: str,
) -> None:
    """Write a generated all-pages index to *outPath*."""
    with mkdocs_gen_files.open(outPath, "w") as f:
        f.write(f"{heading}\n\n")
        f.write(preamble)
        if not sections:
            f.write(emptyMsg)
        else:
            for top in sorted(sections.keys()):
                f.write(f"## {top}\n\n")
                for rel in sections[top]:
                    label = rel if top == "(root)" else rel[len(top) + 1:]
                    f.write(f"- [{label}]({rel})\n")
                f.write("\n")


# ---------------------------------------------------------------------------
# main
# ---------------------------------------------------------------------------

def main() -> None:
    copiedMd: list[str] = []

    mkdocsSrc = repoRoot / "docs" / "mkdocs" / "src"

    # Step 1: Process hand-written files under docs/mkdocs/src/.
    # These files use root-absolute links (e.g. /docs/isa/TADD.md) so they
    # work when browsing the repo statically (GitHub/Gitee). At build time
    # we rewrite them to relative paths for MkDocs, and place them at their
    # virtual path (stripping the docs/mkdocs/src/ prefix).
    #
    # IMPORTANT: We must NOT recurse into docs/mkdocs/src/docs/mkdocs/src/
    # (stale nested copies from previous builds). We skip any path that,
    # relative to mkdocsSrc, starts with "docs/mkdocs/" to avoid that.
    for src in mkdocsSrc.rglob("*.md"):
        virtualPath = src.relative_to(mkdocsSrc).as_posix()  # e.g. manual/01-overview.md
        # Skip stale nested docs/mkdocs/src/ directories that may exist on disk
        # from previous builds (mkdocs_gen_files writes to a temp dir, but the
        # docs_dir itself may have leftover files if docs_dir == src/).
        if virtualPath.startswith("docs/mkdocs/"):
            continue
        text = src.read_text(encoding="utf-8-sig", errors="replace")
        # Strip any stale "Generated from" header left by a previous build.
        text = stripGeneratedHeader(text)
        text = rewriteRelImgsForBuild(text, virtualPath)
        text = rewriteLinksForBuild(text, virtualPath)
        with mkdocs_gen_files.open(virtualPath, "w") as f:
            # Do not add a generated header for hand-written files under
            # docs/mkdocs/src/manual/ or the root index pages — those are
            # source files, not build artefacts, and the comment would
            # pollute the originals.
            if not virtualPath.startswith("manual/") and virtualPath not in ("index.md", "index_zh.md"):
                f.write(f"<!-- Generated from `docs/mkdocs/src/{virtualPath}` -->\n\n")
            f.write(text)
        copiedMd.append(virtualPath)

    # Step 2: Mirror all other repo markdown files preserving their paths.
    for src in repoRoot.rglob("*.md"):
        rel = src.relative_to(repoRoot).as_posix()
        if shouldSkip(rel):
            continue
        # Use utf-8-sig to automatically remove BOM if present
        text = src.read_text(encoding="utf-8-sig", errors="replace")
        # Rewrite relative <img src="..."> paths for all mirrored files.
        text = rewriteRelImgsForBuild(text, rel)
        # Rewrite mkdocs/src/... links to root-absolute /... links, then
        # let rewriteLinksForBuild convert them to correct relative paths.
        if mkdocsSrcLinkRe.search(text):
            text = mkdocsSrcLinkRe.sub(r'](/\1)', text)
            text = rewriteLinksForBuild(text, rel)
        with mkdocs_gen_files.open(rel, "w") as f:
            f.write(f"<!-- Generated from `{rel}` -->\n\n")
            f.write(text)
        copiedMd.append(rel)

    # Generate per-instruction reference indexes for docs/isa/*.md.
    isaDir = repoRoot / "docs" / "isa"
    isaPageEn: list[tuple[str, str]] = []
    isaPageZh: list[tuple[str, str]] = []

    if isaDir.exists():
        for p in sorted(isaDir.glob("*.md")):
            if p.name in ("README.md", "README_zh.md", "conventions.md", "conventions_zh.md"):
                continue
            stem = p.stem
            title = extractFirstHeading(p)
            if stem.endswith("_zh"):
                isaPageZh.append((stem, title))
            else:
                isaPageEn.append((stem, title))

    writeIsaReferenceIndex(
        outPath="manual/isa-reference.md",
        isaPages=isaPageEn,
        heading="# Instruction Reference Pages",
        preamble=(
            "This page is generated at build time.\n\n"
            "- Instruction index: `docs/isa/README.md`\n"
            "- ISA conventions: `docs/isa/conventions.md`\n\n"
        ),
        sectionHeading="## All instructions",
        emptyMsg="No English instruction pages were found under `docs/isa/`.\n",
    )

    writeIsaReferenceIndex(
        outPath="manual/isa-reference_zh.md",
        isaPages=isaPageZh,
        heading="# 指令参考页面（全量）",
        preamble=(
            "本页在构建站点时自动生成。\n\n"
            "- 指令索引：`docs/isa/README_zh.md`\n"
            "- ISA 通用约定：`docs/isa/conventions_zh.md`\n\n"
        ),
        sectionHeading="## 全部指令",
        emptyMsg="未在 `docs/isa/` 下发现中文指令页面。\n",
    )

    # Generate a simple index page that links to all mirrored markdown.
    allMd = sorted(set(copiedMd))
    sections: dict[str, list[str]] = {}
    sectionsZh: dict[str, list[str]] = {}

    for rel in allMd:
        top = rel.split("/", 1)[0] if "/" in rel else "(root)"
        sections.setdefault(top, []).append(rel)
        if "_zh.md" in rel or rel.endswith("_zh/index.md"):
            sectionsZh.setdefault(top, []).append(rel)

    writeAllPagesIndex(
        outPath="all-pages.md",
        sections=sections,
        heading="# All Markdown Pages",
        preamble="This page is generated at build time and lists markdown files mirrored into the site.\n\n",
        emptyMsg="",
    )

    writeAllPagesIndex(
        outPath="all-pages_zh.md",
        sections=sectionsZh,
        heading="# 所有 Markdown 页面",
        preamble="本页面在构建时自动生成，列出了站点中镜像的所有中文 markdown 文件。\n\n",
        emptyMsg="未找到中文页面。\n",
    )

    # Generate lang-map.json for zero-latency language switching.
    langMap = generateLangMap(navPagesEn)
    with mkdocs_gen_files.open("lang-map.json", "w") as f:
        json.dump(langMap, f, ensure_ascii=False, separators=(",", ":"))

    # Mirror commonly referenced doc assets (images) so docs render cleanly.
    for src in repoRoot.rglob("*"):
        if not src.is_file():
            continue
        if src.suffix.lower() not in assetExts:
            continue
        rel = src.relative_to(repoRoot).as_posix()
        if shouldSkip(rel):
            continue
        with mkdocs_gen_files.open(rel, "wb") as f:
            f.write(src.read_bytes())


main()
 