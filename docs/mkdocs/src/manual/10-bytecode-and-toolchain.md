# Bytecode And Toolchain

## Why This Chapter Matters

PTO does not stop at a reference manual and a set of intrinsics. It also has to move programs through textual assembly, structured IR, serialized bytecode, and backend-specific lowering pipelines. If those layers are underspecified, users do not get portability; they get a pile of almost-equivalent representations that drift apart over time.

This chapter defines the interchange contract so that toolchain stages can evolve without silently changing program meaning.

## A Concrete Scenario

Suppose a PTO kernel is emitted as PTO-AS, parsed into IR, serialized to bytecode for caching or transport, then reconstructed and lowered later. The textual form may change whitespace, the IR may normalize structure, and the backend may reorder internal passes. None of that is a problem by itself.

The problem appears when one of those transitions drops required attributes, weakens ordering meaning, or mutates the legality instruction set. That is what the bytecode and toolchain contract is meant to prevent.

## Representation Layers

PTO representation layers are:

1. Virtual ISA semantics
2. PTO-AS textual form
3. PTO IR structured form
4. bytecode serialized interchange form

Layer transitions MUST preserve architecture-observable meaning.

## Bytecode Module Contract

A conforming v1 module MUST preserve:

- operation, block, and function ordering
- SSA def-use topology
- operand and result type information
- required attributes and mode metadata
- symbol and entrypoint identity

If lossless preservation is impossible, serialization MUST fail deterministically instead of degrading the module into something "close enough."

## Validation Pipeline

The recommended validation pipeline is:

1. parse PTO-AS to IR
2. run the structural verifier
3. serialize IR to bytecode
4. deserialize bytecode back to IR
5. re-run the structural verifier
6. optionally run the target legality verifier

CI SHOULD enforce steps 1 through 5. Those steps are the minimum needed to prove that interchange is stable before target-specific legality is even considered.

## Diagnostics Contract

Diagnostics MUST be:

- location-aware for textual forms
- deterministic for equivalent inputs
- actionable, with expected-versus-actual constraints

Minimum error classes are:

- parse error
- structural verification error
- bytecode format or compatibility error
- target legality error

## Compatibility Policy

The evolution policy MUST define:

- a schema version field
- a backward-compatibility window
- handling rules for unknown fields and unknown operations

Default policy:

- unknown required fields: reject
- unknown optional fields: reject unless explicit compatibility mode permits them
- unknown operations: reject with deterministic unsupported-operation diagnostics

## Round-Trip Guarantees

For supported features, `text -> IR -> bytecode -> IR -> text` SHOULD preserve:

- semantics
- verifier-relevant structure
- required metadata

Byte-for-byte textual identity is not required. Semantic and verifier stability are.

## Release Acceptance

Each release SHOULD validate:

- parser positive and negative suites
- structural verifier conformance suites
- malformed-bytecode robustness tests
- round-trip regression corpora
- diagnostic stability snapshots
