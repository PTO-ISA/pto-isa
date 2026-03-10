/**
Copyright (c) 2025 Huawei Technologies Co., Ltd.
This program is free software, you can redistribute it and/or modify it under the terms and conditions of
CANN Open Software License Agreement Version 2.0 (the "License").
Please refer to the License for details. You may not use this file except in compliance with the License.
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
See LICENSE in the root of the software repository for the full text of the License.
*/

#ifndef PTO_COMM_TSCATTER_HPP
#define PTO_COMM_TSCATTER_HPP

#include <type_traits>

#include "pto/common/debug.h"
#include "pto/common/type.hpp"
#include "pto/common/constants.hpp"
#include "pto/common/pto_instr.hpp"
#include "pto/comm/comm_types.hpp"

namespace pto {
namespace comm {

// Ping-pong double-buffering state for chunked TSCATTER transfers
struct TscatterPingPongState {
    bool usePing = true;
    bool hasPending = false;
    int64_t pendingDstOffset = 0;
    int pendingRank = 0;
    int pendingRows = 0;
    int pendingCols = 0;
};

// Simple scatter path: per-rank data fits in a single tile, loop over all ranks
template <typename ParallelGroupType, typename GlobalSrcData, typename TileData>
PTO_INTERNAL void TscatterSimple(ParallelGroupType &parallelGroup, GlobalSrcData &srcGlobalData,
                                 TileData &stagingTileData, int gShape0, int gShape1, int gShape2, int gShape3,
                                 int gShape4, int nranks, int perRankRows)
{
    using T = typename GlobalSrcData::RawDType;
    const int srcStride3 = srcGlobalData.GetStride(GlobalTensorDim::DIM_3);

    using DynShape5D = Shape<DYNAMIC, DYNAMIC, DYNAMIC, DYNAMIC, DYNAMIC>;
    using DynStride = Stride<DYNAMIC, DYNAMIC, DYNAMIC, DYNAMIC, DYNAMIC>;
    using SrcViewT = GlobalTensor<T, DynShape5D, DynStride, GlobalSrcData::layout>;

    DynShape5D perRankShape(gShape0, gShape1, gShape2, gShape3, gShape4);
    DynStride srcViewStride(
        srcGlobalData.GetStride(GlobalTensorDim::DIM_0), srcGlobalData.GetStride(GlobalTensorDim::DIM_1),
        srcGlobalData.GetStride(GlobalTensorDim::DIM_2), srcStride3, srcGlobalData.GetStride(GlobalTensorDim::DIM_4));

    for (int r = 0; r < nranks; ++r) {
        int64_t srcOffset = static_cast<int64_t>(r) * perRankRows * srcStride3;
        SrcViewT srcView(srcGlobalData.data() + srcOffset, perRankShape, srcViewStride);
        TLOAD(stagingTileData, srcView);
        set_flag(PIPE_MTE2, PIPE_MTE3, EVENT_ID0);
        wait_flag(PIPE_MTE2, PIPE_MTE3, EVENT_ID0);
        TSTORE(parallelGroup[r], stagingTileData);
        set_flag(PIPE_MTE3, PIPE_MTE2, EVENT_ID0);
        wait_flag(PIPE_MTE3, PIPE_MTE2, EVENT_ID0);
    }
}

// 2D sliding chunked scatter with single buffer
template <typename ParallelGroupType, typename GlobalSrcData, typename TileData>
PTO_INTERNAL void TscatterChunkedSingle(ParallelGroupType &parallelGroup, GlobalSrcData &srcGlobalData,
                                        TileData &stagingTileData, int gShape0, int gShape1, int gShape2, int gShape3,
                                        int gShape4, int tileValidRow, int tileValidCol, int nranks, int perRankRows)
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

    for (int r = 0; r < nranks; ++r) {
        int64_t rankSrcBase = static_cast<int64_t>(r) * perRankRows * srcStride3;
        for (int i0 = 0; i0 < gShape0; ++i0) {
            for (int i1 = 0; i1 < gShape1; ++i1) {
                for (int i2 = 0; i2 < gShape2; ++i2) {
                    int64_t srcBase = rankSrcBase + static_cast<int64_t>(i0) * srcStride0 +
                                      static_cast<int64_t>(i1) * srcStride1 + static_cast<int64_t>(i2) * srcStride2;
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
                            TLOAD(stagingTileData, srcView);
                            set_flag(PIPE_MTE2, PIPE_MTE3, EVENT_ID0);
                            wait_flag(PIPE_MTE2, PIPE_MTE3, EVENT_ID0);
                            DstViewT dstView(parallelGroup[r].data() + dstOff, chunkShape, dstChunkStride);
                            TSTORE(dstView, stagingTileData);
                            set_flag(PIPE_MTE3, PIPE_MTE2, EVENT_ID0);
                            wait_flag(PIPE_MTE3, PIPE_MTE2, EVENT_ID0);
                        }
                    }
                }
            }
        }
    }
}

// ============================================================================
// TSCATTER_IMPL: Scatter operation - root distributes data to all ranks
//
// The calling NPU (root) splits local source data along DIM_3 (row dimension)
// and distributes the portions to each rank in the parallel group.
// This is the inverse of TGATHER.
//
// The source tensor has shape (D0, D1, D2, N*H, W). Each rank r receives
// rows [r*H, (r+1)*H), i.e., per-rank data of shape (D0, D1, D2, H, W).
//
// When the per-rank data exceeds the UB tile capacity in rows and/or columns,
// the transfer is automatically chunked via 2D sliding:
//   - Outer dimensions (DIM_0, DIM_1, DIM_2) are iterated explicitly.
//   - DIM_3 (rows) is split into tileValidRow-sized chunks.
//   - DIM_4 (cols) is split into tileValidCol-sized chunks.
//
// Constraints for chunked mode:
//   - If TileData has static ValidRow, per-rank DIM_3 must be divisible by ValidRow.
//   - If TileData has static ValidCol, DIM_4 must be divisible by ValidCol.
//   - All destination tensors in the ParallelGroup are assumed to have the same shape/strides.
// ============================================================================

template <typename ParallelGroupType, typename GlobalSrcData, typename TileData>
PTO_INTERNAL void TSCATTER_IMPL(ParallelGroupType &parallelGroup, GlobalSrcData &srcGlobalData,
                                TileData &stagingTileData)
{
    using GlobalDstData = typename ParallelGroupTraits<ParallelGroupType>::GlobalDataType;
    using T = typename GlobalSrcData::RawDType;

    static_assert(std::is_same_v<T, typename GlobalDstData::RawDType>, "TSCATTER: GlobalData type mismatch!");
    static_assert(std::is_same_v<T, typename TileData::DType>,
                  "TSCATTER: TileData element type must match GlobalData element type");
    static_assert(GlobalSrcData::layout == GlobalDstData::layout, "TSCATTER: src/dst layout mismatch");

    const int nranks = parallelGroup.GetSize();
    const int rootIdx = parallelGroup.GetRootIdx();

    PTO_ASSERT(nranks > 0, "ParallelGroup size must be greater than 0!");
    PTO_ASSERT(rootIdx >= 0 && rootIdx < nranks, "rootIdx must be in range [0, nranks)!");

    const int gShape0 = parallelGroup[0].GetShape(GlobalTensorDim::DIM_0);
    const int gShape1 = parallelGroup[0].GetShape(GlobalTensorDim::DIM_1);
    const int gShape2 = parallelGroup[0].GetShape(GlobalTensorDim::DIM_2);
    const int gShape3 = parallelGroup[0].GetShape(GlobalTensorDim::DIM_3);
    const int gShape4 = parallelGroup[0].GetShape(GlobalTensorDim::DIM_4);

    const int perRankRows = gShape3;
    const int64_t totalRows = static_cast<int64_t>(gShape0) * gShape1 * gShape2 * gShape3;
    const int tileValidRow = stagingTileData.GetValidRow();
    const int tileValidCol = stagingTileData.GetValidCol();

    PTO_ASSERT(tileValidRow > 0, "TSCATTER: tileValidRow must be greater than 0");
    PTO_ASSERT(tileValidCol > 0, "TSCATTER: tileValidCol must be greater than 0");

    if (totalRows == 0 || gShape4 == 0) {
        return;
    }

    // Simple path: per-rank data fits in UB tile
    if (totalRows <= tileValidRow && gShape4 <= tileValidCol) {
        TscatterSimple<ParallelGroupType, GlobalSrcData, TileData>(parallelGroup, srcGlobalData, stagingTileData,
                                                                   gShape0, gShape1, gShape2, gShape3, gShape4, nranks,
                                                                   perRankRows);
        return;
    }

    // 2D sliding chunked path
    constexpr bool isDynamicRow = (TileData::ValidRow == DYNAMIC);
    constexpr bool isDynamicCol = (TileData::ValidCol == DYNAMIC);

    if constexpr (!isDynamicRow) {
        PTO_ASSERT(gShape3 % tileValidRow == 0,
                   "TSCATTER chunked: per-rank DIM_3 must be divisible by tile ValidRow when static. "
                   "Use a Tile with DYNAMIC ValidRow for partial row chunk support.");
    }
    if constexpr (!isDynamicCol) {
        PTO_ASSERT(gShape4 % tileValidCol == 0,
                   "TSCATTER chunked: DIM_4 must be divisible by tile ValidCol when static. "
                   "Use a Tile with DYNAMIC ValidCol for partial column chunk support.");
    }

    TscatterChunkedSingle<ParallelGroupType, GlobalSrcData, TileData>(parallelGroup, srcGlobalData, stagingTileData,
                                                                      gShape0, gShape1, gShape2, gShape3, gShape4,
                                                                      tileValidRow, tileValidCol, nranks, perRankRows);
}

// Process one chunk iteration with ping-pong double buffering for scatter
template <typename ParallelGroupType, typename GlobalSrcData, typename TileData, typename DynStrideT>
PTO_INTERNAL void TscatterPingPongProcessChunk(ParallelGroupType &parallelGroup, GlobalSrcData &srcGlobalData,
                                               TileData &pingTile, TileData &pongTile, int64_t srcOffset,
                                               int64_t dstOffset, int currentRows, int currentCols,
                                               const DynStrideT &srcChunkStride, const DynStrideT &dstChunkStride,
                                               int rank, TscatterPingPongState &state)
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
        DstViewT dstView(parallelGroup[state.pendingRank].data() + state.pendingDstOffset, pendShape, dstChunkStride);

        TSTORE(dstView, storeTile);
        TLOAD(loadTile, srcView);

        set_flag(PIPE_MTE3, PIPE_MTE2, prevEvent);
        set_flag(PIPE_MTE2, PIPE_MTE3, curEvent);

        wait_flag(PIPE_MTE3, PIPE_MTE2, prevEvent);
    } else {
        TLOAD(loadTile, srcView);
        set_flag(PIPE_MTE2, PIPE_MTE3, curEvent);
    }

    state.pendingDstOffset = dstOffset;
    state.pendingRank = rank;
    state.pendingRows = currentRows;
    state.pendingCols = currentCols;
    state.hasPending = true;
    state.usePing = !state.usePing;
}

// Drain the last pending TSTORE after the chunked ping-pong loop completes
template <typename ParallelGroupType, typename TileData, typename DynStrideT>
PTO_INTERNAL void TscatterPingPongEpilogue(ParallelGroupType &parallelGroup, TileData &pingTile, TileData &pongTile,
                                           const TscatterPingPongState &state, const DynStrideT &dstChunkStride)
{
    if (!state.hasPending)
        return;
    using GlobalDstData = typename ParallelGroupTraits<ParallelGroupType>::GlobalDataType;
    using T = typename GlobalDstData::RawDType;
    using DynShape = Shape<1, 1, 1, DYNAMIC, DYNAMIC>;
    using DstViewT = GlobalTensor<T, DynShape, DynStrideT, GlobalDstData::layout>;

    TileData &lastTile = state.usePing ? pongTile : pingTile;
    event_t lastEvent = state.usePing ? EVENT_ID1 : EVENT_ID0;

    wait_flag(PIPE_MTE2, PIPE_MTE3, lastEvent);
    DynShape lastShape(1, 1, 1, state.pendingRows, state.pendingCols);
    DstViewT dstView(parallelGroup[state.pendingRank].data() + state.pendingDstOffset, lastShape, dstChunkStride);
    TSTORE(dstView, lastTile);
    set_flag(PIPE_MTE3, PIPE_MTE2, lastEvent);
    wait_flag(PIPE_MTE3, PIPE_MTE2, lastEvent);
}

// 2D sliding chunked scatter with ping-pong double buffering
template <typename ParallelGroupType, typename GlobalSrcData, typename TileData>
PTO_INTERNAL void TscatterChunkedPingPong(ParallelGroupType &parallelGroup, GlobalSrcData &srcGlobalData,
                                          TileData &pingTile, TileData &pongTile, int gShape0, int gShape1, int gShape2,
                                          int gShape3, int gShape4, int tileValidRow, int tileValidCol, int nranks,
                                          int perRankRows)
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
    TscatterPingPongState state;

    for (int r = 0; r < nranks; ++r) {
        int64_t rankSrcBase = static_cast<int64_t>(r) * perRankRows * srcStride3;
        for (int i0 = 0; i0 < gShape0; ++i0) {
            for (int i1 = 0; i1 < gShape1; ++i1) {
                for (int i2 = 0; i2 < gShape2; ++i2) {
                    int64_t srcBase = rankSrcBase + static_cast<int64_t>(i0) * srcStride0 +
                                      static_cast<int64_t>(i1) * srcStride1 + static_cast<int64_t>(i2) * srcStride2;
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
                            TscatterPingPongProcessChunk<ParallelGroupType, GlobalSrcData, TileData>(
                                parallelGroup, srcGlobalData, pingTile, pongTile, srcOff, dstOff, curRows, curCols,
                                srcChunkStride, dstChunkStride, r, state);
                        }
                    }
                }
            }
        }
    }

    TscatterPingPongEpilogue<ParallelGroupType, TileData>(parallelGroup, pingTile, pongTile, state, dstChunkStride);
}

// ============================================================================
// TSCATTER_IMPL (ping-pong): Scatter with double buffering
//
// Uses two staging tiles (pingTile, pongTile) to overlap TLOAD of the next
// chunk (MTE2) with TSTORE of the current chunk (MTE3).
//
// Timeline without ping-pong:
//   [TLOAD chunk0] -> [TSTORE chunk0] -> [TLOAD chunk1] -> [TSTORE chunk1] -> ...
//
// Timeline with ping-pong:
//   [TLOAD chunk0] -> [TSTORE chunk0 | TLOAD chunk1] -> [TSTORE chunk1 | TLOAD chunk2] -> ...
//
// Constraints: same as TSCATTER_IMPL for chunked mode.
// ============================================================================

template <typename ParallelGroupType, typename GlobalSrcData, typename TileData>
PTO_INTERNAL void TSCATTER_IMPL(ParallelGroupType &parallelGroup, GlobalSrcData &srcGlobalData, TileData &pingTile,
                                TileData &pongTile)
{
    using GlobalDstData = typename ParallelGroupTraits<ParallelGroupType>::GlobalDataType;
    using T = typename GlobalSrcData::RawDType;

    static_assert(std::is_same_v<T, typename GlobalDstData::RawDType>, "TSCATTER: GlobalData type mismatch!");
    static_assert(std::is_same_v<T, typename TileData::DType>,
                  "TSCATTER: TileData element type must match GlobalData element type");
    static_assert(GlobalSrcData::layout == GlobalDstData::layout, "TSCATTER: src/dst layout mismatch");

    const int nranks = parallelGroup.GetSize();
    const int rootIdx = parallelGroup.GetRootIdx();

    PTO_ASSERT(nranks > 0, "ParallelGroup size must be greater than 0!");
    PTO_ASSERT(rootIdx >= 0 && rootIdx < nranks, "rootIdx must be in range [0, nranks)!");

    const int gShape0 = parallelGroup[0].GetShape(GlobalTensorDim::DIM_0);
    const int gShape1 = parallelGroup[0].GetShape(GlobalTensorDim::DIM_1);
    const int gShape2 = parallelGroup[0].GetShape(GlobalTensorDim::DIM_2);
    const int gShape3 = parallelGroup[0].GetShape(GlobalTensorDim::DIM_3);
    const int gShape4 = parallelGroup[0].GetShape(GlobalTensorDim::DIM_4);

    const int perRankRows = gShape3;
    const int64_t totalRows = static_cast<int64_t>(gShape0) * gShape1 * gShape2 * gShape3;
    const int tileValidRow = pingTile.GetValidRow();
    const int tileValidCol = pingTile.GetValidCol();

    PTO_ASSERT(tileValidRow > 0, "TSCATTER: tileValidRow must be greater than 0");
    PTO_ASSERT(tileValidCol > 0, "TSCATTER: tileValidCol must be greater than 0");

    if (totalRows == 0 || gShape4 == 0) {
        return;
    }

    // Simple path: per-rank data fits in UB tile, no ping-pong benefit
    if (totalRows <= tileValidRow && gShape4 <= tileValidCol) {
        TscatterSimple<ParallelGroupType, GlobalSrcData, TileData>(
            parallelGroup, srcGlobalData, pingTile, gShape0, gShape1, gShape2, gShape3, gShape4, nranks, perRankRows);
        return;
    }

    // 2D sliding chunked path with ping-pong double buffering
    constexpr bool isDynamicRow = (TileData::ValidRow == DYNAMIC);
    constexpr bool isDynamicCol = (TileData::ValidCol == DYNAMIC);

    if constexpr (!isDynamicRow) {
        PTO_ASSERT(gShape3 % tileValidRow == 0,
                   "TSCATTER chunked: per-rank DIM_3 must be divisible by tile ValidRow when static.");
    }
    if constexpr (!isDynamicCol) {
        PTO_ASSERT(gShape4 % tileValidCol == 0,
                   "TSCATTER chunked: DIM_4 must be divisible by tile ValidCol when static.");
    }

    TscatterChunkedPingPong<ParallelGroupType, GlobalSrcData, TileData>(
        parallelGroup, srcGlobalData, pingTile, pongTile, gShape0, gShape1, gShape2, gShape3, gShape4, tileValidRow,
        tileValidCol, nranks, perRankRows);
}

} // namespace comm
} // namespace pto

#endif // PTO_COMM_TSCATTER_HPP
