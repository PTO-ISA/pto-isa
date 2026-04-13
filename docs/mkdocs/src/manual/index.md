# PTO Virtual Instruction Set Architecture Manual

> Legacy note: this chaptered manual is kept as background material. The preferred entry for PTO VISA is now the merged [PTO ISA manual tree](../docs/isa/README.md).

PTO needs two kinds of documentation at the same time. Kernel authors need a manual that teaches them how the model behaves and where the sharp edges are. Compiler, simulator, and backend engineers need a contract they can implement and test. This manual is the layer that connects those two needs.

The per-instruction pages in `docs/isa/*.md` remain the canonical source for opcode-specific semantics. This manual explains the system around those instructions: why PTO is tile-first, what is architecturally visible, what remains backend-defined, and which invariants every layer of the toolchain must preserve.

## What This Manual Answers

This manual is written for:

- compiler and IR engineers implementing PTO lowering pipelines
- backend engineers implementing target legalization and code generation
- kernel authors validating architecture-visible behavior
- simulator and conformance-test developers

If you are asking questions such as "what does PTO guarantee across hardware generations?", "when is a program portable?", or "where does the backend get freedom to specialize?", this is the document to read before diving into instruction pages.

## How To Read It

The recommended path is:

- [Overview](01-overview.md): why PTO exists and what makes it different from a generic GPU-style ISA
- [Execution Model](02-machine-model.md): how work moves from host to device to core, and where ordering becomes visible
- [State and Types](03-state-and-types.md): how to reason about tiles, valid regions, location intent, and legality
- [Tiles and GlobalTensor](04-tiles-and-globaltensor.md) and [Synchronization](05-synchronization.md): the concrete objects most PTO programs manipulate
- [Programming Guide](08-programming.md): the patterns that remain portable across backends
- [Virtual ISA and IR](09-virtual-isa-and-ir.md), [Bytecode and Toolchain](10-bytecode-and-toolchain.md), [Memory Ordering and Consistency](11-memory-ordering-and-consistency.md), and [Backend Profiles and Conformance](12-backend-profiles-and-conformance.md): the contracts that backend and toolchain implementers must preserve

The appendices are reference material. They should answer follow-up questions, not carry the first explanation.

## How Normative Language Is Used

The key words `MUST`, `MUST NOT`, `SHOULD`, and `MAY` are normative in this manual, but they are used narrowly. A statement only uses those words when a backend, verifier, test, or review can check it.

- `MUST` / `MUST NOT`: mandatory architectural requirement
- `SHOULD`: recommended requirement where deviation is allowed but must be documented
- `MAY`: behavior the architecture explicitly permits

This is deliberate. PTO already has enough abstract terminology; the manual should not add more by turning style advice into pseudo-contract language.

## Authority Order

When documents differ, resolve them in this order:

1. `docs/isa/*.md` for per-instruction semantics and constraints
2. `include/pto/common/pto_instr.hpp` for public API shape and overload instruction set
3. this manual for architecture layering, contracts, and conformance policy

## Maintenance Notes

The English chapters are the structural source for the bilingual manual, but the Chinese chapters are not meant to be machine-translated shadows. Both versions should explain the same architecture, examples, and boundaries in natural prose.

For authoring guidance and the rewrite backlog, see:

- [PTO ISA writing playbook](../docs/reference/pto-isa-writing-playbook.md)
- [Manual rewrite plan](../docs/reference/pto-isa-manual-rewrite-plan.md)
- [Manual review rubric](../docs/reference/pto-isa-manual-review-rubric.md)
