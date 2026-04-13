# Tiles and GlobalTensor

## Scope

This chapter defines data-model contracts between tile operands and global memory operands.
It specifies architecture-visible movement and interpretation rules.

## Tile data model

A tile is the primary architectural data object for compute instructions.
A tile contract includes:

- element type and shape class
- valid-region metadata (`Rv`, `Cv`)
- location-intent role where required by instruction legality
- layout/alignment properties required by backend legality

## GlobalTensor data model

A GlobalTensor (or equivalent memory view) represents addressable global-memory data.
Its architecture-visible contract includes:

- element type compatibility with participating tile operations
- address and stride interpretation required by memory instructions
- visibility behavior under ordering constraints

## GM <-> Tile movement contracts

`TLOAD` and `TSTORE` instruction sets define the primary GM <-> Tile bridge.
Conforming implementations MUST preserve:

- element mapping semantics in the defined valid domain
- required ordering guarantees under event/TSYNC and memory model rules
- documented behavior of quantization/scaling and mode attributes where present

## Shape and domain compatibility

For movement and layout-transform operations:

- source and destination domains MUST satisfy instruction-specific compatibility constraints
- out-of-domain behavior MUST be either explicitly defined (for example pad/fill) or declared unspecified
- backend legality checks MUST reject unsupported shape/layout tuples deterministically

## Layout-transform operations

Operations such as extract/insert/reshape/transpose are architecture-level transforms over tile domains.
They MUST define:

- index-space mapping
- valid-domain mapping
- behavior for partially covered domains
- implementation-defined constraints where hardware-specific behavior exists

## Diagnostics requirements

Movement/layout diagnostics SHOULD report:

- offending operand and operation
- incompatible shape/layout/location dimensions
- relevant index/offset parameter context
- deterministic wording for reproducible CI behavior
