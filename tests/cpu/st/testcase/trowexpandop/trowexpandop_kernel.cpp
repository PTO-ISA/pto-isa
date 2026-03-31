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

template <typename T, int kTRows_, int kTCols_, typename LaunchFn>
AICORE void runTROWEXPANDOP(__gm__ T __out__ *out, __gm__ T __in__ *src0, __gm__ T __in__ *src1, LaunchFn fn)
{
    using GlobShapeDim5 = Shape<1, 1, 1, kTRows_, kTCols_>;
    using GlobStridDim5 = Stride<1, 1, 1, kTCols_, 1>;
    using GlobalMat = GlobalTensor<T, GlobShapeDim5, GlobStridDim5>;

    using GlobShapeVec = Shape<1, 1, 1, kTRows_, 1>;
    using GlobStrideVec = Stride<1, 1, 1, 1, 1>;
    using GlobalVec = GlobalTensor<T, GlobShapeVec, GlobStrideVec>;

    using TileMat = Tile<TileType::Vec, T, kTRows_, kTCols_, BLayout::RowMajor, -1, -1>;
    using TileVec = Tile<TileType::Vec, T, kTRows_, 1, BLayout::ColMajor, -1, -1>;
    TileMat src0Tile(kTRows_, kTCols_);
    TileVec src1Tile(kTRows_, 1);
    TileMat dstTile(kTRows_, kTCols_);

    GlobalMat src0Global(src0);
    GlobalVec src1Global(src1);
    GlobalMat dstGlobal(out);

    TASSIGN(src0Tile, 0);
    TASSIGN(dstTile, kTRows_ * kTCols_ * sizeof(typename TileMat::DType));
    TASSIGN(src1Tile, 2 * kTRows_ * kTCols_ * sizeof(typename TileMat::DType));

    TLOAD(src0Tile, src0Global);
    TLOAD(src1Tile, src1Global);
    fn(dstTile, src0Tile, src1Tile);
    TSTORE(dstGlobal, dstTile);
    out = dstGlobal.data();
}

template <typename T, int kTRows_, int kTCols_>
void LaunchTROWEXPANDDIV(T *out, T *src0, T *src1, void *stream)
{
    using TileDst = Tile<TileType::Vec, T, kTRows_, kTCols_, BLayout::RowMajor, -1, -1>;
    using TileSrc1 = Tile<TileType::Vec, T, kTRows_, 1, BLayout::ColMajor, -1, -1>;
    if constexpr (std::is_same_v<T, aclFloat16>) {
        runTROWEXPANDOP<half, kTRows_, kTCols_>(
            (half *)(out), (half *)(src0), (half *)(src1),
            [](TileDst &dst, TileDst &src0, TileSrc1 &src1) { TROWEXPANDDIV(dst, src0, src1); });
    } else {
        runTROWEXPANDOP<T, kTRows_, kTCols_>(
            out, src0, src1, [](TileDst &dst, TileDst &src0, TileSrc1 &src1) { TROWEXPANDDIV(dst, src0, src1); });
    }
}

template <typename T, int kTRows_, int kTCols_>
void LaunchTROWEXPANDMUL(T *out, T *src0, T *src1, void *stream)
{
    using TileDst = Tile<TileType::Vec, T, kTRows_, kTCols_, BLayout::RowMajor, -1, -1>;
    using TileSrc1 = Tile<TileType::Vec, T, kTRows_, 1, BLayout::ColMajor, -1, -1>;
    if constexpr (std::is_same_v<T, aclFloat16>) {
        runTROWEXPANDOP<half, kTRows_, kTCols_>(
            (half *)(out), (half *)(src0), (half *)(src1),
            [](TileDst &dst, TileDst &src0, TileSrc1 &src1) { TROWEXPANDMUL(dst, src0, src1); });
    } else {
        runTROWEXPANDOP<T, kTRows_, kTCols_>(
            out, src0, src1, [](TileDst &dst, TileDst &src0, TileSrc1 &src1) { TROWEXPANDMUL(dst, src0, src1); });
    }
}

template <typename T, int kTRows_, int kTCols_>
void LaunchTROWEXPANDSUB(T *out, T *src0, T *src1, void *stream)
{
    using TileDst = Tile<TileType::Vec, T, kTRows_, kTCols_, BLayout::RowMajor, -1, -1>;
    using TileSrc1 = Tile<TileType::Vec, T, kTRows_, 1, BLayout::ColMajor, -1, -1>;
    if constexpr (std::is_same_v<T, aclFloat16>) {
        runTROWEXPANDOP<half, kTRows_, kTCols_>(
            (half *)(out), (half *)(src0), (half *)(src1),
            [](TileDst &dst, TileDst &src0, TileSrc1 &src1) { TROWEXPANDSUB(dst, src0, src1); });
    } else {
        runTROWEXPANDOP<T, kTRows_, kTCols_>(
            out, src0, src1, [](TileDst &dst, TileDst &src0, TileSrc1 &src1) { TROWEXPANDSUB(dst, src0, src1); });
    }
}

template <typename T, int kTRows_, int kTCols_>
void LaunchTROWEXPANDADD(T *out, T *src0, T *src1, void *stream)
{
    using TileDst = Tile<TileType::Vec, T, kTRows_, kTCols_, BLayout::RowMajor, -1, -1>;
    using TileSrc1 = Tile<TileType::Vec, T, kTRows_, 1, BLayout::ColMajor, -1, -1>;
    if constexpr (std::is_same_v<T, aclFloat16>) {
        runTROWEXPANDOP<half, kTRows_, kTCols_>(
            (half *)(out), (half *)(src0), (half *)(src1),
            [](TileDst &dst, TileDst &src0, TileSrc1 &src1) { TROWEXPANDADD(dst, src0, src1); });
    } else {
        runTROWEXPANDOP<T, kTRows_, kTCols_>(
            out, src0, src1, [](TileDst &dst, TileDst &src0, TileSrc1 &src1) { TROWEXPANDADD(dst, src0, src1); });
    }
}

template <typename T, int kTRows_, int kTCols_>
void LaunchTROWEXPANDMAX(T *out, T *src0, T *src1, void *stream)
{
    using TileDst = Tile<TileType::Vec, T, kTRows_, kTCols_, BLayout::RowMajor, -1, -1>;
    using TileSrc1 = Tile<TileType::Vec, T, kTRows_, 1, BLayout::ColMajor, -1, -1>;
    if constexpr (std::is_same_v<T, aclFloat16>) {
        runTROWEXPANDOP<half, kTRows_, kTCols_>(
            (half *)(out), (half *)(src0), (half *)(src1),
            [](TileDst &dst, TileDst &src0, TileSrc1 &src1) { TROWEXPANDMAX(dst, src0, src1); });
    } else {
        runTROWEXPANDOP<T, kTRows_, kTCols_>(
            out, src0, src1, [](TileDst &dst, TileDst &src0, TileSrc1 &src1) { TROWEXPANDMAX(dst, src0, src1); });
    }
}

template <typename T, int kTRows_, int kTCols_>
void LaunchTROWEXPANDMIN(T *out, T *src0, T *src1, void *stream)
{
    using TileDst = Tile<TileType::Vec, T, kTRows_, kTCols_, BLayout::RowMajor, -1, -1>;
    using TileSrc1 = Tile<TileType::Vec, T, kTRows_, 1, BLayout::ColMajor, -1, -1>;
    if constexpr (std::is_same_v<T, aclFloat16>) {
        runTROWEXPANDOP<half, kTRows_, kTCols_>(
            (half *)(out), (half *)(src0), (half *)(src1),
            [](TileDst &dst, TileDst &src0, TileSrc1 &src1) { TROWEXPANDMIN(dst, src0, src1); });
    } else {
        runTROWEXPANDOP<T, kTRows_, kTCols_>(
            out, src0, src1, [](TileDst &dst, TileDst &src0, TileSrc1 &src1) { TROWEXPANDMIN(dst, src0, src1); });
    }
}

template <typename T, int kTRows_, int kTCols_>
void LaunchTROWEXPANDEXPDIF(T *out, T *src0, T *src1, void *stream)
{
    using TileDst = Tile<TileType::Vec, T, kTRows_, kTCols_, BLayout::RowMajor, -1, -1>;
    using TileSrc1 = Tile<TileType::Vec, T, kTRows_, 1, BLayout::ColMajor, -1, -1>;
    if constexpr (std::is_same_v<T, aclFloat16>) {
        runTROWEXPANDOP<half, kTRows_, kTCols_>(
            (half *)(out), (half *)(src0), (half *)(src1),
            [](TileDst &dst, TileDst &src0, TileSrc1 &src1) { TROWEXPANDEXPDIF(dst, src0, src1); });
    } else {
        runTROWEXPANDOP<T, kTRows_, kTCols_>(
            out, src0, src1, [](TileDst &dst, TileDst &src0, TileSrc1 &src1) { TROWEXPANDEXPDIF(dst, src0, src1); });
    }
}

template void LaunchTROWEXPANDDIV<float, 64, 64>(float *out, float *src0, float *src1, void *stream);
template void LaunchTROWEXPANDDIV<aclFloat16, 16, 256>(aclFloat16 *out, aclFloat16 *src0, aclFloat16 *src1,
                                                       void *stream);
template void LaunchTROWEXPANDMUL<float, 64, 64>(float *out, float *src0, float *src1, void *stream);
template void LaunchTROWEXPANDMUL<aclFloat16, 16, 256>(aclFloat16 *out, aclFloat16 *src0, aclFloat16 *src1,
                                                       void *stream);
template void LaunchTROWEXPANDSUB<float, 64, 64>(float *out, float *src0, float *src1, void *stream);
template void LaunchTROWEXPANDSUB<aclFloat16, 16, 256>(aclFloat16 *out, aclFloat16 *src0, aclFloat16 *src1,
                                                       void *stream);
template void LaunchTROWEXPANDADD<float, 64, 64>(float *out, float *src0, float *src1, void *stream);
template void LaunchTROWEXPANDADD<aclFloat16, 16, 256>(aclFloat16 *out, aclFloat16 *src0, aclFloat16 *src1,
                                                       void *stream);
template void LaunchTROWEXPANDMAX<float, 64, 64>(float *out, float *src0, float *src1, void *stream);
template void LaunchTROWEXPANDMAX<aclFloat16, 16, 256>(aclFloat16 *out, aclFloat16 *src0, aclFloat16 *src1,
                                                       void *stream);
template void LaunchTROWEXPANDMIN<float, 64, 64>(float *out, float *src0, float *src1, void *stream);
template void LaunchTROWEXPANDMIN<aclFloat16, 16, 256>(aclFloat16 *out, aclFloat16 *src0, aclFloat16 *src1,
                                                       void *stream);
template void LaunchTROWEXPANDEXPDIF<float, 64, 64>(float *out, float *src0, float *src1, void *stream);
template void LaunchTROWEXPANDEXPDIF<aclFloat16, 16, 256>(aclFloat16 *out, aclFloat16 *src0, aclFloat16 *src1,
                                                          void *stream);
