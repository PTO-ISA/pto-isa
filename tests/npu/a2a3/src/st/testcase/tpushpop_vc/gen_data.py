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

np.random.seed(19)


def gen_golden_data(case_name, m, k, n, quant_type, input_type, output_type):
    """
    K-tiling with partial sum accumulation.

    Data layout:
    - A: [M, K] - Matrix A
    - quantB: [K, N] - Quantized Matrix B (shared, quantized along K dimension)
    - scale: [K] - Per-row scale for B
    - offset: [K] - Per-row offset for B

    Computation:
    Output[m, n] = sum_k(A[m, k] * (B_quant[k, n] - offset[k]) * scale[k])
    """
    # Matrix A: [M, K]
    x1_gm = np.random.uniform(-2, 2, [m, k]).astype(input_type)

    # Per-K-row scale and offset
    scale_gm = np.random.uniform(0.01, 0.1, [k]).astype(output_type)
    offset_gm = np.random.uniform(-1, 1, [k]).astype(output_type)

    # Matrix B: [K, N]
    B_float = np.random.uniform(-2, 2, [k, n]).astype(output_type)

    # Quantize B using per-row scale/offset
    if quant_type == np.int8:
        B_quant_raw = B_float / scale_gm[:, None] + offset_gm[:, None]
        quant_b_gm = np.clip(np.round(B_quant_raw), -128, 127).astype(np.int8)
    else:  # int16
        B_quant_raw = B_float / scale_gm[:, None] + offset_gm[:, None]
        quant_b_gm = np.clip(np.round(B_quant_raw), -32768, 32767).astype(np.int16)

    # Save inputs
    x1_gm.tofile("./x1_gm.bin")
    quant_b_gm.tofile("./quant_b_gm.bin")
    scale_gm.tofile("./scale_gm.bin")
    offset_gm.tofile("./offset_gm.bin")

    # Compute golden: dequantize B and compute matmul
    # B_dequant[k, n] = (B_quant[k, n] - offset[k]) * scale[k]
    B_dequant = (quant_b_gm.astype(output_type) - offset_gm[:, None]) * scale_gm[:, None]

    # Output = A @ B_dequant
    golden = np.matmul(x1_gm.astype(output_type), B_dequant).astype(output_type)
    golden.tofile("./golden.bin")


if __name__ == "__main__":
    case_name_list = [
        "TPushPopVCTest.case1_int8_single_k_tile",
        "TPushPopVCTest.case2_int8_two_k_tiles",
        "TPushPopVCTest.case3_int8_four_k_tiles",
        "TPushPopVCTest.case4_int16_single_k_tile",
        "TPushPopVCTest.case5_int16_two_k_tiles",
        "TPushPopVCTest.case6_int16_four_k_tiles",
    ]

    # M=16 fixed, K varies for K-tiling test, TILE_K=64
    case_params_list = [
        # int8 cases: M=16, K varies, TILE_K=64, InT=float
        (16, 64, 32, np.int8, np.float32, np.float32),  # case1: K=64, NUM_K_TILES=1
        (16, 128, 32, np.int8, np.float32, np.float32),  # case2: K=128, NUM_K_TILES=2
        (16, 256, 32, np.int8, np.float32, np.float32),  # case3: K=256, NUM_K_TILES=4 (FIFO wrapping)
        # int16 cases: M=16, K varies, TILE_K=64, InT=float
        (16, 64, 32, np.int16, np.float32, np.float32),  # case4: K=64, NUM_K_TILES=1
        (16, 128, 32, np.int16, np.float32, np.float32),  # case5: K=128, NUM_K_TILES=2
        (16, 256, 32, np.int16, np.float32, np.float32),  # case6: K=256, NUM_K_TILES=4 (FIFO wrapping)
    ]

    for i, case_name in enumerate(case_name_list):
        if not os.path.exists(case_name):
            os.makedirs(case_name)
        original_dir = os.getcwd()
        os.chdir(case_name)
        m, k, n, quant_type, input_type, output_type = case_params_list[i]
        gen_golden_data(case_name, m, k, n, quant_type, input_type, output_type)
        os.chdir(original_dir)
