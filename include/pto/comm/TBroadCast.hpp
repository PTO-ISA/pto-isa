/**
Copyright (c) 2025 Huawei Technologies Co., Ltd.
This program is free software, you can redistribute it and/or modify it under the terms and conditions of
CANN Open Software License Agreement Version 2.0 (the "License").
Please refer to the License for details. You may not use this file except in compliance with the License.
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
See LICENSE in the root of the software repository for the full text of the License.
*/

#ifndef PTO_COMM_TBROADCAST_HPP
#define PTO_COMM_TBROADCAST_HPP

#include <type_traits>

#include "pto/common/debug.h"
#include "pto/common/type.hpp"
#include "pto/common/constants.hpp"
#include "pto/common/pto_instr.hpp"
#include "pto/comm/comm_types.hpp"

namespace pto {
namespace comm {

// Ping-pong state for chunked TBROADCAST transfers
struct TbroadcastPingPongState {
    bool usePing = true;
    bool hasPending = false;
    int64_t pendingDstOffset = 0;
    int pendingRows = 0;
    int pendingCols = 0;
};

// TLOAD src chunk → sync → TSTORE to all ranks → sync
template <typename ParallelGroupType, typename TileData, typename SrcViewT, typename DstViewT, typename DynShapeT,
          typename DynStrideT>
PTO_INTERNAL void TbroadcastChunkTransfer(ParallelGroupType &parallelGroup, TileData &tile, SrcViewT &srcView,
                                          int64_t dstOffset, const DynShapeT &chunkShape,
                                          const DynStrideT &dstChunkStride, int nranks)
{
    TLOAD(tile, srcView);
    set_flag(PIPE_MTE2, PIPE_MTE3, EVENT_ID0);
    wait_flag(PIPE_MTE2, PIPE_MTE3, EVENT_ID0);
    for (int r = 0; r < nranks; ++r) {
        DstViewT dstView(parallelGroup[r].data() + dstOffset, chunkShape, dstChunkStride);
        TSTORE(dstView, tile);
    }
    set_flag(PIPE_MTE3, PIPE_MTE2, EVENT_ID0);
    wait_flag(PIPE_MTE3, PIPE_MTE2, EVENT_ID0);
}

// 2D sliding chunked broadcast with single buffer
template <typename ParallelGroupType, typename GlobalSrcData, typename TileData>
PTO_INTERNAL void TbroadcastChunkedSingle(ParallelGroupType &parallelGroup, GlobalSrcData &srcGlobalData,
                                          TileData &stagingTileData, int gShape0, int gShape1, int gShape2, int gShape3,
                                          int gShape4, int tileValidRow, int tileValidCol, int nranks)
{
    using GlobalDstData = typename ParallelGroupTraits<ParallelGroupType>::GlobalDataType;
    using T = typename GlobalSrcData::RawDType;
    constexpr bool isDynamicRow = (TileData::ValidRow == DYNAMIC);
    constexpr bool isDynamicCol = (TileData::ValidCol == DYNAMIC);

    const int srcStride0 = srcGlobalData.GetStride(GlobalTensorDim::DIM_0);
    const int srcStride1 = srcGlobalData.GetStride(GlobalTensorDim::DIM_1);
    const int srcStride2 = srcGlobalData.GetStride(GlobalTensorDim::DIM_2);
    const int srcStride3 = srcGlobalData.GetStride(GlobalTensorDim::DIM_3);
    const int srcStride4 = srcGlobalData.GetStride(GlobalTensorDim::DIM_4);
    const int dstStride0 = parallelGroup[0].GetStride(GlobalTensorDim::DIM_0);
    const int dstStride1 = parallelGroup[0].GetStride(GlobalTensorDim::DIM_1);
    const int dstStride2 = parallelGroup[0].GetStride(GlobalTensorDim::DIM_2);
    const int dstStride3 = parallelGroup[0].GetStride(GlobalTensorDim::DIM_3);
    const int dstStride4 = parallelGroup[0].GetStride(GlobalTensorDim::DIM_4);

    using DynShape = Shape<1, 1, 1, DYNAMIC, DYNAMIC>;
    using DynStride = Stride<DYNAMIC, DYNAMIC, DYNAMIC, DYNAMIC, DYNAMIC>;
    using SrcViewT = GlobalTensor<T, DynShape, DynStride, GlobalSrcData::layout>;
    using DstViewT = GlobalTensor<T, DynShape, DynStride, GlobalDstData::layout>;
    DynStride srcChunkStride(srcStride0, srcStride1, srcStride2, srcStride3, srcStride4);
    DynStride dstChunkStride(dstStride0, dstStride1, dstStride2, dstStride3, dstStride4);

    for (int i0 = 0; i0 < gShape0; ++i0) {
        for (int i1 = 0; i1 < gShape1; ++i1) {
            for (int i2 = 0; i2 < gShape2; ++i2) {
                int64_t srcBase = static_cast<int64_t>(i0) * srcStride0 + static_cast<int64_t>(i1) * srcStride1 +
                                  static_cast<int64_t>(i2) * srcStride2;
                int64_t dstBase = static_cast<int64_t>(i0) * dstStride0 + static_cast<int64_t>(i1) * dstStride1 +
                                  static_cast<int64_t>(i2) * dstStride2;
                for (int rowOff = 0; rowOff < gShape3; rowOff += tileValidRow) {
                    int curRows = (rowOff + tileValidRow <= gShape3) ? tileValidRow : (gShape3 - rowOff);
                    if constexpr (isDynamicRow)
                        stagingTileData.RowMaskInternal = curRows;
                    for (int colOff = 0; colOff < gShape4; colOff += tileValidCol) {
                        int curCols = (colOff + tileValidCol <= gShape4) ? tileValidCol : (gShape4 - colOff);
                        if constexpr (isDynamicCol)
                            stagingTileData.ColMaskInternal = curCols;
                        int64_t srcOff = srcBase + static_cast<int64_t>(rowOff) * srcStride3 +
                                         static_cast<int64_t>(colOff) * srcStride4;
                        int64_t dstOff = dstBase + static_cast<int64_t>(rowOff) * dstStride3 +
                                         static_cast<int64_t>(colOff) * dstStride4;
                        DynShape chunkShape(1, 1, 1, curRows, curCols);
                        SrcViewT srcView(srcGlobalData.data() + srcOff, chunkShape, srcChunkStride);
                        TbroadcastChunkTransfer<ParallelGroupType, TileData, SrcViewT, DstViewT>(
                            parallelGroup, stagingTileData, srcView, dstOff, chunkShape, dstChunkStride, nranks);
                    }
                }
            }
        }
    }
}

// ============================================================================
// TBROADCAST_IMPL: Broadcast data from root NPU to all ranks
//
// The root loads srcGlobalData and stores it to every rank's buffer in the
// ParallelGroup.
//
// When the GlobalTensor exceeds the UB tile capacity in rows and/or columns,
// the transfer is automatically chunked via 2D sliding:
//   - Outer dimensions (DIM_0, DIM_1, DIM_2) are iterated explicitly.
//   - DIM_3 (rows) is split into tileValidRow-sized chunks.
//   - DIM_4 (cols) is split into tileValidCol-sized chunks.
//
// Constraints for chunked mode:
//   - If TileData has static ValidRow, shape3 must be divisible by ValidRow.
//   - If TileData has static ValidCol, shape4 must be divisible by ValidCol.
//   - All ranks in the ParallelGroup are assumed to have the same shape/strides.
// ============================================================================

template <typename ParallelGroupType, typename GlobalSrcData, typename TileData>
PTO_INTERNAL void TBROADCAST_IMPL(ParallelGroupType &parallelGroup, GlobalSrcData &srcGlobalData,
                                  TileData &stagingTileData)
{
    using GlobalDstData = typename ParallelGroupTraits<ParallelGroupType>::GlobalDataType;
    using T = typename GlobalSrcData::RawDType;

    static_assert(std::is_same_v<T, typename TileData::DType>,
                  "TBROADCAST: TileData element type must match GlobalData element type");
    static_assert(std::is_same_v<T, typename GlobalDstData::RawDType>,
                  "TBROADCAST: ParallelGroup element type must match source element type");
    static_assert(GlobalSrcData::layout == GlobalDstData::layout, "TBROADCAST: src/dst layout mismatch");

    const int nranks = parallelGroup.GetSize();
    const int rootIdx = parallelGroup.GetRootIdx();

    PTO_ASSERT(nranks > 0, "ParallelGroup size must be greater than 0!");
    PTO_ASSERT(rootIdx >= 0 && rootIdx < nranks, "rootIdx must be in range [0, nranks)!");

    const int gShape0 = srcGlobalData.GetShape(GlobalTensorDim::DIM_0);
    const int gShape1 = srcGlobalData.GetShape(GlobalTensorDim::DIM_1);
    const int gShape2 = srcGlobalData.GetShape(GlobalTensorDim::DIM_2);
    const int gShape3 = srcGlobalData.GetShape(GlobalTensorDim::DIM_3);
    const int gShape4 = srcGlobalData.GetShape(GlobalTensorDim::DIM_4);

    const int64_t totalRows = static_cast<int64_t>(gShape0) * gShape1 * gShape2 * gShape3;
    const int tileValidRow = stagingTileData.GetValidRow();
    const int tileValidCol = stagingTileData.GetValidCol();

    PTO_ASSERT(tileValidRow > 0, "TBROADCAST: tileValidRow must be greater than 0");
    PTO_ASSERT(tileValidCol > 0, "TBROADCAST: tileValidCol must be greater than 0");

    if (totalRows == 0 || gShape4 == 0) {
        return;
    }

    if (nranks == 1) {
        TLOAD(stagingTileData, srcGlobalData);
        set_flag(PIPE_MTE2, PIPE_MTE3, EVENT_ID0);
        wait_flag(PIPE_MTE2, PIPE_MTE3, EVENT_ID0);
        TSTORE(parallelGroup[rootIdx], stagingTileData);
        set_flag(PIPE_MTE3, PIPE_MTE2, EVENT_ID0);
        wait_flag(PIPE_MTE3, PIPE_MTE2, EVENT_ID0);
        return;
    }

    // Simple path: data fits in UB tile
    if (totalRows <= tileValidRow && gShape4 <= tileValidCol) {
        TLOAD(stagingTileData, srcGlobalData);
        set_flag(PIPE_MTE2, PIPE_MTE3, EVENT_ID0);
        wait_flag(PIPE_MTE2, PIPE_MTE3, EVENT_ID0);
        for (int r = 0; r < nranks; ++r) {
            TSTORE(parallelGroup[r], stagingTileData);
            set_flag(PIPE_MTE3, PIPE_MTE2, EVENT_ID0);
            wait_flag(PIPE_MTE3, PIPE_MTE2, EVENT_ID0);
        }
        return;
    }

    // 2D sliding chunked path
    PTO_ASSERT(tileValidRow > 0, "TBROADCAST: tile ValidRow must be greater than 0 for chunked transfer");
    PTO_ASSERT(tileValidCol > 0, "TBROADCAST: tile ValidCol must be greater than 0 for chunked transfer");
    constexpr bool isDynamicRow = (TileData::ValidRow == DYNAMIC);
    constexpr bool isDynamicCol = (TileData::ValidCol == DYNAMIC);
    if constexpr (!isDynamicRow) {
        PTO_ASSERT(gShape3 % tileValidRow == 0,
                   "TBROADCAST chunked: shape3 must be divisible by tile ValidRow when ValidRow is static. "
                   "Use a Tile with DYNAMIC ValidRow for partial row chunk support.");
    }
    if constexpr (!isDynamicCol) {
        PTO_ASSERT(gShape4 % tileValidCol == 0,
                   "TBROADCAST chunked: shape4 must be divisible by tile ValidCol when ValidCol is static. "
                   "Use a Tile with DYNAMIC ValidCol for partial column chunk support.");
    }

    TbroadcastChunkedSingle<ParallelGroupType, GlobalSrcData, TileData>(parallelGroup, srcGlobalData, stagingTileData,
                                                                        gShape0, gShape1, gShape2, gShape3, gShape4,
                                                                        tileValidRow, tileValidCol, nranks);
}

// Process one chunk iteration with ping-pong for broadcast
template <typename ParallelGroupType, typename GlobalSrcData, typename TileData, typename DynStrideT>
PTO_INTERNAL void TbroadcastPingPongProcessChunk(ParallelGroupType &parallelGroup, GlobalSrcData &srcGlobalData,
                                                 TileData &pingTile, TileData &pongTile, int64_t srcOffset,
                                                 int64_t dstOffset, int currentRows, int currentCols,
                                                 const DynStrideT &srcChunkStride, const DynStrideT &dstChunkStride,
                                                 int nranks, TbroadcastPingPongState &state)
{
    using GlobalDstData = typename ParallelGroupTraits<ParallelGroupType>::GlobalDataType;
    using T = typename GlobalSrcData::RawDType;
    using DynShape = Shape<1, 1, 1, DYNAMIC, DYNAMIC>;
    using SrcViewT = GlobalTensor<T, DynShape, DynStrideT, GlobalSrcData::layout>;
    using DstViewT = GlobalTensor<T, DynShape, DynStrideT, GlobalDstData::layout>;
    constexpr bool isDynamicRow = (TileData::ValidRow == DYNAMIC);
    constexpr bool isDynamicCol = (TileData::ValidCol == DYNAMIC);

    TileData &loadTile = state.usePing ? pingTile : pongTile;
    event_t curEvent = state.usePing ? EVENT_ID0 : EVENT_ID1;

    if constexpr (isDynamicRow)
        loadTile.RowMaskInternal = currentRows;
    if constexpr (isDynamicCol)
        loadTile.ColMaskInternal = currentCols;

    DynShape chunkShape(1, 1, 1, currentRows, currentCols);
    SrcViewT srcView(srcGlobalData.data() + srcOffset, chunkShape, srcChunkStride);

    if (state.hasPending) {
        TileData &storeTile = state.usePing ? pongTile : pingTile;
        event_t prevEvent = state.usePing ? EVENT_ID1 : EVENT_ID0;

        wait_flag(PIPE_MTE2, PIPE_MTE3, prevEvent);

        DynShape pendShape(1, 1, 1, state.pendingRows, state.pendingCols);
        for (int r = 0; r < nranks; ++r) {
            DstViewT dstView(parallelGroup[r].data() + state.pendingDstOffset, pendShape, dstChunkStride);
            TSTORE(dstView, storeTile);
        }
        TLOAD(loadTile, srcView);

        set_flag(PIPE_MTE3, PIPE_MTE2, prevEvent);
        set_flag(PIPE_MTE2, PIPE_MTE3, curEvent);
        wait_flag(PIPE_MTE3, PIPE_MTE2, prevEvent);
    } else {
        TLOAD(loadTile, srcView);
        set_flag(PIPE_MTE2, PIPE_MTE3, curEvent);
    }

    state.pendingDstOffset = dstOffset;
    state.pendingRows = currentRows;
    state.pendingCols = currentCols;
    state.hasPending = true;
    state.usePing = !state.usePing;
}

// Drain last pending broadcast stores
template <typename ParallelGroupType, typename TileData, typename DynStrideT>
PTO_INTERNAL void TbroadcastPingPongEpilogue(ParallelGroupType &parallelGroup, TileData &pingTile, TileData &pongTile,
                                             const TbroadcastPingPongState &state, const DynStrideT &dstChunkStride,
                                             int nranks)
{
    using GlobalDstData = typename ParallelGroupTraits<ParallelGroupType>::GlobalDataType;
    using T = typename GlobalDstData::RawDType;
    if (!state.hasPending)
        return;
    using DynShape = Shape<1, 1, 1, DYNAMIC, DYNAMIC>;
    using DstViewT = GlobalTensor<T, DynShape, DynStrideT, GlobalDstData::layout>;

    TileData &lastTile = state.usePing ? pongTile : pingTile;
    event_t lastEvent = state.usePing ? EVENT_ID1 : EVENT_ID0;

    wait_flag(PIPE_MTE2, PIPE_MTE3, lastEvent);
    DynShape lastShape(1, 1, 1, state.pendingRows, state.pendingCols);
    for (int r = 0; r < nranks; ++r) {
        DstViewT dstView(parallelGroup[r].data() + state.pendingDstOffset, lastShape, dstChunkStride);
        TSTORE(dstView, lastTile);
    }
    set_flag(PIPE_MTE3, PIPE_MTE2, lastEvent);
    wait_flag(PIPE_MTE3, PIPE_MTE2, lastEvent);
}

// 2D sliding chunked broadcast with ping-pong double buffering
template <typename ParallelGroupType, typename GlobalSrcData, typename TileData>
PTO_INTERNAL void TbroadcastChunkedPingPong(ParallelGroupType &parallelGroup, GlobalSrcData &srcGlobalData,
                                            TileData &pingTile, TileData &pongTile, int gShape0, int gShape1,
                                            int gShape2, int gShape3, int gShape4, int tileValidRow, int tileValidCol,
                                            int nranks)
{
    const int srcStride0 = srcGlobalData.GetStride(GlobalTensorDim::DIM_0);
    const int srcStride1 = srcGlobalData.GetStride(GlobalTensorDim::DIM_1);
    const int srcStride2 = srcGlobalData.GetStride(GlobalTensorDim::DIM_2);
    const int srcStride3 = srcGlobalData.GetStride(GlobalTensorDim::DIM_3);
    const int srcStride4 = srcGlobalData.GetStride(GlobalTensorDim::DIM_4);
    const int dstStride0 = parallelGroup[0].GetStride(GlobalTensorDim::DIM_0);
    const int dstStride1 = parallelGroup[0].GetStride(GlobalTensorDim::DIM_1);
    const int dstStride2 = parallelGroup[0].GetStride(GlobalTensorDim::DIM_2);
    const int dstStride3 = parallelGroup[0].GetStride(GlobalTensorDim::DIM_3);
    const int dstStride4 = parallelGroup[0].GetStride(GlobalTensorDim::DIM_4);

    using DynStride = Stride<DYNAMIC, DYNAMIC, DYNAMIC, DYNAMIC, DYNAMIC>;
    DynStride srcChunkStride(srcStride0, srcStride1, srcStride2, srcStride3, srcStride4);
    DynStride dstChunkStride(dstStride0, dstStride1, dstStride2, dstStride3, dstStride4);
    TbroadcastPingPongState state;

    for (int i0 = 0; i0 < gShape0; ++i0) {
        for (int i1 = 0; i1 < gShape1; ++i1) {
            for (int i2 = 0; i2 < gShape2; ++i2) {
                int64_t srcBase = static_cast<int64_t>(i0) * srcStride0 + static_cast<int64_t>(i1) * srcStride1 +
                                  static_cast<int64_t>(i2) * srcStride2;
                int64_t dstBase = static_cast<int64_t>(i0) * dstStride0 + static_cast<int64_t>(i1) * dstStride1 +
                                  static_cast<int64_t>(i2) * dstStride2;
                for (int rowOff = 0; rowOff < gShape3; rowOff += tileValidRow) {
                    int curRows = (rowOff + tileValidRow <= gShape3) ? tileValidRow : (gShape3 - rowOff);
                    for (int colOff = 0; colOff < gShape4; colOff += tileValidCol) {
                        int curCols = (colOff + tileValidCol <= gShape4) ? tileValidCol : (gShape4 - colOff);
                        int64_t srcOff = srcBase + static_cast<int64_t>(rowOff) * srcStride3 +
                                         static_cast<int64_t>(colOff) * srcStride4;
                        int64_t dstOff = dstBase + static_cast<int64_t>(rowOff) * dstStride3 +
                                         static_cast<int64_t>(colOff) * dstStride4;
                        TbroadcastPingPongProcessChunk<ParallelGroupType, GlobalSrcData, TileData>(
                            parallelGroup, srcGlobalData, pingTile, pongTile, srcOff, dstOff, curRows, curCols,
                            srcChunkStride, dstChunkStride, nranks, state);
                    }
                }
            }
        }
    }

    TbroadcastPingPongEpilogue<ParallelGroupType, TileData>(parallelGroup, pingTile, pongTile, state, dstChunkStride,
                                                            nranks);
}

// ============================================================================
// TBROADCAST_IMPL (ping-pong): Broadcast with double buffering
//
// Uses two staging tiles (pingTile, pongTile) to overlap TLOAD of the next
// chunk (MTE2) with TSTORE of the current chunk to all ranks (MTE3).
//
// Timeline without ping-pong:
//   [TLOAD chunk0] -> [N×TSTORE chunk0] -> [TLOAD chunk1] -> [N×TSTORE chunk1] -> ...
//
// Timeline with ping-pong:
//   [TLOAD chunk0] -> [N×TSTORE chunk0 | TLOAD chunk1] -> [N×TSTORE chunk1 | TLOAD chunk2] -> ...
//
// Constraints: same as TBROADCAST_IMPL for chunked mode.
// ============================================================================

template <typename ParallelGroupType, typename GlobalSrcData, typename TileData>
PTO_INTERNAL void TBROADCAST_IMPL(ParallelGroupType &parallelGroup, GlobalSrcData &srcGlobalData, TileData &pingTile,
                                  TileData &pongTile)
{
    using GlobalDstData = typename ParallelGroupTraits<ParallelGroupType>::GlobalDataType;
    using T = typename GlobalSrcData::RawDType;

    static_assert(std::is_same_v<T, typename TileData::DType>,
                  "TBROADCAST: TileData element type must match GlobalData element type");
    static_assert(std::is_same_v<T, typename GlobalDstData::RawDType>,
                  "TBROADCAST: ParallelGroup element type must match source element type");
    static_assert(GlobalSrcData::layout == GlobalDstData::layout, "TBROADCAST: src/dst layout mismatch");

    const int nranks = parallelGroup.GetSize();
    const int rootIdx = parallelGroup.GetRootIdx();

    PTO_ASSERT(nranks > 0, "ParallelGroup size must be greater than 0!");
    PTO_ASSERT(rootIdx >= 0 && rootIdx < nranks, "rootIdx must be in range [0, nranks)!");

    const int gShape0 = srcGlobalData.GetShape(GlobalTensorDim::DIM_0);
    const int gShape1 = srcGlobalData.GetShape(GlobalTensorDim::DIM_1);
    const int gShape2 = srcGlobalData.GetShape(GlobalTensorDim::DIM_2);
    const int gShape3 = srcGlobalData.GetShape(GlobalTensorDim::DIM_3);
    const int gShape4 = srcGlobalData.GetShape(GlobalTensorDim::DIM_4);

    const int64_t totalRows = static_cast<int64_t>(gShape0) * gShape1 * gShape2 * gShape3;
    const int tileValidRow = pingTile.GetValidRow();
    const int tileValidCol = pingTile.GetValidCol();

    PTO_ASSERT(tileValidRow > 0, "TBROADCAST: tileValidRow must be greater than 0");
    PTO_ASSERT(tileValidCol > 0, "TBROADCAST: tileValidCol must be greater than 0");

    if (totalRows == 0 || gShape4 == 0) {
        return;
    }

    if (nranks == 1) {
        TLOAD(pingTile, srcGlobalData);
        set_flag(PIPE_MTE2, PIPE_MTE3, EVENT_ID0);
        wait_flag(PIPE_MTE2, PIPE_MTE3, EVENT_ID0);
        TSTORE(parallelGroup[rootIdx], pingTile);
        set_flag(PIPE_MTE3, PIPE_MTE2, EVENT_ID0);
        wait_flag(PIPE_MTE3, PIPE_MTE2, EVENT_ID0);
        return;
    }

    // Simple path: single chunk, no ping-pong benefit
    if (totalRows <= tileValidRow && gShape4 <= tileValidCol) {
        TLOAD(pingTile, srcGlobalData);
        set_flag(PIPE_MTE2, PIPE_MTE3, EVENT_ID0);
        wait_flag(PIPE_MTE2, PIPE_MTE3, EVENT_ID0);
        for (int r = 0; r < nranks; ++r) {
            TSTORE(parallelGroup[r], pingTile);
            set_flag(PIPE_MTE3, PIPE_MTE2, EVENT_ID0);
            wait_flag(PIPE_MTE3, PIPE_MTE2, EVENT_ID0);
        }
        return;
    }

    // 2D sliding chunked path with ping-pong double buffering
    constexpr bool isDynamicRow = (TileData::ValidRow == DYNAMIC);
    constexpr bool isDynamicCol = (TileData::ValidCol == DYNAMIC);
    if constexpr (!isDynamicRow) {
        PTO_ASSERT(gShape3 % tileValidRow == 0,
                   "TBROADCAST chunked: shape3 must be divisible by tile ValidRow when ValidRow is static. "
                   "Use a Tile with DYNAMIC ValidRow for partial row chunk support.");
    }
    if constexpr (!isDynamicCol) {
        PTO_ASSERT(gShape4 % tileValidCol == 0,
                   "TBROADCAST chunked: shape4 must be divisible by tile ValidCol when ValidCol is static. "
                   "Use a Tile with DYNAMIC ValidCol for partial column chunk support.");
    }

    TbroadcastChunkedPingPong<ParallelGroupType, GlobalSrcData, TileData>(parallelGroup, srcGlobalData, pingTile,
                                                                          pongTile, gShape0, gShape1, gShape2, gShape3,
                                                                          gShape4, tileValidRow, tileValidCol, nranks);
}

} // namespace comm
} // namespace pto

#endif // PTO_COMM_TBROADCAST_HPP
