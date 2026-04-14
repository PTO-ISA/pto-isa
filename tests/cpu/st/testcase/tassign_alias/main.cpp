/**
Copyright (c) 2025 Huawei Technologies Co., Ltd.
This program is free software, you can redistribute it and/or modify it under the terms and conditions of
CANN Open Software License Agreement Version 2.0 (the "License").
Please refer to the License for details. You may not use this file except in compliance with the License.
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
See LICENSE in the root of the software repository for the full text of the License.
*/

#include <cstdint>

#include <gtest/gtest.h>
#include <pto/pto-inst.hpp>

using namespace pto;

class TAssignAliasTest : public testing::Test {
protected:
    void SetUp() override
    {
        NPU_MEMORY_INIT(NPUArch::A2A3);
        NPU_MEMORY_CLEAR();
    }
};

TEST_F(TAssignAliasTest, exact_alias_reuses_same_backing_buffer)
{
    using BaseTile = Tile<TileType::Vec, float, 8, 64, BLayout::RowMajor, 8, 64>;
    using AliasTile = Tile<TileType::Vec, float, 8, 64, BLayout::RowMajor, 8, 64>;

    BaseTile base;
    AliasTile alias;

    TASSIGN(base, 0);
    TASSIGN(alias, reinterpret_cast<std::uintptr_t>(base.data()));

    ASSERT_EQ(base.data(), alias.data());

    base.SetValue(5, 3.5f);
    EXPECT_FLOAT_EQ(alias.GetValue(5), 3.5f);

    alias.SetValue(17, -2.25f);
    EXPECT_FLOAT_EQ(base.GetValue(17), -2.25f);
}

TEST_F(TAssignAliasTest, interior_alias_treats_pointer_as_pointer_not_offset)
{
    using BaseTile = Tile<TileType::Vec, float, 8, 64, BLayout::RowMajor, 8, 64>;
    using TailTile = Tile<TileType::Vec, float, 4, 64, BLayout::RowMajor, 4, 64>;

    constexpr std::size_t kRowOffset = 4;
    constexpr std::size_t kColCount = 64;
    constexpr std::size_t kElementOffset = kRowOffset * kColCount;

    BaseTile base;
    TailTile tail;

    TASSIGN(base, 0);
    TASSIGN(tail, reinterpret_cast<std::uintptr_t>(base.data() + kElementOffset));

    ASSERT_EQ(tail.data(), base.data() + kElementOffset);

    tail.SetValue(3, 9.0f);
    EXPECT_FLOAT_EQ(base.GetValue(kElementOffset + 3), 9.0f);

    base.SetValue(kElementOffset + 10, -7.0f);
    EXPECT_FLOAT_EQ(tail.GetValue(10), -7.0f);
}
