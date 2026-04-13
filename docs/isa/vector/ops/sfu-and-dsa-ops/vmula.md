# pto.vmula

Standalone reference page for `pto.vmula`. This page belongs to the [SFU And DSA Instructions](../../sfu-and-dsa-ops.md) instruction set in the PTO ISA manual.

## Summary

Multiply-accumulate.

## Mechanism

`pto.vmula` is a specialized `pto.v*` operation. It exposes fused, widening, or domain-specific hardware behavior through one stable virtual mnemonic so the instruction set can be reasoned about at the ISA level.

## Syntax

```mlir
%result = pto.vmula %acc, %lhs, %rhs, %mask : !pto.vreg<NxT>, !pto.vreg<NxT>, !pto.vreg<NxT>, !pto.mask -> !pto.vreg<NxT>
```

## Inputs

`%acc` is the accumulator input, `%lhs` and `%rhs` are the
  multiplicands, and `%mask` selects active lanes.

## Expected Outputs

`%result` is the multiply-accumulate result.

## Side Effects

This operation has no architectural side effect beyond producing its SSA results. It does not implicitly reserve buffers, signal events, or establish memory fences unless the form says so.

## Constraints

`pto.vmula` is a fused multiply-accumulate
  operation and is not always interchangeable with separate `vmul` plus `vadd`.

## Exceptions

- The verifier rejects illegal operand shapes, unsupported element types, and attribute combinations that are not valid for the selected instruction set or target profile.
- Any additional illegality stated in the constraints section is also part of the contract.

## Target-Profile Restrictions

- A5 is the most detailed concrete profile in the current manual; CPU simulation and A2/A3-class targets may support narrower subsets or emulate the behavior while preserving the visible PTO contract.
- Code that depends on an instruction-set-specific type list, distribution mode, or fused form should treat that dependency as target-profile-specific unless the PTO manual states cross-target portability explicitly.

## Examples

```c
for (int i = 0; i < N; i++)
    if (mask[i])
        dst[i] = acc[i] + lhs[i] * rhs[i];
```

## Detailed Notes

```c
for (int i = 0; i < N; i++)
    if (mask[i])
        dst[i] = acc[i] + lhs[i] * rhs[i];
```

## Index Generation

## Related Ops / Instruction Set Links

- Instruction set overview: [SFU And DSA Instructions](../../sfu-and-dsa-ops.md)
- Previous op in instruction set: [pto.vmull](./vmull.md)
- Next op in instruction set: [pto.vtranspose](./vtranspose.md)
