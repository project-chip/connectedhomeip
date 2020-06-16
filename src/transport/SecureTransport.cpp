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
#include <transport/SecureTransport.h>

#include <inttypes.h>

namespace chip {

// Maximum length of application data that can be encrypted as one block.
// The limit is derived from IPv6 MTU (1280 bytes) - expected header overheads.
// This limit would need additional reviews once we have formalized Secure Transport header.
static const size_t kMax_SecureSDU_Length         = 1024;
static const char * kManualKeyExchangeChannelInfo = "Manual Key Exchanged Channel";

SecureTransport::SecureTransport() : mConnectionState(Transport::PeerAddress::Uninitialized()), mState(State::kNotReady)
{
    OnMessageReceived = NULL;
}

CHIP_ERROR SecureTransport::Init(NodeId localNodeId, Inet::InetLayer * inet, const Transport::UdpListenParameters & listenParams)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrExit(mState == State::kNotReady, err = CHIP_ERROR_INCORRECT_STATE);

    err = mTransport.Init(inet, listenParams);
    SuccessOrExit(err);

    mTransport.SetMessageReceiveHandler(HandleDataReceived, this);
    mState       = State::kInitialized;
    mLocalNodeId = localNodeId;

exit:
    return err;
}

CHIP_ERROR SecureTransport::Connect(NodeId peerNodeId, const Transport::PeerAddress & peerAddress)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mState == State::kInitialized, err = CHIP_ERROR_INCORRECT_STATE);

    mConnectionState.SetPeerNodeId(peerNodeId);
    mConnectionState.SetPeerAddress(peerAddress);

    mState = State::kConnected;

exit:
    return err;
}

CHIP_ERROR SecureTransport::ManualKeyExchange(const unsigned char * remote_public_key, const size_t public_key_length,
                                              const unsigned char * local_private_key, const size_t private_key_length)
{
    CHIP_ERROR err  = CHIP_NO_ERROR;
    size_t info_len = strlen(kManualKeyExchangeChannelInfo);

    VerifyOrExit(mState == State::kConnected, err = CHIP_ERROR_INCORRECT_STATE);

    err = mSecureChannel.Init(remote_public_key, public_key_length, local_private_key, private_key_length, NULL, 0,
                              (const unsigned char *) kManualKeyExchangeChannelInfo, info_len);
    SuccessOrExit(err);
    mState = State::kSecureConnected;

exit:
    return err;
}

CHIP_ERROR SecureTransport::SendMessage(NodeId peerNodeId, System::PacketBuffer * msgBuf)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(StateAllowsSend(), err = CHIP_ERROR_INCORRECT_STATE);

    VerifyOrExit(msgBuf != NULL, err = CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrExit(msgBuf != NULL, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(msgBuf->Next() == NULL, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);
    VerifyOrExit(msgBuf->TotalLength() < kMax_SecureSDU_Length, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    {
        uint8_t * plainText = msgBuf->Start();
        size_t plainTextlen = msgBuf->TotalLength();

        uint8_t * encryptedText = plainText - CHIP_SYSTEM_CRYPTO_HEADER_RESERVE_SIZE;
        size_t encryptedLen     = plainTextlen + CHIP_SYSTEM_CRYPTO_HEADER_RESERVE_SIZE;

        err = mSecureChannel.Encrypt(plainText, plainTextlen, encryptedText, encryptedLen);
        SuccessOrExit(err);

        msgBuf->SetStart(encryptedText);

        ChipLogProgress(Inet, "Secure transport transmitting msg %u after encryption", mConnectionState.GetSendMessageIndex());
    }

    {
        MessageHeader header;

        header
            .SetSourceNodeId(mLocalNodeId)    //
            .SetDestinationNodeId(peerNodeId) //
            .SetMessageId(mConnectionState.GetSendMessageIndex());

        err    = mTransport.SendMessage(header, mConnectionState.GetPeerAddress(), msgBuf);
        msgBuf = NULL;
    }
    SuccessOrExit(err);

    mConnectionState.IncrementSendMessageIndex();

exit:
    if (msgBuf != NULL)
    {
        ChipLogProgress(Inet, "Secure transport failed to encrypt msg %u: %s", mConnectionState.GetSendMessageIndex(),
                        ErrorStr(err));
        PacketBuffer::Free(msgBuf);
        msgBuf = NULL;
    }

    return err;
}

void SecureTransport::HandleDataReceived(const MessageHeader & header, const IPPacketInfo & pktInfo, System::PacketBuffer * msg,
                                         SecureTransport * connection)
{
    // TODO: actual key exchange should happen here
    if (!connection->StateAllowsReceive())
    {
        if (connection->OnNewConnection)
        {
            connection->OnNewConnection(header, pktInfo, connection->mNewConnectionArgument);
        }
    }

    // TODO this is where messages should be decoded
    if (connection->StateAllowsReceive() && msg != nullptr)
    {
        uint8_t * encryptedText = msg->Start();
        uint16_t encryptedLen   = msg->TotalLength();

        if (encryptedLen < CHIP_SYSTEM_CRYPTO_HEADER_RESERVE_SIZE)
        {
            PacketBuffer::Free(msg);
            ChipLogProgress(Inet, "Secure transport received smaller than encryption header, discarding");
            return;
        }

        chip::System::PacketBuffer * origMsg = nullptr;
#if CHIP_SYSTEM_CONFIG_USE_LWIP
        /* This is a workaround for the case where PacketBuffer payload is not allocated
           as an inline buffer to PacketBuffer structure */
        origMsg = msg;
        msg     = PacketBuffer::NewWithAvailableSize(encryptedLen);
        msg->SetDataLength(encryptedLen, msg);
#endif

        uint8_t * plainText = msg->Start() + CHIP_SYSTEM_CRYPTO_HEADER_RESERVE_SIZE;
        size_t plainTextlen = encryptedLen - CHIP_SYSTEM_CRYPTO_HEADER_RESERVE_SIZE;

        CHIP_ERROR err = connection->mSecureChannel.Decrypt(encryptedText, encryptedLen, plainText, plainTextlen);

        if (origMsg != nullptr)
        {
            PacketBuffer::Free(origMsg);
        }

        if (err == CHIP_NO_ERROR)
        {
            msg->Consume(CHIP_SYSTEM_CRYPTO_HEADER_RESERVE_SIZE);
            if (connection->OnMessageReceived)
            {
                connection->OnMessageReceived(header, pktInfo, msg, connection->mMessageReceivedArgument);
            }
        }
        else
        {
            PacketBuffer::Free(msg);
            ChipLogProgress(Inet, "Secure transport failed to decrypt msg: err %d", err);
        }
    }
    else if (msg != nullptr)
    {
        PacketBuffer::Free(msg);
        ChipLogProgress(Inet, "Secure transport failed: state not allows receive");
    }
    else
    {
        ChipLogError(Inet, "Not ready to process new messages");
    }
}

} // namespace chip
