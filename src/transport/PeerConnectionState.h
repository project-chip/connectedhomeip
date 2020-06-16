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

namespace chip {
namespace Transport {

/**
 * Defines state of a peer connection at a transport layer.
 *
 * Examples of connection specific state includes: message counters and ack,
 * timeouts, addressing information and encryption data.
 */
class PeerConnectionState
{
public:
    PeerConnectionState(const PeerAddress & addr) : mPeerAddress(addr) {}
    PeerConnectionState(PeerAddress && addr) : mPeerAddress(addr) {}

    const PeerAddress & GetPeerAddress() const { return mPeerAddress; }
    PeerAddress & GetPeerAddress() { return mPeerAddress; }
    void SetPeerAddress(const PeerAddress & address) { mPeerAddress = address; }

    NodeId GetPeerNodeId() const { return mPeerNodeId; }
    void SetPeerNodeId(NodeId peerNodeId) { mPeerNodeId = peerNodeId; }

    uint32_t GetSendMessageIndex() const { return mSendMessageIndex; }
    void IncrementSendMessageIndex() { mSendMessageIndex++; }

private:
    PeerAddress mPeerAddress;
    NodeId mPeerNodeId         = kUndefinedNodeId;
    uint32_t mSendMessageIndex = 0;
};

} // namespace Transport
} // namespace chip

#endif // PEER_CONNCTION_STATE_H_
