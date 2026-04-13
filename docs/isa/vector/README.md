# Vector ISA Reference

This section documents the `pto.v*` vector micro-instruction set of PTO ISA. Pages are organized by instruction set, with standalone per-op pages under `vector/ops/`.

## Instruction Sets

| Instruction Set | Description | Operations |
|--------|-------------|------------|
| [Vector Load Store](./vector-load-store.md) | UB↔vector register transfer, gather/scatter | ~25 |
| [Predicate and Materialization](./predicate-and-materialization.md) | Vector broadcast and duplication | 2 |
| [Unary Vector Instructions](./unary-vector-ops.md) | Single-input elementwise operations | 12 |
| [Binary Vector Instructions](./binary-vector-ops.md) | Two-input elementwise operations | 14 |
| [Vector-Scalar Instructions](./vec-scalar-ops.md) | Vector combined with scalar operand | 14 |
| [Conversion Ops](./conversion-ops.md) | Type conversion between numeric types | 3 |
| [Reduction Instructions](./reduction-ops.md) | Cross-lane reductions | 6 |
| [Compare and Select](./compare-select.md) | Comparison and conditional selection | 5 |
| [Data Rearrangement](./data-rearrangement.md) | Lane permutation and packing | 10 |
| [SFU and DSA Instructions](./sfu-and-dsa-ops.md) | Special function units and DSA instructions | 11 |

## Quick Reference

### Common Vector Types

| Type | Description |
|------|-------------|
| `!pto.vreg<NxT>` | Vector register with N lanes of type T |
| `!pto.mask` | Predicate mask (width matches vector length) |
| `!pto.scalar<T>` | Scalar register |

### Vector Lengths

Vector length `N` is a power of 2. Common values depend on the target profile.

## Navigation

The left sidebar provides standalone per-op pages for all vector instructions. Use the instruction set overviews above to understand shared constraints and mechanisms before reading individual opcode pages.

## See Also

- [Vector instructions](../instruction-surfaces/vector-instructions.md)
- [Vector Instruction Set](../instruction-families/vector-families.md)
- [Format of instruction descriptions](../reference/format-of-instruction-descriptions.md)
