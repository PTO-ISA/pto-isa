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
#
# Golden data generation for tpushpop_vc_nosplit testcase.
#
# This testcase validates TPUSH/TPOP with TileSplitAxis::TILE_NO_SPLIT:
#   - A single Vec core (AIV0) dequantizes quantized B and pushes the full tile to L1.
#   - The Cube core pops the tile and performs matrix multiplication with srcA.
#
# Data layout:
#   x1_gm.bin      : srcA,   float32, shape [M, K]
#   quant_b_gm.bin : quantB, int8/int16, shape [K, N]  (row-major)
#   scale_gm.bin   : scale,  float32, shape [K]        (one scale per row of quantB)
#   offset_gm.bin  : offset, float32, shape [K]        (one offset per row of quantB)
#   golden.bin     : output, float32, shape [M, N]
#
# Dequantization formula (per-row, matching TDEQUANT instruction):
#   dequant_B[k, n] = (float(quantB[k, n]) - offset[k]) * scale[k]
#
# Golden computation:
#   golden = srcA @ dequant_B   ([M, K] @ [K, N] = [M, N])
# --------------------------------------------------------------------------------

import os
import numpy as np

np.random.seed(42)


class TPushPopVCNSParams:
    def __init__(self, name, quant_type, M, K, N):
        self.name = name
        self.quant_type = quant_type
        self.M = M
        self.K = K
        self.N = N


def gen_golden_data(param):
    M, K, N = param.M, param.K, param.N
    quant_type = param.quant_type

    # srcA: float32 matrix [M, K]
    srcA = np.random.uniform(-1.0, 1.0, (M, K)).astype(np.float32)

    # quantized B: int8 or int16 matrix [K, N]
    if quant_type == np.int8:
        quant_B = np.random.randint(-64, 64, (K, N)).astype(np.int8)
    else:
        quant_B = np.random.randint(-512, 512, (K, N)).astype(np.int16)

    # scale: float32 [K] — one positive scale per row of quantB
    scale = np.random.uniform(0.01, 0.1, (K,)).astype(np.float32)

    # offset: float32 [K] — one zero-point per row of quantB
    offset = np.random.uniform(-1.0, 1.0, (K,)).astype(np.float32)

    # dequantize B per row: dequant_B[k, n] = (quantB[k, n] - offset[k]) * scale[k]
    # Use float64 for intermediate computation to minimise rounding error
    dequant_B = (quant_B.astype(np.float64) - offset[:, np.newaxis].astype(np.float64)) * \
                scale[:, np.newaxis].astype(np.float64)

    # golden matmul in float64, then cast to float32 to match hardware output type
    golden = np.matmul(srcA.astype(np.float64), dequant_B).astype(np.float32)

    srcA.tofile("x1_gm.bin")
    quant_B.tofile("quant_b_gm.bin")
    scale.tofile("scale_gm.bin")
    offset.tofile("offset_gm.bin")
    golden.tofile("golden.bin")


if __name__ == "__main__":
    case_params_list = [
        TPushPopVCNSParams("TPushPopVCNSTest.case1_int8_single_k_tile",  np.int8,  16,  64, 32),
        TPushPopVCNSParams("TPushPopVCNSTest.case2_int8_two_k_tiles",    np.int8,  16, 128, 32),
        TPushPopVCNSParams("TPushPopVCNSTest.case3_int8_four_k_tiles",   np.int8,  16, 256, 32),
        TPushPopVCNSParams("TPushPopVCNSTest.case4_int16_single_k_tile", np.int16, 16,  64, 32),
        TPushPopVCNSParams("TPushPopVCNSTest.case5_int16_two_k_tiles",   np.int16, 16, 128, 32),
        TPushPopVCNSParams("TPushPopVCNSTest.case6_int16_four_k_tiles",  np.int16, 16, 256, 32),
    ]

    for param in case_params_list:
        if not os.path.exists(param.name):
            os.makedirs(param.name)
        original_dir = os.getcwd()
        os.chdir(param.name)
        gen_golden_data(param)
        os.chdir(original_dir)
        print(f"Generated: {param.name}  (M={param.M}, K={param.K}, N={param.N}, "
              f"quant={'int8' if param.quant_type == np.int8 else 'int16'})")
