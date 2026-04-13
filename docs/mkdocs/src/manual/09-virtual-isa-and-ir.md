# Virtual ISA And IR

## Why This Layer Exists

This chapter is about the seam where architecture intent turns into structured program representation. PTO needs that seam to be explicit because otherwise every legality check, portability promise, and lowering invariant turns into folklore.

The mistake to avoid is treating PTO-AS or IR as mere serialization detail. They are not. They are where architecture-visible meaning becomes something a verifier can inspect and a backend can lower without inventing new semantics.

## A Practical Layering Model

PTO uses three layers:

1. the Virtual ISA layer, which defines architecture-visible semantics
2. the AS or IR layer, which provides structured typed representation for verification and transformation
3. the backend lowering layer, which performs target-specific legalization and code generation

Backend specialization MUST preserve Virtual ISA-observable behavior. That sounds obvious, but in practice it is exactly where valid-region handling, location intent, and ordering edges get lost if the layering is underspecified.

## A Concrete Scenario

Consider a kernel that uses `TLOAD`, `TADD`, `TSYNC`, and `TSTORE`. The textual PTO-AS form, the in-memory IR form, and the backend-lowered representation do not need to look identical. They do need to preserve the same answers to the questions that matter:

- which values are tiles, memory views, scalars, or events
- which operations carry explicit ordering meaning
- which legality dimensions still have to be checked
- which behavior is architectural and which is profile-specific

That is the contract this chapter defines.

## AS Object Model

A conforming PTO AS model SHOULD define:

- module and symbol contracts
- function and block structure plus ordering
- SSA value topology
- operation schema, including name, operands, results, attributes, and effects
- explicit synchronization and memory effects

The point of this structure is not elegance. It is to ensure that legality and semantic preservation can be checked before target-specific lowering starts reshaping the program.

## Verifier Boundary

Verification is intentionally split into two levels.

### Structural Verifier

The AS-level structural verifier MUST validate operation schema, arity, type classes, and required attributes. It MUST remain target-independent.

### Target Legality Verifier

The backend-level legality verifier MUST validate dtype, layout, location, and shape tuples for the selected backend profile. It MUST emit deterministic diagnostics for unsupported tuples.

Why keep these separate instead of merging them into one big verifier? Because structural validity and target support are different failure modes. Mixing them makes both diagnostics and portability reasoning worse.

## Lowering Invariants

Lowering MUST preserve:

- valid-region semantics
- explicit ordering dependencies such as `event`, `TSYNC`, and memory-ordering points
- operation meaning inside architecture-defined domains

Lowering MUST NOT silently reinterpret implementation-defined behavior as if it were architecture-defined. If a backend wants to narrow or specialize a case, that choice belongs in a documented profile or legality rule.

## Source Alignment Rules

AS and IR contracts MUST stay synchronized with:

- `docs/isa/*.md` for semantic intent
- `include/pto/common/pto_instr.hpp` for API-level shape
- `docs/assembly/PTO-AS.md` for textual assembly-facing forms

## Compatibility And Diagnostics

Additive AS changes SHOULD be preferred. Breaking AS contract changes MUST include versioning and migration notes. Unknown required fields MUST fail verification, and deprecated constructs SHOULD remain parseable for at least one compatibility window.

AS and verifier diagnostics MUST include:

- operation identifier and location context
- expected versus actual contract dimensions
- deterministic error class suitable for CI regression

## Minimum Conformance Scenarios

Conformance validation SHOULD include:

- legal and illegal structural verifier tests
- backend legality pass and fail matrices by profile
- round-trip checks through PTO-AS and bytecode forms
- differential checks against per-instruction semantics
