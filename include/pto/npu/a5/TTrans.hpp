/**
Copyright (c) 2026 Huawei Technologies Co., Ltd.
This program is free software, you can redistribute it and/or modify it under the terms and conditions of
CANN Open Software License Agreement Version 2.0 (the "License").
Please refer to the License for details. You may not use this file except in compliance with the License.
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
See LICENSE in the root of the software repository for the full text of the License.
*/

#ifndef TTRANS_HPP
#define TTRANS_HPP

#include <pto/common/utils.hpp>
#include <pto/common/constants.hpp>
#include "common.hpp"
#include "utils.hpp"

using namespace pto;
using namespace std;

namespace pto {

template <typename TileDataSrc, typename TileDataDst, unsigned elementsPerRepeat, unsigned blockSizeElem>
PTO_INTERNAL void TTransB32ColWise(__ubuf__ typename TileDataDst::DType *dstPtr,
                                   __ubuf__ typename TileDataSrc::DType *srcPtr, unsigned numRows, unsigned numCols,
                                   unsigned dstStride, unsigned srcStride)
{
    using T = typename TileDataSrc::DType;
    if constexpr (std::is_same_v<T, uint32_t> || std::is_same_v<T, int32_t> || std::is_same_v<T, float>) {
        uint16_t repeatTimes = CeilDivision(numRows, elementsPerRepeat);
        __VEC_SCOPE__
        {
            RegTensor<uint32_t> vreg0;
            RegTensor<T> vreg1;
            MaskReg preg;
            constexpr auto distValue =
                std::integral_constant<::DistVST, static_cast<::DistVST>(GetDistVst<T, DistVST::DIST_NORM_B32>())>();
            for (uint16_t col = 0; col < (uint16_t)numCols; ++col) {
                uint32_t sreg = (uint32_t)numRows;
                for (uint16_t chunk = 0; chunk < repeatTimes; ++chunk) {
                    preg = CreatePredicate<T>(sreg);
                    vci((RegTensor<int32_t> &)vreg0, (int32_t)(chunk * elementsPerRepeat), INC_ORDER);
                    vmins(vreg0, vreg0, (uint32_t)(numRows - 1), preg);
                    vmuls(vreg0, vreg0, srcStride, preg);
                    vadds(vreg0, vreg0, col, preg);
                    vgather2(vreg1, srcPtr, (RegTensor<uint32_t> &)vreg0, preg);
                    vsts(vreg1, dstPtr, (col * dstStride + chunk * elementsPerRepeat), distValue, preg);
                }
            }
        }
    } else {
        static_assert(sizeof(T) == 4, "Fix: TTRANS has Invalid b32 data type.");
    }
}

template <typename TileDataSrc, typename TileDataDst, unsigned elementsPerRepeat, unsigned blockSizeElem>
PTO_INTERNAL void TTransB32RowWise(__ubuf__ typename TileDataDst::DType *dstPtr,
                                   __ubuf__ typename TileDataSrc::DType *srcPtr, unsigned numRows, unsigned numCols,
                                   unsigned dstStride, unsigned srcStride)
{
    using T = typename TileDataSrc::DType;

    if constexpr (std::is_same_v<T, uint32_t> || std::is_same_v<T, int32_t> || std::is_same_v<T, float>) {
        uint16_t repeatTimes = CeilDivision(numCols, elementsPerRepeat);
        __VEC_SCOPE__
        {
            RegTensor<uint32_t> vreg0;
            RegTensor<T> vreg1;
            MaskReg preg;
            for (uint16_t row = 0; row < (uint16_t)numRows; ++row) {
                uint32_t sreg = (uint32_t)numCols;
                for (uint16_t chunk = 0; chunk < repeatTimes; ++chunk) {
                    preg = CreatePredicate<T>(sreg);
                    vlds(vreg1, srcPtr, (row * srcStride + chunk * elementsPerRepeat), NORM);
                    vci((RegTensor<int32_t> &)vreg0, (int32_t)(chunk * elementsPerRepeat), INC_ORDER);
                    vmins(vreg0, vreg0, (uint32_t)(numCols - 1), preg);
                    vmuls(vreg0, vreg0, dstStride, preg);
                    vadds(vreg0, vreg0, row, preg);
                    vscatter(vreg1, dstPtr, (RegTensor<uint32_t> &)vreg0, preg);
                }
            }
        }
    } else {
        static_assert(sizeof(T) == 4, "Fix: TTRANS has Invalid b32 data type.");
    }
}

template <typename TileDataSrc, typename TileDataDst, unsigned elementsPerRepeat, unsigned blockSizeElem>
PTO_INTERNAL void TTransB16ColWise(__ubuf__ typename TileDataDst::DType *dstPtr,
                                   __ubuf__ typename TileDataSrc::DType *srcPtr, unsigned numRows, unsigned numCols,
                                   unsigned dstStride, unsigned srcStride)
{
    using T = typename TileDataSrc::DType;
    if constexpr (std::is_same_v<T, uint16_t> || std::is_same_v<T, int16_t> || std::is_same_v<T, half> ||
                  std::is_same_v<T, bfloat16_t>) {
        uint16_t repeatTimes = CeilDivision(numRows, elementsPerRepeat);
        __VEC_SCOPE__
        {
            RegTensor<uint16_t> vreg0;
            RegTensor<T> vreg1;
            MaskReg preg;
            constexpr auto distValue =
                std::integral_constant<::DistVST, static_cast<::DistVST>(GetDistVst<T, DistVST::DIST_NORM_B16>())>();
            for (uint16_t col = 0; col < (uint16_t)numCols; ++col) {
                uint32_t sreg = (uint32_t)numRows;
                for (uint16_t chunk = 0; chunk < repeatTimes; ++chunk) {
                    preg = CreatePredicate<T>(sreg);
                    vci((RegTensor<int16_t> &)vreg0, (int16_t)(chunk * elementsPerRepeat), INC_ORDER);
                    vmins(vreg0, vreg0, (uint16_t)(numRows - 1), preg);
                    vmuls(vreg0, vreg0, srcStride, preg);
                    vadds(vreg0, vreg0, col, preg);
                    vgather2(vreg1, srcPtr, (RegTensor<uint16_t> &)vreg0, preg);
                    vsts(vreg1, dstPtr, (col * dstStride + chunk * elementsPerRepeat), distValue, preg);
                }
            }
        }
    } else {
        static_assert(sizeof(T) == 2, "Fix: TTRANS has invalid b16 data type.");
    }
}

template <typename TileDataSrc, typename TileDataDst, unsigned elementsPerRepeat, unsigned blockSizeElem>
PTO_INTERNAL void TTransB16RowWise(__ubuf__ typename TileDataDst::DType *dstPtr,
                                   __ubuf__ typename TileDataSrc::DType *srcPtr, unsigned numRows, unsigned numCols,
                                   unsigned dstStride, unsigned srcStride)
{
    using T = typename TileDataSrc::DType;

    if constexpr (std::is_same_v<T, uint16_t> || std::is_same_v<T, int16_t> || std::is_same_v<T, half> ||
                  std::is_same_v<T, bfloat16_t>) {
        uint16_t repeatTimes = CeilDivision(numCols, elementsPerRepeat);
        __VEC_SCOPE__
        {
            RegTensor<uint16_t> vreg0;
            RegTensor<T> vreg1;
            MaskReg preg;
            for (uint16_t row = 0; row < (uint16_t)numRows; ++row) {
                uint32_t sreg = (uint32_t)numCols;
                for (uint16_t chunk = 0; chunk < repeatTimes; ++chunk) {
                    preg = CreatePredicate<T>(sreg);
                    vlds(vreg1, srcPtr, (row * srcStride + chunk * elementsPerRepeat), NORM);
                    vci((RegTensor<int16_t> &)vreg0, (int16_t)(chunk * elementsPerRepeat), INC_ORDER);
                    vmins(vreg0, vreg0, (uint16_t)(numCols - 1), preg);
                    vmuls(vreg0, vreg0, dstStride, preg);
                    vadds(vreg0, vreg0, row, preg);
                    vscatter(vreg1, dstPtr, (RegTensor<uint16_t> &)vreg0, preg);
                }
            }
        }
    } else {
        static_assert(sizeof(T) == 2, "Fix: TTRANS has invalid b16 data type.");
    }
}

template <typename TileDataSrc, typename TileDataDst, unsigned elementsPerRepeat, unsigned blockSizeElem>
PTO_INTERNAL void TTransB8ColWise(__ubuf__ typename TileDataDst::DType *dstPtr,
                                  __ubuf__ typename TileDataSrc::DType *srcPtr, unsigned numRows, unsigned numCols,
                                  unsigned dstStride, unsigned srcStride)
{
    using T = typename TileDataSrc::DType;
    if constexpr (std::is_same_v<T, uint8_t> || std::is_same_v<T, int8_t>) {
        constexpr uint32_t sregLower = elementsPerRepeat >> 1;
        uint16_t repeatTimes = CeilDivision(numRows, sregLower);
        __VEC_SCOPE__
        {
            RegTensor<uint16_t> vreg0;
            RegTensor<T> vreg1;
            MaskReg preg;
            constexpr auto distValue =
                std::integral_constant<::DistVST, static_cast<::DistVST>(GetDistVst<T, DistVST::DIST_PK_B16>())>();
            for (uint16_t col = 0; col < (uint16_t)numCols; ++col) {
                uint32_t sreg = (uint32_t)numRows;
                for (uint16_t chunk = 0; chunk < repeatTimes; ++chunk) {
                    preg = CreatePredicate<uint16_t>(sreg);
                    vci((RegTensor<int16_t> &)vreg0, (int16_t)(chunk * sregLower), INC_ORDER);
                    vmins(vreg0, vreg0, (uint16_t)(numRows - 1), preg);
                    vmuls(vreg0, vreg0, srcStride, preg);
                    vadds(vreg0, vreg0, col, preg);
                    vgather2((RegTensor<uint16_t> &)vreg1, (__ubuf__ uint8_t *)srcPtr, (RegTensor<uint16_t> &)vreg0,
                             preg);
                    vsts(vreg1, dstPtr, (col * dstStride + chunk * sregLower), distValue, preg);
                }
            }
        }
    } else {
        static_assert(sizeof(T) == 1, "Fix: TTRANS has invalid b8 data type.");
    }
}

template <typename TileDataSrc, typename TileDataDst, unsigned elementsPerRepeat, unsigned blockSizeElem>
PTO_INTERNAL void TTransB8RowWise(__ubuf__ typename TileDataDst::DType *dstPtr,
                                  __ubuf__ typename TileDataSrc::DType *srcPtr, unsigned numRows, unsigned numCols,
                                  unsigned dstStride, unsigned srcStride)
{
    using T = typename TileDataSrc::DType;

    if constexpr (std::is_same_v<T, uint8_t> || std::is_same_v<T, int8_t>) {
        constexpr uint32_t sregLower = elementsPerRepeat >> 1;
        uint16_t repeatTimes = CeilDivision(numCols, sregLower);
        __VEC_SCOPE__
        {
            RegTensor<uint16_t> vreg0;
            RegTensor<T> vreg1;
            MaskReg preg;
            for (uint16_t row = 0; row < (uint16_t)numRows; ++row) {
                uint32_t sreg = (uint32_t)numCols;
                for (uint16_t chunk = 0; chunk < repeatTimes; ++chunk) {
                    preg = CreatePredicate<uint16_t>(sreg);
                    vlds(vreg1, srcPtr, (row * srcStride + chunk * sregLower), UNPK_B8);
                    vci((RegTensor<int16_t> &)vreg0, (int16_t)(chunk * sregLower), INC_ORDER);
                    vmins(vreg0, vreg0, (uint16_t)(numCols - 1), preg);
                    vmuls(vreg0, vreg0, dstStride, preg);
                    vadds(vreg0, vreg0, row, preg);
                    vscatter(vreg1, dstPtr, (RegTensor<uint16_t> &)vreg0, preg);
                }
            }
        }
    } else {
        static_assert(sizeof(T) == 1, "Fix: TTRANS has invalid b8 data type.");
    }
}

template <typename TileDataDst, typename TileDataSrc, unsigned elementsPerRepeat, unsigned blockSizeElem>
__tf__ PTO_INTERNAL void TTransTile(typename TileDataDst::TileDType __out__ dst,
                                    typename TileDataSrc::TileDType __in__ src, unsigned numRows, unsigned numCols)
{
    using T = typename TileDataSrc::DType;
    __ubuf__ T *dstPtr = (__ubuf__ T *)__cce_get_tile_ptr(dst);
    __ubuf__ T *srcPtr = (__ubuf__ T *)__cce_get_tile_ptr(src);
    constexpr unsigned srcStride = TileDataSrc::RowStride;
    constexpr unsigned dstStride = TileDataDst::RowStride;
    if constexpr (sizeof(T) == 4) {
        if constexpr (TileDataSrc::Rows < TileDataSrc::Cols) {
            static_assert(
                (unsigned long long)(TileDataDst::Rows - 1) * dstStride + (TileDataDst::Cols - 1) <= 0xFFFFFFFFULL,
                "Fix: TTRANS scatter index may overflow uint32_t register");
            TTransB32RowWise<TileDataSrc, TileDataDst, elementsPerRepeat, blockSizeElem>(dstPtr, srcPtr, numRows,
                                                                                         numCols, dstStride, srcStride);
        } else {
            static_assert(
                (unsigned long long)(TileDataSrc::Rows - 1) * srcStride + (TileDataSrc::Cols - 1) <= 0xFFFFFFFFULL,
                "Fix: TTRANS gather index may overflow uint32_t register");
            TTransB32ColWise<TileDataSrc, TileDataDst, elementsPerRepeat, blockSizeElem>(dstPtr, srcPtr, numRows,
                                                                                         numCols, dstStride, srcStride);
        }
    } else if constexpr (sizeof(T) == 2) {
        if constexpr (TileDataSrc::Rows < TileDataSrc::Cols) {
            static_assert(
                (unsigned long long)(TileDataDst::Rows - 1) * dstStride + (TileDataDst::Cols - 1) <= 0xFFFFULL,
                "Fix: TTRANS scatter index may overflow uint16_t register");
            TTransB16RowWise<TileDataSrc, TileDataDst, elementsPerRepeat, blockSizeElem>(dstPtr, srcPtr, numRows,
                                                                                         numCols, dstStride, srcStride);
        } else {
            static_assert(
                (unsigned long long)(TileDataSrc::Rows - 1) * srcStride + (TileDataSrc::Cols - 1) <= 0xFFFFULL,
                "Fix: TTRANS gather index may overflow uint16_t register");
            TTransB16ColWise<TileDataSrc, TileDataDst, elementsPerRepeat, blockSizeElem>(dstPtr, srcPtr, numRows,
                                                                                         numCols, dstStride, srcStride);
        }
    } else if constexpr (sizeof(T) == 1) {
        if constexpr (TileDataSrc::Rows < TileDataSrc::Cols) {
            static_assert(
                (unsigned long long)(TileDataDst::Rows - 1) * dstStride + (TileDataDst::Cols - 1) <= 0xFFFFULL,
                "Fix: TTRANS scatter index may overflow uint16_t register");
            TTransB8RowWise<TileDataSrc, TileDataDst, elementsPerRepeat, blockSizeElem>(dstPtr, srcPtr, numRows,
                                                                                        numCols, dstStride, srcStride);
        } else {
            static_assert(
                (unsigned long long)(TileDataSrc::Rows - 1) * srcStride + (TileDataSrc::Cols - 1) <= 0xFFFFULL,
                "Fix: TTRANS gather index may overflow uint16_t register");
            TTransB8ColWise<TileDataSrc, TileDataDst, elementsPerRepeat, blockSizeElem>(dstPtr, srcPtr, numRows,
                                                                                        numCols, dstStride, srcStride);
        }
    }
}

template <typename T>
PTO_INTERNAL void TTransConvNCHW2NC1HWC0Unalign(__ubuf__ T *dst, __ubuf__ T *src, unsigned srcN, unsigned dstC1,
                                                unsigned rows, unsigned cols, unsigned nStride, unsigned cStride)
{
    for (int n = 0; n < srcN; n++) {
        for (int c = 0; c < dstC1; c++) {
            __ubuf__ T *srcPtr = src + n * nStride + c * cStride;
            __ubuf__ T *dstPtr = dst + n * nStride + c * cStride;
            for (int i = 0; i < rows; i++) {
                for (int j = 0; j < cols; j++) {
                    dstPtr[j * rows + i] = srcPtr[i * cols + j];
                }
            }
        }
    }
    return;
}

template <typename TileData, typename T, unsigned blockSizeElem>
PTO_INTERNAL void TTransConvNCHW2NC1HWC0Align(__ubuf__ T *dst, __ubuf__ T *src, unsigned srcN, unsigned srcC,
                                              unsigned srcH, unsigned srcW, unsigned dstC0)
{
    unsigned srcStride = srcH * srcW;
    unsigned dstStride = dstC0;
    unsigned rows = dstC0;
    unsigned cols = srcH * srcW;
    unsigned dstC1 = (srcC + dstC0 - 1) / dstC0;
    constexpr unsigned elementsPerRepeat = REPEAT_BYTE / sizeof(T); // REPEAT_BYTE = 256
    unsigned nStride = dstC1 * dstC0 * srcH * srcW;
    unsigned cStride = dstC0 * srcH * srcW;
    for (int n = 0; n < srcN; n++) {
        for (int c = 0; c < dstC1; c++) {
            __ubuf__ T *srcPtr = src + n * nStride + c * cStride;
            __ubuf__ T *dstPtr = dst + n * nStride + c * cStride;
            if constexpr (sizeof(T) == 4) {
                TTransB32ColWise<TileData, TileData, elementsPerRepeat, blockSizeElem>(dstPtr, srcPtr, rows, cols,
                                                                                       dstStride, srcStride);
            } else if constexpr (sizeof(T) == 2) {
                TTransB16ColWise<TileData, TileData, elementsPerRepeat, blockSizeElem>(dstPtr, srcPtr, rows, cols,
                                                                                       dstStride, srcStride);
            } else if constexpr (sizeof(T) == 1) {
                TTransB8ColWise<TileData, TileData, elementsPerRepeat, blockSizeElem>(dstPtr, srcPtr, rows, cols,
                                                                                      dstStride, srcStride);
            }
        }
    }
}

template <typename TileData, unsigned blockSizeElem>
__tf__ PTO_INTERNAL void TTransConvNCHW2NC1HWC0(typename TileData::TileDType __out__ dst,
                                                typename TileData::TileDType __in__ src, unsigned srcN, unsigned srcC,
                                                unsigned srcH, unsigned srcW, unsigned dstC0)
{
    using T = typename TileData::DType;
    __ubuf__ T *dstPtrOrig = (__ubuf__ T *)__cce_get_tile_ptr(dst);
    __ubuf__ T *srcPtrOrig = (__ubuf__ T *)__cce_get_tile_ptr(src);
    unsigned rows = dstC0;
    unsigned cols = srcH * srcW;
    unsigned dstC1 = (srcC + dstC0 - 1) / dstC0;
    unsigned nStride = dstC1 * dstC0 * srcH * srcW;
    unsigned cStride = dstC0 * srcH * srcW;
    if (cols * sizeof(T) % BLOCK_BYTE_SIZE != 0 || rows * sizeof(T) % BLOCK_BYTE_SIZE != 0) {
        TTransConvNCHW2NC1HWC0Unalign<T>(dstPtrOrig, srcPtrOrig, srcN, dstC1, rows, cols, nStride, cStride);
        return;
    }
    TTransConvNCHW2NC1HWC0Align<TileData, T, blockSizeElem>(dstPtrOrig, srcPtrOrig, srcN, srcC, srcH, srcW, dstC0);
}

template <typename T>
PTO_INTERNAL void ConvNC1HWC02C1HWNC0Unalign(__ubuf__ T *dst, __ubuf__ T *src, unsigned dstN, unsigned srcN,
                                             unsigned srcC1HW, unsigned srcC0)
{
    unsigned validCol = srcC0;
    unsigned validRow = srcC1HW;
    unsigned srcStride = srcC0;
    unsigned dstStride = dstN * srcC0;
    unsigned nStride = srcC1HW * srcC0;
    for (uint16_t num = 0; num < (uint16_t)srcN; num++) {
        __ubuf__ T *srcPtr = src + num * nStride;
        __ubuf__ T *dstPtr = dst + num * srcC0;
        for (uint16_t i = 0; i < (uint16_t)validRow; ++i) {
            for (uint16_t j = 0; j < (uint16_t)validCol; ++j) {
                dstPtr[i * dstStride + j] = srcPtr[i * srcStride + j];
            }
        }
    }
}

template <typename T>
PTO_INTERNAL void ConvNC1HWC02C1HWNC0Align(__ubuf__ T *dst, __ubuf__ T *src, unsigned dstN, unsigned srcN,
                                           unsigned srcC1HW, unsigned srcC0)
{
    unsigned validCol = srcC0;
    unsigned validRow = srcC1HW;
    unsigned srcStride = srcC0;
    unsigned dstStride = dstN * srcC0;
    constexpr unsigned nRepeatElem = REPEAT_BYTE / sizeof(T);
    uint16_t repeatTimes = CeilDivision(validCol, nRepeatElem);
    unsigned nStride = srcC1HW * srcC0;
    __VEC_SCOPE__
    {
        for (uint16_t num = 0; num < (uint16_t)srcN; num++) {
            __ubuf__ T *srcPtr = src + num * nStride;
            __ubuf__ T *dstPtr = dst + num * srcC0;
            RegTensor<T> vreg0;
            MaskReg preg;
            uint32_t sreg;
            constexpr auto distValue =
                std::integral_constant<::DistVST, static_cast<::DistVST>(GetDistVst<T, DistVST::DIST_NORM>())>();
            for (uint16_t i = 0; i < (uint16_t)validRow; ++i) {
                sreg = (uint32_t)validCol;
                for (uint16_t j = 0; j < (uint16_t)repeatTimes; ++j) {
                    preg = CreatePredicate<T>(sreg);
                    vlds(vreg0, srcPtr, i * srcStride + j * nRepeatElem, NORM);
                    vsts(vreg0, dstPtr, i * dstStride + j * nRepeatElem, distValue, preg);
                }
            }
        }
    }
}

template <typename TileData, unsigned blockSizeElem>
__tf__ PTO_INTERNAL void TTransConvNC1HWC02C1HWNC0(typename TileData::TileDType __out__ dst,
                                                   typename TileData::TileDType __in__ src, unsigned dstN,
                                                   unsigned srcN, unsigned srcC1HW, unsigned srcC0)
{
    using T = typename TileData::DType;
    __ubuf__ T *dstPtrOrig = (__ubuf__ T *)__cce_get_tile_ptr(dst);
    __ubuf__ T *srcPtrOrig = (__ubuf__ T *)__cce_get_tile_ptr(src);
    if (srcC0 * sizeof(T) % BLOCK_BYTE_SIZE == 0) {
        ConvNC1HWC02C1HWNC0Align<T>(dstPtrOrig, srcPtrOrig, dstN, srcN, srcC1HW, srcC0);
    } else {
        ConvNC1HWC02C1HWNC0Unalign<T>(dstPtrOrig, srcPtrOrig, dstN, srcN, srcC1HW, srcC0);
    }
}

template <typename TileDataDst, typename TileDataSrc, typename TileDataTmp>
PTO_INTERNAL void CheckConvTile(TileDataDst &dst, TileDataSrc &src, TileDataTmp &tmp)
{
#ifdef _DEBUG
    using T = typename TileDataSrc::DType;
    constexpr const int UB_SIZE = 262144; // 256*1024 B
    if (TileDataSrc::layout == Layout::NCHW && TileDataDst::layout == Layout::NC1HWC0) {
        unsigned srcN = src.GetShape(GlobalTensorDim::DIM_0);
        unsigned srcC = src.GetShape(GlobalTensorDim::DIM_1);
        unsigned srcH = src.GetShape(GlobalTensorDim::DIM_2);
        unsigned srcW = src.GetShape(GlobalTensorDim::DIM_3);
        unsigned dstN = dst.GetShape(GlobalTensorDim::DIM_0);
        unsigned dstC1 = dst.GetShape(GlobalTensorDim::DIM_1);
        unsigned dstH = dst.GetShape(GlobalTensorDim::DIM_2);
        unsigned dstW = dst.GetShape(GlobalTensorDim::DIM_3);
        unsigned dstC0 = dst.GetShape(GlobalTensorDim::DIM_4);
        unsigned srcSize = srcN * srcC * srcH * srcW;
        unsigned dstSize = dstN * dstC1 * dstC0 * dstH * dstW;
        unsigned tmpSize = TileDataTmp::Rows * TileDataTmp::Cols;
        PTO_ASSERT(srcH * srcW * sizeof(T) % BLOCK_BYTE_SIZE == 0, "expect align for H * W");
        PTO_ASSERT(srcN == dstN && srcH == dstH && srcW == dstW && dstC1 == (srcC + dstC0 - 1) / dstC0,
                   "expect same size for src and dst.");
        PTO_ASSERT((srcSize + dstSize + tmpSize) * sizeof(T) < UB_SIZE, "ERROR: memory usage exceeds UB limit!");
    } else if (TileDataSrc::layout == Layout::NC1HWC0 && TileDataDst::layout == Layout::FRACTAL_Z) {
        unsigned srcN = src.GetShape(GlobalTensorDim::DIM_0);
        unsigned srcC1 = src.GetShape(GlobalTensorDim::DIM_1);
        unsigned srcH = src.GetShape(GlobalTensorDim::DIM_2);
        unsigned srcW = src.GetShape(GlobalTensorDim::DIM_3);
        unsigned srcC0 = src.GetShape(GlobalTensorDim::DIM_4);
        unsigned dstC1HW = dst.GetShape(GlobalTensorDim::DIM_0);
        unsigned dstN1 = dst.GetShape(GlobalTensorDim::DIM_1);
        unsigned dstN0 = dst.GetShape(GlobalTensorDim::DIM_2);
        unsigned dstC0 = dst.GetShape(GlobalTensorDim::DIM_3);
        unsigned srcSize = srcN * srcC1 * srcC0 * srcH * srcW;
        unsigned dstSize = dstC1HW * dstN1 * dstN0 * dstC0;
        unsigned tmpSize = TileDataTmp::Rows * TileDataTmp::Cols;
        PTO_ASSERT(srcC1 * srcH * srcW == dstC1HW && srcC0 == dstC0 && dstN1 == (srcN + dstN0 - 1) / dstN0,
                   "expect same size for src and dst.");
        PTO_ASSERT((srcSize + dstSize + tmpSize) * sizeof(T) < UB_SIZE, "ERROR: memory usage exceeds UB limit!");
    }
#endif
}

template <typename TileDataDst, typename TileDataSrc, typename TileDataTmp>
PTO_INTERNAL void TTRANS_IMPL(TileDataDst &dst, TileDataSrc &src, TileDataTmp &tmp)
{
    using T = typename TileDataSrc::DType;
    using U = typename TileDataDst::DType;
    static_assert(sizeof(T) == 4 || sizeof(T) == 2 || sizeof(T) == 1, "Fix: TTRANS has unsupported data type.");
    static_assert(sizeof(T) == sizeof(U), "Fix: TTRANS has inconsistent input and output data types.");
    if constexpr (is_conv_tile_v<TileDataSrc>) {
        constexpr unsigned blockSizeElem = BLOCK_BYTE_SIZE / sizeof(T);
        CheckConvTile<TileDataDst, TileDataSrc, TileDataTmp>(dst, src, tmp);
        if constexpr (TileDataSrc::layout == Layout::NCHW && TileDataDst::layout == Layout::NC1HWC0) {
            unsigned srcN = src.GetShape(GlobalTensorDim::DIM_0);
            unsigned srcC = src.GetShape(GlobalTensorDim::DIM_1);
            unsigned srcH = src.GetShape(GlobalTensorDim::DIM_2);
            unsigned srcW = src.GetShape(GlobalTensorDim::DIM_3);
            unsigned dstC0 = dst.GetShape(GlobalTensorDim::DIM_4);
            TTransConvNCHW2NC1HWC0<TileDataSrc, blockSizeElem>(dst.data(), src.data(), srcN, srcC, srcH, srcW, dstC0);
        } else if (TileDataSrc::layout == Layout::NC1HWC0 && TileDataDst::layout == Layout::FRACTAL_Z) {
            unsigned srcN = src.GetShape(GlobalTensorDim::DIM_0);
            unsigned srcC1 = src.GetShape(GlobalTensorDim::DIM_1);
            unsigned srcH = src.GetShape(GlobalTensorDim::DIM_2);
            unsigned srcW = src.GetShape(GlobalTensorDim::DIM_3);
            unsigned srcC0 = src.GetShape(GlobalTensorDim::DIM_4);
            unsigned dstN1 = dst.GetShape(GlobalTensorDim::DIM_1);
            unsigned dstN0 = dst.GetShape(GlobalTensorDim::DIM_2);
            TTransConvNC1HWC02C1HWNC0<TileDataSrc, blockSizeElem>(dst.data(), src.data(), dstN0 * dstN1, srcN,
                                                                  srcC1 * srcH * srcW, srcC0);
        }
        return;
    } else {
        static_assert(TileDataSrc::isRowMajor, "Fix: TTRANS has not supported layout type.");
        static_assert(TileDataSrc::Cols * sizeof(T) % 32 == 0, "Fix: TTRANS has inconsistent input shape.");
        static_assert(TileDataDst::Cols * sizeof(U) % 32 == 0, "Fix: TTRANS has inconsistent output shape.");

        constexpr unsigned elementsPerRepeat = REPEAT_BYTE / sizeof(T);
        constexpr unsigned blockSizeElem = BLOCK_BYTE_SIZE / sizeof(T);

        unsigned numRows = (unsigned)src.GetValidRow();
        unsigned numCols = (unsigned)src.GetValidCol();
        TTransTile<TileDataDst, TileDataSrc, elementsPerRepeat, blockSizeElem>(dst.data(), src.data(), numRows,
                                                                               numCols);
    }
}
} // namespace pto

#endif // TTRANS_HPP