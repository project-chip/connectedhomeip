/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *      This file defines a common interface to access various types of secure
 *      pairing sessions (e.g. PASE, CASE)
 *
 */

#pragma once

#include <core/CHIPError.h>
#include <transport/SecureSession.h>

namespace chip {

class DLL_EXPORT PairingSession
{
public:
    PairingSession() {}
    virtual ~PairingSession() {}

    NodeId GetPeerNodeId() const { return mPeerNodeId; }
    void SetPeerNodeId(NodeId peerNodeId) { mPeerNodeId = peerNodeId; }

    uint16_t GetPeerKeyId() const { return mPeerKeyId; }
    void SetPeerKeyId(uint16_t id) { mPeerKeyId = id; }
    bool IsValidPeerKeyId() const { return mPeerKeyId != kInvalidKeyId; }

    uint16_t GetLocalKeyId() const { return mLocalKeyId; }
    void SetLocalKeyId(uint16_t id) { mLocalKeyId = id; }
    bool IsValidLocalKeyId() const { return mLocalKeyId != kInvalidKeyId; }

    const Transport::PeerAddress & GetPeerAddress() const { return mPeerAddress; }
    Transport::PeerAddress & GetPeerAddress() { return mPeerAddress; }
    void SetPeerAddress(const Transport::PeerAddress & address) { mPeerAddress = address; }

    void Clear()
    {
        mPeerAddress = Transport::PeerAddress::Uninitialized();
        mPeerKeyId   = kInvalidKeyId;
        mLocalKeyId  = kInvalidKeyId;
    }

    /**
     * @brief
     *   Derive a secure session from the paired session. The API will return error
     *   if called before pairing is established.
     *
     * @param session     Referene to the secure session that will be
     *                    initialized once pairing is complete
     * @param role        Role of the new session (initiator or responder)
     * @return CHIP_ERROR The result of session derivation
     */
    virtual CHIP_ERROR DeriveSecureSession(SecureSession & session, SecureSession::SessionRole role) = 0;

    /**
     * @brief
     *   Get the value of peer session counter which is synced during session establishment
     */
    virtual uint32_t GetPeerCounter()
    {
        // TODO(#6652): This is a stub implementation, should be replaced by the real one when CASE and PASE is completed
        return LocalSessionMessageCounter::kInitialValue;
    }

    virtual const char * GetI2RSessionInfo() const = 0;

    virtual const char * GetR2ISessionInfo() const = 0;

private:
    NodeId mPeerNodeId = kUndefinedNodeId;
    // TODO(#8206): Remove address and use peer cache instead.
    Transport::PeerAddress mPeerAddress     = Transport::PeerAddress::Uninitialized();
    static constexpr uint16_t kInvalidKeyId = UINT16_MAX;
    uint16_t mPeerKeyId                     = kInvalidKeyId;
    uint16_t mLocalKeyId                    = kInvalidKeyId;
};

} // namespace chip
