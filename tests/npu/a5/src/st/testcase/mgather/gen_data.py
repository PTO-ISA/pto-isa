#!/usr/bin/python3
# coding=utf-8
# --------------------------------------------------------------------------------
# Copyright (c) 2026 Huawei Technologies Co., Ltd.
# This program is free software, you can redistribute it and/or modify it under the terms and conditions of
# CANN Open Software License Agreement Version 2.0 (the "License").
# Please refer to the License for details. You may not use this file except in compliance with the License.
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
# See LICENSE in the root of the software repository for the full text of the License.
# --------------------------------------------------------------------------------

import os
import numpy as np

np.random.seed(42)


class MGatherParams:
    def __init__(self, name, dtype, table_rows, table_cols, out_rows, out_cols, mode="default"):
        self.name = name
        self.dtype = dtype
        self.table_rows = table_rows
        self.table_cols = table_cols
        self.out_rows = out_rows
        self.out_cols = out_cols
        self.mode = mode


def gen_golden_data(param: MGatherParams):
    dtype = param.dtype
    table_rows = param.table_rows
    table_cols = param.table_cols
    out_rows = param.out_rows
    out_cols = param.out_cols

    table_size = table_rows * table_cols
    out_size = out_rows * out_cols

    table = ((np.arange(1, table_size + 1) % 256) + 1).astype(dtype)

    if param.mode == "default":
        indices = (np.arange(0, out_size) % table_size).astype(np.int32)
        golden = table[indices]
    elif param.mode == "clamp":
        indices = np.arange(0, out_size, dtype=np.int32)
        indices[out_size // 2 :] = np.arange(table_size, table_size + out_size // 2, dtype=np.int32)
        clamped = np.clip(indices, 0, table_size - 1)
        golden = table[clamped]
    elif param.mode == "wrap":
        indices = np.arange(0, out_size, dtype=np.int32)
        indices[out_size // 2 :] = np.arange(table_size, table_size + out_size // 2, dtype=np.int32)
        wrapped = indices % table_size
        golden = table[wrapped]
    elif param.mode == "zero":
        indices = np.arange(0, out_size, dtype=np.int32)
        indices[out_size // 2 :] = np.arange(table_size, table_size + out_size // 2, dtype=np.int32)
        golden = np.zeros(out_size, dtype=dtype)
        for i in range(out_size):
            if indices[i] < table_size:
                golden[i] = table[indices[i]]

    table.tofile("table.bin")
    indices.tofile("indices.bin")
    golden.tofile("golden.bin")


if __name__ == "__main__":
    case_params_list = [
        MGatherParams("MGATHERTest.case_half_16x64_8x32", np.float16, 16, 64, 8, 32),
        MGatherParams("MGATHERTest.case_half_16x128_8x64", np.float16, 16, 128, 8, 64),
        MGatherParams("MGATHERTest.case_half_32x128_16x64", np.float16, 32, 128, 16, 64),
        MGatherParams("MGATHERTest.case_half_16x256_8x128", np.float16, 16, 256, 8, 128),
        MGatherParams("MGATHERTest.case_half_64x64_32x32", np.float16, 64, 64, 32, 32),
        MGatherParams("MGATHERTest.case_float_8x64_4x32", np.float32, 8, 64, 4, 32),
        MGatherParams("MGATHERTest.case_float_16x64_8x32", np.float32, 16, 64, 8, 32),
        MGatherParams("MGATHERTest.case_float_32x64_16x32", np.float32, 32, 64, 16, 32),
        MGatherParams("MGATHERTest.case_float_16x16_8x8", np.float32, 16, 16, 8, 8),
        MGatherParams("MGATHERTest.case_int32_8x32_4x16", np.int32, 8, 32, 4, 16),
        MGatherParams("MGATHERTest.case_int32_16x64_8x32", np.int32, 16, 64, 8, 32),
        MGatherParams("MGATHERTest.case_int32_32x32_16x16", np.int32, 32, 32, 16, 16),
        MGatherParams("MGATHERTest.case_uint8_16x64_8x32", np.uint8, 16, 64, 8, 32),
        MGatherParams("MGATHERTest.case_uint8_32x64_16x32", np.uint8, 32, 64, 16, 32),
        MGatherParams("MGATHERTest.case_float_clamp_16x64_8x32", np.float32, 16, 64, 8, 32, "clamp"),
        MGatherParams("MGATHERTest.case_int32_wrap_16x64_8x32", np.int32, 16, 64, 8, 32, "wrap"),
        MGatherParams("MGATHERTest.case_half_zero_16x64_8x32", np.float16, 16, 64, 8, 32, "zero"),
    ]

    for param in case_params_list:
        if not os.path.exists(param.name):
            os.makedirs(param.name)
        original_dir = os.getcwd()
        os.chdir(param.name)
        gen_golden_data(param)
        os.chdir(original_dir)
        print(f"Generated {param.name}")

    print("All MGATHER test data generated successfully")
