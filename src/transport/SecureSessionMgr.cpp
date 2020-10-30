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

#include <string.h>
#include <support/CodeUtils.h>
#include <support/SafeInt.h>
#include <support/logging/CHIPLogging.h>
#include <transport/SecurePairingSession.h>
#include <transport/SecureSessionMgr.h>

#include <inttypes.h>

namespace chip {

using System::PacketBuffer;
using Transport::PeerAddress;
using Transport::PeerConnectionState;

// Maximum length of application data that can be encrypted as one block.
// The limit is derived from IPv6 MTU (1280 bytes) - expected header overheads.
// This limit would need additional reviews once we have formalized Secure Transport header.
//
// TODO: this should be checked within the transport message sending instead of the session management layer.
static const size_t kMax_SecureSDU_Length = 1024;

SecureSessionMgrBase::SecureSessionMgrBase() : mState(State::kNotReady) {}

SecureSessionMgrBase::~SecureSessionMgrBase()
{
    CancelExpiryTimer();
}

CHIP_ERROR SecureSessionMgrBase::InitInternal(NodeId localNodeId, System::Layer * systemLayer, Transport::Base * transport)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrExit(mState == State::kNotReady, err = CHIP_ERROR_INCORRECT_STATE);

    mState       = State::kInitialized;
    mLocalNodeId = localNodeId;
    mSystemLayer = systemLayer;
    mTransport   = transport;

    mTransport->SetMessageReceiveHandler(HandleDataReceived, this);
    mPeerConnections.SetConnectionExpiredHandler(HandleConnectionExpired, this);

    ScheduleExpiryTimer();

exit:
    return err;
}

CHIP_ERROR SecureSessionMgrBase::SendMessage(NodeId peerNodeId, System::PacketBuffer * msgBuf)
{
    PayloadHeader payloadHeader;

    return SendMessage(payloadHeader, peerNodeId, msgBuf);
}

CHIP_ERROR SecureSessionMgrBase::SendMessage(PayloadHeader & payloadHeader, NodeId peerNodeId, System::PacketBuffer * msgBuf)
{
    CHIP_ERROR err              = CHIP_NO_ERROR;
    PeerConnectionState * state = nullptr;

    VerifyOrExit(mState == State::kInitialized, err = CHIP_ERROR_INCORRECT_STATE);

    VerifyOrExit(msgBuf != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(msgBuf->Next() == nullptr, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);
    VerifyOrExit(msgBuf->TotalLength() < kMax_SecureSDU_Length, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    // Find an active connection to the specified peer node
    VerifyOrExit(mPeerConnections.FindPeerConnectionState(peerNodeId, &state), err = CHIP_ERROR_INVALID_DESTINATION_NODE_ID);

    // This marks any connection where we send data to as 'active'
    mPeerConnections.MarkConnectionActive(state);

    {
        uint8_t * data = nullptr;
        PacketHeader packetHeader;
        MessageAuthenticationCode mac;

        const uint16_t headerSize = payloadHeader.EncodeSizeBytes();
        const bool encrypted      = state->GetSecureSession().IsEncrypted();
        uint16_t actualEncodedHeaderSize;
        uint16_t totalLen = 0;
        uint16_t taglen   = 0;
        uint32_t payloadLength; // Make sure it's big enough to add two 16-bit
                                // ints without overflowing.
        static_assert(std::is_same<decltype(msgBuf->TotalLength()), uint16_t>::value,
                      "Addition to generate payloadLength might overflow");
        payloadLength = static_cast<uint32_t>(headerSize + msgBuf->TotalLength());
        VerifyOrExit(CanCastTo<uint16_t>(payloadLength), err = CHIP_ERROR_NO_MEMORY);

        packetHeader
            .SetSourceNodeId(mLocalNodeId)              //
            .SetDestinationNodeId(peerNodeId)           //
            .SetMessageId(state->GetSendMessageIndex()) //
            .SetEncryptionKeyID(state->GetLocalKeyID()) //
            .SetPayloadLength(static_cast<uint16_t>(payloadLength));

        if (encrypted)
        {
            packetHeader.GetFlags().Set(Header::FlagValues::kEncrypted);
        }
        else
        {
            packetHeader.GetFlags().Clear(Header::FlagValues::kEncrypted);
        }

        VerifyOrExit(msgBuf->EnsureReservedSize(headerSize), err = CHIP_ERROR_NO_MEMORY);

        msgBuf->SetStart(msgBuf->Start() - headerSize);
        data     = msgBuf->Start();
        totalLen = msgBuf->TotalLength();

        err = payloadHeader.Encode(data, totalLen, &actualEncodedHeaderSize);
        SuccessOrExit(err);

        // Encrypt will do simple copy for unsecure session.
        err = state->GetSecureSession().Encrypt(data, totalLen, data, packetHeader, payloadHeader.GetEncodePacketFlags(), mac);
        SuccessOrExit(err);

        if (encrypted)
        {
            // Only encrypted packet have mac.
            err = mac.Encode(packetHeader, &data[totalLen], kMaxTagLen, &taglen);
            SuccessOrExit(err);
        }

        VerifyOrExit(CanCastTo<uint16_t>(totalLen + taglen), err = CHIP_ERROR_INTERNAL);
        msgBuf->SetDataLength(static_cast<uint16_t>(totalLen + taglen), nullptr);

        ChipLogDetail(Inet, "Secure transport transmitting msg %u after encryption", state->GetSendMessageIndex());

        err    = mTransport->SendMessage(packetHeader, payloadHeader.GetEncodePacketFlags(), state->GetPeerAddress(), msgBuf);
        msgBuf = nullptr;
    }
    SuccessOrExit(err);
    state->IncrementSendMessageIndex();

exit:
    if (msgBuf != nullptr)
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
        PacketBuffer::Free(msgBuf);
        msgBuf = nullptr;
    }

    return err;
}

CHIP_ERROR SecureSessionMgrBase::NewPairing(const Optional<Transport::PeerAddress> & peerAddr, SecurePairingSession * pairing)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    PeerConnectionState * state = nullptr;
    Optional<NodeId> peerNodeId = Optional<NodeId>::Value(pairing->GetPeerNodeId());
    uint16_t peerKeyId          = pairing->GetPeerKeyId();
    uint16_t localKeyId         = pairing->GetLocalKeyId();

    // Find any existing connection with the same node and key ID
    if (mPeerConnections.FindPeerConnectionState(peerNodeId, peerKeyId, &state))
    {
        mPeerConnections.MarkConnectionExpired(state);
    }

    ChipLogDetail(Inet, "New pairing for key %d!!", peerKeyId);
    state = nullptr;
    err   = mPeerConnections.CreateNewPeerConnectionState(peerNodeId, peerKeyId, localKeyId, &state);
    SuccessOrExit(err);

    if (peerAddr.HasValue())
    {
        state->SetPeerAddress(peerAddr.Value());
    }

    if (state != nullptr)
    {
        err = pairing->DeriveSecureSession(reinterpret_cast<const uint8_t *>(kSpake2pI2RSessionInfo),
                                           strlen(kSpake2pI2RSessionInfo), state->GetSecureSession());
    }

exit:
    return err;
}

CHIP_ERROR SecureSessionMgrBase::NewUnsecureSession(const Transport::PeerAddress & peerAddr, const Optional<NodeId> & nodeId,
                                                    PeerConnectionState ** state)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    *state = nullptr;

    // Find any existing connection with the same node and key ID
    if (mPeerConnections.FindPeerConnectionState(peerAddr, &state))
    {
        mPeerConnections.MarkConnectionExpired(state);
    }

    ChipLogDetail(Inet, "New unsecure session for address!!");
    err = mPeerConnections.CreateNewPeerConnectionState(peerAddr, state);
    SuccessOrExit(err);

    if (nodeId.HasValue())
    {
        (*state)->SetPeerNodeId(nodeId.Value());
    }

exit:
    return err;
}

void SecureSessionMgrBase::ScheduleExpiryTimer()
{
    CHIP_ERROR err =
        mSystemLayer->StartTimer(CHIP_PEER_CONNECTION_TIMEOUT_CHECK_FREQUENCY_MS, SecureSessionMgrBase::ExpiryTimerCallback, this);

    VerifyOrDie(err == CHIP_NO_ERROR);
}

void SecureSessionMgrBase::CancelExpiryTimer()
{
    if (mSystemLayer != nullptr)
    {
        mSystemLayer->CancelTimer(SecureSessionMgrBase::ExpiryTimerCallback, this);
    }
}

void SecureSessionMgrBase::HandleDataReceived(const PacketHeader & packetHeader, const PeerAddress & peerAddress,
                                              System::PacketBuffer * msg, SecureSessionMgrBase * connection)

{
    CHIP_ERROR err                 = CHIP_NO_ERROR;
    System::PacketBuffer * origMsg = nullptr;
    PeerConnectionState * state    = nullptr;
    const bool encrypted           = packetHeader.GetFlags().Get(Header::FlagValues::kEncrypted);

    VerifyOrExit(msg != nullptr, ChipLogError(Inet, "Secure transport received NULL packet, discarding"));

    if (encrypted)
    {
        if (!connection->mPeerConnections.FindPeerConnectionState(packetHeader.GetSourceNodeId(), packetHeader.GetEncryptionKeyID(),
                                                                  &state))
        {
            ChipLogError(Inet, "Data received on an unknown connection (%d). Dropping it!!", packetHeader.GetEncryptionKeyID());
            ExitNow(err = CHIP_ERROR_KEY_NOT_FOUND_FROM_PEER);
        }
    }
    else
    {
        // TODO: Check if such protocol is allowed to be unsecure.
        // Unsecure connections are identified by address instead of NodeId.
        if (!connection->mPeerConnections.FindPeerConnectionState(peerAddress, &state))
        {
            // We did not have connections with this peer before, create a new state
            err = connection->NewUnsecureSession(peerAddress, packetHeader.GetSourceNodeId(), &state);
            SuccessOrExit(err);
        }
    }

    if (!state->GetPeerAddress().IsInitialized())
    {
        state->SetPeerAddress(peerAddress);
    }

    connection->mPeerConnections.MarkConnectionActive(state);

    // TODO this is where messages should be decoded
    {
        PayloadHeader payloadHeader;
        MessageAuthenticationCode mac;

        uint8_t * data            = msg->Start();
        uint8_t * plainText       = nullptr;
        uint16_t len              = msg->TotalLength();
        const uint16_t headerSize = payloadHeader.EncodeSizeBytes();
        uint16_t decodedSize      = 0;
        uint16_t taglen           = 0;
        uint16_t payloadlen       = 0;

#if CHIP_SYSTEM_CONFIG_USE_LWIP
        /* This is a workaround for the case where PacketBuffer payload is not
           allocated as an inline buffer to PacketBuffer structure */
        origMsg = msg;
        msg     = PacketBuffer::NewWithAvailableSize(len);
        VerifyOrExit(msg != nullptr, ChipLogError(Inet, "Insufficient memory for packet buffer."));
        msg->SetDataLength(len, msg);
#endif
        plainText = msg->Start();

        payloadlen = packetHeader.GetPayloadLength();
        VerifyOrExit(
            payloadlen <= len,
            (ChipLogError(Inet, "Secure transport can't find MAC Tag; buffer too short"), err = CHIP_ERROR_INVALID_MESSAGE_LENGTH));

        if (encrypted)
        {
            err = mac.Decode(packetHeader, &data[payloadlen], static_cast<uint16_t>(len - payloadlen), &taglen);
            VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Inet, "Secure transport failed to decode MAC Tag: err %d", err));
            len = static_cast<uint16_t>(len - taglen);
        }
        msg->SetDataLength(len, nullptr);

        err = state->GetSecureSession().Decrypt(data, len, plainText, packetHeader, payloadHeader.GetEncodePacketFlags(), mac);
        VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Inet, "Secure transport failed to decrypt msg: err %d", err));

        err = payloadHeader.Decode(packetHeader.GetFlags(), plainText, len, &decodedSize);
        VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Inet, "Secure transport failed to decode encrypted header: err %d", err));
        VerifyOrExit(headerSize == decodedSize, ChipLogError(Inet, "Secure transport decode encrypted header length mismatched"));

        msg->ConsumeHead(headerSize);

        if (state->GetPeerNodeId() == kUndefinedNodeId && packetHeader.GetSourceNodeId().HasValue())
        {
            state->SetPeerNodeId(packetHeader.GetSourceNodeId().Value());
        }

        if (connection->mCB != nullptr)
        {
            connection->mCB->OnMessageReceived(packetHeader, payloadHeader, state, msg, connection);
            msg = nullptr;
        }
    }

exit:
    if (origMsg != nullptr)
    {
        PacketBuffer::Free(origMsg);
    }

    if (msg != nullptr)
    {
        PacketBuffer::Free(msg);
    }

    if (err != CHIP_NO_ERROR && connection->mCB != nullptr)
    {
        connection->mCB->OnReceiveError(err, peerAddress, connection);
    }
}

void SecureSessionMgrBase::HandleConnectionExpired(const Transport::PeerConnectionState & state, SecureSessionMgrBase * mgr)
{
    char addr[Transport::PeerAddress::kMaxToStringSize];
    state.GetPeerAddress().ToString(addr, sizeof(addr));

    ChipLogDetail(Inet, "Connection from '%s' expired", addr);

    mgr->mTransport->Disconnect(state.GetPeerAddress());
}

void SecureSessionMgrBase::ExpiryTimerCallback(System::Layer * layer, void * param, System::Error error)
{
    SecureSessionMgrBase * mgr = reinterpret_cast<SecureSessionMgrBase *>(param);
#if CHIP_CONFIG_SESSION_REKEYING
    // TODO(#2279): session expiration is currently disabled until rekeying is supported
    // the #ifdef should be removed after that.
    mgr->mPeerConnections.ExpireInactiveConnections(CHIP_PEER_CONNECTION_TIMEOUT_MS);
#endif
    mgr->ScheduleExpiryTimer(); // re-schedule the oneshot timer
}

} // namespace chip
