/**
Copyright (c) 2026 Huawei Technologies Co., Ltd.
This program is free software, you can redistribute it and/or modify it under the terms and conditions of
CANN Open Software License Agreement Version 2.0 (the "License").
Please refer to the License for details. You may not use this file except in compliance with the License.
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
See LICENSE in the root of the software repository for the full text of the License.
*/

// Define missing constants needed by THistogram.hpp (they live locally in TTopK.hpp)
namespace pto {
constexpr unsigned ElemPerRepeatB8 = 256;  // REPEAT_BYTE / sizeof(uint8_t)
constexpr unsigned ElemPerRepeatB16 = 128; // REPEAT_BYTE / sizeof(uint16_t)
} // namespace pto

#include <pto/pto-inst.hpp>
#include <pto/common/constants.hpp>
#include "acl/acl.h"

using namespace pto;

#define PTO_DIV_ROUNDUP(x, y) (((x) + (y)-1) / (y))
#define PTO_CEIL(x, y) (PTO_DIV_ROUNDUP(x, y) * (y))

template <int validRows, int validCols, bool isMSB = true>
__global__ AICORE void runTHistogram(__gm__ uint16_t *src, __gm__ uint32_t __out__ *dst, __gm__ uint8_t *idx)
{
    constexpr uint16_t alignedSrcCol = PTO_CEIL(validCols, BLOCK_BYTE_SIZE / sizeof(uint16_t));
    constexpr uint16_t alignedIdxBytes = PTO_CEIL(validRows * sizeof(uint8_t), BLOCK_BYTE_SIZE); // Align idx to 32B

    using GlobalDataSrc =
        GlobalTensor<uint16_t, Shape<1, 1, 1, validRows, validCols>, pto::Stride<1, 1, 1, validCols, 1>>;
    using GlobalDataDst = GlobalTensor<uint32_t, Shape<1, 1, 1, validRows, 256>, pto::Stride<1, 1, 1, 256, 1>>;
    using GlobalDataIdx = GlobalTensor<uint8_t, Shape<1, 1, 1, 1, validRows>, pto::Stride<1, 1, 1, 1, 1>>;

    using TileDataSrc = Tile<TileType::Vec, uint16_t, validRows, alignedSrcCol, BLayout::RowMajor, -1, -1>;
    using TileDataDst = Tile<TileType::Vec, uint32_t, validRows, 256, BLayout::RowMajor, -1, -1>;
    using TileDataIdx = Tile<TileType::Vec, uint8_t, validRows, 32, BLayout::RowMajor, -1, -1>;

    TileDataSrc srcTile(validRows, validCols);
    TileDataDst dstTile(validRows, 256);
    TileDataIdx idxTile(1, validRows);

    TASSIGN(srcTile, 0x0);
    TASSIGN(dstTile, 0x10000);
    TASSIGN(idxTile, 0x30000);

    GlobalDataSrc srcGlobal(src);
    GlobalDataDst dstGlobal(dst);
    GlobalDataIdx idxGlobal(idx);

    TLOAD(srcTile, srcGlobal);
    if constexpr (!isMSB) {
        TLOAD(idxTile, idxGlobal);
    }

    set_flag(PIPE_MTE2, PIPE_V, EVENT_ID0);
    wait_flag(PIPE_MTE2, PIPE_V, EVENT_ID0);

    THISTOGRAM<isMSB>(dstTile, srcTile, idxTile);

    set_flag(PIPE_V, PIPE_MTE3, EVENT_ID0);
    wait_flag(PIPE_V, PIPE_MTE3, EVENT_ID0);

    TSTORE(dstGlobal, dstTile);
}

template <int validRows, int validCols, bool isMSB>
void LaunchTHistogram(uint16_t *src, uint32_t *dst, void *stream, uint8_t *idx)
{
    runTHistogram<validRows, validCols, isMSB><<<1, nullptr, stream>>>(src, dst, idx);
}

template void LaunchTHistogram<2, 128, true>(uint16_t *src, uint32_t *dst, void *stream, uint8_t *idx);
template void LaunchTHistogram<4, 64, true>(uint16_t *src, uint32_t *dst, void *stream, uint8_t *idx);
template void LaunchTHistogram<8, 128, true>(uint16_t *src, uint32_t *dst, void *stream, uint8_t *idx);
template void LaunchTHistogram<1, 256, true>(uint16_t *src, uint32_t *dst, void *stream, uint8_t *idx);
template void LaunchTHistogram<4, 256, true>(uint16_t *src, uint32_t *dst, void *stream, uint8_t *idx);
template void LaunchTHistogram<2, 100, true>(uint16_t *src, uint32_t *dst, void *stream, uint8_t *idx);
template void LaunchTHistogram<2, 128, false>(uint16_t *src, uint32_t *dst, void *stream, uint8_t *idx);
template void LaunchTHistogram<4, 64, false>(uint16_t *src, uint32_t *dst, void *stream, uint8_t *idx);
template void LaunchTHistogram<8, 128, false>(uint16_t *src, uint32_t *dst, void *stream, uint8_t *idx);
template void LaunchTHistogram<1, 256, false>(uint16_t *src, uint32_t *dst, void *stream, uint8_t *idx);
template void LaunchTHistogram<4, 256, false>(uint16_t *src, uint32_t *dst, void *stream, uint8_t *idx);
template void LaunchTHistogram<2, 100, false>(uint16_t *src, uint32_t *dst, void *stream, uint8_t *idx);
