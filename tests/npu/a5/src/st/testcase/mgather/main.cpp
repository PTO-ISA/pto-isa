/**
Copyright (c) 2026 Huawei Technologies Co., Ltd.
This program is free software, you can redistribute it and/or modify it under the terms and conditions of
CANN Open Software License Agreement Version 2.0 (the "License").
Please refer to the License for details. You may not use this file except in compliance with the License.
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
See LICENSE in the root of the software repository for the full text of the License.
*/

#include "test_common.h"
#include <gtest/gtest.h>
#include "acl/acl.h"
#include "mgather_common.h"

using namespace std;
using namespace PtoTestCommon;

class MGATHERTest : public testing::Test {
protected:
    void SetUp() override
    {}
    void TearDown() override
    {}
};

//#define DEBUG_PRINT

#ifdef DEBUG_PRINT
template <typename T>
void PrintFirst20(const char *name, const T *data, size_t count)
{
    size_t n = std::min(count, static_cast<size_t>(20));
    std::cout << "\n=== " << name << " (first " << n << " values) ===" << std::endl;
    for (size_t i = 0; i < n; ++i) {
        if constexpr (std::is_same_v<T, aclFloat16>) {
            std::cout << std::setw(10) << aclFloat16ToFloat(data[i]);
        } else {
            std::cout << std::setw(10) << static_cast<float>(data[i]);
        }
        if ((i + 1) % 10 == 0)
            std::cout << std::endl;
    }
    std::cout << std::endl;
}
#endif

std::string GetGoldenDir()
{
    const testing::TestInfo *testInfo = testing::UnitTest::GetInstance()->current_test_info();
    const std::string caseName = testInfo->name();
    std::string suiteName = testInfo->test_suite_name();
    std::string fullPath = "../" + suiteName + "." + caseName;
    return fullPath;
}

template <typename T, typename TIdx, int kTableRows, int kTableCols, int kOutRows, int kOutCols>
void LaunchMGATHER(T *out, T *table, TIdx *indices, void *stream);

template <int kTableRows, int kTableCols, int kOutRows, int kOutCols>
void LaunchMGATHERHalf(aclFloat16 *out, aclFloat16 *table, int32_t *indices, void *stream);

template <GatherOOB Mode, typename T, typename TIdx, int kTableRows, int kTableCols, int kOutRows, int kOutCols>
void LaunchMGATHER_mode(T *out, T *table, TIdx *indices, void *stream);

template <GatherOOB Mode, int kTableRows, int kTableCols, int kOutRows, int kOutCols>
void LaunchMGATHERHalf_mode(aclFloat16 *out, aclFloat16 *table, int32_t *indices, void *stream);

template <typename T, typename TIdx, int kTableRows, int kTableCols, int kOutRows, int kOutCols>
void test_mgather()
{
    size_t tableByteSize = kTableRows * kTableCols * sizeof(T);
    size_t outByteSize = kOutRows * kOutCols * sizeof(T);
    size_t idxByteSize = kOutRows * kOutCols * sizeof(TIdx);

    aclInit(nullptr);
    aclrtSetDevice(0);
    aclrtStream stream;
    aclrtCreateStream(&stream);

    T *tableHost, *outHost;
    TIdx *idxHost;
    T *tableDevice, *outDevice;
    TIdx *idxDevice;

    aclrtMallocHost((void **)(&tableHost), tableByteSize);
    aclrtMallocHost((void **)(&idxHost), idxByteSize);
    aclrtMallocHost((void **)(&outHost), outByteSize);

    aclrtMalloc((void **)&tableDevice, tableByteSize, ACL_MEM_MALLOC_HUGE_FIRST);
    aclrtMalloc((void **)&idxDevice, idxByteSize, ACL_MEM_MALLOC_HUGE_FIRST);
    aclrtMalloc((void **)&outDevice, outByteSize, ACL_MEM_MALLOC_HUGE_FIRST);

    ReadFile(GetGoldenDir() + "/table.bin", tableByteSize, tableHost, tableByteSize);
    ReadFile(GetGoldenDir() + "/indices.bin", idxByteSize, idxHost, idxByteSize);

    aclrtMemcpy(tableDevice, tableByteSize, tableHost, tableByteSize, ACL_MEMCPY_HOST_TO_DEVICE);
    aclrtMemcpy(idxDevice, idxByteSize, idxHost, idxByteSize, ACL_MEMCPY_HOST_TO_DEVICE);

    if constexpr (std::is_same<T, aclFloat16>::value) {
        LaunchMGATHERHalf<kTableRows, kTableCols, kOutRows, kOutCols>(outDevice, tableDevice, idxDevice, stream);
    } else {
        LaunchMGATHER<T, TIdx, kTableRows, kTableCols, kOutRows, kOutCols>(outDevice, tableDevice, idxDevice, stream);
    }

    aclrtSynchronizeStream(stream);
    aclrtMemcpy(outHost, outByteSize, outDevice, outByteSize, ACL_MEMCPY_DEVICE_TO_HOST);

    WriteFile(GetGoldenDir() + "/output.bin", outHost, outByteSize);

    aclrtFree(tableDevice);
    aclrtFree(idxDevice);
    aclrtFree(outDevice);

    aclrtFreeHost(tableHost);
    aclrtFreeHost(idxHost);
    aclrtFreeHost(outHost);
    aclrtDestroyStream(stream);
    aclrtResetDevice(0);
    aclFinalize();

    std::vector<T> golden(kOutRows * kOutCols);
    std::vector<T> devFinal(kOutRows * kOutCols);
    ReadFile(GetGoldenDir() + "/golden.bin", outByteSize, golden.data(), outByteSize);
    ReadFile(GetGoldenDir() + "/output.bin", outByteSize, devFinal.data(), outByteSize);

#ifdef DEBUG_PRINT
    PrintFirst20("GOLDEN", golden.data(), golden.size());
    PrintFirst20("OUTPUT", devFinal.data(), devFinal.size());
#endif

    bool ret = ResultCmp<T>(golden, devFinal, 0.001f);
    EXPECT_TRUE(ret);
}

TEST_F(MGATHERTest, case_half_16x64_8x32)
{
    test_mgather<aclFloat16, int32_t, 16, 64, 8, 32>();
}

TEST_F(MGATHERTest, case_half_16x128_8x64)
{
    test_mgather<aclFloat16, int32_t, 16, 128, 8, 64>();
}

TEST_F(MGATHERTest, case_half_32x128_16x64)
{
    test_mgather<aclFloat16, int32_t, 32, 128, 16, 64>();
}

TEST_F(MGATHERTest, case_half_16x256_8x128)
{
    test_mgather<aclFloat16, int32_t, 16, 256, 8, 128>();
}

TEST_F(MGATHERTest, case_half_64x64_32x32)
{
    test_mgather<aclFloat16, int32_t, 64, 64, 32, 32>();
}

TEST_F(MGATHERTest, case_float_8x64_4x32)
{
    test_mgather<float, int32_t, 8, 64, 4, 32>();
}

TEST_F(MGATHERTest, case_float_16x64_8x32)
{
    test_mgather<float, int32_t, 16, 64, 8, 32>();
}

TEST_F(MGATHERTest, case_float_32x64_16x32)
{
    test_mgather<float, int32_t, 32, 64, 16, 32>();
}

TEST_F(MGATHERTest, case_float_16x16_8x8)
{
    test_mgather<float, int32_t, 16, 16, 8, 8>();
}

TEST_F(MGATHERTest, case_int32_8x32_4x16)
{
    test_mgather<int32_t, int32_t, 8, 32, 4, 16>();
}

TEST_F(MGATHERTest, case_int32_16x64_8x32)
{
    test_mgather<int32_t, int32_t, 16, 64, 8, 32>();
}

TEST_F(MGATHERTest, case_int32_32x32_16x16)
{
    test_mgather<int32_t, int32_t, 32, 32, 16, 16>();
}

TEST_F(MGATHERTest, case_uint8_16x64_8x32)
{
    test_mgather<uint8_t, int32_t, 16, 64, 8, 32>();
}

TEST_F(MGATHERTest, case_uint8_32x64_16x32)
{
    test_mgather<uint8_t, int32_t, 32, 64, 16, 32>();
}

template <GatherOOB Mode, typename T, typename TIdx, int kTableRows, int kTableCols, int kOutRows, int kOutCols>
void test_mgather_mode()
{
    size_t tableByteSize = kTableRows * kTableCols * sizeof(T);
    size_t outByteSize = kOutRows * kOutCols * sizeof(T);
    size_t idxByteSize = kOutRows * kOutCols * sizeof(TIdx);

    aclInit(nullptr);
    aclrtSetDevice(0);
    aclrtStream stream;
    aclrtCreateStream(&stream);

    T *tableHost, *outHost;
    TIdx *idxHost;
    T *tableDevice, *outDevice;
    TIdx *idxDevice;

    aclrtMallocHost((void **)(&tableHost), tableByteSize);
    aclrtMallocHost((void **)(&idxHost), idxByteSize);
    aclrtMallocHost((void **)(&outHost), outByteSize);

    aclrtMalloc((void **)&tableDevice, tableByteSize, ACL_MEM_MALLOC_HUGE_FIRST);
    aclrtMalloc((void **)&idxDevice, idxByteSize, ACL_MEM_MALLOC_HUGE_FIRST);
    aclrtMalloc((void **)&outDevice, outByteSize, ACL_MEM_MALLOC_HUGE_FIRST);

    ReadFile(GetGoldenDir() + "/table.bin", tableByteSize, tableHost, tableByteSize);
    ReadFile(GetGoldenDir() + "/indices.bin", idxByteSize, idxHost, idxByteSize);

    aclrtMemcpy(tableDevice, tableByteSize, tableHost, tableByteSize, ACL_MEMCPY_HOST_TO_DEVICE);
    aclrtMemcpy(idxDevice, idxByteSize, idxHost, idxByteSize, ACL_MEMCPY_HOST_TO_DEVICE);

    if constexpr (std::is_same<T, aclFloat16>::value) {
        LaunchMGATHERHalf_mode<Mode, kTableRows, kTableCols, kOutRows, kOutCols>(outDevice, tableDevice, idxDevice,
                                                                                 stream);
    } else {
        LaunchMGATHER_mode<Mode, T, TIdx, kTableRows, kTableCols, kOutRows, kOutCols>(outDevice, tableDevice, idxDevice,
                                                                                      stream);
    }

    aclrtSynchronizeStream(stream);
    aclrtMemcpy(outHost, outByteSize, outDevice, outByteSize, ACL_MEMCPY_DEVICE_TO_HOST);

    WriteFile(GetGoldenDir() + "/output.bin", outHost, outByteSize);

    aclrtFree(tableDevice);
    aclrtFree(idxDevice);
    aclrtFree(outDevice);

    aclrtFreeHost(tableHost);
    aclrtFreeHost(idxHost);
    aclrtFreeHost(outHost);
    aclrtDestroyStream(stream);
    aclrtResetDevice(0);
    aclFinalize();

    std::vector<T> golden(kOutRows * kOutCols);
    std::vector<T> devFinal(kOutRows * kOutCols);
    ReadFile(GetGoldenDir() + "/golden.bin", outByteSize, golden.data(), outByteSize);
    ReadFile(GetGoldenDir() + "/output.bin", outByteSize, devFinal.data(), outByteSize);

    bool ret = ResultCmp<T>(golden, devFinal, 0.001f);
    EXPECT_TRUE(ret);
}

TEST_F(MGATHERTest, case_float_clamp_16x64_8x32)
{
    test_mgather_mode<GatherOOB::Clamp, float, int32_t, 16, 64, 8, 32>();
}

TEST_F(MGATHERTest, case_int32_wrap_16x64_8x32)
{
    test_mgather_mode<GatherOOB::Wrap, int32_t, int32_t, 16, 64, 8, 32>();
}

TEST_F(MGATHERTest, case_half_zero_16x64_8x32)
{
    test_mgather_mode<GatherOOB::Zero, aclFloat16, int32_t, 16, 64, 8, 32>();
}
