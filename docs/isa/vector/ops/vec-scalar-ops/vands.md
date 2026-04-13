# pto.vands

Standalone reference page for `pto.vands`. This page belongs to the [Vector-Scalar Instructions](../../vec-scalar-ops.md) instruction set in the PTO ISA manual.

## Summary

`%result` is the lane-wise bitwise AND.

## Mechanism

`pto.vands` is a `pto.v*` compute operation. It applies its semantics to active lanes, obeys the instruction set operand model, and returns its results in vector-register or mask form.

## Syntax

```mlir
%result = pto.vands %input, %scalar, %mask : !pto.vreg<NxT>, T, !pto.mask -> !pto.vreg<NxT>
```

## Inputs

`%input`, `%scalar`, and `%mask` as above.

## Expected Outputs

`%result` is the lane-wise bitwise AND.

## Side Effects

This operation has no architectural side effect beyond producing its SSA results. It does not implicitly reserve buffers, signal events, or establish memory fences unless the form says so.

## Constraints

Integer element types only.

## Exceptions

- The verifier rejects illegal operand shapes, unsupported element types, and attribute combinations that are not valid for the selected instruction set or target profile.
- Any additional illegality stated in the constraints section is also part of the contract.

## Target-Profile Restrictions

- A5 is the most detailed concrete profile in the current manual; CPU simulation and A2/A3-class targets may support narrower subsets or emulate the behavior while preserving the visible PTO contract.
- Code that depends on an instruction-set-specific type list, distribution mode, or fused form should treat that dependency as target-profile-specific unless the PTO manual states cross-target portability explicitly.

## Examples

```c
for (int i = 0; i < N; i++)
    dst[i] = src[i] & scalar;
```

## Detailed Notes

```c
for (int i = 0; i < N; i++)
    dst[i] = src[i] & scalar;
```

## Related Ops / Instruction Set Links

- Instruction set overview: [Vector-Scalar Instructions](../../vec-scalar-ops.md)
- Previous op in instruction set: [pto.vmins](./vmins.md)
- Next op in instruction set: [pto.vors](./vors.md)
