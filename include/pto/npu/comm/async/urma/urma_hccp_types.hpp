/**
Copyright (c) 2025 Huawei Technologies Co., Ltd.
This program is free software, you can redistribute it and/or modify it under the terms and conditions of
CANN Open Software License Agreement Version 2.0 (the "License").
Please refer to the License for details. You may not use this file except in compliance with the License.
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
See LICENSE in the root of the software repository for the full text of the License.
*/

#ifndef PTO_NPU_COMM_ASYNC_URMA_HCCP_TYPES_HPP
#define PTO_NPU_COMM_ASYNC_URMA_HCCP_TYPES_HPP

#include <cstdint>

namespace pto {
namespace comm {
namespace urma {
namespace hccp {

// ============================================================================
// Constants
// ============================================================================
constexpr int32_t kDevEidInfoMaxName = 64;
constexpr int32_t kDevQpKeySize = 64;
constexpr int32_t kMemKeySize = 128;
constexpr uint32_t kTokenValue = 0;
constexpr uint32_t kCqDepthDefault = 16384;
constexpr uint32_t kSqDepthDefault = 4096;
constexpr uint32_t kRqDepthDefault = 256;
constexpr uint8_t kRnrRetryCountDefault = 7;

// ============================================================================
// Enums (binary-compatible with HCCP V2 library ABI)
// ============================================================================
enum HccpNetworkMode
{
    NETWORK_PEER_ONLINE = 0,
    NETWORK_OFFLINE,
    NETWORK_ONLINE
};

enum DrvHdcServiceType : int
{
    HDC_SERVICE_TYPE_RDMA = 6,
    HDC_SERVICE_TYPE_RDMA_V2 = 18
};

enum SubProcType
{
    TSD_SUB_PROC_HCCP = 0,
    TSD_SUB_PROC_COMPUTE = 1,
    TSD_SUB_PROC_MAX = 0xFF
};

enum JfcMode
{
    JFC_MODE_NORMAL = 0,
    JFC_MODE_STARS_POLL = 1,
    JFC_MODE_CCU_POLL = 2,
    JFC_MODE_USER_CTL_NORMAL = 3,
    JFC_MODE_MAX,
};

enum JettyMode
{
    JETTY_MODE_URMA_NORMAL = 0,
    JETTY_MODE_CACHE_LOCK_DWQE = 1,
    JETTY_MODE_CCU = 2,
    JETTY_MODE_USER_CTL_NORMAL = 3,
    JETTY_MODE_CCU_TA_CACHE = 4,
    JETTY_MODE_MAX,
};

enum TransportModeT
{
    CONN_RM = 1,
    CONN_RC = 2
};

enum TokenPolicy : uint32_t
{
    TOKEN_POLICY_NONE = 0,
    TOKEN_POLICY_PLAIN_TEXT = 1,
    TOKEN_POLICY_SIGNED = 2,
    TOKEN_POLICY_ALL_ENCRYPTED = 3,
};

enum JettyImportMode
{
    JETTY_IMPORT_MODE_NORMAL = 0,
    JETTY_IMPORT_MODE_EXP = 1
};

enum JettyGrpPolicy : uint32_t
{
    JETTY_GRP_POLICY_RR = 0,
    JETTY_GRP_POLICY_HASH_HINT = 1
};

enum TargetType
{
    TARGET_TYPE_JFR = 0,
    TARGET_TYPE_JETTY = 1,
    TARGET_TYPE_JETTY_GROUP = 2
};

enum MemSegAccessFlags
{
    MEM_SEG_ACCESS_LOCAL_ONLY = 1,
    MEM_SEG_ACCESS_READ = (1 << 1),
    MEM_SEG_ACCESS_WRITE = (1 << 2),
    MEM_SEG_ACCESS_ATOMIC = (1 << 3),
    MEM_SEG_ACCESS_DEFAULT = MEM_SEG_ACCESS_READ | MEM_SEG_ACCESS_WRITE | MEM_SEG_ACCESS_ATOMIC,
};

// ============================================================================
// Process / RA initialization structs
// ============================================================================
struct ProcExtParam {
    const char *paramInfo;
    uint64_t paramLen;
};

struct ProcEnvParam {
    const char *envName;
    uint64_t nameLen;
    const char *envValue;
    uint64_t valueLen;
};

struct ProcOpenArgs {
    SubProcType procType;
    ProcEnvParam *envParaList;
    uint64_t envCnt;
    const char *filePath;
    uint64_t pathLen;
    ProcExtParam *extParamList;
    uint64_t extParamCnt;
    int *subPid;
};

struct RaInitConfig {
    unsigned int phyId;
    HccpNetworkMode nicPosition;
    DrvHdcServiceType hdcType;
    bool enableHdcAsync;
};

struct RaInfo {
    HccpNetworkMode mode;
    unsigned int phyId;
};

// ============================================================================
// EID / Device info
// ============================================================================
union HccpEid {
    uint8_t raw[16];
    struct {
        uint64_t reserved;
        uint32_t prefix;
        uint32_t addr;
    } in4;
    struct {
        uint64_t subnetPrefix;
        uint64_t interfaceId;
    } in6;
};

struct DevEidInfo {
    char name[kDevEidInfoMaxName];
    uint32_t type;
    uint32_t eidIndex;
    HccpEid eid;
    uint32_t dieId;
    uint32_t chipId;
    uint32_t funcId;
    uint32_t resv;
};

// ============================================================================
// Context initialization
// ============================================================================
struct CtxInitCfg {
    HccpNetworkMode mode;
    union {
        struct {
            bool disabledLiteThread;
        } rdma;
    };
};

struct CtxInitAttr {
    unsigned int phyId;
    union {
        uint8_t _rdmaPad[24];
        struct {
            uint32_t eidIndex;
            HccpEid eid;
        } ub;
    };
    uint32_t resv[16];
};

struct HccpTokenId {
    uint32_t tokenId;
};

// ============================================================================
// Channel / CQ
// ============================================================================
union DataPlaneCstmFlag {
    struct {
        uint32_t poolCqCstm : 1;
        uint32_t reserved : 31;
    } bs;
    uint32_t value;
};

struct ChanInfoT {
    struct {
        DataPlaneCstmFlag dataPlaneFlag;
    } in;
    struct {
        int fd;
    } out;
};

union JfcFlag {
    struct {
        uint32_t lockFree : 1;
        uint32_t jfcInline : 1;
        uint32_t reserved : 30;
    } bs;
    uint32_t value;
};

struct CqCreateAttr {
    void *chanHandle;
    uint32_t depth;
    union {
        struct {
            uint64_t cqContext;
            uint32_t mode;
            uint32_t compVector;
        } rdma;
        struct {
            uint64_t userCtx;
            JfcMode mode;
            uint32_t ceqn;
            JfcFlag flag;
            struct {
                bool valid;
                uint32_t cqeFlag;
            } ccuExCfg;
        } ub;
    };
};

struct CqCreateInfo {
    uint64_t va;
    uint32_t id;
    uint32_t cqeSize;
    uint64_t bufAddr;
    uint64_t swdbAddr;
};

struct CqInfoT {
    CqCreateAttr in;
    CqCreateInfo out;
};

// ============================================================================
// QP / Jetty
// ============================================================================
union JettyFlag {
    struct {
        uint32_t shareJfr : 1;
        uint32_t reserved : 31;
    } bs;
    uint32_t value;
};

union JfsFlag {
    struct {
        uint32_t lockFree : 1;
        uint32_t errorSuspend : 1;
        uint32_t outorderComp : 1;
        uint32_t orderType : 8;
        uint32_t multiPath : 1;
        uint32_t reserved : 20;
    } bs;
    uint32_t value;
};

union CstmJfsFlag {
    struct {
        uint32_t sqCstm : 1;
        uint32_t dbCstm : 1;
        uint32_t dbCtlCstm : 1;
        uint32_t reserved : 29;
    } bs;
    uint32_t value;
};

struct JettyQueCfgEx {
    uint32_t buffSize;
    uint64_t buffVa;
};

struct QpCreateAttr {
    void *scqHandle;
    void *rcqHandle;
    void *srqHandle;
    uint32_t sqDepth;
    uint32_t rqDepth;
    TransportModeT transportMode;
    union {
        struct {
            uint32_t mode;
            uint32_t udpSport;
            uint8_t trafficClass;
            uint8_t sl;
            uint8_t timeout;
            uint8_t rnrRetry;
            uint8_t retryCnt;
        } rdm;
        struct {
            JettyMode mode;
            uint32_t jettyId;
            JettyFlag flag;
            JfsFlag jfsFlag;
            void *tokenIdHandle;
            uint32_t tokenValue;
            uint8_t priority;
            uint8_t rnrRetry;
            uint8_t errTimeout;
            union {
                struct {
                    JettyQueCfgEx sq;
                    bool piType;
                    CstmJfsFlag cstmFlag;
                    uint32_t sqebbNum;
                } extMode;
                struct {
                    bool lockFlag;
                    uint32_t sqeBufIdx;
                } taCacheMode;
            };
        } ub;
    };
    uint32_t resv[16];
};

struct QpKeyT {
    uint8_t value[kDevQpKeySize];
    uint8_t size;
};

struct QpCreateInfo {
    QpKeyT key;
    union {
        struct {
            uint32_t qpn;
        } rdma;
        struct {
            uint32_t uasid;
            uint32_t id;
            uint64_t sqBuffVa;
            uint64_t wqebbSize;
            uint64_t dbAddr;
            uint32_t dbTokenId;
            uint64_t ciAddr;
        } ub;
    };
    uint64_t va;
    uint32_t resv[16U];
};

// ============================================================================
// Jetty import
// ============================================================================
union ImportJettyFlag {
    struct {
        uint32_t tokenPolicy : 3;
        uint32_t orderType : 8;
        uint32_t shareTp : 1;
        uint32_t reserved : 20;
    } bs;
    uint32_t value;
};

struct JettyImportExpCfg {
    uint64_t tpHandle;
    uint64_t peerTpHandle;
    uint64_t tag;
    uint32_t txPsn;
    uint32_t rxPsn;
    uint32_t rsv[16];
};

struct QpImportAttr {
    QpKeyT key;
    union {
        struct {
            JettyImportMode mode;
            uint32_t tokenValue;
            JettyGrpPolicy policy;
            TargetType type;
            ImportJettyFlag flag;
            JettyImportExpCfg expImportCfg;
            uint32_t tpType;
        } ub;
    };
    uint32_t resv[7];
};

struct QpImportInfo {
    union {
        struct {
            uint64_t tjettyHandle;
            uint32_t tpn;
        } ub;
    };
    uint32_t resv[8];
};

struct QpImportInfoT {
    QpImportAttr in;
    QpImportInfo out;
};

// ============================================================================
// Memory region
// ============================================================================
struct MemKey {
    uint8_t value[kMemKeySize];
    uint8_t size;
};

struct MemInfo {
    uint64_t addr;
    uint64_t size;
};

union RegSegFlag {
    struct {
        uint32_t tokenPolicy : 3;
        uint32_t cacheable : 1;
        uint32_t dsva : 1;
        uint32_t access : 6;
        uint32_t nonPin : 1;
        uint32_t userIova : 1;
        uint32_t tokenIdValid : 1;
        uint32_t reserved : 18;
    } bs;
    uint32_t value;
};

struct MemRegAttr {
    MemInfo mem;
    union {
        struct {
            int access;
        } rdma;
        struct {
            RegSegFlag flags;
            uint32_t tokenValue;
            void *tokenIdHandle;
        } ub;
    };
    uint32_t resv[8];
};

struct MemRegInfo {
    MemKey key;
    union {
        struct {
            uint32_t lkey;
        } rdma;
        struct {
            uint32_t tokenId;
            uint64_t targetSegHandle;
        } ub;
    };
    uint32_t resv[8U];
};

struct MrRegInfoT {
    MemRegAttr in;
    MemRegInfo out;
};

union ImportSegFlag {
    struct {
        uint32_t cacheable : 1;
        uint32_t access : 6;
        uint32_t mapping : 1;
        uint32_t reserved : 24;
    } bs;
    uint32_t value;
};

struct MemImportAttr {
    MemKey key;
    union {
        struct {
            ImportSegFlag flags;
            uint64_t mappingAddr;
            uint32_t tokenValue;
        } ub;
    };
    uint32_t resv[4];
};

struct MemImportInfo {
    union {
        struct {
            uint32_t key;
        } rdma;
        struct {
            uint64_t targetSegHandle;
        } ub;
    };
    uint32_t resv[4];
};

struct MrImportInfoT {
    MemImportAttr in;
    MemImportInfo out;
};

struct RegMemResultInfo {
    uint32_t reserved;
    uint64_t address;
    uint64_t size;
    void *lmemHandle;
    MemKey key;
    uint32_t tokenId;
    uint32_t tokenValue;
    uint64_t targetSegHandle;
    void *tokenIdHandle;
    uint32_t cacheable;
    int32_t access;
};

// ============================================================================
// Function pointer typedefs (match HCCP V2 library ABI)
// ============================================================================
using RaInitFn = int (*)(RaInitConfig *);
using TsdProcessOpenFn = uint32_t (*)(uint32_t, ProcOpenArgs *);
using TsdProcessCloseFn = uint32_t (*)(uint32_t, int);
using RaGetDevEidInfoNumFn = int (*)(RaInfo, unsigned int *);
using RaGetDevEidInfoListFn = int (*)(RaInfo, DevEidInfo[], unsigned int *);
using RaCtxInitFn = int (*)(CtxInitCfg *, CtxInitAttr *, void **);
using RaCtxDeinitFn = int (*)(void *);
using RaCtxChanCreateFn = int (*)(void *, ChanInfoT *, void **);
using RaCtxChanDestroyFn = int (*)(void *, void *);
using RaCtxCqCreateFn = int (*)(void *, CqInfoT *, void **);
using RaCtxCqDestroyFn = int (*)(void *, void *);
using RaCtxQpCreateFn = int (*)(void *, QpCreateAttr *, QpCreateInfo *, void **);
using RaCtxQpDestroyFn = int (*)(void *);
using RaCtxTokenIdAllocFn = int (*)(void *, HccpTokenId *, void **);
using RaCtxTokenIdFreeFn = int (*)(void *, void *);
using RaCtxQpImportFn = int (*)(void *, QpImportInfoT *, void **);
using RaCtxQpUnimportFn = int (*)(void *, void *);
using RaCtxQpBindFn = int (*)(void *, void *);
using RaCtxQpUnbindFn = int (*)(void *);
using RaCtxLmemRegisterFn = int (*)(void *, MrRegInfoT *, void **);
using RaCtxLmemUnregisterFn = int (*)(void *, void *);
using RaCtxRmemImportFn = int (*)(void *, MrImportInfoT *, void **);
using RaCtxRmemUnimportFn = int (*)(void *, void *);

} // namespace hccp
} // namespace urma
} // namespace comm
} // namespace pto

#endif // PTO_NPU_COMM_ASYNC_URMA_HCCP_TYPES_HPP
