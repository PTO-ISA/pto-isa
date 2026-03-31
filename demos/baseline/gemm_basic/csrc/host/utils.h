/**
Copyright (c) 2026 Huawei Technologies Co., Ltd.
This program is free software, you can redistribute it and/or modify it under the terms and conditions of
CANN Open Software License Agreement Version 2.0 (the "License").
Please refer to the License for details. You may not use this file except in compliance with the License.
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
See LICENSE in the root of the software repository for the full text of the License.
*/

#ifndef PTO_GEMM_BASIC_TORCH_CSRC_UTILS_H
#define PTO_GEMM_BASIC_TORCH_CSRC_UTILS_H

#include <ATen/ATen.h>
#include <torch/library.h>
#include "torch_npu/csrc/framework/OpCommand.h"
#include "torch_npu/csrc/core/npu/NPUStream.h"

namespace pto_path {

#define DEVICE_TYPE c10::DeviceType::PrivateUse1

template <typename... Ts>
constexpr auto ConvertTypes(Ts &... args)
{
    return std::make_tuple(ConvertType(args)...);
}

template <typename T>
T ConvertType(T value)
{
    return value;
}

inline void *ConvertType(const at::Tensor &at_tensor)
{
    return const_cast<void *>(at_tensor.storage().data());
}

#define EXEC_KERNEL_CMD(kernel_name, blockdim, ...)                                                                  \
    do {                                                                                                             \
        auto converted_params = ConvertTypes(__VA_ARGS__);                                                           \
        auto acl_stream = c10_npu::getCurrentNPUStream().stream(false);                                              \
        auto acl_call = [acl_stream, blockdim, converted_params]() -> int {                                          \
            uint32_t ret = 0;                                                                                        \
            std::apply(                                                                                              \
                [&](auto &&... params) { ret = ACLRT_LAUNCH_KERNEL(kernel_name)(blockdim, acl_stream, params...); }, \
                converted_params);                                                                                   \
        };                                                                                                           \
        at_npu::native::OpCommand::RunOpApi(#kernel_name, acl_call);                                                 \
    } while (false)

} // namespace pto_path

#endif
