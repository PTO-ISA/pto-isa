# tests/

PTO Tile Lib 的测试与示例，覆盖 CPU 仿真与 NPU（`sim` 和板上 `npu` 两种模式）。

## 目录结构

- `script/`：推荐的入口脚本
  - `run_st.py`：构建并运行 NPU ST（`-r sim|npu -v a3|a5 -t <testcase> -g <gtest_filter>`）
  - `build_st.py`：仅构建 NPU ST
  - `all_cpu_tests.py`：批量构建并运行 CPU ST 套件
  - `README.md`：脚本使用说明
- `cpu/`：CPU 侧 ST 测试（gtest + CMake）
  - `cpu/st/`：CPU ST 工程与 testcase 数据生成脚本
- `npu/`：按 SoC 拆分的 NPU 侧 ST 测试
  - `npu/a2a3/src/st/`：A2/A3 计算 ST
  - `npu/a2a3/comm/st/`：A2/A3 通信 ST
  - `npu/a5/src/st/`：A5 计算 ST
  - `npu/a5/comm/st/`：A5 通信 ST
- `common/`：共享测试资源（如存在）
- `run_comm_test.sh`：通信 ST 一键运行脚本（详见下方说明）

## 通信测试（Comm ST）

通信测试验证多卡间的 PTO 通信原语（Put / Get / Broadcast / Gather / Scatter / Reduce / Notify / Wait / Test），基于 MPI + HCCL 实现。

### 前置依赖：MPI 安装

通信测试需要 MPI 环境（MPICH 或 OpenMPI 均可）。运行时需要两个组件：

1. **`mpirun`**：用于启动多进程
2. **`libmpi.so`**：运行时通过 `dlopen` 动态加载

#### 安装 MPICH（推荐）

```bash
# Ubuntu / Debian
sudo apt install mpich libmpich-dev

# CentOS / RHEL / EulerOS
sudo yum install mpich mpich-devel
# 安装后可能需要加载 module 或手动加入 PATH：
export PATH=/usr/lib64/mpich/bin:$PATH
```

#### 从源码安装 MPICH（无 root 权限时）

```bash
wget https://www.mpich.org/static/downloads/4.2.3/mpich-4.2.3.tar.gz
tar xzf mpich-4.2.3.tar.gz && cd mpich-4.2.3
./configure --prefix=$HOME/mpich --disable-fortran
make -j$(nproc) && make install
export MPI_HOME=$HOME/mpich
export PATH=$MPI_HOME/bin:$PATH
```

#### 环境变量

| 变量 | 说明 |
|------|------|
| `MPI_HOME` | MPI 安装根目录，脚本会自动搜索 `$MPI_HOME/bin/mpirun` |
| `MPI_LIB_PATH` | 直接指定 `libmpi.so` 路径（覆盖默认搜索） |

如果 `mpirun` 已在 `PATH` 中且 `libmpi.so` 在标准库路径下，则无需设置这些变量。

#### 验证安装

```bash
mpirun --version
mpirun -n 2 echo "MPI OK"
```

### 快速开始

```bash
# 8 卡全量测试（默认 A2/A3）
./run_comm_test.sh

# 指定 A5 SoC，2 卡
./run_comm_test.sh -v a5 -n 2

# 仅跑 tput 用例
./run_comm_test.sh -t tput

# 开启 debug 日志
./run_comm_test.sh -d -t tput
```

### 参数说明

| 参数 | 说明 | 默认值 |
|------|------|--------|
| `-n` | 可用 NPU 数量：2、4 或 8 | 8 |
| `-v` | SoC 版本：`a3`（Ascend910B）或 `a5`（Ascend910_9599） | a3 |
| `-t` | 指定测试用例（可多次使用），如 `tput`、`treduce` | 全部 |
| `-d` | 开启调试模式，打印详细初始化与同步日志 | 关闭 |

### 运行机制

脚本会根据 `-n` 指定的卡数，自动为每个测试用例分别以 2 / 4 / 8 rank 运行，通过 GTest Filter 确保每次只执行与当前 rank 数匹配的测试。例如 `-n 4` 时会先以 2 rank 跑默认用例，再以 4 rank 跑带 `4Ranks` 后缀的用例，跳过 8 rank 用例。

## 建议阅读顺序

- 入门指南（建议先 CPU，再 NPU）：[docs/getting-started.md](../docs/getting-started.md)
