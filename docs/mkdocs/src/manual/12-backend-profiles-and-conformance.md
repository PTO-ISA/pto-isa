# Backend Profiles And Conformance

## Why Profiles Exist

PTO wants a stable architectural instruction set, but it does not pretend that every backend supports every legal-looking tuple. Profiles are the mechanism that keeps those two facts compatible.

A profile says, in effect: here is the subset this backend truly supports, here is what remains implementation-defined, and here is the conformance level we are willing to claim publicly. Without that layer, backend limitations leak into the architecture by accident.

## A Concrete Scenario

A tile tuple may be architecturally meaningful yet unsupported on a specific target. That should not force the architecture to shrink. It should force the backend profile to say so clearly.

Likewise, a backend may support an aggressive optimization that changes internal scheduling. That should not automatically become a portability promise. It only becomes part of the public contract if the profile documents it.

## Backend Profile Model

A backend profile MUST document:

- supported instruction sets and operation forms
- supported dtype, layout, location, and shape tuples
- synchronization and memory-ordering limitations
- implementation-defined behavior instruction set
- diagnostics policy for unsupported features

Profiles MAY correspond to concrete targets such as A2, A3, A5, or the CPU simulator.

## Capability Gating

Toolchains MUST gate backend-specific specialization using declared profile capability. If requested behavior falls outside profile support:

- compilation or legalization MUST fail deterministically, or
- an explicitly defined fallback path MUST be selected

Why insist on capability gating instead of "best effort" compilation? Because silent target drift is one of the fastest ways to make a supposedly portable kernel become target-private without anyone noticing.

## Conformance Dimensions

Conformance is evaluated along four dimensions:

1. semantic conformance: instruction behavior
2. legality conformance: contract validation
3. ordering conformance: synchronization and memory visibility
4. diagnostic conformance: deterministic actionable errors

## Conformance Levels

Recommended levels are:

- **Level 0 (parse/shape)**: structural toolchain correctness only
- **Level 1 (instruction set legality)**: documented instruction-set-level legality and diagnostics
- **Level 2 (instruction semantic)**: per-instruction semantics validated on representative suites
- **Level 3 (cross-layer stability)**: semantic, ordering, and diagnostics stability across AS, bytecode, and backend transitions

A backend SHOULD publish the highest validated level together with known gaps.

## Required Test Matrix

A profile conformance suite SHOULD include:

- legal and illegal tuple tests by instruction set
- synchronization and memory-ordering scenarios
- precision and mode interaction tests, including mixed-precision paths
- round-trip toolchain tests across text, AS, and bytecode forms
- deterministic diagnostic snapshots

## Change Management

When backend behavior changes:

- profile documents MUST be updated in the same change set
- conformance impact MUST be stated
- regressions against published levels MUST be treated as release blockers unless explicitly waived with rationale
