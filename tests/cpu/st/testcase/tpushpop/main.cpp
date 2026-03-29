/**
Copyright (c) 2026 Huawei Technologies Co., Ltd.
This program is free software, you can redistribute it and/or modify it under the terms and conditions of
CANN Open Software License Agreement Version 2.0 (the "License").
Please refer to the License for details. You may not use this file except in compliance with the License.
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
See LICENSE in the root of the software repository for the full text of the License.
*/

#include <pto/pto-inst.hpp>
#include <chrono>
#include <gtest/gtest.h>
#include <pto/common/fifo.hpp>
#include <thread>
#include <vector>
#include "test_common.h"

using namespace std;
using namespace pto;
using namespace PtoTestCommon;

template <typename T, int rows, int cols, TileType srcLoc>
void fillTile(auto &tile, int iter)
{
    for (int i = 0; i < tile.Numel; ++i) {
        tile.data()[i] = static_cast<T>(iter * 1000 + i + 1);
    }
}

template <typename T, int rows, int cols, TileType srcLoc>
std::vector<T> makeExpected(int iter)
{
    using PPTile = Tile<srcLoc, T, rows, cols>;
    std::vector<T> expected(PPTile::Numel);
    for (int i = 0; i < PPTile::Numel; ++i) {
        expected[i] = static_cast<T>(iter * 1000 + i + 1);
    }
    return expected;
}

template <typename T, int rows, int cols, TileType srcLoc>
void testPushPopSingleThread()
{
    constexpr int FiFoDepth = 8;
    constexpr int FiFoPeriod = 1;
    constexpr int LocalDepth = 2;
    using PPipe =
        TPipe<0, T, FIFOType::GM_FIFO, FiFoDepth, FiFoPeriod, LocalDepth, TSyncOpType::TSTORE_C2GM, TSyncOpType::TLOAD>;
    using PPTile = Tile<srcLoc, T, rows, cols>;
    std::vector<T> fifoStorage(PPTile::Numel * PPipe::DataFiFo::fifoDepth, static_cast<T>(0));
    PPipe::reset_for_cpu_sim();
    PPipe pipe(fifoStorage.data(), 0x0);
    PPTile src;
    PPTile dst;
    fillTile<T, rows, cols, srcLoc>(src, 0);
    for (int i = 0; i < dst.Numel; ++i) {
        dst.data()[i] = static_cast<T>(0);
    }

    TPUSH(src, pipe);
    TPOP(dst, pipe);

    const auto expected = makeExpected<T, rows, cols, srcLoc>(0);
    EXPECT_TRUE(ResultCmp(expected, dst.data(), 0));
}

template <typename T, int rows, int cols, TileType srcLoc>
void testPushPopMultiCore()
{
    constexpr int FiFoDepth = 4;
    constexpr int FiFoPeriod = 1;
    constexpr int LocalDepth = 0;
    using PPipe =
        TPipe<1, T, FIFOType::GM_FIFO, FiFoDepth, FiFoPeriod, LocalDepth, TSyncOpType::TSTORE_C2GM, TSyncOpType::TLOAD>;
    using PPTile = Tile<srcLoc, T, rows, cols>;

    constexpr int kIterations = 12;
    std::vector<T> fifoStorage(PPTile::Numel * PPipe::DataFiFo::fifoDepth, static_cast<T>(0));
    std::vector<std::vector<T>> actual(kIterations);
    PPipe::reset_for_cpu_sim();
    PPipe pipe(fifoStorage.data(), 0x0);

    std::thread producer([&]() {
        for (int iter = 0; iter < kIterations; ++iter) {
            PPTile src;
            fillTile<T, rows, cols, srcLoc>(src, iter);
            TPUSH(src, pipe);
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    std::thread consumer([&]() {
        for (int iter = 0; iter < kIterations; ++iter) {
            PPTile dst;
            for (int i = 0; i < dst.Numel; ++i) {
                dst.data()[i] = static_cast<T>(0);
            }
            TPOP(dst, pipe);
            actual[iter].assign(dst.data(), dst.data() + dst.Numel);
        }
    });

    producer.join();
    consumer.join();

    for (int iter = 0; iter < kIterations; ++iter) {
        const auto expected = makeExpected<T, rows, cols, srcLoc>(iter);
        EXPECT_TRUE(ResultCmp(expected, actual[iter], 0));
    }
}

class TPushPopTest : public testing::Test {
protected:
    void SetUp() override
    {}
    void TearDown() override
    {}
};

#define TPUSHPOP_TEST(T, rows, cols, srcLoc)             \
    TEST_F(TPushPopTest, T##_##rows##_##cols##_##srcLoc) \
    {                                                    \
        testPushPopSingleThread<T, rows, cols, TileType::srcLoc>(); \
    }

TPUSHPOP_TEST(float, 64, 128, Vec)
TPUSHPOP_TEST(float, 128, 128, Vec)
TPUSHPOP_TEST(float, 64, 128, Mat)
TPUSHPOP_TEST(float, 128, 128, Mat)
TPUSHPOP_TEST(uint32_t, 64, 128, Vec)
TPUSHPOP_TEST(uint32_t, 128, 128, Vec)
TPUSHPOP_TEST(uint32_t, 64, 128, Mat)
TPUSHPOP_TEST(uint32_t, 128, 128, Mat)

TEST_F(TPushPopTest, multicore_float_64_128_Vec)
{
    testPushPopMultiCore<float, 64, 128, TileType::Vec>();
}

TEST_F(TPushPopTest, tfree_discards_waited_slot)
{
    constexpr int FiFoDepth = 4;
    constexpr int FiFoPeriod = 1;
    constexpr int LocalDepth = 0;
    using PPipe =
        TPipe<1, float, FIFOType::GM_FIFO, FiFoDepth, FiFoPeriod, LocalDepth, TSyncOpType::TSTORE_C2GM, TSyncOpType::TLOAD>;
    using PPTile = Tile<TileType::Vec, float, 64, 128>;

    std::vector<float> fifoStorage(PPTile::Numel * PPipe::DataFiFo::fifoDepth, 0.0f);
    PPipe::reset_for_cpu_sim();
    PPipe pipe(fifoStorage.data(), 0x0);

    PPTile first;
    PPTile second;
    PPTile dst;
    fillTile<float, 64, 128, TileType::Vec>(first, 0);
    fillTile<float, 64, 128, TileType::Vec>(second, 1);
    std::fill(dst.data(), dst.data() + dst.Numel, 0.0f);

    TPUSH(first, pipe);
    pipe.cons.wait();
    TFREE(pipe);
    TPUSH(second, pipe);
    TPOP(dst, pipe);

    const auto expected = makeExpected<float, 64, 128, TileType::Vec>(1);
    EXPECT_TRUE(ResultCmp(expected, dst.data(), 0));
}
