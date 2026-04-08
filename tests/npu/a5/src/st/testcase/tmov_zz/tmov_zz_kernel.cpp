/**
Copyright (c) 2026 Huawei Technologies Co., Ltd.
This program is free software, you can redistribute it and/or modify it under the terms and conditions of
CANN Open Software License Agreement Version 2.0 (the "License").
Please refer to the License for details. You may not use this file except in compliance with the License.
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
See LICENSE in the root of the software repository for the full text of the License.
*/
#include <pto/pto-inst.hpp>
#include <pto/common/constants.hpp>
#include <pto/npu/a5/TQuant.hpp>
#include "acl/acl.h"

using namespace pto;

#define PTO_CEIL(x, y) ((((x) + (y)-1) / (y)) * (y))

namespace TMovZZTest {

template <int validRows, int validCols>
AICORE void runTMovZZ(__gm__ uint8_t *outFp8Nz, __gm__ float *src, __gm__ uint8_t *outE8Zz, __gm__ uint16_t *idx)
{
    (void)idx; // idx not used; TMovNdTo2Zz generates gather indices internally

    constexpr int paddedCols = PTO_CEIL(validCols, BLOCK_SIZE / sizeof(uint32_t));
    constexpr int groupedColsValid = paddedCols / 32;
    constexpr int groupedColsFlattened = validRows * groupedColsValid;

    using SrcGlobal = GlobalTensor<float, Shape<1, 1, 1, validRows, validCols>, pto::Stride<1, 1, 1, validCols, 1>>;
    using DstE8Global =
        GlobalTensor<uint8_t, Shape<1, 1, 1, 1, groupedColsFlattened>, pto::Stride<1, 1, 1, groupedColsFlattened, 1>>;
    using DstFp8GlobalNZ = GlobalTensor<int8_t, TileShape2D<int8_t, validRows, paddedCols, Layout::NZ>,
                                        BaseShape2D<int8_t, validRows, paddedCols, Layout::NZ>, Layout::NZ>;

    using SrcTile = Tile<TileType::Vec, float, validRows, paddedCols, BLayout::RowMajor, -1, -1, SLayout::NoneBox, 512,
                         PadValue::Zero>;
    using DstFP8Tile = Tile<TileType::Vec, int8_t, validRows, paddedCols, BLayout::RowMajor, validRows, paddedCols,
                            SLayout::NoneBox, 512, PadValue::Zero>;
    using MaxTile = Tile<TileType::Vec, float, 1, groupedColsFlattened, BLayout::RowMajor, -1, -1>;
    // E8M0 exponent tile from TQUANT: 1D flat (TQUANT writes exponents contiguously)
    using E8NdTile = Tile<TileType::Vec, uint8_t, 1, groupedColsFlattened, BLayout::RowMajor, -1, -1, SLayout::NoneBox,
                          512, PadValue::Zero>;
    // E8M0 ZZ destination: 2D with fractalMxSize=32 for [16,2] inner box
    using E8ZzTile = Tile<TileType::Vec, uint8_t, validRows, groupedColsValid, BLayout::RowMajor, -1, -1,
                          SLayout::RowMajor, 32, PadValue::Zero>;
    // 1D flat tile for TSTORE (TSTORE has no dispatch path for isRowMajor + SLayout::RowMajor)
    using E8StoreTile = Tile<TileType::Vec, uint8_t, 1, groupedColsFlattened, BLayout::RowMajor, -1, -1,
                             SLayout::NoneBox, 512, PadValue::Zero>;
    // Scratch tile for TMOV ZZ gather-index generation
    constexpr int tmpBufSize = (16 + (validRows / 16) * (groupedColsValid / 2) + 16) * sizeof(uint16_t);
    constexpr int tmpBufSizeAligned = PTO_CEIL(tmpBufSize, 32);
    using TmpTile = Tile<TileType::Vec, uint8_t, 1, tmpBufSizeAligned, BLayout::RowMajor, -1, -1, SLayout::NoneBox, 512,
                         PadValue::Zero>;

    constexpr int virtualRow = PTO_CEIL(validRows, FRACTAL_NZ_ROW) + 1;
    using Fp8NZTile = Tile<TileType::Vec, int8_t, virtualRow, paddedCols, BLayout::ColMajor, validRows, paddedCols,
                           SLayout::RowMajor, 512, PadValue::Null, CompactMode::RowPlusOne>;

    SrcTile srcTile(validRows, validCols);
    SrcTile scalingTile(validRows, validCols);
    DstFP8Tile fp8Tile;
    E8NdTile e8Tile(1, groupedColsFlattened);
    E8ZzTile e8ZzTile(validRows, groupedColsValid);
    E8StoreTile e8StoreTile(1, groupedColsFlattened);
    MaxTile maxPerGpTile(1, groupedColsFlattened);
    Fp8NZTile fp8TileNZ;
    TmpTile tmpTile(1, tmpBufSizeAligned);

    SrcGlobal srcGlobal(src);
    DstE8Global e8Global(outE8Zz);
    DstFp8GlobalNZ fp8GlobalNZ((__gm__ int8_t *)outFp8Nz);

    constexpr int UB_SIZE = 0x40000;
    constexpr int srcTileBytes = validRows * paddedCols * sizeof(float);
    constexpr int maxTileBytes = groupedColsFlattened * sizeof(float);
    constexpr bool unrollCondition = (validRows * paddedCols > 1024) && ((validRows * paddedCols) % 256 == 0);
    constexpr int scalingTileBytes = groupedColsFlattened * sizeof(float) * (unrollCondition ? 2 : 1);
    constexpr int e8TileBytes = groupedColsFlattened * sizeof(uint8_t);
    constexpr int fp8TileBytes = validRows * paddedCols * sizeof(int8_t);
    constexpr int fp8TileNZBytes = virtualRow * paddedCols * sizeof(int8_t);

    constexpr int srcTileAddr = 0x0;
    constexpr int maxTileAddr = srcTileAddr + srcTileBytes;
    constexpr int scalingTileAddr = maxTileAddr + maxTileBytes;
    constexpr int e8TileAddr = scalingTileAddr + scalingTileBytes;
    constexpr int fp8TileAddr = 0x0;
    constexpr int fp8TileNZAddr = fp8TileAddr + fp8TileBytes; // must not overlap fp8Tile for TMOV
    constexpr int tmpTileAddr = scalingTileAddr;              // reuses freed scaling space after TQUANT
    constexpr int workTileEnd = e8TileAddr + e8TileBytes;
    constexpr int fp8End = fp8TileNZAddr + fp8TileNZBytes;
    constexpr int layoutEnd = PTO_CEIL(workTileEnd > fp8End ? workTileEnd : fp8End, 0x100);
    static_assert(layoutEnd <= UB_SIZE, "UB layout exceeds 0x40000.");
    static_assert(tmpBufSizeAligned <= scalingTileBytes, "Tmp tile exceeds reused scaling space.");

    TASSIGN(srcTile, srcTileAddr);
    TASSIGN(maxPerGpTile, maxTileAddr);
    TASSIGN(scalingTile, scalingTileAddr);
    TASSIGN(e8Tile, e8TileAddr);
    TASSIGN(e8ZzTile, maxTileAddr);    // reuses maxPerGpTile space (freed after TQUANT)
    TASSIGN(e8StoreTile, maxTileAddr); // alias for flat TSTORE
    TASSIGN(fp8Tile, fp8TileAddr);
    TASSIGN(fp8TileNZ, fp8TileNZAddr);
    TASSIGN(tmpTile, tmpTileAddr);

    TLOAD(srcTile, srcGlobal);
    set_flag(PIPE_MTE2, PIPE_V, EVENT_ID0);
    wait_flag(PIPE_MTE2, PIPE_V, EVENT_ID0);

    // Phase 1: Quantize FP32 -> MXFP8 (FP8 e4m3 in ND + E8M0 exponents in ND)
    TQUANT<pto::QuantType::MXFP8>(fp8Tile, srcTile, &e8Tile, &maxPerGpTile, &scalingTile);
    // Phase 2: Convert FP8 data ND -> NZ layout
    TMOV(fp8TileNZ, fp8Tile);
    // Phase 3: Convert E8M0 exponents ND -> ZZ layout
    TMOV(e8ZzTile, e8Tile, tmpTile);

    set_flag(PIPE_V, PIPE_MTE3, EVENT_ID0);
    wait_flag(PIPE_V, PIPE_MTE3, EVENT_ID0);
    TSTORE(e8Global, e8StoreTile);
    TSTORE(fp8GlobalNZ, fp8TileNZ);
}

template <int validRows, int validCols>
__global__ AICORE void launchTMovZZKernel(__gm__ uint8_t *outFp8Nz, __gm__ float *src, __gm__ uint8_t *outE8Zz,
                                          __gm__ uint16_t *idx)
{
    runTMovZZ<validRows, validCols>(outFp8Nz, src, outE8Zz, idx);
}

template <int validRows, int validCols>
void LaunchTMovZZ(uint8_t *dstFp8Nz, float *src, uint8_t *dstE8Zz, uint16_t *idx, void *stream)
{
    launchTMovZZKernel<validRows, validCols><<<1, nullptr, stream>>>(dstFp8Nz, src, dstE8Zz, idx);
}

template void LaunchTMovZZ<32, 64>(uint8_t *dstFp8Nz, float *src, uint8_t *dstE8Zz, uint16_t *idx, void *stream);
template void LaunchTMovZZ<64, 64>(uint8_t *dstFp8Nz, float *src, uint8_t *dstE8Zz, uint16_t *idx, void *stream);
template void LaunchTMovZZ<64, 128>(uint8_t *dstFp8Nz, float *src, uint8_t *dstE8Zz, uint16_t *idx, void *stream);
template void LaunchTMovZZ<64, 192>(uint8_t *dstFp8Nz, float *src, uint8_t *dstE8Zz, uint16_t *idx, void *stream);
template void LaunchTMovZZ<64, 256>(uint8_t *dstFp8Nz, float *src, uint8_t *dstE8Zz, uint16_t *idx, void *stream);
template void LaunchTMovZZ<64, 320>(uint8_t *dstFp8Nz, float *src, uint8_t *dstE8Zz, uint16_t *idx, void *stream);
template void LaunchTMovZZ<64, 384>(uint8_t *dstFp8Nz, float *src, uint8_t *dstE8Zz, uint16_t *idx, void *stream);
template void LaunchTMovZZ<64, 448>(uint8_t *dstFp8Nz, float *src, uint8_t *dstE8Zz, uint16_t *idx, void *stream);
template void LaunchTMovZZ<64, 512>(uint8_t *dstFp8Nz, float *src, uint8_t *dstE8Zz, uint16_t *idx, void *stream);
template void LaunchTMovZZ<64, 576>(uint8_t *dstFp8Nz, float *src, uint8_t *dstE8Zz, uint16_t *idx, void *stream);
template void LaunchTMovZZ<64, 640>(uint8_t *dstFp8Nz, float *src, uint8_t *dstE8Zz, uint16_t *idx, void *stream);
template void LaunchTMovZZ<64, 704>(uint8_t *dstFp8Nz, float *src, uint8_t *dstE8Zz, uint16_t *idx, void *stream);
template void LaunchTMovZZ<64, 768>(uint8_t *dstFp8Nz, float *src, uint8_t *dstE8Zz, uint16_t *idx, void *stream);
template void LaunchTMovZZ<64, 832>(uint8_t *dstFp8Nz, float *src, uint8_t *dstE8Zz, uint16_t *idx, void *stream);
template void LaunchTMovZZ<64, 896>(uint8_t *dstFp8Nz, float *src, uint8_t *dstE8Zz, uint16_t *idx, void *stream);
template void LaunchTMovZZ<128, 128>(uint8_t *dstFp8Nz, float *src, uint8_t *dstE8Zz, uint16_t *idx, void *stream);
template void LaunchTMovZZ<128, 256>(uint8_t *dstFp8Nz, float *src, uint8_t *dstE8Zz, uint16_t *idx, void *stream);
template void LaunchTMovZZ<128, 384>(uint8_t *dstFp8Nz, float *src, uint8_t *dstE8Zz, uint16_t *idx, void *stream);
template void LaunchTMovZZ<256, 192>(uint8_t *dstFp8Nz, float *src, uint8_t *dstE8Zz, uint16_t *idx, void *stream);

} // namespace TMovZZTest
