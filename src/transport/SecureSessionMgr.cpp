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
#include <support/logging/CHIPLogging.h>
#include <transport/SecureSessionMgr.h>

#include <inttypes.h>

namespace chip {

using Transport::PeerAddress;
using Transport::PeerConnectionState;

// Maximum length of application data that can be encrypted as one block.
// The limit is derived from IPv6 MTU (1280 bytes) - expected header overheads.
// This limit would need additional reviews once we have formalized Secure Transport header.
static const size_t kMax_SecureSDU_Length = 1024;

SecureSessionMgr::SecureSessionMgr() : mState(State::kNotReady) {}

SecureSessionMgr::~SecureSessionMgr()
{
    CancelExpiryTimer();
    if (mCB != nullptr)
    {
        mCB->Release();
    }
}

CHIP_ERROR SecureSessionMgr::Init(NodeId localNodeId, Inet::InetLayer * inet, const Transport::UdpListenParameters & listenParams)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrExit(mState == State::kNotReady, err = CHIP_ERROR_INCORRECT_STATE);

    err = mTransport.Init(inet, listenParams);
    SuccessOrExit(err);

    mTransport.SetMessageReceiveHandler(HandleDataReceived, this);
    mPeerConnections.SetConnectionExpiredHandler(HandleConnectionExpired, this);

    mState       = State::kInitialized;
    mLocalNodeId = localNodeId;
    mSystemLayer = inet->SystemLayer();

    ScheduleExpiryTimer();

exit:
    return err;
}

CHIP_ERROR SecureSessionMgr::Connect(NodeId peerNodeId, const Transport::PeerAddress & peerAddress)
{
    CHIP_ERROR err              = CHIP_NO_ERROR;
    PeerConnectionState * state = nullptr;

    VerifyOrExit(mState == State::kInitialized, err = CHIP_ERROR_INCORRECT_STATE);

    err = mPeerConnections.CreateNewPeerConnectionState(peerAddress, &state);
    SuccessOrExit(err);

    state->SetPeerNodeId(peerNodeId);

    // TODO:
    //   - on new connection for other transports may not be inline.
    //   - determine if logic is required to prevent multiple connections to the
    //     same node or if some connection reuse is required.
    if (mCB != nullptr)
    {
        mCB->OnNewConnection(state, this);
    }

exit:
    return err;
}

CHIP_ERROR SecureSessionMgr::SendMessage(NodeId peerNodeId, System::PacketBuffer * msgBuf)
{
    CHIP_ERROR err              = CHIP_NO_ERROR;
    PeerConnectionState * state = nullptr;

    VerifyOrExit(mState == State::kInitialized, err = CHIP_ERROR_INCORRECT_STATE);

    VerifyOrExit(msgBuf != NULL, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(msgBuf->Next() == NULL, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);
    VerifyOrExit(msgBuf->TotalLength() < kMax_SecureSDU_Length, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    // Find an active connection to the specified peer node
    VerifyOrExit(mPeerConnections.FindPeerConnectionState(peerNodeId, &state), err = CHIP_ERROR_INVALID_DESTINATION_NODE_ID);

    // This marks any connection where we send data to as 'active'
    mPeerConnections.MarkConnectionActive(state);

    {
        uint8_t * data = msgBuf->Start();
        MessageHeader header;

        err = state->GetSecureSession().Encrypt(data, msgBuf->TotalLength(), data, header);
        SuccessOrExit(err);

        ChipLogProgress(Inet, "Secure transport transmitting msg %u after encryption", state->GetSendMessageIndex());

        header
            .SetSourceNodeId(mLocalNodeId)    //
            .SetDestinationNodeId(peerNodeId) //
            .SetMessageId(state->GetSendMessageIndex());

        err    = mTransport.SendMessage(header, state->GetPeerAddress(), msgBuf);
        msgBuf = NULL;
    }
    SuccessOrExit(err);
    state->IncrementSendMessageIndex();

exit:
    if (msgBuf != NULL)
    {
        const char * errStr = ErrorStr(err);
        if (state == nullptr)
        {
            ChipLogProgress(Inet, "Secure transport could not find a valid PeerConnection: %s", errStr);
        }
        else
        {
            ChipLogProgress(Inet, "Secure transport failed to encrypt msg %u: %s", state->GetSendMessageIndex(), errStr);
        }
        PacketBuffer::Free(msgBuf);
        msgBuf = NULL;
    }

    return err;
}

CHIP_ERROR SecureSessionMgr::AllocateNewConnection(const MessageHeader & header, const PeerAddress & address,
                                                   Transport::PeerConnectionState ** state)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = mPeerConnections.CreateNewPeerConnectionState(address, state);
    SuccessOrExit(err);

    if (header.GetSourceNodeId().HasValue())
    {
        (*state)->SetPeerNodeId(header.GetSourceNodeId().Value());
    }

    if (mCB != nullptr)
    {
        mCB->OnNewConnection(*state, this);
    }

exit:
    return err;
}

void SecureSessionMgr::ScheduleExpiryTimer(void)
{
    CHIP_ERROR err =
        mSystemLayer->StartTimer(CHIP_PEER_CONNECTION_TIMEOUT_CHECK_FREQUENCY_MS, SecureSessionMgr::ExpiryTimerCallback, this);

    VerifyOrDie(err == CHIP_NO_ERROR);
}

void SecureSessionMgr::CancelExpiryTimer(void)
{
    if (mSystemLayer != nullptr)
    {
        mSystemLayer->CancelTimer(SecureSessionMgr::ExpiryTimerCallback, this);
    }
}

void SecureSessionMgr::HandleDataReceived(const MessageHeader & header, const PeerAddress & peerAddress, System::PacketBuffer * msg,
                                          SecureSessionMgr * connection)

{
    CHIP_ERROR err                 = CHIP_NO_ERROR;
    System::PacketBuffer * origMsg = nullptr;
    PeerConnectionState * state    = nullptr;

    VerifyOrExit(msg != nullptr, ChipLogError(Inet, "Secure transport received NULL packet, discarding"));

    {
        if (!connection->mPeerConnections.FindPeerConnectionState(peerAddress, &state))
        {
            ChipLogProgress(Inet, "New peer connection received.");

            err = connection->AllocateNewConnection(header, peerAddress, &state);
            SuccessOrExit(err);
        }
        else
        {
            connection->mPeerConnections.MarkConnectionActive(state);
        }
    }

    // TODO this is where messages should be decoded
    {
        uint8_t * data      = msg->Start();
        uint8_t * plainText = nullptr;
        uint16_t len        = msg->TotalLength();
#if CHIP_SYSTEM_CONFIG_USE_LWIP
        /* This is a workaround for the case where PacketBuffer payload is not allocated
           as an inline buffer to PacketBuffer structure */
        origMsg = msg;
        msg     = PacketBuffer::NewWithAvailableSize(len);
        msg->SetDataLength(len, msg);
#endif
        plainText = msg->Start();

        err = state->GetSecureSession().Decrypt(data, len, plainText, header);
        VerifyOrExit(err == CHIP_NO_ERROR, ChipLogProgress(Inet, "Secure transport failed to decrypt msg: err %d", err));

        if (connection->mCB != nullptr)
        {
            connection->mCB->OnMessageReceived(header, state, msg, connection);
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

void SecureSessionMgr::HandleConnectionExpired(const Transport::PeerConnectionState & state, SecureSessionMgr * mgr)
{
    char addr[Transport::PeerAddress::kMaxToStringSize];
    state.GetPeerAddress().ToString(addr, sizeof(addr));

    ChipLogProgress(Inet, "Connection from '%s' expired", addr);
}

void SecureSessionMgr::ExpiryTimerCallback(System::Layer * layer, void * param, System::Error error)
{
    ChipLogProgress(Inet, "Checking for expired connections");

    SecureSessionMgr * mgr = reinterpret_cast<SecureSessionMgr *>(param);
    mgr->mPeerConnections.ExpireInactiveConnections(CHIP_PEER_CONNECTION_TIMEOUT_MS);
    mgr->ScheduleExpiryTimer(); // re-schedule the oneshot timer
}

} // namespace chip
