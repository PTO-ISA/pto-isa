/**
Copyright (c) 2025 Huawei Technologies Co., Ltd.
This program is free software, you can redistribute it and/or modify it under the terms and conditions of
CANN Open Software License Agreement Version 2.0 (the "License").
Please refer to the License for details. You may not use this file except in compliance with the License.
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
See LICENSE in the root of the software repository for the full text of the License.
*/

#include <pto/pto-inst.hpp>
#include <pto/common/constants.hpp>

using namespace pto;

template <typename T, int sTRows_, int sTCols_, int dTRows_, int dTCols_, int kGRows_, int kGCols_>
AICORE void runTRelu(__gm__ T __out__ *out, __gm__ T __in__ *src0)
{
    using DynShapeDim5 = Shape<1, 1, 1, kGRows_, kGCols_>;
    using DynStridDim5 = Stride<1, 1, 1, kGCols_, 1>;
    using GlobalDataSrc = GlobalTensor<T, DynShapeDim5, DynStridDim5>;
    using GlobalDataDst = GlobalTensor<T, DynShapeDim5, DynStridDim5>;

    using TileDataSrc = Tile<TileType::Vec, T, sTRows_, sTCols_, BLayout::RowMajor, -1, -1>;
    using TileDataDst = Tile<TileType::Vec, T, dTRows_, dTCols_, BLayout::RowMajor, -1, -1>;
    TileDataSrc src0Tile(kGRows_, kGCols_);
    TileDataDst dstTile(kGRows_, kGCols_);

    GlobalDataSrc src0Global(src0);
    GlobalDataDst dstGlobal(out);

    TASSIGN(src0Tile, 0);
    TASSIGN(dstTile, sTRows_ * sTCols_ * sizeof(typename TileDataSrc::DType));

    TLOAD(src0Tile, src0Global);
    TRELU(dstTile, src0Tile);
    TSTORE(dstGlobal, dstTile);
    out = dstGlobal.data();
}

template <typename T, int sTRows_, int sTCols_, int dTRows_, int dTCols_, int kGRows_, int kGCols_>
void LaunchTRelu(T *out, T *src0, void *stream)
{
    if constexpr (std::is_same_v<T, aclFloat16>)
        runTRelu<half, sTRows_, sTCols_, dTRows_, dTCols_, kGRows_, kGCols_>((half *)(out), (half *)(src0));
    else
        runTRelu<T, sTRows_, sTCols_, dTRows_, dTCols_, kGRows_, kGCols_>(out, src0);
}

const int NUM_16 = 16;
const int NUM_64 = 64;
const int NUM_256 = 256;

template void LaunchTRelu<float, NUM_64, NUM_64, NUM_64, NUM_64, NUM_64, NUM_64>(float *out, float *src0,
                                                                                   void *stream);
template void LaunchTRelu<int32_t, NUM_64, NUM_64, NUM_64, NUM_64, NUM_64, NUM_64>(int32_t *out, int32_t *src0,
                                                                                     void *stream);
template void LaunchTRelu<aclFloat16, NUM_16, NUM_256, NUM_16, NUM_256, NUM_16, NUM_256>(aclFloat16 *out,
                                                                                            aclFloat16 *src0,
                                                                                            void *stream);
template void LaunchTRelu<int16_t, NUM_64, NUM_64, NUM_64, NUM_64, NUM_64, NUM_64>(int16_t *out, int16_t *src0,
                                                                                     void *stream);
#ifdef CPU_SIM_BFLOAT_ENABLED
template void LaunchTRelu<bfloat16_t, NUM_16, NUM_256, NUM_16, NUM_256, NUM_16, NUM_256>(bfloat16_t *out,
                                                                                           bfloat16_t *src0,
                                                                                           void *stream);
#endif
template void LaunchTRelu<float, NUM_64, NUM_64, NUM_64, NUM_64, 60, 55>(float *out, float *src0, void *stream);
template void LaunchTRelu<int32_t, NUM_64, NUM_64, NUM_64, NUM_64, 60, 55>(int32_t *out, int32_t *src0,
                                                                            void *stream);
template void LaunchTRelu<aclFloat16, NUM_64, NUM_64, 96, 96, NUM_64, 60>(aclFloat16 *out, aclFloat16 *src0,
                                                                           void *stream);
template void LaunchTRelu<int16_t, NUM_64, NUM_64, 96, 96, NUM_64, 60>(int16_t *out, int16_t *src0, void *stream);
