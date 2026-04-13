# PTO 虚拟指令集架构手册

PTO 需要同时满足两类文档需求。写 kernel 的人需要一份能教会他怎么思考 Tile、valid region、同步和可移植边界的手册；写编译器、仿真器、后端的人需要一份能拿来实现和验收的契约。这个 manual 就是把这两件事接起来的那一层。

`docs/isa/*_zh.md` 仍然是单条指令语义的权威来源。本手册负责解释这些指令所在的系统：为什么 PTO 以 Tile 为中心、哪些行为是架构可见的、哪些空间留给后端定义，以及工具链每一层必须守住哪些不变量。

## 这份手册回答什么问题

本手册面向：

- 实现 PTO 降层链路的编译器与 IR 工程师
- 实现目标合法化与代码生成的后端工程师
- 需要验证架构可见行为的内核开发者
- 仿真器与一致性测试开发者

如果你的问题是“PTO 到底想稳定什么？”、“一个程序什么时候算可移植？”、“后端到底哪里可以自由发挥？”，那应该先读这份手册，再去看逐条指令页。

## 建议阅读顺序

推荐按下面的顺序阅读：

- [概述](01-overview_zh.md)：PTO 为什么存在，它和 generic GPU 风格 ISA 有什么不同
- [执行模型](02-machine-model_zh.md)：工作如何从 host 到 device 再到 core，顺序在哪里变得可见
- [状态与类型](03-state-and-types_zh.md)：如何判断 Tile、valid region、location intent 和合法性
- [Tile 与 GlobalTensor](04-tiles-and-globaltensor_zh.md) 与 [同步](05-synchronization_zh.md)：PTO 程序最常操作的对象
- [编程指南](08-programming_zh.md)：哪些写法在跨后端时仍然稳妥
- [虚拟 ISA 与 IR](09-virtual-isa-and-ir_zh.md)、[字节码与工具链](10-bytecode-and-toolchain_zh.md)、[内存顺序与一致性](11-memory-ordering-and-consistency_zh.md)、[后端画像与一致性](12-backend-profiles-and-conformance_zh.md)：后端与工具链实现者必须遵守的契约

附录适合查词和查矩阵，不适合承担第一轮解释。

## 规范性术语怎么用

本手册中的 `MUST`、`MUST NOT`、`SHOULD`、`MAY` 只用于可验证的要求。只有当后端、验证器、测试或评审能真正检查一条要求时，才应该使用这些词。

- `MUST` / `MUST NOT`：强制架构要求
- `SHOULD`：推荐要求，允许偏离，但必须说明
- `MAY`：架构明确允许的可选行为

这样做是刻意的。PTO 现有文档已经有不少抽象术语，不应再把写作建议伪装成规范条款。

## 权威来源优先级

文档冲突时按以下顺序对齐：

1. `docs/isa/*_zh.md`：逐条指令语义与约束
2. `include/pto/common/pto_instr.hpp`：公共 API 形态与重载契约
3. 本手册：分层模型、架构契约与一致性策略

## 维护说明

英文章节是双语 manual 的结构源，但中文章节不应该成为逐句镜像。两种语言都应该自然地解释同一套架构、同一批例子、同一组边界。

写作准则和改造清单见：

- [PTO ISA 写作手册](../docs/reference/pto-isa-writing-playbook_zh.md)
- [Manual 重写计划](../docs/reference/pto-isa-manual-rewrite-plan_zh.md)
- [Manual 评审 rubric](../docs/reference/pto-isa-manual-review-rubric_zh.md)
