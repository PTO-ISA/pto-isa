# pto.vcmax

Standalone reference page for `pto.vcmax`. This page belongs to the [Reduction Instructions](../../reduction-ops.md) instruction set in the PTO ISA manual.

## Summary

Find max element with argmax. Result value + index in lane 0.

## Mechanism

`pto.vcmax` is a `pto.v*` compute operation. It applies its semantics to active lanes, obeys the instruction set operand model, and returns its results in vector-register or mask form.

## Syntax

```mlir
%result = pto.vcmax %input, %mask : !pto.vreg<NxT>, !pto.mask -> !pto.vreg<NxT>
```

Documented A5 types or forms: `i16-i32, f16, f32`.

## Inputs

`%input` is the source vector and `%mask` selects participating
  lanes.

## Expected Outputs

`%result` carries the reduction result in the low destination
  positions.

## Side Effects

This operation has no architectural side effect beyond producing its SSA results. It does not implicitly reserve buffers, signal events, or establish memory fences unless the form says so.

## Constraints

This instruction set computes both the extremum and
  location information, but the exact packing of that information into the
  destination vector depends on the chosen form. If all predicate bits are zero,
  the result follows the zero-filled convention.

## Exceptions

- The verifier rejects illegal operand shapes, unsupported element types, and attribute combinations that are not valid for the selected instruction set or target profile.
- Any additional illegality stated in the constraints section is also part of the contract.

## Target-Profile Restrictions

- Documented A5 coverage: `i16-i32, f16, f32`.
- A5 is the most detailed concrete profile in the current manual; CPU simulation and A2/A3-class targets may support narrower subsets or emulate the behavior while preserving the visible PTO contract.
- Code that depends on an instruction-set-specific type list, distribution mode, or fused form should treat that dependency as target-profile-specific unless the PTO manual states cross-target portability explicitly.

## Examples

```c
T mx = -INF; int idx = 0;
for (int i = 0; i < N; i++)
    if (src[i] > mx) { mx = src[i]; idx = i; }
dst_val[0] = mx;
dst_idx[0] = idx;
```

## Detailed Notes

```c
T mx = -INF; int idx = 0;
for (int i = 0; i < N; i++)
    if (src[i] > mx) { mx = src[i]; idx = i; }
dst_val[0] = mx;
dst_idx[0] = idx;
```

## Related Ops / Instruction Set Links

- Instruction set overview: [Reduction Instructions](../../reduction-ops.md)
- Previous op in instruction set: [pto.vcadd](./vcadd.md)
- Next op in instruction set: [pto.vcmin](./vcmin.md)
