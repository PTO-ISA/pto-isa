# pto.vmrgsort

Standalone reference page for `pto.vmrgsort`. This page belongs to the [SFU And DSA Instructions](../../sfu-and-dsa-ops.md) instruction set in the PTO ISA manual.

## Summary

Merge-sort 4 pre-sorted input vectors.

## Mechanism

`pto.vmrgsort` performs a 4-way merge sort. It takes 4 pre-sorted input vectors from UB memory, merges them into a single sorted output vector, and writes the result back to UB.

**Key properties:**
- Inputs MUST be pre-sorted in ascending (or descending, per `%config`) order
- Exactly 4 input segments are merged
- The operation is stable: equal elements retain their relative order from the original inputs
- Sort order is controlled by the `%config` control word

**Control word (`%config`):**
- Encodes sort order direction (ASC/DESC)
- May encode element width and comparison mode
- See the target-profile specification for the full bit-field layout

## Syntax

### PTO Assembly Form

```asm
vmrgsort4 %dest, %src0, %src1, %src2, %src3, %count, %config : !pto.ptr<T, ub>, !pto.ptr<T, ub>, !pto.ptr<T, ub>, !pto.ptr<T, ub>, !pto.ptr<T, ub>, i64, i64
```

### AS Level 1 (SSA)

```mlir
pto.vmrgsort4 %dest, %src0, %src1, %src2, %src3, %count, %config
    : !pto.ptr<f32, ub>, !pto.ptr<f32, ub>, !pto.ptr<f32, ub>, !pto.ptr<f32, ub>, !pto.ptr<f32, ub>, i64, i64
```

## Inputs

| Operand | Type | Description |
|---------|------|-------------|
| `%dest` | `!pto.ptr<T, ub>` | UB destination pointer for merged output |
| `%src0` | `!pto.ptr<T, ub>` | First pre-sorted input segment |
| `%src1` | `!pto.ptr<T, ub>` | Second pre-sorted input segment |
| `%src2` | `!pto.ptr<T, ub>` | Third pre-sorted input segment |
| `%src3` | `!pto.ptr<T, ub>` | Fourth pre-sorted input segment |
| `%count` | `i64` | Number of valid elements per input segment |
| `%config` | `i64` | Sort order and comparison mode control word |

## Expected Outputs

This op writes UB memory directly and returns no SSA value. The sorted result is written to `%dest`.

## Side Effects

This operation has no architectural side effect beyond producing its SSA results. It does not implicitly reserve buffers, signal events, or establish memory fences.

## Constraints

- **Pre-sorted inputs**: ALL four input segments MUST be pre-sorted in the order specified by `%config`. Feeding unsorted data produces undefined output.
- **Same sort order**: All four input segments MUST use the same sort order and comparison mode as encoded in `%config`.
- **Same element type**: All inputs and the destination MUST use the same element type `T`.
- **UB address space**: All pointers MUST have address space `ub`.
- **Single active predicate**: Loading a new predicate does not implicitly save a prior predicate. Programs that need to preserve predicate state MUST save it first.

## Exceptions

- Illegal if any input pointer is not a UB-space pointer.
- Illegal if the effective address (base + areg * 8) is not 64-bit aligned.
- Illegal if the `dist` attribute value is not in the allowed set for this form.

## Target-Profile Restrictions

- A5 is the most detailed concrete profile in the current manual; CPU simulation and A2/A3-class targets may support narrower subsets or emulate the behavior while preserving the visible PTO contract.
- The `config` word layout and supported element types are profile-specific.

## Examples

### Merge 4 sorted segments

```mlir
// Pre-condition: four sorted arrays in UB
// %sorted_a, %sorted_b, %sorted_c, %sorted_d each contain sorted data

// Merge all four into one sorted output
pto.vmrgsort4 %dest,
               %sorted_a, %sorted_b, %sorted_c, %sorted_d,
               %count, %config
    : !pto.ptr<f32, ub>, !pto.ptr<f32, ub>, !pto.ptr<f32, ub>,
       !pto.ptr<f32, ub>, !pto.ptr<f32, ub>, i64, i64
```

### Sort preparation pipeline

```mlir
// Step 1: Sort each segment independently (e.g., using vsort32)
pto.vsort32 %sorted_a, %unsorted_a, %config : !pto.ptr<f32, ub>, !pto.ptr<f32, ub>, i64
pto.vsort32 %sorted_b, %unsorted_b, %config : !pto.ptr<f32, ub>, !pto.ptr<f32, ub>, i64
pto.vsort32 %sorted_c, %unsorted_c, %config : !pto.ptr<f32, ub>, !pto.ptr<f32, ub>, i64
pto.vsort32 %sorted_d, %unsorted_d, %config : !pto.ptr<f32, ub>, !pto.ptr<f32, ub>, i64

// Step 2: Merge the 4 sorted segments
pto.vmrgsort4 %dest,
               %sorted_a, %sorted_b, %sorted_c, %sorted_d,
               %count, %config
    : !pto.ptr<f32, ub>, !pto.ptr<f32, ub>, !pto.ptr<f32, ub>,
       !pto.ptr<f32, ub>, !pto.ptr<f32, ub>, i64, i64
```

## Detailed Notes

`pto.vmrgsort` is a UB-to-UB accelerator operation. Unlike pure `vreg -> vreg` ops that operate on vector registers, this instruction moves data directly between UB locations with hardware-accelerated merge logic.

The 4-way merge pattern is common in:
- **Parallel sort**: Partition data into 4 segments, sort each with `vsort32`, then merge
- **K-way merge**: Merge results from independent sorted streams
- **Top-K selection**: Find the K smallest/largest elements across multiple sources

## Related Ops / Instruction Set Links

- Instruction set overview: [SFU And DSA Instructions](../../sfu-and-dsa-ops.md)
- Previous op in instruction set: [pto.vsort32](./vsort32.md)
