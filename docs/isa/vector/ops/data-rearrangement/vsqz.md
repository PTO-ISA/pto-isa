# pto.vsqz

Standalone reference page for `pto.vsqz`. This page belongs to the [Data Rearrangement](../../data-rearrangement.md) instruction set in the PTO ISA manual.

## Summary

Compress — pack active lanes to front.

## Mechanism

`pto.vsqz` is a `pto.v*` compute operation. It applies its semantics to active lanes, obeys the instruction set operand model, and returns its results in vector-register or mask form.

## Syntax

```mlir
%result = pto.vsqz %src, %mask : !pto.vreg<NxT>, !pto.mask -> !pto.vreg<NxT>
```

## Inputs

`%src` is the source vector and `%mask` selects which elements are
  kept.

## Expected Outputs

`%result` is the compacted vector.

## Side Effects

This operation has no architectural side effect beyond producing its SSA results. It does not implicitly reserve buffers, signal events, or establish memory fences unless the form says so.

## Constraints

This is a reduction-style compaction instruction set.
  Preserved element order MUST match source lane order.

## Exceptions

- The verifier rejects illegal operand shapes, unsupported element types, and attribute combinations that are not valid for the selected instruction set or target profile.
- Any additional illegality stated in the constraints section is also part of the contract.

## Target-Profile Restrictions

- A5 is the most detailed concrete profile in the current manual; CPU simulation and A2/A3-class targets may support narrower subsets or emulate the behavior while preserving the visible PTO contract.
- Code that depends on an instruction-set-specific type list, distribution mode, or fused form should treat that dependency as target-profile-specific unless the PTO manual states cross-target portability explicitly.

## Examples

```c
int j = 0;
for (int i = 0; i < N; i++)
    if (mask[i]) dst[j++] = src[i];
while (j < N) dst[j++] = 0;
```

## Detailed Notes

```c
int j = 0;
for (int i = 0; i < N; i++)
    if (mask[i]) dst[j++] = src[i];
while (j < N) dst[j++] = 0;
```

**Use case:** Sparse data compaction, filtering.

## Related Ops / Instruction Set Links

- Instruction set overview: [Data Rearrangement](../../data-rearrangement.md)
- Previous op in instruction set: [pto.vshift](./vshift.md)
- Next op in instruction set: [pto.vusqz](./vusqz.md)
