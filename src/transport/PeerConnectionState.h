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

#ifndef PEER_CONNCTION_STATE_H_
#define PEER_CONNCTION_STATE_H_

#include <transport/MessageHeader.h>
#include <transport/PeerAddress.h>
#include <transport/SecureSession.h>

namespace chip {
namespace Transport {

/**
 * Defines state of a peer connection at a transport layer.
 *
 * Information contained within the state:
 *   - PeerAddress represents how to talk to the peer
 *   - PeerNodeId is the unique ID of the peer
 *   - SendMessageIndex is an ever increasing index for sending messages
 *   - LastActivityTimeMs is a monotonic timestamp of when this connection was
 *     last used. Inactive connections can expire.
 *   - SecureSession contains the encryption context of a connection
 *
 * TODO: to add any message ACK information
 */
class PeerConnectionState
{
public:
    PeerConnectionState() : mPeerAddress(PeerAddress::Uninitialized()) {}
    PeerConnectionState(const PeerAddress & addr) : mPeerAddress(addr) {}
    PeerConnectionState(PeerAddress && addr) : mPeerAddress(addr) {}

    PeerConnectionState(PeerConnectionState &&)      = default;
    PeerConnectionState(const PeerConnectionState &) = default;
    PeerConnectionState & operator=(const PeerConnectionState &) = default;
    PeerConnectionState & operator=(PeerConnectionState &&) = default;

    const PeerAddress & GetPeerAddress() const { return mPeerAddress; }
    PeerAddress & GetPeerAddress() { return mPeerAddress; }
    void SetPeerAddress(const PeerAddress & address) { mPeerAddress = address; }

    NodeId GetPeerNodeId() const { return mPeerNodeId; }
    void SetPeerNodeId(NodeId peerNodeId) { mPeerNodeId = peerNodeId; }

    uint32_t GetSendMessageIndex() const { return mSendMessageIndex; }
    void IncrementSendMessageIndex() { mSendMessageIndex++; }

    uint32_t GetPeerKeyID() const { return mPeerKeyID; }
    void SetPeerKeyID(uint32_t id) { mPeerKeyID = id; }

    uint32_t GetLocalKeyID() const { return mLocalKeyID; }
    void SetLocalKeyID(uint32_t id) { mLocalKeyID = id; }

    uint64_t GetLastActivityTimeMs() const { return mLastActityTimeMs; }
    void SetLastActivityTimeMs(uint64_t value) { mLastActityTimeMs = value; }

    SecureSession & GetSecureSession() { return mSecureSession; }
    const SecureSession & GetSecureSession() const { return mSecureSession; }

    bool IsInitialized()
    {
        return (mPeerAddress.IsInitialized() || mPeerNodeId != kUndefinedNodeId || mPeerKeyID != UINT32_MAX ||
                mLocalKeyID != UINT32_MAX);
    }

    /**
     *  Reset the connection state to a completely uninitialized status.
     */
    void Reset()
    {
        mPeerAddress      = PeerAddress::Uninitialized();
        mPeerNodeId       = kUndefinedNodeId;
        mSendMessageIndex = 0;
        mLastActityTimeMs = 0;
        mSecureSession.Reset();
    }

private:
    PeerAddress mPeerAddress;
    NodeId mPeerNodeId         = kUndefinedNodeId;
    uint32_t mSendMessageIndex = 0;
    uint32_t mPeerKeyID        = UINT32_MAX;
    uint32_t mLocalKeyID       = UINT32_MAX;
    uint64_t mLastActityTimeMs = 0;
    SecureSession mSecureSession;
};

} // namespace Transport
} // namespace chip

#endif // PEER_CONNCTION_STATE_H_
