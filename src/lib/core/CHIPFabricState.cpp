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
 *      This file implements objects for managing the active node
 *      state necessary to participate in a CHIP fabric.
 *
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif // __STDC_LIMIT_MACROS
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif // __STDC_FORMAT_MACROS

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#include <crypto/CHIPCryptoPAL.h>
#include <core/CHIPFabricState.h>
#include <core/CHIPCore.h>
#include <core/CHIPEncoding.h>
#include <core/CHIPTLV.h>
#include <support/CodeUtils.h>
#include <support/RandUtils.h>
#include <support/logging/CHIPLogging.h>

#if HAVE_NEW
#include <new>
#else
inline void * operator new(size_t, void * p) throw()
{
    return p;
}
inline void * operator new[](size_t, void * p) throw()
{
    return p;
}
#endif

namespace chip {

using namespace chip::TLV;
using namespace chip::Encoding;

#if !CHIP_CONFIG_REQUIRE_AUTH
#pragma message "\n \
                 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n \
                 !!!!  WARNING - REQUIRE_AUTH IS DISABLED   !!!!\n \
                 !!!! CLIENT AUTHENTICATION IS NOT REQUIRED !!!!\n \
                 !!!!    DEVELOPMENT ONLY -- DO NOT SHIP    !!!!\n \
                 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n \
                 "
#endif

#ifndef nlDEFINE_ALIGNED_VAR
#define nlDEFINE_ALIGNED_VAR(varName, bytes, alignment_type)                                                                       \
    alignment_type varName[(((bytes) + (sizeof(alignment_type) - 1)) / sizeof(alignment_type))]
#endif

namespace {

/**
 * Fabric Provisioning Data Element Tags
 */
enum
{
    // ---- Top-level Data Elements ----
    kTag_FabricConfig = 1, /**< [ structure ] Contains provisioning information for an existing fabric.
                                  IMPORTANT NOTE: As a convenience to readers, all elements in a FabricConfig
                                  must be encoded in numeric tag order, at all levels. */

    // ---- Context-specific Tags for FabricConfig Structure ----
    kTag_FabricId = 1, /**< [ uint ] Fabric ID. */
};

/**
 * Profile values and tags from Weave protocol, kept for CHIP porting
 * purposes. These are @deprecated and should be removed.
 */

enum ChipVendorId
{
    kChipVendor_Common = 0
};

enum ChipProfileId
{
    kChipProfile_Common             = (kChipVendor_Common << 16) | 0x0000, // Common Profile
    kChipProfile_FabricProvisioning = (kChipVendor_Common << 16) | 0x0005, // Fabric Provisioning Profile
};

} // namespace

ChipFabricState::ChipFabricState()
{
    State = kState_NotInitialized;
}

CHIP_ERROR ChipFabricState::Init()
{
    if (State != kState_NotInitialized)
        return CHIP_ERROR_INCORRECT_STATE;

    FabricId      = kFabricIdNotSpecified;
    LocalNodeId   = 1;
    DefaultSubnet = kChipSubnetId_PrimaryWiFi;
    PeerCount     = 0;
    NextUnencUDPMsgId.Init(GetRandU32());
    NextUnencTCPMsgId.Init(0);
    memset(&PeerStates, 0, sizeof(PeerStates));
    Delegate = NULL;

#if CHIP_CONFIG_ENABLE_TARGETED_LISTEN

    ListenIPv4Addr = IPAddress::Any;
    ListenIPv6Addr = IPAddress::Any;

#if defined(DEBUG) && !CHIP_SYSTEM_CONFIG_USE_LWIP
    {
        const char * envVal = getenv("CHIP_IPV6_LISTEN_ADDR");
        if (envVal != NULL)
            IPAddress::FromString(envVal, ListenIPv6Addr);
        envVal = getenv("CHIP_IPV4_LISTEN_ADDR");
        if (envVal != NULL)
            IPAddress::FromString(envVal, ListenIPv4Addr);
    }
#endif

#endif

    State = kState_Initialized;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipFabricState::Shutdown()
{
    State = kState_NotInitialized;

    return CHIP_NO_ERROR;
}

/**
 * Returns an IPAddress containing a CHIP ULA for a specified node.
 *
 * This variant allows for a subnet to be specified.
 *
 * @param[in] nodeId            The Node ID number of the node in question.
 *
 * @param[in] subnet            The desired subnet of the ULA.
 *
 * @retval IPAddress            An IPAddress object.
 */
IPAddress ChipFabricState::SelectNodeAddress(uint64_t nodeId, uint16_t subnetId) const
{
    // Translate 'any' node id to the IPv6 link-local all-nodes multicast address.
    if (nodeId == kAnyNodeId)
    {
        return IPAddress::MakeIPv6WellKnownMulticast(kIPv6MulticastScope_Link, kIPV6MulticastGroup_AllNodes);
    }
    else
    {
        return IPAddress::MakeULA(FabricId, subnetId, ChipNodeIdToIPv6InterfaceId(nodeId));
    }
}

/**
 * Returns an IPAddress containing a CHIP ULA for a specified node.
 *
 * This variant uses the local node's default subnet.
 *
 * @param[in] nodeId            The Node ID number of the node in question.
 *
 * @retval IPAddress            An IPAddress object.
 */
IPAddress ChipFabricState::SelectNodeAddress(uint64_t nodeId) const
{
    return ChipFabricState::SelectNodeAddress(nodeId, DefaultSubnet);
}

/**
 * Determines if an IP address represents an address of a node within the local CHIP fabric.
 */
bool ChipFabricState::IsFabricAddress(const IPAddress & addr) const
{
    return (FabricId != kFabricIdNotSpecified && addr.IsIPv6ULA() && addr.GlobalId() == ChipFabricIdToIPv6GlobalId(FabricId));
}

/**
 * Determines if an IP address represents a CHIP fabric address for the local node.
 */
bool ChipFabricState::IsLocalFabricAddress(const IPAddress & addr) const
{
    return (IsFabricAddress(addr) && IPv6InterfaceIdToChipNodeId(addr.InterfaceId()) == LocalNodeId);
}

/**
 * This method finds, allocates (optional), and returns index to the peer entry in the peer state table.
 *
 * @param[in]  peerNodeId       The node identifier of the peer.
 * @param[in]  allocEntry       A boolean value indicating whether new entry should be
 *                              allocated for the specified peer if not found in the table.
 * @param[out] retPeerIndex     Index to the specified peer entry in the peer state table.
 *
 * @retval bool                 Whether or not peer's entry found in the peer state table.
 *                              Note, that function always returns true if entry allocation
 *                              was requested.
 *
 */
bool ChipFabricState::FindOrAllocPeerEntry(uint64_t peerNodeId, bool allocEntry, PeerIndexType & retPeerIndex)
{
    uint16_t i;
    bool retVal = false;

    // Find peer entry in the peer state table.
    for (i = 0; i < PeerCount; i++)
    {
        retPeerIndex = PeerStates.MostRecentlyUsedIndexes[i];
        if (PeerStates.NodeId[retPeerIndex] == peerNodeId)
        {
            retVal = true;
            break;
        }
    }

    // If peer entry is not found in the peer state table and allocation was requested.
    if (!retVal && allocEntry)
    {
        // If PeerStates table is full then the least recently used entry is discarded
        // and allocated for the new peer node. The replacement algorithms tries to find
        // least recently used entry that didn't use encryption to avoid future
        // complexity associated with encrypted message counter synchronization.
        if (PeerCount == CHIP_CONFIG_MAX_PEER_NODES)
        {
            // Choose the least recently used peer entry by default.
            i = CHIP_CONFIG_MAX_PEER_NODES - 1;

            // The peer index chosen for replacement.
            retPeerIndex = PeerStates.MostRecentlyUsedIndexes[i];
        }

        // If PeerStates table is not full then the next available entry is "i".
        // Entries in the table are allocated sequentially and never discarded until
        // the table is full. Only when table is full the least recently used entry
        // is discarded and replaced with the new entry.
        else
        {
            PeerCount++;
            retPeerIndex = i;
        }

        PeerStates.NodeId[retPeerIndex]               = peerNodeId;
        PeerStates.MaxUnencUDPMsgIdRcvd[retPeerIndex] = 0;
        PeerStates.UnencRcvFlags[retPeerIndex]        = 0;
        retVal                                        = true;
    }

    // Move the requested entry to the top of the most recently used indexes list.
    if (retVal)
    {
        memmove(&PeerStates.MostRecentlyUsedIndexes[1], &PeerStates.MostRecentlyUsedIndexes[0], i * sizeof(PeerIndexType));
        PeerStates.MostRecentlyUsedIndexes[0] = retPeerIndex;
    }

    return retVal;
}

CHIP_ERROR ChipFabricState::CreateFabric()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Fail if the node is already a member of a fabric.
    if (FabricId != 0)
        return CHIP_ERROR_INCORRECT_STATE;

    // Make sure the fabric state is cleared.
    ClearFabricState();

    {
        // Generate a unique id for the new fabric, being careful to
        // avoid reserved ids.
        //
        // NOTE: The fabric id is used to form the global ids that
        // make up the fabric IPv6 unique local addresses, as
        // described in RFC 4193. The mechanism used here to create
        // the fabric id (essentially a CSRNG) is different from the
        // algorithm described in the RFC.  However the algorithm in
        // the RFC is a suggestion and this algorithm is more
        // straightforward and is sufficient to produce the required
        // uniqueness.
        do
        {

            err = chip::Crypto::DRBG_get_bytes((unsigned char *) &FabricId, sizeof(FabricId));
            SuccessOrExit(err);
        } while (FabricId == kFabricIdNotSpecified || FabricId >= kReservedFabricIdStart);
    }

    // Create an initial fabric secret.

    if (Delegate != NULL)
        Delegate->DidJoinFabric(this, FabricId);

exit:
    if (err != CHIP_NO_ERROR)
        ClearFabricState();

    return err;
}

void ChipFabricState::ClearFabricState()
{
    uint64_t oldFabricId;

    oldFabricId = FabricId;
    FabricId    = kFabricIdNotSpecified;

    if (oldFabricId != kFabricIdNotSpecified)
    {
        if (Delegate != NULL)
            Delegate->DidLeaveFabric(this, oldFabricId);
    }
}

CHIP_ERROR ChipFabricState::GetFabricState(uint8_t * buf, uint32_t bufSize, uint32_t & fabricStateLen)
{
    CHIP_ERROR err;
    TLVWriter writer;
    TLVType containerType;

    // Fail if the node is not a member of a fabric.
    if (FabricId == 0)
        return CHIP_ERROR_INCORRECT_STATE;

    // IMPORTANT NOTE: As a convenience to readers, all elements in a FabricConfig
    // must be encoded in numeric tag order, at all levels.

    writer.Init(buf, bufSize);

    err = writer.StartContainer(ProfileTag(kChipProfile_FabricProvisioning, kTag_FabricConfig), kTLVType_Structure, containerType);
    SuccessOrExit(err);

    err = writer.Put(ContextTag(kTag_FabricId), FabricId);
    SuccessOrExit(err);

    err = writer.EndContainer(containerType);
    SuccessOrExit(err);

    err = writer.Finalize();
    SuccessOrExit(err);

    fabricStateLen = writer.GetLengthWritten();

exit:
    return err;
}

CHIP_ERROR ChipFabricState::JoinExistingFabric(const uint8_t * fabricState, uint32_t fabricStateLen)
{
    CHIP_ERROR err;
    TLVReader reader;

    // Fail if the node is already a member of a fabric.
    if (FabricId != 0)
        return CHIP_ERROR_INCORRECT_STATE;

    // Make sure the fabric state is cleared.
    ClearFabricState();

    reader.Init(fabricState, fabricStateLen);

    err = reader.Next(kTLVType_Structure, ProfileTag(kChipProfile_FabricProvisioning, kTag_FabricConfig));
    SuccessOrExit(err);

    {
        TLVType containerType;

        err = reader.EnterContainer(containerType);
        SuccessOrExit(err);

        err = reader.Next(kTLVType_UnsignedInteger, ContextTag(kTag_FabricId));
        SuccessOrExit(err);

        err = reader.Get(FabricId);
        SuccessOrExit(err);
    }

    if (Delegate != NULL)
        Delegate->DidJoinFabric(this, FabricId);

exit:
    if (err != CHIP_NO_ERROR)
        ClearFabricState();

    return err;
}

// ChipSessionState Members

ChipSessionState::ChipSessionState(void)
{
    NextMsgId    = NULL;
    MaxMsgIdRcvd = NULL;
    RcvFlags     = NULL;
}

ChipSessionState::ChipSessionState(MonotonicallyIncreasingCounter * nextMsgId, uint32_t * maxMsgIdRcvd, ReceiveFlagsType * rcvFlags)
{
    NextMsgId    = nextMsgId;
    MaxMsgIdRcvd = maxMsgIdRcvd;
    RcvFlags     = rcvFlags;
}

uint32_t ChipSessionState::NewMessageId(void)
{
    uint32_t newMsgId = NextMsgId->GetValue();

    NextMsgId->Advance();

    return newMsgId;
}

bool ChipSessionState::MessageIdNotSynchronized(void)
{
    return (RcvFlags == NULL) || (((*RcvFlags) & kReceiveFlags_MessageIdSynchronized) == 0);
}

bool ChipSessionState::IsDuplicateMessage(uint32_t msgId)
{
    bool isDup = false;
    int32_t delta;
    ReceiveFlagsType msgIdFlags;

    // This algorithm relies on two values to determine whether a message has been received before:
    //
    //    *MaxMsgIdRcvd is the maximum message id received from from the peer node.
    //
    //    *RcvFlags is a set of flags describing the history of message reception from the peer.
    //
    //        The top-most bit in *RcvFlags indicates whether any messages have ever been received from the peer.
    //
    //            The remaining bits represent individual message ids that have been received prior to the
    //        message identified by *MaxMsgIdRcvd.  Specifically, bit 0 represents the message immediately
    //        prior to the max id message (i.e. *MaxMsgIdRcvd - 1), bit 1 represents the message immediately
    //        prior to that message, and so on.

    // If message Id is not synchronized.
    if (MessageIdNotSynchronized())
    {
        // Return immediately if duplicate message detection is disabled for this session.
        //
        // This happens for unencrypted messages sent over TCP. Such messages provide no security against replay
        // attacks (since they are not encrypted) and are not subject to message reordering in the network layer
        // (since TCP eliminates such reorderings). Thus duplicate message detection is unnecessary in this case.
        if (RcvFlags == NULL)
        {
            ExitNow();
        }

        // Otherwise mark message as synchronized and initialize peer's max counter.
        else
        {
            *RcvFlags     = kReceiveFlags_MessageIdSynchronized;
            *MaxMsgIdRcvd = msgId;
            ExitNow();
        }
    }

    // Extract the message id flags from the receive flags field.
    msgIdFlags = (*RcvFlags) & kReceiveFlags_MessageIdFlagsMask;

    // Determine the difference between the id of the newly received message (msgId) and the maximum message
    // id received so far (*MaxMsgIdRcvd).
    //
    // Note that the math here is designed to accommodate wrapping of message ids.  Specifically, any newly
    // received message with an id in the range (*MaxMsgIdRcvd + 1) to ((*MaxMsgIdRcvd + 2^31 - 1) mod 2^32)
    // will be considered to be later than the max id message (i.e. delta > 0), and thus cannot be a duplicate.
    // Conversely any message with an id not in this range (i.e. delta <= 0) represents an earlier message
    // (or the same message) and thus may be a duplicate.
    //
    // This approach ensures that duplicates will continue to be detected for an amount of time equal to
    // (send-rate * 2^31) past a message's original send time, while allowing (send-rate * (2^31 - 1)) time
    // between message arrivals before a new message will be mistakenly considered a duplicate.
    //
    delta = (int32_t)(msgId - *MaxMsgIdRcvd);

    // If the new message was sent after the max id message...
    if (delta > 0)
    {
        // Shift the message received flags by the delta (or simply set the flags to zero if the delta is larger
        // than the number of flags).
        if (delta < kReceiveFlags_NumMessageIdFlags)
            msgIdFlags = ((msgIdFlags << 1) | 1) << (delta - 1);
        else
            msgIdFlags = 0;

        // Update the max received message id.
        *MaxMsgIdRcvd = msgId;
    }

    // If the new id is the same as the max id message, the message is a duplicate.
    else if (delta == 0)
    {
        ExitNow(isDup = true);
    }
    // Otherwise the new message was sent earlier than the max id message...
    else
    {
        // Make the delta positive.
        delta = -delta;

        // If the delta is within the range of the message id flags, form the appropriate mask
        // and check if the message has already been received. If not, set the corresponding flag.
        if (delta <= kReceiveFlags_NumMessageIdFlags)
        {
            ReceiveFlagsType mask = 1 << (delta - 1);
            if ((msgIdFlags & mask) == 0)
                msgIdFlags |= mask;
            else
            {
                ExitNow(isDup = true);
            }
        }

        // If the delta is greater than the range of the message id flags...
        else
        {
            // Senders of unencrypted messages are not required to preserve message id ordering across restarts.
            // Since duplicate message detection for unencrypted messages is only to eliminate duplicates created
            // in the network layer, we allow message ids for unencrypted messages from the same peer to go backwards.
            msgIdFlags    = 0;
            *MaxMsgIdRcvd = msgId;
        }
    }

    // Update the message id flags within the receive flags value and set the MessageIdIsSynchronized flag.
    *RcvFlags = kReceiveFlags_MessageIdSynchronized | msgIdFlags | (*RcvFlags & ~kReceiveFlags_MessageIdFlagsMask);

exit:
    return isDup;
}

void ChipFabricState::SetDelegate(FabricStateDelegate * aDelegate)
{
    Delegate = aDelegate;
}

CHIP_ERROR ChipFabricState::GetSessionState(uint64_t remoteNodeId, ChipConnection *con, ChipSessionState& outSessionState) {
    if (con == NULL) {
        PeerIndexType peerIndex;
        FindOrAllocPeerEntry(remoteNodeId, true, peerIndex);
        outSessionState = ChipSessionState(
                &NextUnencUDPMsgId, &PeerStates.MaxUnencUDPMsgIdRcvd[peerIndex], &PeerStates.UnencRcvFlags[peerIndex]);
    } else{
        outSessionState = ChipSessionState(&NextUnencTCPMsgId, NULL, NULL);
    }

   return CHIP_NO_ERROR;
}


} // namespace chip
