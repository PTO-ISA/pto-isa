/**
Copyright (c) 2026 Huawei Technologies Co., Ltd.
This program is free software, you can redistribute it and/or modify it under the terms and conditions of
CANN Open Software License Agreement Version 2.0 (the "License").
Please refer to the License for details. You may not use this file except in compliance with the License.
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
See LICENSE in the root of the software repository for the full text of the License.
*/

#ifndef THISTOGRAM_HPP
#define THISTOGRAM_HPP

#include <pto/common/constants.hpp>
#include <pto/common/utils.hpp>
#include <pto/npu/a5/common.hpp>
#include <pto/npu/a5/utils.hpp>

namespace pto {

PTO_INTERNAL void histogram_b16i_b32o(vector_u8 &vb8_src, vector_u16 &vb16_bin_n0, vector_u16 &vb16_bin_n1,
                                      vector_u32 &vb32_bin_n0_even_inc, vector_u32 &vb32_bin_n0_odd_inc,
                                      vector_u32 &vb32_bin_n1_even_inc, vector_u32 &vb32_bin_n1_odd_inc,
                                      vector_bool &preg_b8_0, vector_bool &preg_b8_1, vector_bool &preg_b16,
                                      vector_bool &preg_b32)
{
    vector_u32 vb32_bin_n0_even, vb32_bin_n0_odd, vb32_bin_n1_even, vb32_bin_n1_odd;
    chistv2(vb16_bin_n0, vb8_src, preg_b8_0, Bin_N0);
    chistv2(vb16_bin_n1, vb8_src, preg_b8_1, Bin_N1);
    vcvt(vb32_bin_n0_even, vb16_bin_n0, preg_b16, PART_EVEN);
    vcvt(vb32_bin_n0_odd, vb16_bin_n0, preg_b16, PART_ODD);
    vcvt(vb32_bin_n1_even, vb16_bin_n1, preg_b16, PART_EVEN);
    vcvt(vb32_bin_n1_odd, vb16_bin_n1, preg_b16, PART_ODD);
    vadd(vb32_bin_n0_even_inc, vb32_bin_n0_even_inc, vb32_bin_n0_even, preg_b32, MODE_ZEROING);
    vadd(vb32_bin_n0_odd_inc, vb32_bin_n0_odd_inc, vb32_bin_n0_odd, preg_b32, MODE_ZEROING);
    vadd(vb32_bin_n1_even_inc, vb32_bin_n1_even_inc, vb32_bin_n1_even, preg_b32, MODE_ZEROING);
    vadd(vb32_bin_n1_odd_inc, vb32_bin_n1_odd_inc, vb32_bin_n1_odd, preg_b32, MODE_ZEROING);
    vbr(vb16_bin_n0, 0);
    vbr(vb16_bin_n1, 0);
}

template <typename TileDst, typename TileSrc, typename TileIdx, bool isMSB = true>
__tf__ PTO_INTERNAL void THistogram(typename TileDst::TileDType __out__ bin_count,
                                    typename TileSrc::TileDType __in__ scores, typename TileIdx::TileDType __in__ idx,
                                    unsigned validRows, unsigned validCols)
{
    __ubuf__ typename TileDst::DType *dstPtr = (__ubuf__ typename TileDst::DType *)__cce_get_tile_ptr(bin_count);
    __ubuf__ typename TileSrc::DType *srcPtr = (__ubuf__ typename TileSrc::DType *)__cce_get_tile_ptr(scores);
    __ubuf__ typename TileIdx::DType *idxPtr = (__ubuf__ typename TileIdx::DType *)__cce_get_tile_ptr(idx);
    constexpr unsigned Type_coeff = sizeof(uint16_t) / sizeof(uint8_t); // input is of type b16, load of type b8
    __VEC_SCOPE__
    {
        vector_u16 vb16_BIN_N0, vb16_BIN_N1;
        vector_u32 vb32_BIN_N0_even, vb32_BIN_N0_odd, vb32_BIN_N1_even, vb32_BIN_N1_odd;
        vector_u8 vb8_src_MSB, vb8_src_LSB, vb8_idx;
        vector_bool preg_idx;
        constexpr unsigned ElemPerRepeatB8 = REPEAT_BYTE / sizeof(uint8_t);
        unsigned repeatTimesPerRow = CeilDivision(validCols, ElemPerRepeatB8);
        vbr(vb16_BIN_N0, 0);
        vbr(vb16_BIN_N1, 0);
        vector_bool preg_all_b16 = pset_b16(PAT_ALL);
        vector_bool preg_all_b32 = pset_b32(PAT_ALL);
        __ubuf__ uint32_t *dstPtr128ElemShift = (__ubuf__ uint32_t *)dstPtr + 128;
        for (uint16_t r = 0; r < (uint16_t)validRows; ++r) {
            vlds(vb8_idx, idxPtr, 1, BRC_B8, POST_UPDATE);
            uint32_t sreg_even = validCols, sreg_odd = validCols;
            vbr(vb32_BIN_N0_even, 0);
            vbr(vb32_BIN_N0_odd, 0);
            vbr(vb32_BIN_N1_even, 0);
            vbr(vb32_BIN_N1_odd, 0);
            for (uint16_t c = 0; c < (uint16_t)repeatTimesPerRow; ++c) {
                vector_bool preg_b8_0 = CreatePredicate<uint8_t>(sreg_even);
                vector_bool preg_b8_1 = CreatePredicate<uint8_t>(sreg_odd);
                vlds((vector_u8 &)vb8_src_LSB, (vector_u8 &)vb8_src_MSB, (__ubuf__ uint8_t *&)srcPtr,
                     Type_coeff * (r * TileSrc::Cols + c * ElemPerRepeatB8), DINTLV_B8);
                if constexpr (isMSB) {
                    histogram_b16i_b32o(vb8_src_MSB, vb16_BIN_N0, vb16_BIN_N1, (vector_u32 &)vb32_BIN_N0_even,
                                        (vector_u32 &)vb32_BIN_N0_odd, (vector_u32 &)vb32_BIN_N1_even,
                                        (vector_u32 &)vb32_BIN_N1_odd, preg_b8_0, preg_b8_1, preg_all_b16,
                                        preg_all_b32);
                } else {
                    vcmp_eq(preg_idx, (vector_u8 &)vb8_src_MSB, vb8_idx, preg_b8_0);
                    histogram_b16i_b32o(vb8_src_LSB, vb16_BIN_N0, vb16_BIN_N1, (vector_u32 &)vb32_BIN_N0_even,
                                        (vector_u32 &)vb32_BIN_N0_odd, (vector_u32 &)vb32_BIN_N1_even,
                                        (vector_u32 &)vb32_BIN_N1_odd, preg_idx, preg_idx, preg_all_b16, preg_all_b32);
                }
            }
            vsts((vector_u32 &)vb32_BIN_N0_even, (vector_u32 &)vb32_BIN_N0_odd, (__ubuf__ uint32_t *&)dstPtr, 256 * r,
                 INTLV_B32, preg_all_b32);
            vsts((vector_u32 &)vb32_BIN_N1_even, (vector_u32 &)vb32_BIN_N1_odd,
                 (__ubuf__ uint32_t *&)dstPtr128ElemShift, 256 * r, INTLV_B32, preg_all_b32);
        }
    }
}

template <bool MSBorLSB = true, typename TileDst, typename TileSrc, typename TileIdx>
PTO_INTERNAL void THISTOGRAM_IMPL(TileDst &dst, TileSrc &src, TileIdx &idx)
{
    using SrcT = typename TileSrc::DType;
    using DstT = typename TileDst::DType;
    using IdxT = typename TileIdx::DType;
    static_assert(std::is_same<SrcT, uint16_t>::value, "Fix: THISTOGRAM source must be uint16_t.");
    static_assert(std::is_same<DstT, uint32_t>::value, "Fix: THISTOGRAM destination must be uint32_t.");
    static_assert(std::is_same<IdxT, uint8_t>::value, "Fix: THISTOGRAM index must be uint8_t.");
    static_assert(TileSrc::isRowMajor, "Fix: THISTOGRAM only supports row major layout.");
    static_assert(TileDst::isRowMajor, "Fix: THISTOGRAM only supports row major layout.");
    static_assert(TileIdx::isRowMajor, "Fix: THISTOGRAM only supports row major layout.");

    THistogram<TileDst, TileSrc, TileIdx, MSBorLSB>(dst.data(), src.data(), idx.data(), src.GetValidRow(),
                                                    src.GetValidCol());
}

} // namespace pto
#endif
