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
#include <lib/core/Global.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/Protocols.h>
#include <protocols/secure_channel/Constants.h>
#include <tracing/macros.h>
#include <transport/GroupPeerMessageCounter.h>
#include <transport/GroupSession.h>
#include <transport/SecureMessageCodec.h>
#include <transport/TracingStructs.h>
#include <transport/TransportMgr.h>

namespace chip {

using System::PacketBufferHandle;
using Transport::GroupPeerTable;
using Transport::PeerAddress;
using Transport::SecureSession;

namespace {
Global<GroupPeerTable> gGroupPeerTable;

/// RAII class for iterators that guarantees that Release() will be called
/// on the underlying type
template <typename Releasable>
class AutoRelease
{
public:
    AutoRelease(Releasable * iter) : mIter(iter) {}
    ~AutoRelease() { Release(); }

    Releasable * operator->() { return mIter; }
    const Releasable * operator->() const { return mIter; }

    bool IsNull() const { return mIter == nullptr; }

    void Release()
    {
        VerifyOrReturn(mIter != nullptr);
        mIter->Release();
        mIter = nullptr;
    }

private:
    Releasable * mIter = nullptr;
};

// Helper function that strips off the interface ID from a peer address that is
// not an IPv6 link-local address.  For any other address type we should rely on
// the device's routing table to route messages sent.  Forcing messages down a
// specific interface might fail with "no route to host".
void CorrectPeerAddressInterfaceID(Transport::PeerAddress & peerAddress)
{
    if (peerAddress.GetIPAddress().IsIPv6LinkLocal())
    {
        return;
    }
    peerAddress.SetInterface(Inet::InterfaceId::Null());
}

} // namespace

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
                                chip::PersistentStorageDelegate * storageDelegate, FabricTable * fabricTable,
                                Crypto::SessionKeystore & sessionKeystore)
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
    mSessionKeystore       = &sessionKeystore;

    mSecureSessions.Init();

    mGlobalUnencryptedMessageCounter.Init();

    ReturnErrorOnFailure(mGroupClientCounter.Init(storageDelegate));

    mTransportMgr->SetSessionManager(this);

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    mConnCompleteCb = nullptr;
    mConnClosedCb   = nullptr;
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

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

    // Just in case some consumer forgot to do it, expire all our secure
    // sessions.  Note that this stands a good chance of crashing with a
    // null-deref if there are in fact any secure sessions left, since they will
    // try to notify their exchanges, which will then try to operate on
    // partially-shut-down objects.
    ExpireAllSecureSessions();

    // We don't have a safe way to check or affect the state of our
    // mUnauthenticatedSessions.  We can only hope they got shut down properly.

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
    gGroupPeerTable->FabricRemoved(fabricIndex);
}

CHIP_ERROR SessionManager::PrepareMessage(const SessionHandle & sessionHandle, PayloadHeader & payloadHeader,
                                          System::PacketBufferHandle && message, EncryptedPacketBufferHandle & preparedMessage)
{
    MATTER_TRACE_SCOPE("PrepareMessage", "SessionManager");

    PacketHeader packetHeader;
    bool isControlMsg = IsControlMessage(payloadHeader);
    if (isControlMsg)
    {
        packetHeader.SetSecureSessionControlMsg(true);
    }

    if (sessionHandle->AllowsLargePayload())
    {
        VerifyOrReturnError(message->TotalLength() <= kMaxLargeAppMessageLen, CHIP_ERROR_MESSAGE_TOO_LONG);
    }
    else
    {
        VerifyOrReturnError(message->TotalLength() <= kMaxAppMessageLen, CHIP_ERROR_MESSAGE_TOO_LONG);
    }

#if CHIP_PROGRESS_LOGGING
    NodeId destination;
    FabricIndex fabricIndex;
#endif // CHIP_PROGRESS_LOGGING

    NodeId sourceNodeId = kUndefinedNodeId;
    PeerAddress destination_address;

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
        sourceNodeId = fabric->GetNodeId();
        packetHeader.SetSourceNodeId(sourceNodeId);

        if (!packetHeader.IsValidGroupMsg())
        {
            return CHIP_ERROR_INTERNAL;
        }

        destination_address = Transport::PeerAddress::Multicast(fabric->GetFabricId(), groupSession->GetGroupId());

        // Trace before any encryption
        MATTER_LOG_MESSAGE_SEND(chip::Tracing::OutgoingMessageType::kGroupMessage, &payloadHeader, &packetHeader,
                                chip::ByteSpan(message->Start(), message->TotalLength()));

        CHIP_TRACE_MESSAGE_SENT(payloadHeader, packetHeader, destination_address, message->Start(), message->TotalLength());

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

        destination_address = session->GetPeerAddress();

        // Trace before any encryption
        MATTER_LOG_MESSAGE_SEND(chip::Tracing::OutgoingMessageType::kSecureSession, &payloadHeader, &packetHeader,
                                chip::ByteSpan(message->Start(), message->TotalLength()));
        CHIP_TRACE_MESSAGE_SENT(payloadHeader, packetHeader, destination_address, message->Start(), message->TotalLength());

        CryptoContext::NonceStorage nonce;
        sourceNodeId = session->GetLocalScopedNodeId().GetNodeId();
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

        auto unauthenticated = sessionHandle->AsUnauthenticatedSession();
        destination_address  = unauthenticated->GetPeerAddress();

        // Trace after all headers are settled.
        MATTER_LOG_MESSAGE_SEND(chip::Tracing::OutgoingMessageType::kUnauthenticated, &payloadHeader, &packetHeader,
                                chip::ByteSpan(message->Start(), message->TotalLength()));
        CHIP_TRACE_MESSAGE_SENT(payloadHeader, packetHeader, destination_address, message->Start(), message->TotalLength());

        ReturnErrorOnFailure(payloadHeader.EncodeBeforeData(message));

#if CHIP_PROGRESS_LOGGING
        destination = kUndefinedNodeId;
        fabricIndex = kUndefinedFabricIndex;
        if (session->GetSessionRole() == Transport::UnauthenticatedSession::SessionRole::kResponder)
        {
            destination = session->GetEphemeralInitiatorNodeID();
        }
        else if (session->GetSessionRole() == Transport::UnauthenticatedSession::SessionRole::kInitiator)
        {
            sourceNodeId = session->GetEphemeralInitiatorNodeID();
        }
#endif // CHIP_PROGRESS_LOGGING
    }
    break;
    default:
        return CHIP_ERROR_INTERNAL;
    }

    ReturnErrorOnFailure(packetHeader.EncodeBeforeData(message));

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

    char addressStr[Transport::PeerAddress::kMaxToStringSize] = { 0 };
    destination_address.ToString(addressStr);

    // Work around pigweed not allowing more than 14 format args in a log
    // message when using tokenized logs.
    char typeStr[4 + 1 + 2 + 1];
    snprintf(typeStr, sizeof(typeStr), "%04X:%02X", payloadHeader.GetProtocolID().GetProtocolId(), payloadHeader.GetMessageType());

    // More work around pigweed not allowing more than 14 format args in a log
    // message when using tokenized logs.
    // ChipLogFormatExchangeId logs the numeric exchange ID (at most 5 chars,
    // since it's a uint16_t) and one char for initiator/responder.  Plus we
    // need a null-terminator.
    char exchangeStr[5 + 1 + 1];
    snprintf(exchangeStr, sizeof(exchangeStr), ChipLogFormatExchangeId, ChipLogValueExchangeIdFromSentHeader(payloadHeader));

    // More work around pigweed not allowing more than 14 format args in a log
    // message when using tokenized logs.
    // text(5) + source(16) + text(4) + fabricIndex(uint16_t, at most 5 chars) + text(1) + destination(16) + text(2) + compressed
    // fabric id(4) + text(1) + null-terminator
    char sourceDestinationStr[5 + 16 + 4 + 5 + 1 + 16 + 2 + 4 + 1 + 1];
    snprintf(sourceDestinationStr, sizeof(sourceDestinationStr), "from " ChipLogFormatX64 " to %u:" ChipLogFormatX64 " [%04X]",
             ChipLogValueX64(sourceNodeId), fabricIndex, ChipLogValueX64(destination), static_cast<uint16_t>(compressedFabricId));

    //
    // Legend that can be used to decode this log line can be found in messaging/README.md
    //
    ChipLogProgress(ExchangeManager,
                    "<<< [E:%s S:%u M:" ChipLogFormatMessageCounter "%s] (%s) Msg TX %s [%s] --- Type %s (%s:%s) (B:%u)",
                    exchangeStr, sessionHandle->SessionIdForLogging(), packetHeader.GetMessageCounter(), ackBuf,
                    Transport::GetSessionTypeString(sessionHandle), sourceDestinationStr, addressStr, typeStr, protocolName,
                    msgTypeName, static_cast<unsigned>(message->TotalLength()));
#endif

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
    }
    break;
    case Transport::Session::SessionType::kSecure: {
        // Find an active connection to the specified peer node
        SecureSession * secure = sessionHandle->AsSecureSession();

        // This marks any connection where we send data to as 'active'
        secure->MarkActive();

        destination = &secure->GetPeerAddress();
    }
    break;
    case Transport::Session::SessionType::kUnauthenticated: {
        auto unauthenticated = sessionHandle->AsUnauthenticatedSession();
        unauthenticated->MarkActive();
        destination = &unauthenticated->GetPeerAddress();
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
            char name[Inet::InterfaceId::kMaxIfNameLength];
            interfaceIt.GetInterfaceName(name, Inet::InterfaceId::kMaxIfNameLength);
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
        CHIP_ERROR err = mTransportMgr->SendMessage(*destination, std::move(msgBuf));
#if CHIP_ERROR_LOGGING
        if (err != CHIP_NO_ERROR)
        {
            char addressStr[Transport::PeerAddress::kMaxToStringSize] = { 0 };
            destination->ToString(addressStr);
            ChipLogError(Inet, "SendMessage() to %s failed: %" CHIP_ERROR_FORMAT, addressStr, err.Format());
        }
#endif // CHIP_ERROR_LOGGING
        return err;
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

void SessionManager::ExpireAllSecureSessions()
{
    mSecureSessions.ForEachSession([&](auto session) {
        session->MarkForEviction();
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

    size_t secretLen = CHIP_CONFIG_TEST_SHARED_SECRET_LENGTH;
    ByteSpan secret(reinterpret_cast<const uint8_t *>(CHIP_CONFIG_TEST_SHARED_SECRET_VALUE), secretLen);
    ReturnErrorOnFailure(secureSession->GetCryptoContext().InitFromSecret(
        *mSessionKeystore, secret, ByteSpan(), CryptoContext::SessionInfoType::kSessionEstablishment, role));
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

    size_t secretLen = CHIP_CONFIG_TEST_SHARED_SECRET_LENGTH;
    ByteSpan secret(reinterpret_cast<const uint8_t *>(CHIP_CONFIG_TEST_SHARED_SECRET_VALUE), secretLen);
    ReturnErrorOnFailure(secureSession->GetCryptoContext().InitFromSecret(
        *mSessionKeystore, secret, ByteSpan(), CryptoContext::SessionInfoType::kSessionEstablishment, role));
    secureSession->GetSessionMessageCounter().GetPeerMessageCounter().SetCounter(Transport::PeerMessageCounter::kInitialSyncValue);
    sessionHolder.Grab(session.Value());
    return CHIP_NO_ERROR;
}

void SessionManager::OnMessageReceived(const PeerAddress & peerAddress, System::PacketBufferHandle && msg,
                                       Transport::MessageTransportContext * ctxt)
{
    PacketHeader partialPacketHeader;

    CHIP_ERROR err = partialPacketHeader.DecodeFixed(msg);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Inet, "Failed to decode packet header: %" CHIP_ERROR_FORMAT, err.Format());
        return;
    }

    if (partialPacketHeader.IsEncrypted())
    {
        if (partialPacketHeader.IsGroupSession())
        {
            SecureGroupMessageDispatch(partialPacketHeader, peerAddress, std::move(msg));
        }
        else
        {
            SecureUnicastMessageDispatch(partialPacketHeader, peerAddress, std::move(msg), ctxt);
        }
    }
    else
    {
        UnauthenticatedMessageDispatch(partialPacketHeader, peerAddress, std::move(msg), ctxt);
    }
}

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
void SessionManager::HandleConnectionReceived(Transport::ActiveTCPConnectionState * conn)
{
    char peerAddrBuf[chip::Transport::PeerAddress::kMaxToStringSize];

    VerifyOrReturn(conn != nullptr);
    conn->mPeerAddr.ToString(peerAddrBuf);
    ChipLogProgress(Inet, "Received TCP connection request from %s.", peerAddrBuf);

    Transport::AppTCPConnectionCallbackCtxt * appTCPConnCbCtxt = conn->mAppState;
    if (appTCPConnCbCtxt != nullptr && appTCPConnCbCtxt->connReceivedCb != nullptr)
    {
        appTCPConnCbCtxt->connReceivedCb(conn);
    }
}

void SessionManager::HandleConnectionAttemptComplete(Transport::ActiveTCPConnectionState * conn, CHIP_ERROR conErr)
{
    VerifyOrReturn(conn != nullptr);

    Transport::AppTCPConnectionCallbackCtxt * appTCPConnCbCtxt = conn->mAppState;
    if (appTCPConnCbCtxt == nullptr)
    {
        TCPDisconnect(conn, /* shouldAbort = */ true);
        return;
    }

    if (appTCPConnCbCtxt->connCompleteCb != nullptr)
    {
        appTCPConnCbCtxt->connCompleteCb(conn, conErr);
    }
    else
    {
        char peerAddrBuf[chip::Transport::PeerAddress::kMaxToStringSize];
        conn->mPeerAddr.ToString(peerAddrBuf);

        ChipLogProgress(Inet, "TCP Connection established with peer %s, but no registered handler. Disconnecting.", peerAddrBuf);

        // Close the connection
        TCPDisconnect(conn, /* shouldAbort = */ true);
    }
}

void SessionManager::HandleConnectionClosed(Transport::ActiveTCPConnectionState * conn, CHIP_ERROR conErr)
{
    VerifyOrReturn(conn != nullptr);

    MarkSecureSessionOverTCPForEviction(conn, conErr);

    // TODO: A mechanism to mark an unauthenticated session as unusable when
    // the underlying connection is broken. Issue #32323

    Transport::AppTCPConnectionCallbackCtxt * appTCPConnCbCtxt = conn->mAppState;
    VerifyOrReturn(appTCPConnCbCtxt != nullptr);

    if (appTCPConnCbCtxt->connClosedCb != nullptr)
    {
        appTCPConnCbCtxt->connClosedCb(conn, conErr);
    }
}

CHIP_ERROR SessionManager::TCPConnect(const PeerAddress & peerAddress, Transport::AppTCPConnectionCallbackCtxt * appState,
                                      Transport::ActiveTCPConnectionState ** peerConnState)
{
    char peerAddrBuf[chip::Transport::PeerAddress::kMaxToStringSize];
    peerAddress.ToString(peerAddrBuf);
    if (mTransportMgr != nullptr)
    {
        ChipLogProgress(Inet, "Connecting over TCP with peer at %s.", peerAddrBuf);
        return mTransportMgr->TCPConnect(peerAddress, appState, peerConnState);
    }

    ChipLogError(Inet, "The transport manager is not initialized. Unable to connect to peer at %s.", peerAddrBuf);

    return CHIP_ERROR_INCORRECT_STATE;
}

CHIP_ERROR SessionManager::TCPDisconnect(const PeerAddress & peerAddress)
{
    if (mTransportMgr != nullptr)
    {
        char peerAddrBuf[chip::Transport::PeerAddress::kMaxToStringSize];
        peerAddress.ToString(peerAddrBuf);
        ChipLogProgress(Inet, "Disconnecting TCP connection from peer at %s.", peerAddrBuf);
        mTransportMgr->TCPDisconnect(peerAddress);
    }

    return CHIP_NO_ERROR;
}

void SessionManager::TCPDisconnect(Transport::ActiveTCPConnectionState * conn, bool shouldAbort)
{
    if (mTransportMgr != nullptr && conn != nullptr)
    {
        char peerAddrBuf[chip::Transport::PeerAddress::kMaxToStringSize];
        conn->mPeerAddr.ToString(peerAddrBuf);
        ChipLogProgress(Inet, "Disconnecting TCP connection from peer at %s.", peerAddrBuf);
        mTransportMgr->TCPDisconnect(conn, shouldAbort);

        MarkSecureSessionOverTCPForEviction(conn, CHIP_NO_ERROR);
    }
}
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

void SessionManager::UnauthenticatedMessageDispatch(const PacketHeader & partialPacketHeader,
                                                    const Transport::PeerAddress & peerAddress, System::PacketBufferHandle && msg,
                                                    Transport::MessageTransportContext * ctxt)
{
    MATTER_TRACE_SCOPE("Unauthenticated Message Dispatch", "SessionManager");

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    if (peerAddress.GetTransportType() == Transport::Type::kTcp && ctxt->conn == nullptr)
    {
        ChipLogError(Inet, "Connection object is missing for received message.");
        return;
    }
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

    // Drop unsecured messages with privacy enabled.
    if (partialPacketHeader.HasPrivacyFlag())
    {
        ChipLogError(Inet, "Dropping unauthenticated message with privacy flag set");
        return;
    }

    PacketHeader packetHeader;
    ReturnOnFailure(packetHeader.DecodeAndConsume(msg));

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
        optionalSession = mUnauthenticatedSessions.FindOrAllocateResponder(source.Value(), GetDefaultMRPConfig(), peerAddress);
        if (!optionalSession.HasValue())
        {
            ChipLogError(Inet, "UnauthenticatedSession exhausted");
            return;
        }
    }
    else
    {
        // Assume peer is the responder, we are the initiator.
        optionalSession = mUnauthenticatedSessions.FindInitiator(destination.Value(), peerAddress);
        if (!optionalSession.HasValue())
        {
            ChipLogProgress(Inet, "Received unknown unsecure packet for initiator 0x" ChipLogFormatX64,
                            ChipLogValueX64(destination.Value()));
            return;
        }
    }

    const SessionHandle & session                        = optionalSession.Value();
    Transport::UnauthenticatedSession * unsecuredSession = session->AsUnauthenticatedSession();
    Transport::PeerAddress mutablePeerAddress            = peerAddress;
    CorrectPeerAddressInterfaceID(mutablePeerAddress);
    unsecuredSession->SetPeerAddress(mutablePeerAddress);
    SessionMessageDelegate::DuplicateMessage isDuplicate = SessionMessageDelegate::DuplicateMessage::No;
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    // Associate the unauthenticated session with the connection, if not done already.
    if (peerAddress.GetTransportType() == Transport::Type::kTcp)
    {
        Transport::ActiveTCPConnectionState * sessionConn = unsecuredSession->GetTCPConnection();
        if (sessionConn == nullptr)
        {
            unsecuredSession->SetTCPConnection(ctxt->conn);
        }
        else
        {
            if (sessionConn != ctxt->conn)
            {
                ChipLogError(Inet, "Data received over wrong connection %p. Dropping it!", ctxt->conn);
                return;
            }
        }
    }
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

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
        MATTER_LOG_MESSAGE_RECEIVED(chip::Tracing::IncomingMessageType::kUnauthenticated, &payloadHeader, &packetHeader,
                                    unsecuredSession, &peerAddress, chip::ByteSpan(msg->Start(), msg->TotalLength()));

        CHIP_TRACE_MESSAGE_RECEIVED(payloadHeader, packetHeader, unsecuredSession, peerAddress, msg->Start(), msg->TotalLength());
        mCB->OnMessageReceived(packetHeader, payloadHeader, session, isDuplicate, std::move(msg));
    }
    else
    {
        ChipLogError(Inet, "Received UNSECURED message was not processed.");
    }
}

void SessionManager::SecureUnicastMessageDispatch(const PacketHeader & partialPacketHeader,
                                                  const Transport::PeerAddress & peerAddress, System::PacketBufferHandle && msg,
                                                  Transport::MessageTransportContext * ctxt)
{
    MATTER_TRACE_SCOPE("Secure Unicast Message Dispatch", "SessionManager");

    CHIP_ERROR err = CHIP_NO_ERROR;

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    if (peerAddress.GetTransportType() == Transport::Type::kTcp && ctxt->conn == nullptr)
    {
        ChipLogError(Inet, "Connection object is missing for received message.");
        return;
    }
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

    Optional<SessionHandle> session = mSecureSessions.FindSecureSessionByLocalKey(partialPacketHeader.GetSessionId());
    if (!session.HasValue())
    {
        ChipLogError(Inet, "Data received on an unknown session (LSID=%d). Dropping it!", partialPacketHeader.GetSessionId());
        return;
    }

    Transport::SecureSession * secureSession  = session.Value()->AsSecureSession();
    Transport::PeerAddress mutablePeerAddress = peerAddress;
    CorrectPeerAddressInterfaceID(mutablePeerAddress);
    if (secureSession->GetPeerAddress() != mutablePeerAddress)
    {
        secureSession->SetPeerAddress(mutablePeerAddress);
    }

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    // Associate the secure session with the connection, if not done already.
    if (peerAddress.GetTransportType() == Transport::Type::kTcp)
    {
        Transport::ActiveTCPConnectionState * sessionConn = secureSession->GetTCPConnection();
        if (sessionConn == nullptr)
        {
            secureSession->SetTCPConnection(ctxt->conn);
        }
        else
        {
            if (sessionConn != ctxt->conn)
            {
                ChipLogError(Inet, "Data received over wrong connection %p. Dropping it!", ctxt->conn);
                return;
            }
        }
    }
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

    PayloadHeader payloadHeader;

    // Drop secure unicast messages with privacy enabled.
    if (partialPacketHeader.HasPrivacyFlag())
    {
        ChipLogError(Inet, "Dropping secure unicast message with privacy flag set");
        return;
    }

    PacketHeader packetHeader;
    ReturnOnFailure(packetHeader.DecodeAndConsume(msg));

    SessionMessageDelegate::DuplicateMessage isDuplicate = SessionMessageDelegate::DuplicateMessage::No;

    if (msg.IsNull())
    {
        ChipLogError(Inet, "Secure transport received Unicast NULL packet, discarding");
        return;
    }

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

    if (mCB != nullptr)
    {
        MATTER_LOG_MESSAGE_RECEIVED(chip::Tracing::IncomingMessageType::kSecureUnicast, &payloadHeader, &packetHeader,
                                    secureSession, &peerAddress, chip::ByteSpan(msg->Start(), msg->TotalLength()));
        CHIP_TRACE_MESSAGE_RECEIVED(payloadHeader, packetHeader, secureSession, peerAddress, msg->Start(), msg->TotalLength());

        // Always recompute whether a message is for a commissioning session based on the latest knowledge of
        // the fabric table.
        if (secureSession->IsCASESession())
        {
            secureSession->SetCaseCommissioningSessionStatus(secureSession->GetFabricIndex() ==
                                                             mFabricTable->GetPendingNewFabricIndex());
        }
        mCB->OnMessageReceived(packetHeader, payloadHeader, session.Value(), isDuplicate, std::move(msg));
    }
    else
    {
        ChipLogError(Inet, "Received SECURED message was not processed.");
    }
}

/**
 * Helper function to implement a single attempt to decrypt a groupcast message
 * using the given group key and privacy setting.
 *
 * @param[in] partialPacketHeader The partial packet header with non-obfuscated message fields (result of calling DecodeFixed).
 * @param[out] packetHeaderCopy A copy of the packet header, to be filled with privacy decrypted fields
 * @param[out] payloadHeader The payload header of the decrypted message
 * @param[in] applyPrivacy Whether to apply privacy deobfuscation
 * @param[out] msgCopy A copy of the message, to be filled with the decrypted message
 * @param[in] mac The MAC of the message
 * @param[in] groupContext The group context to use for decryption key material
 *
 * @return true if the message was decrypted successfully
 * @return false if the message could not be decrypted
 */
static bool GroupKeyDecryptAttempt(const PacketHeader & partialPacketHeader, PacketHeader & packetHeaderCopy,
                                   PayloadHeader & payloadHeader, bool applyPrivacy, System::PacketBufferHandle & msgCopy,
                                   const MessageAuthenticationCode & mac,
                                   const Credentials::GroupDataProvider::GroupSession & groupContext)
{
    bool decrypted = false;
    CryptoContext context(groupContext.keyContext);

    if (applyPrivacy)
    {
        // Perform privacy deobfuscation, if applicable.
        uint8_t * privacyHeader = partialPacketHeader.PrivacyHeader(msgCopy->Start());
        size_t privacyLength    = partialPacketHeader.PrivacyHeaderLength();
        if (CHIP_NO_ERROR != context.PrivacyDecrypt(privacyHeader, privacyLength, privacyHeader, partialPacketHeader, mac))
        {
            return false;
        }
    }

    if (packetHeaderCopy.DecodeAndConsume(msgCopy) != CHIP_NO_ERROR)
    {
        ChipLogError(Inet, "Failed to decode Groupcast packet header. Discarding.");
        return false;
    }

    // Optimization to reduce number of decryption attempts
    GroupId groupId = packetHeaderCopy.GetDestinationGroupId().Value();
    if (groupId != groupContext.group_id)
    {
        return false;
    }

    CryptoContext::NonceStorage nonce;
    CryptoContext::BuildNonce(nonce, packetHeaderCopy.GetSecurityFlags(), packetHeaderCopy.GetMessageCounter(),
                              packetHeaderCopy.GetSourceNodeId().Value());
    decrypted = (CHIP_NO_ERROR == SecureMessageCodec::Decrypt(context, nonce, payloadHeader, packetHeaderCopy, msgCopy));

    return decrypted;
}

void SessionManager::SecureGroupMessageDispatch(const PacketHeader & partialPacketHeader,
                                                const Transport::PeerAddress & peerAddress, System::PacketBufferHandle && msg)
{
    MATTER_TRACE_SCOPE("Group Message Dispatch", "SessionManager");

    PayloadHeader payloadHeader;
    PacketHeader packetHeaderCopy; /// Packet header decoded per group key, with privacy decrypted fields
    System::PacketBufferHandle msgCopy;
    Credentials::GroupDataProvider * groups = Credentials::GetGroupDataProvider();
    VerifyOrReturn(nullptr != groups);
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (!partialPacketHeader.HasDestinationGroupId())
    {
        return; // malformed packet
    }

    // Check if Message Header is valid first
    if (!(partialPacketHeader.IsValidMCSPMsg() || partialPacketHeader.IsValidGroupMsg()))
    {
        ChipLogError(Inet, "Invalid condition found in packet header");
        return;
    }

    // Trial decryption with GroupDataProvider
    Credentials::GroupDataProvider::GroupSession groupContext;

    AutoRelease<Credentials::GroupDataProvider::GroupSessionIterator> iter(
        groups->IterateGroupSessions(partialPacketHeader.GetSessionId()));

    if (iter.IsNull())
    {
        ChipLogError(Inet, "Failed to retrieve Groups iterator. Discarding everything");
        return;
    }

    // Extract MIC from the end of the message.
    uint8_t * data     = msg->Start();
    size_t len         = msg->DataLength();
    uint16_t footerLen = partialPacketHeader.MICTagLength();
    VerifyOrReturn(footerLen <= len);

    uint16_t taglen = 0;
    MessageAuthenticationCode mac;
    ReturnOnFailure(mac.Decode(partialPacketHeader, &data[len - footerLen], footerLen, &taglen));
    VerifyOrReturn(taglen == footerLen);

    bool decrypted = false;
    while (!decrypted && iter->Next(groupContext))
    {
        CryptoContext context(groupContext.keyContext);
        msgCopy = msg.CloneData();
        if (msgCopy.IsNull())
        {
            ChipLogError(Inet, "Failed to clone Groupcast message buffer. Discarding.");
            return;
        }

        bool privacy = partialPacketHeader.HasPrivacyFlag();
        decrypted =
            GroupKeyDecryptAttempt(partialPacketHeader, packetHeaderCopy, payloadHeader, privacy, msgCopy, mac, groupContext);

#if CHIP_CONFIG_PRIVACY_ACCEPT_NONSPEC_SVE2
        if (privacy && !decrypted)
        {
            // Try processing the P=1 message again without privacy as a work-around for invalid early-SVE2 nodes.
            msgCopy = msg.CloneData();
            if (msgCopy.IsNull())
            {
                ChipLogError(Inet, "Failed to clone Groupcast message buffer. Discarding.");
                return;
            }
            decrypted =
                GroupKeyDecryptAttempt(partialPacketHeader, packetHeaderCopy, payloadHeader, false, msgCopy, mac, groupContext);
        }
#endif // CHIP_CONFIG_PRIVACY_ACCEPT_NONSPEC_SVE2
    }
    iter.Release();

    if (!decrypted)
    {
        ChipLogError(Inet, "Failed to decrypt group message. Discarding everything");
        return;
    }
    msg = std::move(msgCopy);

    // MCSP check
    if (packetHeaderCopy.IsValidMCSPMsg())
    {
        // TODO: When MCSP Msg, create Secure Session instead of a Group session

        // TODO
        // if (packetHeaderCopy.GetDestinationNodeId().Value() == ThisDeviceNodeID)
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
        gGroupPeerTable->FindOrAddPeer(groupContext.fabric_index, packetHeaderCopy.GetSourceNodeId().Value(),
                                       packetHeaderCopy.IsSecureSessionControlMsg(), counter))
    {
        if (Credentials::GroupDataProvider::SecurityPolicy::kTrustFirst == groupContext.security_policy)
        {
            err = counter->VerifyOrTrustFirstGroup(packetHeaderCopy.GetMessageCounter());
        }
        else
        {

            // TODO support cache and sync with MCSP. Issue  #11689
            ChipLogError(Inet, "Received Group Msg with key policy Cache and Sync, but MCSP is not implemented");
            return;

            // cache and sync
            // err = counter->VerifyGroup(packetHeaderCopy.GetMessageCounter());
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

    counter->CommitGroup(packetHeaderCopy.GetMessageCounter());

    if (mCB != nullptr)
    {
        // TODO : When MCSP is done, clean up session creation logic
        Transport::IncomingGroupSession groupSession(groupContext.group_id, groupContext.fabric_index,
                                                     packetHeaderCopy.GetSourceNodeId().Value());

        MATTER_LOG_MESSAGE_RECEIVED(chip::Tracing::IncomingMessageType::kGroupMessage, &payloadHeader, &packetHeaderCopy,
                                    &groupSession, &peerAddress, chip::ByteSpan(msg->Start(), msg->TotalLength()));

        CHIP_TRACE_MESSAGE_RECEIVED(payloadHeader, packetHeaderCopy, &groupSession, peerAddress, msg->Start(), msg->TotalLength());
        mCB->OnMessageReceived(packetHeaderCopy, payloadHeader, SessionHandle(groupSession),
                               SessionMessageDelegate::DuplicateMessage::No, std::move(msg));
    }
    else
    {
        ChipLogError(Inet, "Received GROUP message was not processed.");
    }
}

Optional<SessionHandle> SessionManager::FindSecureSessionForNode(ScopedNodeId peerNodeId,
                                                                 const Optional<Transport::SecureSession::Type> & type,
                                                                 TransportPayloadCapability transportPayloadCapability)
{
    SecureSession * mrpSession = nullptr;
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    SecureSession * tcpSession = nullptr;
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

    mSecureSessions.ForEachSession([&peerNodeId, &type, &mrpSession,
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
                                    &tcpSession,
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT
                                    &transportPayloadCapability](auto session) {
        if (session->IsActiveSession() && session->GetPeer() == peerNodeId &&
            (!type.HasValue() || type.Value() == session->GetSecureSessionType()))
        {
            if (transportPayloadCapability == TransportPayloadCapability::kMRPOrTCPCompatiblePayload ||
                transportPayloadCapability == TransportPayloadCapability::kLargePayload)
            {
#if INET_CONFIG_ENABLE_TCP_ENDPOINT
                // Set up a TCP transport based session as standby
                if ((tcpSession == nullptr || tcpSession->GetLastPeerActivityTime() < session->GetLastPeerActivityTime()) &&
                    session->GetTCPConnection() != nullptr)
                {
                    tcpSession = session;
                }
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT
            }

            if ((mrpSession == nullptr) || (mrpSession->GetLastPeerActivityTime() < session->GetLastPeerActivityTime()))
            {
                mrpSession = session;
            }
        }

        return Loop::Continue;
    });

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
    if (transportPayloadCapability == TransportPayloadCapability::kLargePayload)
    {
        return tcpSession != nullptr ? MakeOptional<SessionHandle>(*tcpSession) : Optional<SessionHandle>::Missing();
    }

    if (transportPayloadCapability == TransportPayloadCapability::kMRPOrTCPCompatiblePayload)
    {
        // If MRP-based session is available, use it.
        if (mrpSession != nullptr)
        {
            return MakeOptional<SessionHandle>(*mrpSession);
        }

        // Otherwise, look for a tcp-based session
        if (tcpSession != nullptr)
        {
            return MakeOptional<SessionHandle>(*tcpSession);
        }

        return Optional<SessionHandle>::Missing();
    }
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

    return mrpSession != nullptr ? MakeOptional<SessionHandle>(*mrpSession) : Optional<SessionHandle>::Missing();
}

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
void SessionManager::MarkSecureSessionOverTCPForEviction(Transport::ActiveTCPConnectionState * conn, CHIP_ERROR conErr)
{
    // Mark the corresponding secure sessions for eviction
    mSecureSessions.ForEachSession([&](auto session) {
        if (session->IsActiveSession() && session->GetTCPConnection() == conn)
        {
            SessionHandle handle(*session);
            // Notify the SessionConnection delegate of the connection
            // closure.
            if (mConnDelegate != nullptr)
            {
                mConnDelegate->OnTCPConnectionClosed(handle, conErr);
            }

            // Dis-associate the connection from session by setting it to a
            // nullptr.
            session->SetTCPConnection(nullptr);
            // Mark session for eviction.
            session->MarkForEviction();
        }

        return Loop::Continue;
    });
}
#endif // INET_CONFIG_ENABLE_TCP_ENDPOINT

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
