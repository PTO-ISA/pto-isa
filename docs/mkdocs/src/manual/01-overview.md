# Overview

## What This Chapter Answers

This chapter answers a simple question: why does PTO need its own architectural manual instead of looking like a thin wrapper around a generic GPU-style ISA?

The short answer is that PTO is not trying to expose "threads plus opaque local memory." It is trying to expose the objects that PTO programmers and PTO toolchains already reason about: tiles, valid regions, location intent, explicit synchronization, and a split between auto-managed and manually scheduled code. Those choices are visible enough that hiding them behind a generic execution model makes programs harder to write and harder to verify.

## Why PTO Exists

PTO sits in an awkward but useful place. Hardware generations change quickly, on-chip storage details move around, and backends keep learning new tricks. At the same time, kernel authors need a stable vocabulary for describing data movement and compute, and compiler or simulator engineers need contracts they can test. PTO exists to hold that middle layer steady.

That is why PTO is tile-first. Most PTO kernels are already written in terms of tile-sized chunks, not individual scalar threads. A generic SIMD or SIMT abstraction can describe the same hardware eventually, but it pushes the interesting questions down into backend folklore: which data shape is legal, which region is meaningful, when can two tiles alias, where does synchronization actually matter. PTO makes those questions first-class because users already have to answer them.

## A Running Example

The smallest useful PTO story is still a tile program:

```cpp
TileT a(rows, cols), b(rows, cols), c(rows, cols);
GT ga(src0), gb(src1), gc(dst);

TLOAD(a, ga);
TLOAD(b, gb);
TADD(c, a, b);
TSTORE(gc, c);
```

This is deliberately boring, and that is the point. Even this tiny example already depends on the PTO programmer's model:

- the fundamental unit of compute is a tile, not a scalar lane
- the result is only defined on the tile's valid region
- the legality of the operation depends on dtype, shape, layout, and tile role
- ordering is explicit when the dataflow alone is not enough

The rest of the manual keeps unpacking those ideas.

## What Makes PTO Different

### Tile-First Semantics

PTO defines most instruction semantics over tile domains. This is not just a convenience wrapper over vector instructions. It means the architecture cares about tile shape, valid rows and columns, and location intent as part of legality, not merely as backend lowering detail.

The practical consequence is that legality questions show up early. A backend is not free to silently reinterpret an illegal tile combination into "something close enough." It must either accept a documented tuple or reject it with a deterministic diagnostic.

### Valid-Region-First Behavior

PTO does not pretend that every element in a rectangular tile is always meaningful. `Rv` and `Cv` are part of the architectural story because edge tiles, partial tiles, and padded tiles are normal in real kernels.

This is one of the places where PTO is easier to reason about than a generic low-level ISA. Instead of leaving edge behavior to custom conventions in every backend, PTO makes it explicit which region contributes to semantics and which region is unspecified unless an instruction page says otherwise.

### Location Intent, Not Just Raw Storage

Tile roles such as `Mat`, `Left`, `Right`, `Acc`, `Bias`, and `Scale` are not cosmetic. They tell the toolchain and the backend what kind of use a tile is meant for, and they participate in legality checks.

Why not treat every tile as an untyped byte container and let the backend infer intent? Because that would move real architectural errors into late backend heuristics. PTO chooses the opposite tradeoff: make intent visible early, catch mismatches early, and let profile documents explain the supported subsets.

### Auto and Manual Are Both Real PTO

PTO has two programming styles for a reason. Auto mode exists because most users want portability and a reasonable default schedule. Manual mode exists because some kernels only make sense when the author controls placement, synchronization, and pipeline reuse precisely.

The architecture therefore treats both as first-class citizens. Auto mode is not "high-level PTO" and Manual mode is not "escape hatch assembly." They are two responsibility splits over the same visible semantics.

### Synchronization Is Architectural

PTO uses events and `TSYNC` because the pipeline and data-movement boundaries matter. The architecture does not expose every microarchitectural detail, but it does require that ordering edges which are visible to a programmer stay visible through lowering and execution.

## Architecture Boundary

PTO defines:

- observable instruction results inside the valid region
- legality boundaries that users and toolchains can check
- ordering and synchronization semantics that survive lowering

PTO does not define:

- microarchitectural scheduling details
- exact on-chip storage layout
- backend-specific optimization strategies

Backend-specific behavior is allowed, but it must be documented as implementation-defined rather than smuggled in as "implied architecture."

## Source Of Truth

The manual does not replace the rest of the repository. It composes the other sources into a system-level contract:

- [PTO ISA Reference](../docs/isa/README.md) for per-instruction semantics
- `include/pto/common/pto_instr.hpp` for public API signatures and overload instruction set
- [PTO-AS Specification](../docs/assembly/PTO-AS.md) and `docs/assembly/PTO-AS.bnf` for textual assembly forms

## Compatibility Principles

PTO should evolve by adding capability and tightening documentation, not by silently changing old meaning. The practical rules are simple:

- additive evolution SHOULD be preferred over breaking changes
- breaking architectural changes MUST carry explicit versioning and migration notes
- implementation-defined behavior MUST stay tagged consistently across the manual, IR contracts, and backend profile documents
