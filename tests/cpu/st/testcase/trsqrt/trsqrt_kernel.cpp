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

using namespace pto;

template <int kRows, int kCols>
AICORE void runTRSQRT(__gm__ float __out__ *out, __gm__ float __in__ *src)
{
    using DynShapeDim5 = Shape<1, 1, 1, kRows, kCols>;
    using DynStridDim5 = Stride<1, 1, 1, kCols, 1>;
    using GlobalData = GlobalTensor<float, DynShapeDim5, DynStridDim5>;

    using TileT = Tile<TileType::Vec, float, kRows, kCols, BLayout::RowMajor, -1, -1>;
    TileT srcTile(kRows, kCols);
    TileT dstTile(kRows, kCols);

    GlobalData srcGlobal(src);
    GlobalData dstGlobal(out);

    TASSIGN(srcTile, 0);
    TASSIGN(dstTile, srcTile.Numel * sizeof(typename TileT::DType));
    TLOAD(srcTile, srcGlobal);
    TRSQRT(dstTile, srcTile);
    TSTORE(dstGlobal, dstTile);
    out = dstGlobal.data();
}

template <int kRows, int kCols>
void LaunchTRSQRT(float *out, float *src, void *stream)
{
    (void)stream;
    runTRSQRT<kRows, kCols>(out, src);
}

template void LaunchTRSQRT<64, 64>(float *out, float *src, void *stream);
