# 文档工具

本目录包含用于保持 PTO ISA 文档、公开指令清单以及 MkDocs 手册同步的脚本。

## 可用工具

- `check_isa_consistency.py`：校验 ISA manifest、逐指令页面、SVG 图示以及生成的 ISA 索引。
- `check_virtual_manual_consistency.py`：校验章节化虚拟手册、附录 D 覆盖范围以及手册导航顺序。
- `gen_isa_indexes.py`：根据 `docs/isa/manifest.yaml` 重新生成 `docs/isa/README*.md` 与 `docs/PTOISA*.md`。
- `gen_isa_svgs.py`：重新生成 `docs/figures/isa/` 下的逐指令 SVG 图示。
- `gen_virtual_manual_matrix.py`：重新生成 MkDocs 手册中的附录 D 指令族矩阵。
- `normalize_isa_docs.py`：规范化英文 ISA 指令页面，并重新生成中文对应页面。

## 常见流程

```bash
python3 docs/tools/gen_isa_indexes.py
python3 docs/tools/gen_virtual_manual_matrix.py
python3 docs/tools/check_isa_consistency.py
python3 docs/tools/check_virtual_manual_consistency.py
```

如果需要本地构建或预览站点，请结合 `docs/mkdocs/check_mkdocs.py` 与 `docs/website_zh.md` 使用。
