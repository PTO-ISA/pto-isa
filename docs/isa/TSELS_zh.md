# TSELS

## 指令示意图

![TSELS tile operation](../figures/isa/TSELS.svg)

## 简介

使用标量 `selectMode` 在两个源 Tile 中选择一个（全局选择）。

## 数学语义

对每个元素 `(i, j)` 在有效区域内：

$$
\mathrm{dst}_{i,j} =
\begin{cases}
\mathrm{src0}_{i,j} & \text{if } \mathrm{selectMode} = 1 \\
\mathrm{src1}_{i,j} & \text{otherwise}
\end{cases}
$$

## 汇编语法

PTO-AS 形式：参见 [PTO-AS 规范](../assembly/PTO-AS_zh.md)。

同步形式：

```text
%dst = tsels %src0, %src1, %selectMode : !pto.tile<...>
```

### AS Level 1（SSA）

```text
%dst = pto.tsels %src0, %src1, %scalar : (!pto.tile<...>, !pto.tile<...>, dtype) -> !pto.tile<...>
```

### AS Level 2（DPS）

```text
pto.tsels ins(%src0, %src1, %scalar : !pto.tile_buf<...>, !pto.tile_buf<...>, dtype) outs(%dst : !pto.tile_buf<...>)
```

## C++ 内建接口

声明于 `include/pto/common/pto_instr.hpp`：

```cpp
template <typename TileData, typename... WaitEvents>
PTO_INST RecordEvent TSELS(TileData& dst, TileData& src0, TileData& src1, uint8_t selectMode, WaitEvents&... events);
```

## 约束

- **实现检查 (A2A3)**:
  - `TileData::DType` 必须是以下之一： `half`, `float16_t`, `float`, `float32_t`.
  - Tile 位置必须是向量（`TileData::Loc == TileType::Vec`）。
  - 静态有效边界： `TileData::ValidRow <= TileData::Rows`且`TileData::ValidCol <= TileData::Cols`.
  - 运行时：实现期望 `src0/src1/dst` 具有匹配的有效行/列。
- **实现检查 (A5)**:
  - `sizeof(TileData::DType)` 必须是 `1`、`2` 或 `4` 字节。
  - Tile 位置必须是向量（`TileData::Loc == TileType::Vec`）。
  - 静态有效边界：`TileData::ValidRow <= TileData::Rows` 且 `TileData::ValidCol <= TileData::Cols`。
  - 运行时：实现期望 `src0/src1/dst` 具有匹配的有效行/列。
  - 填充行为取决于 `TileData::PadVal`（`Null`/`Zero` 与 `-INF/+INF` 模式）。
- **有效区域**:
  - 实现使用 `dst.GetValidRow()` / `dst.GetValidCol()` 作为选择域。

## 示例

### 自动（Auto）

```cpp
#include <pto/pto-inst.hpp>

using namespace pto;

void example_auto() {
  using TileT = Tile<TileType::Vec, float, 16, 16>;
  TileT src0, src1, dst;
  TSELS(dst, src0, src1, /*selectMode=*/1);
}
```

### 手动（Manual）

```cpp
#include <pto/pto-inst.hpp>

using namespace pto;

void example_manual() {
  using TileT = Tile<TileType::Vec, float, 16, 16>;
  TileT src0, src1, dst;
  TASSIGN(src0, 0x1000);
  TASSIGN(src1, 0x2000);
  TASSIGN(dst,  0x3000);
  TSELS(dst, src0, src1, /*selectMode=*/1);
}
```

## 汇编示例（ASM）

### 自动模式

```text
# 自动模式：由编译器/运行时负责资源放置与调度。
%dst = pto.tsels %src0, %src1, %scalar : (!pto.tile<...>, !pto.tile<...>, dtype) -> !pto.tile<...>
```

### 手动模式

```text
# 手动模式：先显式绑定资源，再发射指令。
# 可选（当该指令包含 tile 操作数时）：
# pto.tassign %arg0, @tile(0x1000)
# pto.tassign %arg1, @tile(0x2000)
%dst = pto.tsels %src0, %src1, %scalar : (!pto.tile<...>, !pto.tile<...>, dtype) -> !pto.tile<...>
```

### PTO 汇编形式

```text
%dst = tsels %src0, %src1, %selectMode : !pto.tile<...>
# IR Level 2 (DPS)
pto.tsels ins(%src0, %src1, %scalar : !pto.tile_buf<...>, !pto.tile_buf<...>, dtype) outs(%dst : !pto.tile_buf<...>)
```

