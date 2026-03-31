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
AICORE void runTCOLEXPANDOP(__gm__ T __out__ *out, __gm__ T __in__ *src0, __gm__ T __in__ *src1, LaunchFn fn)
{
    using DynShapeDim5 = Shape<1, 1, 1, kTRows_, kTCols_>;
    using DynStridDim5 = Stride<1, 1, 1, kTCols_, 1>;
    using GlobalData = GlobalTensor<T, DynShapeDim5, DynStridDim5>;

    using ShapeVec = Shape<1, 1, 1, 1, kTCols_>;
    using StrideVec = Stride<1, 1, 1, 1, 1>;
    using GlobalVec = GlobalTensor<T, ShapeVec, StrideVec>;

    using TileT = Tile<TileType::Vec, T, kTRows_, kTCols_, BLayout::RowMajor, -1, -1>;
    using TileVec = Tile<TileType::Vec, T, 1, kTCols_, BLayout::RowMajor, -1, -1>;
    TileT src0Tile(kTRows_, kTCols_);
    TileVec src1Tile(1, kTCols_);
    TileT dstTile(kTRows_, kTCols_);

    GlobalData src0Global(src0);
    GlobalVec src1Global(src1);
    GlobalData dstGlobal(out);

    TASSIGN(src0Tile, 0);
    TASSIGN(src1Tile, kTRows_ * kTCols_ * sizeof(typename TileT::DType));
    TASSIGN(dstTile, 2 * kTRows_ * kTCols_ * sizeof(typename TileVec::DType));

    TLOAD(src0Tile, src0Global);
    TLOAD(src1Tile, src1Global);
    fn(dstTile, src0Tile, src1Tile);
    TSTORE(dstGlobal, dstTile);
    out = dstGlobal.data();
}

template <typename T, int kTRows_, int kTCols_>
void LaunchTCOLEXPANDDIV(T *out, T *src0, T *src1, void *stream)
{
    using TileDst = Tile<TileType::Vec, T, kTRows_, kTCols_, BLayout::RowMajor, -1, -1>;
    using TileSrc1 = Tile<TileType::Vec, T, 1, kTCols_, BLayout::RowMajor, -1, -1>;
    if constexpr (std::is_same_v<T, aclFloat16>) {
        runTCOLEXPANDOP<half, kTRows_, kTCols_>(
            (half *)(out), (half *)(src0), (half *)(src1),
            [](TileDst &dst, TileDst &src0, TileSrc1 &src1) { TCOLEXPANDDIV(dst, src0, src1); });
    } else {
        runTCOLEXPANDOP<T, kTRows_, kTCols_>(
            out, src0, src1, [](TileDst &dst, TileDst &src0, TileSrc1 &src1) { TCOLEXPANDDIV(dst, src0, src1); });
    }
}

template <typename T, int kTRows_, int kTCols_>
void LaunchTCOLEXPANDMUL(T *out, T *src0, T *src1, void *stream)
{
    using TileDst = Tile<TileType::Vec, T, kTRows_, kTCols_, BLayout::RowMajor, -1, -1>;
    using TileSrc1 = Tile<TileType::Vec, T, 1, kTCols_, BLayout::RowMajor, -1, -1>;
    if constexpr (std::is_same_v<T, aclFloat16>) {
        runTCOLEXPANDOP<half, kTRows_, kTCols_>(
            (half *)(out), (half *)(src0), (half *)(src1),
            [](TileDst &dst, TileDst &src0, TileSrc1 &src1) { TCOLEXPANDMUL(dst, src0, src1); });
    } else {
        runTCOLEXPANDOP<T, kTRows_, kTCols_>(
            out, src0, src1, [](TileDst &dst, TileDst &src0, TileSrc1 &src1) { TCOLEXPANDMUL(dst, src0, src1); });
    }
}

template <typename T, int kTRows_, int kTCols_>
void LaunchTCOLEXPANDSUB(T *out, T *src0, T *src1, void *stream)
{
    using TileDst = Tile<TileType::Vec, T, kTRows_, kTCols_, BLayout::RowMajor, -1, -1>;
    using TileSrc1 = Tile<TileType::Vec, T, 1, kTCols_, BLayout::RowMajor, -1, -1>;
    if constexpr (std::is_same_v<T, aclFloat16>) {
        runTCOLEXPANDOP<half, kTRows_, kTCols_>(
            (half *)(out), (half *)(src0), (half *)(src1),
            [](TileDst &dst, TileDst &src0, TileSrc1 &src1) { TCOLEXPANDSUB(dst, src0, src1); });
    } else {
        runTCOLEXPANDOP<T, kTRows_, kTCols_>(
            out, src0, src1, [](TileDst &dst, TileDst &src0, TileSrc1 &src1) { TCOLEXPANDSUB(dst, src0, src1); });
    }
}

template <typename T, int kTRows_, int kTCols_>
void LaunchTCOLEXPANDADD(T *out, T *src0, T *src1, void *stream)
{
    using TileDst = Tile<TileType::Vec, T, kTRows_, kTCols_, BLayout::RowMajor, -1, -1>;
    using TileSrc1 = Tile<TileType::Vec, T, 1, kTCols_, BLayout::RowMajor, -1, -1>;
    if constexpr (std::is_same_v<T, aclFloat16>) {
        runTCOLEXPANDOP<half, kTRows_, kTCols_>(
            (half *)(out), (half *)(src0), (half *)(src1),
            [](TileDst &dst, TileDst &src0, TileSrc1 &src1) { TCOLEXPANDADD(dst, src0, src1); });
    } else {
        runTCOLEXPANDOP<T, kTRows_, kTCols_>(
            out, src0, src1, [](TileDst &dst, TileDst &src0, TileSrc1 &src1) { TCOLEXPANDADD(dst, src0, src1); });
    }
}

template <typename T, int kTRows_, int kTCols_>
void LaunchTCOLEXPANDMAX(T *out, T *src0, T *src1, void *stream)
{
    using TileDst = Tile<TileType::Vec, T, kTRows_, kTCols_, BLayout::RowMajor, -1, -1>;
    using TileSrc1 = Tile<TileType::Vec, T, 1, kTCols_, BLayout::RowMajor, -1, -1>;
    if constexpr (std::is_same_v<T, aclFloat16>) {
        runTCOLEXPANDOP<half, kTRows_, kTCols_>(
            (half *)(out), (half *)(src0), (half *)(src1),
            [](TileDst &dst, TileDst &src0, TileSrc1 &src1) { TCOLEXPANDMAX(dst, src0, src1); });
    } else {
        runTCOLEXPANDOP<T, kTRows_, kTCols_>(
            out, src0, src1, [](TileDst &dst, TileDst &src0, TileSrc1 &src1) { TCOLEXPANDMAX(dst, src0, src1); });
    }
}

template <typename T, int kTRows_, int kTCols_>
void LaunchTCOLEXPANDMIN(T *out, T *src0, T *src1, void *stream)
{
    using TileDst = Tile<TileType::Vec, T, kTRows_, kTCols_, BLayout::RowMajor, -1, -1>;
    using TileSrc1 = Tile<TileType::Vec, T, 1, kTCols_, BLayout::RowMajor, -1, -1>;
    if constexpr (std::is_same_v<T, aclFloat16>) {
        runTCOLEXPANDOP<half, kTRows_, kTCols_>(
            (half *)(out), (half *)(src0), (half *)(src1),
            [](TileDst &dst, TileDst &src0, TileSrc1 &src1) { TCOLEXPANDMIN(dst, src0, src1); });
    } else {
        runTCOLEXPANDOP<T, kTRows_, kTCols_>(
            out, src0, src1, [](TileDst &dst, TileDst &src0, TileSrc1 &src1) { TCOLEXPANDMIN(dst, src0, src1); });
    }
}

template <typename T, int kTRows_, int kTCols_>
void LaunchTCOLEXPANDEXPDIF(T *out, T *src0, T *src1, void *stream)
{
    using TileDst = Tile<TileType::Vec, T, kTRows_, kTCols_, BLayout::RowMajor, -1, -1>;
    using TileSrc1 = Tile<TileType::Vec, T, 1, kTCols_, BLayout::RowMajor, -1, -1>;
    if constexpr (std::is_same_v<T, aclFloat16>) {
        runTCOLEXPANDOP<half, kTRows_, kTCols_>(
            (half *)(out), (half *)(src0), (half *)(src1),
            [](TileDst &dst, TileDst &src0, TileSrc1 &src1) { TCOLEXPANDEXPDIF(dst, src0, src1); });
    } else {
        runTCOLEXPANDOP<T, kTRows_, kTCols_>(
            out, src0, src1, [](TileDst &dst, TileDst &src0, TileSrc1 &src1) { TCOLEXPANDEXPDIF(dst, src0, src1); });
    }
}

template void LaunchTCOLEXPANDDIV<float, 64, 64>(float *out, float *src0, float *src1, void *stream);
template void LaunchTCOLEXPANDDIV<aclFloat16, 16, 256>(aclFloat16 *out, aclFloat16 *src0, aclFloat16 *src1,
                                                       void *stream);
template void LaunchTCOLEXPANDMUL<float, 64, 64>(float *out, float *src0, float *src1, void *stream);
template void LaunchTCOLEXPANDMUL<aclFloat16, 16, 256>(aclFloat16 *out, aclFloat16 *src0, aclFloat16 *src1,
                                                       void *stream);
template void LaunchTCOLEXPANDSUB<float, 64, 64>(float *out, float *src0, float *src1, void *stream);
template void LaunchTCOLEXPANDSUB<aclFloat16, 16, 256>(aclFloat16 *out, aclFloat16 *src0, aclFloat16 *src1,
                                                       void *stream);
template void LaunchTCOLEXPANDADD<float, 64, 64>(float *out, float *src0, float *src1, void *stream);
template void LaunchTCOLEXPANDADD<aclFloat16, 16, 256>(aclFloat16 *out, aclFloat16 *src0, aclFloat16 *src1,
                                                       void *stream);
template void LaunchTCOLEXPANDMAX<float, 64, 64>(float *out, float *src0, float *src1, void *stream);
template void LaunchTCOLEXPANDMAX<aclFloat16, 16, 256>(aclFloat16 *out, aclFloat16 *src0, aclFloat16 *src1,
                                                       void *stream);
template void LaunchTCOLEXPANDMIN<float, 64, 64>(float *out, float *src0, float *src1, void *stream);
template void LaunchTCOLEXPANDMIN<aclFloat16, 16, 256>(aclFloat16 *out, aclFloat16 *src0, aclFloat16 *src1,
                                                       void *stream);
template void LaunchTCOLEXPANDEXPDIF<float, 64, 64>(float *out, float *src0, float *src1, void *stream);
template void LaunchTCOLEXPANDEXPDIF<aclFloat16, 16, 256>(aclFloat16 *out, aclFloat16 *src0, aclFloat16 *src1,
                                                          void *stream);
