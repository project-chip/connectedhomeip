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
 *   - LastActivityTimeMs is a monotonic timestamp of when this connection was
 *     last used. Inactive connections can expire.
 *   - CryptoContext contains the encryption context of a connection
 *
 * TODO: to add any message ACK information
 */
class SecureSession
{
public:
    SecureSession() : mPeerAddress(PeerAddress::Uninitialized()) {}
    SecureSession(const PeerAddress & addr) : mPeerAddress(addr) {}
    SecureSession(PeerAddress && addr) : mPeerAddress(addr) {}

    SecureSession(SecureSession &&)      = default;
    SecureSession(const SecureSession &) = default;
    SecureSession & operator=(const SecureSession &) = default;
    SecureSession & operator=(SecureSession &&) = default;

    const PeerAddress & GetPeerAddress() const { return mPeerAddress; }
    PeerAddress & GetPeerAddress() { return mPeerAddress; }
    void SetPeerAddress(const PeerAddress & address) { mPeerAddress = address; }

    NodeId GetPeerNodeId() const { return mPeerNodeId; }
    void SetPeerNodeId(NodeId peerNodeId) { mPeerNodeId = peerNodeId; }

    uint16_t GetPeerSessionId() const { return mPeerSessionId; }
    void SetPeerSessionId(uint16_t id) { mPeerSessionId = id; }

    // TODO: Rename KeyID to SessionID
    uint16_t GetLocalSessionId() const { return mLocalSessionId; }
    void SetLocalSessionId(uint16_t id) { mLocalSessionId = id; }

    uint64_t GetLastActivityTimeMs() const { return mLastActivityTimeMs; }
    void SetLastActivityTimeMs(uint64_t value) { mLastActivityTimeMs = value; }

    CryptoContext & GetCryptoContext() { return mCryptoContext; }

    FabricIndex GetFabricIndex() const { return mFabric; }
    void SetFabricIndex(FabricIndex fabricIndex) { mFabric = fabricIndex; }

    bool IsInitialized()
    {
        return (mPeerAddress.IsInitialized() || mPeerNodeId != kUndefinedNodeId || mPeerSessionId != UINT16_MAX ||
                mLocalSessionId != UINT16_MAX);
    }

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
    PeerAddress mPeerAddress;
    NodeId mPeerNodeId           = kUndefinedNodeId;
    uint16_t mPeerSessionId      = UINT16_MAX;
    uint16_t mLocalSessionId     = UINT16_MAX;
    uint64_t mLastActivityTimeMs = 0;
    CryptoContext mCryptoContext;
    SessionMessageCounter mSessionMessageCounter;
    FabricIndex mFabric = kUndefinedFabricIndex;
};

} // namespace Transport
} // namespace chip
