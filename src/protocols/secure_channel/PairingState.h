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

#pragma once

#include <transport/AdminPairingTable.h>
#include <transport/SecureSession.h>
#include <transport/SessionMessageCounter.h>
#include <transport/raw/Base.h>
#include <transport/raw/MessageHeader.h>
#include <transport/raw/PeerAddress.h>

namespace chip {

/**
 * Defines state of a pairing session
 */
class PairingState
{
public:
    PairingState() : mPeerAddress(Transport::PeerAddress::Uninitialized()) {}
    PairingState(const Transport::PeerAddress & addr) : mPeerAddress(addr) {}
    PairingState(Transport::PeerAddress && addr) : mPeerAddress(addr) {}

    PairingState(PairingState &&)      = default;
    PairingState(const PairingState &) = default;
    PairingState & operator=(const PairingState &) = default;
    PairingState & operator=(PairingState &&) = default;

    const Transport::PeerAddress & GetPeerAddress() const { return mPeerAddress; }
    Transport::PeerAddress & GetPeerAddress() { return mPeerAddress; }
    void SetPeerAddress(const Transport::PeerAddress & address) { mPeerAddress = address; }

    NodeId GetPeerNodeId() const { return mPeerNodeId; }
    void SetPeerNodeId(NodeId peerNodeId) { mPeerNodeId = peerNodeId; }

    uint16_t GetPeerKeyID() const { return mPeerKeyID; }
    void SetPeerKeyID(uint16_t id) { mPeerKeyID = id; }

    uint16_t GetLocalKeyID() const { return mLocalKeyID; }
    void SetLocalKeyID(uint16_t id) { mLocalKeyID = id; }

    void Reset()
    {
        mPeerAddress = Transport::PeerAddress::Uninitialized();
        mPeerNodeId  = kUndefinedNodeId;
        mPeerKeyID   = UINT16_MAX;
        mLocalKeyID  = UINT16_MAX;
    }

private:
    Transport::PeerAddress mPeerAddress = Transport::PeerAddress::Uninitialized();
    NodeId mPeerNodeId                  = kUndefinedNodeId;
    uint16_t mPeerKeyID                 = UINT16_MAX;
    uint16_t mLocalKeyID                = UINT16_MAX;
};

} // namespace chip
