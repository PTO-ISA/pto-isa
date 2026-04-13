# 总览

## 本章回答什么

本章回答一个最基础的问题：为什么 PTO 需要一套自己的架构手册，而不是把自己写成 generic GPU 风格 ISA 上的一层薄封装？

简短回答是：PTO 暴露的不是“线程 + 一块模糊的本地内存”，而是 PTO 程序员和 PTO 工具链本来就在思考的对象：Tile、valid region、location intent、显式同步，以及 Auto/Manual 两种责任分工。既然这些概念已经决定了程序怎么写、怎么验、怎么降层，那么把它们藏到后端细节里只会让文档更难懂，程序更难验证。

## PTO 为什么存在

PTO 处在一个很实用但也很尴尬的位置。硬件代际变化快，片上存储布局和流水线行为会变，后端还会持续学会新的优化技巧。与此同时，kernel 作者需要一套稳定词汇来描述数据搬运和计算，编译器与仿真器工程师需要一套可以落到测试上的契约。PTO 的存在，就是为了把这层中间语义稳定下来。

这也是 PTO 选择 tile-first 的原因。大多数 PTO kernel 本来就是按 tile 组织，而不是按单个标量线程组织。generic SIMD 或 SIMT 抽象当然也能最终描述同一块硬件，但它会把真正重要的问题都压到后端传说里：这个 shape 到底合法吗？边角 tile 哪些元素有效？两个 tile 在什么条件下可以别名？同步到底在哪一层变得必要？PTO 选择把这些问题抬成一等概念，因为用户本来就必须回答它们。

## 一个贯穿全手册的最小例子

最小但有用的 PTO 故事，其实就是一个 tile 程序：

```cpp
TileT a(rows, cols), b(rows, cols), c(rows, cols);
GT ga(src0), gb(src1), gc(dst);

TLOAD(a, ga);
TLOAD(b, gb);
TADD(c, a, b);
TSTORE(gc, c);
```

这个例子很朴素，正因为朴素，它适合拿来解释 PTO 的核心。

- 计算的基本单位是 tile，而不是单个标量 lane
- 结果只在 tile 的 valid region 内有定义
- 指令合法性取决于 dtype、shape、layout 和 tile role
- 当数据流不够表达顺序时，同步必须显式表达

后面的章节，都是把这四件事慢慢展开。

## PTO 到底哪里不同

### Tile-first 语义

PTO 把大多数指令语义定义在 tile 域上。这不是“向量指令的语法糖”，而是说架构真的关心 tile shape、有效行列和 location intent，它们不是 backend lowering 才会关心的私货。

实际含义是：合法性问题会很早暴露。后端不能把一个非法 tile 组合偷偷解释成“差不多能跑”。它必须要么接受一个已文档化的组合，要么用确定性诊断拒绝。

### valid-region-first 行为

PTO 不假装一个矩形 tile 的每个元素都总是有意义。`Rv` 和 `Cv` 是架构的一部分，因为 edge tile、partial tile、padding tile 在真实 kernel 里非常常见。

这也是 PTO 比 generic 底层 ISA 更容易讲清楚的一点。它不是让每个 backend 自己发明边角约定，而是先把“语义在哪一块区域成立”讲清楚，再允许每条指令在必要时补充域外行为。

### location intent，而不是裸存储

`Mat`、`Left`、`Right`、`Acc`、`Bias`、`Scale` 这些 tile role 不是装饰性命名。它们告诉工具链和后端：这个 tile 准备用来干什么，并且它们真的参与合法性检查。

为什么不把所有 tile 都看成无类型字节块，让 backend 自己猜？因为那样会把真正的架构错误推迟到后端启发式阶段。PTO 选择相反的取舍：意图尽早可见、错误尽早暴露、支持子集由 profile 文档明确解释。

### Auto 和 Manual 都是 PTO 的正统写法

PTO 同时保留两种编程方式，不是因为设计摇摆，而是因为用户需求真的分成两类。Auto 模式服务于可移植性和默认生产力；Manual 模式服务于那些只有手工控制放置、同步和流水线复用才值得写的 kernel。

所以架构把两者都当作一等公民。Auto 不是“高层 PTO”，Manual 也不是“逃生口汇编”。它们只是同一套可见语义上的不同责任分配。

### 同步是架构问题

PTO 使用 event 和 `TSYNC`，是因为流水线边界和数据搬运边界本来就重要。架构不会把每个微架构细节都公开出来，但它要求程序员可见的顺序边必须在降层和执行后仍然可见。

## 架构边界

PTO 定义：

- valid region 内可观察的指令结果
- 用户和工具链都能检查的合法性边界
- 降层后必须保留的顺序与同步语义

PTO 不定义：

- 微架构调度细节
- 精确的片上存储布局
- 后端特定优化策略

后端差异可以存在，但必须老老实实写成 implementation-defined，而不是伪装成“架构默认如此”。

## 权威来源

本手册不替代仓库里的其他文档，它负责把这些来源拼成系统级契约：

- [PTO ISA 参考](../docs/isa/README_zh.md)：逐条指令语义
- `include/pto/common/pto_instr.hpp`：公共 API 形态与重载面
- [PTO-AS 规范](../docs/assembly/PTO-AS_zh.md) 与 `docs/assembly/PTO-AS.bnf`：文本汇编形态

## 兼容性原则

PTO 的演进方式应该是增加能力、补齐文档，而不是偷偷改变旧语义。落到工程上，就是下面三条：

- 增量演进 SHOULD 优先于破坏性变更
- 破坏性架构变更 MUST 携带显式版本信息与迁移说明
- implementation-defined 行为 MUST 在 manual、IR 契约和 backend profile 中保持一致标注
