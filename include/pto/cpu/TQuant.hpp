/**
Copyright (c) 2026 Huawei Technologies Co., Ltd.
This program is free software, you can redistribute it and/or modify it under the terms and conditions of
CANN Open Software License Agreement Version 2.0 (the "License").
Please refer to the License for details. You may not use this file except in compliance with the License.
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
See LICENSE in the root of the software repository for the full text of the License.
*/

#ifndef PTO_CPU_TQUANT_HPP
#define PTO_CPU_TQUANT_HPP

#include <algorithm>
#include <type_traits>

#include "pto/common/pto_tile.hpp"
#include "pto/cpu/TCvt.hpp"
#include "pto/cpu/tile_offsets.hpp"

namespace pto {

template <typename TilePara>
PTO_INTERNAL typename TilePara::DType LoadRowQuantParam(TilePara &src, std::size_t rowIndex)
{
    const std::size_t vr = static_cast<std::size_t>(src.GetValidRow());
    const std::size_t vc = static_cast<std::size_t>(src.GetValidCol());
    if (vr == 1 && rowIndex < vc) {
        return static_cast<typename TilePara::DType>(src.data()[GetTileElementOffset<TilePara>(0, rowIndex)]);
    }
    if (vc == 1 && rowIndex < vr) {
        return static_cast<typename TilePara::DType>(src.data()[GetTileElementOffset<TilePara>(rowIndex, 0)]);
    }
    return static_cast<typename TilePara::DType>(src.data()[rowIndex % static_cast<std::size_t>(TilePara::Numel)]);
}

template <auto quant_type, typename TileDataOut, typename TileDataSrc, typename TileDataPara>
PTO_INTERNAL void TQUANT_IMPL(TileDataOut &dst, TileDataSrc &src, TileDataPara &scale, TileDataPara *offset = nullptr)
{
    using SrcT = typename TileDataSrc::DType;
    using DstT = typename TileDataOut::DType;
    constexpr int quantType = static_cast<int>(quant_type);
    static_assert(std::is_same<SrcT, float>::value, "Fix: Input has to be float 32");
    if constexpr (quantType == 0) {
        static_assert(std::is_same<DstT, int8_t>::value, "Fix: Quant INT8 sym: Out data type has to be int8");
    } else if constexpr (quantType == 1) {
        static_assert(std::is_same<DstT, uint8_t>::value, "Fix: Quant INT8 asym: Out data type has to be uint8");
    }

    const std::size_t rows = static_cast<std::size_t>(src.GetValidRow());
    const std::size_t cols = static_cast<std::size_t>(src.GetValidCol());
    for (std::size_t r = 0; r < rows; ++r) {
        const double scaleVal = static_cast<double>(LoadRowQuantParam(scale, r));
        const double offsetVal = (quantType == 1 && offset != nullptr)
                                     ? static_cast<double>(LoadRowQuantParam(*offset, r))
                                     : 0.0;
        for (std::size_t c = 0; c < cols; ++c) {
            const std::size_t idxSrc = GetTileElementOffset<TileDataSrc>(r, c);
            const std::size_t idxDst = GetTileElementOffset<TileDataOut>(r, c);
            double value = static_cast<double>(src.data()[idxSrc]) * scaleVal + offsetVal;
            value = applyRoundingToIntegral(value, RoundMode::CAST_RINT);
            if constexpr (std::is_same_v<DstT, int8_t>) {
                value = std::clamp(value, -128.0, 127.0);
            } else {
                value = std::clamp(value, 0.0, 255.0);
            }
            dst.data()[idxDst] = static_cast<DstT>(value);
        }
    }
}

} // namespace pto

#endif
