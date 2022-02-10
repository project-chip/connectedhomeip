/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
#include <credentials/CHIPCert.h>
#include <messaging/ReliableMessageProtocolConfig.h>
#include <transport/CryptoContext.h>
#include <transport/Session.h>
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
 *   - SecureSessionType represents CASE or PASE session
 *   - PeerAddress represents how to talk to the peer
 *   - PeerNodeId is the unique ID of the peer
 *   - PeerCATs represents CASE Authenticated Tags
 *   - SendMessageIndex is an ever increasing index for sending messages
 *   - LastActivityTime is a monotonic timestamp of when this connection was
 *     last used. Inactive connections can expire.
 *   - CryptoContext contains the encryption context of a connection
 */
class SecureSession : public Session
{
public:
    /**
     *  @brief
     *    Defines SecureSession Type. Currently supported types are PASE and CASE.
     */
    enum class Type : uint8_t
    {
        kUndefined = 0,
        kPASE      = 1,
        kCASE      = 2,
    };

    SecureSession(Type secureSessionType, uint16_t localSessionId, NodeId peerNodeId, CATValues peerCATs, uint16_t peerSessionId,
                  FabricIndex fabric, const ReliableMessageProtocolConfig & config) :
        mSecureSessionType(secureSessionType),
        mPeerNodeId(peerNodeId), mPeerCATs(peerCATs), mLocalSessionId(localSessionId), mPeerSessionId(peerSessionId),
        mLastActivityTime(System::SystemClock().GetMonotonicTimestamp()), mMRPConfig(config)
    {
        SetFabricIndex(fabric);
    }
    ~SecureSession() { NotifySessionReleased(); }

    SecureSession(SecureSession &&)      = delete;
    SecureSession(const SecureSession &) = delete;
    SecureSession & operator=(const SecureSession &) = delete;
    SecureSession & operator=(SecureSession &&) = delete;

    Session::SessionType GetSessionType() const override { return Session::SessionType::kSecure; }
#if CHIP_PROGRESS_LOGGING
    const char * GetSessionTypeString() const override { return "secure"; };
#endif

    Access::SubjectDescriptor GetSubjectDescriptor() const override;

    bool RequireMRP() const override { return GetPeerAddress().GetTransportType() == Transport::Type::kUdp; }

    System::Clock::Milliseconds32 GetAckTimeout() const override
    {
        switch (mPeerAddress.GetTransportType())
        {
        case Transport::Type::kUdp:
            return GetMRPConfig().mIdleRetransTimeout * (CHIP_CONFIG_RMP_DEFAULT_MAX_RETRANS + 1);
        case Transport::Type::kTcp:
            return System::Clock::Seconds16(30);
        default:
            break;
        }
        return System::Clock::Timeout();
    }

    const PeerAddress & GetPeerAddress() const { return mPeerAddress; }
    void SetPeerAddress(const PeerAddress & address) { mPeerAddress = address; }

    Type GetSecureSessionType() const { return mSecureSessionType; }
    NodeId GetPeerNodeId() const { return mPeerNodeId; }
    CATValues GetPeerCATs() const { return mPeerCATs; }

    void SetMRPConfig(const ReliableMessageProtocolConfig & config) { mMRPConfig = config; }

    const ReliableMessageProtocolConfig & GetMRPConfig() const override { return mMRPConfig; }

    uint16_t GetLocalSessionId() const { return mLocalSessionId; }
    uint16_t GetPeerSessionId() const { return mPeerSessionId; }

    // Should only be called for PASE sessions, which start with undefined fabric,
    // to migrate to a newly commissioned fabric after successful
    // OperationalCredentialsCluster::AddNOC
    CHIP_ERROR NewFabric(FabricIndex fabricIndex)
    {
#if 0
        // TODO(#13711): this check won't work until the issue is addressed
        if (mSecureSessionType == Type::kPASE)
        {
            SetFabricIndex(fabricIndex);
        }
#else
        SetFabricIndex(fabricIndex);
#endif
        return CHIP_NO_ERROR;
    }

    System::Clock::Timestamp GetLastActivityTime() const { return mLastActivityTime; }
    void MarkActive() { mLastActivityTime = System::SystemClock().GetMonotonicTimestamp(); }

    CryptoContext & GetCryptoContext() { return mCryptoContext; }

    SessionMessageCounter & GetSessionMessageCounter() { return mSessionMessageCounter; }

private:
    const Type mSecureSessionType;
    const NodeId mPeerNodeId;
    const CATValues mPeerCATs;
    const uint16_t mLocalSessionId;
    const uint16_t mPeerSessionId;

    PeerAddress mPeerAddress;
    System::Clock::Timestamp mLastActivityTime;
    ReliableMessageProtocolConfig mMRPConfig;
    CryptoContext mCryptoContext;
    SessionMessageCounter mSessionMessageCounter;
};

} // namespace Transport
} // namespace chip
