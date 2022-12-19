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
#include <inttypes.h>
#include <lib/core/CHIPKeyIds.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/Protocols.h>
#include <protocols/secure_channel/Constants.h>
#include <transport/GroupPeerMessageCounter.h>
#include <transport/GroupSession.h>
#include <transport/SecureMessageCodec.h>
#include <transport/TransportMgr.h>

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

    ChipLogError(Inet, "Failed to decode EncryptedPacketBufferHandle header with error: %" CHIP_ERROR_FORMAT, err.Format());

    return 0;
}

SessionManager::SessionManager() : mState(State::kNotReady) {}

SessionManager::~SessionManager()
{
    this->Shutdown();
}

CHIP_ERROR SessionManager::Init(System::Layer * systemLayer, TransportMgrBase * transportMgr,
                                Transport::MessageCounterManagerInterface * messageCounterManager,
                                chip::PersistentStorageDelegate * storageDelegate, FabricTable * fabricTable)
{
    VerifyOrReturnError(mState == State::kNotReady, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(transportMgr != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(storageDelegate != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(fabricTable != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorOnFailure(fabricTable->AddFabricDelegate(this));

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
    if (mFabricTable != nullptr)
    {
        mFabricTable->RemoveFabricDelegate(this);
        mFabricTable = nullptr;
    }

    // Ensure that we don't create new sessions as we iterate our session table.
    mState = State::kNotReady;

    mSecureSessions.ForEachSession([&](auto session) {
        session->MarkForEviction();
        return Loop::Continue;
    });

    mMessageCounterManager = nullptr;

    mSystemLayer  = nullptr;
    mTransportMgr = nullptr;
    mCB           = nullptr;
}

/**
 * @brief Notification that a fabric was removed.
 *        This function doesn't call ExpireAllSessionsForFabric
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

        const FabricInfo * fabric = mFabricTable->FindFabricWithIndex(groupSession->GetFabricIndex());
        VerifyOrReturnError(fabric != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

        packetHeader.SetDestinationGroupId(groupSession->GetGroupId());
        packetHeader.SetMessageCounter(mGroupClientCounter.GetCounter(isControlMsg));
        mGroupClientCounter.IncrementCounter(isControlMsg);
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
        destination = NodeIdFromGroupId(groupSession->GetGroupId());
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
        uint32_t messageCounter;
        ReturnErrorOnFailure(counter.AdvanceAndConsume(messageCounter));
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

#if CHIP_PROGRESS_LOGGING
        destination = session->GetPeerNodeId();
        fabricIndex = session->GetFabricIndex();
#endif // CHIP_PROGRESS_LOGGING
    }
    break;
    case Transport::Session::SessionType::kUnauthenticated: {
        MessageCounter & counter = mGlobalUnencryptedMessageCounter;
        uint32_t messageCounter;
        ReturnErrorOnFailure(counter.AdvanceAndConsume(messageCounter));
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

#if CHIP_PROGRESS_LOGGING
    CompressedFabricId compressedFabricId = kUndefinedCompressedFabricId;

    if (fabricIndex != kUndefinedFabricIndex && mFabricTable != nullptr)
    {
        auto fabricInfo = mFabricTable->FindFabricWithIndex(fabricIndex);
        if (fabricInfo)
        {
            compressedFabricId = fabricInfo->GetCompressedFabricId();
        }
    }

    auto * protocolName = Protocols::GetProtocolName(payloadHeader.GetProtocolID());
    auto * msgTypeName  = Protocols::GetMessageTypeName(payloadHeader.GetProtocolID(), payloadHeader.GetMessageType());

    //
    // 32-bit value maximum = 10 chars + text preamble (6) + trailer (1) + null (1) + 2 buffer = 20
    //
    char ackBuf[20];
    ackBuf[0] = '\0';
    if (payloadHeader.GetAckMessageCounter().HasValue())
    {
        snprintf(ackBuf, sizeof(ackBuf), " (Ack:" ChipLogFormatMessageCounter ")", payloadHeader.GetAckMessageCounter().Value());
    }

    //
    // Legend that can be used to decode this log line can be found in messaging/README.md
    //
    ChipLogProgress(ExchangeManager,
                    "<<< [E:" ChipLogFormatExchangeId " M:" ChipLogFormatMessageCounter "%s] (%s) Msg TX to %u:" ChipLogFormatX64
                    " [%04X] --- Type %04X:%02X (%s:%s)",
                    ChipLogValueExchangeIdFromSentHeader(payloadHeader), packetHeader.GetMessageCounter(), ackBuf,
                    Transport::GetSessionTypeString(sessionHandle), fabricIndex, ChipLogValueX64(destination),
                    static_cast<uint16_t>(compressedFabricId), payloadHeader.GetProtocolID().GetProtocolId(),
                    payloadHeader.GetMessageType(), protocolName, msgTypeName);
#endif

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

        const FabricInfo * fabric = mFabricTable->FindFabricWithIndex(groupSession->GetFabricIndex());
        VerifyOrReturnError(fabric != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

        multicastAddress = Transport::PeerAddress::Multicast(fabric->GetFabricId(), groupSession->GetGroupId());
        destination      = &multicastAddress;
        char addressStr[Transport::PeerAddress::kMaxToStringSize];
        multicastAddress.ToString(addressStr, Transport::PeerAddress::kMaxToStringSize);

        ChipLogProgress(Inet, "(G) Sending msg " ChipLogFormatMessageCounter " to Multicast IPV6 address '%s'",
                        preparedMessage.GetMessageCounter(), addressStr);
    }
    break;
    case Transport::Session::SessionType::kSecure: {
        // Find an active connection to the specified peer node
        SecureSession * secure = sessionHandle->AsSecureSession();

        // This marks any connection where we send data to as 'active'
        secure->MarkActive();

        destination = &secure->GetPeerAddress();

        ChipLogProgress(Inet, "(S) Sending msg " ChipLogFormatMessageCounter " on secure session with LSID: %u",
                        preparedMessage.GetMessageCounter(), secure->GetLocalSessionId());
    }
    break;
    case Transport::Session::SessionType::kUnauthenticated: {
        auto unauthenticated = sessionHandle->AsUnauthenticatedSession();
        unauthenticated->MarkActive();
        destination = &unauthenticated->GetPeerAddress();

#if CHIP_PROGRESS_LOGGING
        char addressStr[Transport::PeerAddress::kMaxToStringSize];
        destination->ToString(addressStr);

        ChipLogProgress(Inet, "(U) Sending msg " ChipLogFormatMessageCounter " to IP address '%s'",
                        preparedMessage.GetMessageCounter(), addressStr);
#endif
    }
    break;
    default:
        return CHIP_ERROR_INTERNAL;
    }

    PacketBufferHandle msgBuf = preparedMessage.CastToWritable();
    VerifyOrReturnError(!msgBuf.IsNull(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(!msgBuf->HasChainedBuffer(), CHIP_ERROR_INVALID_MESSAGE_LENGTH);

#if CHIP_SYSTEM_CONFIG_MULTICAST_HOMING
    if (sessionHandle->GetSessionType() == Transport::Session::SessionType::kGroupOutgoing)
    {
        chip::Inet::InterfaceIterator interfaceIt;
        chip::Inet::InterfaceId interfaceId = chip::Inet::InterfaceId::Null();
        chip::Inet::IPAddress addr;
        bool interfaceFound = false;

        while (interfaceIt.Next())
        {
            char name[chip::Inet::InterfaceId::kMaxIfNameLength];
            interfaceIt.GetInterfaceName(name, chip::Inet::InterfaceId::kMaxIfNameLength);
            if (interfaceIt.SupportsMulticast() && interfaceIt.IsUp())
            {
                interfaceId = interfaceIt.GetInterfaceId();
                if (CHIP_NO_ERROR == interfaceId.GetLinkLocalAddr(&addr))
                {
                    ChipLogDetail(Inet, "Interface %s has a link local address", name);

                    interfaceFound             = true;
                    PacketBufferHandle tempBuf = msgBuf.CloneData();
                    VerifyOrReturnError(!tempBuf.IsNull(), CHIP_ERROR_INVALID_ARGUMENT);
                    VerifyOrReturnError(!tempBuf->HasChainedBuffer(), CHIP_ERROR_INVALID_MESSAGE_LENGTH);

                    destination = &(multicastAddress.SetInterface(interfaceId));
                    if (mTransportMgr != nullptr)
                    {
                        CHIP_TRACE_PREPARED_MESSAGE_SENT(destination, &tempBuf);
                        if (CHIP_NO_ERROR != mTransportMgr->SendMessage(*destination, std::move(tempBuf)))
                        {
                            ChipLogError(Inet, "Failed to send Multicast message on interface %s", name);
                        }
                        else
                        {
                            ChipLogDetail(Inet, "Successfully send Multicast message on interface %s", name);
                        }
                    }
                }
            }
        }

        if (!interfaceFound)
        {
            ChipLogError(Inet, "No valid Interface found.. Sending to the default one.. ");
        }
        else
        {
            // Always return No error, because we expect some interface to fails and others to always succeed (e.g. lo interface)
            return CHIP_NO_ERROR;
        }
    }

#endif // CHIP_SYSTEM_CONFIG_MULTICAST_HOMING

    if (mTransportMgr != nullptr)
    {
        CHIP_TRACE_PREPARED_MESSAGE_SENT(destination, &msgBuf);
        return mTransportMgr->SendMessage(*destination, std::move(msgBuf));
    }

    ChipLogError(Inet, "The transport manager is not initialized. Unable to send the message");
    return CHIP_ERROR_INCORRECT_STATE;
}

void SessionManager::ExpireAllSessions(const ScopedNodeId & node)
{
    ChipLogDetail(Inet, "Expiring all sessions for node " ChipLogFormatScopedNodeId "!!", ChipLogValueScopedNodeId(node));

    ForEachMatchingSession(node, [](auto * session) { session->MarkForEviction(); });
}

void SessionManager::ExpireAllSessionsForFabric(FabricIndex fabricIndex)
{
    ChipLogDetail(Inet, "Expiring all sessions for fabric 0x%x!!", static_cast<unsigned>(fabricIndex));

    ForEachMatchingSession(fabricIndex, [](auto * session) { session->MarkForEviction(); });
}

CHIP_ERROR SessionManager::ExpireAllSessionsOnLogicalFabric(const ScopedNodeId & node)
{
    ChipLogDetail(Inet, "Expiring all sessions to peer " ChipLogFormatScopedNodeId " that are on the same logical fabric!!",
                  ChipLogValueScopedNodeId(node));

    return ForEachMatchingSessionOnLogicalFabric(node, [](auto * session) { session->MarkForEviction(); });
}

CHIP_ERROR SessionManager::ExpireAllSessionsOnLogicalFabric(FabricIndex fabricIndex)
{
    ChipLogDetail(Inet, "Expiring all sessions on the same logical fabric as fabric 0x%x!!", static_cast<unsigned>(fabricIndex));

    return ForEachMatchingSessionOnLogicalFabric(fabricIndex, [](auto * session) { session->MarkForEviction(); });
}

void SessionManager::ExpireAllPASESessions()
{
    ChipLogDetail(Inet, "Expiring all PASE sessions");
    mSecureSessions.ForEachSession([&](auto session) {
        if (session->GetSecureSessionType() == Transport::SecureSession::Type::kPASE)
        {
            session->MarkForEviction();
        }
        return Loop::Continue;
    });
}

void SessionManager::MarkSessionsAsDefunct(const ScopedNodeId & node, const Optional<Transport::SecureSession::Type> & type)
{
    mSecureSessions.ForEachSession([&node, &type](auto session) {
        if (session->IsActiveSession() && session->GetPeer() == node &&
            (!type.HasValue() || type.Value() == session->GetSecureSessionType()))
        {
            session->MarkAsDefunct();
        }
        return Loop::Continue;
    });
}

void SessionManager::UpdateAllSessionsPeerAddress(const ScopedNodeId & node, const Transport::PeerAddress & addr)
{
    mSecureSessions.ForEachSession([&node, &addr](auto session) {
        // Arguably we should only be updating active and defunct sessions, but there is no harm
        // in updating evicted sessions.
        if (session->GetPeer() == node && Transport::SecureSession::Type::kCASE == session->GetSecureSessionType())
        {
            session->SetPeerAddress(addr);
        }
        return Loop::Continue;
    });
}

Optional<SessionHandle> SessionManager::AllocateSession(SecureSession::Type secureSessionType,
                                                        const ScopedNodeId & sessionEvictionHint)
{
    VerifyOrReturnValue(mState == State::kInitialized, NullOptional);
    return mSecureSessions.CreateNewSecureSession(secureSessionType, sessionEvictionHint);
}

CHIP_ERROR SessionManager::InjectPaseSessionWithTestKey(SessionHolder & sessionHolder, uint16_t localSessionId, NodeId peerNodeId,
                                                        uint16_t peerSessionId, FabricIndex fabric,
                                                        const Transport::PeerAddress & peerAddress, CryptoContext::SessionRole role)
{
    NodeId localNodeId              = kUndefinedNodeId;
    Optional<SessionHandle> session = mSecureSessions.CreateNewSecureSessionForTest(
        chip::Transport::SecureSession::Type::kPASE, localSessionId, localNodeId, peerNodeId, CATValues{}, peerSessionId, fabric,
        GetLocalMRPConfig().ValueOr(GetDefaultMRPConfig()));
    VerifyOrReturnError(session.HasValue(), CHIP_ERROR_NO_MEMORY);
    SecureSession * secureSession = session.Value()->AsSecureSession();
    secureSession->SetPeerAddress(peerAddress);

    size_t secretLen = strlen(CHIP_CONFIG_TEST_SHARED_SECRET_VALUE);
    ByteSpan secret(reinterpret_cast<const uint8_t *>(CHIP_CONFIG_TEST_SHARED_SECRET_VALUE), secretLen);
    ReturnErrorOnFailure(secureSession->GetCryptoContext().InitFromSecret(
        secret, ByteSpan(nullptr, 0), CryptoContext::SessionInfoType::kSessionEstablishment, role));
    secureSession->GetSessionMessageCounter().GetPeerMessageCounter().SetCounter(Transport::PeerMessageCounter::kInitialSyncValue);
    sessionHolder.Grab(session.Value());
    return CHIP_NO_ERROR;
}

CHIP_ERROR SessionManager::InjectCaseSessionWithTestKey(SessionHolder & sessionHolder, uint16_t localSessionId,
                                                        uint16_t peerSessionId, NodeId localNodeId, NodeId peerNodeId,
                                                        FabricIndex fabric, const Transport::PeerAddress & peerAddress,
                                                        CryptoContext::SessionRole role, const CATValues & cats)
{
    Optional<SessionHandle> session = mSecureSessions.CreateNewSecureSessionForTest(
        chip::Transport::SecureSession::Type::kCASE, localSessionId, localNodeId, peerNodeId, cats, peerSessionId, fabric,
        GetLocalMRPConfig().ValueOr(GetDefaultMRPConfig()));
    VerifyOrReturnError(session.HasValue(), CHIP_ERROR_NO_MEMORY);
    SecureSession * secureSession = session.Value()->AsSecureSession();
    secureSession->SetPeerAddress(peerAddress);

    size_t secretLen = strlen(CHIP_CONFIG_TEST_SHARED_SECRET_VALUE);
    ByteSpan secret(reinterpret_cast<const uint8_t *>(CHIP_CONFIG_TEST_SHARED_SECRET_VALUE), secretLen);
    ReturnErrorOnFailure(secureSession->GetCryptoContext().InitFromSecret(
        secret, ByteSpan(nullptr, 0), CryptoContext::SessionInfoType::kSessionEstablishment, role));
    secureSession->GetSessionMessageCounter().GetPeerMessageCounter().SetCounter(Transport::PeerMessageCounter::kInitialSyncValue);
    sessionHolder.Grab(session.Value());
    return CHIP_NO_ERROR;
}

void SessionManager::OnMessageReceived(const PeerAddress & peerAddress, System::PacketBufferHandle && msg)
{
    CHIP_TRACE_PREPARED_MESSAGE_RECEIVED(&peerAddress, &msg);
    PacketHeader packetHeader;

    CHIP_ERROR err = packetHeader.DecodeAndConsume(msg);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Inet, "Failed to decode packet header: %" CHIP_ERROR_FORMAT, err.Format());
        return;
    }

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
        optionalSession = mUnauthenticatedSessions.FindOrAllocateResponder(source.Value(), GetDefaultMRPConfig());
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

    // We need to allow through messages even on sessions that are pending
    // evictions, because for some cases (UpdateNOC, RemoveFabric, etc) there
    // can be a single exchange alive on the session waiting for a MRP ack, and
    // we need to make sure to send the ack through.  The exchange manager is
    // responsible for ensuring that such messages do not lead to new exchange
    // creation.
    if (!secureSession->IsDefunct() && !secureSession->IsActiveSession() && !secureSession->IsPendingEviction())
    {
        ChipLogError(Inet, "Secure transport received message on a session in an invalid state (state = '%s')",
                     secureSession->GetStateStr());
        return;
    }

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
    Transport::PeerMessageCounter * counter = nullptr;

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
            //
            // Select the active session with the most recent activity to return back to the caller.
            //
            if ((found == nullptr) || (found->GetLastActivityTime() < session->GetLastActivityTime()))
            {
                found = session;
            }
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
