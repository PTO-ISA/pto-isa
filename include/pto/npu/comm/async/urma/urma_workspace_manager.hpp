/**
Copyright (c) 2025 Huawei Technologies Co., Ltd.
This program is free software, you can redistribute it and/or modify it under the terms and conditions of
CANN Open Software License Agreement Version 2.0 (the "License").
Please refer to the License for details. You may not use this file except in compliance with the License.
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
See LICENSE in the root of the software repository for the full text of the License.
*/

#ifndef PTO_NPU_COMM_ASYNC_URMA_WORKSPACE_MANAGER_HPP
#define PTO_NPU_COMM_ASYNC_URMA_WORKSPACE_MANAGER_HPP

#if defined(__CCE_KT_TEST__)
#error "urma_workspace_manager.hpp is a host-only header and cannot be included in device code."
#endif

#include <cstdint>
#include <iostream>
#include <vector>

#include "pto/npu/comm/async/urma/urma_workspace_helpers.hpp"

namespace pto {
namespace comm {
namespace urma {

// ============================================================================
// UrmaWorkspaceManager: Host-side URMA workspace initialization.
//
// 10-step initialization:
//   1. TsdProcessOpen
//   2. RaInit
//   3. RaCtxInit (+ RaGetDevEidInfo + RaCtxTokenIdAlloc)
//   4. RaCtxLmemRegister (register symmetric memory as MR)
//   5. JFCCreate (Channel + CQ with device-side polling)
//   6. JettyCreate (QP with SQ/RQ)
//   7. JettyImport (allgather QpKey, RaCtxQpImport per remote rank)
//   8. JettyBind (skip in RM mode)
//   9. FillUrmaInfo (construct device layout + aclrtMemcpy)
//  10. RmemImport (allgather MR, RaCtxRmemImport per remote rank)
// ============================================================================
class UrmaWorkspaceManager {
public:
    UrmaWorkspaceManager() = default;
    ~UrmaWorkspaceManager()
    {
        Finalize();
    }

    UrmaWorkspaceManager(const UrmaWorkspaceManager &) = delete;
    UrmaWorkspaceManager &operator=(const UrmaWorkspaceManager &) = delete;

    bool Init(uint32_t deviceId, uint32_t rankId, uint32_t rankCount, void *symmetricAddr, uint64_t symmetricSize,
              const UrmaBootstrapHandle &bootstrap)
    {
        deviceId_ = deviceId;
        rankId_ = rankId;
        rankCount_ = rankCount;
        symmetricAddr_ = symmetricAddr;
        symmetricSize_ = symmetricSize;
        bootstrap_ = bootstrap;

        wqInfoList_.resize(rankCount_);
        cqInfoList_.resize(rankCount_);
        ubMemInfoList_.resize(rankCount_);
        hccpEidList_.resize(rankCount_);
        tpnList_.resize(rankCount_, 0);
        qpKeyList_.resize(rankCount_);
        allQpImportInfoT_.resize(rankCount_);
        remoteQpHandles_.resize(rankCount_, nullptr);
        rmemHandles_.resize(rankCount_, nullptr);

        auto &loader = HccpV2Loader::Instance();
        if (!loader.Load()) {
            std::cerr << "[URMA] Failed to load HCCP V2 libraries" << std::endl;
            return false;
        }

        if (!OpenTsd())
            return false;
        if (!RaInit())
            return false;
        if (!RaCtxInit())
            return false;
        if (!RegisterMR())
            return false;
        if (!JFCCreate())
            return false;
        if (!JettyCreate())
            return false;
        if (!JettyImport())
            return false;
        if (!JettyBind())
            return false;
        if (!FillUrmaInfo())
            return false;
        if (!RmemImport())
            return false;

        initialized_ = true;
        return true;
    }

    void Finalize()
    {
        FreeDeviceAddrs();
        DestroyHccpHandles();
        initialized_ = false;
    }

    void *GetWorkspaceAddr() const
    {
        return urmaInfoDevice_;
    }

private:
    void FreeDeviceAddrs()
    {
        FreeDeviceAddr(urmaInfoDevice_);
        FreeDeviceAddr(hccpEidDevice_);
        FreeDeviceAddr(cqPiAddr_);
        FreeDeviceAddr(cqCiAddr_);
        FreeDeviceAddr(sqPiAddr_);
        FreeDeviceAddr(sqCiAddr_);
    }

    void DestroyHccpHandles()
    {
        UnbindQp();
        UnimportRemoteResources();
        DestroyLocalHandles();
    }

    void UnbindQp()
    {
        auto &api = HccpV2Loader::Instance();
        if (transportMode_ != hccp::CONN_RM && qpHandle_ && api.raCtxQpUnbind) {
            api.raCtxQpUnbind(qpHandle_);
        }
    }

    void UnimportRemoteResources()
    {
        auto &api = HccpV2Loader::Instance();
        for (uint32_t i = 0; i < rankCount_; ++i) {
            if (i == rankId_)
                continue;
            if (remoteQpHandles_[i] && api.raCtxQpUnimport) {
                api.raCtxQpUnimport(ctxHandle_, remoteQpHandles_[i]);
                remoteQpHandles_[i] = nullptr;
            }
            if (rmemHandles_[i] && api.raCtxRmemUnimport) {
                api.raCtxRmemUnimport(ctxHandle_, rmemHandles_[i]);
                rmemHandles_[i] = nullptr;
            }
        }
    }

    void DestroyLocalHandles()
    {
        auto &api = HccpV2Loader::Instance();
        if (api.raCtxQpDestroy && qpHandle_) {
            api.raCtxQpDestroy(qpHandle_);
            qpHandle_ = nullptr;
        }
        if (api.raCtxCqDestroy && cqHandle_ && ctxHandle_) {
            api.raCtxCqDestroy(ctxHandle_, cqHandle_);
            cqHandle_ = nullptr;
        }
        if (api.raCtxChanDestroy && chanHandle_ && ctxHandle_) {
            api.raCtxChanDestroy(ctxHandle_, chanHandle_);
            chanHandle_ = nullptr;
        }
        if (api.raCtxLmemUnregister && lmemHandle_ && ctxHandle_) {
            api.raCtxLmemUnregister(ctxHandle_, lmemHandle_);
            lmemHandle_ = nullptr;
        }
        if (api.raCtxTokenIdFree && tokenIdHandle_ && ctxHandle_) {
            api.raCtxTokenIdFree(ctxHandle_, tokenIdHandle_);
            tokenIdHandle_ = nullptr;
        }
        if (api.raCtxDeinit && ctxHandle_) {
            api.raCtxDeinit(ctxHandle_);
            ctxHandle_ = nullptr;
        }
    }

    bool OpenTsd()
    {
        return OpenTsdIfNeeded(deviceId_, tsdOpened_);
    }

    bool RaInit()
    {
        return RaInitIfNeeded(deviceId_, raInitialized_);
    }

    // Step 3: Create RA context (+ get EID + allocate token)
    bool RaCtxInit()
    {
        auto &api = HccpV2Loader::Instance();

        hccp::CtxInitAttr attr{};
        attr.phyId = deviceId_;
        if (!QueryFirstEid(deviceId_, attr.ub.eid, attr.ub.eidIndex))
            return false;

        hccp::CtxInitCfg cfg{};
        cfg.mode = hccp::NETWORK_OFFLINE;
        int ret = api.raCtxInit(&cfg, &attr, &ctxHandle_);
        if (ret != 0) {
            std::cerr << "[URMA] RaCtxInit failed: " << ret << std::endl;
            return false;
        }

        localHccpEid_ = attr.ub.eid;
        SwapEidByteOrder(localHccpEid_);

        hccp::HccpTokenId tokenId{0};
        ret = api.raCtxTokenIdAlloc(ctxHandle_, &tokenId, &tokenIdHandle_);
        if (ret != 0) {
            std::cerr << "[URMA] RaCtxTokenIdAlloc failed: " << ret << std::endl;
            return false;
        }
        return true;
    }

    // Step 4: Register symmetric memory as MR
    bool RegisterMR()
    {
        auto &api = HccpV2Loader::Instance();
        hccp::MrRegInfoT mrInfo{};
        mrInfo.in.mem.addr = reinterpret_cast<uint64_t>(symmetricAddr_);
        mrInfo.in.mem.size = symmetricSize_;
        mrInfo.in.ub.tokenValue = hccp::kTokenValue;
        mrInfo.in.ub.tokenIdHandle = tokenIdHandle_;
        mrInfo.in.ub.flags.bs.access = hccp::MEM_SEG_ACCESS_DEFAULT;
        mrInfo.in.ub.flags.bs.cacheable = 0;
        mrInfo.in.ub.flags.bs.tokenIdValid = 1;
        mrInfo.in.ub.flags.bs.nonPin = 0;
        mrInfo.in.ub.flags.bs.userIova = 0;
        mrInfo.in.ub.flags.bs.tokenPolicy = hccp::TOKEN_POLICY_PLAIN_TEXT;

        int ret = api.raCtxLmemRegister(ctxHandle_, &mrInfo, &lmemHandle_);
        if (ret != 0) {
            std::cerr << "[URMA] RaCtxLmemRegister failed: " << ret << std::endl;
            return false;
        }

        PopulateMrResult(localMR_, mrInfo, lmemHandle_, tokenIdHandle_);
        PopulateLocalMemInfo(localMemInfo_, mrInfo.out.ub.tokenId, symmetricSize_, symmetricAddr_);
        return true;
    }

    // Step 5: Create JFC (Channel + CQ)
    bool JFCCreate()
    {
        auto &api = HccpV2Loader::Instance();

        hccp::ChanInfoT chanInfo{};
        chanInfo.in.dataPlaneFlag.bs.poolCqCstm = 1;
        int ret = api.raCtxChanCreate(ctxHandle_, &chanInfo, &chanHandle_);
        if (ret != 0) {
            std::cerr << "[URMA] RaCtxChanCreate failed: " << ret << std::endl;
            return false;
        }

        cqInfo_.in.chanHandle = chanHandle_;
        cqInfo_.in.depth = hccp::kCqDepthDefault;
        cqInfo_.in.ub.userCtx = 0;
        cqInfo_.in.ub.mode = hccp::JFC_MODE_USER_CTL_NORMAL;
        cqInfo_.in.ub.ceqn = 0;
        cqInfo_.in.ub.flag.bs.lockFree = 0;
        cqInfo_.in.ub.flag.bs.jfcInline = 0;

        ret = api.raCtxCqCreate(ctxHandle_, &cqInfo_, &cqHandle_);
        if (ret != 0) {
            std::cerr << "[URMA] RaCtxCqCreate failed: " << ret << std::endl;
            return false;
        }

        UrmaCqCtx localCq{};
        localCq.cqn = 0;
        localCq.bufAddr = cqInfo_.out.bufAddr;
        localCq.cqeShiftSize = Log2U32(cqInfo_.out.cqeSize);
        localCq.depth = cqInfo_.in.depth;

        if (!AllocDeviceCounter(cqPiAddr_, "cqPiAddr") || !AllocDeviceCounter(cqCiAddr_, "cqCiAddr"))
            return false;
        localCq.headAddr = reinterpret_cast<uintptr_t>(cqPiAddr_);
        localCq.tailAddr = reinterpret_cast<uintptr_t>(cqCiAddr_);
        localCq.dbMode = UrmaDbMode::SW_DB;
        localCq.dbAddr = cqInfo_.out.swdbAddr;

        bootstrap_.allgather(&localCq, cqInfoList_.data(), sizeof(UrmaCqCtx), bootstrap_.ctx);
        return true;
    }

    // Step 6: Create Jetty QP
    bool JettyCreate()
    {
        auto &api = HccpV2Loader::Instance();

        hccp::QpCreateAttr qpAttr{};
        InitDefaultQpAttr(qpAttr, cqHandle_, tokenIdHandle_, transportMode_);

        int ret = api.raCtxQpCreate(ctxHandle_, &qpAttr, &qpCreateInfo_, &qpHandle_);
        if (ret != 0) {
            std::cerr << "[URMA] RaCtxQpCreate failed: " << ret << std::endl;
            return false;
        }

        UrmaWQCtx localWq{};
        localWq.wqn = 0;
        localWq.bufAddr = qpCreateInfo_.ub.sqBuffVa;
        localWq.wqeShiftSize = Log2U32(static_cast<uint32_t>(qpCreateInfo_.ub.wqebbSize));
        localWq.depth = qpAttr.sqDepth;

        if (!AllocDeviceCounter(sqPiAddr_, "sqPiAddr") || !AllocDeviceCounter(sqCiAddr_, "sqCiAddr"))
            return false;
        localWq.headAddr = reinterpret_cast<uintptr_t>(sqPiAddr_);
        localWq.tailAddr = reinterpret_cast<uintptr_t>(sqCiAddr_);
        localWq.dbMode = UrmaDbMode::SW_DB;
        localWq.dbAddr = qpCreateInfo_.ub.dbAddr;
        localWq.sl = 0;

        bootstrap_.allgather(&localWq, wqInfoList_.data(), sizeof(UrmaWQCtx), bootstrap_.ctx);
        return true;
    }

    // Step 7: Exchange QP info and import remote Jetty QPs
    bool JettyImport()
    {
        auto &api = HccpV2Loader::Instance();

        hccp::QpImportInfoT localQpImport{};
        localQpImport.in.ub.mode = hccp::JETTY_IMPORT_MODE_NORMAL;
        localQpImport.in.ub.tokenValue = hccp::kTokenValue;
        localQpImport.in.ub.policy = hccp::JETTY_GRP_POLICY_RR;
        localQpImport.in.ub.type = hccp::TARGET_TYPE_JETTY;
        localQpImport.in.ub.flag.bs.tokenPolicy = hccp::TOKEN_POLICY_PLAIN_TEXT;
        localQpImport.in.ub.tpType = 1;

        bootstrap_.allgather(&localQpImport, allQpImportInfoT_.data(), sizeof(hccp::QpImportInfoT), bootstrap_.ctx);
        bootstrap_.allgather(&qpCreateInfo_.key, qpKeyList_.data(), sizeof(hccp::QpKeyT), bootstrap_.ctx);

        for (uint32_t i = 0; i < rankCount_; ++i) {
            if (i == rankId_)
                continue;
            allQpImportInfoT_[i].in.key = qpKeyList_[i];
            int ret = api.raCtxQpImport(ctxHandle_, &allQpImportInfoT_[i], &remoteQpHandles_[i]);
            if (ret != 0) {
                std::cerr << "[URMA] RaCtxQpImport for rank " << i << " failed: " << ret << std::endl;
                return false;
            }
            tpnList_[i] = allQpImportInfoT_[i].out.ub.tpn;
        }
        return true;
    }

    // Step 8: Bind (skip in RM mode)
    bool JettyBind()
    {
        if (transportMode_ == hccp::CONN_RM)
            return true;
        auto &api = HccpV2Loader::Instance();
        for (uint32_t i = 0; i < rankCount_; ++i) {
            if (i == rankId_)
                continue;
            int ret = api.raCtxQpBind(qpHandle_, remoteQpHandles_[i]);
            if (ret != 0) {
                std::cerr << "[URMA] RaCtxQpBind for rank " << i << " failed: " << ret << std::endl;
                return false;
            }
        }
        return true;
    }

    // Step 10: Exchange MR info and import remote memory
    bool RmemImport()
    {
        auto &api = HccpV2Loader::Instance();
        std::vector<hccp::RegMemResultInfo> mrList(rankCount_);
        bootstrap_.allgather(&localMR_, mrList.data(), sizeof(hccp::RegMemResultInfo), bootstrap_.ctx);

        for (uint32_t i = 0; i < rankCount_; ++i) {
            if (i == rankId_) {
                rmemHandles_[i] = lmemHandle_;
                continue;
            }
            hccp::MrImportInfoT mrImport{};
            mrImport.in.key = mrList[i].key;
            mrImport.in.ub.tokenValue = mrList[i].tokenValue;
            mrImport.in.ub.flags.bs.cacheable = mrList[i].cacheable;
            mrImport.in.ub.flags.bs.access = mrList[i].access;

            int ret = api.raCtxRmemImport(ctxHandle_, &mrImport, &rmemHandles_[i]);
            if (ret != 0) {
                std::cerr << "[URMA] RaCtxRmemImport for rank " << i << " failed: " << ret << std::endl;
                return false;
            }
        }
        return true;
    }

    // Step 9: Construct UrmaInfo on host, copy to device
    bool FillUrmaInfo()
    {
        bootstrap_.allgather(&localMemInfo_, ubMemInfoList_.data(), sizeof(UrmaMemInfo), bootstrap_.ctx);
        bootstrap_.allgather(&localHccpEid_, hccpEidList_.data(), sizeof(hccp::HccpEid), bootstrap_.ctx);
        bootstrap_.barrier(bootstrap_.ctx);

        if (!AllocAndCopyEidTable(hccpEidDevice_, hccpEidList_, rankCount_))
            return false;

        constexpr uint32_t qpNum = 1;
        size_t totalSize =
            sizeof(UrmaInfo) + rankCount_ * (2U * sizeof(UrmaWQCtx) * qpNum + 2U * sizeof(UrmaCqCtx) * qpNum +
                                             sizeof(UrmaMemInfo) * qpNum);

        aclError err = aclrtMalloc(&urmaInfoDevice_, totalSize, ACL_MEM_MALLOC_HUGE_FIRST);
        if (err != ACL_SUCCESS) {
            std::cerr << "[URMA] aclrtMalloc for urmaInfo failed: " << err << std::endl;
            return false;
        }

        std::vector<uint8_t> hostBuf(totalSize, 0);
        auto *copyInfo = reinterpret_cast<UrmaInfo *>(hostBuf.data());
        copyInfo->qpNum = qpNum;
        copyInfo->localTokenId = localMR_.tokenId;
        copyInfo->rankCount = rankCount_;

        ComputeInfoLayout(copyInfo, hostBuf.data(), rankCount_);
        FillPerRankData(copyInfo, wqInfoList_, cqInfoList_, ubMemInfoList_, tpnList_, hccpEidDevice_, rankId_,
                        rankCount_);

        ComputeInfoLayout(copyInfo, reinterpret_cast<uint8_t *>(urmaInfoDevice_), rankCount_);

        err = aclrtMemcpy(urmaInfoDevice_, totalSize, hostBuf.data(), totalSize, ACL_MEMCPY_HOST_TO_DEVICE);
        if (err != ACL_SUCCESS) {
            std::cerr << "[URMA] aclrtMemcpy for urmaInfo failed: " << err << std::endl;
            aclrtFree(urmaInfoDevice_);
            urmaInfoDevice_ = nullptr;
            return false;
        }
        return true;
    }

    uint32_t deviceId_{0};
    uint32_t rankId_{0};
    uint32_t rankCount_{0};
    void *symmetricAddr_{nullptr};
    uint64_t symmetricSize_{0};
    UrmaBootstrapHandle bootstrap_{};
    hccp::TransportModeT transportMode_{hccp::CONN_RM};

    void *ctxHandle_{nullptr};
    void *chanHandle_{nullptr};
    void *tokenIdHandle_{nullptr};
    void *lmemHandle_{nullptr};
    void *cqHandle_{nullptr};
    void *qpHandle_{nullptr};
    std::vector<void *> remoteQpHandles_;
    std::vector<void *> rmemHandles_;

    hccp::HccpEid localHccpEid_{};
    hccp::RegMemResultInfo localMR_{};
    hccp::CqInfoT cqInfo_{};
    hccp::QpCreateInfo qpCreateInfo_{};
    UrmaMemInfo localMemInfo_{};

    void *urmaInfoDevice_{nullptr};
    void *hccpEidDevice_{nullptr};
    void *cqPiAddr_{nullptr};
    void *cqCiAddr_{nullptr};
    void *sqPiAddr_{nullptr};
    void *sqCiAddr_{nullptr};

    std::vector<UrmaWQCtx> wqInfoList_;
    std::vector<UrmaCqCtx> cqInfoList_;
    std::vector<UrmaMemInfo> ubMemInfoList_;
    std::vector<hccp::HccpEid> hccpEidList_;
    std::vector<uint32_t> tpnList_;
    std::vector<hccp::QpKeyT> qpKeyList_;
    std::vector<hccp::QpImportInfoT> allQpImportInfoT_;

    bool initialized_{false};
    static inline bool tsdOpened_{false};
    static inline bool raInitialized_{false};
};

} // namespace urma
} // namespace comm
} // namespace pto

#endif // PTO_NPU_COMM_ASYNC_URMA_WORKSPACE_MANAGER_HPP
