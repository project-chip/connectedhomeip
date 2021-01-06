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
 *      This file implements the CHIP Connection object that maintains a UDP connection.
 *      TODO This class should be extended to support TCP as well...
 *
 */

#include "SecureSessionMgr.h"

#include <inttypes.h>
#include <string.h>

#include <platform/CHIPDeviceLayer.h>
#include <support/CodeUtils.h>
#include <support/SafeInt.h>
#include <support/logging/CHIPLogging.h>
#include <transport/RendezvousSession.h>
#include <transport/SecurePairingSession.h>
#include <transport/SecureSessionMgr.h>
#include <transport/TransportMgr.h>

#include <inttypes.h>

namespace chip {

using System::PacketBuffer;
using System::PacketBufferHandle;
using Transport::PeerAddress;
using Transport::PeerConnectionState;

// Maximum length of application data that can be encrypted as one block.
// The limit is derived from IPv6 MTU (1280 bytes) - expected header overheads.
// This limit would need additional reviews once we have formalized Secure Transport header.
//
// TODO: this should be checked within the transport message sending instead of the session management layer.
static const size_t kMax_SecureSDU_Length = 1024;

SecureSessionMgr::SecureSessionMgr() : mState(State::kNotReady) {}

SecureSessionMgr::~SecureSessionMgr()
{
    CancelExpiryTimer();
}

CHIP_ERROR SecureSessionMgr::Init(NodeId localNodeId, System::Layer * systemLayer, TransportMgrBase * transportMgr)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrExit(mState == State::kNotReady, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(transportMgr != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    mState        = State::kInitialized;
    mLocalNodeId  = localNodeId;
    mSystemLayer  = systemLayer;
    mTransportMgr = transportMgr;

    ChipLogProgress(Inet, "local node id is %llu\n", mLocalNodeId);

    ScheduleExpiryTimer();

    mTransportMgr->SetSecureSessionMgr(this);

exit:
    return err;
}

CHIP_ERROR SecureSessionMgr::SendMessage(NodeId peerNodeId, System::PacketBufferHandle msgBuf)
{
    PayloadHeader payloadHeader;

    return SendMessage(payloadHeader, peerNodeId, std::move(msgBuf));
}

CHIP_ERROR SecureSessionMgr::SendMessage(PayloadHeader & payloadHeader, NodeId peerNodeId, System::PacketBufferHandle msgBuf,
                                         EncryptedPacketBufferHandle * bufferRetainSlot)
{
    return SendMessage(payloadHeader, peerNodeId, std::move(msgBuf), bufferRetainSlot, false);
}

CHIP_ERROR SecureSessionMgr::SendMessage(EncryptedPacketBufferHandle msgBuf, EncryptedPacketBufferHandle * bufferRetainSlot)
{
    PayloadHeader payloadHeader;

    return SendMessage(payloadHeader, 0, std::move(msgBuf), bufferRetainSlot, true);
}

CHIP_ERROR SecureSessionMgr::SendMessage(PayloadHeader & payloadHeader, NodeId peerNodeId, System::PacketBufferHandle msgBuf,
                                         EncryptedPacketBufferHandle * bufferRetainSlot, bool isEncrypted)
{
    CHIP_ERROR err              = CHIP_NO_ERROR;
    PeerConnectionState * state = nullptr;
    PacketHeader packetHeader;
    uint16_t headerSize = 0;

    if (isEncrypted)
    {
        err = packetHeader.Decode(msgBuf->Start(), msgBuf->DataLength(), &headerSize);
        SuccessOrExit(err);

        VerifyOrExit(packetHeader.GetDestinationNodeId().HasValue(), err = CHIP_ERROR_INVALID_DESTINATION_NODE_ID);
        peerNodeId = packetHeader.GetDestinationNodeId().Value();
    }

    state = mPeerConnections.FindPeerConnectionState(peerNodeId, nullptr);
    VerifyOrExit(mState == State::kInitialized, err = CHIP_ERROR_INCORRECT_STATE);

    VerifyOrExit(!msgBuf.IsNull(), err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(!msgBuf->HasChainedBuffer(), err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);
    VerifyOrExit(msgBuf->TotalLength() < kMax_SecureSDU_Length, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    // Find an active connection to the specified peer node
    VerifyOrExit(state != nullptr, err = CHIP_ERROR_INVALID_DESTINATION_NODE_ID);

    // This marks any connection where we send data to as 'active'
    mPeerConnections.MarkConnectionActive(state);

    {
        uint8_t * data         = nullptr;
        uint8_t * p            = nullptr;
        uint32_t msgId         = 0;
        uint32_t payloadLength = 0; // Make sure it's big enough to add two 16-bit ints without overflowing.
        uint16_t len           = 0;
        MessageAuthenticationCode mac;

        if (!isEncrypted)
        {
            msgId = state->GetSendMessageIndex();

            static_assert(std::is_same<decltype(msgBuf->TotalLength()), uint16_t>::value,
                          "Addition to generate payloadLength might overflow");

            headerSize    = payloadHeader.EncodeSizeBytes();
            payloadLength = static_cast<uint32_t>(headerSize + msgBuf->TotalLength());
            VerifyOrExit(CanCastTo<uint16_t>(payloadLength), err = CHIP_ERROR_NO_MEMORY);

            packetHeader
                .SetSourceNodeId(mLocalNodeId)              //
                .SetDestinationNodeId(peerNodeId)           //
                .SetMessageId(msgId)                        //
                .SetEncryptionKeyID(state->GetLocalKeyID()) //
                .SetPayloadLength(static_cast<uint16_t>(payloadLength));
            packetHeader.GetFlags().Set(Header::FlagValues::kSecure);
        }
        else
        {
            // Advancing the start to encrypted header, since the transport will attach the packet header on top of it
            msgBuf->SetStart(msgBuf->Start() + headerSize);
        }

        ChipLogProgress(Inet, "Sending msg from %llu to %llu", mLocalNodeId, peerNodeId);

        // Encrypt the packet if it's not already encrypted
        if (!isEncrypted)
        {
            uint16_t totalLen = 0;
            uint16_t taglen   = 0;
            uint16_t actualEncodedHeaderSize;

            VerifyOrExit(msgBuf->EnsureReservedSize(headerSize), err = CHIP_ERROR_NO_MEMORY);

            msgBuf->SetStart(msgBuf->Start() - headerSize);
            data     = msgBuf->Start();
            totalLen = msgBuf->TotalLength();

            err = payloadHeader.Encode(data, totalLen, &actualEncodedHeaderSize);
            SuccessOrExit(err);

            err = state->GetSecureSession().Encrypt(data, totalLen, data, packetHeader, mac);
            SuccessOrExit(err);
            err = mac.Encode(packetHeader, &data[totalLen], kMaxTagLen, &taglen);
            SuccessOrExit(err);

            VerifyOrExit(CanCastTo<uint16_t>(totalLen + taglen), err = CHIP_ERROR_INTERNAL);
            msgBuf->SetDataLength(static_cast<uint16_t>(totalLen + taglen));

            ChipLogDetail(Inet, "Secure transport transmitting msg %u after encryption", msgId);
        }

        if (bufferRetainSlot)
        {
            // The start of buffer points to the beginning of the encrypted header, and the length of buffer
            // contains both the encrypted header and encrypted data.
            // Locally store the start and length of the retained buffer after accounting for the size of packet header.
            headerSize = packetHeader.EncodeSizeBytes();

            p   = static_cast<uint8_t *>(msgBuf->Start() - headerSize);
            len = static_cast<uint16_t>(msgBuf->DataLength() + headerSize);

            // Retain the PacketBuffer for following retransmit.
            *bufferRetainSlot        = msgBuf.Retain();
            bufferRetainSlot->mMsgId = msgId;
        }

        err = mTransportMgr->SendMessage(packetHeader, state->GetPeerAddress(), std::move(msgBuf));

        if (bufferRetainSlot)
        {
            // Rewind the start and len of the buffer back to pre-send state for following possible retransmition.
            (*bufferRetainSlot)->SetStart(p);
            (*bufferRetainSlot)->SetDataLength(len);
        }
    }
    SuccessOrExit(err);

    if (!isEncrypted)
        state->IncrementSendMessageIndex();

exit:
    if (!msgBuf.IsNull())
    {
        const char * errStr = ErrorStr(err);
        if (state == nullptr)
        {
            ChipLogError(Inet, "Secure transport could not find a valid PeerConnection: %s", errStr);
        }
        else
        {
            ChipLogError(Inet, "Secure transport failed to encrypt msg %u: %s", state->GetSendMessageIndex(), errStr);
        }
    }

    return err;
}

CHIP_ERROR SecureSessionMgr::NewPairing(const Optional<Transport::PeerAddress> & peerAddr, NodeId peerNodeId,
                                        SecurePairingSession * pairing)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    uint16_t peerKeyId          = pairing->GetPeerKeyId();
    uint16_t localKeyId         = pairing->GetLocalKeyId();
    PeerConnectionState * state = mPeerConnections.FindPeerConnectionState(Optional<NodeId>::Value(peerNodeId), peerKeyId, nullptr);

    // Find any existing connection with the same node and key ID
    if (state)
    {
        mPeerConnections.MarkConnectionExpired(
            state, [this](const Transport::PeerConnectionState & state1) { HandleConnectionExpired(state1); });
    }

    ChipLogDetail(Inet, "New pairing for device %llu, key %d!!", peerNodeId, peerKeyId);

    state = nullptr;
    err   = mPeerConnections.CreateNewPeerConnectionState(Optional<NodeId>::Value(peerNodeId), peerKeyId, localKeyId, &state);
    SuccessOrExit(err);

    if (peerAddr.HasValue() && peerAddr.Value().GetIPAddress() != Inet::IPAddress::Any)
    {
        state->SetPeerAddress(peerAddr.Value());
    }
    else if (peerAddr.HasValue() &&
             (peerAddr.Value().GetTransportType() == Transport::Type::kTcp ||
              peerAddr.Value().GetTransportType() == Transport::Type::kUdp))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (state != nullptr)
    {
        err = pairing->DeriveSecureSession(reinterpret_cast<const uint8_t *>(kSpake2pI2RSessionInfo),
                                           strlen(kSpake2pI2RSessionInfo), state->GetSecureSession());
    }

exit:
    return err;
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

void SecureSessionMgr::OnMessageReceived(const PacketHeader & packetHeader, const PeerAddress & peerAddress,
                                         System::PacketBufferHandle msg)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    PeerConnectionState * state =
        mPeerConnections.FindPeerConnectionState(packetHeader.GetSourceNodeId(), packetHeader.GetEncryptionKeyID(), nullptr);
    PacketBufferHandle origMsg;

    VerifyOrExit(!msg.IsNull(), ChipLogError(Inet, "Secure transport received NULL packet, discarding"));

    if (state == nullptr)
    {
        ChipLogError(Inet, "Data received on an unknown connection (%d). Dropping it!!", packetHeader.GetEncryptionKeyID());
        ExitNow(err = CHIP_ERROR_KEY_NOT_FOUND_FROM_PEER);
    }

    if (!state->GetPeerAddress().IsInitialized())
    {
        state->SetPeerAddress(peerAddress);
    }

    mPeerConnections.MarkConnectionActive(state);

    // TODO this is where messages should be decoded
    {
        PayloadHeader payloadHeader;
        MessageAuthenticationCode mac;

        uint8_t * data       = msg->Start();
        uint8_t * plainText  = nullptr;
        uint16_t len         = msg->TotalLength();
        uint16_t headerSize  = 0;
        uint16_t decodedSize = 0;
        uint16_t taglen      = 0;
        uint16_t payloadlen  = 0;

#if CHIP_SYSTEM_CONFIG_USE_LWIP
        /* This is a workaround for the case where PacketBuffer payload is not
           allocated as an inline buffer to PacketBuffer structure */
        origMsg = std::move(msg);
        msg     = PacketBuffer::NewWithAvailableSize(len);
        VerifyOrExit(!msg.IsNull(), ChipLogError(Inet, "Insufficient memory for packet buffer."));
        msg->SetDataLength(len);
#endif
        plainText = msg->Start();

        payloadlen = packetHeader.GetPayloadLength();
        VerifyOrExit(
            payloadlen <= len,
            (ChipLogError(Inet, "Secure transport can't find MAC Tag; buffer too short"), err = CHIP_ERROR_INVALID_MESSAGE_LENGTH));
        err = mac.Decode(packetHeader, &data[payloadlen], static_cast<uint16_t>(len - payloadlen), &taglen);
        VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Inet, "Secure transport failed to decode MAC Tag: err %d", err));
        len = static_cast<uint16_t>(len - taglen);
        msg->SetDataLength(len);

        err = state->GetSecureSession().Decrypt(data, len, plainText, packetHeader, mac);
        VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Inet, "Secure transport failed to decrypt msg: err %d", err));

        err        = payloadHeader.Decode(plainText, len, &decodedSize);
        headerSize = payloadHeader.EncodeSizeBytes();
        VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Inet, "Secure transport failed to decode encrypted header: err %d", err));
        VerifyOrExit(headerSize == decodedSize, ChipLogError(Inet, "Secure transport decode encrypted header length mismatched"));

        msg->ConsumeHead(headerSize);

        if (state->GetPeerNodeId() == kUndefinedNodeId && packetHeader.GetSourceNodeId().HasValue())
        {
            state->SetPeerNodeId(packetHeader.GetSourceNodeId().Value());
        }

        if (mCB != nullptr)
        {
            mCB->OnMessageReceived(packetHeader, payloadHeader, state, std::move(msg), this);
        }
    }

exit:
    if (err != CHIP_NO_ERROR && mCB != nullptr)
    {
        mCB->OnReceiveError(err, peerAddress, this);
    }
}

void SecureSessionMgr::HandleConnectionExpired(const Transport::PeerConnectionState & state)
{
    char addr[Transport::PeerAddress::kMaxToStringSize];
    state.GetPeerAddress().ToString(addr, sizeof(addr));

    ChipLogDetail(Inet, "Connection from '%s' expired", addr);

    if (mCB != nullptr)
    {
        mCB->OnConnectionExpired(&state, this);
    }

    mTransportMgr->Disconnect(state.GetPeerAddress());
}

void SecureSessionMgr::ExpiryTimerCallback(System::Layer * layer, void * param, System::Error error)
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

} // namespace chip
