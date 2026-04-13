# Execution Model

## What This Chapter Answers

This chapter explains how a PTO program moves through the machine model and where ordering becomes architectural instead of accidental.

It is tempting to describe PTO with a single slogan such as "host submits work, device schedules work, core executes work." That slogan is true, but not useful enough. PTO users need to know which part of that path can vary by backend and which part must remain stable across lowering, simulation, and execution.

## A Concrete Execution Path

Consider the simple pattern from the overview:

```cpp
TLOAD(a, ga);
TLOAD(b, gb);
TADD(c, a, b);
TSTORE(gc, c);
```

A real implementation may distribute that work differently, but the architecture-visible story is always the same:

1. the host side prepares code and global memory arguments
2. the device side decides where the tile program runs
3. a core-side execution context performs the tile loads, compute, and store
4. any required ordering between those steps must still be visible after lowering

This is why PTO documents a machine model at all. It is not to freeze internal schedulers. It is to explain which boundaries remain meaningful to the programmer and to the verifier.

## The Three Execution Agents

### Host Machine

The host machine prepares workloads, manages global resources, and submits execution. In practice this is where compilation, caching, buffer management, and graph-level orchestration often live.

From the ISA point of view, the host is outside most instruction semantics. Still, it matters because the host is often where Auto mode decisions get materialized and where backend profile selection begins.

### Device Machine

The device machine schedules tile programs across execution resources. It may look like an SPMD launcher on one backend and a more task-oriented scheduler on another.

What PTO cares about is not the exact scheduler algorithm. PTO cares that dependence-ordered work remains dependence-ordered, and that independently runnable work may execute in any legal order.

### Core Machine

The core machine executes tile or scalar instructions and the synchronization primitives that connect them. This is where placement, pipeline interaction, and event ordering become concrete.

The core model is intentionally abstract. PTO does not standardize the exact number of pipelines or the physical storage layout. It standardizes the visible effects of instructions and ordering edges.

## Program Granularity

PTO programs operate at tile granularity. A PTO program is an ordered sequence of operations over tiles, scalars, memory views, and events. Different tile programs may run concurrently if they are independent.

This is one place where PTO deliberately avoids a generic alternative. A thread-centric model would be more familiar on paper, but it would obscure the unit that legality and most high-value optimizations already depend on. Tile granularity is the real programming unit, so the machine model uses it directly.

## Ordering Domains

The architecture-visible ordering rules live in three domains.

### Program-Order Domain

Within a dependent instruction chain, later operations MUST observe the committed effects of earlier operations. PTO does not allow a backend to break an explicit dataflow chain simply because the hardware could speculate internally.

### Event And Synchronization Domain

Events and `TSYNC` define the ordering edges that dataflow alone does not capture. A conforming implementation MUST preserve those edges through lowering, scheduling, and execution.

This is especially important in Manual mode, where the author may be expressing pipeline-safe producer/consumer structure directly.

### Memory-Visibility Domain

`TLOAD` and `TSTORE` participate in the memory-ordering rules defined in the memory chapter. The short version is that required synchronization points must be reflected in visibility, while unspecified global ordering must not be invented by wishful thinking.

## Auto And Manual Responsibilities

Auto and Manual are not separate ISAs. They are separate responsibility splits over the same machine model.

In Auto mode, the compiler or runtime SHOULD infer legal placement, ordering, and scheduling choices. The generated program MUST still preserve the same visible PTO semantics as if those choices had been written explicitly.

In Manual mode, the programmer chooses placement, synchronization, and schedule structure more directly. The toolchain MUST preserve those authored ordering semantics; it is not allowed to "simplify" them away if doing so changes visible behavior.

## What Remains Backend-Defined

Several important details are still implementation-defined:

- scheduler heuristics
- pipeline occupancy and issue behavior
- internal buffering and transient placement
- the supported subset of legality tuples for a given profile

That freedom is intentional. PTO is not trying to freeze microarchitecture. But those details MUST NOT change architecture-defined instruction semantics or invalidate documented ordering guarantees.
