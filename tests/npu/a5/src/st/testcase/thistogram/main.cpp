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
#include "acl/acl.h"
#include "test_common.h"

using namespace std;
using namespace PtoTestCommon;

template <int validRows, int validCols, bool isMSB = true>
void LaunchTHistogram(uint16_t *src, uint32_t *dst, void *stream, uint8_t *idx);

class THISTOGRAMTest : public testing::Test {
protected:
    void SetUp() override
    {}
    void TearDown() override
    {}
};

std::string GetGoldenDir()
{
    const testing::TestInfo *testInfo = testing::UnitTest::GetInstance()->current_test_info();
    const std::string caseName = testInfo->name();
    std::string suiteName = testInfo->test_suite_name();
    std::string fullPath = "../" + suiteName + "." + caseName;
    return fullPath;
}

template <int validRows, int validCols, bool isMSB>
void test_thistogram()
{
    size_t srcFileSize = validRows * validCols * sizeof(uint16_t);
    size_t dstFileSize = validRows * 256 * sizeof(uint32_t);
    const size_t idxFileSize = validRows * sizeof(uint8_t);

    aclInit(nullptr);
    aclrtSetDevice(0);
    aclrtStream stream;
    aclrtCreateStream(&stream);

    uint16_t *srcHost, *srcDevice;
    uint8_t *idxHost, *idxDevice;
    uint32_t *dstHost, *dstDevice;

    aclrtMallocHost((void **)(&srcHost), srcFileSize);
    aclrtMalloc((void **)&srcDevice, srcFileSize, ACL_MEM_MALLOC_HUGE_FIRST);
    aclrtMallocHost((void **)(&idxHost), idxFileSize);
    aclrtMalloc((void **)&idxDevice, idxFileSize, ACL_MEM_MALLOC_HUGE_FIRST);
    aclrtMallocHost((void **)(&dstHost), dstFileSize);
    aclrtMalloc((void **)&dstDevice, dstFileSize, ACL_MEM_MALLOC_HUGE_FIRST);

    // Read input data and copy to device
    size_t readSize = srcFileSize;
    ReadFile(GetGoldenDir() + "/input.bin", readSize, srcHost, srcFileSize);
    aclrtMemcpy(srcDevice, srcFileSize, srcHost, srcFileSize, ACL_MEMCPY_HOST_TO_DEVICE);
    if constexpr (isMSB) {
        idxHost[0] = 0;
    } else {
        size_t idxReadSize = idxFileSize;
        ReadFile(GetGoldenDir() + "/idx.bin", idxReadSize, idxHost, idxFileSize);
    }
    aclrtMemcpy(idxDevice, idxFileSize, idxHost, idxFileSize, ACL_MEMCPY_HOST_TO_DEVICE);

    LaunchTHistogram<validRows, validCols, isMSB>(srcDevice, dstDevice, stream, idxDevice);
    aclrtSynchronizeStream(stream);
    aclrtMemcpy(dstHost, dstFileSize, dstDevice, dstFileSize, ACL_MEMCPY_DEVICE_TO_HOST);

    WriteFile(GetGoldenDir() + "/output.bin", dstHost, dstFileSize);

    aclrtFree(srcDevice);
    aclrtFree(idxDevice);
    aclrtFree(dstDevice);
    aclrtFreeHost(srcHost);
    aclrtFreeHost(idxHost);
    aclrtFreeHost(dstHost);
    aclrtDestroyStream(stream);
    aclrtResetDevice(0);
    aclFinalize();

    constexpr size_t numBinsPerRow = 256;
    std::vector<uint32_t> golden(validRows * numBinsPerRow);
    std::vector<uint32_t> devFinal(validRows * numBinsPerRow);
    ReadFile(GetGoldenDir() + "/golden.bin", dstFileSize, golden.data(), dstFileSize);
    ReadFile(GetGoldenDir() + "/output.bin", dstFileSize, devFinal.data(), dstFileSize);

    bool ret = ResultCmp<uint32_t>(golden, devFinal, 0.001f);

    EXPECT_TRUE(ret);
}

TEST_F(THISTOGRAMTest, case_2x128_MSB)
{
    test_thistogram<2, 128, true>();
}
TEST_F(THISTOGRAMTest, case_4x64_MSB)
{
    test_thistogram<4, 64, true>();
}
TEST_F(THISTOGRAMTest, case_8x128_MSB)
{
    test_thistogram<8, 128, true>();
}
TEST_F(THISTOGRAMTest, case_1x256_MSB)
{
    test_thistogram<1, 256, true>();
}
TEST_F(THISTOGRAMTest, case_4x256_MSB)
{
    test_thistogram<4, 256, true>();
}
TEST_F(THISTOGRAMTest, case_2x100_MSB)
{
    test_thistogram<2, 100, true>();
}

TEST_F(THISTOGRAMTest, case_2x128_LSB_k108)
{
    test_thistogram<2, 128, false>();
}

TEST_F(THISTOGRAMTest, case_4x64_LSB_k52)
{
    test_thistogram<4, 64, false>();
}

TEST_F(THISTOGRAMTest, case_8x128_LSB_k104)
{
    test_thistogram<8, 128, false>();
}

TEST_F(THISTOGRAMTest, case_1x256_LSB_k210)
{
    test_thistogram<1, 256, false>();
}

TEST_F(THISTOGRAMTest, case_4x256_LSB_k220)
{
    test_thistogram<4, 256, false>();
}

TEST_F(THISTOGRAMTest, case_2x100_LSB_k82)
{
    test_thistogram<2, 100, false>();
}
