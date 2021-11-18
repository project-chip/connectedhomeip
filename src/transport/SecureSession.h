/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 * @brief Defines state relevant for an active connection to a peer.
 */

#pragma once

#include <app/util/basic-types.h>
#include <messaging/ReliableMessageProtocolConfig.h>
#include <transport/CryptoContext.h>
#include <transport/SessionMessageCounter.h>
#include <transport/raw/Base.h>
#include <transport/raw/MessageHeader.h>
#include <transport/raw/PeerAddress.h>

namespace chip {
namespace Transport {

static constexpr uint32_t kUndefinedMessageIndex = UINT32_MAX;

/**
 * Defines state of a peer connection at a transport layer.
 *
 * Information contained within the state:
 *   - PeerAddress represents how to talk to the peer
 *   - PeerNodeId is the unique ID of the peer
 *   - SendMessageIndex is an ever increasing index for sending messages
 *   - LastActivityTime is a monotonic timestamp of when this connection was
 *     last used. Inactive connections can expire.
 *   - CryptoContext contains the encryption context of a connection
 *
 * TODO: to add any message ACK information
 */
class SecureSession
{
public:
    SecureSession(uint16_t localSessionId, NodeId peerNodeId, uint16_t peerSessionId, FabricIndex fabric,
                  ReliableMessageProtocolConfig config, System::Clock::Timestamp currentTime) :
        mPeerNodeId(peerNodeId),
        mLocalSessionId(localSessionId), mPeerSessionId(peerSessionId), mFabric(fabric), mMRPConfig(config)
    {
        SetLastActivityTime(currentTime);
    }

    SecureSession(SecureSession &&)      = delete;
    SecureSession(const SecureSession &) = delete;
    SecureSession & operator=(const SecureSession &) = delete;
    SecureSession & operator=(SecureSession &&) = delete;

    const PeerAddress & GetPeerAddress() const { return mPeerAddress; }
    PeerAddress & GetPeerAddress() { return mPeerAddress; }
    void SetPeerAddress(const PeerAddress & address) { mPeerAddress = address; }

    NodeId GetPeerNodeId() const { return mPeerNodeId; }

    void SetMRPConfig(ReliableMessageProtocolConfig config) { mMRPConfig = config; }

    ReliableMessageProtocolConfig GetMRPConfig() const { return mMRPConfig; }

    uint16_t GetLocalSessionId() const { return mLocalSessionId; }
    uint16_t GetPeerSessionId() const { return mPeerSessionId; }
    FabricIndex GetFabricIndex() const { return mFabric; }

    System::Clock::Timestamp GetLastActivityTime() const { return mLastActivityTime; }
    void SetLastActivityTime(System::Clock::Timestamp value) { mLastActivityTime = value; }

    CryptoContext & GetCryptoContext() { return mCryptoContext; }

    CHIP_ERROR EncryptBeforeSend(const uint8_t * input, size_t input_length, uint8_t * output, PacketHeader & header,
                                 MessageAuthenticationCode & mac) const
    {
        return mCryptoContext.Encrypt(input, input_length, output, header, mac);
    }

    CHIP_ERROR DecryptOnReceive(const uint8_t * input, size_t input_length, uint8_t * output, const PacketHeader & header,
                                const MessageAuthenticationCode & mac) const
    {
        return mCryptoContext.Decrypt(input, input_length, output, header, mac);
    }

    SessionMessageCounter & GetSessionMessageCounter() { return mSessionMessageCounter; }

private:
    const NodeId mPeerNodeId;
    const uint16_t mLocalSessionId;
    const uint16_t mPeerSessionId;
    const FabricIndex mFabric;

    PeerAddress mPeerAddress;
    System::Clock::Timestamp mLastActivityTime;
    ReliableMessageProtocolConfig mMRPConfig;
    CryptoContext mCryptoContext;
    SessionMessageCounter mSessionMessageCounter;
};

} // namespace Transport
} // namespace chip
