# 状态与类型

## 本章回答什么

本章解释：在 backend 还没开始“发挥聪明才智”之前，怎么判断一个 PTO 程序是否类型正确、结构正确、并且真的合法。

PTO 里最常见的错误，不是看不懂 `TADD` 或 `TMATMUL` 的数学语义，而是误以为“这个 tile 看起来差不多”就一定能当合法操作数。PTO 的合法性活在一组组合关系里：type class、shape、valid region、layout 和 location intent 缺一不可。这也是本章存在的原因。

## 什么算架构状态

PTO 把四类状态视为与可见行为相关：

- tile 值以及 tile 元数据，包括 valid region 元数据
- 标量值和立即数式属性
- 全局内存视图与地址
- 参与顺序关系的同步或事件状态

后端私有的临时状态默认不在架构范围内，除非它改变了可见行为。这个边界很重要。Backend 可以有额外临时 buffer，也可以有隐藏调度状态，但不能把这些隐藏决策变成“架构本来就这样”。

## 主要类型类别

PTO Virtual ISA 会反复用到几类值：

- tile 类值，例如 `!pto.tile<...>`
- memory 或 global view 类值，例如 `!pto.memref<...>` 或等价形式
- 标量值，例如整型、浮点和 index 一类
- 用于同步依赖的 event 或 token 一类

每个指令族 MUST 明确写出每个操作数和结果位置接受哪些类型类别。这样做的目的，是让合法性能在 verifier 边界被检查，而不是留给 backend 猜。

## Tile 合法性的真实形状

当 PTO 用户说“这个 tile 合法吗”，他问的通常不是单一属性，而是一个组合问题。

### 元素类型

`dtype` 是最直观的合法性维度，但它通常不是全部。相同操作形式在 vector tile、acc tile 和某个特定 backend profile 上，接受的 dtype 子集可能都不同。

### 形状与 valid region

PTO 明确区分 tile 的物理尺寸和真正承载语义的区域。这也是 `Rv` 和 `Cv` 如此重要的原因。它们告诉你哪些行列是定义域，哪些只是存储空间。

为什么不强制所有 tile 都完全有效？因为真实 kernel 很快就会重新发明一套自己的 edge-tile 约定。PTO 选择直接把 partial validity 建模出来，好让这些规则能在工具链和 backend 之间共享。

### location intent

`Mat`、`Left`、`Right`、`Acc`、`Bias`、`Scale` 这些 role 参与合法性判断。它们不是给读者看的注释，而是在告诉工具链：这个 tile 准备进入怎样的 producer/consumer 结构。

### layout 与对齐

Layout 和对齐属于合法性表面的一部分，同时也是 backend profile 最常收窄支持范围的地方。虚拟 ISA 负责定义“必须检查哪些维度”，profile 文档负责定义“目标实际上支持哪些子集”。

## valid region 语义

在 PTO 里，valid region 是一等语义：

- 语义只作用在声明的有效域内
- 有效域外的值，除非指令页另有定义，否则都是未指定
- 多输入操作 MUST 定义参与操作的有效域之间如何兼容

标准记号 `Rv` 和 `Cv` 分别表示有效行和有效列。如果某个 backend、verifier 或例子完全忽略这两个符号，通常不是在简化问题，而是在回避真正的边角语义。

## 属性也是契约的一部分

比较模式、舍入模式、变换模式这类属性，不是松散的 modifier，而是操作契约的一部分。一个一致性定义里的属性 MUST 说明：

- 它的类型和允许取值域
- 是否有默认行为
- 它如何改变语义或合法性
- 当取值非法时应该如何报错

## 常见合法性误区

PTO 里最常见的错误其实很稳定：

- 把“dtype 一样”误当成合法性的充分条件
- 忘记 valid region 兼容性和矩形 shape 兼容性是两回事
- 以为 location intent 可以安全地在降层后再猜
- 没做 profile 门控就依赖 backend 私有 layout 行为

## 诊断要求

类型和合法性诊断 SHOULD 至少报告：

- 操作数位置
- 期望与实际的类型类别
- 触发拒绝的合法性维度，例如 dtype、layout、location 或 shape
- 适合 CI 的确定性标识或稳定文案

如果一个诊断只说“illegal tile”，那它通常还不够让 backend 工程师或 kernel 作者真正采取行动。
