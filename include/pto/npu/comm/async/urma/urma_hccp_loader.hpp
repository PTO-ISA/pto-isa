/**
Copyright (c) 2025 Huawei Technologies Co., Ltd.
This program is free software, you can redistribute it and/or modify it under the terms and conditions of
CANN Open Software License Agreement Version 2.0 (the "License").
Please refer to the License for details. You may not use this file except in compliance with the License.
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
See LICENSE in the root of the software repository for the full text of the License.
*/

#ifndef PTO_NPU_COMM_ASYNC_URMA_HCCP_LOADER_HPP
#define PTO_NPU_COMM_ASYNC_URMA_HCCP_LOADER_HPP

#include <iostream>
#include <mutex>
#include <dlfcn.h>

#include "pto/npu/comm/async/urma/urma_hccp_types.hpp"

namespace pto {
namespace comm {
namespace urma {

// ============================================================================
// HccpV2Loader: dynamic loader for HCCP V2 libraries. Thread-safe singleton.
// ============================================================================
class HccpV2Loader {
public:
    static HccpV2Loader &Instance()
    {
        static HccpV2Loader inst;
        return inst;
    }

    bool Load()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (loaded_) {
            return true;
        }
        if (!OpenLibraries() || !LoadAllSymbols()) {
            CleanupUnlocked();
            return false;
        }
        loaded_ = true;
        return true;
    }

    void Cleanup()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        CleanupUnlocked();
    }

    // Public function pointers — set by LoadAllSymbols()
    hccp::RaInitFn raInit{nullptr};
    hccp::TsdProcessOpenFn tsdProcessOpen{nullptr};
    hccp::TsdProcessCloseFn tsdProcessClose{nullptr};
    hccp::RaGetDevEidInfoNumFn raGetDevEidInfoNum{nullptr};
    hccp::RaGetDevEidInfoListFn raGetDevEidInfoList{nullptr};
    hccp::RaCtxInitFn raCtxInit{nullptr};
    hccp::RaCtxDeinitFn raCtxDeinit{nullptr};
    hccp::RaCtxChanCreateFn raCtxChanCreate{nullptr};
    hccp::RaCtxChanDestroyFn raCtxChanDestroy{nullptr};
    hccp::RaCtxCqCreateFn raCtxCqCreate{nullptr};
    hccp::RaCtxCqDestroyFn raCtxCqDestroy{nullptr};
    hccp::RaCtxQpCreateFn raCtxQpCreate{nullptr};
    hccp::RaCtxQpDestroyFn raCtxQpDestroy{nullptr};
    hccp::RaCtxTokenIdAllocFn raCtxTokenIdAlloc{nullptr};
    hccp::RaCtxTokenIdFreeFn raCtxTokenIdFree{nullptr};
    hccp::RaCtxQpImportFn raCtxQpImport{nullptr};
    hccp::RaCtxQpUnimportFn raCtxQpUnimport{nullptr};
    hccp::RaCtxQpBindFn raCtxQpBind{nullptr};
    hccp::RaCtxQpUnbindFn raCtxQpUnbind{nullptr};
    hccp::RaCtxLmemRegisterFn raCtxLmemRegister{nullptr};
    hccp::RaCtxLmemUnregisterFn raCtxLmemUnregister{nullptr};
    hccp::RaCtxRmemImportFn raCtxRmemImport{nullptr};
    hccp::RaCtxRmemUnimportFn raCtxRmemUnimport{nullptr};

private:
    HccpV2Loader() = default;
    ~HccpV2Loader()
    {
        CleanupUnlocked();
    }
    HccpV2Loader(const HccpV2Loader &) = delete;
    HccpV2Loader &operator=(const HccpV2Loader &) = delete;

    void CleanupUnlocked()
    {
        loaded_ = false;
        auto safeClose = [](void *&h) {
            if (h) {
                dlclose(h);
                h = nullptr;
            }
        };
        safeClose(tsdHandle_);
        safeClose(raHandle_);
        safeClose(hcclV2Handle_);
        safeClose(hcclV1Handle_);
    }

    bool OpenOneLib(void *&handle, const char *name)
    {
        handle = dlopen(name, RTLD_NOW);
        if (!handle) {
            std::cerr << "[URMA] Failed to load " << name << ": " << dlerror() << std::endl;
            return false;
        }
        return true;
    }

    bool OpenLibraries()
    {
        return OpenOneLib(hcclV1Handle_, "libhccl.so") && OpenOneLib(hcclV2Handle_, "libhccl_v2.so") &&
               OpenOneLib(raHandle_, "libra.so") && OpenOneLib(tsdHandle_, "libtsdclient.so");
    }

    template <typename T>
    bool LoadSym(T &fn, void *lib, const char *primary, const char *fallback)
    {
        fn = reinterpret_cast<T>(dlsym(lib, primary));
        if (!fn && fallback) {
            fn = reinterpret_cast<T>(dlsym(lib, fallback));
        }
        if (!fn) {
            std::cerr << "[URMA] Failed to load symbol " << primary << ": " << dlerror() << std::endl;
            return false;
        }
        return true;
    }

    bool LoadHcclV2Symbols()
    {
        return LoadSym(raInit, hcclV2Handle_, "RaInit", "ra_init") &&
               LoadSym(raGetDevEidInfoNum, hcclV2Handle_, "RaGetDevEidInfoNum", "ra_get_dev_eid_info_num") &&
               LoadSym(raGetDevEidInfoList, hcclV2Handle_, "RaGetDevEidInfoList", "ra_get_dev_eid_info_list") &&
               LoadSym(raCtxInit, hcclV2Handle_, "RaCtxInit", "ra_ctx_init") &&
               LoadSym(raCtxDeinit, hcclV2Handle_, "RaCtxDeinit", "ra_ctx_deinit") &&
               LoadSym(raCtxCqCreate, hcclV2Handle_, "RaCtxCqCreate", "ra_ctx_cq_create") &&
               LoadSym(raCtxCqDestroy, hcclV2Handle_, "RaCtxCqDestroy", "ra_ctx_cq_destroy") &&
               LoadSym(raCtxQpCreate, hcclV2Handle_, "RaCtxQpCreate", "ra_ctx_qp_create") &&
               LoadSym(raCtxQpDestroy, hcclV2Handle_, "RaCtxQpDestroy", "ra_ctx_qp_destroy") &&
               LoadSym(raCtxTokenIdAlloc, hcclV2Handle_, "RaCtxTokenIdAlloc", "ra_ctx_token_id_alloc") &&
               LoadSym(raCtxTokenIdFree, hcclV2Handle_, "RaCtxTokenIdFree", "ra_ctx_token_id_free") &&
               LoadSym(raCtxQpImport, hcclV2Handle_, "RaCtxQpImport", "ra_ctx_qp_import") &&
               LoadSym(raCtxQpUnimport, hcclV2Handle_, "RaCtxQpUnimport", "ra_ctx_qp_unimport") &&
               LoadSym(raCtxQpBind, hcclV2Handle_, "RaCtxQpBind", "ra_ctx_qp_bind") &&
               LoadSym(raCtxQpUnbind, hcclV2Handle_, "RaCtxQpUnbind", "ra_ctx_qp_unbind") &&
               LoadSym(raCtxLmemRegister, hcclV2Handle_, "RaCtxLmemRegister", "ra_ctx_lmem_register") &&
               LoadSym(raCtxLmemUnregister, hcclV2Handle_, "RaCtxLmemUnregister", "ra_ctx_lmem_unregister") &&
               LoadSym(raCtxRmemImport, hcclV2Handle_, "RaCtxRmemImport", "ra_ctx_rmem_import") &&
               LoadSym(raCtxRmemUnimport, hcclV2Handle_, "RaCtxRmemUnimport", "ra_ctx_rmem_unimport");
    }

    bool LoadRaSymbols()
    {
        return LoadSym(raCtxChanCreate, raHandle_, "RaCtxChanCreate", "ra_ctx_chan_create") &&
               LoadSym(raCtxChanDestroy, raHandle_, "RaCtxChanDestroy", "ra_ctx_chan_destroy");
    }

    bool LoadTsdSymbols()
    {
        return LoadSym(tsdProcessOpen, tsdHandle_, "TsdProcessOpen", "tsd_process_open") &&
               LoadSym(tsdProcessClose, tsdHandle_, "TsdProcessClose", "tsd_process_close");
    }

    bool LoadAllSymbols()
    {
        return LoadHcclV2Symbols() && LoadRaSymbols() && LoadTsdSymbols();
    }

    std::mutex mutex_;
    bool loaded_{false};
    void *hcclV1Handle_{nullptr};
    void *hcclV2Handle_{nullptr};
    void *raHandle_{nullptr};
    void *tsdHandle_{nullptr};
};

} // namespace urma
} // namespace comm
} // namespace pto

#endif // PTO_NPU_COMM_ASYNC_URMA_HCCP_LOADER_HPP
