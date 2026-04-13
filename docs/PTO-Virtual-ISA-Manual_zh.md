<p align="center">
  <img src="figures/pto_logo.svg" alt="PTO Tile Lib" width="200" />
</p>

# PTO 虚拟 ISA 手册

本文页是 PTO ISA 手册的稳定着陆页。权威手册现位于 `docs/isa/`，作为一棵合并文档树。它将 PTO 呈现为一个多目标虚拟 ISA，在编程模型、机器模型、内存模型、指令表面和家族契约之间有清晰的划分。

使用右上角的语言图标可在英文和中文版本之间切换。切换语言时，有对应中文页面的会跳转到相应页面；否则会回退到当前语言的着陆页。

## 从这里开始

- [PTO ISA 是什么](isa/introduction/what-is-pto-visa.md)
- [文档结构](isa/introduction/document-structure.md)
- [PTO 的设计目标](isa/introduction/goals-of-pto.md)
- [PTO ISA 版本 1.0](isa/introduction/pto-isa-version-1-0.md)
- [范围与边界](isa/introduction/design-goals-and-boundaries.md)
- [编程模型](isa/programming-model/tiles-and-valid-regions.md)
- [机器模型](isa/machine-model/execution-agents.md)
- [语法与操作数](isa/syntax-and-operands/assembly-model.md)
- [通用约定](isa/conventions.md)
- [类型系统](isa/state-and-types/type-system.md)
- [位置意图与合法性](isa/state-and-types/location-intent-and-legality.md)
- [内存模型](isa/memory-model/consistency-baseline.md)

## 手册结构

- [指令表面](isa/instruction-surfaces/README.md)
- [指令族](isa/instruction-families/README.md)
- [指令描述格式](isa/reference/format-of-instruction-descriptions.md)
- [参考注释](isa/reference/README.md)

## 表面参考

- [Tile ISA 参考](isa/tile/README.md)
- [Vector ISA 参考](isa/vector/README.md)
- [标量与控制参考](isa/scalar/README.md)
- [其他与通信参考](isa/other/README.md)
- [语法与操作数](isa/syntax-and-operands/assembly-model.md)
- [通用约定](isa/conventions.md)

## PTO ISA 一览

PTO 是一个跨越多目标的虚拟 ISA，涵盖 CPU 仿真、A2/A3 类目标和 A5 类目标。PTO 可见 ISA 表面并非一个扁平的指令池：

- `pto.t*` 覆盖以 tile 为导向的计算与数据移动
- `pto.v*` 覆盖向量微指令行为及其 buffer/register/predicate 模型
- `pto.*` 覆盖标量、控制、配置和共享支持操作
- 通信与其他支持操作在需要时补全整个表面

手册阐明了 PTO 自身保证的内容与仅作为目标 profile 限制的内容之间的区别。

## 权威入口

合并后的手册索引位于 [PTO ISA 手册与参考](isa/README_zh.md)。
