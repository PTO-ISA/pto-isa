# Appendix D. Instruction Family Matrix

## D.1 Scope

This appendix is generated from `docs/isa/manifest.yaml` and provides a source-synchronized matrix of PTO virtual instruction families.

## D.2 Coverage summary

| Category | Instruction Count |
|---|---:|
| Synchronization | 1 |
| Manual / Resource Binding | 4 |
| Elementwise (Tile-Tile) | 28 |
| Tile-Scalar / Tile-Immediate | 19 |
| Axis Reduce / Expand | 22 |
| Memory (GM <-> Tile) | 6 |
| Matrix Multiply | 8 |
| Data Movement / Layout | 12 |
| Complex | 13 |
| Total | 113 |

## D.3 Header synchronization status

- Header inventory source: `include/pto/common/pto_instr.hpp` (113 unique instruction APIs)
- Manifest inventory source: `docs/isa/manifest.yaml` (113 entries)
- Missing in manifest: none
- Present in manifest but missing in header: none

## D.4 Family matrix

| Category | Instruction | Diagram Template | Operand Contract | Semantic Page |
|---|---|---|---|---|
| Synchronization | `TSYNC` | `sync` | `producer, consumer` | `docs/isa/TSYNC.md` |
| Manual / Resource Binding | `TASSIGN` | `config` | `config, state` | `docs/isa/TASSIGN.md` |
| Manual / Resource Binding | `TSETFMATRIX` | `config` | `config, state` | `docs/isa/TSETFMATRIX.md` |
| Elementwise (Tile-Tile) | `TADD` | `elementwise` | `dst, src0, src1` | `docs/isa/TADD.md` |
| Elementwise (Tile-Tile) | `TABS` | `elementwise` | `dst, src0, src1` | `docs/isa/TABS.md` |
| Elementwise (Tile-Tile) | `TAND` | `elementwise` | `dst, src0, src1` | `docs/isa/TAND.md` |
| Elementwise (Tile-Tile) | `TOR` | `elementwise` | `dst, src0, src1` | `docs/isa/TOR.md` |
| Elementwise (Tile-Tile) | `TSUB` | `elementwise` | `dst, src0, src1` | `docs/isa/TSUB.md` |
| Elementwise (Tile-Tile) | `TMUL` | `elementwise` | `dst, src0, src1` | `docs/isa/TMUL.md` |
| Elementwise (Tile-Tile) | `TMIN` | `elementwise` | `dst, src0, src1` | `docs/isa/TMIN.md` |
| Elementwise (Tile-Tile) | `TMAX` | `elementwise` | `dst, src0, src1` | `docs/isa/TMAX.md` |
| Elementwise (Tile-Tile) | `TCMP` | `elementwise` | `dst, src0, src1` | `docs/isa/TCMP.md` |
| Elementwise (Tile-Tile) | `TDIV` | `elementwise` | `dst, src0, src1` | `docs/isa/TDIV.md` |
| Elementwise (Tile-Tile) | `TSHL` | `elementwise` | `dst, src0, src1` | `docs/isa/TSHL.md` |
| Elementwise (Tile-Tile) | `TSHR` | `elementwise` | `dst, src0, src1` | `docs/isa/TSHR.md` |
| Elementwise (Tile-Tile) | `TXOR` | `elementwise` | `dst, src0, src1` | `docs/isa/TXOR.md` |
| Elementwise (Tile-Tile) | `TLOG` | `elementwise` | `dst, src0, src1` | `docs/isa/TLOG.md` |
| Elementwise (Tile-Tile) | `TRECIP` | `elementwise` | `dst, src0, src1` | `docs/isa/TRECIP.md` |
| Elementwise (Tile-Tile) | `TPRELU` | `elementwise` | `dst, src0, src1` | `docs/isa/TPRELU.md` |
| Elementwise (Tile-Tile) | `TADDC` | `elementwise` | `dst, src0, src1` | `docs/isa/TADDC.md` |
| Elementwise (Tile-Tile) | `TSUBC` | `elementwise` | `dst, src0, src1` | `docs/isa/TSUBC.md` |
| Elementwise (Tile-Tile) | `TCVT` | `elementwise` | `dst, src0, src1` | `docs/isa/TCVT.md` |
| Elementwise (Tile-Tile) | `TSEL` | `elementwise` | `dst, src0, src1` | `docs/isa/TSEL.md` |
| Elementwise (Tile-Tile) | `TRSQRT` | `elementwise` | `dst, src0, src1` | `docs/isa/TRSQRT.md` |
| Elementwise (Tile-Tile) | `TSQRT` | `elementwise` | `dst, src0, src1` | `docs/isa/TSQRT.md` |
| Elementwise (Tile-Tile) | `TEXP` | `elementwise` | `dst, src0, src1` | `docs/isa/TEXP.md` |
| Elementwise (Tile-Tile) | `TNOT` | `elementwise` | `dst, src0, src1` | `docs/isa/TNOT.md` |
| Elementwise (Tile-Tile) | `TRELU` | `elementwise` | `dst, src0, src1` | `docs/isa/TRELU.md` |
| Elementwise (Tile-Tile) | `TNEG` | `elementwise` | `dst, src0, src1` | `docs/isa/TNEG.md` |
| Elementwise (Tile-Tile) | `TREM` | `elementwise` | `dst, src0, src1` | `docs/isa/TREM.md` |
| Elementwise (Tile-Tile) | `TFMOD` | `elementwise` | `dst, src0, src1` | `docs/isa/TFMOD.md` |
| Tile-Scalar / Tile-Immediate | `TEXPANDS` | `scalar` | `dst, src, scalar` | `docs/isa/TEXPANDS.md` |
| Tile-Scalar / Tile-Immediate | `TCMPS` | `scalar` | `dst, src, scalar` | `docs/isa/TCMPS.md` |
| Tile-Scalar / Tile-Immediate | `TSELS` | `scalar` | `dst, src, scalar` | `docs/isa/TSELS.md` |
| Tile-Scalar / Tile-Immediate | `TMINS` | `scalar` | `dst, src, scalar` | `docs/isa/TMINS.md` |
| Tile-Scalar / Tile-Immediate | `TADDS` | `scalar` | `dst, src, scalar` | `docs/isa/TADDS.md` |
| Tile-Scalar / Tile-Immediate | `TSUBS` | `scalar` | `dst, src, scalar` | `docs/isa/TSUBS.md` |
| Tile-Scalar / Tile-Immediate | `TDIVS` | `scalar` | `dst, src, scalar` | `docs/isa/TDIVS.md` |
| Tile-Scalar / Tile-Immediate | `TMULS` | `scalar` | `dst, src, scalar` | `docs/isa/TMULS.md` |
| Tile-Scalar / Tile-Immediate | `TFMODS` | `scalar` | `dst, src, scalar` | `docs/isa/TFMODS.md` |
| Tile-Scalar / Tile-Immediate | `TREMS` | `scalar` | `dst, src, scalar` | `docs/isa/TREMS.md` |
| Tile-Scalar / Tile-Immediate | `TMAXS` | `scalar` | `dst, src, scalar` | `docs/isa/TMAXS.md` |
| Tile-Scalar / Tile-Immediate | `TANDS` | `scalar` | `dst, src, scalar` | `docs/isa/TANDS.md` |
| Tile-Scalar / Tile-Immediate | `TORS` | `scalar` | `dst, src, scalar` | `docs/isa/TORS.md` |
| Tile-Scalar / Tile-Immediate | `TSHLS` | `scalar` | `dst, src, scalar` | `docs/isa/TSHLS.md` |
| Tile-Scalar / Tile-Immediate | `TSHRS` | `scalar` | `dst, src, scalar` | `docs/isa/TSHRS.md` |
| Tile-Scalar / Tile-Immediate | `TXORS` | `scalar` | `dst, src, scalar` | `docs/isa/TXORS.md` |
| Tile-Scalar / Tile-Immediate | `TLRELU` | `scalar` | `dst, src, scalar` | `docs/isa/TLRELU.md` |
| Tile-Scalar / Tile-Immediate | `TADDSC` | `scalar` | `dst, src, scalar` | `docs/isa/TADDSC.md` |
| Tile-Scalar / Tile-Immediate | `TSUBSC` | `scalar` | `dst, src, scalar` | `docs/isa/TSUBSC.md` |
| Axis Reduce / Expand | `TROWSUM` | `reduce_expand` | `dst, src` | `docs/isa/TROWSUM.md` |
| Axis Reduce / Expand | `TCOLSUM` | `reduce_expand` | `dst, src` | `docs/isa/TCOLSUM.md` |
| Axis Reduce / Expand | `TCOLMAX` | `reduce_expand` | `dst, src` | `docs/isa/TCOLMAX.md` |
| Axis Reduce / Expand | `TROWMAX` | `reduce_expand` | `dst, src` | `docs/isa/TROWMAX.md` |
| Axis Reduce / Expand | `TROWMIN` | `reduce_expand` | `dst, src` | `docs/isa/TROWMIN.md` |
| Axis Reduce / Expand | `TROWEXPAND` | `reduce_expand` | `dst, src` | `docs/isa/TROWEXPAND.md` |
| Axis Reduce / Expand | `TROWEXPANDDIV` | `reduce_expand` | `dst, src` | `docs/isa/TROWEXPANDDIV.md` |
| Axis Reduce / Expand | `TROWEXPANDMUL` | `reduce_expand` | `dst, src` | `docs/isa/TROWEXPANDMUL.md` |
| Axis Reduce / Expand | `TROWEXPANDSUB` | `reduce_expand` | `dst, src` | `docs/isa/TROWEXPANDSUB.md` |
| Axis Reduce / Expand | `TROWEXPANDADD` | `reduce_expand` | `dst, src` | `docs/isa/TROWEXPANDADD.md` |
| Axis Reduce / Expand | `TROWEXPANDMAX` | `reduce_expand` | `dst, src` | `docs/isa/TROWEXPANDMAX.md` |
| Axis Reduce / Expand | `TROWEXPANDMIN` | `reduce_expand` | `dst, src` | `docs/isa/TROWEXPANDMIN.md` |
| Axis Reduce / Expand | `TROWEXPANDEXPDIF` | `reduce_expand` | `dst, src` | `docs/isa/TROWEXPANDEXPDIF.md` |
| Axis Reduce / Expand | `TCOLMIN` | `reduce_expand` | `dst, src` | `docs/isa/TCOLMIN.md` |
| Axis Reduce / Expand | `TCOLEXPAND` | `reduce_expand` | `dst, src` | `docs/isa/TCOLEXPAND.md` |
| Axis Reduce / Expand | `TCOLEXPANDDIV` | `reduce_expand` | `dst, src` | `docs/isa/TCOLEXPANDDIV.md` |
| Axis Reduce / Expand | `TCOLEXPANDMUL` | `reduce_expand` | `dst, src` | `docs/isa/TCOLEXPANDMUL.md` |
| Axis Reduce / Expand | `TCOLEXPANDADD` | `reduce_expand` | `dst, src` | `docs/isa/TCOLEXPANDADD.md` |
| Axis Reduce / Expand | `TCOLEXPANDMAX` | `reduce_expand` | `dst, src` | `docs/isa/TCOLEXPANDMAX.md` |
| Axis Reduce / Expand | `TCOLEXPANDMIN` | `reduce_expand` | `dst, src` | `docs/isa/TCOLEXPANDMIN.md` |
| Axis Reduce / Expand | `TCOLEXPANDSUB` | `reduce_expand` | `dst, src` | `docs/isa/TCOLEXPANDSUB.md` |
| Axis Reduce / Expand | `TCOLEXPANDEXPDIF` | `reduce_expand` | `dst, src` | `docs/isa/TCOLEXPANDEXPDIF.md` |
| Memory (GM <-> Tile) | `TLOAD` | `memory` | `tile, global` | `docs/isa/TLOAD.md` |
| Memory (GM <-> Tile) | `TPREFETCH` | `memory` | `tile, global` | `docs/isa/TPREFETCH.md` |
| Memory (GM <-> Tile) | `TSTORE` | `memory` | `tile, global` | `docs/isa/TSTORE.md` |
| Memory (GM <-> Tile) | `TSTORE_FP` | `memory` | `tile, global` | `docs/isa/TSTORE_FP.md` |
| Memory (GM <-> Tile) | `MGATHER` | `memory` | `tile, global` | `docs/isa/MGATHER.md` |
| Memory (GM <-> Tile) | `MSCATTER` | `memory` | `tile, global` | `docs/isa/MSCATTER.md` |
| Matrix Multiply | `TGEMV_MX` | `matmul` | `dst, lhs, rhs` | `docs/isa/TGEMV_MX.md` |
| Matrix Multiply | `TMATMUL_MX` | `matmul` | `dst, lhs, rhs` | `docs/isa/TMATMUL_MX.md` |
| Matrix Multiply | `TMATMUL` | `matmul` | `dst, lhs, rhs` | `docs/isa/TMATMUL.md` |
| Matrix Multiply | `TMATMUL_ACC` | `matmul` | `dst, lhs, rhs` | `docs/isa/TMATMUL_ACC.md` |
| Matrix Multiply | `TMATMUL_BIAS` | `matmul` | `dst, lhs, rhs` | `docs/isa/TMATMUL_BIAS.md` |
| Matrix Multiply | `TGEMV` | `matmul` | `dst, lhs, rhs` | `docs/isa/TGEMV.md` |
| Matrix Multiply | `TGEMV_ACC` | `matmul` | `dst, lhs, rhs` | `docs/isa/TGEMV_ACC.md` |
| Matrix Multiply | `TGEMV_BIAS` | `matmul` | `dst, lhs, rhs` | `docs/isa/TGEMV_BIAS.md` |
| Data Movement / Layout | `TEXTRACT` | `reshape_move` | `dst, src` | `docs/isa/TEXTRACT.md` |
| Data Movement / Layout | `TEXTRACT_FP` | `reshape_move` | `dst, src` | `docs/isa/TEXTRACT_FP.md` |
| Data Movement / Layout | `TIMG2COL` | `reshape_move` | `dst, src` | `docs/isa/TIMG2COL.md` |
| Data Movement / Layout | `TINSERT` | `reshape_move` | `dst, src` | `docs/isa/TINSERT.md` |
| Data Movement / Layout | `TINSERT_FP` | `reshape_move` | `dst, src` | `docs/isa/TINSERT_FP.md` |
| Data Movement / Layout | `TFILLPAD` | `reshape_move` | `dst, src` | `docs/isa/TFILLPAD.md` |
| Data Movement / Layout | `TFILLPAD_INPLACE` | `reshape_move` | `dst, src` | `docs/isa/TFILLPAD_INPLACE.md` |
| Data Movement / Layout | `TFILLPAD_EXPAND` | `reshape_move` | `dst, src` | `docs/isa/TFILLPAD_EXPAND.md` |
| Data Movement / Layout | `TMOV` | `reshape_move` | `dst, src` | `docs/isa/TMOV.md` |
| Data Movement / Layout | `TMOV_FP` | `reshape_move` | `dst, src` | `docs/isa/TMOV_FP.md` |
| Data Movement / Layout | `TRESHAPE` | `reshape_move` | `dst, src` | `docs/isa/TRESHAPE.md` |
| Data Movement / Layout | `TTRANS` | `reshape_move` | `dst, src` | `docs/isa/TTRANS.md` |
| Complex | `TPRINT` | `complex` | `dst, src0, src1` | `docs/isa/TPRINT.md` |
| Complex | `TMRGSORT` | `complex` | `dst, src0, src1` | `docs/isa/TMRGSORT.md` |
| Complex | `TSORT32` | `complex` | `dst, src0, src1` | `docs/isa/TSORT32.md` |
| Complex | `TGATHER` | `complex` | `dst, src0, src1` | `docs/isa/TGATHER.md` |
| Complex | `TCI` | `complex` | `dst, src0, src1` | `docs/isa/TCI.md` |
| Complex | `TTRI` | `complex` | `dst, src0, src1` | `docs/isa/TTRI.md` |
| Complex | `TPARTADD` | `complex` | `dst, src0, src1` | `docs/isa/TPARTADD.md` |
| Complex | `TPARTMUL` | `complex` | `dst, src0, src1` | `docs/isa/TPARTMUL.md` |
| Complex | `TPARTMAX` | `complex` | `dst, src0, src1` | `docs/isa/TPARTMAX.md` |
| Complex | `TPARTMIN` | `complex` | `dst, src0, src1` | `docs/isa/TPARTMIN.md` |
| Complex | `TGATHERB` | `complex` | `dst, src0, src1` | `docs/isa/TGATHERB.md` |
| Complex | `TSCATTER` | `complex` | `dst, src0, src1` | `docs/isa/TSCATTER.md` |
| Complex | `TQUANT` | `complex` | `dst, src0, src1` | `docs/isa/TQUANT.md` |

## D.5 Notes

- Per-instruction semantics remain canonical in `docs/isa/*.md`.
- This appendix is a taxonomy and coverage matrix, not a replacement for per-op normative semantics.
