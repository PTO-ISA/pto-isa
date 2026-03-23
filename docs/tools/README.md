# Documentation Tooling

This directory contains the scripts used to keep the PTO ISA documentation synchronized with the public instruction inventory and the MkDocs manual.

## Available Tools

- `check_isa_consistency.py`: Validates the ISA manifest, instruction pages, SVG diagrams, and generated ISA indexes.
- `check_virtual_manual_consistency.py`: Validates the chaptered virtual manual, Appendix D coverage, and manual navigation order.
- `gen_isa_indexes.py`: Regenerates `docs/isa/README*.md` and `docs/PTOISA*.md` from `docs/isa/manifest.yaml`.
- `gen_isa_svgs.py`: Regenerates per-instruction SVG diagrams under `docs/figures/isa/`.
- `gen_virtual_manual_matrix.py`: Regenerates Appendix D instruction-family matrices for the MkDocs manual.
- `normalize_isa_docs.py`: Normalizes English ISA instruction pages and regenerates Chinese counterparts.

## Typical Workflow

```bash
python3 docs/tools/gen_isa_indexes.py
python3 docs/tools/gen_virtual_manual_matrix.py
python3 docs/tools/check_isa_consistency.py
python3 docs/tools/check_virtual_manual_consistency.py
```

Use `docs/mkdocs/check_mkdocs.py` and `docs/website.md` when you need to build or serve the site locally.
