/**
Copyright (c) 2025 Huawei Technologies Co., Ltd.
This program is free software, you can redistribute it and/or modify it under the terms and conditions of
CANN Open Software License Agreement Version 2.0 (the "License").
Please refer to the License for details. You may not use this file except in compliance with the License.
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
See LICENSE in the root of the software repository for the full text of the License.
*/

#ifndef TPOP_HPP
#define TPOP_HPP

#include <pto/common/fifo.hpp>
#include <pto/cpu/TStore.hpp>
#include <pto/cpu/TPush.hpp>

namespace pto {
/**
 * TPOP: Pop Tile from FIFO
 * * Flow:
 * 1. [Wait]    Wait for data ready (Cross-Core)
 * 2. [Load]    Load data from GM
 * 3. [Free]    Release GM space (Cross-Core)
 */
template <typename TileData, typename Pipe>
PTO_INTERNAL void TPOP_IMPL(TileData &tile, Pipe &pipe)
{
    // 1. Cross-Core: Wait for Data
    pipe.cons.wait();

    // 2. Address Calculation & Load
    typename Pipe::DataFiFo::DType *addr;
    addr = pipe.fifo.getBasePtr() + TileData::Numel * (pipe.cons.get_tile_id() % Pipe::DataFiFo::fifoDepth);
    constexpr unsigned int cols = TileData::Cols;
    constexpr unsigned int rows = TileData::Rows;
    GlobalTensor<typename Pipe::DataFiFo::DType, Shape<1, 1, 1, rows, cols>,
                 Stride<rows * cols, rows * cols, rows * cols, cols, 1>>
        gt(addr);
    TLOAD(tile, gt);

    // 3. Cross-Core: Free Space
    pipe.cons.free();
}

template <typename Pipe, TileSplitAxis Split>
PTO_INTERNAL void TFREE_IMPL(Pipe &pipe)
{
    (void)Split;
    pipe.cons.free();
}

template <typename Pipe>
PTO_INTERNAL void TFREE_IMPL(Pipe &pipe)
{
    pipe.cons.free();
}

} // namespace pto

#endif
