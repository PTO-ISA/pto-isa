/**
Copyright (c) 2026 Huawei Technologies Co., Ltd.
This program is free software, you can redistribute it and/or modify it under the terms and conditions of
CANN Open Software License Agreement Version 2.0 (the "License").
Please refer to the License for details. You may not use this file except in compliance with the License.
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
See LICENSE in the root of the software repository for the full text of the License.
*/

#ifndef PTO_GPU_SM121_TMATMUL_HPP
#define PTO_GPU_SM121_TMATMUL_HPP

#include <type_traits>
#include <pto/common/type.hpp>
#include "pto/gpu/common/tile_offsets.hpp"

namespace pto::gpu::sm121 {

template <typename T>
PTO_INTERNAL float ToAccumFloat(T value)
{
    return static_cast<float>(value);
}

template <>
PTO_INTERNAL float ToAccumFloat<half>(half value)
{
    return __half2float(value);
}

template <>
PTO_INTERNAL float ToAccumFloat<bfloat16_t>(bfloat16_t value)
{
    return __bfloat162float(value);
}

PTO_INTERNAL float InlinePtxFma(float a, float b, float c)
{
    float out;
    asm volatile("fma.rn.f32 %0, %1, %2, %3;" : "=f"(out) : "f"(a), "f"(b), "f"(c));
    return out;
}

template <typename TileAcc, typename TileLeft, typename TileRight>
PTO_INTERNAL bool TryInlinePtxTMATMUL(TileAcc &cMatrix, TileLeft &aMatrix, TileRight &bMatrix)
{
    using CType = typename TileAcc::DType;
    using AType = typename TileLeft::DType;
    using BType = typename TileRight::DType;

    constexpr bool supportedTypes = std::is_same_v<CType, float> &&
                                    ((std::is_same_v<AType, float> && std::is_same_v<BType, float>) ||
                                     (std::is_same_v<AType, half> && std::is_same_v<BType, half>) ||
                                     (std::is_same_v<AType, bfloat16_t> && std::is_same_v<BType, bfloat16_t>));
    if constexpr (!supportedTypes) {
        return false;
    }

    const uint16_t m = aMatrix.GetValidRow();
    const uint16_t k = aMatrix.GetValidCol();
    const uint16_t n = bMatrix.GetValidCol();
    if (k != bMatrix.GetValidRow()) {
        return false;
    }

#pragma unroll 1
    for (uint16_t row = 0; row < m; ++row) {
        for (uint16_t col = 0; col < n; ++col) {
            float acc = 0.0f;
#pragma unroll 4
            for (uint16_t kk = 0; kk < k; ++kk) {
                const std::size_t aIdx = gpu::GetTileElementOffset<TileLeft>(row, kk);
                const std::size_t bIdx = gpu::GetTileElementOffset<TileRight>(kk, col);
                acc = InlinePtxFma(ToAccumFloat(aMatrix.data()[aIdx]), ToAccumFloat(bMatrix.data()[bIdx]), acc);
            }
            cMatrix.data()[gpu::GetTileElementOffset<TileAcc>(row, col)] = acc;
        }
    }
    return true;
}

} // namespace pto::gpu::sm121

#endif
