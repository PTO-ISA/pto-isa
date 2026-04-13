# PTO assembly (PTO-AS)

## Scope

This chapter defines the Virtual ISA contract of PTO-AS as the textual form of PTO programs.
The normative grammar remains:

- `docs/assembly/PTO-AS.md`
- `docs/assembly/PTO-AS.bnf`

## Core form

PTO-AS uses an instruction-centric SSA-like textual form.
A typical statement shape is:

```text
%dst = tadd %src0, %src1 : (!pto.tile<...>, !pto.tile<...>) -> !pto.tile<...>;
```

The textual form SHOULD remain deterministic under round-trip tooling.

## Operand classes

PTO-AS operands include:

- tile operands
- memory/global operands
- scalar/immediate operands
- event/dependency operands (where applicable)
- attributes/modifiers expressed by dictionary form

Each instruction set MUST define required operand classes and positional constraints.

## Attribute and modifier contract

Attributes MUST define:

- name and type
- allowed value domain
- default value policy (if any)
- semantic impact
- diagnostics behavior for invalid values

## Structural validity rules

A structurally valid PTO-AS program MUST satisfy:

- operand/result arity consistency
- type-class compatibility per operation contract
- required attribute presence
- parseable and schema-valid statement forms

## Diagnostics contract

PTO-AS diagnostics MUST be:

- location-aware for parse and structural errors
- deterministic for equivalent inputs
- actionable with expected-vs-actual constraints

## Compatibility and evolution

PTO-AS evolution SHOULD be additive.
Breaking textual-syntax changes MUST be versioned and accompanied by migration guidance.
Toolchains MUST reject unsupported syntax with deterministic diagnostics.
