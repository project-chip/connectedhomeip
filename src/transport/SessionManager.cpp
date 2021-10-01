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

#include <app/util/basic-types.h>
#include <lib/core/CHIPKeyIds.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/secure_channel/Constants.h>
#include <transport/PairingSession.h>
#include <transport/SecureMessageCodec.h>
#include <transport/TransportMgr.h>

#include <inttypes.h>

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
                                Transport::MessageCounterManagerInterface * messageCounterManager)
{
    VerifyOrReturnError(mState == State::kNotReady, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(transportMgr != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    mState                 = State::kInitialized;
    mSystemLayer           = systemLayer;
    mTransportMgr          = transportMgr;
    mMessageCounterManager = messageCounterManager;

    mGlobalEncryptedMessageCounter.Init();

    ScheduleExpiryTimer();

    mTransportMgr->SetSessionManager(this);

    return CHIP_NO_ERROR;
}

void SessionManager::Shutdown()
{
    CancelExpiryTimer();

    mMessageCounterManager = nullptr;

    mState        = State::kNotReady;
    mSystemLayer  = nullptr;
    mTransportMgr = nullptr;
    mCB           = nullptr;
}

CHIP_ERROR SessionManager::PrepareMessage(SessionHandle session, PayloadHeader & payloadHeader,
                                          System::PacketBufferHandle && message, EncryptedPacketBufferHandle & preparedMessage)
{
    PacketHeader packetHeader;
    if (IsControlMessage(payloadHeader))
    {
        packetHeader.SetSecureSessionControlMsg(true);
    }

#if CHIP_PROGRESS_LOGGING
    NodeId destination;
#endif // CHIP_PROGRESS_LOGGING
    if (session.IsSecure())
    {
        SecureSession * state = GetSecureSession(session);
        if (state == nullptr)
        {
            return CHIP_ERROR_NOT_CONNECTED;
        }

        MessageCounter & counter = GetSendCounterForPacket(payloadHeader, *state);
        ReturnErrorOnFailure(SecureMessageCodec::Encode(state, payloadHeader, packetHeader, message, counter));

#if CHIP_PROGRESS_LOGGING
        destination = state->GetPeerNodeId();
#endif // CHIP_PROGRESS_LOGGING
    }
    else
    {
        ReturnErrorOnFailure(payloadHeader.EncodeBeforeData(message));

        MessageCounter & counter = session.GetUnauthenticatedSession()->GetLocalMessageCounter();
        uint32_t messageCounter  = counter.Value();
        ReturnErrorOnFailure(counter.Advance());

        packetHeader.SetMessageCounter(messageCounter);

#if CHIP_PROGRESS_LOGGING
        destination = kUndefinedNodeId;
#endif // CHIP_PROGRESS_LOGGING
    }

    ChipLogProgress(Inet,
                    "Prepared %s message %p to 0x" ChipLogFormatX64 " of type " ChipLogFormatMessageType
                    " and protocolId " ChipLogFormatProtocolId " on exchange " ChipLogFormatExchangeId
                    " with MessageCounter:" ChipLogFormatMessageCounter ".",
                    session.IsSecure() ? "encrypted" : "plaintext", &preparedMessage, ChipLogValueX64(destination),
                    payloadHeader.GetMessageType(), ChipLogValueProtocolId(payloadHeader.GetProtocolID()),
                    ChipLogValueExchangeIdFromSentHeader(payloadHeader), packetHeader.GetMessageCounter());

    ReturnErrorOnFailure(packetHeader.EncodeBeforeData(message));
    preparedMessage = EncryptedPacketBufferHandle::MarkEncrypted(std::move(message));

    return CHIP_NO_ERROR;
}

CHIP_ERROR SessionManager::SendPreparedMessage(SessionHandle session, const EncryptedPacketBufferHandle & preparedMessage)
{
    VerifyOrReturnError(mState == State::kInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(!preparedMessage.IsNull(), CHIP_ERROR_INVALID_ARGUMENT);

    const Transport::PeerAddress * destination;

    if (session.IsSecure())
    {
        // Find an active connection to the specified peer node
        SecureSession * state = GetSecureSession(session);
        if (state == nullptr)
        {
            ChipLogError(Inet, "Secure transport could not find a valid PeerConnection");
            return CHIP_ERROR_NOT_CONNECTED;
        }

        // This marks any connection where we send data to as 'active'
        mPeerConnections.MarkConnectionActive(state);

        destination = &state->GetPeerAddress();

        ChipLogProgress(Inet,
                        "Sending %s msg %p with MessageCounter:" ChipLogFormatMessageCounter " to 0x" ChipLogFormatX64
                        " at monotonic time: %" PRId64 " msec",
                        "encrypted", &preparedMessage, preparedMessage.GetMessageCounter(), ChipLogValueX64(state->GetPeerNodeId()),
                        System::SystemClock().GetMonotonicMilliseconds());
    }
    else
    {
        auto unauthenticated = session.GetUnauthenticatedSession();
        mUnauthenticatedSessions.MarkSessionActive(unauthenticated.Get());
        destination = &unauthenticated->GetPeerAddress();

        ChipLogProgress(Inet,
                        "Sending %s msg %p with MessageCounter:" ChipLogFormatMessageCounter " to 0x" ChipLogFormatX64
                        " at monotonic time: %" PRId64 " msec",
                        "plaintext", &preparedMessage, preparedMessage.GetMessageCounter(), ChipLogValueX64(kUndefinedNodeId),
                        System::SystemClock().GetMonotonicMilliseconds());
    }

    PacketBufferHandle msgBuf = preparedMessage.CastToWritable();
    VerifyOrReturnError(!msgBuf.IsNull(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(!msgBuf->HasChainedBuffer(), CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    if (mTransportMgr != nullptr)
    {
        return mTransportMgr->SendMessage(*destination, std::move(msgBuf));
    }
    else
    {
        ChipLogError(Inet, "The transport manager is not initialized. Unable to send the message");
        return CHIP_ERROR_INCORRECT_STATE;
    }
}

void SessionManager::ExpirePairing(SessionHandle session)
{
    SecureSession * state = GetSecureSession(session);
    if (state != nullptr)
    {
        mPeerConnections.MarkConnectionExpired(
            state, [this](const Transport::SecureSession & state1) { HandleConnectionExpired(state1); });
    }
}

void SessionManager::ExpireAllPairings(NodeId peerNodeId, FabricIndex fabric)
{
    SecureSession * state = mPeerConnections.FindPeerConnectionState(peerNodeId, nullptr);
    while (state != nullptr)
    {
        if (fabric == state->GetFabricIndex())
        {
            mPeerConnections.MarkConnectionExpired(
                state, [this](const Transport::SecureSession & state1) { HandleConnectionExpired(state1); });
            state = mPeerConnections.FindPeerConnectionState(peerNodeId, nullptr);
        }
        else
        {
            state = mPeerConnections.FindPeerConnectionState(peerNodeId, state);
        }
    }
}

void SessionManager::ExpireAllPairingsForFabric(FabricIndex fabric)
{
    ChipLogDetail(Inet, "Expiring all connections for fabric %d!!", fabric);
    SecureSession * state = mPeerConnections.FindPeerConnectionStateByFabric(fabric);
    while (state != nullptr)
    {
        mPeerConnections.MarkConnectionExpired(
            state, [this](const Transport::SecureSession & state1) { HandleConnectionExpired(state1); });
        state = mPeerConnections.FindPeerConnectionStateByFabric(fabric);
    }
}

CHIP_ERROR SessionManager::NewPairing(const Optional<Transport::PeerAddress> & peerAddr, NodeId peerNodeId,
                                      PairingSession * pairing, CryptoContext::SessionRole direction, FabricIndex fabric)
{
    uint16_t peerSessionId  = pairing->GetPeerSessionId();
    uint16_t localSessionId = pairing->GetLocalSessionId();
    SecureSession * state =
        mPeerConnections.FindPeerConnectionStateByLocalKey(Optional<NodeId>::Value(peerNodeId), localSessionId, nullptr);

    // Find any existing connection with the same local key ID
    if (state)
    {
        mPeerConnections.MarkConnectionExpired(
            state, [this](const Transport::SecureSession & state1) { HandleConnectionExpired(state1); });
    }

    ChipLogDetail(Inet, "New secure session created for device 0x" ChipLogFormatX64 ", key %d!!", ChipLogValueX64(peerNodeId),
                  peerSessionId);
    state = nullptr;
    ReturnErrorOnFailure(
        mPeerConnections.CreateNewPeerConnectionState(Optional<NodeId>::Value(peerNodeId), peerSessionId, localSessionId, &state));
    ReturnErrorCodeIf(state == nullptr, CHIP_ERROR_NO_MEMORY);

    state->SetFabricIndex(fabric);

    if (peerAddr.HasValue() && peerAddr.Value().GetIPAddress() != Inet::IPAddress::Any)
    {
        state->SetPeerAddress(peerAddr.Value());
    }
    else if (peerAddr.HasValue() && peerAddr.Value().GetTransportType() == Transport::Type::kBle)
    {
        state->SetPeerAddress(peerAddr.Value());
    }
    else if (peerAddr.HasValue() &&
             (peerAddr.Value().GetTransportType() == Transport::Type::kTcp ||
              peerAddr.Value().GetTransportType() == Transport::Type::kUdp))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ReturnErrorOnFailure(pairing->DeriveSecureSession(state->GetCryptoContext(), direction));

    if (mCB != nullptr)
    {
        state->GetSessionMessageCounter().GetPeerMessageCounter().SetCounter(pairing->GetPeerCounter());
        mCB->OnNewConnection(SessionHandle(state->GetPeerNodeId(), state->GetLocalSessionId(), state->GetPeerSessionId(), fabric));
    }

    return CHIP_NO_ERROR;
}

void SessionManager::ScheduleExpiryTimer()
{
    CHIP_ERROR err =
        mSystemLayer->StartTimer(CHIP_PEER_CONNECTION_TIMEOUT_CHECK_FREQUENCY_MS, SessionManager::ExpiryTimerCallback, this);

    VerifyOrDie(err == CHIP_NO_ERROR);
}

void SessionManager::CancelExpiryTimer()
{
    if (mSystemLayer != nullptr)
    {
        mSystemLayer->CancelTimer(SessionManager::ExpiryTimerCallback, this);
    }
}

void SessionManager::OnMessageReceived(const PeerAddress & peerAddress, System::PacketBufferHandle && msg)
{
    PacketHeader packetHeader;

    ReturnOnFailure(packetHeader.DecodeAndConsume(msg));

    if (packetHeader.GetFlags().Has(Header::FlagValues::kEncryptedMessage))
    {
        SecureMessageDispatch(packetHeader, peerAddress, std::move(msg));
    }
    else
    {
        MessageDispatch(packetHeader, peerAddress, std::move(msg));
    }
}

void SessionManager::MessageDispatch(const PacketHeader & packetHeader, const Transport::PeerAddress & peerAddress,
                                     System::PacketBufferHandle && msg)
{
    Transport::UnauthenticatedSession * session = mUnauthenticatedSessions.FindOrAllocateEntry(peerAddress);
    if (session == nullptr)
    {
        ChipLogError(Inet, "UnauthenticatedSession exhausted");
        return;
    }

    SessionManagerDelegate::DuplicateMessage isDuplicate = SessionManagerDelegate::DuplicateMessage::No;

    // Verify message counter
    CHIP_ERROR err = session->GetPeerMessageCounter().VerifyOrTrustFirst(packetHeader.GetMessageCounter());
    if (err == CHIP_ERROR_DUPLICATE_MESSAGE_RECEIVED)
    {
        isDuplicate = SessionManagerDelegate::DuplicateMessage::Yes;
        err         = CHIP_NO_ERROR;
    }
    VerifyOrDie(err == CHIP_NO_ERROR);

    mUnauthenticatedSessions.MarkSessionActive(*session);

    PayloadHeader payloadHeader;
    ReturnOnFailure(payloadHeader.DecodeAndConsume(msg));

    if (isDuplicate == SessionManagerDelegate::DuplicateMessage::Yes)
    {
        ChipLogDetail(Inet,
                      "Received a duplicate message with MessageCounter:" ChipLogFormatMessageCounter
                      " on exchange " ChipLogFormatExchangeId,
                      packetHeader.GetMessageCounter(), ChipLogValueExchangeIdFromSentHeader(payloadHeader));
    }

    session->GetPeerMessageCounter().Commit(packetHeader.GetMessageCounter());

    if (mCB != nullptr)
    {
        mCB->OnMessageReceived(packetHeader, payloadHeader, SessionHandle(Transport::UnauthenticatedSessionHandle(*session)),
                               peerAddress, isDuplicate, std::move(msg));
    }
}

void SessionManager::SecureMessageDispatch(const PacketHeader & packetHeader, const Transport::PeerAddress & peerAddress,
                                           System::PacketBufferHandle && msg)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    SecureSession * state = mPeerConnections.FindPeerConnectionState(packetHeader.GetSessionId(), nullptr);

    PayloadHeader payloadHeader;

    SessionManagerDelegate::DuplicateMessage isDuplicate = SessionManagerDelegate::DuplicateMessage::No;

    VerifyOrExit(!msg.IsNull(), ChipLogError(Inet, "Secure transport received NULL packet, discarding"));

    if (state == nullptr)
    {
        ChipLogError(Inet, "Data received on an unknown connection (%d). Dropping it!!", packetHeader.GetSessionId());
        ExitNow(err = CHIP_ERROR_KEY_NOT_FOUND_FROM_PEER);
    }

    // Verify message counter
    if (packetHeader.GetFlags().Has(Header::FlagValues::kSecureSessionControlMessage))
    {
        // TODO: control message counter is not implemented yet
    }
    else
    {
        if (!state->GetSessionMessageCounter().GetPeerMessageCounter().IsSynchronized())
        {
            // Queue and start message sync procedure
            err = mMessageCounterManager->QueueReceivedMessageAndStartSync(
                packetHeader,
                SessionHandle(state->GetPeerNodeId(), state->GetLocalSessionId(), state->GetPeerSessionId(),
                              state->GetFabricIndex()),
                state, peerAddress, std::move(msg));

            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Inet,
                             "Message counter synchronization for received message, failed to "
                             "QueueReceivedMessageAndStartSync, err = %" CHIP_ERROR_FORMAT,
                             err.Format());
            }
            else
            {
                ChipLogDetail(Inet, "Received message have been queued due to peer counter is not synced");
            }

            return;
        }

        err = state->GetSessionMessageCounter().GetPeerMessageCounter().Verify(packetHeader.GetMessageCounter());
        if (err == CHIP_ERROR_DUPLICATE_MESSAGE_RECEIVED)
        {
            isDuplicate = SessionManagerDelegate::DuplicateMessage::Yes;
            err         = CHIP_NO_ERROR;
        }
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Inet, "Message counter verify failed, err = %" CHIP_ERROR_FORMAT, err.Format());
        }
        SuccessOrExit(err);
    }

    mPeerConnections.MarkConnectionActive(state);

    // Decode the message
    VerifyOrExit(CHIP_NO_ERROR == SecureMessageCodec::Decode(state, payloadHeader, packetHeader, msg),
                 ChipLogError(Inet, "Secure transport received message, but failed to decode it, discarding"));

    if (isDuplicate == SessionManagerDelegate::DuplicateMessage::Yes && !payloadHeader.NeedsAck())
    {
        ChipLogDetail(Inet,
                      "Received a duplicate message with MessageCounter:" ChipLogFormatMessageCounter
                      " on exchange " ChipLogFormatExchangeId,
                      packetHeader.GetMessageCounter(), ChipLogValueExchangeIdFromSentHeader(payloadHeader));
        if (!payloadHeader.NeedsAck())
        {
            // If it's a duplicate message, but doesn't require an ack, let's drop it right here to save CPU
            // cycles on further message processing.
            ExitNow(err = CHIP_NO_ERROR);
        }
    }

    if (packetHeader.GetFlags().Has(Header::FlagValues::kSecureSessionControlMessage))
    {
        // TODO: control message counter is not implemented yet
    }
    else
    {
        state->GetSessionMessageCounter().GetPeerMessageCounter().Commit(packetHeader.GetMessageCounter());
    }

    // TODO: once mDNS address resolution is available reconsider if this is required
    // This updates the peer address once a packet is received from a new address
    // and serves as a way to auto-detect peer changing IPs.
    if (state->GetPeerAddress() != peerAddress)
    {
        state->SetPeerAddress(peerAddress);
    }

    if (mCB != nullptr)
    {
        SessionHandle session(state->GetPeerNodeId(), state->GetLocalSessionId(), state->GetPeerSessionId(),
                              state->GetFabricIndex());
        mCB->OnMessageReceived(packetHeader, payloadHeader, session, peerAddress, isDuplicate, std::move(msg));
    }

exit:
    if (err != CHIP_NO_ERROR && mCB != nullptr)
    {
        mCB->OnReceiveError(err, peerAddress);
    }
}

void SessionManager::HandleConnectionExpired(const Transport::SecureSession & state)
{
    ChipLogDetail(Inet, "Marking old secure session for device 0x" ChipLogFormatX64 " as expired",
                  ChipLogValueX64(state.GetPeerNodeId()));

    if (mCB != nullptr)
    {
        mCB->OnConnectionExpired(
            SessionHandle(state.GetPeerNodeId(), state.GetLocalSessionId(), state.GetPeerSessionId(), state.GetFabricIndex()));
    }

    mTransportMgr->Disconnect(state.GetPeerAddress());
}

void SessionManager::ExpiryTimerCallback(System::Layer * layer, void * param)
{
    SessionManager * mgr = reinterpret_cast<SessionManager *>(param);
#if CHIP_CONFIG_SESSION_REKEYING
    // TODO(#2279): session expiration is currently disabled until rekeying is supported
    // the #ifdef should be removed after that.
    mgr->mPeerConnections.ExpireInactiveConnections(
        CHIP_PEER_CONNECTION_TIMEOUT_MS, [this](const Transport::SecureSession & state1) { HandleConnectionExpired(state1); });
#endif
    mgr->ScheduleExpiryTimer(); // re-schedule the oneshot timer
}

SecureSession * SessionManager::GetSecureSession(SessionHandle session)
{
    return mPeerConnections.FindPeerConnectionStateByLocalKey(Optional<NodeId>::Value(session.mPeerNodeId),
                                                              session.mLocalSessionId.ValueOr(0), nullptr);
}

} // namespace chip
