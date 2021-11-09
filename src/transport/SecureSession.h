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
#include <lib/support/Serializer.h>
#include <transport/CryptoContext.h>
#include <transport/SessionMessageCounter.h>
#include <transport/raw/Base.h>
#include <transport/raw/MessageHeader.h>
#include <transport/raw/PeerAddress.h>

namespace chip {
namespace Transport {

static constexpr uint32_t kUndefinedMessageIndex = UINT32_MAX;

/**
 * Defines state of a secure session at a transport layer.
 *
 * Information contained within the state:
 *   - PeerAddress represents how to talk to the peer
 *   - PeerNodeId is the unique ID of the peer
 *   - LastActivityTime is a monotonic timestamp of when this connection was last used. Inactive connections can expire.
 *   - CryptoContext contains the encryption context of a connection
 */
class SecureSession
{
public:
    SecureSession(uint16_t localSessionId, NodeId peerNodeId, uint16_t peerSessionId, FabricIndex fabric,
                  System::Clock::Timestamp currentTime) :
        mPeerNodeId(peerNodeId),
        mLocalSessionId(localSessionId), mPeerSessionId(peerSessionId), mFabric(fabric)
    {
        SetLastActivityTime(currentTime);
    }

    SecureSession(uint16_t localSessionId, NodeId peerNodeId, uint16_t peerSessionId, FabricIndex fabric,
                  CryptoContext && cryptoContext, uint32_t localSessionMessageCounterValue,
                  uint32_t peerSessionMessageCounterValue) :
        mPeerNodeId(peerNodeId),
        mLocalSessionId(localSessionId), mPeerSessionId(peerSessionId), mFabric(fabric), mCryptoContext(std::move(cryptoContext)),
        mSessionMessageCounter(localSessionMessageCounterValue, peerSessionMessageCounterValue)
    {
        SetLastActivityTime(System::Clock::kZero);
    }

    SecureSession(SecureSession &&)      = default;
    SecureSession(const SecureSession &) = delete;
    SecureSession & operator=(const SecureSession &) = delete;
    SecureSession & operator=(SecureSession &&) = delete;

    const PeerAddress & GetPeerAddress() const { return mPeerAddress; }
    PeerAddress & GetPeerAddress() { return mPeerAddress; }
    void SetPeerAddress(const PeerAddress & address) { mPeerAddress = address; }

    NodeId GetPeerNodeId() const { return mPeerNodeId; }

    void GetMRPIntervals(uint32_t & idleInterval, uint32_t & activeInterval)
    {
        idleInterval   = mMRPIdleInterval;
        activeInterval = mMRPActiveInterval;
    }

    void SetMRPIntervals(uint32_t idleInterval, uint32_t activeInterval)
    {
        mMRPIdleInterval   = idleInterval;
        mMRPActiveInterval = activeInterval;
    }

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

    class Serializable
    {
    public:
        NodeId mPeerNodeId;
        uint16_t mLocalSessionId;
        uint16_t mPeerSessionId;
        FabricIndex mFabric;

        CryptoContext::Serializable mCryptoContext;
        uint32_t mLocalSessionMessageCounterValue;
        uint32_t mPeerSessionMessageCounterValue;

        using Serializer =
            chip::Serializer<Serializable, FieldSerializer<Serializable, NodeId, &Serializable::mPeerNodeId>,
                             FieldSerializer<Serializable, uint16_t, &Serializable::mLocalSessionId>,
                             FieldSerializer<Serializable, uint16_t, &Serializable::mPeerSessionId>,
                             FieldSerializer<Serializable, FabricIndex, &Serializable::mFabric>,
                             FieldSerializer<Serializable, CryptoContext::Serializable, &Serializable::mCryptoContext>,
                             FieldSerializer<Serializable, uint32_t, &Serializable::mLocalSessionMessageCounterValue>,
                             FieldSerializer<Serializable, uint32_t, &Serializable::mPeerSessionMessageCounterValue>>;
    };

    static_assert(std::is_pod<Serializable>::value, "SecureSession::Serializable is not a POD");

    Serializable Save() const
    {
        Serializable result;
        result.mPeerNodeId     = mPeerNodeId;
        result.mPeerSessionId  = mPeerSessionId;
        result.mLocalSessionId = mLocalSessionId;
        result.mFabric         = mFabric;

        result.mCryptoContext                   = mCryptoContext.Save();
        result.mLocalSessionMessageCounterValue = mSessionMessageCounter.GetLocalMessageCounter().Value();
        result.mPeerSessionMessageCounterValue  = mSessionMessageCounter.GetPeerMessageCounter().GetCounter();

        return result;
    }

    static SecureSession Load(const Serializable & from)
    {
        return SecureSession(from.mLocalSessionId, from.mPeerNodeId, from.mPeerSessionId, from.mFabric,
                             CryptoContext::Load(from.mCryptoContext), from.mLocalSessionMessageCounterValue,
                             from.mPeerSessionMessageCounterValue);
    }

private:
    const NodeId mPeerNodeId;
    const uint16_t mLocalSessionId;
    const uint16_t mPeerSessionId;
    const FabricIndex mFabric;

    PeerAddress mPeerAddress;
    System::Clock::Timestamp mLastActivityTime = System::Clock::kZero;
    uint32_t mMRPIdleInterval                  = 0;
    uint32_t mMRPActiveInterval                = 0;
    CryptoContext mCryptoContext;
    SessionMessageCounter mSessionMessageCounter;
};

} // namespace Transport
} // namespace chip
