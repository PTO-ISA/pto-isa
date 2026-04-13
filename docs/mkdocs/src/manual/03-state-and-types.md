# State And Types

## What This Chapter Answers

This chapter explains how to tell whether a PTO program is well-typed, well-formed, and legal before a backend ever starts "being clever."

The common failure mode in PTO is not misunderstanding the mathematical meaning of `TADD` or `TMATMUL`. It is assuming that a tile which looks plausible is automatically a legal operand. In PTO, legality lives in the combination of type class, shape, valid region, layout, and location intent. That is why this chapter exists.

## What Counts As Architectural State

PTO models four kinds of state that matter to visible behavior:

- tile values together with tile metadata, including valid-region metadata
- scalar values and immediate-style attributes
- global memory views and addresses
- synchronization or event state that participates in ordering

Backend-private transient state is intentionally out of scope unless it changes visible behavior. That separation matters. A backend may use extra temporary buffers or hidden scheduling state, but it is not allowed to make those hidden choices observable as architecture behavior.

## The Main Type Classes

PTO Virtual ISA uses a small set of type classes repeatedly:

- tile-like values such as `!pto.tile<...>`
- memory or global-view values such as `!pto.memref<...>` or equivalent forms
- scalar values such as integer, float, and index-like classes
- event or token-like values for synchronization dependencies

Each instruction set MUST define which type classes are accepted for every operand and result position. That requirement exists to keep legality checkable at the verifier boundary instead of leaving it to backend guesswork.

## The Real Shape Of Tile Legality

When PTO users say "is this tile legal here?", they usually mean a combination question, not a single-property question.

### Element Type

`dtype` is the obvious part of legality, but it is rarely the whole story. The same operation form may accept one dtype in a vector tile, another in an accumulator tile, and a narrower subset in a backend profile.

### Shape And Valid Region

PTO distinguishes the physical tile extent from the part that carries meaningful semantics. That distinction is why `Rv` and `Cv` matter so much. They tell you which rows and columns are part of the defined result and which are only storage.

Why not force every tile to be fully valid? Because real kernels would immediately recreate edge-tile conventions in ad hoc ways. PTO chooses to model partial validity directly so the rules can be shared across toolchains and backends.

### Location Intent

Roles such as `Mat`, `Left`, `Right`, `Acc`, `Bias`, and `Scale` participate in legality. They are not comments for the reader. They tell the toolchain what kind of producer/consumer structure the tile is meant to enter.

### Layout And Alignment

Layout and alignment are part of the legality instruction set, but they are also where backend profiles often narrow support. The virtual ISA defines the dimensions that must be checked; the profile documents define which subsets are actually supported on a target.

## Valid Region Semantics

Valid-region semantics are first-class in PTO:

- semantics apply to indices inside the declared valid domain
- values outside the valid domain are unspecified unless an instruction page defines them
- multi-operand operations MUST define the compatibility rule between the participating valid domains

The standard notation uses `Rv` and `Cv` for valid rows and valid columns. If a backend, verifier, or example ignores those symbols, it is usually hiding a real edge-case question rather than simplifying one.

## Attributes Are Part Of The Contract

Instruction attributes such as compare mode, rounding mode, and transform mode are not loose modifiers. They are part of the operation contract. A conforming specification for an attribute MUST define:

- its type and allowed value domain
- its default behavior, if one exists
- how it changes semantics or legality
- what diagnostic should appear when the value is invalid

## Common Legality Mistakes

The most common PTO mistakes are predictable:

- treating "same dtype" as sufficient legality proof
- forgetting that valid-region compatibility is separate from rectangular shape compatibility
- assuming location intent can be inferred safely after lowering
- depending on backend-private layout behavior without profile gating

## Diagnostics Requirements

Type and legality diagnostics SHOULD report:

- operand position
- expected and actual type class
- the legality dimensions that caused rejection, such as dtype, layout, location, or shape
- deterministic identifiers or stable wording suitable for CI

If a diagnostic only says "illegal tile," it is usually not precise enough for a backend engineer or a kernel author to act on it.
