#!/usr/bin/env python3
# --------------------------------------------------------------------------------
# Copyright (c) 2026 Huawei Technologies Co., Ltd.
# This program is free software, you can redistribute it and/or modify it under the terms and conditions of
# CANN Open Software License Agreement Version 2.0 (the "License").
# Please refer to the License for details. You may not use this file except in compliance with the License.
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
# See LICENSE in the root of the software repository for the full text of the License.
# --------------------------------------------------------------------------------

from __future__ import annotations

import argparse
import csv
import json
import os
import re
import shutil
import subprocess
import time
from collections import defaultdict
from dataclasses import dataclass
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[2]
ISA_MANIFEST = REPO_ROOT / "docs" / "isa" / "manifest.yaml"
PUBLIC_HEADERS = [
    REPO_ROOT / "include" / "pto" / "common" / "pto_instr.hpp",
    REPO_ROOT / "include" / "pto" / "comm" / "pto_comm_inst.hpp",
]
CPU_TESTCASE_ROOT = REPO_ROOT / "tests" / "cpu" / "st" / "testcase"
NPU_SRC_TESTCASE_ROOT = REPO_ROOT / "tests" / "npu" / "a2a3" / "src" / "st" / "testcase"
NPU_COMM_TESTCASE_ROOT = REPO_ROOT / "tests" / "npu" / "a2a3" / "comm" / "st" / "testcase"
REPORT_ROOT = REPO_ROOT / "tests" / "reports"
RUNTIME_CACHE_PATH = REPORT_ROOT / "pto_isa_runtime_cache_latest.json"
CPU_BUILD_ROOT = Path("/tmp") / "pto_isa_checklist_cpu"
CPU_TIMEOUT_SEC = 600
NPU_TIMEOUT_SEC = 1800

INTRINSIC_PATTERN = re.compile(r"\bPTO_INST\b[^(\n;]*\b(T[A-Z][A-Za-z0-9_]*)\s*\(")
TOKEN_PATTERN = re.compile(r"\b(T[A-Z][A-Z0-9_]*)\b")

IMPL_ALIASES = {
    "TEXTRACT_FP": "TEXTRACT",
    "TINSERT_FP": "TINSERT",
    "TSTORE_FP": "TSTORE",
    "TMOV_FP": "TMOV",
}


def read_text(path: Path) -> str:
    return path.read_text(encoding="utf-8", errors="ignore")


def camel_to_snake_tail(instruction: str) -> str:
    tail = instruction[1:].lower()
    return tail


def instruction_sort_key(name: str) -> tuple[int, str]:
    if name.startswith("T"):
        return (0, name)
    return (1, name)


def load_manifest() -> dict[str, dict[str, object]]:
    payload = json.loads(read_text(ISA_MANIFEST))
    return {entry["instruction"]: entry for entry in payload.get("instructions", [])}


def load_public_intrinsics() -> list[str]:
    names: set[str] = set()
    for header in PUBLIC_HEADERS:
        if header.exists():
            names.update(INTRINSIC_PATTERN.findall(read_text(header)))
    return sorted(names)


def list_testcase_dirs(root: Path) -> list[Path]:
    if not root.exists():
        return []
    return sorted([path for path in root.iterdir() if path.is_dir()])


def testcase_instruction_tokens(testcase_dir: Path, instructions: set[str]) -> set[str]:
    tokens: set[str] = set()
    for path in testcase_dir.rglob("*"):
        if path.is_file() and path.suffix in {".cpp", ".cc", ".cxx", ".hpp", ".h"}:
            tokens.update(token for token in TOKEN_PATTERN.findall(read_text(path)) if token in instructions)
    return tokens


def build_testcase_index(testcase_dirs: list[Path], instructions: set[str]) -> dict[str, list[str]]:
    index: dict[str, list[str]] = defaultdict(list)
    for testcase_dir in testcase_dirs:
        for token in testcase_instruction_tokens(testcase_dir, instructions):
            index[token].append(testcase_dir.name)
    for token in index:
        index[token] = sorted(set(index[token]))
    return dict(index)


def has_impl_token(root: Path, instruction: str) -> bool:
    needle = f"{instruction}_IMPL"
    for suffix in ("*.hpp", "*.h"):
        for path in root.rglob(suffix):
            text = read_text(path)
            if needle in text:
                return True
    return False


def has_a3_impl(instruction: str) -> bool:
    impl_name = IMPL_ALIASES.get(instruction, instruction)
    if has_impl_token(REPO_ROOT / "include" / "pto" / "npu" / "a2a3", impl_name):
        return True
    if instruction in {"TBROADCAST", "TGET", "TGET_ASYNC", "TNOTIFY", "TPUT", "TPUT_ASYNC", "TREDUCE", "TSCATTER", "TTEST", "TWAIT"}:
        return True
    return False


def has_cpu_impl(instruction: str) -> bool:
    impl_name = IMPL_ALIASES.get(instruction, instruction)
    return has_impl_token(REPO_ROOT / "include" / "pto" / "cpu", impl_name)


def testcase_score(instruction: str, testcase: str, backend: str) -> int:
    lower = instruction.lower()
    compact = lower.replace("_", "")
    score = 0
    if testcase == lower or testcase == compact:
        score = 100
    elif testcase == "trowexpandop" and instruction.startswith("TROWEXPAND") and instruction != "TROWEXPAND":
        score = 95
    elif testcase == "tcolexpandop" and instruction.startswith("TCOLEXPAND") and instruction != "TCOLEXPAND":
        score = 95
    elif testcase == "tcpu_missing_ops":
        score = 80
    elif testcase == "tpushpop" and instruction in {"TPUSH", "TPOP"}:
        score = 90
    elif testcase == "tpushpop_cv" and instruction in {"TPUSH", "TPOP"}:
        score = 85
    elif testcase == "timg2col" and instruction in {"TSETFMATRIX", "TSET_IMG2COL_RPT", "TSET_IMG2COL_PADDING"}:
        score = 90
    elif testcase == "tpushpop" and instruction == "TSYNC":
        score = 88
    elif testcase == "tpushpop_cv" and instruction == "TSYNC":
        score = 86
    elif testcase == "tloadconv" and instruction in {"TSETFMATRIX", "TSET_IMG2COL_RPT", "TSET_IMG2COL_PADDING"}:
        score = 75
    elif compact in testcase or lower in testcase:
        score = 70
    if backend == "npu" and testcase.startswith("comm/"):
        score += 2
    return score


def pick_representative(instruction: str, candidates: list[str], backend: str) -> tuple[str | None, str]:
    if not candidates:
        return None, "none"
    ranked = sorted(candidates, key=lambda name: (-testcase_score(instruction, name, backend), name))
    selected = ranked[0]
    if selected in {instruction.lower(), instruction.lower().replace("_", "")}:
        kind = "direct"
    elif selected in {"tcpu_missing_ops", "trowexpandop", "tcolexpandop"}:
        kind = "aggregate"
    elif selected in {"timg2col", "tloadconv", "tpushpop", "tpushpop_cv"}:
        kind = "shared"
    else:
        kind = "indirect"
    return selected, kind


MANUAL_CPU_TESTS = {
    "TSYNC": ["tpushpop", "tpushpop_cv"],
    "TSETFMATRIX": ["timg2col_config"],
    "TSET_IMG2COL_RPT": ["timg2col_config"],
    "TSET_IMG2COL_PADDING": ["timg2col_config"],
}

MANUAL_NPU_TESTS = {
    "TSYNC": ["tpushpop_cv", "tpushpop_vc"],
    "TSETFMATRIX": ["timg2col"],
    "TSET_IMG2COL_RPT": ["timg2col"],
    "TSET_IMG2COL_PADDING": ["timg2col"],
}


@dataclass
class InstructionRow:
    instruction: str
    source: str
    category: str
    summary_zh: str
    doc_present: bool
    public_intrinsic: bool
    cpu_impl: bool
    a3_impl: bool
    cpu_covering_tests: list[str]
    cpu_representative_test: str | None
    cpu_representative_kind: str
    cpu_runtime_status: str
    cpu_runtime_reason: str
    npu_covering_tests: list[str]
    npu_representative_test: str | None
    npu_representative_kind: str
    npu_runtime_status: str
    npu_runtime_reason: str

    def to_json(self) -> dict[str, object]:
        return {
            "instruction": self.instruction,
            "source": self.source,
            "category": self.category,
            "summary_zh": self.summary_zh,
            "doc_present": self.doc_present,
            "public_intrinsic": self.public_intrinsic,
            "cpu_impl": self.cpu_impl,
            "a3_impl": self.a3_impl,
            "cpu_covering_tests": self.cpu_covering_tests,
            "cpu_representative_test": self.cpu_representative_test,
            "cpu_representative_kind": self.cpu_representative_kind,
            "cpu_runtime_status": self.cpu_runtime_status,
            "cpu_runtime_reason": self.cpu_runtime_reason,
            "npu_covering_tests": self.npu_covering_tests,
            "npu_representative_test": self.npu_representative_test,
            "npu_representative_kind": self.npu_representative_kind,
            "npu_runtime_status": self.npu_runtime_status,
            "npu_runtime_reason": self.npu_runtime_reason,
        }


def load_runtime_cache(path: Path) -> dict[str, dict[str, str]]:
    if not path.exists():
        return {}
    try:
        payload = json.loads(read_text(path))
    except json.JSONDecodeError:
        return {}
    if not isinstance(payload, dict):
        return {}
    return {
        str(key): {
            "status": str(value.get("status", "unknown")),
            "reason": str(value.get("reason", "")),
        }
        for key, value in payload.items()
        if isinstance(value, dict)
    }


def save_runtime_cache(cache: dict[str, dict[str, str]], path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(cache, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")


def trim_reason(text: str, max_lines: int = 20, max_chars: int = 2400) -> str:
    text = text.strip()
    if not text:
        return ""
    lines = text.splitlines()
    clipped = "\n".join(lines[-max_lines:])
    if len(clipped) > max_chars:
        clipped = clipped[-max_chars:]
    return clipped


def run_subprocess(cmd: list[str], cwd: Path, timeout_sec: int) -> dict[str, str]:
    start = time.time()
    try:
        result = subprocess.run(
            cmd,
            cwd=cwd,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            timeout=timeout_sec,
            check=False,
        )
    except subprocess.TimeoutExpired as exc:
        output = trim_reason((exc.stdout or "") + "\n" + (exc.stderr or ""))
        return {
            "status": "timeout",
            "reason": f"timeout after {timeout_sec}s\n{output}".strip(),
        }
    duration = time.time() - start
    output = trim_reason(result.stdout or "")
    if result.returncode == 0:
        reason = f"pass in {duration:.2f}s"
        if output:
            reason = f"{reason}\n{output}"
        return {"status": "pass", "reason": reason}
    return {
        "status": "fail",
        "reason": f"rc={result.returncode} after {duration:.2f}s\n{output}".strip(),
    }


def run_cpu_testcase(testcase: str) -> dict[str, str]:
    build_dir = CPU_BUILD_ROOT / testcase
    if build_dir.exists():
        shutil.rmtree(build_dir)
    build_dir.parent.mkdir(parents=True, exist_ok=True)
    configure = run_subprocess(
        [
            "cmake",
            "-S",
            str(REPO_ROOT / "tests" / "cpu" / "st"),
            "-B",
            str(build_dir),
            f"-DTEST_CASE={testcase}",
        ],
        REPO_ROOT,
        CPU_TIMEOUT_SEC,
    )
    if configure["status"] != "pass":
        configure["status"] = "build_fail"
        return configure
    build = run_subprocess(
        ["cmake", "--build", str(build_dir), "-j", str(os.cpu_count() or 4)],
        REPO_ROOT,
        CPU_TIMEOUT_SEC,
    )
    if build["status"] != "pass":
        build["status"] = "build_fail"
        return build
    binary = build_dir / "bin" / testcase
    if not binary.exists():
        return {"status": "build_fail", "reason": f"missing binary: {binary}"}
    return run_subprocess([str(binary)], build_dir, CPU_TIMEOUT_SEC)


def run_npu_testcase(testcase: str) -> dict[str, str]:
    return run_subprocess(
        [
            "python3",
            str(REPO_ROOT / "tests" / "script" / "run_st.py"),
            "-r",
            "npu",
            "-v",
            "a3",
            "-t",
            testcase,
        ],
        REPO_ROOT,
        NPU_TIMEOUT_SEC,
    )


def runtime_result(
    backend: str,
    representative_test: str | None,
    impl_present: bool,
    enabled: bool,
    allowed_tests: set[str] | None,
    cache: dict[str, dict[str, str]],
) -> tuple[str, str]:
    if not impl_present:
        return "impl_missing", ""
    if not representative_test:
        return "not_covered", ""
    cache_key = f"{backend}:{representative_test}"
    if cache_key in cache:
        return cache[cache_key]["status"], cache[cache_key]["reason"]
    if allowed_tests is not None and representative_test not in allowed_tests:
        return "not_run", "filtered"
    if not enabled:
        return "not_run", ""
    result = run_cpu_testcase(representative_test) if backend == "cpu" else run_npu_testcase(representative_test)
    cache[cache_key] = result
    return result["status"], result["reason"]


def build_rows(
    run_cpu: bool,
    run_npu: bool,
    cpu_allowed_tests: set[str] | None,
    npu_allowed_tests: set[str] | None,
    runtime_cache: dict[str, dict[str, str]],
) -> list[InstructionRow]:
    manifest = load_manifest()
    manifest_names = set(manifest)
    public_intrinsics = set(load_public_intrinsics())
    instructions = sorted(manifest_names | public_intrinsics, key=instruction_sort_key)

    cpu_dirs = list_testcase_dirs(CPU_TESTCASE_ROOT)
    npu_src_dirs = list_testcase_dirs(NPU_SRC_TESTCASE_ROOT)
    npu_comm_dirs = list_testcase_dirs(NPU_COMM_TESTCASE_ROOT)

    cpu_index = build_testcase_index(cpu_dirs, set(instructions))
    npu_src_index = build_testcase_index(npu_src_dirs, set(instructions))
    npu_comm_index_raw = build_testcase_index(npu_comm_dirs, set(instructions))
    npu_comm_index = {
        key: [f"comm/{name}" for name in values]
        for key, values in npu_comm_index_raw.items()
    }

    rows: list[InstructionRow] = []
    for instruction in instructions:
        if instruction in manifest_names and instruction in public_intrinsics:
            source = "manifest+public"
        elif instruction in manifest_names:
            source = "manifest"
        else:
            source = "public_only"
        entry = manifest.get(instruction, {})
        cpu_tests = sorted(set(cpu_index.get(instruction, []) + MANUAL_CPU_TESTS.get(instruction, [])))
        npu_tests = sorted(
            set(npu_src_index.get(instruction, []) + npu_comm_index.get(instruction, []) + MANUAL_NPU_TESTS.get(instruction, []))
        )
        cpu_rep, cpu_kind = pick_representative(instruction, cpu_tests, "cpu")
        npu_rep, npu_kind = pick_representative(instruction, npu_tests, "npu")
        cpu_impl = has_cpu_impl(instruction)
        a3_impl = has_a3_impl(instruction)
        cpu_runtime_status, cpu_runtime_reason = runtime_result(
            "cpu", cpu_rep, cpu_impl, run_cpu, cpu_allowed_tests, runtime_cache
        )
        npu_runtime_status, npu_runtime_reason = runtime_result(
            "npu", npu_rep, a3_impl, run_npu, npu_allowed_tests, runtime_cache
        )
        rows.append(
            InstructionRow(
                instruction=instruction,
                source=source,
                category=str(entry.get("category", "Undocumented Public Intrinsic")),
                summary_zh=str(entry.get("summary_zh", "")),
                doc_present=instruction in manifest_names,
                public_intrinsic=instruction in public_intrinsics,
                cpu_impl=cpu_impl,
                a3_impl=a3_impl,
                cpu_covering_tests=cpu_tests,
                cpu_representative_test=cpu_rep,
                cpu_representative_kind=cpu_kind,
                cpu_runtime_status=cpu_runtime_status,
                cpu_runtime_reason=cpu_runtime_reason,
                npu_covering_tests=npu_tests,
                npu_representative_test=npu_rep,
                npu_representative_kind=npu_kind,
                npu_runtime_status=npu_runtime_status,
                npu_runtime_reason=npu_runtime_reason,
            )
        )
    return rows


def write_json(rows: list[InstructionRow], output: Path) -> None:
    summary = {
        "instruction_count": len(rows),
        "rows": [row.to_json() for row in rows],
        "counts": {
            "doc_present": sum(row.doc_present for row in rows),
            "public_intrinsic": sum(row.public_intrinsic for row in rows),
            "cpu_impl": sum(row.cpu_impl for row in rows),
            "a3_impl": sum(row.a3_impl for row in rows),
            "cpu_test_covered": sum(bool(row.cpu_covering_tests) for row in rows),
            "npu_test_covered": sum(bool(row.npu_covering_tests) for row in rows),
            "cpu_runtime_pass": sum(row.cpu_runtime_status == "pass" for row in rows),
            "npu_runtime_pass": sum(row.npu_runtime_status == "pass" for row in rows),
        },
    }
    output.write_text(json.dumps(summary, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")


def write_csv(rows: list[InstructionRow], output: Path) -> None:
    with output.open("w", encoding="utf-8", newline="") as f:
        writer = csv.DictWriter(
            f,
            fieldnames=[
                "instruction",
                "source",
                "category",
                "doc_present",
                "public_intrinsic",
                "cpu_impl",
                "a3_impl",
                "cpu_representative_test",
                "cpu_representative_kind",
                "cpu_runtime_status",
                "cpu_runtime_reason",
                "cpu_covering_tests",
                "npu_representative_test",
                "npu_representative_kind",
                "npu_runtime_status",
                "npu_runtime_reason",
                "npu_covering_tests",
                "summary_zh",
            ],
        )
        writer.writeheader()
        for row in rows:
            writer.writerow(
                {
                    "instruction": row.instruction,
                    "source": row.source,
                    "category": row.category,
                    "doc_present": "yes" if row.doc_present else "no",
                    "public_intrinsic": "yes" if row.public_intrinsic else "no",
                    "cpu_impl": "yes" if row.cpu_impl else "no",
                    "a3_impl": "yes" if row.a3_impl else "no",
                    "cpu_representative_test": row.cpu_representative_test or "",
                    "cpu_representative_kind": row.cpu_representative_kind,
                    "cpu_runtime_status": row.cpu_runtime_status,
                    "cpu_runtime_reason": row.cpu_runtime_reason,
                    "cpu_covering_tests": json.dumps(row.cpu_covering_tests, ensure_ascii=False),
                    "npu_representative_test": row.npu_representative_test or "",
                    "npu_representative_kind": row.npu_representative_kind,
                    "npu_runtime_status": row.npu_runtime_status,
                    "npu_runtime_reason": row.npu_runtime_reason,
                    "npu_covering_tests": json.dumps(row.npu_covering_tests, ensure_ascii=False),
                    "summary_zh": row.summary_zh,
                }
            )


def write_markdown(rows: list[InstructionRow], output: Path) -> None:
    lines = [
        "# PTO ISA Checklist",
        "",
        f"- 指令总数: `{len(rows)}`",
        f"- 文档覆盖: `{sum(row.doc_present for row in rows)}`",
        f"- Public intrinsic 覆盖: `{sum(row.public_intrinsic for row in rows)}`",
        f"- CPU SIM 实现存在: `{sum(row.cpu_impl for row in rows)}`",
        f"- A3 NPU 实现存在: `{sum(row.a3_impl for row in rows)}`",
        f"- CPU testcase 覆盖: `{sum(bool(row.cpu_covering_tests) for row in rows)}`",
        f"- A3 NPU testcase 覆盖: `{sum(bool(row.npu_covering_tests) for row in rows)}`",
        f"- CPU runtime pass: `{sum(row.cpu_runtime_status == 'pass' for row in rows)}`",
        f"- A3 NPU runtime pass: `{sum(row.npu_runtime_status == 'pass' for row in rows)}`",
        "",
        "| Instruction | Source | CPU Impl | CPU Run | A3 Impl | A3 Run | CPU Test | NPU Test |",
        "|---|---|---:|---|---:|---|---|---|",
    ]
    for row in rows:
        lines.append(
            "| {instruction} | {source} | {cpu_impl} | {cpu_run} | {a3_impl} | {npu_run} | {cpu_test} | {npu_test} |".format(
                instruction=row.instruction,
                source=row.source,
                cpu_impl="yes" if row.cpu_impl else "no",
                cpu_run=row.cpu_runtime_status,
                a3_impl="yes" if row.a3_impl else "no",
                npu_run=row.npu_runtime_status,
                cpu_test=row.cpu_representative_test or "-",
                npu_test=row.npu_representative_test or "-",
            )
        )
    output.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser(description="Generate a PTO ISA checklist against CPU SIM and A3 test coverage")
    parser.add_argument("--output-dir", type=Path, default=REPORT_ROOT)
    parser.add_argument("--run-cpu", action="store_true")
    parser.add_argument("--run-npu", action="store_true")
    parser.add_argument("--cpu-testcases", default="", help="comma-separated CPU testcase names to run")
    parser.add_argument("--npu-testcases", default="", help="comma-separated NPU testcase names to run")
    parser.add_argument("--runtime-cache", type=Path, default=RUNTIME_CACHE_PATH)
    parser.add_argument("--refresh-runtime-cache", action="store_true")
    args = parser.parse_args()

    args.output_dir.mkdir(parents=True, exist_ok=True)
    cpu_allowed_tests = {item.strip() for item in args.cpu_testcases.split(",") if item.strip()} or None
    npu_allowed_tests = {item.strip() for item in args.npu_testcases.split(",") if item.strip()} or None
    runtime_cache = {} if args.refresh_runtime_cache else load_runtime_cache(args.runtime_cache)
    rows = build_rows(args.run_cpu, args.run_npu, cpu_allowed_tests, npu_allowed_tests, runtime_cache)
    write_json(rows, args.output_dir / "pto_isa_checklist_latest.json")
    write_csv(rows, args.output_dir / "pto_isa_checklist_latest.csv")
    write_markdown(rows, args.output_dir / "pto_isa_checklist_latest.md")
    save_runtime_cache(runtime_cache, args.runtime_cache)
    print(f"wrote {len(rows)} instructions to {args.output_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
