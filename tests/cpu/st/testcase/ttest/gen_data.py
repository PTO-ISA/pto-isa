#!/usr/bin/python3
# coding=utf-8
# --------------------------------------------------------------------------------
# Copyright (c) 2025 Huawei Technologies Co., Ltd.
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

    #Generate input data
    if param.cmp == "LE":
        input_arr = np.random.randint(1, 100, size=[row, col]).astype(dtype)
        threshold_true = input_arr.max()
        threshold_false = input_arr.min()
    elif param.cmp == "GE":
        input_arr = np.random.randint(1, 100, size=[row, col]).astype(dtype)
        threshold_true = input_arr.min()
        threshold_false = input_arr.max()
    elif param.cmp == "EQ":
        input_arr = np.ones([row, col]).astype(dtype)
        threshold_true = 1
        threshold_false = 2

    with open("./cmp_file.bin", 'wb') as f:
        f.write(struct.pack('ii', threshold_true, threshold_false))

    #Save the input and golden data to binary files
    input_arr.tofile("input.bin") 
    
    with open("./golden.bin", 'wb') as f:
        f.write(struct.pack('??', True, False))


class TTestParams:
    def __init__(self, dtype, global_row, global_col, tile_row, tile_col, valid_row, valid_col, cmp):
        self.dtype = dtype 
        self.global_row = global_row 
        self.global_col = global_col 
        self.tile_row = tile_row 
        self.tile_col = tile_col 
        self.valid_row = valid_row 
        self.valid_col = valid_col
        self.cmp = cmp

if __name__ == "__main__":
    #Get the absolute path of the script
    script_dir = os.path.dirname(os.path.abspath(__file__)) 
    testcases_dir = os.path.join(script_dir, "testcases")

    #Ensure the testcases directory exists
    if not os.path.exists(testcases_dir):
        os.makedirs(testcases_dir)

    case_params_list = [
        TTestParams(np.int32, 64, 64, 64, 64, 64, 64, 'LE'), 
        TTestParams(np.int32, 64, 64, 64, 64, 64, 64, 'GE'), 
        TTestParams(np.int32, 64, 64, 64, 64, 64, 64, 'EQ'), 
        TTestParams(np.int32, 16, 256, 16, 256, 16, 256, 'LE'),
        TTestParams(np.int32, 16, 256, 16, 256, 16, 256, 'GE'),
        TTestParams(np.int32, 16, 256, 16, 256, 16, 256, 'EQ'),
    ]

    for i, param in enumerate(case_params_list):
        case_name = f"TTESTTest.case{i+1}"
        if not os.path.exists(case_name):
            os.makedirs(case_name)
        original_dir = os.getcwd()
        os.chdir(case_name)
        gen_golden_data(case_name, param)
        os.chdir(original_dir)
