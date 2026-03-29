/**
Copyright (c) 2026 Huawei Technologies Co., Ltd.
This program is free software, you can redistribute it and/or modify it under the terms and conditions of
CANN Open Software License Agreement Version 2.0 (the "License").
Please refer to the License for details. You may not use this file except in compliance with the License.
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
See LICENSE in the root of the software repository for the full text of the License.
*/

#include <algorithm>
#include <cstdint>
#include <pto/pto-inst.hpp>
#include <pto/cpu/tile_offsets.hpp>
#include <gtest/gtest.h>

using namespace pto;

namespace {

enum class QuantType
{
    INT8_SYM,
    INT8_ASYM
};

template <typename TileData>
void FillZero(TileData &tile)
{
    std::fill(tile.data(), tile.data() + TileData::Numel, typename TileData::DType(0));
}

template <typename TileData>
auto GetValue(const TileData &tile, int r, int c) -> typename TileData::DType
{
    return tile.data()[GetTileElementOffset<TileData>(r, c)];
}

template <typename TileData>
void SetValue(TileData &tile, int r, int c, typename TileData::DType value)
{
    tile.data()[GetTileElementOffset<TileData>(r, c)] = value;
}

} // namespace

TEST(TQUANTCpuSim, int8_sym_per_row_scale)
{
    using SrcTile = Tile<TileType::Vec, float, 2, 32, BLayout::RowMajor, 2, 4>;
    using ParaTile = Tile<TileType::Vec, float, 1, 8, BLayout::RowMajor, 1, 2>;
    using DstTile = Tile<TileType::Vec, int8_t, 2, 32, BLayout::RowMajor, 2, 4>;

    SrcTile src;
    ParaTile scale;
    DstTile dst;

    SetValue(src, 0, 0, 1.2f);
    SetValue(src, 0, 1, -2.7f);
    SetValue(src, 0, 2, 100.0f);
    SetValue(src, 0, 3, -100.0f);
    SetValue(src, 1, 0, 2.0f);
    SetValue(src, 1, 1, -3.0f);
    SetValue(src, 1, 2, 260.0f);
    SetValue(src, 1, 3, -260.0f);
    scale.data()[GetTileElementOffset<ParaTile>(0, 0)] = 2.0f;
    scale.data()[GetTileElementOffset<ParaTile>(0, 1)] = 0.5f;
    FillZero(dst);

    TQUANT<QuantType::INT8_SYM>(dst, src, scale);

    EXPECT_EQ(GetValue(dst, 0, 0), 2);
    EXPECT_EQ(GetValue(dst, 0, 1), -5);
    EXPECT_EQ(GetValue(dst, 0, 2), 127);
    EXPECT_EQ(GetValue(dst, 0, 3), -128);
    EXPECT_EQ(GetValue(dst, 1, 0), 1);
    EXPECT_EQ(GetValue(dst, 1, 1), -2);
    EXPECT_EQ(GetValue(dst, 1, 2), 127);
    EXPECT_EQ(GetValue(dst, 1, 3), -128);
}

TEST(TQUANTCpuSim, int8_asym_per_row_scale_and_offset)
{
    using SrcTile = Tile<TileType::Vec, float, 2, 32, BLayout::RowMajor, 2, 4>;
    using ParaTile = Tile<TileType::Vec, float, 1, 8, BLayout::RowMajor, 1, 2>;
    using DstTile = Tile<TileType::Vec, uint8_t, 2, 32, BLayout::RowMajor, 2, 4>;

    SrcTile src;
    ParaTile scale;
    ParaTile offset;
    DstTile dst;

    SetValue(src, 0, 0, 1.2f);
    SetValue(src, 0, 1, -2.7f);
    SetValue(src, 0, 2, 300.0f);
    SetValue(src, 0, 3, -300.0f);
    SetValue(src, 1, 0, 4.0f);
    SetValue(src, 1, 1, -4.0f);
    SetValue(src, 1, 2, 200.0f);
    SetValue(src, 1, 3, -400.0f);
    scale.data()[GetTileElementOffset<ParaTile>(0, 0)] = 2.0f;
    scale.data()[GetTileElementOffset<ParaTile>(0, 1)] = 0.5f;
    offset.data()[GetTileElementOffset<ParaTile>(0, 0)] = 10.0f;
    offset.data()[GetTileElementOffset<ParaTile>(0, 1)] = 100.0f;
    FillZero(dst);

    TQUANT<QuantType::INT8_ASYM>(dst, src, scale, &offset);

    EXPECT_EQ(GetValue(dst, 0, 0), 12);
    EXPECT_EQ(GetValue(dst, 0, 1), 5);
    EXPECT_EQ(GetValue(dst, 0, 2), 255);
    EXPECT_EQ(GetValue(dst, 0, 3), 0);
    EXPECT_EQ(GetValue(dst, 1, 0), 102);
    EXPECT_EQ(GetValue(dst, 1, 1), 98);
    EXPECT_EQ(GetValue(dst, 1, 2), 200);
    EXPECT_EQ(GetValue(dst, 1, 3), 0);
}
