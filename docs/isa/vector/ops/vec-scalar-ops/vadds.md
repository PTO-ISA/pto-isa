# pto.vadds

Standalone reference page for `pto.vadds`. This page belongs to the [Vector-Scalar Instructions](../../vec-scalar-ops.md) instruction set in the PTO ISA manual.

## Summary

`%result` is the lane-wise sum.

## Mechanism

`pto.vadds` is a `pto.v*` compute operation. It applies its semantics to active lanes, obeys the instruction set operand model, and returns its results in vector-register or mask form.

## Syntax

```mlir
%result = pto.vadds %input, %scalar, %mask : !pto.vreg<NxT>, T, !pto.mask -> !pto.vreg<NxT>
```

## Inputs

`%input` is the source vector, `%scalar` is broadcast logically to
  each active lane, and `%mask` selects active lanes.

## Expected Outputs

`%result` is the lane-wise sum.

## Side Effects

This operation has no architectural side effect beyond producing its SSA results. It does not implicitly reserve buffers, signal events, or establish memory fences unless the form says so.

## Constraints

Inactive lanes follow the predication
  behavior defined for this instruction set. On the current instruction set, inactive lanes are
  treated as zeroing lanes.

## Exceptions

- The verifier rejects illegal operand shapes, unsupported element types, and attribute combinations that are not valid for the selected instruction set or target profile.
- Any additional illegality stated in the constraints section is also part of the contract.

## Target-Profile Restrictions

- A5 is the most detailed concrete profile in the current manual; CPU simulation and A2/A3-class targets may support narrower subsets or emulate the behavior while preserving the visible PTO contract.
- Code that depends on an instruction-set-specific type list, distribution mode, or fused form should treat that dependency as target-profile-specific unless the PTO manual states cross-target portability explicitly.

## Examples

```c
for (int i = 0; i < N; i++)
    dst[i] = src[i] + scalar;
```

## Detailed Notes

```c
for (int i = 0; i < N; i++)
    dst[i] = src[i] + scalar;
```

## Related Ops / Instruction Set Links

- Instruction set overview: [Vector-Scalar Instructions](../../vec-scalar-ops.md)
- Next op in instruction set: [pto.vsubs](./vsubs.md)
