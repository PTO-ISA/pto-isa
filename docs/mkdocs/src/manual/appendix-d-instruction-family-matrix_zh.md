# 附录 D. 指令族矩阵

## D.1 范围

本附录由 `docs/isa/manifest.yaml` 自动生成，用于给出 PTO 虚拟指令族的源同步矩阵。

## D.2 覆盖统计

| 分类 | 指令数量 |
|---|---:|
| 同步 | 1 |
| 手动 / 资源绑定 | 4 |
| 逐元素（Tile-Tile） | 28 |
| Tile-标量 / Tile-立即数 | 19 |
| 轴归约 / 扩展 | 22 |
| 内存（GM <-> Tile） | 6 |
| 矩阵乘 | 8 |
| 数据搬运 / 布局 | 12 |
| 复杂指令 | 13 |
| 总计 | 113 |

## D.3 头文件同步状态

- 头文件清单来源：`include/pto/common/pto_instr.hpp`（113 个唯一指令 API）
- Manifest 清单来源：`docs/isa/manifest.yaml`（113 条目）
- 头文件有但 manifest 缺失：无
- manifest 有但头文件缺失：无

## D.4 指令族矩阵

| 分类 | 指令 | 图示模板 | 操作数契约 | 语义页面 |
|---|---|---|---|---|
| 同步 | `TSYNC` | `sync` | `producer, consumer` | `docs/isa/TSYNC_zh.md` |
| 手动 / 资源绑定 | `TASSIGN` | `config` | `config, state` | `docs/isa/TASSIGN_zh.md` |
| 手动 / 资源绑定 | `TSETFMATRIX` | `config` | `config, state` | `docs/isa/TSETFMATRIX_zh.md` |
| 逐元素（Tile-Tile） | `TADD` | `elementwise` | `dst, src0, src1` | `docs/isa/TADD_zh.md` |
| 逐元素（Tile-Tile） | `TABS` | `elementwise` | `dst, src0, src1` | `docs/isa/TABS_zh.md` |
| 逐元素（Tile-Tile） | `TAND` | `elementwise` | `dst, src0, src1` | `docs/isa/TAND_zh.md` |
| 逐元素（Tile-Tile） | `TOR` | `elementwise` | `dst, src0, src1` | `docs/isa/TOR_zh.md` |
| 逐元素（Tile-Tile） | `TSUB` | `elementwise` | `dst, src0, src1` | `docs/isa/TSUB_zh.md` |
| 逐元素（Tile-Tile） | `TMUL` | `elementwise` | `dst, src0, src1` | `docs/isa/TMUL_zh.md` |
| 逐元素（Tile-Tile） | `TMIN` | `elementwise` | `dst, src0, src1` | `docs/isa/TMIN_zh.md` |
| 逐元素（Tile-Tile） | `TMAX` | `elementwise` | `dst, src0, src1` | `docs/isa/TMAX_zh.md` |
| 逐元素（Tile-Tile） | `TCMP` | `elementwise` | `dst, src0, src1` | `docs/isa/TCMP_zh.md` |
| 逐元素（Tile-Tile） | `TDIV` | `elementwise` | `dst, src0, src1` | `docs/isa/TDIV_zh.md` |
| 逐元素（Tile-Tile） | `TSHL` | `elementwise` | `dst, src0, src1` | `docs/isa/TSHL_zh.md` |
| 逐元素（Tile-Tile） | `TSHR` | `elementwise` | `dst, src0, src1` | `docs/isa/TSHR_zh.md` |
| 逐元素（Tile-Tile） | `TXOR` | `elementwise` | `dst, src0, src1` | `docs/isa/TXOR_zh.md` |
| 逐元素（Tile-Tile） | `TLOG` | `elementwise` | `dst, src0, src1` | `docs/isa/TLOG_zh.md` |
| 逐元素（Tile-Tile） | `TRECIP` | `elementwise` | `dst, src0, src1` | `docs/isa/TRECIP_zh.md` |
| 逐元素（Tile-Tile） | `TPRELU` | `elementwise` | `dst, src0, src1` | `docs/isa/TPRELU_zh.md` |
| 逐元素（Tile-Tile） | `TADDC` | `elementwise` | `dst, src0, src1` | `docs/isa/TADDC_zh.md` |
| 逐元素（Tile-Tile） | `TSUBC` | `elementwise` | `dst, src0, src1` | `docs/isa/TSUBC_zh.md` |
| 逐元素（Tile-Tile） | `TCVT` | `elementwise` | `dst, src0, src1` | `docs/isa/TCVT_zh.md` |
| 逐元素（Tile-Tile） | `TSEL` | `elementwise` | `dst, src0, src1` | `docs/isa/TSEL_zh.md` |
| 逐元素（Tile-Tile） | `TRSQRT` | `elementwise` | `dst, src0, src1` | `docs/isa/TRSQRT_zh.md` |
| 逐元素（Tile-Tile） | `TSQRT` | `elementwise` | `dst, src0, src1` | `docs/isa/TSQRT_zh.md` |
| 逐元素（Tile-Tile） | `TEXP` | `elementwise` | `dst, src0, src1` | `docs/isa/TEXP_zh.md` |
| 逐元素（Tile-Tile） | `TNOT` | `elementwise` | `dst, src0, src1` | `docs/isa/TNOT_zh.md` |
| 逐元素（Tile-Tile） | `TRELU` | `elementwise` | `dst, src0, src1` | `docs/isa/TRELU_zh.md` |
| 逐元素（Tile-Tile） | `TNEG` | `elementwise` | `dst, src0, src1` | `docs/isa/TNEG_zh.md` |
| 逐元素（Tile-Tile） | `TREM` | `elementwise` | `dst, src0, src1` | `docs/isa/TREM_zh.md` |
| 逐元素（Tile-Tile） | `TFMOD` | `elementwise` | `dst, src0, src1` | `docs/isa/TFMOD_zh.md` |
| Tile-标量 / Tile-立即数 | `TEXPANDS` | `scalar` | `dst, src, scalar` | `docs/isa/TEXPANDS_zh.md` |
| Tile-标量 / Tile-立即数 | `TCMPS` | `scalar` | `dst, src, scalar` | `docs/isa/TCMPS_zh.md` |
| Tile-标量 / Tile-立即数 | `TSELS` | `scalar` | `dst, src, scalar` | `docs/isa/TSELS_zh.md` |
| Tile-标量 / Tile-立即数 | `TMINS` | `scalar` | `dst, src, scalar` | `docs/isa/TMINS_zh.md` |
| Tile-标量 / Tile-立即数 | `TADDS` | `scalar` | `dst, src, scalar` | `docs/isa/TADDS_zh.md` |
| Tile-标量 / Tile-立即数 | `TSUBS` | `scalar` | `dst, src, scalar` | `docs/isa/TSUBS_zh.md` |
| Tile-标量 / Tile-立即数 | `TDIVS` | `scalar` | `dst, src, scalar` | `docs/isa/TDIVS_zh.md` |
| Tile-标量 / Tile-立即数 | `TMULS` | `scalar` | `dst, src, scalar` | `docs/isa/TMULS_zh.md` |
| Tile-标量 / Tile-立即数 | `TFMODS` | `scalar` | `dst, src, scalar` | `docs/isa/TFMODS_zh.md` |
| Tile-标量 / Tile-立即数 | `TREMS` | `scalar` | `dst, src, scalar` | `docs/isa/TREMS_zh.md` |
| Tile-标量 / Tile-立即数 | `TMAXS` | `scalar` | `dst, src, scalar` | `docs/isa/TMAXS_zh.md` |
| Tile-标量 / Tile-立即数 | `TANDS` | `scalar` | `dst, src, scalar` | `docs/isa/TANDS_zh.md` |
| Tile-标量 / Tile-立即数 | `TORS` | `scalar` | `dst, src, scalar` | `docs/isa/TORS_zh.md` |
| Tile-标量 / Tile-立即数 | `TSHLS` | `scalar` | `dst, src, scalar` | `docs/isa/TSHLS_zh.md` |
| Tile-标量 / Tile-立即数 | `TSHRS` | `scalar` | `dst, src, scalar` | `docs/isa/TSHRS_zh.md` |
| Tile-标量 / Tile-立即数 | `TXORS` | `scalar` | `dst, src, scalar` | `docs/isa/TXORS_zh.md` |
| Tile-标量 / Tile-立即数 | `TLRELU` | `scalar` | `dst, src, scalar` | `docs/isa/TLRELU_zh.md` |
| Tile-标量 / Tile-立即数 | `TADDSC` | `scalar` | `dst, src, scalar` | `docs/isa/TADDSC_zh.md` |
| Tile-标量 / Tile-立即数 | `TSUBSC` | `scalar` | `dst, src, scalar` | `docs/isa/TSUBSC_zh.md` |
| 轴归约 / 扩展 | `TROWSUM` | `reduce_expand` | `dst, src` | `docs/isa/TROWSUM_zh.md` |
| 轴归约 / 扩展 | `TCOLSUM` | `reduce_expand` | `dst, src` | `docs/isa/TCOLSUM_zh.md` |
| 轴归约 / 扩展 | `TCOLMAX` | `reduce_expand` | `dst, src` | `docs/isa/TCOLMAX_zh.md` |
| 轴归约 / 扩展 | `TROWMAX` | `reduce_expand` | `dst, src` | `docs/isa/TROWMAX_zh.md` |
| 轴归约 / 扩展 | `TROWMIN` | `reduce_expand` | `dst, src` | `docs/isa/TROWMIN_zh.md` |
| 轴归约 / 扩展 | `TROWEXPAND` | `reduce_expand` | `dst, src` | `docs/isa/TROWEXPAND_zh.md` |
| 轴归约 / 扩展 | `TROWEXPANDDIV` | `reduce_expand` | `dst, src` | `docs/isa/TROWEXPANDDIV_zh.md` |
| 轴归约 / 扩展 | `TROWEXPANDMUL` | `reduce_expand` | `dst, src` | `docs/isa/TROWEXPANDMUL_zh.md` |
| 轴归约 / 扩展 | `TROWEXPANDSUB` | `reduce_expand` | `dst, src` | `docs/isa/TROWEXPANDSUB_zh.md` |
| 轴归约 / 扩展 | `TROWEXPANDADD` | `reduce_expand` | `dst, src` | `docs/isa/TROWEXPANDADD_zh.md` |
| 轴归约 / 扩展 | `TROWEXPANDMAX` | `reduce_expand` | `dst, src` | `docs/isa/TROWEXPANDMAX_zh.md` |
| 轴归约 / 扩展 | `TROWEXPANDMIN` | `reduce_expand` | `dst, src` | `docs/isa/TROWEXPANDMIN_zh.md` |
| 轴归约 / 扩展 | `TROWEXPANDEXPDIF` | `reduce_expand` | `dst, src` | `docs/isa/TROWEXPANDEXPDIF_zh.md` |
| 轴归约 / 扩展 | `TCOLMIN` | `reduce_expand` | `dst, src` | `docs/isa/TCOLMIN_zh.md` |
| 轴归约 / 扩展 | `TCOLEXPAND` | `reduce_expand` | `dst, src` | `docs/isa/TCOLEXPAND_zh.md` |
| 轴归约 / 扩展 | `TCOLEXPANDDIV` | `reduce_expand` | `dst, src` | `docs/isa/TCOLEXPANDDIV_zh.md` |
| 轴归约 / 扩展 | `TCOLEXPANDMUL` | `reduce_expand` | `dst, src` | `docs/isa/TCOLEXPANDMUL_zh.md` |
| 轴归约 / 扩展 | `TCOLEXPANDADD` | `reduce_expand` | `dst, src` | `docs/isa/TCOLEXPANDADD_zh.md` |
| 轴归约 / 扩展 | `TCOLEXPANDMAX` | `reduce_expand` | `dst, src` | `docs/isa/TCOLEXPANDMAX_zh.md` |
| 轴归约 / 扩展 | `TCOLEXPANDMIN` | `reduce_expand` | `dst, src` | `docs/isa/TCOLEXPANDMIN_zh.md` |
| 轴归约 / 扩展 | `TCOLEXPANDSUB` | `reduce_expand` | `dst, src` | `docs/isa/TCOLEXPANDSUB_zh.md` |
| 轴归约 / 扩展 | `TCOLEXPANDEXPDIF` | `reduce_expand` | `dst, src` | `docs/isa/TCOLEXPANDEXPDIF_zh.md` |
| 内存（GM <-> Tile） | `TLOAD` | `memory` | `tile, global` | `docs/isa/TLOAD_zh.md` |
| 内存（GM <-> Tile） | `TPREFETCH` | `memory` | `tile, global` | `docs/isa/TPREFETCH_zh.md` |
| 内存（GM <-> Tile） | `TSTORE` | `memory` | `tile, global` | `docs/isa/TSTORE_zh.md` |
| 内存（GM <-> Tile） | `TSTORE_FP` | `memory` | `tile, global` | `docs/isa/TSTORE_FP_zh.md` |
| 内存（GM <-> Tile） | `MGATHER` | `memory` | `tile, global` | `docs/isa/MGATHER_zh.md` |
| 内存（GM <-> Tile） | `MSCATTER` | `memory` | `tile, global` | `docs/isa/MSCATTER_zh.md` |
| 矩阵乘 | `TGEMV_MX` | `matmul` | `dst, lhs, rhs` | `docs/isa/TGEMV_MX_zh.md` |
| 矩阵乘 | `TMATMUL_MX` | `matmul` | `dst, lhs, rhs` | `docs/isa/TMATMUL_MX_zh.md` |
| 矩阵乘 | `TMATMUL` | `matmul` | `dst, lhs, rhs` | `docs/isa/TMATMUL_zh.md` |
| 矩阵乘 | `TMATMUL_ACC` | `matmul` | `dst, lhs, rhs` | `docs/isa/TMATMUL_ACC_zh.md` |
| 矩阵乘 | `TMATMUL_BIAS` | `matmul` | `dst, lhs, rhs` | `docs/isa/TMATMUL_BIAS_zh.md` |
| 矩阵乘 | `TGEMV` | `matmul` | `dst, lhs, rhs` | `docs/isa/TGEMV_zh.md` |
| 矩阵乘 | `TGEMV_ACC` | `matmul` | `dst, lhs, rhs` | `docs/isa/TGEMV_ACC_zh.md` |
| 矩阵乘 | `TGEMV_BIAS` | `matmul` | `dst, lhs, rhs` | `docs/isa/TGEMV_BIAS_zh.md` |
| 数据搬运 / 布局 | `TEXTRACT` | `reshape_move` | `dst, src` | `docs/isa/TEXTRACT_zh.md` |
| 数据搬运 / 布局 | `TEXTRACT_FP` | `reshape_move` | `dst, src` | `docs/isa/TEXTRACT_FP_zh.md` |
| 数据搬运 / 布局 | `TIMG2COL` | `reshape_move` | `dst, src` | `docs/isa/TIMG2COL_zh.md` |
| 数据搬运 / 布局 | `TINSERT` | `reshape_move` | `dst, src` | `docs/isa/TINSERT_zh.md` |
| 数据搬运 / 布局 | `TINSERT_FP` | `reshape_move` | `dst, src` | `docs/isa/TINSERT_FP_zh.md` |
| 数据搬运 / 布局 | `TFILLPAD` | `reshape_move` | `dst, src` | `docs/isa/TFILLPAD_zh.md` |
| 数据搬运 / 布局 | `TFILLPAD_INPLACE` | `reshape_move` | `dst, src` | `docs/isa/TFILLPAD_INPLACE_zh.md` |
| 数据搬运 / 布局 | `TFILLPAD_EXPAND` | `reshape_move` | `dst, src` | `docs/isa/TFILLPAD_EXPAND_zh.md` |
| 数据搬运 / 布局 | `TMOV` | `reshape_move` | `dst, src` | `docs/isa/TMOV_zh.md` |
| 数据搬运 / 布局 | `TMOV_FP` | `reshape_move` | `dst, src` | `docs/isa/TMOV_FP_zh.md` |
| 数据搬运 / 布局 | `TRESHAPE` | `reshape_move` | `dst, src` | `docs/isa/TRESHAPE_zh.md` |
| 数据搬运 / 布局 | `TTRANS` | `reshape_move` | `dst, src` | `docs/isa/TTRANS_zh.md` |
| 复杂指令 | `TPRINT` | `complex` | `dst, src0, src1` | `docs/isa/TPRINT_zh.md` |
| 复杂指令 | `TMRGSORT` | `complex` | `dst, src0, src1` | `docs/isa/TMRGSORT_zh.md` |
| 复杂指令 | `TSORT32` | `complex` | `dst, src0, src1` | `docs/isa/TSORT32_zh.md` |
| 复杂指令 | `TGATHER` | `complex` | `dst, src0, src1` | `docs/isa/TGATHER_zh.md` |
| 复杂指令 | `TCI` | `complex` | `dst, src0, src1` | `docs/isa/TCI_zh.md` |
| 复杂指令 | `TTRI` | `complex` | `dst, src0, src1` | `docs/isa/TTRI_zh.md` |
| 复杂指令 | `TPARTADD` | `complex` | `dst, src0, src1` | `docs/isa/TPARTADD_zh.md` |
| 复杂指令 | `TPARTMUL` | `complex` | `dst, src0, src1` | `docs/isa/TPARTMUL_zh.md` |
| 复杂指令 | `TPARTMAX` | `complex` | `dst, src0, src1` | `docs/isa/TPARTMAX_zh.md` |
| 复杂指令 | `TPARTMIN` | `complex` | `dst, src0, src1` | `docs/isa/TPARTMIN_zh.md` |
| 复杂指令 | `TGATHERB` | `complex` | `dst, src0, src1` | `docs/isa/TGATHERB_zh.md` |
| 复杂指令 | `TSCATTER` | `complex` | `dst, src0, src1` | `docs/isa/TSCATTER_zh.md` |
| 复杂指令 | `TQUANT` | `complex` | `dst, src0, src1` | `docs/isa/TQUANT_zh.md` |

## D.5 说明

- 逐条指令语义仍以 `docs/isa/*_zh.md` 为准。
- 本附录用于分类与覆盖追踪，不替代逐条指令的规范化语义描述。
