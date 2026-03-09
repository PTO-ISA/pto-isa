# TROWEXPAND

## 指令示意图

![TROWEXPAND tile operation](../figures/isa/TROWEXPAND.svg)

## 简介

将每个源行的第一个元素广播到目标行中。

## 数学语义

Let `R = dst.GetValidRow()` and `C = dst.GetValidCol()`. For `0 <= i < R` and `0 <= j < C`:

$$ \mathrm{dst}_{i,j} = \mathrm{src}_{i,0} $$

## 汇编语法

PTO-AS 形式：参见 [PTO-AS 规范](../assembly/PTO-AS_zh.md)。

同步形式：

```text
%dst = trowexpand %src : !pto.tile<...> -> !pto.tile<...>
```

### AS Level 1（SSA）

```text
%dst = pto.trowexpand %src : !pto.tile<...> -> !pto.tile<...>
```

### AS Level 2（DPS）

```text
pto.trowexpand ins(%src : !pto.tile_buf<...>) outs(%dst : !pto.tile_buf<...>)
```

## C++ 内建接口

声明于 `include/pto/common/pto_instr.hpp`：

```cpp
template <typename TileDataDst, typename TileDataSrc, typename... WaitEvents>
PTO_INST RecordEvent TROWEXPAND(TileDataDst& dst, TileDataSrc& src, WaitEvents&... events);
```

## 约束

实现检查 (NPU):

- Tile Type: `dst`且`src` 必须是 `TileType::Vec`.
- Tile 布局: ND fractal (`isRowMajor`且`SLayout::NoneBox`) 对于都`src`且`dst`.
- Data type: A2A3/A5 element types 必须是以下之一： `int8_t`或`uint8_t`或`int16_t`或`uint16_t`或`int32_t`或`uint32_t`或`half`或`bfloat16_t`或`float`.
- 运行期有效区域检查:
  - A2A3: returns early if any of `dstValidRow`, `dstValidCol`, `srcValidRow`, `srcValidCol` is zero.
  - A5: asserts `srcValidRow == dstValidRow`且asserts `srcValidRow != 0 && srcValidCol != 0`.

## 示例

### 自动（Auto）

```cpp
#include <pto/pto-inst.hpp>

using namespace pto;

void example_auto() {
  using SrcT = Tile<TileType::Vec, float, 16, 16>;
  using DstT = Tile<TileType::Vec, float, 16, 16>;
  SrcT src;
  DstT dst;
  TROWEXPAND(dst, src);
}
```

### 手动（Manual）

```cpp
#include <pto/pto-inst.hpp>

using namespace pto;

void example_manual() {
  using SrcT = Tile<TileType::Vec, float, 16, 16>;
  using DstT = Tile<TileType::Vec, float, 16, 16>;
  SrcT src;
  DstT dst;
  TASSIGN(src, 0x1000);
  TASSIGN(dst, 0x2000);
  TROWEXPAND(dst, src);
}
```

## 汇编示例（ASM）

### 自动模式

```text
# 自动模式：由编译器/运行时负责资源放置与调度。
%dst = pto.trowexpand %src : !pto.tile<...> -> !pto.tile<...>
```

### 手动模式

```text
# 手动模式：先显式绑定资源，再发射指令。
# 可选（当该指令包含 tile 操作数时）：
# pto.tassign %arg0, @tile(0x1000)
# pto.tassign %arg1, @tile(0x2000)
%dst = pto.trowexpand %src : !pto.tile<...> -> !pto.tile<...>
```

### PTO 汇编形式

```text
%dst = trowexpand %src : !pto.tile<...> -> !pto.tile<...>
# IR Level 2 (DPS)
pto.trowexpand ins(%src : !pto.tile_buf<...>) outs(%dst : !pto.tile_buf<...>)
```

