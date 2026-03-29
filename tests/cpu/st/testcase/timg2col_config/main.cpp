/**
Copyright (c) 2026 Huawei Technologies Co., Ltd.
This program is free software, you can redistribute it and/or modify it under the terms and conditions of
CANN Open Software License Agreement Version 2.0 (the "License").
Please refer to the License for details. You may not use this file except in compliance with the License.
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
See LICENSE in the root of the software repository for the full text of the License.
*/

#include <gtest/gtest.h>
#include <pto/pto-inst.hpp>
#include <pto/cpu/TSetFmatrix.hpp>
#include <pto/cpu/TSetImg2colPadding.hpp>
#include <pto/cpu/TSetImg2colRpt.hpp>

using namespace pto;

namespace {
struct DummyConvTileData {
    using DType = float;

    uint16_t fmapW = 17;
    uint16_t fmapH = 9;
    uint8_t padList[4] = {1, 2, 3, 4};
    uint16_t repeatStride = 5;
    uint8_t repeatTime = 6;
    uint8_t repeatMode = 7;
    float padValue = 1.25f;

    uint16_t GetFmapW() const { return fmapW; }
    uint16_t GetFmapH() const { return fmapH; }
    const uint8_t *GetPadListArray() const { return padList; }
    uint16_t GetRepeatStride() const { return repeatStride; }
    uint8_t GetRepeatTime() const { return repeatTime; }
    uint8_t GetRepeatMode() const { return repeatMode; }
    DType GetPadValue() const { return padValue; }
};
} // namespace

TEST(TIMG2COLConfigCpuSim, tsetfmatrix_noop)
{
    DummyConvTileData tile;
    TSETFMATRIX<DummyConvTileData, SetFmatrixMode::FMATRIX_A_MANUAL>(tile);
    TSETFMATRIX<DummyConvTileData, SetFmatrixMode::FMATRIX_B_MANUAL>(tile);
    EXPECT_EQ(tile.GetFmapW(), 17);
    EXPECT_EQ(tile.GetFmapH(), 9);
}

TEST(TIMG2COLConfigCpuSim, tset_img2col_rpt_noop)
{
    DummyConvTileData tile;
    TSET_IMG2COL_RPT_IMPL(tile);
    EXPECT_EQ(tile.GetRepeatStride(), 5);
    EXPECT_EQ(tile.GetRepeatTime(), 6);
    EXPECT_EQ(tile.GetRepeatMode(), 7);
}

TEST(TIMG2COLConfigCpuSim, tset_img2col_padding_noop)
{
    DummyConvTileData tile;
    TSET_IMG2COL_PADDING_IMPL(tile);
    EXPECT_FLOAT_EQ(tile.GetPadValue(), 1.25f);
}
