# TCOLARGMIN


## Tile Operation Diagram

![TCOLARGMIN tile operation](../figures/isa/TCOLARGMIN.svg)

## Introduction

Get the row index of the minimum element for each column.

## Math Interpretation

Let `R = src.GetValidRow()` and `C = src.GetValidCol()`. For `0 <= j < C`:

$$ \mathrm{dst}_{0,j} = \underset{0 \le i < R}{\operatorname{argmin}} \; \mathrm{src}_{i,j} $$

## Assembly Syntax

PTO-AS form: see `docs/grammar/PTO-AS.md`.

Synchronous form:

```text
%dst = tcolargmin %src : !pto.tile<...> -> !pto.tile<...>
```
Lowering may introduce internal scratch tiles; the C++ intrinsic requires an explicit `tmp` operand.

### IR Level 1 (SSA)

```text
%dst = pto.tcolargmin %src, %tmp : (!pto.tile<...>, !pto.tile<...>) -> !pto.tile<...>
```

### IR Level 2 (DPS)

```text
pto.tcolargmin ins(%src, %tmp : !pto.tile_buf<...>, !pto.tile_buf<...>) outs(%dst : !pto.tile_buf<...>)
```
## C++ Intrinsic

Declared in `include/pto/common/pto_instr.hpp`:

```cpp
template <typename TileDataOut, typename TileDataIn, typename TileDataTmp, typename... WaitEvents>
PTO_INST RecordEvent TCOLARGMIN(TileDataOut& dst, TileDataIn& src, TileDataTmp& tmp, WaitEvents&... events);
```

## Constraints

Implementation checks (NPU):

- A2A3:
  - Tile location: `dst` and `src` must be `TileType::Vec`.
  - Tile layout of `src`: ND fractal (`isRowMajor` and `SLayout::NoneBox`).
  - Tile layout of `dst`: ND fractal (`isRowMajor` and `SLayout::NoneBox`).
  - Source data types: `half`, `float`, `uint16_t`, `uint32_t`.
  - Destination data types: `uint32_t` or `int32_t`.
  - `tmp` data type must be consistent with `src` data type.
  - Compile-time check: `src.ValidCol` must be `1` or `-1` (dynamic).
  - Runtime valid checks:
    - `srcValidCol != 0` and `srcValidRow != 0`.
    - `dstValidRow == 1`.
    - `srcValidCol == dstValidCol`.
- A5:
  - Tile location: `dst` and `src` must be `TileType::Vec`.
  - Tile layout of `src`: ND fractal (`isRowMajor` and `SLayout::NoneBox`).
  - Tile layout of `dst`: ND fractal (`isRowMajor` and `SLayout::NoneBox`).
  - Source data types: `int8_t`, `uint8_t`, `int16_t`, `uint16_t`, `int32_t`, `uint32_t`, `half`, `float`.
  - Destination data types: `uint32_t` or `int32_t`.
  - Compile-time check: `src.ValidCol` must be `1` or `-1` (dynamic).
  - Runtime valid checks:
    - `srcValidCol != 0` and `srcValidRow != 0`.
    - `dstValidRow == 1`.
    - `srcValidCol == dstValidCol`.
  - `tmp` temporary tile is not used, only for compatibility.

### About temporary tile `tmp` for A2A3

* `tmp` is always used in the A2A3 implementation as scratch space for intermediate results (current index, argmin index, and current min elements).
* `tmp` tile's data type must be the same as `src`'s data type.
* `tmp` tile is organized into three regions within a single row:
  - Region 0 (`[0, tmpGapEles)`): current row index counter (incremented per row).
  - Region 1 (`[tmpGapEles, 2 * tmpGapEles)`): current minimum elements for comparison.
  - Region 2 (`[2 * tmpGapEles, 3 * tmpGapEles)`): argmin index result (before final conversion to `dst`).
* `tmpGapEles` is determined as follows:
  - When `srcValidCol >= elemPerRpt`: `tmpGapEles = elemPerRpt`.
  - When `srcValidCol < elemPerRpt`: `tmpGapEles = ceil(srcValidCol / elemPerBlock) * elemPerBlock`.
* Simply set `tmp` tile size the same as `src` when `src` is small, or calculate the required stride based on `src`'s `validCol` using the following formula:

```text
repeats = ceil(validCol / elementPerRepeat)
stride = ceil(repeats * 2 / elementPerBlock) * elementPerBlock + ceil(repeats / elementPerBlock) * elementPerBlock
```

### About temporary tile `tmp` for A5

* `tmp` temporary tile is **not used** in the A5 implementation. The A5 uses vector register-based computation (`__VEC_SCOPE__`) and does not require scratch tile storage.
* `tmp` is retained in the C++ intrinsic signature solely for API compatibility with A2A3.

## Examples

### Auto

```cpp
#include <pto/pto-inst.hpp>

using namespace pto;

void example_auto() {
  using SrcT = Tile<TileType::Vec, float, 16, 256, BLayout::RowMajor, -1, -1>;
  using DstT = Tile<TileType::Vec, uint32_t, 1, 256, BLayout::RowMajor, -1, -1>;
  using TmpT = Tile<TileType::Vec, float, 1, 32, BLayout::RowMajor, -1, -1>;
  SrcT src(16, 255);
  DstT dst(1, 255);
  TmpT tmp(1, 32);
  TCOLARGMIN(dst, src, tmp);
}
```

### Manual

```cpp
#include <pto/pto-inst.hpp>

using namespace pto;

void example_manual() {
  using SrcT = Tile<TileType::Vec, float, 16, 256, BLayout::RowMajor, -1, -1>;
  using DstT = Tile<TileType::Vec, uint32_t, 1, 256, BLayout::RowMajor, -1, -1>;
  using TmpT = Tile<TileType::Vec, float, 1, 32, BLayout::RowMajor, -1, -1>;
  SrcT src(16, 255);
  DstT dst(1, 255);
  TmpT tmp(1, 32);
  TASSIGN(src, 0x0);
  TASSIGN(dst, 0x1000);
  TASSIGN(tmp, 0x2000);
  TCOLARGMIN(dst, src, tmp);
}
```

## ASM Form Examples

### Auto Mode

```text
# Auto mode: compiler/runtime-managed placement and scheduling.
%dst = pto.tcolargmin %src, %tmp : (!pto.tile<...>, !pto.tile<...>) -> !pto.tile<...>
```

### Manual Mode

```text
# Manual mode: bind resources explicitly before issuing the instruction.
# Optional for tile operands:
# pto.tassign %arg0, @tile(0x1000)
# pto.tassign %arg1, @tile(0x2000)
%dst = pto.tcolargmin %src, %tmp : (!pto.tile<...>, !pto.tile<...>) -> !pto.tile<...>
```

### PTO Assembly Form

```text
%dst = tcolargmin %src : !pto.tile<...> -> !pto.tile<...>
# IR Level 2 (DPS)
pto.tcolargmin ins(%src, %tmp : !pto.tile_buf<...>, !pto.tile_buf<...>) outs(%dst : !pto.tile_buf<...>)
```
</task_progress>
- [x] Write tcolargmin English documentation (docs/isa/TCOLARGMIN.md)
- [ ] Write tcolargmin Chinese documentation (docs/isa/TCOLARGMIN_zh.md)
</task_progress>
</write_to_file>