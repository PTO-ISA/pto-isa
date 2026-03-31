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

template <int kTileRows, int kTileCols, int kSrcLen>
AICORE void runMGather(__gm__ float __out__ *out, __gm__ float __in__ *src, __gm__ uint32_t __in__ *idx)
{
    using TileT = Tile<TileType::Vec, float, kTileRows, kTileCols, BLayout::RowMajor, -1, -1>;
    using IdxT = Tile<TileType::Vec, uint32_t, kTileRows, kTileCols, BLayout::RowMajor, -1, -1>;

    using SrcShape = Shape<1, 1, 1, 1, kSrcLen>;
    using SrcStride = Stride<1, 1, 1, kSrcLen, 1>;
    using SrcGT = GlobalTensor<float, SrcShape, SrcStride>;

    using TileShape = Shape<1, 1, 1, kTileRows, kTileCols>;
    using TileStride = Stride<1, 1, 1, kTileCols, 1>;
    using TileGTf = GlobalTensor<float, TileShape, TileStride>;
    using TileGTi = GlobalTensor<uint32_t, TileShape, TileStride>;

    SrcGT srcGlobal(src);
    TileGTf outGlobal(out);
    TileGTi idxGlobal(idx);

    TileT outTile(kTileRows, kTileCols);
    IdxT idxTile(kTileRows, kTileCols);

    TASSIGN(idxTile, 0);
    TASSIGN(outTile, kTileRows * kTileCols * sizeof(typename IdxT::DType));
    TLOAD(idxTile, idxGlobal);
    MGATHER(outTile, srcGlobal, idxTile);
    TSTORE(outGlobal, outTile);
    out = outGlobal.data();
}

template <int kTileRows, int kTileCols, int kSrcLen>
void LaunchMGather(float *out, float *src, uint32_t *idx, void *stream)
{
    runMGather<kTileRows, kTileCols, kSrcLen>(out, src, idx);
}

template void LaunchMGather<16, 16, 512>(float *out, float *src, uint32_t *idx, void *stream);
