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
import struct 
np.random.seed(25)

def gen_golden_data(case_name, param):
    dtype = param.dtype
    row, col = [param.tile_row, param.tile_col] 
    row_valid, col_valid = [param.valid_row, param.valid_col]

    #Generate random input arrays
    input_1 = np.random.randint(0, 6, size=[row, col]).astype(dtype)
    input_2 = np.random.randint(0, 6, size=[row, col]).astype(dtype)
    op = param.op
    output = op(input_1, input_2)

    #Save the input and golden data to binary files
    input_1.tofile("input0.bin") 
    input_2.tofile("input1.bin") 

    output.tofile("golden.bin")


class TReduceParams:
    def __init__(self, dtype, global_row, global_col, tile_row, tile_col, valid_row, valid_col, op):
        self.dtype = dtype 
        self.global_row = global_row 
        self.global_col = global_col 
        self.tile_row = tile_row 
        self.tile_col = tile_col 
        self.valid_row = valid_row 
        self.valid_col = valid_col
        self.op = op

if __name__ == "__main__":
    #Get the absolute path of the script
    script_dir = os.path.dirname(os.path.abspath(__file__)) 
    testcases_dir = os.path.join(script_dir, "testcases")

    #Ensure the testcases directory exists
    if not os.path.exists(testcases_dir):
        os.makedirs(testcases_dir)

    case_name_list = [
        "TREDUCETest.case1",
        "TREDUCETest.case2",
    ]
    case_params_list = [
        TReduceParams(np.int32, 64, 64, 64, 64, 64, 64, np.maximum), 
        TReduceParams(np.int32, 64, 64, 64, 64, 64, 64, np.add), 
    ]

    for i, param in enumerate(case_params_list):
        case_name = case_name_list[i]
        if not os.path.exists(case_name):
            os.makedirs(case_name)
        original_dir = os.getcwd()
        os.chdir(case_name)
        gen_golden_data(case_name, param)
        os.chdir(original_dir)
