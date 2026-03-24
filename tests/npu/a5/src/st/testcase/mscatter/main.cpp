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
#include "mscatter_common.h"

using namespace std;
using namespace PtoTestCommon;

class MSCATTERTest : public testing::Test {
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

template <typename T, typename TIdx, int kSrcRows, int kSrcCols, int kOutSize>
void LaunchMSCATTER(T *out, T *src, TIdx *indices, void *stream);

template <int kSrcRows, int kSrcCols, int kOutSize>
void LaunchMSCATTERHalf(aclFloat16 *out, aclFloat16 *src, int32_t *indices, void *stream);

template <ScatterAtomicOp Atomic, ScatterOOB Mode, typename T, typename TIdx, int kSrcRows, int kSrcCols, int kOutSize>
void LaunchMSCATTER_mode(T *out, T *src, TIdx *indices, void *stream);

template <ScatterAtomicOp Atomic, ScatterOOB Mode, int kSrcRows, int kSrcCols, int kOutSize>
void LaunchMSCATTERHalf_mode(aclFloat16 *out, aclFloat16 *src, int32_t *indices, void *stream);

template <typename T, typename TIdx, int kSrcRows, int kSrcCols, int kOutSize>
void test_mscatter()
{
    size_t srcByteSize = kSrcRows * kSrcCols * sizeof(T);
    size_t outByteSize = kOutSize * sizeof(T);
    size_t idxByteSize = kSrcRows * kSrcCols * sizeof(TIdx);

    aclInit(nullptr);
    aclrtSetDevice(0);
    aclrtStream stream;
    aclrtCreateStream(&stream);

    T *srcHost, *outHost;
    TIdx *idxHost;
    T *srcDevice, *outDevice;
    TIdx *idxDevice;

    aclrtMallocHost((void **)(&srcHost), srcByteSize);
    aclrtMallocHost((void **)(&idxHost), idxByteSize);
    aclrtMallocHost((void **)(&outHost), outByteSize);

    aclrtMalloc((void **)&srcDevice, srcByteSize, ACL_MEM_MALLOC_HUGE_FIRST);
    aclrtMalloc((void **)&idxDevice, idxByteSize, ACL_MEM_MALLOC_HUGE_FIRST);
    aclrtMalloc((void **)&outDevice, outByteSize, ACL_MEM_MALLOC_HUGE_FIRST);

    ReadFile(GetGoldenDir() + "/src.bin", srcByteSize, srcHost, srcByteSize);
    ReadFile(GetGoldenDir() + "/indices.bin", idxByteSize, idxHost, idxByteSize);

    aclrtMemcpy(srcDevice, srcByteSize, srcHost, srcByteSize, ACL_MEMCPY_HOST_TO_DEVICE);
    aclrtMemcpy(idxDevice, idxByteSize, idxHost, idxByteSize, ACL_MEMCPY_HOST_TO_DEVICE);

    aclrtMemset(outDevice, outByteSize, 0, outByteSize);

    if constexpr (std::is_same<T, aclFloat16>::value) {
        LaunchMSCATTERHalf<kSrcRows, kSrcCols, kOutSize>(outDevice, srcDevice, idxDevice, stream);
    } else {
        LaunchMSCATTER<T, TIdx, kSrcRows, kSrcCols, kOutSize>(outDevice, srcDevice, idxDevice, stream);
    }

    aclrtSynchronizeStream(stream);
    aclrtMemcpy(outHost, outByteSize, outDevice, outByteSize, ACL_MEMCPY_DEVICE_TO_HOST);

    WriteFile(GetGoldenDir() + "/output.bin", outHost, outByteSize);

    aclrtFree(srcDevice);
    aclrtFree(idxDevice);
    aclrtFree(outDevice);

    aclrtFreeHost(srcHost);
    aclrtFreeHost(idxHost);
    aclrtFreeHost(outHost);
    aclrtDestroyStream(stream);
    aclrtResetDevice(0);
    aclFinalize();

    std::vector<T> golden(kOutSize);
    std::vector<T> devFinal(kOutSize);
    ReadFile(GetGoldenDir() + "/golden.bin", outByteSize, golden.data(), outByteSize);
    ReadFile(GetGoldenDir() + "/output.bin", outByteSize, devFinal.data(), outByteSize);

#ifdef DEBUG_PRINT
    PrintFirst20("GOLDEN", golden.data(), golden.size());
    PrintFirst20("OUTPUT", devFinal.data(), devFinal.size());
#endif

    bool ret = ResultCmp<T>(golden, devFinal, 0.001f);
    EXPECT_TRUE(ret);
}

TEST_F(MSCATTERTest, case_half_8x32_1024)
{
    test_mscatter<aclFloat16, int32_t, 8, 32, 1024>();
}

TEST_F(MSCATTERTest, case_half_16x64_2048)
{
    test_mscatter<aclFloat16, int32_t, 16, 64, 2048>();
}

TEST_F(MSCATTERTest, case_float_8x32_512)
{
    test_mscatter<float, int32_t, 8, 32, 512>();
}

TEST_F(MSCATTERTest, case_float_16x32_1024)
{
    test_mscatter<float, int32_t, 16, 32, 1024>();
}

TEST_F(MSCATTERTest, case_float_16x64_2048)
{
    test_mscatter<float, int32_t, 16, 64, 2048>();
}

TEST_F(MSCATTERTest, case_float_8x8_128)
{
    test_mscatter<float, int32_t, 8, 8, 128>();
}

TEST_F(MSCATTERTest, case_int32_8x16_256)
{
    test_mscatter<int32_t, int32_t, 8, 16, 256>();
}

TEST_F(MSCATTERTest, case_int32_16x32_1024)
{
    test_mscatter<int32_t, int32_t, 16, 32, 1024>();
}

TEST_F(MSCATTERTest, case_int32_16x16_512)
{
    test_mscatter<int32_t, int32_t, 16, 16, 512>();
}

TEST_F(MSCATTERTest, case_uint8_16x32_1024)
{
    test_mscatter<uint8_t, int32_t, 16, 32, 1024>();
}

TEST_F(MSCATTERTest, case_uint8_16x64_2048)
{
    test_mscatter<uint8_t, int32_t, 16, 64, 2048>();
}

template <ScatterAtomicOp Atomic, ScatterOOB Mode, typename T, typename TIdx, int kSrcRows, int kSrcCols, int kOutSize>
void test_mscatter_mode()
{
    size_t srcByteSize = kSrcRows * kSrcCols * sizeof(T);
    size_t outByteSize = kOutSize * sizeof(T);
    size_t idxByteSize = kSrcRows * kSrcCols * sizeof(TIdx);

    aclInit(nullptr);
    aclrtSetDevice(0);
    aclrtStream stream;
    aclrtCreateStream(&stream);

    T *srcHost, *outHost;
    TIdx *idxHost;
    T *srcDevice, *outDevice;
    TIdx *idxDevice;

    aclrtMallocHost((void **)(&srcHost), srcByteSize);
    aclrtMallocHost((void **)(&idxHost), idxByteSize);
    aclrtMallocHost((void **)(&outHost), outByteSize);

    aclrtMalloc((void **)&srcDevice, srcByteSize, ACL_MEM_MALLOC_HUGE_FIRST);
    aclrtMalloc((void **)&idxDevice, idxByteSize, ACL_MEM_MALLOC_HUGE_FIRST);
    aclrtMalloc((void **)&outDevice, outByteSize, ACL_MEM_MALLOC_HUGE_FIRST);

    ReadFile(GetGoldenDir() + "/src.bin", srcByteSize, srcHost, srcByteSize);
    ReadFile(GetGoldenDir() + "/indices.bin", idxByteSize, idxHost, idxByteSize);

    aclrtMemcpy(srcDevice, srcByteSize, srcHost, srcByteSize, ACL_MEMCPY_HOST_TO_DEVICE);
    aclrtMemcpy(idxDevice, idxByteSize, idxHost, idxByteSize, ACL_MEMCPY_HOST_TO_DEVICE);

    aclrtMemset(outDevice, outByteSize, 0, outByteSize);

    if constexpr (std::is_same<T, aclFloat16>::value) {
        LaunchMSCATTERHalf_mode<Atomic, Mode, kSrcRows, kSrcCols, kOutSize>(outDevice, srcDevice, idxDevice, stream);
    } else {
        LaunchMSCATTER_mode<Atomic, Mode, T, TIdx, kSrcRows, kSrcCols, kOutSize>(outDevice, srcDevice, idxDevice,
                                                                                 stream);
    }

    aclrtSynchronizeStream(stream);
    aclrtMemcpy(outHost, outByteSize, outDevice, outByteSize, ACL_MEMCPY_DEVICE_TO_HOST);

    WriteFile(GetGoldenDir() + "/output.bin", outHost, outByteSize);

    aclrtFree(srcDevice);
    aclrtFree(idxDevice);
    aclrtFree(outDevice);

    aclrtFreeHost(srcHost);
    aclrtFreeHost(idxHost);
    aclrtFreeHost(outHost);
    aclrtDestroyStream(stream);
    aclrtResetDevice(0);
    aclFinalize();

    std::vector<T> golden(kOutSize);
    std::vector<T> devFinal(kOutSize);
    ReadFile(GetGoldenDir() + "/golden.bin", outByteSize, golden.data(), outByteSize);
    ReadFile(GetGoldenDir() + "/output.bin", outByteSize, devFinal.data(), outByteSize);

    bool ret = ResultCmp<T>(golden, devFinal, 0.001f);
    EXPECT_TRUE(ret);
}

TEST_F(MSCATTERTest, case_float_skip_8x32_512)
{
    test_mscatter_mode<ScatterAtomicOp::None, ScatterOOB::Skip, float, int32_t, 8, 32, 512>();
}

TEST_F(MSCATTERTest, case_int32_clamp_8x16_256)
{
    test_mscatter_mode<ScatterAtomicOp::None, ScatterOOB::Clamp, int32_t, int32_t, 8, 16, 256>();
}

TEST_F(MSCATTERTest, case_half_wrap_8x32_1024)
{
    test_mscatter_mode<ScatterAtomicOp::None, ScatterOOB::Wrap, aclFloat16, int32_t, 8, 32, 1024>();
}

TEST_F(MSCATTERTest, case_float_atomicadd_8x32_512)
{
    test_mscatter_mode<ScatterAtomicOp::Add, ScatterOOB::Undefined, float, int32_t, 8, 32, 512>();
}

TEST_F(MSCATTERTest, case_int32_atomicadd_skip_8x16_256)
{
    test_mscatter_mode<ScatterAtomicOp::Add, ScatterOOB::Skip, int32_t, int32_t, 8, 16, 256>();
}
