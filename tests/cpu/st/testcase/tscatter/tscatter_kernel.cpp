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

template <int kTRows_, int kTCols_>
AICORE void runTScatter(__gm__ float __out__ *out, __gm__ float __in__ *src, __gm__ uint16_t __in__ *idx)
{
    using TileT = Tile<TileType::Vec, float, kTRows_, kTCols_, BLayout::RowMajor, -1, -1>;
    using IdxT = Tile<TileType::Vec, uint16_t, kTRows_, kTCols_, BLayout::RowMajor, -1, -1>;

    using SrcShape = Shape<1, 1, 1, kTRows_, kTCols_>;
    using SrcStride = Stride<1, 1, 1, kTCols_, 1>;
    using GTf = GlobalTensor<float, SrcShape, SrcStride>;
    using GTi = GlobalTensor<uint16_t, SrcShape, SrcStride>;

    TileT srcTile(kTRows_, kTCols_);
    TileT dstTile(kTRows_, kTCols_);
    IdxT idxTile(kTRows_, kTCols_);

    GTf srcGlobal(src);
    GTf dstGlobal(out);
    GTi idxGlobal(idx);

    TASSIGN(srcTile, 0);
    TASSIGN(dstTile, kTRows_ * kTCols_ * sizeof(typename TileT::DType));
    TASSIGN(idxTile, 2 * kTRows_ * kTCols_ * sizeof(typename TileT::DType));

    TLOAD(srcTile, srcGlobal);
    TLOAD(idxTile, idxGlobal);
    TEXPANDS(dstTile, 0.0f);
    TSCATTER(dstTile, srcTile, idxTile);
    TSTORE(dstGlobal, dstTile);
    out = dstGlobal.data();
}

template <int kTRows_, int kTCols_>
void LaunchTScatter(float *out, float *src, uint16_t *idx, void *stream)
{
    runTScatter<kTRows_, kTCols_>(out, src, idx);
}

template void LaunchTScatter<16, 16>(float *out, float *src, uint16_t *idx, void *stream);
