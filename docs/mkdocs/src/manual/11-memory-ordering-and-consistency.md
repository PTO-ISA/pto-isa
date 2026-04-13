# Memory Ordering And Consistency

## Why PTO Needs A Memory Chapter

Many PTO bugs are not arithmetic bugs. They are ordering bugs that happened to pass once because a backend or simulator behaved generously. PTO therefore needs to say not only what `TLOAD` and `TSTORE` do, but also when their effects are required to become visible.

This chapter defines that visible contract without pretending to standardize every cache policy or internal pipeline detail.

## A Concrete Scenario

Imagine one tile program stores data that a later tile program loads. If the programmer or toolchain expressed a required producer and consumer edge, the consumer must not observe stale data just because the target happened to buffer aggressively. On the other hand, if no dependency or synchronization edge exists, PTO should not invent a global-ordering guarantee out of thin air.

That is the balance this chapter defines.

## Memory Objects And Domains

Architecture-visible memory domains include:

- tile-local values
- global memory views accessed by memory operations
- synchronization state that affects visibility boundaries

Backend-private caches and buffers are implementation-defined, but they MUST respect architecture-visible ordering outcomes.

## Consistency Baseline

PTO uses dependency-ordered consistency as the baseline model:

- data dependencies and explicit synchronization define required visibility order
- independent operations MAY be reordered internally
- required synchronization points MUST establish visibility as specified

This is a deliberate middle ground. PTO does not promise full global ordering everywhere, but it also does not let targets erase programmer-visible dependency structure.

## Ordering Guarantees

A conforming implementation MUST ensure:

- producer writes become visible to dependent consumers after the required synchronization or ordering points
- memory operations in explicit dependency chains preserve those chains
- semantics defined by `TSYNC` and event dependencies are reflected in memory visibility

## Unspecified Versus Implementation-Defined

PTO needs both terms here, and they mean different things.

- accesses or interpretations outside defined domains may be unspecified
- timing, cache policy, and similar internal details are implementation-defined
- backend-specific memory optimizations are allowed only when they preserve required visible behavior

The practical rule is simple: unspecified behavior is not portable data; implementation-defined behavior must be documented if users are expected to rely on it.

## Programming Requirements

Programs SHOULD:

- use explicit synchronization at producer and consumer boundaries
- avoid assuming implicit global ordering without a defined dependency
- avoid relying on unspecified out-of-domain values

Manual-mode programmers MUST ensure required ordering when tool-managed synchronization is not used.

## Diagnostics And Conformance

Backends SHOULD provide diagnostics for:

- missing ordering assumptions in illegal contexts
- unsupported memory-ordering forms
- profile-specific restrictions

Conformance tests SHOULD include ordered-visibility scenarios across representative dependency patterns.
