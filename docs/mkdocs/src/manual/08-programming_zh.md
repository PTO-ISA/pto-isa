# 编程指南

## 本章回答什么

和手册其他章节相比，本章的问题更务实：怎么写 PTO 代码，才能在保持正确和可移植的同时，又给 backend 特化留下空间？

重点不是把 PTO 写成一份风格规范，而是说明哪些写法能扛住 backend 变化、profile 收窄和 simulator 验证，哪些写法只是某个目标“刚好没炸”。

## 从一个可移植的基本形状开始

最稳妥的 PTO 程序，往往都从一个朴素内核开始：

```cpp
TLOAD(a, ga);
TLOAD(b, gb);
TADD(c, a, b);
TSTORE(gc, c);
```

这个形状之所以稳，是因为关键边界都写出来了：

- 数据搬运是显式的
- 计算步骤是显式的
- producer 和 consumer 关系是显式的

后续优化的目标，不是把这些边界藏起来，而是在不让它们变模糊的前提下，把它们做快。

## Auto 和 Manual 在实践中的分工

### Auto 模式

当主要风险是合法性，而不是极致调度细节时，Auto 模式是更好的起点。工具链 SHOULD 推导出合法的放置、顺序和调度结构。无论这些决策是不是显式写在源码里，生成代码都 MUST 保持同样的 PTO 可见语义。

当你还在探索 shape、dtype 或算法结构时，Auto 模式尤其合适。它能降低正确性负担，同时仍允许 backend 在文档化的 profile 边界内积极特化。

### Manual 模式

当调度本身就是算法价值的一部分时，就应该进入 Manual 模式：显式双缓冲、producer/consumer overlap、固定 event wiring、面向 backend 的 tile 放置，都是典型例子。

这份控制权伴随着真实责任。用户显式写下的依赖和顺序点 MUST 被工具链保留；非法 manual 配置 MUST 用可执行诊断报错，而不是变成 backend 轮盘赌。

## 一个 worked pattern：load、compute、store

`docs/coding/tutorials/vec-add.md` 展示了基础模式，以及它在 Manual 模式下的 ping-pong 扩展。即便你永远不写这个一模一样的 kernel，它仍然很好地概括了 PTO 的编程节奏：

1. 先选出能明确表达数据形状的 tile 和 global view
2. 再把真正要计算的数据加载进来
3. 只在合法且有意义的 tile 域上做计算
4. 需要时用显式顺序把结果写回或交给下一阶段

为什么要把这个模式写得这么显式，而不是交给 backend 全部推导？因为 PTO 希望在做性能优化之前，正确性问题就已经能回答清楚。

## 真正重要的可移植规则

想跨 backend 存活下来的程序 SHOULD：

- 待在文档定义的指令族合法域内
- 让 dtype、layout、location、shape 的组合落在声明过的 profile 交集里
- 当数据流本身不能保证顺序时，显式写出同步
- 避免依赖 implementation-defined 副作用

这些规则不是写作洁癖，而是防止一个 kernel 在没人意识到的情况下慢慢变成目标私有代码。

## 既重性能又可移植的模式

有些 PTO 模式既适合优化，也仍然可移植：

- 显式 tiling，并把 valid region 管理讲清楚
- 源码里能一眼看见的 phase boundary，特别是 event 和 `TSYNC` 附近
- 通过声明过的 capability check 做 backend 门控特化
- 当首选组合不支持时，提供确定性 fallback

核心原则一直没变：欢迎优化，但架构定义与 backend 定义的分界必须继续可见。

## 常见误区

最常见的非可移植 PTO 习惯其实很好点名：

- 把 valid region 外的值当成结果的一部分读取
- 依赖未文档化的流水线时序
- 在没有依赖或同步边时假设存在隐式顺序
- 不做 profile 门控就硬编码 backend 私有假设

这些问题往往第一次能跑通，之后却会在 simulator、跨 profile 测试或后续 backend 清理时炸出来。

## 建议的验证流程

最可靠的 PTO 开发循环通常是：

1. 结构检查：类型、元数、属性是否正确
2. 合法性检查：shape、layout、location、valid region 是否兼容
3. 同步检查：依赖是否完整、顺序边是否真的存在
4. backend 一致性检查：profile 是否支持、诊断是否稳定
5. 跨代表目标做差分检查

CPU simulator 在这里尤其有价值，因为它会在目标特定行为把问题掩盖之前，先把合法性和顺序问题揪出来。

## 当 implementation-defined 行为不可避免

有些代码最终还是会依赖 implementation-defined 行为。出现这种情况时：

- 这个假设 MUST 被写进文档
- backend profile 约束 MUST 被声明
- 在可行时 SHOULD 提供 fallback 路径

这不是形式主义负担，而是防止今天的战术优化变成明天难以排查的可移植性回归。
