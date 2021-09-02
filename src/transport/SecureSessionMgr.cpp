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

#include "SecureSessionMgr.h"

#include <inttypes.h>
#include <string.h>

#include <app/util/basic-types.h>
#include <lib/core/CHIPKeyIds.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/secure_channel/Constants.h>
#include <transport/FabricTable.h>
#include <transport/SecureMessageCodec.h>
#include <transport/TransportMgr.h>

#include <inttypes.h>

namespace chip {

using System::PacketBufferHandle;
using Transport::PeerAddress;
using Transport::PeerConnectionState;

uint32_t EncryptedPacketBufferHandle::GetMsgId() const
{
    PacketHeader header;
    uint16_t headerSize = 0;
    CHIP_ERROR err      = header.Decode((*this)->Start(), (*this)->DataLength(), &headerSize);

    if (err == CHIP_NO_ERROR)
    {
        return header.GetMessageId();
    }

    ChipLogError(Inet, "Failed to decode EncryptedPacketBufferHandle header with error: %s", ErrorStr(err));

    return 0;
}

SecureSessionMgr::SecureSessionMgr() : mState(State::kNotReady) {}

SecureSessionMgr::~SecureSessionMgr() {}

CHIP_ERROR SecureSessionMgr::Init(System::Layer * systemLayer, TransportMgrBase * transportMgr, Transport::FabricTable * fabrics,
                                  Transport::MessageCounterManagerInterface * messageCounterManager)
{
    VerifyOrReturnError(mState == State::kNotReady, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(transportMgr != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    mState                 = State::kInitialized;
    mSystemLayer           = systemLayer;
    mTransportMgr          = transportMgr;
    mFabrics               = fabrics;
    mMessageCounterManager = messageCounterManager;

    mGlobalEncryptedMessageCounter.Init();

    ScheduleExpiryTimer();

    mTransportMgr->SetSecureSessionMgr(this);

    return CHIP_NO_ERROR;
}

void SecureSessionMgr::Shutdown()
{
    CancelExpiryTimer();

    mMessageCounterManager = nullptr;

    mState        = State::kNotReady;
    mSystemLayer  = nullptr;
    mTransportMgr = nullptr;
    mFabrics      = nullptr;
    mCB           = nullptr;
}

CHIP_ERROR SecureSessionMgr::BuildEncryptedMessagePayload(SessionHandle session, PayloadHeader & payloadHeader,
                                                          System::PacketBufferHandle && msgBuf,
                                                          EncryptedPacketBufferHandle & encryptedMessage)
{
    PacketHeader packetHeader;
    if (IsControlMessage(payloadHeader))
    {
        packetHeader.SetSecureSessionControlMsg(true);
    }

    PeerConnectionState * state = GetPeerConnectionState(session);
    if (state == nullptr)
    {
        return CHIP_ERROR_NOT_CONNECTED;
    }

    MessageCounter & counter = GetSendCounterForPacket(payloadHeader, *state);
    ReturnErrorOnFailure(SecureMessageCodec::Encode(state, payloadHeader, packetHeader, msgBuf, counter));

    ReturnErrorOnFailure(packetHeader.EncodeBeforeData(msgBuf));

    encryptedMessage = EncryptedPacketBufferHandle::MarkEncrypted(std::move(msgBuf));
    ChipLogProgress(Inet, "Encrypted message %p to 0x" ChipLogFormatX64 " of type %d and protocolId %" PRIu32 " on exchange %d.",
                    &encryptedMessage, ChipLogValueX64(state->GetPeerNodeId()), payloadHeader.GetMessageType(),
                    payloadHeader.GetProtocolID().ToFullyQualifiedSpecForm(), payloadHeader.GetExchangeID());

    return CHIP_NO_ERROR;
}

CHIP_ERROR SecureSessionMgr::SendPreparedMessage(SessionHandle session, const EncryptedPacketBufferHandle & preparedMessage)
{
    CHIP_ERROR err              = CHIP_NO_ERROR;
    PeerConnectionState * state = nullptr;
    PacketBufferHandle msgBuf;

    VerifyOrExit(mState == State::kInitialized, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(!preparedMessage.IsNull(), err = CHIP_ERROR_INVALID_ARGUMENT);
    msgBuf = preparedMessage.CastToWritable();
    VerifyOrExit(!msgBuf.IsNull(), err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(!msgBuf->HasChainedBuffer(), err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    // Find an active connection to the specified peer node
    state = GetPeerConnectionState(session);
    VerifyOrExit(state != nullptr, err = CHIP_ERROR_NOT_CONNECTED);

    // This marks any connection where we send data to as 'active'
    mPeerConnections.MarkConnectionActive(state);

    ChipLogProgress(Inet, "Sending msg %p to 0x" ChipLogFormatX64 " at utc time: %" PRId64 " msec", &preparedMessage,
                    ChipLogValueX64(state->GetPeerNodeId()), System::Clock::GetMonotonicMilliseconds());

    if (mTransportMgr != nullptr)
    {
        ChipLogProgress(Inet, "Sending secure msg on generic transport");
        err = mTransportMgr->SendMessage(state->GetPeerAddress(), std::move(msgBuf));
    }
    else
    {
        ChipLogError(Inet, "The transport manager is not initialized. Unable to send the message");
        err = CHIP_ERROR_INCORRECT_STATE;
    }
    ChipLogProgress(Inet, "Secure msg send status %s", ErrorStr(err));
    SuccessOrExit(err);

exit:
    if (!msgBuf.IsNull())
    {
        const char * errStr = ErrorStr(err);
        if (state == nullptr)
        {
            ChipLogError(Inet, "Secure transport could not find a valid PeerConnection: %s", errStr);
        }
    }

    return err;
}

void SecureSessionMgr::ExpirePairing(SessionHandle session)
{
    PeerConnectionState * state = GetPeerConnectionState(session);
    if (state != nullptr)
    {
        mPeerConnections.MarkConnectionExpired(
            state, [this](const Transport::PeerConnectionState & state1) { HandleConnectionExpired(state1); });
    }
}

void SecureSessionMgr::ExpireAllPairings(NodeId peerNodeId, FabricIndex fabric)
{
    PeerConnectionState * state = mPeerConnections.FindPeerConnectionState(peerNodeId, nullptr);
    while (state != nullptr)
    {
        if (fabric == state->GetFabricIndex())
        {
            mPeerConnections.MarkConnectionExpired(
                state, [this](const Transport::PeerConnectionState & state1) { HandleConnectionExpired(state1); });
            state = mPeerConnections.FindPeerConnectionState(peerNodeId, nullptr);
        }
        else
        {
            state = mPeerConnections.FindPeerConnectionState(peerNodeId, state);
        }
    }
}

CHIP_ERROR SecureSessionMgr::NewPairing(const Optional<Transport::PeerAddress> & peerAddr, NodeId peerNodeId,
                                        PairingSession * pairing, SecureSession::SessionRole direction, FabricIndex fabric)
{
    uint16_t peerKeyId  = pairing->GetPeerKeyId();
    uint16_t localKeyId = pairing->GetLocalKeyId();
    PeerConnectionState * state =
        mPeerConnections.FindPeerConnectionStateByLocalKey(Optional<NodeId>::Value(peerNodeId), localKeyId, nullptr);

    // Find any existing connection with the same local key ID
    if (state)
    {
        mPeerConnections.MarkConnectionExpired(
            state, [this](const Transport::PeerConnectionState & state1) { HandleConnectionExpired(state1); });
    }

    ChipLogDetail(Inet, "New secure session created for device 0x" ChipLogFormatX64 ", key %d!!", ChipLogValueX64(peerNodeId),
                  peerKeyId);
    state = nullptr;
    ReturnErrorOnFailure(
        mPeerConnections.CreateNewPeerConnectionState(Optional<NodeId>::Value(peerNodeId), peerKeyId, localKeyId, &state));
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

    ReturnErrorOnFailure(pairing->DeriveSecureSession(state->GetSecureSession(), direction));

    if (mCB != nullptr)
    {
        state->GetSessionMessageCounter().GetPeerMessageCounter().SetCounter(pairing->GetPeerCounter());
        mCB->OnNewConnection(SessionHandle(state->GetPeerNodeId(), state->GetLocalKeyID(), state->GetPeerKeyID(), fabric));
    }

    return CHIP_NO_ERROR;
}

void SecureSessionMgr::ScheduleExpiryTimer()
{
    CHIP_ERROR err =
        mSystemLayer->StartTimer(CHIP_PEER_CONNECTION_TIMEOUT_CHECK_FREQUENCY_MS, SecureSessionMgr::ExpiryTimerCallback, this);

    VerifyOrDie(err == CHIP_NO_ERROR);
}

void SecureSessionMgr::CancelExpiryTimer()
{
    if (mSystemLayer != nullptr)
    {
        mSystemLayer->CancelTimer(SecureSessionMgr::ExpiryTimerCallback, this);
    }
}

void SecureSessionMgr::OnMessageReceived(const PeerAddress & peerAddress, System::PacketBufferHandle && msg)
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

void SecureSessionMgr::MessageDispatch(const PacketHeader & packetHeader, const Transport::PeerAddress & peerAddress,
                                       System::PacketBufferHandle && msg)
{
    if (mCB != nullptr)
    {
        PayloadHeader payloadHeader;
        ReturnOnFailure(payloadHeader.DecodeAndConsume(msg));
        mCB->OnMessageReceived(packetHeader, payloadHeader, SessionHandle::TemporaryUnauthenticatedSession(), peerAddress,
                               SecureSessionMgrDelegate::DuplicateMessage::No, std::move(msg));
    }
}

void SecureSessionMgr::SecureMessageDispatch(const PacketHeader & packetHeader, const Transport::PeerAddress & peerAddress,
                                             System::PacketBufferHandle && msg)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    PeerConnectionState * state = mPeerConnections.FindPeerConnectionState(packetHeader.GetEncryptionKeyID(), nullptr);

    PayloadHeader payloadHeader;

    SecureSessionMgrDelegate::DuplicateMessage isDuplicate = SecureSessionMgrDelegate::DuplicateMessage::No;

    VerifyOrExit(!msg.IsNull(), ChipLogError(Inet, "Secure transport received NULL packet, discarding"));

    if (state == nullptr)
    {
        ChipLogError(Inet, "Data received on an unknown connection (%d). Dropping it!!", packetHeader.GetEncryptionKeyID());
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
                SessionHandle(state->GetPeerNodeId(), state->GetLocalKeyID(), state->GetPeerKeyID(), state->GetFabricIndex()),
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

        err = state->GetSessionMessageCounter().GetPeerMessageCounter().Verify(packetHeader.GetMessageId());
        if (err == CHIP_ERROR_DUPLICATE_MESSAGE_RECEIVED)
        {
            ChipLogDetail(Inet, "Received a duplicate message");
            isDuplicate = SecureSessionMgrDelegate::DuplicateMessage::Yes;
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

    if (isDuplicate == SecureSessionMgrDelegate::DuplicateMessage::Yes && !payloadHeader.NeedsAck())
    {
        // If it's a duplicate message, but doesn't require an ack, let's drop it right here to save CPU
        // cycles on further message processing.
        ExitNow(err = CHIP_NO_ERROR);
    }

    if (packetHeader.GetFlags().Has(Header::FlagValues::kSecureSessionControlMessage))
    {
        // TODO: control message counter is not implemented yet
    }
    else
    {
        state->GetSessionMessageCounter().GetPeerMessageCounter().Commit(packetHeader.GetMessageId());
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
        SessionHandle session(state->GetPeerNodeId(), state->GetLocalKeyID(), state->GetPeerKeyID(), state->GetFabricIndex());
        mCB->OnMessageReceived(packetHeader, payloadHeader, session, peerAddress, isDuplicate, std::move(msg));
    }

exit:
    if (err != CHIP_NO_ERROR && mCB != nullptr)
    {
        mCB->OnReceiveError(err, peerAddress);
    }
}

void SecureSessionMgr::HandleConnectionExpired(const Transport::PeerConnectionState & state)
{
    ChipLogDetail(Inet, "Marking old secure session for device 0x" ChipLogFormatX64 " as expired",
                  ChipLogValueX64(state.GetPeerNodeId()));

    if (mCB != nullptr)
    {
        mCB->OnConnectionExpired(
            SessionHandle(state.GetPeerNodeId(), state.GetLocalKeyID(), state.GetPeerKeyID(), state.GetFabricIndex()));
    }

    mTransportMgr->Disconnect(state.GetPeerAddress());
}

void SecureSessionMgr::ExpiryTimerCallback(System::Layer * layer, void * param)
{
    SecureSessionMgr * mgr = reinterpret_cast<SecureSessionMgr *>(param);
#if CHIP_CONFIG_SESSION_REKEYING
    // TODO(#2279): session expiration is currently disabled until rekeying is supported
    // the #ifdef should be removed after that.
    mgr->mPeerConnections.ExpireInactiveConnections(
        CHIP_PEER_CONNECTION_TIMEOUT_MS,
        [this](const Transport::PeerConnectionState & state1) { HandleConnectionExpired(state1); });
#endif
    mgr->ScheduleExpiryTimer(); // re-schedule the oneshot timer
}

PeerConnectionState * SecureSessionMgr::GetPeerConnectionState(SessionHandle session)
{
    return mPeerConnections.FindPeerConnectionStateByLocalKey(Optional<NodeId>::Value(session.mPeerNodeId),
                                                              session.mLocalKeyId.ValueOr(0), nullptr);
}

} // namespace chip
