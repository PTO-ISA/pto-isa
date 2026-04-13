# Vector Instruction Set: Shared Scalar Arithmetic

Vector programs in PTO rely on the shared MLIR `arith` instruction set for scalar setup around `pto.v*` regions. This page keeps that relationship explicit without pretending that scalar bookkeeping is itself a vector payload instruction set.

## Summary

Shared scalar arithmetic is part of the documented PTO source instruction set. It feeds vector regions with constants, offsets, loop bounds, and scalar predicates, but it does not replace `pto.v*` compute.

## Mechanism

Around vector code, `arith` is used to:

- compute UB offsets and loop counters
- derive tail counts and active-lane conditions
- build scalar values broadcast or materialized into vector state
- compare scalar loop/control values that guard vector regions

The canonical scalar-side explanation lives in [Scalar And Control Instruction Set: Shared Scalar Arithmetic](../scalar/shared-arith.md). This page exists so the vector reference stays self-contained about what surrounds `pto.v*` execution.

## Inputs

- scalar integers
- scalar floating-point values
- `index` values
- boolean-like results from scalar comparisons

## Expected Outputs

- scalar values consumed by vector configuration or control
- branch predicates for structured control around vector scopes
- scalar operands later materialized into vector state

## Constraints

- `arith` MUST remain scalar; vector payload math belongs to `pto.v*`.
- Width changes, `index` conversions, and scalar comparisons that affect vector legality SHOULD be spelled explicitly.
- This shared instruction set MUST be documented as supporting source syntax, not as hidden compiler-only machinery.

## Cases That Are Not Allowed

- documenting scalar setup as if it were a vector ALU instruction set
- using `arith` to stand in for vector-register semantics
- leaving scalar-to-vector boundary assumptions implicit

## Related Ops And Instruction Set Links

- [Scalar And Control Instruction Set: Shared Scalar Arithmetic](../scalar/shared-arith.md)
- [Vector Instruction Set: Predicate And Materialization](./predicate-and-materialization.md)
- [Vector Instruction Set: Shared Structured Control Flow](./shared-scf.md)
