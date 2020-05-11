/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 *    @file
 *      This file defines data types and objects for managing the
 *      active node state necessary to participate in a CHIP fabric.
 *
 */

// Include ChipCore.h OUTSIDE of the include guard for ChipFabricState.h.
// This allows ChipCore.h to enforce a canonical include order for core
// header files, making it easier to manage dependencies between these files.
#include <core/CHIPCore.h>

#ifndef FABRICSTATE_H
#define FABRICSTATE_H

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <inttypes.h>

#include <support/DLLUtil.h>
#include <support/CHIPCounter.h>
#include <support/PersistedCounter.h>
#include <support/FlagUtils.hpp>
#include <core/CHIPKeyIds.h>
#include <core/CHIPConnection.h>
#include <core/IdConstants.h>

namespace chip {

class DLL_EXPORT ChipMessageLayer;
class DLL_EXPORT ChipExchangeManager;
struct ChipMessageInfo;

/**
 * @class ChipSessionState
 *
 * @brief
 *   Conveys the communication state needed to send/receive messages with another node.
 */
class ChipSessionState
{
public:
    typedef uint16_t ReceiveFlagsType;

    enum
    {
        kReceiveFlags_NumMessageIdFlags     = (sizeof(ReceiveFlagsType) * 8) - 1,
        kReceiveFlags_MessageIdSynchronized = (1 << kReceiveFlags_NumMessageIdFlags),
        kReceiveFlags_MessageIdFlagsMask    = ~kReceiveFlags_MessageIdSynchronized
    };

    ChipSessionState(void);
    ChipSessionState(MonotonicallyIncreasingCounter * nextMsgId, uint32_t * maxRcvdMsgId, ReceiveFlagsType * rcvFlags);

    uint32_t NewMessageId(void);
    bool MessageIdNotSynchronized(void);
    bool IsDuplicateMessage(uint32_t msgId);

private:
    MonotonicallyIncreasingCounter * NextMsgId;
    uint32_t * MaxMsgIdRcvd;
    ReceiveFlagsType * RcvFlags;
};

/**
 * @class ChipFabricState
 *
 * @brief
 *   Forward declaration of ChipFabricState
 */
class ChipFabricState;

/**
 * @class FabricStateDelegate
 *
 * @brief
 *   This abstract delegate class communicates fabric state changes.
 */
class FabricStateDelegate
{
public:
    /**
     * This method is called when ChipFabricState joins or creates a new fabric.
     *
     * @param[in] fabricState: A pointer to ChipFabricState that was changed.
     * @param[in] newFabricId: The new fabric ID of the ChipFabricState.
     *
     */
    virtual void DidJoinFabric(ChipFabricState * fabricState, uint64_t newFabricId) = 0;

    /**
     * This method is called when ChipFabricState leaves a fabric (i.e., fabric state
     * is cleared).
     *
     * @param[in] fabricState: A pointer to the ChipFabricState that was changed.
     * @param[in] oldFabricId: The old fabric ID that was cleared.
     *
     */
    virtual void DidLeaveFabric(ChipFabricState * fabricState, uint64_t oldFabricId) = 0;
};

#define KEEP

class DLL_EXPORT ChipFabricState
{
public:
#if CHIP_CONFIG_MAX_PEER_NODES <= UINT8_MAX
    typedef uint8_t PeerIndexType;
#else
    typedef uint16_t PeerIndexType;
#endif

    enum FabricState
    {
        kState_NotInitialized = 0,
        kState_Initialized    = 1
    };

    ChipFabricState(void);

    uint64_t FabricId; // [READ ONLY] Node's Fabric Id (0 means node is not a member of a fabric).
    uint64_t LocalNodeId;
    uint16_t DefaultSubnet;
    FabricState State; // [READ ONLY] State of the CHIP Fabric State object

#if CHIP_CONFIG_ENABLE_TARGETED_LISTEN
    IPAddress ListenIPv4Addr;
    IPAddress ListenIPv6Addr;
#endif

    CHIP_ERROR Init(void);
    CHIP_ERROR Shutdown(void);

    IPAddress SelectNodeAddress(uint64_t nodeId, uint16_t subnet) const;
    IPAddress SelectNodeAddress(uint64_t nodeId) const;
    bool IsFabricAddress(const IPAddress & addr) const;
    bool IsLocalFabricAddress(const IPAddress & addr) const;

    CHIP_ERROR CreateFabric(void);
    void ClearFabricState(void);
    CHIP_ERROR GetFabricState(uint8_t * buf, uint32_t bufSize, uint32_t & fabricStateLen);
    CHIP_ERROR JoinExistingFabric(const uint8_t * fabricState, uint32_t fabricStateLen);

    /**
     * This method sets the delegate object.
     * The callback methods of delegate are invoked whenever the FabricId is changed,
     * i.e., when we join/create a fabric, or when we leave a fabric (clear fabric state)
     *
     * If the delegate is previously set, then a second call to this method will overwrite
     * the previous delegate.
     *
     * @param[in] aDelegate                     The delegate object. It can be NULL if no
     *                                          delegate is required.
     */
    void SetDelegate(FabricStateDelegate * aDelegate);

private:
    PeerIndexType PeerCount;
    MonotonicallyIncreasingCounter NextUnencUDPMsgId;
    MonotonicallyIncreasingCounter NextUnencTCPMsgId;
    struct
    {
        uint64_t NodeId[CHIP_CONFIG_MAX_PEER_NODES];
        uint32_t MaxUnencUDPMsgIdRcvd[CHIP_CONFIG_MAX_PEER_NODES];
        ChipSessionState::ReceiveFlagsType UnencRcvFlags[CHIP_CONFIG_MAX_PEER_NODES];
        // Array of peer indexes in sorted order from most- to least- recently used.
        PeerIndexType MostRecentlyUsedIndexes[CHIP_CONFIG_MAX_PEER_NODES];
    } PeerStates;
    FabricStateDelegate * Delegate;

    bool FindOrAllocPeerEntry(uint64_t peerNodeId, bool allocEntry, PeerIndexType & retPeerIndex);
};

} // namespace chip

#endif // FABRICSTATE_H
