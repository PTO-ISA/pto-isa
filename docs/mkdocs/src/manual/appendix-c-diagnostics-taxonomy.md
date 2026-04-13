# Diagnostics Taxonomy

## Scope

This appendix defines the diagnostic taxonomy and stability requirements for PTO Virtual ISA toolchains.

## Diagnostic quality contract

All diagnostics SHOULD satisfy:

- deterministic error class
- deterministic primary message shape
- actionable context (expected vs actual)
- source location when available

## Primary diagnostic classes

### Parse diagnostics (`PARSE_*`)

Use for textual PTO-AS errors:

- malformed token
- grammar violation
- invalid literal/attribute syntax

### Structural diagnostics (`STRUCT_*`)

Use for IR shape violations:

- wrong operand/result arity
- missing required attributes
- incompatible type classes

### Legality diagnostics (`LEGAL_*`)

Use for backend/profile legality failures:

- unsupported dtype/layout/location/shape tuple
- unsupported mode combination
- unsupported instruction variant in selected profile

### Ordering diagnostics (`ORDER_*`)

Use for synchronization/ordering failures:

- missing required dependency edge
- invalid synchronization form
- ordering contract violation

### Bytecode diagnostics (`BCODE_*`)

Use for interchange/serialization failures:

- unsupported bytecode version
- malformed section/record
- unknown required field/opcode

## Recommended message fields

Diagnostics SHOULD include:

- error class (stable identifier)
- operation name and operand position (if applicable)
- expected contract summary
- actual offending value/shape/type/mode
- location or source context

## Stability policy

- Error class identifiers MUST be stable across patch releases.
- Message wording SHOULD remain stable for CI snapshots.
- If message wording changes materially, release notes SHOULD document the change.

## Example format

```text
LEGAL_UNSUPPORTED_TUPLE: tmatmul operand src1 has unsupported tuple
  expected: layout in {fractal_a, fractal_b}, dtype in {fp16, bf16}
  actual: layout=row_major, dtype=int8
  context: backend_profile=A3, op_loc=line 42
```
