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
        // kPending denotes a secure session object that is internally
        // reserved by the stack before and during session establishment.
        //
        // Although the stack can tolerate eviction of these (releasing one
        // out from under the holder would exhibit as CHIP_ERROR_INCORRECT_STATE
        // during CASE or PASE), intent is that we should not and would leave
        // these untouched until CASE or PASE complete.
        kPending = 3,
    };

    SecureSession(Type secureSessionType, uint16_t localSessionId, NodeId peerNodeId, CATValues peerCATs, uint16_t peerSessionId,
                  FabricIndex fabric, const ReliableMessageProtocolConfig & config) :
        mSecureSessionType(secureSessionType),
        mPeerNodeId(peerNodeId), mPeerCATs(peerCATs), mLocalSessionId(localSessionId), mPeerSessionId(peerSessionId),
        mLastActivityTime(System::SystemClock().GetMonotonicTimestamp()),
        mLastPeerActivityTime(System::SystemClock().GetMonotonicTimestamp()), mMRPConfig(config)
    {
        SetFabricIndex(fabric);
    }

    /**
     * @brief
     *   Construct a secure session object to associate with a pending secure
     *   session establishment attempt.  The object for the pending session
     *   receives a local session ID, but no other state.
     */
    SecureSession(uint16_t localSessionId) :
        SecureSession(Type::kPending, localSessionId, kUndefinedNodeId, CATValues{}, 0, kUndefinedFabricIndex, GetLocalMRPConfig())
    {}

    /**
     * @brief
     *   Activate a pending Secure Session that had been reserved during CASE or
     *   PASE, setting internal state according to the parameters used and
     *   discovered during session establishment.
     */
    void Activate(Type secureSessionType, NodeId peerNodeId, CATValues peerCATs, uint16_t peerSessionId, FabricIndex fabric,
                  const ReliableMessageProtocolConfig & config)
    {
        mSecureSessionType = secureSessionType;
        mPeerNodeId        = peerNodeId;
        mPeerCATs          = peerCATs;
        mPeerSessionId     = peerSessionId;
        mMRPConfig         = config;
        SetFabricIndex(fabric);
    }
    ~SecureSession() override { NotifySessionReleased(); }

    SecureSession(SecureSession &&)      = delete;
    SecureSession(const SecureSession &) = delete;
    SecureSession & operator=(const SecureSession &) = delete;
    SecureSession & operator=(SecureSession &&) = delete;

    Session::SessionType GetSessionType() const override { return Session::SessionType::kSecure; }
#if CHIP_PROGRESS_LOGGING
    const char * GetSessionTypeString() const override { return "secure"; };
#endif

    ScopedNodeId GetPeer() const override;
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
    System::Clock::Timestamp GetLastPeerActivityTime() const { return mLastPeerActivityTime; }
    void MarkActive() { mLastActivityTime = System::SystemClock().GetMonotonicTimestamp(); }
    void MarkActiveRx()
    {
        mLastPeerActivityTime = System::SystemClock().GetMonotonicTimestamp();
        MarkActive();
    }

    bool IsPeerActive() { return ((System::SystemClock().GetMonotonicTimestamp() - GetLastPeerActivityTime()) < kMinActiveTime); }

    System::Clock::Timestamp GetMRPBaseTimeout() override
    {
        return IsPeerActive() ? GetMRPConfig().mActiveRetransTimeout : GetMRPConfig().mIdleRetransTimeout;
    }

    CryptoContext & GetCryptoContext() { return mCryptoContext; }

    SessionMessageCounter & GetSessionMessageCounter() { return mSessionMessageCounter; }

private:
    Type mSecureSessionType;
    NodeId mPeerNodeId;
    CATValues mPeerCATs;
    const uint16_t mLocalSessionId;
    uint16_t mPeerSessionId;

    PeerAddress mPeerAddress;
    System::Clock::Timestamp mLastActivityTime;     ///< Timestamp of last tx or rx
    System::Clock::Timestamp mLastPeerActivityTime; ///< Timestamp of last rx
    ReliableMessageProtocolConfig mMRPConfig;
    CryptoContext mCryptoContext;
    SessionMessageCounter mSessionMessageCounter;
};

} // namespace Transport
} // namespace chip
