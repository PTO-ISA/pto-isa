# include/

Public C/C++ headers for PTO Tile Lib (primarily header-only, template-based). Upper-layer frameworks or operator code can include these headers to emit PTO ISA Tile-level operations.

## Quick Start

Include the unified entry header:

```cpp
#include <pto/pto-inst.hpp>
```

`pto/pto-inst.hpp` selects the appropriate backend (CPU simulation/stub or NPU implementation) based on build configuration. See [include/pto/README.md](pto/README.md) for details.

## Layout

- `include/pto/`: Public PTO ISA API and backend implementations (common / cpu / npu)

## Related Docs

- [ISA guide](../docs/README.md)
- [Getting started](../docs/getting-started.md)

## PTO Instruction Implementation Status (CPU / Costmodel / A2 / A3 / A5)

This table tracks per-instruction backend availability:

- **CPU**: `__CPU_SIM` (CPU simulation backend).
- **Costmodel**: `__COSTMODEL` (A2 / A3 cost model backend).
- **A2 (Ascend 910B) / A3 (Ascend 910C)**: share the `include/pto/npu/a2a3/` implementation today (so the status is identical for both columns).
- **A5 (Ascend 950)**: uses the `include/pto/npu/a5/` implementation.
- **TODO** means the instruction is part of the public API but the backend implementation is not available yet.

| Instruction | CPU | Costmodel| A2 | A3 | A5 |
|---|---:|---:|---:|---:|---:|
| [`MGATHER`](../docs/isa/MGATHER.md) | Yes | TODO | TODO | TODO | TODO |
| [`MSCATTER`](../docs/isa/MSCATTER.md) | Yes | TODO | TODO | TODO | TODO |
| [`TABS`](../docs/isa/TABS.md) | Yes | Yes | Yes | Yes | Yes |
| [`TADD`](../docs/isa/TADD.md) | Yes | Yes | Yes | Yes | Yes |
| [`TADDC`](../docs/isa/TADDC.md) | Yes | TODO | TODO | TODO | TODO |
| [`TADDS`](../docs/isa/TADDS.md) | Yes | Yes | Yes | Yes | Yes |
| [`TADDSC`](../docs/isa/TADDSC.md) | Yes | TODO | TODO | TODO | TODO |
| [`TAND`](../docs/isa/TAND.md) | Yes | TODO | Yes | Yes | Yes |
| [`TANDS`](../docs/isa/TANDS.md) | Yes | TODO | Yes | Yes | Yes |
| [`TASSIGN`](../docs/isa/TASSIGN.md) | Yes | TODO | Yes | Yes | Yes |
| [`TAXPY`]() | TODO | TODO | Yes | Yes | Yes |
| [`TCI`](../docs/isa/TCI.md) | Yes | TODO | Yes | Yes | Yes |
| [`TCMP`](../docs/isa/TCMP.md) | Yes | TODO | Yes | Yes | Yes |
| [`TCMPS`](../docs/isa/TCMPS.md) | Yes | TODO | Yes | Yes | Yes |
| [`TCOLEXPAND`](../docs/isa/TCOLEXPAND.md) | Yes | TODO | TODO | TODO | TODO |
| [`TCOLEXPANDADD`]() | TODO | TODO | Yes | Yes | Yes |
| [`TCOLEXPANDDIV`]() | TODO | TODO | Yes | Yes | Yes |
| [`TCOLEXPANDEXPDIF`]() | TODO | TODO | Yes | Yes | Yes |
| [`TCOLEXPANDMAX`]() | TODO | TODO | Yes | Yes | Yes |
| [`TCOLEXPANDMIN`]() | TODO | TODO | Yes | Yes | Yes |
| [`TCOLEXPANDMUL`]() | TODO | TODO | Yes | Yes | Yes |
| [`TCOLEXPANDSUB`]() | TODO | TODO | Yes | Yes | Yes |
| [`TCOLMAX`](../docs/isa/TCOLMAX.md) | Yes | TODO | Yes | Yes | Yes |
| [`TCOLMIN`](../docs/isa/TCOLMIN.md) | Yes | TODO | Yes | Yes | Yes |
| [`TCOLSUM`](../docs/isa/TCOLSUM.md) | Yes | TODO | Yes | Yes | Yes |
| [`TCOLPROD`](../docs/isa/TCOLPROD.md) | TODO | TODO | Yes | Yes | Yes |
| [`TCVT`](../docs/isa/TCVT.md) | Yes | TODO | Yes | Yes | Yes |
| [`TDIV`](../docs/isa/TDIV.md) | Yes | TODO | Yes | Yes | Yes |
| [`TDIVS`](../docs/isa/TDIVS.md) | Yes | Yes | Yes | Yes | Yes |
| [`TEXP`](../docs/isa/TEXP.md) | Yes | Yes | Yes | Yes | Yes |
| [`TEXPANDS`](../docs/isa/TEXPANDS.md) | Yes | TODO | Yes | Yes | Yes |
| [`TEXTRACT`](../docs/isa/TEXTRACT.md) | Yes | TODO | Yes | Yes | Yes |
| [`TFILLPAD`](../docs/isa/TFILLPAD.md) | Yes | TODO | Yes | Yes | Yes |
| [`TGATHER`](../docs/isa/TGATHER.md) | Yes | TODO | Yes | Yes | Yes |
| [`TGATHERB`](../docs/isa/TGATHERB.md) | Yes | TODO | Yes | Yes | Yes |
| [`TLOAD`](../docs/isa/TLOAD.md) | Yes | TODO | Yes | Yes | Yes |
| [`TLOG`](../docs/isa/TLOG.md) | Yes | TODO | Yes | Yes | Yes |
| [`TLRELU`](../docs/isa/TLRELU.md) | Yes | TODO | Yes | Yes | Yes |
| [`TMATMUL`](../docs/isa/TMATMUL.md) | Yes | TODO | Yes | Yes | Yes |
| [`TMATMUL_ACC`](../docs/isa/TMATMUL_ACC.md) | Yes | TODO | Yes | Yes | Yes |
| [`TMATMUL_BIAS`](../docs/isa/TMATMUL_BIAS.md) | Yes | TODO | Yes | Yes | Yes |
| [`TMATMUL_MX`](../docs/isa/TMATMUL_MX.md) | Yes | TODO | Yes | Yes | Yes |
| [`TMAX`](../docs/isa/TMAX.md) | Yes | TODO | Yes | Yes | Yes |
| [`TMAXS`](../docs/isa/TMAXS.md) | Yes | TODO | Yes | Yes | Yes |
| [`TMIN`](../docs/isa/TMIN.md) | Yes | TODO | Yes | Yes | Yes |
| [`TMINS`](../docs/isa/TMINS.md) | Yes | Yes | Yes | Yes | Yes |
| [`TMOV`](../docs/isa/TMOV.md) | Yes | TODO | Yes | Yes | Yes |
| [`TMOV_FP`](../docs/isa/TMOV_FP.md) | TODO | TODO | TODO | TODO | TODO |
| [`TMRGSORT`](../docs/isa/TMRGSORT.md) | Yes | TODO | Yes | Yes | Yes |
| [`TMUL`](../docs/isa/TMUL.md) | Yes | Yes | Yes | Yes | Yes |
| [`TMULS`](../docs/isa/TMULS.md) | Yes | Yes | Yes | Yes | Yes |
| [`TNEG`](../docs/isa/TNEG.md) | Yes | TODO | Yes | Yes | Yes |
| [`TNOT`](../docs/isa/TNOT.md) | Yes | TODO | Yes | Yes | Yes |
| [`TOR`](../docs/isa/TOR.md) | Yes | TODO | Yes | Yes | Yes |
| [`TORS`](../docs/isa/TORS.md) | Yes | TODO | Yes | Yes | Yes |
| [`TPARTADD`](../docs/isa/TPARTADD.md) | Yes | TODO | Yes | Yes | Yes |
| [`TPARTMAX`](../docs/isa/TPARTMAX.md) | Yes | TODO | Yes | Yes | Yes |
| [`TPARTMIN`](../docs/isa/TPARTMIN.md) | Yes | TODO | Yes | Yes | Yes |
| [`TPARTMUL`]() | TODO | TODO | Yes | Yes | Yes |
| [`TPRELU`](../docs/isa/TPRELU.md) | Yes | TODO | Yes | Yes | Yes |
| [`TPREFETCH`]() | TODO | TODO | Yes | Yes | Yes |
| [`TPRINT`]() | TODO | TODO | Yes | Yes | Yes |
| [`TRECIP`](../docs/isa/TRECIP.md) | Yes | TODO | Yes | Yes | Yes |
| [`TRELU`](../docs/isa/TRELU.md) | Yes | TODO | Yes | Yes | Yes |
| [`TREM`](../docs/isa/TREM.md) | Yes | TODO | Yes | Yes | Yes |
| [`TREMS`](../docs/isa/TREMS.md) | Yes | TODO | Yes | Yes | Yes |
| [`TFMOD`](../docs/isa/TFMOD.md) | TODO | TODO | Yes | Yes | Yes |
| [`TFMODS`](../docs/isa/TFMODS.md) | TODO | TODO | Yes | Yes | Yes |
| [`TRESHAPE`](../docs/isa/TRESHAPE.md) | Yes | TODO | Yes | Yes | Yes |
| [`TROWEXPAND`](../docs/isa/TROWEXPAND.md) | Yes | TODO | Yes | Yes | Yes |
| [`TROWEXPANDADD `]() | TODO | TODO | Yes | Yes | Yes |
| [`TROWEXPANDDIV`](../docs/isa/TROWEXPANDDIV.md) | Yes | TODO | Yes | Yes | Yes |
| [`TROWEXPANDEXPDIF`]() | TODO | TODO | Yes | Yes | Yes |
| [`TROWEXPANDMAX`](../docs/isa/TROWEXPANDMUL.md) | Yes | TODO | Yes | Yes | Yes |
| [`TROWEXPANDMIN`]() | TODO | TODO | Yes | Yes | Yes |
| [`TROWEXPANDMUL`]() | TODO | TODO | Yes | Yes | Yes |
| [`TROWEXPANDSUB`](../docs/isa/TROWEXPANDSUB.md) | Yes | TODO | Yes | Yes | Yes |
| [`TROWMAX`](../docs/isa/TROWMAX.md) | Yes | Yes | Yes | Yes | Yes |
| [`TROWMIN`](../docs/isa/TROWMIN.md) | Yes | TODO | Yes | Yes | Yes |
| [`TROWARGMAX`](../docs/isa/TROWARGMAX.md) | TODO | TODO | Yes | Yes | Yes |
| [`TROWARGMIN`](../docs/isa/TROWARGMIN.md) | TODO | TODO | Yes | Yes | Yes |
| [`TROWSUM`](../docs/isa/TROWSUM.md) | Yes | TODO | Yes | Yes | Yes |
| [`TROWPROD`](../docs/isa/TROWPROD.md) | TODO | TODO | Yes | Yes | Yes |
| [`TRSQRT`](../docs/isa/TRSQRT.md) | Yes | TODO | Yes | Yes | Yes |
| [`TSCATTER`](../docs/isa/TSCATTER.md) | Yes | TODO | Yes | Yes | Yes |
| [`TSEL`](../docs/isa/TSEL.md) | Yes | TODO | Yes | Yes | Yes |
| [`TSELS`](../docs/isa/TSELS.md) | Yes | TODO | Yes | Yes | Yes |
| [`TSHL`](../docs/isa/TSHL.md) | Yes | TODO | Yes | Yes | Yes |
| [`TSHLS`](../docs/isa/TSHLS.md) | Yes | TODO | Yes | Yes | Yes |
| [`TSHR`](../docs/isa/TSHR.md) | Yes | TODO | Yes | Yes | Yes |
| [`TSHRS`](../docs/isa/TSHRS.md) | Yes | TODO | Yes | Yes | Yes |
| [`TSORT32`](../docs/isa/TSORT32.md) | Yes | TODO | Yes | Yes | Yes |
| [`TSQRT`](../docs/isa/TSQRT.md) | Yes | Yes | Yes | Yes | Yes |
| [`TSTORE`](../docs/isa/TSTORE.md) | Yes | TODO | Yes | Yes | Yes |
| [`TSTORE_FP`](../docs/isa/TSTORE_FP.md) | TODO | TODO | TODO | TODO | TODO |
| [`TSUB`](../docs/isa/TSUB.md) | Yes | Yes | Yes | Yes | Yes |
| [`TSUBC`](../docs/isa/TSUBC.md) | Yes | TODO | TODO | TODO | TODO |
| [`TSUBS`](../docs/isa/TSUBS.md) | Yes | TODO | Yes | Yes | Yes |
| [`TSUBSC`](../docs/isa/TSUBSC.md) | Yes | TODO | TODO | TODO | TODO |
| [`TSYNC`](../docs/isa/TSYNC.md) | TODO | TODO | Yes | Yes | Yes |
| [`TTRANS`](../docs/isa/TTRANS.md) | Yes | TODO | Yes | Yes | Yes |
| [`TTRI`]() | TODO | TODO | Yes | Yes | Yes |
| [`TRANDOM`](../docs/isa/TRANDOM.md) | TODO | TODO | TODO | TODO | Yes |
| [`TXOR`](../docs/isa/TXOR.md) | Yes | TODO | Yes | Yes | Yes |
| [`TXORS`](../docs/isa/TXORS.md) | Yes | TODO | Yes | Yes | Yes |
