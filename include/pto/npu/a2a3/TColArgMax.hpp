/**
Copyright (c) 2025 Huawei Technologies Co., Ltd.
This program is free software, you can redistribute it and/or modify it under the terms and conditions of
CANN Open Software License Agreement Version 2.0 (the "License").
Please refer to the License for details. You may not use this file except in compliance with the License.
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
See LICENSE in the root of the software repository for the full text of the License.
*/

#ifndef TCOLARGMAX_HPP
#define TCOLARGMAX_HPP

#include <pto/common/constants.hpp>
#include <pto/common/utils.hpp>

namespace pto {
template <typename TileDataOut, typename TileDataIn, typename TileDataTmp>
PTO_INTERNAL void TColArgMaxCheck(unsigned srcValidRow, unsigned srcValidCol, unsigned dstValidRow,
                                  unsigned dstValidCol)
{
    static_assert(TileDataIn::ValidCol == 1 || TileDataIn::ValidCol == -1,
                  "Fix: TCOLARGMAX Src ValidCol must be 1 or -1");
    static_assert(
        std::is_same_v<typename TileDataIn::DType, uint32_t> || std::is_same_v<typename TileDataIn::DType, uint16_t> ||
            std::is_same_v<typename TileDataIn::DType, half> || std::is_same_v<typename TileDataIn::DType, float>,
        "Fix: TCOLARGMAX input data type must be f16/u16/f32/u32");
    static_assert(TileDataIn::Loc == pto::TileType::Vec, "Fix: TCOLARGMAX Src TileType must be Vec Tile!");
    static_assert(TileDataOut::Loc == pto::TileType::Vec, "Fix: TCOLARGMAX Dst TileType must be Vec Tile!");
    static_assert(TileDataIn::SFractal == SLayout::NoneBox, "Fix: TCOLARGMAX only support Nd or Dn fractal Tile");
    static_assert(TileDataOut::isRowMajor && TileDataOut::SFractal == SLayout::NoneBox,
                  "Fix: TCOLARGMAX only support Nd fractal Tile");
    static_assert(
        std::is_same_v<typename TileDataOut::DType, uint32_t> || std::is_same_v<typename TileDataOut::DType, int32_t>,
        "Fix: TCOLARGMAX output data type must be s32 or u32.");
    static_assert(std::is_same_v<typename TileDataIn::DType, typename TileDataTmp::DType>,
                  "Fix: TCOLARGMAX input type must be consistent with the tmp type");
    PTO_ASSERT(srcValidRow != 0 && srcValidCol != 0,
               "Fix: TCOLARGMAX input shape is invalid, validCol or validRow is 0.");
    PTO_ASSERT(dstValidRow != 1, "Fix: TCOLARGMAX output validRow must be 1");
    PTO_ASSERT(srcValidCol != dstValidCol,
               "Fix: TCOLARGMAX input validCol must be consistent with the output validCol");
}

template <typename TileDataOut, typename TileDataIn, typename TileDataTmp>
__tf__ PTO_INTERNAL void TColArgMax16(typename TileDataOut::TileDType __out__ dst,
                                      typename TileDataIn::TileDType __in__ src,
                                      typename TileDataTmp::TileDType __in__ tmp, unsigned srcValidRow,
                                      unsigned srcValidCol)
{
    using TOUT = typename TileDataOut::DType;
    using T = typename TileDataIn::DType;
    __ubuf__ TOUT *dstPtr = (__ubuf__ TOUT *)__cce_get_tile_ptr(dst);
    __ubuf__ T *srcPtr = (__ubuf__ T *)__cce_get_tile_ptr(src);
    __ubuf__ T *tmpPtr = (__ubuf__ T *)__cce_get_tile_ptr(tmp);

    constexpr uint32_t srcRowStride = TileDataOut::Cols;
    constexpr uint32_t elemPerRpt = REPEAT_BYTE / sizeof(T);
    constexpr uint32_t elemPerBlock = BLOCK_BYTE_SIZE / sizeof(T);
    uint16_t numLoop = srcValidCol / elemPerRpt;
    uint16_t remainAfterLoop = srcValidCol % elemPerRpt;
    uint32_t tmpGapEles = numLoop > 0 ? elemPerRpt : CeilDivision(srcValidCol, elemPerBlock) * elemPerBlock;

    if (numLoop > 0) {
        for (uint16_t j = 0; j < numLoop; j++) {
            pipe_barrier(PIPE_V);

            vector_dup((__ubuf__ int16_t *)tmpPtr, 0, 1, 1, 1, 0, 0);                  // cur index
            vector_dup((__ubuf__ int16_t *)tmpPtr + 2 * tmpGapEles, 0, 1, 1, 1, 0, 0); // argmin index

            copy_ubuf_to_ubuf(tmpPtr + tmpGapEles, srcPtr + j * elemPerRpt, 0, 1, 8, 0, 0); // min elements
            for (uint16_t i = 1; i < srcValidRow; i++) {
                pipe_barrier(PIPE_V);

                vadds((__ubuf__ int16_t *)tmpPtr, (__ubuf__ int16_t *)tmpPtr, 1, 1, 1, 1, 0, 0);
                vcmp_ge((__ubuf__ half *)tmpPtr + tmpGapEles,
                        (__ubuf__ half *)srcPtr + i * srcRowStride + j * elemPerRpt, 1, 1, 1, 1, 0, 0, 0);
                pipe_barrier(PIPE_V);
                vsel((__ubuf__ half *)tmpPtr + 2 * tmpGapEles, (__ubuf__ half *)tmpPtr + 2 * tmpGapEles,
                     (__ubuf__ half *)tmpPtr, 1, 1, 1, 1, 0, 0, 0, 0);
                vmax((__ubuf__ half *)tmpPtr + tmpGapEles, (__ubuf__ half *)tmpPtr + tmpGapEles,
                     (__ubuf__ half *)srcPtr + i * srcRowStride + j * elemPerRpt, 1, 1, 1, 1, 0, 0, 0);
            }
            pipe_barrier(PIPE_V);

            vconv_s162f16a((__ubuf__ half *)tmpPtr + 2 * tmpGapEles, (__ubuf__ int16_t *)tmpPtr + 2 * tmpGapEles, 1, 1,
                           1, 0, 0);
            pipe_barrier(PIPE_V);
            vconv_f162s32a((__ubuf__ int32_t *)dstPtr + j * elemPerRpt, (__ubuf__ half *)tmpPtr + 2 * tmpGapEles, 2, 1,
                           1, 8, 4);
        }
    }
    if (remainAfterLoop > 0) {
        set_mask_count();
        vector_dup(tmpPtr, 0, 1, 1, 1, 0, 0);
        vector_dup((__ubuf__ int16_t *)tmpPtr + 2 * tmpGapEles, 0, 1, 1, 1, 0, 0);
        copy_ubuf_to_ubuf(tmpPtr + tmpGapEles, srcPtr + numLoop * elemPerRpt, 0, 1, 8, 0, 0); // max elements
        for (uint16_t i = 1; i < srcValidRow; i++) {
            set_vector_mask(0, remainAfterLoop);
            pipe_barrier(PIPE_V);
            vadds((__ubuf__ int16_t *)tmpPtr, (__ubuf__ int16_t *)tmpPtr, 1, 1, 1, 1, 0, 0);
            vcmp_ge((__ubuf__ half *)tmpPtr + tmpGapEles,
                    (__ubuf__ half *)srcPtr + i * srcRowStride + numLoop * elemPerRpt, 1, 1, 1, 1, 0, 0, 0);
            pipe_barrier(PIPE_V);
            vsel((__ubuf__ half *)tmpPtr + 2 * tmpGapEles, (__ubuf__ half *)tmpPtr + 2 * tmpGapEles,
                 (__ubuf__ half *)tmpPtr, 1, 1, 1, 1, 0, 0, 0, 0);
            vmax((__ubuf__ half *)tmpPtr + tmpGapEles, (__ubuf__ half *)tmpPtr + tmpGapEles,
                 (__ubuf__ half *)srcPtr + i * srcRowStride + numLoop * elemPerRpt, 1, 1, 1, 1, 0, 0, 0);
        }
        pipe_barrier(PIPE_V);

        vconv_s162f16a((__ubuf__ half *)tmpPtr + 2 * tmpGapEles, (__ubuf__ int16_t *)tmpPtr + 2 * tmpGapEles, 1, 1, 1,
                       0, 0);
        pipe_barrier(PIPE_V);
        vconv_f162s32a((__ubuf__ int32_t *)dstPtr + numLoop * elemPerRpt, (__ubuf__ half *)tmpPtr + 2 * tmpGapEles, 2,
                       1, 1, 8, 4);
        set_mask_norm();
        set_vector_mask(-1, -1);
    }
}

template <typename TileDataOut, typename TileDataIn, typename TileDataTmp>
__tf__ PTO_INTERNAL void TColArgMax32(typename TileDataOut::TileDType __out__ dst,
                                      typename TileDataIn::TileDType __in__ src,
                                      typename TileDataTmp::TileDType __in__ tmp, unsigned srcValidRow,
                                      unsigned srcValidCol)
{
    using TOUT = typename TileDataOut::DType;
    using T = typename TileDataIn::DType;
    __ubuf__ TOUT *dstPtr = (__ubuf__ TOUT *)__cce_get_tile_ptr(dst);
    __ubuf__ T *srcPtr = (__ubuf__ T *)__cce_get_tile_ptr(src);
    __ubuf__ T *tmpPtr = (__ubuf__ T *)__cce_get_tile_ptr(tmp);

    constexpr uint32_t srcRowStride = TileDataOut::Cols;
    constexpr uint32_t elemPerRpt = REPEAT_BYTE / sizeof(T);
    constexpr uint32_t elemPerBlock = BLOCK_BYTE_SIZE / sizeof(T);
    uint16_t numLoop = srcValidCol / elemPerRpt;
    uint16_t remainAfterLoop = srcValidCol % elemPerRpt;
    uint32_t tmpGapEles = numLoop > 0 ? elemPerRpt : CeilDivision(srcValidCol, elemPerBlock) * elemPerBlock;

    if (numLoop > 0) {
        for (uint16_t j = 0; j < numLoop; j++) {
            vector_dup(dstPtr + j * elemPerRpt, 0, 1, 1, 1, 0, 0);                          // argmin index
            vector_dup(tmpPtr, 0, 1, 1, 1, 0, 0);                                           // cur index
            copy_ubuf_to_ubuf(tmpPtr + tmpGapEles, srcPtr + j * elemPerRpt, 0, 1, 8, 0, 0); // min elements
            for (uint16_t i = 1; i < srcValidRow; i++) {
                pipe_barrier(PIPE_V);
                vadds((__ubuf__ int32_t *)tmpPtr, (__ubuf__ int32_t *)tmpPtr, 1, 1, 1, 1, 0, 0);
                vcmp_ge((__ubuf__ float *)tmpPtr + tmpGapEles,
                        (__ubuf__ float *)srcPtr + i * srcRowStride + j * elemPerRpt, 1, 1, 1, 1, 0, 0, 0);
                pipe_barrier(PIPE_V);
                vsel((__ubuf__ float *)dstPtr + j * elemPerRpt, (__ubuf__ float *)dstPtr + j * elemPerRpt,
                     (__ubuf__ float *)tmpPtr, 1, 1, 1, 1, 0, 0, 0, 0);
                vmax((__ubuf__ float *)tmpPtr + tmpGapEles, (__ubuf__ float *)tmpPtr + tmpGapEles,
                     (__ubuf__ float *)srcPtr + i * srcRowStride + j * elemPerRpt, 1, 1, 1, 1, 0, 0, 0);
            }
            pipe_barrier(PIPE_V);
        }
    }
    if (remainAfterLoop > 0) {
        set_mask_count();
        vector_dup(dstPtr + numLoop * elemPerRpt, 0, 1, 1, 1, 0, 0);
        vector_dup(tmpPtr, 0, 1, 1, 1, 0, 0);
        copy_ubuf_to_ubuf(tmpPtr + tmpGapEles, srcPtr + numLoop * elemPerRpt, 0, 1, 8, 0, 0);
        for (uint16_t i = 1; i < srcValidRow; i++) {
            set_vector_mask(0, remainAfterLoop);
            pipe_barrier(PIPE_V);
            vadds((__ubuf__ int32_t *)tmpPtr, (__ubuf__ int32_t *)tmpPtr, 1, 1, 1, 1, 0, 0);
            vcmp_ge((__ubuf__ float *)tmpPtr + tmpGapEles,
                    (__ubuf__ float *)srcPtr + i * srcRowStride + numLoop * elemPerRpt, 1, 1, 1, 1, 0, 0, 0);
            pipe_barrier(PIPE_V);
            vsel((__ubuf__ float *)dstPtr + numLoop * elemPerRpt, (__ubuf__ float *)dstPtr + numLoop * elemPerRpt,
                 (__ubuf__ float *)tmpPtr, 1, 1, 1, 1, 0, 0, 0, 0);
            vmax((__ubuf__ float *)tmpPtr + tmpGapEles, (__ubuf__ float *)tmpPtr + tmpGapEles,
                 (__ubuf__ float *)srcPtr + i * srcRowStride + numLoop * elemPerRpt, 1, 1, 1, 1, 0, 0, 0);
        }
        set_mask_norm();
        set_vector_mask(-1, -1);
        pipe_barrier(PIPE_V);
    }
}

template <typename TileDataOut, typename TileDataIn, typename TileDataTmp>
PTO_INTERNAL void TCOLARGMAX_IMPL(TileDataOut &dst, TileDataIn &src, TileDataTmp &tmp)
{
    unsigned dstValidRow = dst.GetValidRow();
    unsigned dstValidCol = dst.GetValidCol();
    unsigned srcValidRow = src.GetValidRow();
    unsigned srcValidCol = src.GetValidCol();
    TColArgMaxCheck<TileDataOut, TileDataIn, TileDataTmp>(srcValidRow, srcValidCol, dstValidRow, dstValidCol);

    if constexpr (sizeof(typename TileDataIn::DType) == 2) {
        TColArgMax16<TileDataOut, TileDataIn, TileDataTmp>(dst.data(), src.data(), tmp.data(), srcValidRow,
                                                           srcValidCol);
    } else if constexpr (sizeof(typename TileDataIn::DType) == 4) {
        TColArgMax32<TileDataOut, TileDataIn, TileDataTmp>(dst.data(), src.data(), tmp.data(), srcValidRow,
                                                           srcValidCol);
    }
}
} // namespace pto
#endif