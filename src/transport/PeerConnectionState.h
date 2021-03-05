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

#include <transport/AdminPairingTable.h>
#include <transport/SecureSession.h>
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
 *   - SecureSession contains the encryption context of a connection
 *
 * TODO: to add any message ACK information
 */
class PeerConnectionState
{
public:
    PeerConnectionState() : mMsgCounterSynStatus(MsgCounterSyncStatus::NotSync), mPeerAddress(PeerAddress::Uninitialized()) {}
    PeerConnectionState(const PeerAddress & addr) : mPeerAddress(addr) {}
    PeerConnectionState(PeerAddress && addr) : mPeerAddress(addr) {}

    PeerConnectionState(PeerConnectionState &&)      = default;
    PeerConnectionState(const PeerConnectionState &) = default;
    PeerConnectionState & operator=(const PeerConnectionState &) = default;
    PeerConnectionState & operator=(PeerConnectionState &&) = default;

    const PeerAddress & GetPeerAddress() const { return mPeerAddress; }
    PeerAddress & GetPeerAddress() { return mPeerAddress; }
    void SetPeerAddress(const PeerAddress & address) { mPeerAddress = address; }

    void SetTransport(Transport::Base * transport) { mTransport = transport; }
    Transport::Base * GetTransport() { return mTransport; }

    bool IsPeerMsgCounterSynced() { return (mPeerMessageIndex != kUndefinedMessageIndex); }
    void SetPeerMessageIndex(uint32_t id) { mPeerMessageIndex = id; }

    NodeId GetPeerNodeId() const { return mPeerNodeId; }
    void SetPeerNodeId(NodeId peerNodeId) { mPeerNodeId = peerNodeId; }

    uint32_t GetSendMessageIndex() const { return mSendMessageIndex; }
    void IncrementSendMessageIndex() { mSendMessageIndex++; }

    uint16_t GetPeerKeyID() const { return mPeerKeyID; }
    void SetPeerKeyID(uint16_t id) { mPeerKeyID = id; }

    uint16_t GetLocalKeyID() const { return mLocalKeyID; }
    void SetLocalKeyID(uint16_t id) { mLocalKeyID = id; }

    uint64_t GetLastActivityTimeMs() const { return mLastActivityTimeMs; }
    void SetLastActivityTimeMs(uint64_t value) { mLastActivityTimeMs = value; }

    SecureSession & GetSenderSecureSession() { return mSenderSecureSession; }
    SecureSession & GetReceiverSecureSession() { return mReceiverSecureSession; }

    Transport::AdminId GetAdminId() const { return mAdmin; }
    void SetAdminId(Transport::AdminId admin) { mAdmin = admin; }

    void SetMsgCounterSyncInProgress(bool value)
    {
        mMsgCounterSynStatus = value ? MsgCounterSyncStatus::SyncInProcess : MsgCounterSyncStatus::Synced;
    }

    bool IsInitialized()
    {
        return (mPeerAddress.IsInitialized() || mPeerNodeId != kUndefinedNodeId || mPeerKeyID != UINT16_MAX ||
                mLocalKeyID != UINT16_MAX);
    }

    bool IsMsgCounterSyncInProgress() { return mMsgCounterSynStatus == MsgCounterSyncStatus::SyncInProcess; }

    /**
     *  Reset the connection state to a completely uninitialized status.
     */
    void Reset()
    {
        mPeerAddress        = PeerAddress::Uninitialized();
        mPeerNodeId         = kUndefinedNodeId;
        mSendMessageIndex   = 0;
        mLastActivityTimeMs = 0;
        mSenderSecureSession.Reset();
        mReceiverSecureSession.Reset();
        mMsgCounterSynStatus = MsgCounterSyncStatus::NotSync;
    }

    CHIP_ERROR EncryptBeforeSend(const uint8_t * input, size_t input_length, uint8_t * output, PacketHeader & header,
                                 MessageAuthenticationCode & mac) const
    {
        return mSenderSecureSession.Encrypt(input, input_length, output, header, mac);
    }

    CHIP_ERROR DecryptOnReceive(const uint8_t * input, size_t input_length, uint8_t * output, const PacketHeader & header,
                                const MessageAuthenticationCode & mac) const
    {
        return mReceiverSecureSession.Decrypt(input, input_length, output, header, mac);
    }

private:
    enum class MsgCounterSyncStatus
    {
        NotSync,
        SyncInProcess,
        Synced,
    } mMsgCounterSynStatus;

    PeerAddress mPeerAddress;
    NodeId mPeerNodeId           = kUndefinedNodeId;
    uint32_t mSendMessageIndex   = 0;
    uint32_t mPeerMessageIndex   = kUndefinedMessageIndex;
    uint16_t mPeerKeyID          = UINT16_MAX;
    uint16_t mLocalKeyID         = UINT16_MAX;
    uint64_t mLastActivityTimeMs = 0;
    Transport::Base * mTransport = nullptr;
    SecureSession mSenderSecureSession;
    SecureSession mReceiverSecureSession;
    Transport::AdminId mAdmin = kUndefinedAdminId;
};

} // namespace Transport
} // namespace chip
