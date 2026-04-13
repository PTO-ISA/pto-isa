# Instruction set contracts

## Scope

This chapter defines instruction-set-level normative contracts.
Per-op normative details remain in `docs/isa/*.md`.

## Instruction Set taxonomy

PTO instruction sets:

1. synchronization and resource binding
2. elementwise tile-tile operations
3. tile-scalar and tile-immediate operations
4. axis reduce and expand operations
5. memory operations (`GM <-> Tile` and indexed variants)
6. matrix multiply and GEMV operations
7. data movement and layout transforms
8. irregular/complex operations

The source-synchronized inventory is maintained by `docs/isa/manifest.yaml`.

## Common instruction set contract

Every instruction set MUST define:

- operand/result classes and position rules
- semantic domain (valid-region handling)
- required constraints (dtype/layout/location/shape)
- synchronization/ordering implications
- diagnostics behavior for illegal use
- implementation-defined boundaries

## Valid-region-first rule

Unless a specific instruction states otherwise:

- semantics are defined only on the operation's valid domain
- out-of-domain results are unspecified
- instruction set contracts MUST state domain-composition rules for multi-input operations

## Instruction-set-level summaries

### Synchronization and resource binding

Includes `TSYNC`, `TASSIGN`, mode/config instructions.
These operations define ordering or state-configuration effects and MUST preserve architecture ordering semantics.

### Elementwise and scalar variants

Includes arithmetic, bitwise, compare, select, unary math, and scalar-fused forms.
Operations MUST define per-element behavior and mode-specific constraints.

### Reduce/expand instruction sets

Includes row/column reductions and broadcast-like expansions.
Operations MUST define axis semantics and domain compatibility.

### Memory instruction sets

Includes load/store/prefetch and indexed gather/scatter forms.
Operations MUST define mapping between tile domains and memory domains.

### Matrix instruction sets

Includes `TMATMUL*` and `TGEMV*` instruction sets.
Contracts MUST define accumulation domain, operand-role legality, and precision-mode interactions.

### Movement/layout instruction sets

Includes extract/insert/reshape/transpose/fillpad-like transforms.
Contracts MUST define index mapping and domain preservation rules.

### Complex/irregular instruction sets

Includes sort/quant/partial/gather variants and other special operations.
Contracts MUST explicitly identify implementation-defined portions.

## Documentation contract for per-op pages

Each per-instruction page SHOULD follow Appendix B template sections:

- Syntax
- Operands
- Semantics
- Constraints
- Diagnostics
- Implementation-defined behavior
- Compatibility notes

## Coverage and synchronization policy

Instruction Set and instruction indexes MUST stay synchronized with:

- `docs/isa/manifest.yaml`
- `include/pto/common/pto_instr.hpp`
- generated index/matrix tooling in `docs/tools/`
