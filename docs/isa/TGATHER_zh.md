# TGATHER

## 指令示意图

![TGATHER tile operation](../figures/isa/TGATHER.svg)

## 简介

使用索引 Tile 或编译时掩码模式来收集/选择元素。

## 数学语义

基于索引的 gather（概念性定义）：

Let `R = dst.GetValidRow()` and `C = dst.GetValidCol()`. For `0 <= i < R` and `0 <= j < C`:

$$ \mathrm{dst}_{i,j} = \mathrm{src0}\!\left[\mathrm{indices}_{i,j}\right] $$

Exact index interpretation and bounds behavior are implementation-defined.

Mask-pattern gather is an implementation-defined selection/reduction controlled by `pto::MaskPattern`.

## 汇编语法

PTO-AS 形式：参见 [PTO-AS 规范](../assembly/PTO-AS_zh.md)。

Index-based gather:

```text
%dst = tgather %src0, %indices : !pto.tile<...> -> !pto.tile<...>
```

基于掩码模式的 gather：

```text
%dst = tgather %src {maskPattern = #pto.mask_pattern<P0101>} : !pto.tile<...> -> !pto.tile<...>
```

### AS Level 1（SSA）

```text
%dst = pto.tgather %src, %indices : (!pto.tile<...>, !pto.tile<...>) -> !pto.tile<...>
%dst = pto.tgather %src {maskPattern = #pto.mask_pattern<P0101>}: !pto.tile<...> -> !pto.tile<...>
```

### AS Level 2（DPS）

```text
pto.tgather ins(%src, %indices : !pto.tile_buf<...>, !pto.tile_buf<...>) outs(%dst : !pto.tile_buf<...>)
pto.tgather ins(%src, {maskPattern = #pto.mask_pattern<P0101>} : !pto.tile_buf<...>) outs(%dst : !pto.tile_buf<...>)
```

## C++ 内建接口

声明于 `include/pto/common/pto_instr.hpp` and `include/pto/common/type.hpp`:

```cpp
template <typename TileDataD, typename TileDataS0, typename TileDataS1, typename... WaitEvents>
PTO_INST RecordEvent TGATHER(TileDataD& dst, TileDataS0& src0, TileDataS1& src1, WaitEvents&... events);

template <typename DstTileData, typename SrcTileData, MaskPattern maskPattern, typename... WaitEvents>
PTO_INST RecordEvent TGATHER(DstTileData& dst, SrcTileData& src, WaitEvents&... events);
```

## 约束

- **基于索引的 gather： 实现检查 (A2A3)**:
  - `sizeof(DstTileData::DType)` 必须是 必须是 `int16_t`, `uint16_t`, `int32_t`, `uint32_t`, `half`, `float`.
  - `sizeof(Src1TileData::DType)` 必须是 必须是 `int32_t`, `uint32_t`.
  - `DstTileData::DType` 必须是 相同的类型 `Src0TileData::DType`.
  - `src1.GetValidCol() == Src1TileData::Cols`且`dst.GetValidCol() == DstTileData::Cols`.
- **基于索引的 gather： 实现检查 (A5)**:
  - `sizeof(DstTileData::DType)` 必须是 必须是 `int16_t`, `uint16_t`, `int32_t`, `uint32_t`, `half`, `float`.
  - `sizeof(Src1TileData::DType)` 必须是 必须是 `int16_t`, `uint16_t`, `int32_t`, `uint32_t`.
  - `DstTileData::DType` 必须是 相同的类型 `Src0TileData::DType`.
  - `src1.GetValidCol() == Src1TileData::Cols`且`dst.GetValidCol() == DstTileData::Cols`.
- **基于掩码模式的 gather： 实现检查 (A2A3)**:
  - 源元素大小 必须是 `2`或`4` 字节.
  - `SrcTileData::DType`/`DstTileData::DType` 必须是 `int16_t`或`uint16_t`或`int32_t`或`uint32_t`
   或`half`或`bfloat16_t`或`float`.
  - `dst`且`src` 必须都是 `TileType::Vec`且行主序.
  - `sizeof(dst element) == sizeof(src element)`且`dst.GetValidCol() == DstTileData::Cols` (连续的目标存储).
- **基于掩码模式的 gather： 实现检查 (A5)**:
  - 源元素大小 必须是 `1`或`2`或`4` 字节.
  - `dst`且`src` 必须都是 `TileType::Vec`且行主序.
  - `SrcTileData::DType`/`DstTileData::DType` 必须是 `int8_t`或`uint8_t`或`int16_t`或`uint16_t`或`int32_t`或`uint32_t`
   或`half`或`bfloat16_t`或`float`或`float8_e4m3_t`or `float8_e5m2_t`或`hifloat8_t`.
  - 支持的数据类型限制为目标定义的集合 (通过 `static_assert` 在实现中),且`sizeof(dst element) == sizeof(src element)`, `dst.GetValidCol() == DstTileData::Cols` (连续的目标存储).
- **边界 / 有效性**:
  - Index bounds 不通过显式运行时断言进行验证; 超出范围的索引由目标定义.

## 示例

### 自动（Auto）

```cpp
#include <pto/pto-inst.hpp>

using namespace pto;

void example_auto() {
  using SrcT = Tile<TileType::Vec, float, 16, 16>;
  using IdxT = Tile<TileType::Vec, int32_t, 16, 16>;
  using DstT = Tile<TileType::Vec, float, 16, 16>;
  SrcT src0;
  IdxT idx;
  DstT dst;
  TGATHER(dst, src0, idx);
}
```

### 手动（Manual）

```cpp
#include <pto/pto-inst.hpp>

using namespace pto;

void example_manual() {
  using SrcT = Tile<TileType::Vec, float, 16, 16>;
  using DstT = Tile<TileType::Vec, float, 1, 16>;
  SrcT src;
  DstT dst;
  TASSIGN(src, 0x1000);
  TASSIGN(dst, 0x2000);
  TGATHER<DstT, SrcT, MaskPattern::P0101>(dst, src);
}
```

## 汇编示例（ASM）

### 自动模式

```text
# 自动模式：由编译器/运行时负责资源放置与调度。
%dst = pto.tgather %src, %indices : (!pto.tile<...>, !pto.tile<...>) -> !pto.tile<...>
```

### 手动模式

```text
# 手动模式：先显式绑定资源，再发射指令。
# 可选（当该指令包含 tile 操作数时）：
# pto.tassign %arg0, @tile(0x1000)
# pto.tassign %arg1, @tile(0x2000)
%dst = pto.tgather %src, %indices : (!pto.tile<...>, !pto.tile<...>) -> !pto.tile<...>
```

### PTO 汇编形式

```text
%dst = pto.tgather %src, %indices : (!pto.tile<...>, !pto.tile<...>) -> !pto.tile<...>
# IR Level 2 (DPS)
pto.tgather ins(%src, %indices : !pto.tile_buf<...>, !pto.tile_buf<...>) outs(%dst : !pto.tile_buf<...>)
```

