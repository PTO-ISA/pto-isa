/**
Copyright (c) 2026 Huawei Technologies Co., Ltd.
This program is free software, you can redistribute it and/or modify it under the terms and conditions of
CANN Open Software License Agreement Version 2.0 (the "License").
Please refer to the License for details. You may not use this file except in compliance with the License.
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
See LICENSE in the root of the software repository for the full text of the License.
*/

#ifndef TCONCAT_HPP
#define TCONCAT_HPP

#include <pto/common/constants.hpp>
#include <pto/common/utils.hpp>
#include <pto/npu/a5/common.hpp>
#include <pto/npu/a5/utils.hpp>
#include <pto/common/debug.h>

namespace pto {

template <typename T>
struct IndexConcat {
    static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4, "Fix: Unsupported DType size for index vector.");
    using Scalar = std::conditional_t<sizeof(T) == 4, int32_t, std::conditional_t<sizeof(T) == 2, int16_t, int8_t>>;
    using type = RegTensor<Scalar>;
};

template <typename TileDataDst, typename TileDataSrc0, typename TileDataSrc1, unsigned ElementsPerRepeat>
__tf__ PTO_INTERNAL OP_NAME(TCONCAT)
    OP_TYPE(element_wise) void TConcat(typename TileDataDst::TileDType __out__ dst,
                                       typename TileDataSrc0::TileDType __in__ src0,
                                       typename TileDataSrc1::TileDType __in__ src1, unsigned validRows,
                                       unsigned validCols0, unsigned validCols1)
{
    using T = typename TileDataDst::DType;
    __ubuf__ T *dstPtr = (__ubuf__ T *)__cce_get_tile_ptr(dst);
    __ubuf__ T *src0Ptr = (__ubuf__ T *)__cce_get_tile_ptr(src0);
    __ubuf__ T *src1Ptr = (__ubuf__ T *)__cce_get_tile_ptr(src1);

    unsigned validColsDst = validCols0 + validCols1;
    unsigned repeatTimes0 = CeilDivision(validCols0, ElementsPerRepeat);
    unsigned repeatTimes1 = CeilDivision(validCols1, ElementsPerRepeat);

    using IndexScalar = typename IndexConcat<T>::Scalar;
    typename IndexConcat<T>::type vreg_idx;
    using UnsignedIndexScalar = typename std::make_unsigned<IndexScalar>::type;

    __VEC_SCOPE__
    {
        RegTensor<T> vreg_0;
        RegTensor<T> vreg_1;
        MaskReg preg;
        constexpr auto distValue =
            std::integral_constant<::DistVST, static_cast<::DistVST>(GetDistVst<T, DistVST::DIST_NORM>())>();

        for (uint16_t i = 0; i < (uint16_t)validRows; ++i) {
            uint32_t sreg0 = validCols0;
            for (uint16_t j = 0; j < (uint16_t)repeatTimes0; ++j) {
                preg = CreatePredicate<T>(sreg0);
                vlds(vreg_0, src0Ptr, i * TileDataSrc0::RowStride + j * ElementsPerRepeat, NORM);
                vsts(vreg_0, dstPtr, i * TileDataDst::RowStride + j * ElementsPerRepeat, distValue, preg);
            }

            mem_bar(VST_VLD);
            uint32_t sreg1 = validCols1;
            for (uint16_t j = 0; j < (uint16_t)repeatTimes1; ++j) {
                preg = CreatePredicate<T>(sreg1);
                vlds(vreg_1, src1Ptr, i * TileDataSrc1::RowStride + j * ElementsPerRepeat, NORM);
                vci((RegTensor<IndexScalar> &)vreg_idx,
                    (IndexScalar)(i * TileDataDst::RowStride + validCols0 + j * ElementsPerRepeat), INC_ORDER);
                vscatter(vreg_1, dstPtr, (RegTensor<UnsignedIndexScalar> &)vreg_idx, preg);
            }
        }
    }
    return;
}

template <typename TileDataDst, typename TileDataSrc0, typename TileDataSrc1>
PTO_INTERNAL void TConcatCheck(const TileDataDst &dst, const TileDataSrc0 &src0, const TileDataSrc1 &src1)
{
    using T = typename TileDataDst::DType;
    static_assert(std::is_same_v<T, int32_t> || std::is_same_v<T, uint32_t> || std::is_same_v<T, float> ||
                      std::is_same_v<T, int16_t> || std::is_same_v<T, uint16_t> || std::is_same_v<T, half> ||
                      std::is_same_v<T, bfloat16_t> || std::is_same_v<T, uint8_t> || std::is_same_v<T, int8_t>,
                  "Fix: TCONCAT has invalid data type.");
    static_assert(TileDataDst::isRowMajor && TileDataSrc0::isRowMajor && TileDataSrc1::isRowMajor,
                  "Fix: TCONCAT only support row major layout.");
    static_assert(std::is_same_v<T, typename TileDataSrc0::DType> && std::is_same_v<T, typename TileDataSrc1::DType>,
                  "Fix: TCONCAT input tile src0, src1 and dst tile data type mismatch.");
    unsigned validRows = dst.GetValidRow();
    unsigned validCols0 = src0.GetValidCol();
    unsigned validCols1 = src1.GetValidCol();
    unsigned validColsDst = dst.GetValidCol();
    PTO_ASSERT(src0.GetValidRow() == validRows,
               "Fix: TCONCAT input tile src0 valid row mismatch with output tile dst row.");
    PTO_ASSERT(src1.GetValidRow() == validRows,
               "Fix: TCONCAT input tile src1 valid row mismatch with output tile dst row.");
    PTO_ASSERT(validCols0 + validCols1 == validColsDst,
               "Fix: TCONCAT output tile valid col should be equal to sum of input tiles cols.");
}

template <typename TileDataDst, typename TileDataSrc0, typename TileDataSrc1>
PTO_INTERNAL void TCONCAT_IMPL(TileDataDst &dst, TileDataSrc0 &src0, TileDataSrc1 &src1)
{
    using T = typename TileDataDst::DType;
    TConcatCheck<TileDataDst, TileDataSrc0, TileDataSrc1>(dst, src0, src1);
    constexpr unsigned elementsPerRepeat = REPEAT_BYTE / sizeof(T);

    TConcat<TileDataDst, TileDataSrc0, TileDataSrc1, elementsPerRepeat>(
        dst.data(), src0.data(), src1.data(), dst.GetValidRow(), src0.GetValidCol(), src1.GetValidCol());
}
} // namespace pto
#endif
