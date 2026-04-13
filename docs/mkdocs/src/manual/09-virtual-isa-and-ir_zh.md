# 虚拟 ISA 与 IR

## 为什么需要这一层

本章讲的是一条很关键的缝：架构意图如何变成结构化程序表示。PTO 必须把这条缝写清楚，否则合法性检查、可移植承诺和降层不变量都会退化成口口相传。

要避免的误区是把 PTO-AS 或 IR 看成“只是某种序列化细节”。它们不是。它们是让架构可见语义变成 verifier 能检查、backend 能在不发明新语义的前提下降层的那一层。

## 一种实用的分层模型

PTO 使用三层：

1. 虚拟 ISA 层，定义架构可见语义
2. AS / IR 层，提供可验证、可变换的结构化强类型表示
3. backend lowering 层，执行目标相关合法化和代码生成

Backend 特化 MUST 保持虚拟 ISA 的可观察行为。这个说法听起来很显然，但实际工程里，valid region、location intent 和顺序边最容易在这里被弄丢。

## 一个具体场景

设想一个 kernel 里用到了 `TLOAD`、`TADD`、`TSYNC` 和 `TSTORE`。文本 PTO-AS、内存中的 IR 和 backend lowering 后的表示，不需要长得一模一样；但它们必须对下面这些问题给出相同答案：

- 哪些值是 tile、memory view、scalar 或 event
- 哪些操作携带显式顺序意义
- 哪些合法性维度仍然需要检查
- 哪些行为是架构定义的，哪些行为由 profile 决定

这就是本章定义的契约。

## AS 对象模型

一致性 PTO AS 模型 SHOULD 定义：

- 模块与符号契约
- 函数与基本块结构及其顺序
- SSA 值拓扑
- 操作 schema，包括名称、操作数、结果、属性和副作用
- 显式同步与内存副作用

这套结构的目的不是追求形式优雅，而是确保在 backend 开始重塑程序之前，合法性和语义保持已经能被检查。

## verifier 边界

验证被有意拆成两层。

### 结构验证器

AS 层的结构验证器 MUST 检查操作 schema、元数、类型类别和必需属性，并且 MUST 与目标无关。

### 目标合法性验证器

Backend 层的合法性验证器 MUST 检查选定 profile 下的 dtype、layout、location、shape 组合，并且 MUST 对不支持组合给出确定性诊断。

为什么不把两者合并成一个大 verifier？因为结构合法和目标支持是两种不同故障。混在一起，会同时伤害诊断质量和可移植性分析。

## 降层不变量

降层 MUST 保持：

- valid region 语义
- 显式顺序依赖，例如 `event`、`TSYNC` 和内存顺序点
- 架构定义域内的操作含义

降层 MUST NOT 把 implementation-defined 行为静默改写成 architecture-defined 行为。如果 backend 想收窄或特化某种情况，这个决定必须写进 profile 或 legality rule。

## 源同步规则

AS / IR 契约 MUST 与下面几类来源保持同步：

- `docs/isa/*_zh.md`：语义意图
- `include/pto/common/pto_instr.hpp`：API 形态
- `docs/assembly/PTO-AS_zh.md`：文本汇编形态

## 兼容与诊断

增量 AS 变更 SHOULD 优先。破坏性 AS 契约变更 MUST 携带版本信息和迁移说明。未知必需字段 MUST 验证失败；已弃用结构 SHOULD 至少在一个兼容窗口内仍可解析。

AS 和 verifier 诊断 MUST 包含：

- 操作标识和定位上下文
- 期望与实际契约维度的差异
- 适合 CI 回归的确定性错误类别

## 最小一致性场景

一致性验证 SHOULD 覆盖：

- 结构验证器的合法与非法测试
- 按 profile 划分的 backend 合法性通过/失败矩阵
- 经过 PTO-AS 和字节码形态的往返检查
- 对照逐条指令语义的差分检查
