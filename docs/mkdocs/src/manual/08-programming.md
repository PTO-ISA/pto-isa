# Programming Guide

## What This Chapter Answers

This chapter answers a more practical question than the rest of the manual: how do you write PTO code that stays correct and portable while still leaving room for backend-specific optimization?

The point is not to turn PTO into a style guide. The point is to explain which patterns survive backend changes, profile narrowing, and simulator validation, and which patterns only work because a particular target happens to tolerate them.

## Start From A Portable Shape

The safest PTO programs start from a boring core:

```cpp
TLOAD(a, ga);
TLOAD(b, gb);
TADD(c, a, b);
TSTORE(gc, c);
```

This shape is portable because the interesting parts are explicit:

- the data movement is visible
- the compute step is visible
- the producer and consumer relationship is visible

When you move beyond this pattern, the goal is not to hide those edges. The goal is to make them more efficient without making them ambiguous.

## Auto And Manual In Practice

### Auto Mode

Auto mode is the right starting point when the main risk is legality, not peak scheduling detail. The toolchain SHOULD choose legal placement, ordering, and schedule structure. Generated code MUST preserve the same visible PTO semantics that the source program expresses.

Auto mode is a good default when you are still exploring shape, dtype, or algorithm structure. It keeps the correctness burden low while still letting the backend specialize aggressively inside documented profile boundaries.

### Manual Mode

Manual mode is for the cases where the schedule is part of the algorithmic value: explicit double buffering, producer and consumer overlap, fixed event wiring, or backend-aware tile placement.

That extra control carries real responsibility. User-authored dependencies and ordering points MUST be preserved by the toolchain, and illegal manual configurations MUST fail with actionable diagnostics rather than becoming backend roulette.

## A Worked Pattern: Load, Compute, Store

The `docs/coding/tutorials/vec-add.md` tutorial shows the basic pattern and its Manual-mode ping-pong extension. Even if you never write that exact kernel, it captures the PTO programming rhythm:

1. choose tiles and global views that make the data shape explicit
2. load the tile data you mean to compute on
3. perform compute only on legal, meaningful tile domains
4. store or hand off the result with explicit ordering where required

Why use a pattern this explicit instead of relying on a backend to infer everything? Because PTO wants correctness questions to be answerable before performance tuning starts.

## Portability Rules That Matter

Programs meant to survive across backends SHOULD:

- stay within documented instruction-set-level legality domains
- keep dtype, layout, location, and shape tuples inside the declared profile intersection
- use explicit synchronization when dataflow alone does not guarantee order
- avoid depending on implementation-defined side effects

These rules are not about style purity. They are what keep a kernel from silently becoming target-specific without anyone noticing.

## Portable Optimization Patterns

Some PTO patterns are both performance-aware and still portable:

- explicit tiling with clear valid-region handling
- phase boundaries that are obvious in the source, especially around events and `TSYNC`
- backend-gated specialization chosen through declared capability checks
- deterministic fallback paths when a preferred tuple is unsupported

The theme is consistent: optimization is welcome, but the boundary between architecture-defined and backend-defined behavior must remain visible.

## Common Mistakes

The most common non-portable PTO habits are easy to name:

- reading values outside the valid region as if they were part of the result
- relying on undocumented pipeline timing
- assuming implicit ordering where no dependency or synchronization edge exists
- hard-coding backend-specific assumptions without profile gating

These mistakes often work once and then fail during simulator runs, cross-profile testing, or later backend cleanup.

## Recommended Validation Workflow

The most reliable PTO development loop is:

1. structural checks: type, arity, and attribute correctness
2. legality checks: shape, layout, location, and valid-region compatibility
3. synchronization checks: dependency completeness and ordering edges
4. backend conformance checks: profile-specific support and diagnostics
5. differential checks across representative targets

The CPU simulator is especially useful here because it catches legality and ordering mistakes before target-specific behavior hides them.

## When Implementation-Defined Behavior Is Unavoidable

Some code will still depend on implementation-defined behavior. When that happens:

- the assumption MUST be documented
- the backend profile constraint MUST be declared
- a fallback path SHOULD be provided when feasible

That documentation burden is not busywork. It is what stops today's tactical optimization from turning into tomorrow's un-debuggable portability regression.
