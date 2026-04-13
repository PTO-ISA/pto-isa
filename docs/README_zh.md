<p align="center">
  <img src="figures/pto_logo.svg" alt="PTO Tile Lib" width="200" />
</p>

# PTO ISA 指南

本文档目录包含权威的 PTO ISA 手册和支持性指令参考。建议阅读路径为合并后的 `docs/isa/` 文档树，将 PTO 作为一个内聚的多目标虚拟 ISA 来呈现，而非在概念手册与参考文档之间割裂。

## 从这里开始

手册页面是 PTO ISA 的稳定着陆页，提供完整阅读路径和指令参考入口。

- [手册入口](PTO-Virtual-ISA-Manual_zh.md)
- [什么是 PTO 虚拟 ISA](isa/introduction/what-is-pto-visa.md)
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

## 快速进入参考文档

如需查阅具体指令，请使用以下参考入口：

- [Tile ISA 参考](isa/tile/README.md)
- [Vector ISA 参考](isa/vector/README.md)
- [标量与控制参考](isa/scalar/README.md)
- [其他与通信参考](isa/other/README.md)
- [通用约定](isa/conventions.md)

完整的章节地图请参阅[文档结构](isa/introduction/document-structure.md)。

## PTO ISA 一览

PTO 是一个跨越多目标的虚拟 ISA，涵盖 CPU 仿真、A2/A3 类目标和 A5 类目标。PTO 可见 ISA 表面并非一个扁平的指令池：

- `pto.t*` 覆盖以 tile 为导向的计算与数据移动
- `pto.v*` 覆盖向量微指令行为及其 buffer/register/predicate 模型
- `pto.*` 覆盖标量、控制、配置和共享支持操作
- 通信与其他支持操作在需要时补全整个表面

手册阐明了 PTO 自身保证的内容与仅作为目标 profile 限制的内容之间的区别。

## 权威入口

合并后的手册索引位于 [PTO ISA 手册与参考](isa/README_zh.md)。

## 文档组织

- `docs/isa/`：权威 PTO ISA 手册与指令表面文档树
- `docs/isa/tile/`：tile 表面参考与家族分组
- `docs/isa/vector/`：源自 PTOAS VPTO 结构的 vector 表面参考
- `docs/isa/scalar/`：标量/控制/配置参考
- `docs/isa/other/`：通信与残余支持表面
- `docs/assembly/`：PTO 汇编语法与规范（PTO-AS）
- `docs/coding/`：扩展 PTO Tile Lib 的开发者说明
- `docs/reference/`：面向维护者的参考材料与文档流程
