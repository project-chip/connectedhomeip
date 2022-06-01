/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 *      This file implements the CHIP Connection object that maintains a UDP connection.
 *      TODO This class should be extended to support TCP as well...
 *
 */

#include "SessionManager.h"

#include <inttypes.h>
#include <string.h>

#include "transport/TraceMessage.h"
#include <app/util/basic-types.h>
#include <credentials/GroupDataProvider.h>
#include <lib/core/CHIPKeyIds.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/secure_channel/Constants.h>
#include <transport/GroupPeerMessageCounter.h>
#include <transport/GroupSession.h>
#include <transport/SecureMessageCodec.h>
#include <transport/TransportMgr.h>

#include <inttypes.h>

// Global object
chip::Transport::GroupPeerTable mGroupPeerMsgCounter;

namespace chip {

using System::PacketBufferHandle;
using Transport::PeerAddress;
using Transport::SecureSession;

uint32_t EncryptedPacketBufferHandle::GetMessageCounter() const
{
    PacketHeader header;
    uint16_t headerSize = 0;
    CHIP_ERROR err      = header.Decode((*this)->Start(), (*this)->DataLength(), &headerSize);

    if (err == CHIP_NO_ERROR)
    {
        return header.GetMessageCounter();
    }

    ChipLogError(Inet, "Failed to decode EncryptedPacketBufferHandle header with error: %s", ErrorStr(err));

    return 0;
}

SessionManager::SessionManager() : mState(State::kNotReady) {}

SessionManager::~SessionManager() {}

CHIP_ERROR SessionManager::Init(System::Layer * systemLayer, TransportMgrBase * transportMgr,
                                Transport::MessageCounterManagerInterface * messageCounterManager,
                                chip::PersistentStorageDelegate * storageDelegate, FabricTable * fabricTable)
{
    VerifyOrReturnError(mState == State::kNotReady, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(transportMgr != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(storageDelegate != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(fabricTable != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    mState                 = State::kInitialized;
    mSystemLayer           = systemLayer;
    mTransportMgr          = transportMgr;
    mMessageCounterManager = messageCounterManager;
    mFabricTable           = fabricTable;

    mSecureSessions.Init();

    mGlobalUnencryptedMessageCounter.Init();

    ReturnErrorOnFailure(mGroupClientCounter.Init(storageDelegate));

    mTransportMgr->SetSessionManager(this);

    return CHIP_NO_ERROR;
}

void SessionManager::Shutdown()
{
    mMessageCounterManager = nullptr;

    mState        = State::kNotReady;
    mSystemLayer  = nullptr;
    mTransportMgr = nullptr;
    mCB           = nullptr;
}

/**
 * @brief Notification that a fabric was removed.
 *        This function doesn't call ExpireAllPairingsForFabric
 *        since the CASE session might still be open to send a response
 *        on the removed fabric.
 */
void SessionManager::FabricRemoved(FabricIndex fabricIndex)
{
    mGroupPeerMsgCounter.FabricRemoved(fabricIndex);
}

CHIP_ERROR SessionManager::PrepareMessage(const SessionHandle & sessionHandle, PayloadHeader & payloadHeader,
                                          System::PacketBufferHandle && message, EncryptedPacketBufferHandle & preparedMessage)
{
    PacketHeader packetHeader;
    bool isControlMsg = IsControlMessage(payloadHeader);
    if (isControlMsg)
    {
        packetHeader.SetSecureSessionControlMsg(true);
    }

#if CHIP_PROGRESS_LOGGING
    NodeId destination;
    FabricIndex fabricIndex;
#endif // CHIP_PROGRESS_LOGGING

    switch (sessionHandle->GetSessionType())
    {
    case Transport::Session::SessionType::kGroupOutgoing: {
        auto groupSession = sessionHandle->AsOutgoingGroupSession();
        auto * groups     = Credentials::GetGroupDataProvider();
        VerifyOrReturnError(nullptr != groups, CHIP_ERROR_INTERNAL);

        FabricInfo * fabric = mFabricTable->FindFabricWithIndex(groupSession->GetFabricIndex());
        VerifyOrReturnError(fabric != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

        packetHeader.SetDestinationGroupId(groupSession->GetGroupId());
        packetHeader.SetMessageCounter(mGroupClientCounter.GetCounter(isControlMsg));
        mGroupClientCounter.IncrementCounter(isControlMsg);
        packetHeader.SetFlags(Header::SecFlagValues::kPrivacyFlag);
        packetHeader.SetSessionType(Header::SessionType::kGroupSession);
        NodeId sourceNodeId = fabric->GetNodeId();
        packetHeader.SetSourceNodeId(sourceNodeId);

        if (!packetHeader.IsValidGroupMsg())
        {
            return CHIP_ERROR_INTERNAL;
        }

        // Trace before any encryption
        CHIP_TRACE_MESSAGE_SENT(payloadHeader, packetHeader, message->Start(), message->TotalLength());

        Crypto::SymmetricKeyContext * keyContext =
            groups->GetKeyContext(groupSession->GetFabricIndex(), groupSession->GetGroupId());
        VerifyOrReturnError(nullptr != keyContext, CHIP_ERROR_INTERNAL);

        packetHeader.SetSessionId(keyContext->GetKeyHash());
        CryptoContext::NonceStorage nonce;
        CryptoContext::BuildNonce(nonce, packetHeader.GetSecurityFlags(), packetHeader.GetMessageCounter(), sourceNodeId);
        CHIP_ERROR err = SecureMessageCodec::Encrypt(CryptoContext(keyContext), nonce, payloadHeader, packetHeader, message);
        keyContext->Release();
        ReturnErrorOnFailure(err);

#if CHIP_PROGRESS_LOGGING
        destination = kUndefinedNodeId;
        fabricIndex = groupSession->GetFabricIndex();
#endif // CHIP_PROGRESS_LOGGING
    }
    break;
    case Transport::Session::SessionType::kSecure: {
        SecureSession * session = sessionHandle->AsSecureSession();
        if (session == nullptr)
        {
            return CHIP_ERROR_NOT_CONNECTED;
        }

        MessageCounter & counter = session->GetSessionMessageCounter().GetLocalMessageCounter();
        uint32_t messageCounter  = counter.Value();
        packetHeader
            .SetMessageCounter(messageCounter)         //
            .SetSessionId(session->GetPeerSessionId()) //
            .SetSessionType(Header::SessionType::kUnicastSession);

        // Trace before any encryption
        CHIP_TRACE_MESSAGE_SENT(payloadHeader, packetHeader, message->Start(), message->TotalLength());

        CryptoContext::NonceStorage nonce;
        NodeId sourceNodeId = session->GetLocalScopedNodeId().GetNodeId();
        CryptoContext::BuildNonce(nonce, packetHeader.GetSecurityFlags(), messageCounter, sourceNodeId);

        ReturnErrorOnFailure(SecureMessageCodec::Encrypt(session->GetCryptoContext(), nonce, payloadHeader, packetHeader, message));
        ReturnErrorOnFailure(counter.Advance());

#if CHIP_PROGRESS_LOGGING
        destination = session->GetPeerNodeId();
        fabricIndex = session->GetFabricIndex();
#endif // CHIP_PROGRESS_LOGGING
    }
    break;
    case Transport::Session::SessionType::kUnauthenticated: {
        MessageCounter & counter = mGlobalUnencryptedMessageCounter;
        uint32_t messageCounter  = counter.Value();
        ReturnErrorOnFailure(counter.Advance());
        packetHeader.SetMessageCounter(messageCounter);
        Transport::UnauthenticatedSession * session = sessionHandle->AsUnauthenticatedSession();
        switch (session->GetSessionRole())
        {
        case Transport::UnauthenticatedSession::SessionRole::kInitiator:
            packetHeader.SetSourceNodeId(session->GetEphemeralInitiatorNodeID());
            break;
        case Transport::UnauthenticatedSession::SessionRole::kResponder:
            packetHeader.SetDestinationNodeId(session->GetEphemeralInitiatorNodeID());
            break;
        }

        // Trace after all headers are settled.
        CHIP_TRACE_MESSAGE_SENT(payloadHeader, packetHeader, message->Start(), message->TotalLength());

        ReturnErrorOnFailure(payloadHeader.EncodeBeforeData(message));

#if CHIP_PROGRESS_LOGGING
        destination = kUndefinedNodeId;
        fabricIndex = kUndefinedFabricIndex;
#endif // CHIP_PROGRESS_LOGGING
    }
    break;
    default:
        return CHIP_ERROR_INTERNAL;
    }

    ChipLogProgress(Inet,
                    "Prepared %s message %p to 0x" ChipLogFormatX64 " (%u)  of type " ChipLogFormatMessageType
                    " and protocolId " ChipLogFormatProtocolId " on exchange " ChipLogFormatExchangeId
                    " with MessageCounter:" ChipLogFormatMessageCounter ".",
                    sessionHandle->GetSessionTypeString(), &preparedMessage, ChipLogValueX64(destination), fabricIndex,
                    payloadHeader.GetMessageType(), ChipLogValueProtocolId(payloadHeader.GetProtocolID()),
                    ChipLogValueExchangeIdFromSentHeader(payloadHeader), packetHeader.GetMessageCounter());

    ReturnErrorOnFailure(packetHeader.EncodeBeforeData(message));
    preparedMessage = EncryptedPacketBufferHandle::MarkEncrypted(std::move(message));

    return CHIP_NO_ERROR;
}

CHIP_ERROR SessionManager::SendPreparedMessage(const SessionHandle & sessionHandle,
                                               const EncryptedPacketBufferHandle & preparedMessage)
{
    VerifyOrReturnError(mState == State::kInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(!preparedMessage.IsNull(), CHIP_ERROR_INVALID_ARGUMENT);

    Transport::PeerAddress multicastAddress; // Only used for the group case
    const Transport::PeerAddress * destination;

    switch (sessionHandle->GetSessionType())
    {
    case Transport::Session::SessionType::kGroupOutgoing: {
        auto groupSession = sessionHandle->AsOutgoingGroupSession();

        FabricInfo * fabric = mFabricTable->FindFabricWithIndex(groupSession->GetFabricIndex());
        VerifyOrReturnError(fabric != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

        multicastAddress = Transport::PeerAddress::Multicast(fabric->GetFabricId(), groupSession->GetGroupId());
        destination      = &multicastAddress;
        char addressStr[Transport::PeerAddress::kMaxToStringSize];
        multicastAddress.ToString(addressStr, Transport::PeerAddress::kMaxToStringSize);

        ChipLogProgress(Inet,
                        "Sending %s msg %p with MessageCounter:" ChipLogFormatMessageCounter " to %d"
                        " at monotonic time: " ChipLogFormatX64
                        " msec to Multicast IPV6 address : %s with GroupID of %d and fabric index of %x",
                        "encrypted group", &preparedMessage, preparedMessage.GetMessageCounter(), groupSession->GetGroupId(),
                        ChipLogValueX64(System::SystemClock().GetMonotonicMilliseconds64().count()), addressStr,
                        groupSession->GetGroupId(), groupSession->GetFabricIndex());
    }
    break;
    case Transport::Session::SessionType::kSecure: {
        // Find an active connection to the specified peer node
        SecureSession * secure = sessionHandle->AsSecureSession();

        // This marks any connection where we send data to as 'active'
        secure->MarkActive();

        destination = &secure->GetPeerAddress();

        ChipLogProgress(Inet,
                        "Sending %s msg %p with MessageCounter:" ChipLogFormatMessageCounter " to 0x" ChipLogFormatX64
                        " (%u) at monotonic time: " ChipLogFormatX64 " msec",
                        "encrypted", &preparedMessage, preparedMessage.GetMessageCounter(),
                        ChipLogValueX64(secure->GetPeerNodeId()), secure->GetFabricIndex(),
                        ChipLogValueX64(System::SystemClock().GetMonotonicMilliseconds64().count()));
    }
    break;
    case Transport::Session::SessionType::kUnauthenticated: {
        auto unauthenticated = sessionHandle->AsUnauthenticatedSession();
        unauthenticated->MarkActive();
        destination = &unauthenticated->GetPeerAddress();

        ChipLogProgress(Inet,
                        "Sending %s msg %p with MessageCounter:" ChipLogFormatMessageCounter " to 0x" ChipLogFormatX64
                        " at monotonic time: " ChipLogFormatX64 " msec",
                        sessionHandle->GetSessionTypeString(), &preparedMessage, preparedMessage.GetMessageCounter(),
                        ChipLogValueX64(kUndefinedNodeId),
                        ChipLogValueX64(System::SystemClock().GetMonotonicMilliseconds64().count()));
    }
    break;
    default:
        return CHIP_ERROR_INTERNAL;
    }

    PacketBufferHandle msgBuf = preparedMessage.CastToWritable();
    VerifyOrReturnError(!msgBuf.IsNull(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(!msgBuf->HasChainedBuffer(), CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    if (mTransportMgr != nullptr)
    {
        CHIP_TRACE_PREPARED_MESSAGE_SENT(destination, &msgBuf);
        return mTransportMgr->SendMessage(*destination, std::move(msgBuf));
    }

    ChipLogError(Inet, "The transport manager is not initialized. Unable to send the message");
    return CHIP_ERROR_INCORRECT_STATE;
}

void SessionManager::ExpirePairing(const SessionHandle & sessionHandle)
{
    sessionHandle->AsSecureSession()->MarkForRemoval();
}

void SessionManager::ExpireAllPairings(const ScopedNodeId & node)
{
    mSecureSessions.ForEachSession([&](auto session) {
        if (session->GetPeer() == node)
        {
            session->MarkForRemoval();
        }
        return Loop::Continue;
    });
}

void SessionManager::ExpireAllPairingsForPeerExceptPending(const ScopedNodeId & node)
{
    mSecureSessions.ForEachSession([&](auto session) {
        if ((session->GetPeer() == node) && session->IsActiveSession() &&
            (session->GetSecureSessionType() == SecureSession::Type::kCASE))
        {
            ChipLogDetail(Inet, "Expired/released previous local session ID %u for peer " ChipLogFormatScopedNodeId,
                          static_cast<unsigned>(session->GetLocalSessionId()), ChipLogValueScopedNodeId(session->GetPeer()));
            session->MarkForRemoval();
        }
        return Loop::Continue;
    });
}

void SessionManager::ExpireAllPairingsForFabric(FabricIndex fabric)
{
    ChipLogDetail(Inet, "Expiring all connections for fabric %d!!", fabric);
    mSecureSessions.ForEachSession([&](auto session) {
        if (session->GetFabricIndex() == fabric)
        {
            session->MarkForRemoval();
        }
        return Loop::Continue;
    });
}

void SessionManager::ExpireAllPASEPairings()
{
    ChipLogDetail(Inet, "Expiring all PASE pairings");
    mSecureSessions.ForEachSession([&](auto session) {
        if (session->GetSecureSessionType() == Transport::SecureSession::Type::kPASE)
        {
            session->MarkForRemoval();
        }
        return Loop::Continue;
    });
}

Optional<SessionHandle> SessionManager::AllocateSession(SecureSession::Type secureSessionType)
{
    return mSecureSessions.CreateNewSecureSession(secureSessionType);
}

CHIP_ERROR SessionManager::InjectPaseSessionWithTestKey(SessionHolder & sessionHolder, uint16_t localSessionId, NodeId peerNodeId,
                                                        uint16_t peerSessionId, FabricIndex fabric,
                                                        const Transport::PeerAddress & peerAddress, CryptoContext::SessionRole role)
{
    NodeId localNodeId = kUndefinedNodeId;
    Optional<SessionHandle> session =
        mSecureSessions.CreateNewSecureSessionForTest(chip::Transport::SecureSession::Type::kPASE, localSessionId, localNodeId,
                                                      peerNodeId, CATValues{}, peerSessionId, fabric, GetLocalMRPConfig());
    VerifyOrReturnError(session.HasValue(), CHIP_ERROR_NO_MEMORY);
    SecureSession * secureSession = session.Value()->AsSecureSession();
    secureSession->SetPeerAddress(peerAddress);

    size_t secretLen = strlen(CHIP_CONFIG_TEST_SHARED_SECRET_VALUE);
    ByteSpan secret(reinterpret_cast<const uint8_t *>(CHIP_CONFIG_TEST_SHARED_SECRET_VALUE), secretLen);
    ReturnErrorOnFailure(secureSession->GetCryptoContext().InitFromSecret(
        secret, ByteSpan(nullptr, 0), CryptoContext::SessionInfoType::kSessionEstablishment, role));
    secureSession->GetSessionMessageCounter().GetPeerMessageCounter().SetCounter(LocalSessionMessageCounter::kInitialSyncValue);
    sessionHolder.Grab(session.Value());
    return CHIP_NO_ERROR;
}

void SessionManager::OnMessageReceived(const PeerAddress & peerAddress, System::PacketBufferHandle && msg)
{
    CHIP_TRACE_PREPARED_MESSAGE_RECEIVED(&peerAddress, &msg);
    PacketHeader packetHeader;

    ReturnOnFailure(packetHeader.DecodeAndConsume(msg));

    if (packetHeader.IsEncrypted())
    {
        if (packetHeader.IsGroupSession())
        {
            SecureGroupMessageDispatch(packetHeader, peerAddress, std::move(msg));
        }
        else
        {
            SecureUnicastMessageDispatch(packetHeader, peerAddress, std::move(msg));
        }
    }
    else
    {
        UnauthenticatedMessageDispatch(packetHeader, peerAddress, std::move(msg));
    }
}

void SessionManager::UnauthenticatedMessageDispatch(const PacketHeader & packetHeader, const Transport::PeerAddress & peerAddress,
                                                    System::PacketBufferHandle && msg)
{
    Optional<NodeId> source      = packetHeader.GetSourceNodeId();
    Optional<NodeId> destination = packetHeader.GetDestinationNodeId();
    if ((source.HasValue() && destination.HasValue()) || (!source.HasValue() && !destination.HasValue()))
    {
        ChipLogProgress(Inet,
                        "Received malformed unsecure packet with source 0x" ChipLogFormatX64 " destination 0x" ChipLogFormatX64,
                        ChipLogValueX64(source.ValueOr(kUndefinedNodeId)), ChipLogValueX64(destination.ValueOr(kUndefinedNodeId)));
        return; // ephemeral node id is only assigned to the initiator, there should be one and only one node id exists.
    }

    Optional<SessionHandle> optionalSession;
    if (source.HasValue())
    {
        // Assume peer is the initiator, we are the responder.
        optionalSession = mUnauthenticatedSessions.FindOrAllocateResponder(source.Value(), GetLocalMRPConfig());
        if (!optionalSession.HasValue())
        {
            ChipLogError(Inet, "UnauthenticatedSession exhausted");
            return;
        }
    }
    else
    {
        // Assume peer is the responder, we are the initiator.
        optionalSession = mUnauthenticatedSessions.FindInitiator(destination.Value());
        if (!optionalSession.HasValue())
        {
            ChipLogProgress(Inet, "Received unknown unsecure packet for initiator 0x" ChipLogFormatX64,
                            ChipLogValueX64(destination.Value()));
            return;
        }
    }

    const SessionHandle & session                        = optionalSession.Value();
    Transport::UnauthenticatedSession * unsecuredSession = session->AsUnauthenticatedSession();
    unsecuredSession->SetPeerAddress(peerAddress);
    SessionMessageDelegate::DuplicateMessage isDuplicate = SessionMessageDelegate::DuplicateMessage::No;

    unsecuredSession->MarkActiveRx();

    PayloadHeader payloadHeader;
    ReturnOnFailure(payloadHeader.DecodeAndConsume(msg));

    // Verify message counter
    CHIP_ERROR err = unsecuredSession->GetPeerMessageCounter().VerifyUnencrypted(packetHeader.GetMessageCounter());
    if (err == CHIP_ERROR_DUPLICATE_MESSAGE_RECEIVED)
    {
        ChipLogDetail(Inet,
                      "Received a duplicate message with MessageCounter:" ChipLogFormatMessageCounter
                      " on exchange " ChipLogFormatExchangeId,
                      packetHeader.GetMessageCounter(), ChipLogValueExchangeIdFromReceivedHeader(payloadHeader));
        isDuplicate = SessionMessageDelegate::DuplicateMessage::Yes;
        err         = CHIP_NO_ERROR;
    }
    else
    {
        // VerifyUnencrypted always returns one of CHIP_NO_ERROR or
        // CHIP_ERROR_DUPLICATE_MESSAGE_RECEIVED.
        unsecuredSession->GetPeerMessageCounter().CommitUnencrypted(packetHeader.GetMessageCounter());
    }

    if (mCB != nullptr)
    {
        CHIP_TRACE_MESSAGE_RECEIVED(payloadHeader, packetHeader, unsecuredSession, peerAddress, msg->Start(), msg->TotalLength());
        mCB->OnMessageReceived(packetHeader, payloadHeader, session, isDuplicate, std::move(msg));
    }
}

void SessionManager::SecureUnicastMessageDispatch(const PacketHeader & packetHeader, const Transport::PeerAddress & peerAddress,
                                                  System::PacketBufferHandle && msg)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    Optional<SessionHandle> session = mSecureSessions.FindSecureSessionByLocalKey(packetHeader.GetSessionId());

    PayloadHeader payloadHeader;

    SessionMessageDelegate::DuplicateMessage isDuplicate = SessionMessageDelegate::DuplicateMessage::No;

    if (msg.IsNull())
    {
        ChipLogError(Inet, "Secure transport received Unicast NULL packet, discarding");
        return;
    }

    if (!session.HasValue())
    {
        ChipLogError(Inet, "Data received on an unknown session (LSID=%d). Dropping it!", packetHeader.GetSessionId());
        return;
    }

    Transport::SecureSession * secureSession = session.Value()->AsSecureSession();
    // Decrypt and verify the message before message counter verification or any further processing.
    CryptoContext::NonceStorage nonce;
    // PASE Sessions use the undefined node ID of all zeroes, since there is no node ID to use
    // and the key is short-lived and always different for each PASE session.
    CryptoContext::BuildNonce(nonce, packetHeader.GetSecurityFlags(), packetHeader.GetMessageCounter(),
                              secureSession->GetSecureSessionType() == SecureSession::Type::kCASE ? secureSession->GetPeerNodeId()
                                                                                                  : kUndefinedNodeId);
    if (SecureMessageCodec::Decrypt(secureSession->GetCryptoContext(), nonce, payloadHeader, packetHeader, msg) != CHIP_NO_ERROR)
    {
        ChipLogError(Inet, "Secure transport received message, but failed to decode/authenticate it, discarding");
        return;
    }

    err =
        secureSession->GetSessionMessageCounter().GetPeerMessageCounter().VerifyEncryptedUnicast(packetHeader.GetMessageCounter());
    if (err == CHIP_ERROR_DUPLICATE_MESSAGE_RECEIVED)
    {
        ChipLogDetail(Inet,
                      "Received a duplicate message with MessageCounter:" ChipLogFormatMessageCounter
                      " on exchange " ChipLogFormatExchangeId,
                      packetHeader.GetMessageCounter(), ChipLogValueExchangeIdFromReceivedHeader(payloadHeader));
        isDuplicate = SessionMessageDelegate::DuplicateMessage::Yes;
        err         = CHIP_NO_ERROR;
    }
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Inet, "Message counter verify failed, err = %" CHIP_ERROR_FORMAT, err.Format());
        return;
    }

    secureSession->MarkActiveRx();

    if (isDuplicate == SessionMessageDelegate::DuplicateMessage::Yes && !payloadHeader.NeedsAck())
    {
        // If it's a duplicate message, but doesn't require an ack, let's drop it right here to save CPU
        // cycles on further message processing.
        return;
    }

    if (isDuplicate == SessionMessageDelegate::DuplicateMessage::No)
    {
        secureSession->GetSessionMessageCounter().GetPeerMessageCounter().CommitEncryptedUnicast(packetHeader.GetMessageCounter());
    }

    // TODO: once mDNS address resolution is available reconsider if this is required
    // This updates the peer address once a packet is received from a new address
    // and serves as a way to auto-detect peer changing IPs.
    if (secureSession->GetPeerAddress() != peerAddress)
    {
        secureSession->SetPeerAddress(peerAddress);
    }

    if (mCB != nullptr)
    {
        CHIP_TRACE_MESSAGE_RECEIVED(payloadHeader, packetHeader, secureSession, peerAddress, msg->Start(), msg->TotalLength());
        mCB->OnMessageReceived(packetHeader, payloadHeader, session.Value(), isDuplicate, std::move(msg));
    }
}

void SessionManager::SecureGroupMessageDispatch(const PacketHeader & packetHeader, const Transport::PeerAddress & peerAddress,
                                                System::PacketBufferHandle && msg)
{
    PayloadHeader payloadHeader;
    Credentials::GroupDataProvider * groups = Credentials::GetGroupDataProvider();
    VerifyOrReturn(nullptr != groups);
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (!packetHeader.GetDestinationGroupId().HasValue())
    {
        return; // malformed packet
    }

    GroupId groupId = packetHeader.GetDestinationGroupId().Value();

    if (msg.IsNull())
    {
        ChipLogError(Inet, "Secure transport received Groupcast NULL packet, discarding");
        return;
    }

    // Check if Message Header is valid first
    if (!(packetHeader.IsValidMCSPMsg() || packetHeader.IsValidGroupMsg()))
    {
        ChipLogError(Inet, "Invalid condition found in packet header");
        return;
    }

    // Trial decryption with GroupDataProvider
    Credentials::GroupDataProvider::GroupSession groupContext;
    auto iter = groups->IterateGroupSessions(packetHeader.GetSessionId());
    if (iter == nullptr)
    {
        ChipLogError(Inet, "Failed to retrieve Groups iterator. Discarding everything");
        return;
    }

    System::PacketBufferHandle msgCopy;
    bool decrypted = false;
    while (!decrypted && iter->Next(groupContext))
    {
        // Optimization to reduce number of decryption attempts
        if (groupId != groupContext.group_id)
        {
            continue;
        }
        msgCopy = msg.CloneData();
        CryptoContext::NonceStorage nonce;
        CryptoContext::BuildNonce(nonce, packetHeader.GetSecurityFlags(), packetHeader.GetMessageCounter(),
                                  packetHeader.GetSourceNodeId().Value());
        decrypted = (CHIP_NO_ERROR ==
                     SecureMessageCodec::Decrypt(CryptoContext(groupContext.key), nonce, payloadHeader, packetHeader, msgCopy));
    }
    iter->Release();
    if (!decrypted)
    {
        ChipLogError(Inet, "Failed to retrieve Key. Discarding everything");
        return;
    }
    msg = std::move(msgCopy);

    // MCSP check
    if (packetHeader.IsValidMCSPMsg())
    {
        // TODO: When MCSP Msg, create Secure Session instead of a Group session

        // TODO
        // if (packetHeader.GetDestinationNodeId().Value() == ThisDeviceNodeID)
        // {
        //     MCSP processing..
        // }

        return;
    }

    // Group Messages should never send an Ack
    if (payloadHeader.NeedsAck())
    {
        ChipLogError(Inet, "Unexpected ACK requested for group message");
        return;
    }

    // Handle Group message counter here spec 4.7.3
    // spec 4.5.1.2 for msg counter
    chip::Transport::PeerMessageCounter * counter = nullptr;

    if (CHIP_NO_ERROR ==
        mGroupPeerMsgCounter.FindOrAddPeer(groupContext.fabric_index, packetHeader.GetSourceNodeId().Value(),
                                           packetHeader.IsSecureSessionControlMsg(), counter))
    {

        if (Credentials::GroupDataProvider::SecurityPolicy::kTrustFirst == groupContext.security_policy)
        {
            err = counter->VerifyOrTrustFirstGroup(packetHeader.GetMessageCounter());
        }
        else
        {

            // TODO support cache and sync with MCSP. Issue  #11689
            ChipLogError(Inet, "Received Group Msg with key policy Cache and Sync, but MCSP is not implemented");
            return;

            // cache and sync
            // err = counter->VerifyGroup(packetHeader.GetMessageCounter());
        }

        if (err != CHIP_NO_ERROR)
        {
            // Exit now, since Group Messages don't have acks or responses of any kind.
            ChipLogError(Inet, "Message counter verify failed, err = %" CHIP_ERROR_FORMAT, err.Format());
            return;
        }
    }
    else
    {
        ChipLogError(Inet,
                     "Group Counter Tables full or invalid NodeId/FabricIndex after decryption of message, dropping everything");
        return;
    }

    counter->CommitGroup(packetHeader.GetMessageCounter());

    if (mCB != nullptr)
    {
        // TODO : When MCSP is done, clean up session creation logic
        Transport::IncomingGroupSession groupSession(groupContext.group_id, groupContext.fabric_index,
                                                     packetHeader.GetSourceNodeId().Value());
        CHIP_TRACE_MESSAGE_RECEIVED(payloadHeader, packetHeader, &groupSession, peerAddress, msg->Start(), msg->TotalLength());
        mCB->OnMessageReceived(packetHeader, payloadHeader, SessionHandle(groupSession),
                               SessionMessageDelegate::DuplicateMessage::No, std::move(msg));
    }
}

Optional<SessionHandle> SessionManager::FindSecureSessionForNode(ScopedNodeId peerNodeId,
                                                                 const Optional<Transport::SecureSession::Type> & type)
{
    SecureSession * found = nullptr;
    mSecureSessions.ForEachSession([&peerNodeId, &type, &found](auto session) {
        if (session->IsActiveSession() && session->GetPeer() == peerNodeId &&
            (!type.HasValue() || type.Value() == session->GetSecureSessionType()))
        {
            found = session;
            return Loop::Break;
        }
        return Loop::Continue;
    });

    return found != nullptr ? MakeOptional<SessionHandle>(*found) : Optional<SessionHandle>::Missing();
}

/**
 * Provides a means to get diagnostic information such as number of sessions.
 */
[[maybe_unused]] CHIP_ERROR SessionManager::ForEachSessionHandle(void * context, SessionHandleCallback lambda)
{
    mSecureSessions.ForEachSession([&](auto session) {
        SessionHandle handle(*session);
        lambda(context, handle);
        return Loop::Continue;
    });
    return CHIP_NO_ERROR;
}

} // namespace chip
