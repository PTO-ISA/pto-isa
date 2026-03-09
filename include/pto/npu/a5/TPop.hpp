/**
Copyright (c) 2026 Huawei Technologies Co., Ltd.
This program is free software, you can redistribute it and/or modify it under the terms and conditions of
CANN Open Software License Agreement Version 2.0 (the "License").
Please refer to the License for details. You may not use this file except in compliance with the License.
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
See LICENSE in the root of the software repository for the full text of the License.
*/

#ifndef TPOP_HPP
#define TPOP_HPP

#include <type_traits>
#include <pto/common/fifo.hpp>
#include <pto/npu/a5/TStore.hpp>
#include <pto/npu/a5/TPush.hpp>

namespace pto {
/**
 * TPOP: Pop Tile from FIFO
 * * Flow:
 * 1. [Wait]    Wait for data ready (Cross-Core)
 * 2. [Load]    Load data from GM
 * 3. [Free]    Release GM space (Cross-Core)
 */
template <typename PipeCons, typename TileDataSrc, typename DataFiFo>
PTO_INTERNAL void TPOP_IMPL(PipeCons &cons, TileDataSrc &tile, DataFiFo &fifo)
{
    // // 1. Cross-Core: Wait for Data
    bool isWait = cons.getWaitStatus();
    if (isWait) {
        cons.wait();
    }

    // 2. Address Calculation & Pop
    bool reqFree = cons.pop(fifo, tile);

    // 3. Cross-Core: Free Space
    bool isFree = reqFree && cons.getFreeStatus();
    if (isFree) {
        cons.free();
    }
}

template <typename TileData, typename Pipe>
PTO_INTERNAL void TPOP_IMPL(TileData &tile, Pipe &pipe)
{
    TPOP_IMPL(pipe.cons, tile, pipe.fifo);
}

template <typename Pipe>
PTO_INTERNAL void TFREE_IMPL(Pipe &pipe)
{
    bool isFree = pipe.cons.getFreeStatus();
    if (isFree) {
        pipe.cons.free();
    }
}

} // namespace pto

#endif
