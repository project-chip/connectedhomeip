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

#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <Profiles/fabric-provisioning/FabricProvisioning.h>
#include <Profiles/security/CHIPApplicationKeys.h>
#include <Profiles/security/CHIPDummyGroupKeyStore.h>
#include <Profiles/security/CHIPSecurity.h>
#include <core/CHIPCore.h>
#include <core/CHIPEncoding.h>
#include <core/CHIPKeyIds.h>
#include <core/CHIPTLV.h>
#include <crypto/CHIPCryptoPAL.h>
#include <message/CHIPFabricState.h>
#include <profiles/CHIPProfiles.h>
#include <support/CodeUtils.h>
#include <support/RandUtils.h>
#include <support/crypto/CHIPRNG.h>
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
using namespace chip::Crypto;
using namespace chip::Profiles;
using namespace chip::Profiles::FabricProvisioning;
using namespace chip::Profiles::Security;
using namespace chip::Profiles::Security::AppKeys;

#if CHIP_CONFIG_SECURITY_TEST_MODE
#pragma message "\n \
                 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n \
                 !!!!    WARNING - SECURITY_TEST_MODE IS ENABLED    !!!!\n \
                 !!!! BASIC CHIP SECURITY / ENCRYPTION IS CRIPPLED !!!!\n \
                 !!!!        DEVELOPMENT ONLY -- DO NOT SHIP        !!!!\n \
                 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n \
                 "
#endif

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

// Identifies the scheme used to rotate fabric secret. The use of this
// identifier is deprecated because fabric secret value never rotates.
//
// NOTE: This value is communicated over the wire--do not renumber.
//
typedef uint8_t FabricSecretRotationScheme;

enum
{
    kDeprecatedRotationScheme = 0x00
};

// Key diversifier used for CHIP message encryption key derivation.
const uint8_t kChipMsgEncAppKeyDiversifier[] = { 0xB1, 0x1D, 0xAE, 0x5B };

/**
 * Initialize a ChipSessionKey object.
 */
void ChipSessionKey::Init(void)
{
    NodeId = kNodeIdNotSpecified;
    NextMsgId.Init(0);
    MaxRcvdMsgId = 0;
    BoundCon     = NULL;
    RcvFlags     = 0;
    AuthMode     = kChipAuthMode_NotSpecified;
    memset(&MsgEncKey, 0, sizeof(MsgEncKey));
    ReserveCount = 0;
    Flags        = 0;
}

/**
 * Reset a ChipSessionKey object.
 */
void ChipSessionKey::Clear(void)
{
    Init();
    ClearSecretData((uint8_t *) &MsgEncKey.EncKey, sizeof(MsgEncKey.EncKey));
}

/**
 * @fn bool ChipSessionKey::IsAllocated() const
 *
 * @return True if the ChipSessionKey object is allocated.
 */

/**
 * @fn bool ChipSessionKey::IsKeySet() const
 *
 * @return True if the encryption key value has been set in a ChipSessionKey object.
 */

/**
 * @fn bool ChipSessionKey::IsLocallyInitiated() const
 *
 * @return True if the session was initiated by the local node.
 */

/*
 * @fn void ChipSessionKey::SetLocallyInitiated(bool val)
 *
 * Sets a flag indicating whether the session was initiated by the local node.
 *
 * @param[in] val The value to set the kFlag_IsLocallyInitiated flag to.
 */

/*
 * @fn bool ChipSessionKey::IsSharedSession() const
 *
 * @return True if the session is shared--i.e. can be used for multiplexed communication with different peer node ids.
 */

/**
 * @fn void ChipSessionKey::SetSharedSession(bool val)
 *
 * Sets a flag indicating whether the session is a shared session.
 *
 * @param[in] val The value to set the kFlag_IsSharedSession flag to.
 */

/**
 * @fn bool ChipSessionKey::IsRemoveOnIdle() const
 *
 * @return True if the session is flagged for automatic removal when idle for a period of time.
 */

/**
 * @fn void ChipSessionKey::SetRemoveOnIdle(bool val)
 *
 * Sets a flag indicating whether the session should be automatically removed after a period of idle time.
 *
 * @param[in] val The value to set the kFlag_IsRemoveOnIdle flag to.
 */

/**
 * @fn bool ChipSessionKey::IsRecentlyActive() const
 *
 * @return True if the session has been active in the recent past.
 */

/**
 * @fn void ChipSessionKey::MarkRecentlyActive()
 *
 * Signals the session as having been active in the recent past.
 */

/**
 * @fn void ChipSessionKey::ClearRecentlyActive()
 *
 * Signals the session as NOT having been active in the recent past.
 */

ChipFabricState::ChipFabricState()
{
    State = kState_NotInitialized;
}

CHIP_ERROR ChipFabricState::Init()
{
    static nlDEFINE_ALIGNED_VAR(sDummyGroupKeyStore, sizeof(DummyGroupKeyStore), void *);

    return Init(new (&sDummyGroupKeyStore) DummyGroupKeyStore());
}

CHIP_ERROR ChipFabricState::Init(GroupKeyStoreBase * groupKeyStore)
{
    if (State != kState_NotInitialized)
        return CHIP_ERROR_INCORRECT_STATE;

    if (groupKeyStore == NULL)
        return CHIP_ERROR_INVALID_ARGUMENT;

#ifdef CHIP_NON_PRODUCTION_MARKER
    // This is a trick to force the linker to include the CHIP_NON_PRODUCTION_MARKER symbol
    // in the linked output.  (Note that the test will never evaluate to true).
    if (CHIP_NON_PRODUCTION_MARKER[0] == 0)
        return CHIP_ERROR_INCORRECT_STATE;
#endif

    GroupKeyStore = groupKeyStore;

    FabricId      = kFabricIdNotSpecified;
    LocalNodeId   = 1;
    PairingCode   = NULL;
    DefaultSubnet = kChipSubnetId_PrimaryWiFi;
    PeerCount     = 0;
    NextUnencUDPMsgId.Init(GetRandU32());
    NextUnencTCPMsgId.Init(0);
    for (int i = 0; i < CHIP_CONFIG_MAX_SESSION_KEYS; i++)
        SessionKeys[i].Init();
#if CHIP_CONFIG_USE_APP_GROUP_KEYS_FOR_MSG_ENC
    CHIP_ERROR err =
        NextGroupKeyMsgId.Init(CHIP_CONFIG_PERSISTED_STORAGE_ENC_MSG_CNTR_ID, CHIP_CONFIG_PERSISTED_STORAGE_ENC_MSG_CNTR_EPOCH);
    if (err != CHIP_NO_ERROR)
        return err;

    GroupKeyMsgIdFreshWindowStart = 0;
    MsgCounterSyncStatus          = 0;
    AppKeyCache.Init();
#endif
    memset(&PeerStates, 0, sizeof(PeerStates));
    Delegate = NULL;
    memset(SharedSessionsNodes, 0, sizeof(SharedSessionsNodes));

#if CHIP_CONFIG_SECURITY_TEST_MODE
    DebugFabricId  = 0;
    LogKeys        = false;
    UseTestKey     = false; // DEPRECATED -- Temporarily retained for API compatibility
    AutoCreateKeys = false; // DEPRECATED -- Temporarily retained for API compatibility
#endif                      // CHIP_CONFIG_SECURITY_TEST_MODE

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

    sessionEndCallbackList = NULL;

    State = kState_Initialized;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipFabricState::Shutdown()
{
    State = kState_NotInitialized;

#if CHIP_CONFIG_USE_APP_GROUP_KEYS_FOR_MSG_ENC
    AppKeyCache.Shutdown();
#endif

    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipFabricState::AllocSessionKey(uint64_t peerNodeId, uint16_t keyId, ChipConnection * boundCon,
                                            ChipSessionKey *& sessionKey)
{
    CHIP_ERROR err;
    bool chooseRandomKeyId = (keyId == ChipKeyId::kNone);

    while (true)
    {
        if (chooseRandomKeyId)
            keyId = ChipKeyId::MakeSessionKeyId(GetRandU16());
        err = FindSessionKey(keyId, peerNodeId, true, sessionKey);
        if (err != CHIP_NO_ERROR)
            return err;
        if (!sessionKey->IsAllocated())
            break;
        if (!chooseRandomKeyId)
            return CHIP_ERROR_DUPLICATE_KEY_ID;
    }

    sessionKey->MsgEncKey.KeyId   = keyId;
    sessionKey->NodeId            = peerNodeId;
    sessionKey->MsgEncKey.EncType = kChipEncryptionType_None;
    sessionKey->NextMsgId.Init(UINT32_MAX);
    sessionKey->MaxRcvdMsgId = UINT32_MAX;
    sessionKey->BoundCon     = boundCon;
    sessionKey->RcvFlags     = 0;
    sessionKey->Flags        = ChipSessionKey::kFlag_RecentlyActive;
    sessionKey->ReserveCount = 1;

    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipFabricState::SetSessionKey(uint16_t keyId, uint64_t peerNodeId, uint8_t encType, ChipAuthMode authMode,
                                          const ChipEncryptionKey * encKey)
{
    CHIP_ERROR err;
    ChipSessionKey * sessionKey;

    err = FindSessionKey(keyId, peerNodeId, false, sessionKey);
    SuccessOrExit(err);

    err = SetSessionKey(sessionKey, encType, authMode, encKey);
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR ChipFabricState::SetSessionKey(ChipSessionKey * sessionKey, uint8_t encType, ChipAuthMode authMode,
                                          const ChipEncryptionKey * encKey)
{
    CHIP_ERROR err;
    uint32_t msgId;

    // Randomize the initial 32-bit message counter on session establishment.
    // This value should be secure random to prevent man-in-the-middle adversary
    // guessing this number.
    err = chip::Platform::Security::GetSecureRandomData(reinterpret_cast<uint8_t *>(&msgId), sizeof(msgId));
    SuccessOrExit(err);

    sessionKey->MsgEncKey.EncType = encType;
    sessionKey->MsgEncKey.EncKey  = *encKey;
    sessionKey->NextMsgId.Init(msgId);
    sessionKey->MaxRcvdMsgId = 0;
    sessionKey->RcvFlags     = 0;
    sessionKey->AuthMode     = authMode;

#if CHIP_CONFIG_SECURITY_TEST_MODE && CHIP_DETAIL_LOGGING
    if (LogKeys)
    {
        char keyString[kMaxEncKeyStringSize];
        ChipEncryptionKeyToString(encType, *encKey, keyString, sizeof(keyString));
        ChipLogDetail(MessageLayer,
                      "Message Encryption Key: Id=%04" PRIX16 " Type=SessionKey Peer=%016" PRIX64 " EncType=%02" PRIX8 " Key=%s",
                      sessionKey->MsgEncKey.KeyId, sessionKey->NodeId, encType, keyString);
    }
#endif // CHIP_CONFIG_SECURITY_TEST_MODE

exit:
    return err;
}

CHIP_ERROR ChipFabricState::RemoveSessionKey(uint16_t keyId, uint64_t peerNodeId)
{
    CHIP_ERROR err;
    ChipSessionKey * sessionKey;

    err = FindSessionKey(keyId, peerNodeId, false, sessionKey);
    SuccessOrExit(err);

    RemoveSessionKey(sessionKey);

    NotifySessionEndSubscribers(keyId, peerNodeId);

exit:
    return err;
}

void ChipFabricState::RemoveSessionKey(ChipSessionKey * sessionKey, bool wasIdle)
{
    ChipLogDetail(MessageLayer, "Removing %ssession key: Id=%04" PRIX16 " Peer=%016" PRIX64, (wasIdle) ? "idle " : "",
                  sessionKey->MsgEncKey.KeyId, sessionKey->NodeId);

    RemoveSharedSessionEndNodes(sessionKey);
    sessionKey->Clear();
}

CHIP_ERROR ChipFabricState::GetSessionKey(uint16_t keyId, uint64_t peerNodeId, ChipSessionKey *& outSessionKey)
{
    return FindSessionKey(keyId, peerNodeId, false, outSessionKey);
}

/**
 * Search the session keys table for an established shared session key that targets the specified
 * terminating node and matches the given auth mode and encryption type.
 *
 * @param[in]  terminatingNodeId  The node identifier of the session terminator.
 * @param[in]  authMode           The desired session authentication mode.
 * @param[in]  encType            The desired message encryption type.
 *
 * @retval     ChipSessionKey *  A pointer to a ChipSessionKey object representing the matching
 *                                shared session; or NULL if no matching session was found.
 *
 */
ChipSessionKey * ChipFabricState::FindSharedSession(uint64_t terminatingNodeId, ChipAuthMode authMode, uint8_t encType)
{
    ChipSessionKey * sessionKey;

    // Search the session keys table for an established shared session key that targets the specified
    // terminating node and matches the given auth mode and encryption type.
    sessionKey = SessionKeys;
    for (int i = 0; i < CHIP_CONFIG_MAX_SESSION_KEYS; i++, sessionKey++)
    {
        if (sessionKey->IsAllocated() && sessionKey->IsKeySet() && sessionKey->IsSharedSession() &&
            sessionKey->NodeId == terminatingNodeId && sessionKey->AuthMode == authMode && sessionKey->MsgEncKey.EncType == encType)
        {
            return sessionKey;
        }
    }

    return NULL;
}

/**
 * This method checks whether secure session associated with the specified peer and keyId is shared.
 *
 * @param[in]  keyId              The session key identifier.
 * @param[in]  peerNodeId         The node identifier of the peer.
 *
 * @retval     bool               Whether or not specified session is shared.
 *
 */
bool ChipFabricState::IsSharedSession(uint16_t keyId, uint64_t peerNodeId)
{
    CHIP_ERROR err;
    ChipSessionKey * sessionKey;
    bool retVal = false;

    err = FindSessionKey(keyId, peerNodeId, false, sessionKey);
    SuccessOrExit(err);

    retVal = sessionKey->IsSharedSession();

exit:
    return retVal;
}

/**
 * This method checks whether end node already recorded and returns true if
 * node is found in the shared end node list.
 *
 * @param[in]  endNodeId          Identifier of the session end node.
 * @param[in]  sessionKey         A pointer to the session key object.
 *
 * @retval     bool               Whether or not end node exists in the
 *                                shared end nodes list.
 *
 */
bool ChipFabricState::FindSharedSessionEndNode(uint64_t endNodeId, const ChipSessionKey * sessionKey)
{
    SharedSessionEndNode * endNode = SharedSessionsNodes;
    bool retVal                    = false;

    for (int i = 0; i < CHIP_CONFIG_MAX_SHARED_SESSIONS_END_NODES; i++, endNode++)
    {
        if (endNode->SessionKey == sessionKey && endNode->EndNodeId == endNodeId)
        {
            retVal = true;
            break;
        }
    }

    return retVal;
}

CHIP_ERROR ChipFabricState::AddSharedSessionEndNode(uint64_t endNodeId, uint64_t terminatingNodeId, uint16_t keyId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ChipSessionKey * sessionKey;

    err = FindSessionKey(keyId, terminatingNodeId, false, sessionKey);
    SuccessOrExit(err);

    err = AddSharedSessionEndNode(sessionKey, endNodeId);
    SuccessOrExit(err);

exit:
    return err;
}

/**
 * This method adds new end node to the shared end nodes record.
 *
 * @param[in]  sessionKey         The ChipSessionKey object representing the session for which the new
 *                                end node should be added.
 * @param[in]  endNodeId          The node id of the session end node to be added.
 *
 * @retval #CHIP_ERROR_TOO_MANY_SHARED_SESSION_END_NODES
 *                                If there is no free space for a new entry in the shared end nodes list.
 * @retval #CHIP_NO_ERROR        On success.
 *
 */
CHIP_ERROR ChipFabricState::AddSharedSessionEndNode(ChipSessionKey * sessionKey, uint64_t endNodeId)
{
    CHIP_ERROR err                     = CHIP_NO_ERROR;
    SharedSessionEndNode * endNode     = SharedSessionsNodes;
    SharedSessionEndNode * freeEndNode = NULL;
    uint8_t endNodeCount               = 0;

    // No need to add new shared entry record if the end node is also the terminating node.
    VerifyOrExit(endNodeId != sessionKey->NodeId, /* Exit without error. */);

    // Check if entry already exists.
    for (int i = 0; i < CHIP_CONFIG_MAX_SHARED_SESSIONS_END_NODES; i++, endNode++)
    {
        if (endNode->SessionKey == sessionKey)
        {
            if (endNode->EndNodeId == endNodeId)
            {
                ExitNow();
            }
            else
            {
                endNodeCount++;
            }
        }
        else if (endNode->EndNodeId == kNodeIdNotSpecified && freeEndNode == NULL)
        {
            freeEndNode = endNode;
        }
    }

    // Verify that there is free entry in the list and that we don't exit maximum
    // allowed number of end nodes per single shared session.
    VerifyOrExit(freeEndNode != NULL && endNodeCount <= CHIP_CONFIG_MAX_END_NODES_PER_SHARED_SESSION,
                 err = CHIP_ERROR_TOO_MANY_SHARED_SESSION_END_NODES);

    // Add new end node.
    freeEndNode->EndNodeId  = endNodeId;
    freeEndNode->SessionKey = sessionKey;

exit:
    return err;
}

/**
 * This method returns all end node IDs that share specified session.
 *
 * @param[in]  sessionKey         A pointer to the session key object.
 * @param[in]  endNodeIds         A pointer to buffer of node IDs.
 * @param[in]  endNodeIdsMaxCount The maximum number of node IDs that can fit in the buffer.
 * @param[out] endNodeIdsCount    Number of found end node IDs that share specified session.
 *
 * @retval #CHIP_ERROR_BUFFER_TOO_SMALL
 *                                If provided end node Ids buffer is not big enough.
 * @retval #CHIP_NO_ERROR        On success.
 *
 */
CHIP_ERROR ChipFabricState::GetSharedSessionEndNodeIds(const ChipSessionKey * sessionKey, uint64_t * endNodeIds,
                                                       uint8_t endNodeIdsMaxCount, uint8_t & endNodeIdsCount)
{
    CHIP_ERROR err                 = CHIP_NO_ERROR;
    SharedSessionEndNode * endNode = SharedSessionsNodes;

    endNodeIdsCount = 0;

    for (int i = 0; i < CHIP_CONFIG_MAX_SHARED_SESSIONS_END_NODES; i++, endNode++)
    {
        if (endNode->SessionKey == sessionKey)
        {
            VerifyOrExit(endNodeIdsCount < endNodeIdsMaxCount, err = CHIP_ERROR_BUFFER_TOO_SMALL);

            endNodeIds[endNodeIdsCount] = endNode->EndNodeId;
            endNodeIdsCount++;
        }
    }

exit:
    return err;
}

void ChipFabricState::RemoveSharedSessionEndNodes(const ChipSessionKey * sessionKey)
{
    if (sessionKey->IsSharedSession())
    {
        SharedSessionEndNode * endNode = SharedSessionsNodes;

        // Clear all information about shared session end nodes.
        for (int i = 0; i < CHIP_CONFIG_MAX_SHARED_SESSIONS_END_NODES; i++, endNode++)
        {
            if (endNode->SessionKey == sessionKey)
            {
                memset((uint8_t *) endNode, 0, sizeof(SharedSessionEndNode));
            }
        }
    }
}

/**
 * Suspend and serialize the state of an active CHIP security session.
 *
 * Serializes the state of an identified CHIP security session into the supplied buffer
 * and suspends the session such that no further messages can be sent or received.
 *
 * This method is intended to be used by devices that do not retain RAM while sleeping,
 * allowing them to persist the state of an active session and thereby avoid the need to
 * re-establish the session when they wake.
 */
CHIP_ERROR ChipFabricState::SuspendSession(uint16_t keyId, uint64_t peerNodeId, uint8_t * buf, uint16_t bufSize,
                                           uint16_t & serializedSessionLen)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ChipSessionKey * sessionKey;

    // Lookup the specified session.
    err = GetSessionKey(keyId, peerNodeId, sessionKey);
    SuccessOrExit(err);

    // Assert various requirements about the session.
    VerifyOrExit(sessionKey->IsKeySet(), err = CHIP_ERROR_KEY_NOT_FOUND);
    VerifyOrExit(!sessionKey->IsSuspended(), err = CHIP_ERROR_SESSION_KEY_SUSPENDED);
    VerifyOrExit(sessionKey->BoundCon == NULL, err = CHIP_ERROR_INVALID_USE_OF_SESSION_KEY);
    VerifyOrExit(IsCertAuthMode(sessionKey->AuthMode), err = CHIP_ERROR_INVALID_USE_OF_SESSION_KEY);

    {
        TLVWriter writer;
        TLVType container;

        writer.Init(buf, bufSize);

        // Begin encoding a Security:SerializedSession TLV structure.
        err = writer.StartContainer(ProfileTag(kChipProfile_Security, kTag_SerializedSession), kTLVType_Structure, container);
        SuccessOrExit(err);

        // Encode various information about the session, in tag order.
        err = writer.Put(ContextTag(kTag_SerializedSession_KeyId), sessionKey->MsgEncKey.KeyId);
        SuccessOrExit(err);
        err = writer.Put(ContextTag(kTag_SerializedSession_PeerNodeId), sessionKey->NodeId);
        SuccessOrExit(err);
        err = writer.Put(ContextTag(kTag_SerializedSession_NextMessageId), sessionKey->NextMsgId.GetValue());
        SuccessOrExit(err);
        err = writer.Put(ContextTag(kTag_SerializedSession_MaxRcvdMessageId), sessionKey->MaxRcvdMsgId);
        SuccessOrExit(err);
        err = writer.Put(ContextTag(kTag_SerializedSession_MessageRcvdFlags), sessionKey->RcvFlags);
        SuccessOrExit(err);
        err = writer.PutBoolean(ContextTag(kTag_SerializedSession_IsLocallyInitiated), sessionKey->IsLocallyInitiated());
        SuccessOrExit(err);
        err = writer.PutBoolean(ContextTag(kTag_SerializedSession_IsShared), sessionKey->IsSharedSession());
        SuccessOrExit(err);

        // If the session is shared...
        if (sessionKey->IsSharedSession())
        {
            SharedSessionEndNode * endNode = SharedSessionsNodes;
            TLVType container2;

            // Begin encoding an array containing the alternate node ids for the peer.
            err = writer.StartContainer(ContextTag(kTag_SerializedSession_SharedSessionAltNodeIds), kTLVType_Array, container2);
            SuccessOrExit(err);

            for (int i = 0; i < CHIP_CONFIG_MAX_SHARED_SESSIONS_END_NODES; i++, endNode++)
            {
                if (endNode->SessionKey == sessionKey)
                {
                    err = writer.Put(AnonymousTag, endNode->EndNodeId);
                    SuccessOrExit(err);
                }
            }

            // End the array.
            err = writer.EndContainer(container2);
            SuccessOrExit(err);
        }

        // For a CASE-based session, encode the certificate type presented by the peer.
        err = writer.Put(ContextTag(kTag_SerializedSession_CASE_PeerCertType), CertTypeFromAuthMode(sessionKey->AuthMode));
        SuccessOrExit(err);

        // Encode the encryption type used to encrypt messages via this session.
        err = writer.Put(ContextTag(kTag_SerializedSession_EncryptionType), sessionKey->MsgEncKey.EncType);
        SuccessOrExit(err);

        // Encode the encryption key(s) associated with the session.
        switch (sessionKey->MsgEncKey.EncType)
        {
        case kChipEncryptionType_AES128CTRSHA1:
            err = writer.PutBytes(ContextTag(kTag_SerializedSession_AES128CTRSHA1_DataKey),
                                  sessionKey->MsgEncKey.EncKey.AES128CTRSHA1.DataKey, ChipEncryptionKey_AES128CTRSHA1::DataKeySize);
            SuccessOrExit(err);
            err = writer.PutBytes(ContextTag(kTag_SerializedSession_AES128CTRSHA1_IntegrityKey),
                                  sessionKey->MsgEncKey.EncKey.AES128CTRSHA1.IntegrityKey,
                                  ChipEncryptionKey_AES128CTRSHA1::IntegrityKeySize);
            SuccessOrExit(err);
            break;
        default:
            ExitNow(err = CHIP_ERROR_UNSUPPORTED_ENCRYPTION_TYPE);
        }

        // End the Security:SerializedSession TLV structure and finalize the encoding.
        err = writer.EndContainer(container);
        SuccessOrExit(err);
        err = writer.Finalize();
        SuccessOrExit(err);

        serializedSessionLen = (uint16_t) writer.GetLengthWritten();
    }

    // Mark the session key as suspended.
    sessionKey->MarkSuspended();

    // Wipe the key.
    sessionKey->MsgEncKey.EncType = kChipEncryptionType_None;
    ClearSecretData((uint8_t *) &sessionKey->MsgEncKey.EncKey, sizeof(sessionKey->MsgEncKey.EncKey));

exit:
    // If something goes wrong, make sure we don't leave any key material behind.
    if (err != CHIP_NO_ERROR)
    {
        ClearSecretData(buf, bufSize);
    }
    return err;
}

/**
 * Restore a previously suspended CHIP Security Session from a serialized state.
 *
 */
CHIP_ERROR ChipFabricState::RestoreSession(uint8_t * serializedSession, uint16_t serializedSessionLen)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TLVReader reader;
    TLVType container;
    uint16_t keyId;
    uint64_t peerNodeId;
    ChipSessionKey * sessionKey = NULL;
    bool removeSessionOnError   = false;

    reader.Init(serializedSession, serializedSessionLen);

    // Look for and enter the Security:SerializedSession TLV structure.
    err = reader.Next(kTLVType_Structure, ProfileTag(kChipProfile_Security, kTag_SerializedSession));
    SuccessOrExit(err);
    err = reader.EnterContainer(container);
    SuccessOrExit(err);

    // Read the key id and peer node id.
    err = reader.Next(kTLVType_UnsignedInteger, ContextTag(kTag_SerializedSession_KeyId));
    SuccessOrExit(err);
    err = reader.Get(keyId);
    SuccessOrExit(err);
    err = reader.Next(kTLVType_UnsignedInteger, ContextTag(kTag_SerializedSession_PeerNodeId));
    SuccessOrExit(err);
    err = reader.Get(peerNodeId);
    SuccessOrExit(err);

    // Look for / create a session key entry for the given key id and peer node.
    err = FindSessionKey(keyId, peerNodeId, true, sessionKey);
    SuccessOrExit(err);
    if (!sessionKey->IsAllocated())
    {
        sessionKey->MsgEncKey.KeyId = keyId;
        sessionKey->NodeId          = peerNodeId;
        sessionKey->BoundCon        = NULL;
        sessionKey->ReserveCount    = 0;
        sessionKey->Flags           = 0;
    }
    else
    {
        // If the key id / peer node matches an existing session that is NOT suspended, fail with an error.
        VerifyOrExit(sessionKey->IsSuspended(), err = CHIP_ERROR_DUPLICATE_KEY_ID);
    }
    sessionKey->SetRemoveOnIdle(true);
    sessionKey->MarkRecentlyActive();

    // After this point, if an error occurs, remove the session key.
    removeSessionOnError = true;

    // If the key id / peer node matched a suspended session key, clear the suspended flag.
    sessionKey->ClearSuspended();

    // Clear any alternate end node ids associated with the session key.
    RemoveSharedSessionEndNodes(sessionKey);

    // Read the encoded session information in tag order and restore it into the session key entry.
    {
        uint32_t nextMsgId;
        err = reader.Next(kTLVType_UnsignedInteger, ContextTag(kTag_SerializedSession_NextMessageId));
        SuccessOrExit(err);
        err = reader.Get(nextMsgId);
        SuccessOrExit(err);
        err = sessionKey->NextMsgId.Init(nextMsgId);
        SuccessOrExit(err);
    }
    err = reader.Next(kTLVType_UnsignedInteger, ContextTag(kTag_SerializedSession_MaxRcvdMessageId));
    SuccessOrExit(err);
    err = reader.Get(sessionKey->MaxRcvdMsgId);
    SuccessOrExit(err);
    err = reader.Next(kTLVType_UnsignedInteger, ContextTag(kTag_SerializedSession_MessageRcvdFlags));
    SuccessOrExit(err);
    err = reader.Get(sessionKey->RcvFlags);
    SuccessOrExit(err);
    {
        bool b;
        err = reader.Next(kTLVType_Boolean, ContextTag(kTag_SerializedSession_IsLocallyInitiated));
        SuccessOrExit(err);
        err = reader.Get(b);
        SuccessOrExit(err);
        sessionKey->SetLocallyInitiated(b);
        err = reader.Next(kTLVType_Boolean, ContextTag(kTag_SerializedSession_IsShared));
        SuccessOrExit(err);
        err = reader.Get(b);
        SuccessOrExit(err);
        sessionKey->SetSharedSession(b);
    }

    // If the session is a shared session, restore the list of alternate end node ids.
    if (sessionKey->IsSharedSession())
    {
        TLVType container2;

        err = reader.Next(kTLVType_Array, ContextTag(kTag_SerializedSession_SharedSessionAltNodeIds));
        SuccessOrExit(err);
        err = reader.EnterContainer(container2);
        SuccessOrExit(err);

        while ((err = reader.Next(kTLVType_UnsignedInteger, AnonymousTag)) == CHIP_NO_ERROR)
        {
            uint64_t altNodeId;

            err = reader.Get(altNodeId);
            SuccessOrExit(err);

            err = AddSharedSessionEndNode(sessionKey, altNodeId);
            SuccessOrExit(err);
        }

        err = reader.ExitContainer(container2);
        SuccessOrExit(err);
    }

    // Read and restore the session AuthMode.
    {
        uint8_t certType;
        err = reader.Next(kTLVType_UnsignedInteger, ContextTag(kTag_SerializedSession_CASE_PeerCertType));
        SuccessOrExit(err);
        err = reader.Get(certType);
        SuccessOrExit(err);
        sessionKey->AuthMode = CASEAuthMode(certType);
    }

    // Restore the session message encryption type.
    err = reader.Next(kTLVType_UnsignedInteger, ContextTag(kTag_SerializedSession_EncryptionType));
    SuccessOrExit(err);
    err = reader.Get(sessionKey->MsgEncKey.EncType);
    SuccessOrExit(err);

    // Based on the encryption type, restore the associated keys.
    switch (sessionKey->MsgEncKey.EncType)
    {
    case kChipEncryptionType_AES128CTRSHA1:
        err = reader.Next(kTLVType_ByteString, ContextTag(kTag_SerializedSession_AES128CTRSHA1_DataKey));
        SuccessOrExit(err);
        VerifyOrExit(reader.GetLength() == ChipEncryptionKey_AES128CTRSHA1::DataKeySize, err = CHIP_ERROR_INVALID_ARGUMENT);
        err = reader.GetBytes(sessionKey->MsgEncKey.EncKey.AES128CTRSHA1.DataKey, ChipEncryptionKey_AES128CTRSHA1::DataKeySize);
        SuccessOrExit(err);
        err = reader.Next(kTLVType_ByteString, ContextTag(kTag_SerializedSession_AES128CTRSHA1_IntegrityKey));
        SuccessOrExit(err);
        VerifyOrExit(reader.GetLength() == ChipEncryptionKey_AES128CTRSHA1::IntegrityKeySize, err = CHIP_ERROR_INVALID_ARGUMENT);
        err = reader.GetBytes(sessionKey->MsgEncKey.EncKey.AES128CTRSHA1.IntegrityKey,
                              ChipEncryptionKey_AES128CTRSHA1::IntegrityKeySize);
        SuccessOrExit(err);
        break;
    default:
        ExitNow(err = CHIP_ERROR_UNSUPPORTED_ENCRYPTION_TYPE);
    }

    // Verify no other data in the serialized session structure.
    err = reader.VerifyEndOfContainer();
    SuccessOrExit(err);
    err = reader.ExitContainer(container);
    SuccessOrExit(err);

exit:
    if (removeSessionOnError && err != CHIP_NO_ERROR)
    {
        RemoveSessionKey(sessionKey, false);
    }
    return err;
}

CHIP_ERROR ChipFabricState::GetSessionState(uint64_t remoteNodeId, uint16_t keyId, uint8_t encType, ChipConnection * con,
                                            ChipSessionState & outSessionState)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    PeerIndexType peerIndex;

    switch (ChipKeyId::GetType(keyId))
    {
    case ChipKeyId::kType_None:
        if (keyId != ChipKeyId::kNone)
            return CHIP_ERROR_INVALID_KEY_ID;

        if (encType != kChipEncryptionType_None)
            return CHIP_ERROR_WRONG_ENCRYPTION_TYPE;

        if (con == NULL)
        {
            FindOrAllocPeerEntry(remoteNodeId, true, peerIndex);
            outSessionState = ChipSessionState(NULL, kChipAuthMode_Unauthenticated, &NextUnencUDPMsgId,
                                               &PeerStates.MaxUnencUDPMsgIdRcvd[peerIndex], &PeerStates.UnencRcvFlags[peerIndex]);
        }
        else
            outSessionState = ChipSessionState(NULL, kChipAuthMode_Unauthenticated, &NextUnencTCPMsgId, NULL, NULL);
        break;

    case ChipKeyId::kType_Session:
        ChipSessionKey * sessionKey;
        err = FindSessionKey(keyId, remoteNodeId, false, sessionKey);
        if (err != CHIP_NO_ERROR)
            return err;
        if (sessionKey->IsSuspended())
            return CHIP_ERROR_SESSION_KEY_SUSPENDED;
        if (sessionKey->MsgEncKey.EncType != encType)
            return (sessionKey->MsgEncKey.EncType == kChipEncryptionType_None) ? CHIP_ERROR_KEY_NOT_FOUND
                                                                               : CHIP_ERROR_WRONG_ENCRYPTION_TYPE;
        if (sessionKey->BoundCon != NULL && sessionKey->BoundCon != con)
            return CHIP_ERROR_INVALID_USE_OF_SESSION_KEY;
        outSessionState = ChipSessionState(&sessionKey->MsgEncKey, sessionKey->AuthMode, &sessionKey->NextMsgId,
                                           &sessionKey->MaxRcvdMsgId, &sessionKey->RcvFlags);
        break;

#if CHIP_CONFIG_USE_APP_GROUP_KEYS_FOR_MSG_ENC
    case ChipKeyId::kType_AppStaticKey:
    case ChipKeyId::kType_AppRotatingKey: {
        ChipMsgEncryptionKey * applicationKey;
        err = FindMsgEncAppKey(keyId, encType, applicationKey);
        SuccessOrExit(err);

        ChipAuthMode authMode = GroupKeyAuthMode(keyId);

        if (FindOrAllocPeerEntry(remoteNodeId, false, peerIndex))
            outSessionState =
                ChipSessionState(applicationKey, authMode, &NextGroupKeyMsgId, &PeerStates.MaxGroupKeyMsgIdRcvd[peerIndex],
                                 &PeerStates.GroupKeyRcvFlags[peerIndex]);
        else
            outSessionState = ChipSessionState(applicationKey, authMode, &NextGroupKeyMsgId, NULL, NULL);
        break;
    }
#endif

    default:
        ExitNow(err = CHIP_ERROR_UNKNOWN_KEY_TYPE);
    }

exit:
    return err;
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

#if CHIP_CONFIG_USE_APP_GROUP_KEYS_FOR_MSG_ENC
/**
 * This method verifies that information received in the message counter synchronization
 * message is valid (i.e. requestor message counter is fresh). On success, it initializes
 * group key entry in the peer state table.
 *
 * @param[in] peerNodeId          The node identifier of the peer.
 * @param[in] peerMsgId           Identifier of the received MsgCounterSync message.
 * @param[in] requestorMsgCounter Requestor message counter field from the message counter
 *                                synchronization message.
 *
 */
void ChipFabricState::OnMsgCounterSyncRespRcvd(uint64_t peerNodeId, uint32_t peerMsgId, uint32_t requestorMsgCounter)
{
    PeerIndexType peerIndex;

    // If requestor message counter is fresh.
    if (IsMsgCounterSyncReqInProgress() && (requestorMsgCounter >= GroupKeyMsgIdFreshWindowStart) &&
        (requestorMsgCounter < NextGroupKeyMsgId.GetValue()))
    {
        FindOrAllocPeerEntry(peerNodeId, true, peerIndex);

        // If not already synchronized.
        if ((PeerStates.GroupKeyRcvFlags[peerIndex] & ChipSessionState::kReceiveFlags_MessageIdSynchronized) == 0)
        {
            // Initialize group key entry in the peer state table.
            PeerStates.GroupKeyRcvFlags[peerIndex]     = ChipSessionState::kReceiveFlags_MessageIdSynchronized;
            PeerStates.MaxGroupKeyMsgIdRcvd[peerIndex] = peerMsgId;

#if CHIP_CONFIG_ENABLE_RELIABLE_MESSAGING
            // Clear MsgCounterSyncReq flag for all pending messages to that peer.
            MessageLayer->ExchangeMgr->ClearMsgCounterSyncReq(peerNodeId);
#endif
        }
    }

    return;
}

// Start message counter synchronization timer.
void ChipFabricState::StartMsgCounterSyncTimer(void)
{
    // Arm timer to call MsgCounterSyncRespTimeout after CHIP_CONFIG_MSG_COUNTER_SYNC_RESP_TIMEOUT.
    CHIP_ERROR res = MessageLayer->SystemLayer->StartTimer((uint32_t) CHIP_CONFIG_MSG_COUNTER_SYNC_RESP_TIMEOUT,
                                                           OnMsgCounterSyncRespTimeout, this);
    VerifyOrDie(res == CHIP_NO_ERROR);
}

/**
 * This method is called when message counter synchronization request is sent.
 *
 * @param[in] messageId         Identification of the message with which message
 *                              counter synchronization request is sent.
 *
 */
void ChipFabricState::OnMsgCounterSyncReqSent(uint32_t messageId)
{
    // Set ReqSentThisPeriod flag.
    MsgCounterSyncStatus |= kFlag_ReqSentThisPeriod;

    // If no message counter synchronization request in progress.
    if (!IsMsgCounterSyncReqInProgress())
    {
        // Set ReqInProgress flag.
        MsgCounterSyncStatus |= kFlag_ReqInProgress;

        // Set fresh window start field.
        GroupKeyMsgIdFreshWindowStart = messageId;

        // Arm timer.
        StartMsgCounterSyncTimer();

        // Enable fast-poll mode for SEDs if not already enabled.
        MessageLayer->SignalMessageLayerActivityChanged();
    }

    return;
}

// Handle MsgCounterSyncRespTimeout.
void ChipFabricState::OnMsgCounterSyncRespTimeout(System::Layer * aSystemLayer, void * aAppState, System::Error aError)
{
    ChipFabricState * fabricState = reinterpret_cast<ChipFabricState *>(aAppState);
    uint32_t freshWindoWidth;

    VerifyOrDie(fabricState != NULL && fabricState->MessageLayer->SystemLayer == aSystemLayer);

    // If message counter synchronization request was sent this period.
    if (fabricState->MsgCounterSyncStatus & fabricState->kFlag_ReqSentThisPeriod)
    {
        fabricState->GroupKeyMsgIdFreshWindowStart +=
            (fabricState->MsgCounterSyncStatus & fabricState->kMask_GroupKeyMsgIdFreshWindowWidth);

        freshWindoWidth = fabricState->NextGroupKeyMsgId.GetValue() - fabricState->GroupKeyMsgIdFreshWindowStart;

        // If fresh window exceeds highest supported width.
        if (freshWindoWidth > fabricState->kMask_GroupKeyMsgIdFreshWindowWidth)
        {
            // Adjust fresh window start value.
            fabricState->GroupKeyMsgIdFreshWindowStart += (freshWindoWidth - fabricState->kMask_GroupKeyMsgIdFreshWindowWidth);

            // Set the highest supported fresh window width.
            freshWindoWidth = fabricState->kMask_GroupKeyMsgIdFreshWindowWidth;
        }

        // Set fresh window width field.
        fabricState->MsgCounterSyncStatus |= (freshWindoWidth & fabricState->kMask_GroupKeyMsgIdFreshWindowWidth);

        // Clear ReqSentThisPeriod flag.
        fabricState->MsgCounterSyncStatus &= ~fabricState->kFlag_ReqSentThisPeriod;

        // Arm timer.
        fabricState->StartMsgCounterSyncTimer();
    }
    else
    {
        // Clear status fields.
        fabricState->MsgCounterSyncStatus = 0;

        // Disable fast-poll mode for SEDs if needed.
        fabricState->MessageLayer->SignalMessageLayerActivityChanged();
    }

    return;
}

/**
 * Get the key ID to be used to encrypt messages for a specific CHIP Application Group.
 *
 * @param[in]   appGroupGlobalId
 *                              The global ID of the application group for which the encryption
 *                              key ID should be returned.
 * @param[in]   rootKeyId       The root key used to derive encryption keys for the specified
 *                              CHIP Application Group.
 * @param[in]   useRotatingKey  True if the CHIP Application Group uses rotating message keys.
 * @param[out]  keyId           The key ID to be used to encrypt messages for the specified
 *                              CHIP Application Group.
 */
CHIP_ERROR ChipFabricState::GetMsgEncKeyIdForAppGroup(uint32_t appGroupGlobalId, uint32_t rootKeyId, bool useRotatingKey,
                                                      uint32_t & keyId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint32_t masterKeyId;

    // Lookup the master key id for the specified Application Group Global Id.
    err = GetAppGroupMasterKeyId(appGroupGlobalId, GroupKeyStore, masterKeyId);
    SuccessOrExit(err);

    // Form the appropriate message encryption key id.
    if (useRotatingKey)
    {
        keyId = ChipKeyId::MakeAppRotatingKeyId(rootKeyId, 0, masterKeyId, true);
    }
    else
    {
        keyId = ChipKeyId::MakeAppStaticKeyId(rootKeyId, masterKeyId);
    }

exit:
    return err;
}

/**
 * Ensure that a CHIP message was encrypted using the message encryption key for a specific
 * CHIP Application Group key.
 *
 * @param[in]   msgInfo         A pointer to a ChipMessageInfo structure containing information
 *                              about the received message.
 * @param[in]   appGroupGlobalId
 *                              The global ID of the application group for which the message is
 *                              expected to be encrypted.
 * @param[in]   rootKeyId       The root key used to derive encryption keys for the specified
 *                              CHIP Application Group.
 * @param[in]   requireRotatingKey
 *                              True if the CHIP Application Group uses rotating message keys.
 */
CHIP_ERROR ChipFabricState::CheckMsgEncForAppGroup(const ChipMessageInfo * msgInfo, uint32_t appGroupGlobalId, uint32_t rootKeyId,
                                                   bool requireRotatingKey)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint32_t expectedMasterKeyId;

    // Verify that the message was encrypted with a group key.
    VerifyOrExit(ChipKeyId::IsAppGroupKey(msgInfo->KeyId), err = CHIP_ERROR_WRONG_KEY_TYPE);

    // Verify that the message encryption key was derived from the specified root key.
    VerifyOrExit(ChipKeyId::GetRootKeyId(msgInfo->KeyId) == rootKeyId, err = CHIP_ERROR_WRONG_KEY_TYPE);

    // If requested, verify that the message was encrypted with a rotating key.
    VerifyOrExit(!requireRotatingKey || ChipKeyId::IsAppRotatingKey(msgInfo->KeyId), err = CHIP_ERROR_WRONG_KEY_TYPE);

    // Lookup the master key id for the specified Application Group.
    err = GetAppGroupMasterKeyId(appGroupGlobalId, GroupKeyStore, expectedMasterKeyId);
    SuccessOrExit(err);

    // Verify that the message was encrypted using the master key for the specified Application Group.
    VerifyOrExit(ChipKeyId::GetAppGroupMasterKeyId(msgInfo->KeyId) == expectedMasterKeyId, err = CHIP_ERROR_WRONG_KEY_TYPE);

exit:
    return err;
}

#endif // CHIP_CONFIG_USE_APP_GROUP_KEYS_FOR_MSG_ENC

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

#if CHIP_CONFIG_USE_APP_GROUP_KEYS_FOR_MSG_ENC
            // Try to find the least recently used peer entry that didn't use encryption.
            for (int j = CHIP_CONFIG_MAX_PEER_NODES - 1; j >= 0; j--)
            {
                PeerIndexType peerInd = PeerStates.MostRecentlyUsedIndexes[j];
                if ((PeerStates.GroupKeyRcvFlags[peerInd] & ChipSessionState::kReceiveFlags_MessageIdSynchronized) == 0)
                {
                    i = j;
                    break;
                }
            }
#endif

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
#if CHIP_CONFIG_USE_APP_GROUP_KEYS_FOR_MSG_ENC
        PeerStates.MaxGroupKeyMsgIdRcvd[retPeerIndex] = 0;
        PeerStates.GroupKeyRcvFlags[retPeerIndex]     = 0;
#endif
        PeerStates.UnencRcvFlags[retPeerIndex] = 0;
        retVal                                 = true;
    }

    // Move the requested entry to the top of the most recently used indexes list.
    if (retVal)
    {
        memmove(&PeerStates.MostRecentlyUsedIndexes[1], &PeerStates.MostRecentlyUsedIndexes[0], i * sizeof(PeerIndexType));
        PeerStates.MostRecentlyUsedIndexes[0] = retPeerIndex;
    }

    return retVal;
}

/*
 * This method is used by provisioning servers to register callbacks with the
 * ChipFabricState to be notified when the current session is closed.
 *
 * @param[in]  sessionEndCb       The context containing the callback function
 *                                pointer.
 *
 * @retval CHIP_ERROR            CHIP error encountered.
 *
 */
CHIP_ERROR ChipFabricState::RegisterSessionEndCallback(SessionEndCbCtxt * sessionEndCb)
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    SessionEndCbCtxt * iter = sessionEndCallbackList;

    VerifyOrExit(sessionEndCb, err = CHIP_ERROR_INVALID_ARGUMENT);

    sessionEndCb->next = NULL;
    if (sessionEndCallbackList == NULL)
    {
        sessionEndCallbackList = sessionEndCb;
        ExitNow();
    }

    while (iter->next)
    {
        iter = iter->next;
    }

    iter->next = sessionEndCb;

exit:
    return err;
}

/*
 * Notify the registered callbacks when the given session is closed and removed
 * from the session table.
 */
void ChipFabricState::NotifySessionEndSubscribers(uint16_t keyId, uint64_t peerNodeId)
{
    SessionEndCbCtxt * iter = sessionEndCallbackList;

    while (iter)
    {
        if (iter->OnSessionRemoved)
        {
            iter->OnSessionRemoved(keyId, peerNodeId, iter->context);
        }
        iter = iter->next;
    }
}

CHIP_ERROR ChipFabricState::GetPassword(uint8_t pwSrc, const char *& ps, uint16_t & pwLen)
{
    switch (pwSrc)
    {
    case kPasswordSource_PairingCode:
        if (PairingCode == NULL)
            return CHIP_ERROR_INVALID_ARGUMENT; // TODO: use proper error code
        ps    = PairingCode;
        pwLen = (uint16_t) strlen(PairingCode);
        return CHIP_NO_ERROR;
    default:
        return CHIP_ERROR_INVALID_ARGUMENT; // TODO: use proper error code
    }
}

CHIP_ERROR ChipFabricState::CreateFabric()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ChipGroupKey fabricSecret;

    // Fail if the node is already a member of a fabric.
    if (FabricId != 0)
        return CHIP_ERROR_INCORRECT_STATE;

    // Make sure the fabric state is cleared.
    ClearFabricState();

#if CHIP_CONFIG_SECURITY_TEST_MODE
    if (DebugFabricId == 0)
#endif
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
            err = chip::Platform::Security::GetSecureRandomData((unsigned char *) &FabricId, sizeof(FabricId));
            SuccessOrExit(err);
        } while (FabricId == kFabricIdNotSpecified || FabricId >= kReservedFabricIdStart);
    }
#if CHIP_CONFIG_SECURITY_TEST_MODE
    else
    {
        // Use our debug fabric ID.
        FabricId = DebugFabricId;
    }
#endif

    // Create an initial fabric secret.
    fabricSecret.KeyId  = ChipKeyId::kFabricSecret;
    fabricSecret.KeyLen = kChipFabricSecretSize;
    err                 = chip::Platform::Security::GetSecureRandomData(fabricSecret.Key, kChipFabricSecretSize);
    SuccessOrExit(err);

    err = GroupKeyStore->StoreGroupKey(fabricSecret);
    SuccessOrExit(err);

    if (Delegate != NULL)
        Delegate->DidJoinFabric(this, FabricId);

exit:
    if (err != CHIP_NO_ERROR)
        ClearFabricState();

    ClearSecretData((uint8_t *) &fabricSecret, sizeof(fabricSecret));

    return err;
}

void ChipFabricState::ClearFabricState()
{
    uint64_t oldFabricId;

    oldFabricId = FabricId;
    FabricId    = kFabricIdNotSpecified;
    GroupKeyStore->Clear();

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

    {
        TLVType containerType2;

        err = writer.StartContainer(ContextTag(kTag_FabricKeys), kTLVType_Array, containerType2);
        SuccessOrExit(err);

        {
            TLVType containerType3;
            ChipGroupKey fabricSecret;

            err = GroupKeyStore->RetrieveGroupKey(ChipKeyId::kFabricSecret, fabricSecret);
            SuccessOrExit(err);

            err = writer.StartContainer(AnonymousTag, kTLVType_Structure, containerType3);
            SuccessOrExit(err);

            err = writer.Put(ContextTag(kTag_FabricKeyId), (uint16_t)(fabricSecret.KeyId));
            SuccessOrExit(err);

            err = writer.Put(ContextTag(kTag_EncryptionType), (uint8_t) kChipEncryptionType_AES128CTRSHA1);
            SuccessOrExit(err);

            err = writer.PutBytes(ContextTag(kTag_DataKey), fabricSecret.Key, ChipEncryptionKey_AES128CTRSHA1::DataKeySize);
            SuccessOrExit(err);

            err = writer.PutBytes(ContextTag(kTag_IntegrityKey), fabricSecret.Key + ChipEncryptionKey_AES128CTRSHA1::DataKeySize,
                                  ChipEncryptionKey_AES128CTRSHA1::IntegrityKeySize);
            SuccessOrExit(err);

            err = writer.Put(ContextTag(kTag_KeyScope), (FabricSecretScope) kFabricSecretScope_All);
            SuccessOrExit(err);

            err = writer.Put(ContextTag(kTag_RotationScheme), (FabricSecretRotationScheme) kDeprecatedRotationScheme);
            SuccessOrExit(err);

            err = writer.EndContainer(containerType3);
            SuccessOrExit(err);
        }

        err = writer.EndContainer(containerType2);
        SuccessOrExit(err);
    }

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

        err = reader.Next(kTLVType_Array, ContextTag(kTag_FabricKeys));
        SuccessOrExit(err);

        {
            TLVType containerType2;

            err = reader.EnterContainer(containerType2);
            SuccessOrExit(err);

            err = reader.Next(kTLVType_Structure, AnonymousTag);
            SuccessOrExit(err);

            {
                TLVType containerType3;
                ChipGroupKey fabricSecret;
                uint16_t keyId;
                uint8_t encType;
                FabricSecretScope keyScope;
                FabricSecretRotationScheme rotationScheme;

                err = reader.EnterContainer(containerType3);
                SuccessOrExit(err);

                err = reader.Next(kTLVType_UnsignedInteger, ContextTag(kTag_FabricKeyId));
                SuccessOrExit(err);
                err = reader.Get(keyId);
                SuccessOrExit(err);
                VerifyOrExit(keyId == ChipKeyId::kFabricSecret, err = CHIP_ERROR_INVALID_KEY_ID);
                fabricSecret.KeyId = keyId;

                err = reader.Next(kTLVType_UnsignedInteger, ContextTag(kTag_EncryptionType));
                SuccessOrExit(err);
                err = reader.Get(encType);
                SuccessOrExit(err);
                VerifyOrExit(encType == kChipEncryptionType_AES128CTRSHA1, err = CHIP_ERROR_UNSUPPORTED_ENCRYPTION_TYPE);

                err = reader.Next(kTLVType_ByteString, ContextTag(kTag_DataKey));
                SuccessOrExit(err);
                VerifyOrExit(reader.GetLength() == ChipEncryptionKey_AES128CTRSHA1::DataKeySize, err = CHIP_ERROR_INVALID_ARGUMENT);
                err = reader.GetBytes(fabricSecret.Key, ChipEncryptionKey_AES128CTRSHA1::DataKeySize);
                SuccessOrExit(err);

                err = reader.Next(kTLVType_ByteString, ContextTag(kTag_IntegrityKey));
                SuccessOrExit(err);
                VerifyOrExit(reader.GetLength() == ChipEncryptionKey_AES128CTRSHA1::IntegrityKeySize,
                             err = CHIP_ERROR_INVALID_ARGUMENT);
                err = reader.GetBytes(fabricSecret.Key + ChipEncryptionKey_AES128CTRSHA1::DataKeySize,
                                      ChipEncryptionKey_AES128CTRSHA1::IntegrityKeySize);
                SuccessOrExit(err);

                fabricSecret.KeyLen = kChipFabricSecretSize;

                err = reader.Next(kTLVType_UnsignedInteger, ContextTag(kTag_KeyScope));
                SuccessOrExit(err);
                err = reader.Get(keyScope);
                SuccessOrExit(err);
                VerifyOrExit(keyScope == kFabricSecretScope_All, err = CHIP_ERROR_INVALID_ARGUMENT);

                err = reader.Next(kTLVType_UnsignedInteger, ContextTag(kTag_RotationScheme));
                SuccessOrExit(err);
                err = reader.Get(rotationScheme);
                SuccessOrExit(err);
                VerifyOrExit(rotationScheme == kDeprecatedRotationScheme, err = CHIP_ERROR_INVALID_ARGUMENT);

                err = reader.ExitContainer(containerType3);
                SuccessOrExit(err);

                err = GroupKeyStore->StoreGroupKey(fabricSecret);
                SuccessOrExit(err);
            }

            err = reader.Next(kTLVType_Structure, AnonymousTag);
            VerifyOrExit(err == CHIP_END_OF_TLV, /* no action */);

            err = reader.ExitContainer(containerType2);
            SuccessOrExit(err);
        }
    }

    if (Delegate != NULL)
        Delegate->DidJoinFabric(this, FabricId);

exit:
    if (err != CHIP_NO_ERROR)
        ClearFabricState();

    return err;
}

void ChipFabricState::HandleConnectionClosed(ChipConnection * con)
{
    ChipSessionKey * sessionKey;

    // Remove any session keys that are bound to the closed connection.
    sessionKey = SessionKeys;
    for (int i = 0; i < CHIP_CONFIG_MAX_SESSION_KEYS; i++, sessionKey++)
    {
        if (sessionKey->IsAllocated() && SessionKeys[i].BoundCon == con)
        {
            RemoveSessionKey(sessionKey);
        }
    }
}

// ChipSessionState Members

ChipSessionState::ChipSessionState(void)
{
    MsgEncKey    = NULL;
    AuthMode     = kChipAuthMode_NotSpecified;
    NextMsgId    = NULL;
    MaxMsgIdRcvd = NULL;
    RcvFlags     = NULL;
}

ChipSessionState::ChipSessionState(ChipMsgEncryptionKey * msgEncKey, ChipAuthMode authMode,
                                   MonotonicallyIncreasingCounter * nextMsgId, uint32_t * maxMsgIdRcvd, ReceiveFlagsType * rcvFlags)
{
    MsgEncKey    = msgEncKey;
    AuthMode     = authMode;
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
        if (MsgEncKey == NULL && RcvFlags == NULL)
        {
            ExitNow();
        }
#if CHIP_CONFIG_USE_APP_GROUP_KEYS_FOR_MSG_ENC
        // Mark message as a duplicate and exit if it was encrypted with application group key.
        // In this case, peer's message counter synchronization can only be done by
        // ChipSecurityManager::HandleMsgCounterSyncRespMsg() function.
        else if (MsgEncKey != NULL && ChipKeyId::IsAppGroupKey(MsgEncKey->KeyId))
        {
            ExitNow(isDup = true);
        }
#endif
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
            // If the message was encrypted then assume the message is a duplicate.
            if (MsgEncKey != NULL)
            {
                ExitNow(isDup = true);
            }

            // Otherwise the message is not encrypted, so assume the message is valid and reset the received state.
            //
            // Senders of unencrypted messages are not required to preserve message id ordering across restarts.
            // Since duplicate message detection for unencrypted messages is only to eliminate duplicates created
            // in the network layer, we allow message ids for unencrypted messages from the same peer to go backwards.
            else
            {
                msgIdFlags    = 0;
                *MaxMsgIdRcvd = msgId;
            }
        }
    }

    // Update the message id flags within the receive flags value and set the MessageIdIsSynchronized flag.
    *RcvFlags = kReceiveFlags_MessageIdSynchronized | msgIdFlags | (*RcvFlags & ~kReceiveFlags_MessageIdFlagsMask);

exit:
    return isDup;
}

/**
 * This method finds session key entry.
 *
 * @param[in] keyId               CHIP key identifier.
 * @param[in] peerNodeId          The node identifier of the peer.
 * @param[in] create              A boolean value indicating whether new key should be created
 *                                if the specified key is not found.
 * @param[out] retRec             A pointer reference to a ChipSessionKey object.
 *
 * @retval #CHIP_ERROR_WRONG_KEY_TYPE     If specified key is not a session key type.
 * @retval #CHIP_ERROR_INVALID_ARGUMENT   If input arguments have wrong values.
 * @retval #CHIP_ERROR_KEY_NOT_FOUND      If specified key is not found.
 * @retval #CHIP_ERROR_TOO_MANY_KEYS      If there is no free entry to create new session key.
 * @retval #CHIP_NO_ERROR                 On success.
 *
 */
CHIP_ERROR ChipFabricState::FindSessionKey(uint16_t keyId, uint64_t peerNodeId, bool create, ChipSessionKey *& retRec)
{
    ChipSessionKey * curRec  = SessionKeys;
    ChipSessionKey * freeRec = NULL;

    if (!ChipKeyId::IsSessionKey(keyId))
        return CHIP_ERROR_WRONG_KEY_TYPE;

    if (peerNodeId == kNodeIdNotSpecified || peerNodeId == kAnyNodeId)
        return CHIP_ERROR_INVALID_ARGUMENT;

    for (int i = 0; i < CHIP_CONFIG_MAX_SESSION_KEYS; i++, curRec++)
    {
        if (!curRec->IsAllocated())
        {
            if (freeRec == NULL)
                freeRec = curRec;
        }
        else if (curRec->MsgEncKey.KeyId == keyId &&
                 (curRec->NodeId == peerNodeId || (curRec->IsSharedSession() && FindSharedSessionEndNode(peerNodeId, curRec))))
        {
            retRec = curRec;
            return CHIP_NO_ERROR;
        }
    }

    if (!create)
        return CHIP_ERROR_KEY_NOT_FOUND;

    if (freeRec == NULL)
        return CHIP_ERROR_TOO_MANY_KEYS;

    retRec = freeRec;

    return CHIP_NO_ERROR;
}

#if CHIP_CONFIG_USE_APP_GROUP_KEYS_FOR_MSG_ENC
CHIP_ERROR ChipFabricState::FindMsgEncAppKey(uint16_t keyId, uint8_t encType, ChipMsgEncryptionKey *& retRec)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Find key or allocate empty key entry in the key cache.
    retRec = AppKeyCache.FindOrAllocateKeyEntry(keyId, encType);

    // Derive application key if it's not in the key cache.
    if (retRec->KeyId == ChipKeyId::kNone)
    {
        uint32_t appGroupGlobalId;

        err = DeriveMsgEncAppKey(keyId, encType, *retRec, appGroupGlobalId);
        SuccessOrExit(err);

#if CHIP_CONFIG_SECURITY_TEST_MODE && CHIP_DETAIL_LOGGING
        if (LogKeys)
        {
            char keyString[kMaxEncKeyStringSize];
            ChipEncryptionKeyToString(encType, retRec->EncKey, keyString, sizeof(keyString));
            ChipLogDetail(MessageLayer,
                          "Message Encryption Key: Id=%04" PRIX16 " Type=GroupKey(%08" PRIX32 ") EncType=%02" PRIX8 " Key=%s",
                          keyId, appGroupGlobalId, encType, keyString);
        }
#endif
    }

exit:
    return err;
}

/**
 * Derives message encryption application key.
 * Three types of message encryption application keys can be requested: current application
 * key, rotating application key, and static application key. When current application key
 * is requested, the function finds and uses the current epoch key based on the current system
 * time and the start time parameter of each epoch key.
 *
 * @param[in]    keyId              The requested key ID.
 * @param[in]    encType            The type of the requested message encryption key.
 * @param[out]   appKey             A reference to the message encryption key object.
 * @param[out]   appGroupGlobalId   The application group global ID of the associated key.
 *
 * @retval #CHIP_NO_ERROR          On success.
 * @retval #CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE
 *                                  If group key store functionality is not implemented.
 * @retval #CHIP_ERROR_UNSUPPORTED_ENCRYPTION_TYPE
 *                                  If the requested encryption type is not supported.
 * @retval #CHIP_ERROR_INVALID_KEY_ID
 *                                  If the requested key has an invalid key ID.
 * @retval #CHIP_ERROR_INVALID_ARGUMENT
 *                                  If the platform key store returns invalid key parameters.
 * @retval other                    Other platform-specific errors returned by the platform
 *                                  key store APIs.
 *
 */
CHIP_ERROR ChipFabricState::DeriveMsgEncAppKey(uint32_t keyId, uint8_t encType, ChipMsgEncryptionKey & appKey,
                                               uint32_t & appGroupGlobalId)
{
    CHIP_ERROR err;
    uint8_t keyData[ChipEncryptionKey_AES128CTRSHA1::KeySize];
    uint8_t keyDiversifier[kChipMsgEncAppKeyDiversifierSize];

    // Verify supported key type.
    VerifyOrExit(encType == kChipEncryptionType_AES128CTRSHA1, err = CHIP_ERROR_UNSUPPORTED_ENCRYPTION_TYPE);

    // Set application key size and info value.
    memcpy(keyDiversifier, kChipMsgEncAppKeyDiversifier, sizeof(kChipMsgEncAppKeyDiversifier));
    keyDiversifier[sizeof(kChipMsgEncAppKeyDiversifier)] = encType;

    // Derive application key data.
    err = GroupKeyStore->DeriveApplicationKey(keyId, NULL, 0, keyDiversifier, kChipMsgEncAppKeyDiversifierSize, keyData,
                                              sizeof(keyData), ChipEncryptionKey_AES128CTRSHA1::KeySize, appGroupGlobalId);
    SuccessOrExit(err);

    // Copy the generated key data to the appropriate destinations.
    memcpy(appKey.EncKey.AES128CTRSHA1.DataKey, keyData, ChipEncryptionKey_AES128CTRSHA1::DataKeySize);
    memcpy(appKey.EncKey.AES128CTRSHA1.IntegrityKey, keyData + ChipEncryptionKey_AES128CTRSHA1::DataKeySize,
           ChipEncryptionKey_AES128CTRSHA1::IntegrityKeySize);

    // Set key parameters.
    appKey.KeyId   = keyId;
    appKey.EncType = encType;

exit:
    ClearSecretData(keyData, sizeof(keyData));

    return err;
}
#endif // CHIP_CONFIG_USE_APP_GROUP_KEYS_FOR_MSG_ENC

void ChipFabricState::SetDelegate(FabricStateDelegate * aDelegate)
{
    Delegate = aDelegate;
}

bool ChipFabricState::RemoveIdleSessionKeys()
{
    ChipSessionKey * sessionKey;
    bool potentialIdleSessionsExist = false;

    // For each allocated session key...
    sessionKey = SessionKeys;
    for (int i = 0; i < CHIP_CONFIG_MAX_SESSION_KEYS; i++, sessionKey++)
        if (sessionKey->IsAllocated())
        {
            // Ignore the session if it is still in the process of being established.
            if (!sessionKey->IsKeySet())
                continue;

            // Capture and clear the recently active flag.
            bool recentlyActive = sessionKey->IsRecentlyActive();
            sessionKey->ClearRecentlyActive();

            // Ignore the session if it is bound to a connection. (Connection bound
            // sessions persist until their connections close).
            if (sessionKey->BoundCon != NULL)
                continue;

            // If the session is marked for remove-on-idle and is not currently reserved...
            if (sessionKey->IsRemoveOnIdle() && sessionKey->ReserveCount == 0)
            {
                // Remove the session if it hasn't been active since the last time RemoveIdleSessionKeys()
                // was called.
                if (!recentlyActive)
                {
                    RemoveSessionKey(sessionKey, true);
                }

                // Otherwise, tell the caller that unreserved, remove-on-idle sessions exist which may
                // need to be removed on a future call to RemoveIdleSessionKeys().
                else
                {
                    potentialIdleSessionsExist = true;
                }
            }
        }

    return potentialIdleSessionsExist;
}

// ============================================================
// CHIP Message Encryption Application Key Cache.
// ============================================================

void ChipMsgEncryptionKeyCache::Init()
{
    Reset();
}

void ChipMsgEncryptionKeyCache::Shutdown()
{
    Reset();
}

void ChipMsgEncryptionKeyCache::Reset()
{
    for (uint8_t keyEntry = 0; keyEntry < CHIP_CONFIG_MAX_CACHED_MSG_ENC_APP_KEYS; keyEntry++)
        Clear(keyEntry);
    memset(mMostRecentlyUsedKeyEntries, 0, sizeof(mMostRecentlyUsedKeyEntries));
}

// Clear key cache entry.
void ChipMsgEncryptionKeyCache::Clear(uint8_t keyEntryIndex)
{
    ClearSecretData((uint8_t *) (&mKeyCache[keyEntryIndex]), sizeof(ChipMsgEncryptionKey));
    mKeyCache[keyEntryIndex].KeyId   = ChipKeyId::kNone;
    mKeyCache[keyEntryIndex].EncType = kChipEncryptionType_None;
}

// If the key is found in the cache then function returns pointer to the key.
// If the key is not found in the cache then function allocates and returns pointer to the empty key entry in the cache.
ChipMsgEncryptionKey * ChipMsgEncryptionKeyCache::FindOrAllocateKeyEntry(uint16_t keyId, uint8_t encType)
{
    ChipMsgEncryptionKey * keyEntry;
    uint8_t retKeyEntryIndex = CHIP_CONFIG_MAX_CACHED_MSG_ENC_APP_KEYS;
    uint8_t i;

    // Find if key is in the cache.
    for (i = 0, keyEntry = mKeyCache; i < CHIP_CONFIG_MAX_CACHED_MSG_ENC_APP_KEYS; i++, keyEntry++)
    {
        if (keyEntry->KeyId == keyId && keyEntry->EncType == encType)
        {
            retKeyEntryIndex = i;
            break;
        }
        else if (retKeyEntryIndex == CHIP_CONFIG_MAX_CACHED_MSG_ENC_APP_KEYS && keyEntry->KeyId == ChipKeyId::kNone)
        {
            retKeyEntryIndex = i;
        }
    }

    // If cache is full and specified key was not found in the cache then replace the least-recently used key entry.
    if (retKeyEntryIndex == CHIP_CONFIG_MAX_CACHED_MSG_ENC_APP_KEYS)
    {
        // Chose the least-recently used entry in the key cache.
        retKeyEntryIndex = mMostRecentlyUsedKeyEntries[CHIP_CONFIG_MAX_CACHED_MSG_ENC_APP_KEYS - 1];

        // Clear replaced key cache entry.
        Clear(retKeyEntryIndex);
    }

    // Find key entry index in the most-recently used list of entries.
    for (i = 0; i < CHIP_CONFIG_MAX_CACHED_MSG_ENC_APP_KEYS; i++)
        if (mMostRecentlyUsedKeyEntries[i] == retKeyEntryIndex)
            break;

    // Mark selected key entry as most-recently used by moving it to the top of the most-recently used key entries list.
    memmove(&mMostRecentlyUsedKeyEntries[1], &mMostRecentlyUsedKeyEntries[0], i * sizeof(uint8_t));
    mMostRecentlyUsedKeyEntries[0] = retKeyEntryIndex;

    return &mKeyCache[retKeyEntryIndex];
}

#if CHIP_CONFIG_SECURITY_TEST_MODE

static inline char ToHex(const uint8_t data)
{
    return (data < 10) ? '0' + data : 'A' + (data - 10);
}

static void ToHexString(const uint8_t * data, size_t dataLen, char *& outBuf, size_t & outBufSize)
{
    for (; dataLen > 0 && outBufSize >= 2; data++, dataLen--, outBuf += 2, outBufSize -= 2)
    {
        outBuf[0] = ToHex(*data >> 4);
        outBuf[1] = ToHex(*data & 0xF);
    }
}

void ChipEncryptionKeyToString(uint8_t encType, const ChipEncryptionKey & key, char * buf, size_t bufSize)
{
    if (encType == kChipEncryptionType_AES128CTRSHA1)
    {
        bufSize -= 2; // Reserve size for the comma and null terminator.
        ToHexString(key.AES128CTRSHA1.DataKey, sizeof(key.AES128CTRSHA1.DataKey), buf, bufSize);
        *buf++ = ',';
        ToHexString(key.AES128CTRSHA1.IntegrityKey, sizeof(key.AES128CTRSHA1.IntegrityKey), buf, bufSize);
    }

    *buf = 0;
}

#endif // CHIP_CONFIG_SECURITY_TEST_MODE

} // namespace chip
