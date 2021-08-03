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

#include <transport/FabricTable.h>
#include <transport/PeerCache.h>
#include <transport/SecureSession.h>
#include <transport/SecureSessionHandle.h>
#include <transport/SessionMessageCounter.h>
#include <transport/raw/Base.h>
#include <transport/raw/MessageHeader.h>

namespace chip {
namespace Transport {

static constexpr uint32_t kUndefinedMessageIndex = UINT32_MAX;

/**
 * Defines state of a peer connection at a transport layer.
 *
 * Information contained within the state:
 *   - PeerHandle holds node id, admin id, address of the peer.
 *   - LastActivityTimeMs is a monotonic timestamp of when this connection was last used. Inactive connections can expire.
 *   - SecureSession contains the encryption context of a connection
 */
class PeerConnectionState
{
public:
    PeerConnectionState(PeerCacheEntry & peer, FabricIndex fabric, uint16_t peerKeyID, uint16_t localKeyID) :
        mPeer(peer), mFabric(fabric), mPeerKeyID(peerKeyID), mLocalKeyID(localKeyID)
    {}

    PeerConnectionState(const PeerConnectionState &) = delete;
    PeerConnectionState & operator=(const PeerConnectionState &) = delete;
    PeerConnectionState(PeerConnectionState &&)                  = delete;
    PeerConnectionState & operator=(PeerConnectionState &&) = delete;

    SecureSessionHandle ToSessionHandle() const
    {
        return SecureSessionHandle{ GetPeerInfo().GetPeer().GetNodeId(), GetPeerKeyID(), GetFabricIndex() };
    }

    FabricIndex GetFabricIndex() const { return mFabric; }

    PeerCacheEntry & GetPeerInfo() const { return mPeer.Get(); }
    uint16_t GetPeerKeyID() const { return mPeerKeyID; }

    // TODO: Rename KeyID to SessionID
    uint16_t GetLocalKeyID() const { return mLocalKeyID; }

    uint64_t GetLastActivityTimeMs() const { return mLastActivityTimeMs; }
    void SetLastActivityTimeMs(uint64_t value) { mLastActivityTimeMs = value; }

    SecureSession & GetSecureSession() { return mSecureSession; }

    CHIP_ERROR EncryptBeforeSend(const uint8_t * input, size_t input_length, uint8_t * output, PacketHeader & header,
                                 MessageAuthenticationCode & mac) const
    {
        return mSecureSession.Encrypt(input, input_length, output, header, mac);
    }

    CHIP_ERROR DecryptOnReceive(const uint8_t * input, size_t input_length, uint8_t * output, const PacketHeader & header,
                                const MessageAuthenticationCode & mac) const
    {
        return mSecureSession.Decrypt(input, input_length, output, header, mac);
    }

    SessionMessageCounter & GetSessionMessageCounter() { return mSessionMessageCounter; }

private:
    const PeerHandle mPeer;
    const FabricIndex mFabric;
    const uint16_t mPeerKeyID;
    const uint16_t mLocalKeyID;
    uint64_t mLastActivityTimeMs = 0;
    SecureSession mSecureSession;
    SessionMessageCounter mSessionMessageCounter;
};

} // namespace Transport
} // namespace chip
