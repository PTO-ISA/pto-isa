# pto.vbitsort

Standalone reference page for `pto.vbitsort`. This page belongs to the [SFU And DSA Instructions](../../sfu-and-dsa-ops.md) instruction set in the PTO ISA manual.

## Summary

Sort 32 region proposals by score (descending) and materialize sorted proposal records into the destination buffer. Used for hardware-accelerated top-K selection (e.g., NMS — Non-Maximum Suppression).

## Mechanism

`pto.vbitsort` is a UB-to-UB accelerator operation. It reads 32 score values from a source buffer, sorts them in **descending order** by score, and writes fixed-size records to the destination buffer. Each output record contains the original index and the score value.

**Output record format** (8 bytes per record):

| Field | Bytes | Description |
|-------|-------|-------------|
| Upper 4 bytes | `[31:0]` | Original index |
| Lower 4 bytes | `[31:0]` | Score value |

For `f16` score forms, the score occupies the lower 2 bytes of the 4-byte score field; the upper 2 bytes are reserved.

**Sort order:** Descending by score — the highest score is written to the lowest destination address. Equal-score ties preserve the earlier input proposal first (stable sort).

**Repeat count:** `%repeat_times` controls how many adjacent groups of 32 elements to process sequentially.

## Syntax

### PTO Assembly Form

```asm
vbitsort %dest, %src, %indices, %repeat_times : !pto.ptr<T, ub>, !pto.ptr<T, ub>, !pto.ptr<T, ub>, index
```

### AS Level 1 (SSA)

```mlir
pto.vbitsort %dest, %src, %indices, %repeat_times
    : !pto.ptr<f32, ub>, !pto.ptr<f32, ub>, !pto.ptr<i32, ub>, index
```

## Inputs

| Operand | Type | Description |
|---------|------|-------------|
| `%dest` | `!pto.ptr<T, ub>` | UB destination buffer for sorted output records |
| `%src` | `!pto.ptr<T, ub>` | UB source buffer containing scores to sort |
| `%indices` | `!pto.ptr<i32, ub>` | UB index buffer — original indices paired with scores |
| `%repeat_times` | `index` | Number of 32-element groups to process sequentially |

**Note:** The index buffer and score buffer must be pre-populated with the data to be sorted. Each lane of 32 elements corresponds to one output record group.

## Expected Outputs

This op writes UB memory directly and returns no SSA value. Each output record is 8 bytes:

- For `f32` scores: `[index: u32][score: f32]` — upper 4 bytes index, lower 4 bytes score
- For `f16` scores: `[index: u32][score: f16][reserved: u16]` — upper 4 bytes index, lower 2 bytes score, 2 reserved bytes

## Side Effects

This operation has no architectural side effect beyond producing its SSA results. It does not implicitly reserve buffers, signal events, or establish memory fences.

## Constraints

- **Descending sort**: Scores are sorted in **descending** order. The highest score appears at the lowest destination address.
- **Stable sort**: Equal-score ties preserve the original input order.
- **UB-backed pointers**: `%dest`, `%src`, and `%indices` MUST all be backed by UB-space pointers.
- **Alignment contract**: Pointers SHOULD satisfy the backend alignment contract expected by the A5 `VBS32` instruction. Misaligned buffers may produce undefined results.
- **32-element groups**: Each invocation processes exactly 32 score/index pairs. The `%repeat_times` parameter scales this to `32 × repeat_times` total elements.
- **UB-to-UB operation**: This is a UB helper, not a pure `vreg -> vreg` op. It does not use vector registers directly.

## Exceptions

- Illegal if any pointer operand is not a UB-space pointer.
- Illegal if `%repeat_times` is zero or negative.

## Target-Profile Restrictions

- This operation is A5-specific hardware acceleration (`VBS32`).
- CPU simulation may provide a software fallback that preserves the observable PTO semantics.
- Not available on A2/A3 profiles.

## Examples

### Basic NMS-style top-K selection

```mlir
// Pre-condition: 32 scores and their original indices in UB
// %score_buf contains f32 scores
// %idx_buf contains original indices (0-31)

// Sort by score (descending), produce sorted index+score records
pto.vbitsort %sorted_records, %score_buf, %idx_buf, %c1
    : !pto.ptr<f32, ub>, !pto.ptr<f32, ub>, !pto.ptr<i32, ub>, index
```

### Batch processing of multiple 32-element groups

```mlir
// Process 4 groups of 32 (total 128 elements)
pto.vbitsort %dest0, %src0, %idx0, %c1
    : !pto.ptr<f32, ub>, !pto.ptr<f32, ub>, !pto.ptr<i32, ub>, index
pto.vbitsort %dest1, %src1, %idx1, %c1
    : !pto.ptr<f32, ub>, !pto.ptr<f32, ub>, !pto.ptr<i32, ub>, index
pto.vbitsort %dest2, %src2, %idx2, %c1
    : !pto.ptr<f32, ub>, !pto.ptr<f32, ub>, !pto.ptr<i32, ub>, index
pto.vbitsort %dest3, %src3, %idx3, %c1
    : !pto.ptr<f32, ub>, !pto.ptr<f32, ub>, !pto.ptr<i32, ub>, index
```

### NMS pipeline (score → sort → select top-K)

```mlir
// 1. Compute proposal scores
pto.vmul %proposals, %scales, %mask : !pto.vreg<128xf32>, !pto.vreg<128xf32>, !pto.mask -> !pto.vreg<128xf32>

// 2. Copy to UB for sorting
pto.copy_vreg_to_ub %score_ub, %proposals, %c128 : !pto.ptr<f32, ub>, !pto.vreg<128xf32>, index

// 3. Initialize index buffer [0, 1, 2, ..., 127]
%base_idx = pto.vci %c0 {order = "ASC"} : i32 -> !pto.vreg<128xi32>
pto.copy_vreg_to_ub %idx_ub, %base_idx, %c128 : !pto.ptr<i32, ub>, !pto.vreg<128xi32>, index

// 4. Sort by score (descending) in groups of 32
scf.for %g = %c0 to %c4 step %c1 {
  // ... load group g ...
  pto.vbitsort %sorted[%g], %scores[%g], %indices[%g], %c1
      : !pto.ptr<f32, ub>, !pto.ptr<f32, ub>, !pto.ptr<i32, ub>, index
}

// 5. Select top-K (first few entries of sorted output are the highest-scoring)
```

## Performance

### A5 Latency

| Metric | Value | Notes |
|--------|-------|-------|
| Per-invocation latency | TBD | `VBS32` unit — cycles vary by data layout |
| Per-repeat throughput | 4 | `A2A3_RPT_4` equivalent (high-throughput unit) |
| Pipeline interval | 18 | `A2A3_INTERVAL` |

### A2/A3 Throughput

The cost model for `TSORT32` (which invokes `vbitsort` on the tile level) uses the following parameters:

| Metric | Value | Constant |
|--------|-------|----------|
| Startup | 14 | `A2A3_STARTUP_BINARY` |
| Completion | 14 | `A2A3_COMPL_DUP_VCOPY` |
| Per-repeat | 4 | `A2A3_RPT_4` |
| Pipeline interval | 18 | `A2A3_INTERVAL` |

**Repeat calculation**: `repeatTimes = ceil(validCol / blockLen)` where `blockLen = 32`.

**Example**: Sorting 128 f32 elements (`repeatTimes = 4`):

```
total ≈ 14 + 14 + 4 × 4 + 3 × 18 = 28 + 16 + 54 = 98 cycles
```

### Execution Note

The `VBS32` hardware unit is a dedicated sort accelerator. It processes 32 elements per invocation with hardware sorting (descending, stable). For best throughput, batch sort requests and ensure all buffers are 32-element aligned. Misaligned accesses may trigger the software fallback path.

---

## Detailed Notes

`pto.vbitsort` is a hardware-accelerated sort helper designed for:

1. **NMS (Non-Maximum Suppression)**: Sort region proposals by confidence score, then select the top-K non-overlapping regions.
2. **Top-K selection**: Extract the K highest-scoring elements from a set of candidates.
3. **Sorting-based kernels**: Any kernel that benefits from sorting acceleration on fixed-size batches.

**Performance note:** The hardware `VBS32` unit processes exactly 32 elements per invocation. For best performance, organize data in 32-element aligned chunks.

**Relationship with `pto.vmrgsort`**: `vbitsort` performs a descending sort of pre-computed scores/indices. `vmrgsort` merges already-sorted segments. Together they support parallel sort pipelines: partition → local sort (`vsort32`) → merge (`vmrgsort`), or score → sort (`vbitsort`) → select top-K.

## Related Ops / Instruction Set Links

- Instruction set overview: [SFU And DSA Instructions](../../sfu-and-dsa-ops.md)
- Previous op in instruction set: [pto.vsort32](./vsort32.md)
- Next op in instruction set: [pto.vmrgsort](./vmrgsort.md)
