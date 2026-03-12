#!/usr/bin/python3
# coding=utf-8
# --------------------------------------------------------------------------------
# Copyright (c) 2026 Huawei Technologies Co., Ltd.
# This program is free software, you can redistribute it and/or modify it under the
# terms and conditions of CANN Open Software License Agreement Version 2.0 (the "License").
# Please refer to the License for details. You may not use this file except in compliance
# with the License.
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY,
# OR FITNESS FOR A PARTICULAR PURPOSE.
# See LICENSE in the root of the software repository for the full text of the License.
# --------------------------------------------------------------------------------

import random
import csv
import torch
import torch_npu
from jit_util_flash import jit_compile_flash

NUM_ITERATIONS = 50
WARMUP = 10
SEED = 1

random.seed(SEED)
torch.manual_seed(SEED)
torch.npu.manual_seed(SEED)


def attn_flops_matmul_softmax_scale(
    batch_size: int,
    s_q: int,
    s_k: int,
    h: int,
    include_scale: bool = True,
    count_exp_as_flop: bool = True,
    count_max_as_flop: bool = True,
):
    # 1) Matmuls
    flops_matmul = 4 * batch_size * s_q * s_k * h

    # 2) Scale
    flops_scale = (batch_size * s_q * s_k) if include_scale else 0

    # 3) Softmax
    rows = batch_size * s_q
    softmax_ops = 0

    if count_max_as_flop:
        softmax_ops += rows * (s_k - 1)  # max reduction comparisons

    softmax_ops += rows * s_k  # subtract max
    if count_exp_as_flop:
        softmax_ops += rows * s_k  # exp
    softmax_ops += rows * (s_k - 1)  # sum reduction
    softmax_ops += rows * s_k  # normalize (div or mul)

    total = flops_matmul + flops_scale + softmax_ops
    return {
        "total": total,
        "matmul": flops_matmul,
        "scale": flops_scale,
        "softmax": softmax_ops,
    }


def tflops(flops, ms):
    return flops / (ms * 1e-3) / 1e12


def time_npu(fn, iters=NUM_ITERATIONS, warmup=WARMUP):
    for _ in range(warmup):
        _ = fn()
    torch.npu.synchronize()

    start = torch.npu.Event(enable_timing=True)
    end = torch.npu.Event(enable_timing=True)

    start.record()
    for _ in range(iters):
        _ = fn()
    end.record()
    torch.npu.synchronize()

    return start.elapsed_time(end) / iters


# ---------------------------
# 2) Fused attention
# ---------------------------
def fused_reference(q_bsh, k_bsh, v_bsh):
    o, _ = torch_npu.npu_fused_infer_attention_score(
        q_bsh, k_bsh, v_bsh, input_layout="BSH"
    )
    return o


def bench(
    csv_path="jit_attn_bench.csv",
    sqs=(128, 256, 512, 1024, 2048),
    sks=(1024, 2048, 4096, 8192),
    head_size=128,
    scale=True,
    check=False,
    rtol=0.3,
    atol=1e-1,
):
    device = "npu"
    torch.npu.set_device(device)
    dtype = torch.float16
    batch_size = 1

    rows_out = []
    header = ["sq", "sk", "head_size", "kernel", "time_us", "tflops", "flops_total"]

    # Compile JIT flash once
    flash = jit_compile_flash(verbose=False)

    for sq in sqs:
        for sk in sks:
            # Inputs
            q = torch.rand((sq, head_size), device=device, dtype=dtype)
            k = torch.rand((sk, head_size), device=device, dtype=dtype)
            v = torch.rand((sk, head_size), device=device, dtype=dtype)

            # FLOPs: matmul + softmax (+scale)
            flops_dict = attn_flops_matmul_softmax_scale(
                batch_size,
                sq,
                sk,
                head_size,
                include_scale=scale,
            )
            flops_total = flops_dict["total"]

            # Fused inputs (BSH)
            q_bsh = q.unsqueeze(0)
            k_bsh = k.unsqueeze(0)
            v_bsh = v.unsqueeze(0)

            # JIT flash buffers
            num_tiles = sq // 128

            o_out = torch.empty((sq, head_size), device=device, dtype=torch.float32)

            out_device = torch.empty((sq, sk), device=device, dtype=torch.float32)
            xexp_device = torch.empty((sq, sk), device=device, dtype=torch.float16)
            pout_fp32_device = torch.empty((sq, sk), device=device, dtype=torch.float32)

            out_2d_device = torch.empty((num_tiles, sq, head_size), device=device, dtype=torch.float32)
            g_sum_device = torch.empty((num_tiles, sq), device=device, dtype=torch.float32)
            exp_max_device = torch.empty((num_tiles, sq), device=device, dtype=torch.float32)
            o_parts_device = torch.empty((num_tiles, sq, head_size), device=device, dtype=torch.float32)

            ms_fused = time_npu(lambda: fused_reference(q_bsh, k_bsh, v_bsh))

            ms_jit = time_npu(
                lambda: flash(
                    q,
                    k,
                    v,
                    o_out,
                    out_device,
                    xexp_device,
                    pout_fp32_device,
                    out_2d_device,
                    g_sum_device,
                    exp_max_device,
                    o_parts_device,
                )
            )

            # Correctness check: fused vs flash (run once per shape, not timed)
            if check:
                # Reference: fused (1, sq, head) -> (sq, head) fp32
                fused_out = (fused_reference(q_bsh, k_bsh, v_bsh).squeeze(0).to(torch.float32))
                torch.testing.assert_close(o_out, fused_out, rtol=rtol, atol=atol)

            def add_row(kernel_name, ms, flops_total, sq, sk):
                time_us = ms * 1000.0
                perf = tflops(flops_total, ms)
                rows_out.append(
                    [
                        sq,
                        sk,
                        head_size,
                        kernel_name,
                        f"{time_us:.3f}",
                        f"{perf:.6f}",
                        int(flops_total),
                    ]
                )

            add_row("npu_fused_attention", ms_fused, flops_total, sq, sk)
            add_row("jit_flash", ms_jit, flops_total, sq, sk)

            print(
                f"done sq={sq}, sk={sk} | "
                f"fused {ms_fused*1000:.2f}us  "
                f"jit {ms_jit*1000:.2f}us" + ("" if not check else "  (checked)")
            )

    # Write benchmark results
    with open(csv_path, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(header)
        writer.writerows(rows_out)


if __name__ == "__main__":
    bench()
