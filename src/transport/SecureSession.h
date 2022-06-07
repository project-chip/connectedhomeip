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
#include <lib/core/ReferenceCounted.h>
#include <messaging/ReliableMessageProtocolConfig.h>
#include <transport/CryptoContext.h>
#include <transport/Session.h>
#include <transport/SessionMessageCounter.h>
#include <transport/raw/PeerAddress.h>

namespace chip {
namespace Transport {

class SecureSessionTable;
class SecureSessionDeleter
{
public:
    static void Release(SecureSession * entry);
};

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
class SecureSession : public Session, public ReferenceCounted<SecureSession, SecureSessionDeleter, 0, uint16_t>
{
public:
    /**
     *  @brief
     *    Defines SecureSession Type. Currently supported types are PASE and CASE.
     */
    enum class Type : uint8_t
    {
        kPASE = 1,
        kCASE = 2,
    };

    // Test-only: inject a session in Active state.
    // TODO: Tests should allocate a pending session and then call Activate(), just like non-test code does.
    SecureSession(SecureSessionTable & table, Type secureSessionType, uint16_t localSessionId, NodeId localNodeId,
                  NodeId peerNodeId, CATValues peerCATs, uint16_t peerSessionId, FabricIndex fabric,
                  const ReliableMessageProtocolConfig & config) :
        mTable(table),
        mState(State::kActive), mSecureSessionType(secureSessionType), mLocalNodeId(localNodeId), mPeerNodeId(peerNodeId),
        mPeerCATs(peerCATs), mLocalSessionId(localSessionId), mPeerSessionId(peerSessionId), mMRPConfig(config)
    {
        Retain(); // Put the test session in Active state. This ref is released inside MarkForRemoval
        SetFabricIndex(fabric);
        ChipLogDetail(Inet, "SecureSession[%p]: Allocated for Test Type:%d LSID:%d", this, to_underlying(mSecureSessionType),
                      mLocalSessionId);
    }

    /**
     * @brief
     *   Construct a secure session object to associate with a pending secure
     *   session establishment attempt.  The object for the pending session
     *   receives a local session ID, but no other state.
     */
    SecureSession(SecureSessionTable & table, Type secureSessionType, uint16_t localSessionId) :
        mTable(table), mState(State::kPairing), mSecureSessionType(secureSessionType), mLocalSessionId(localSessionId)
    {
        ChipLogDetail(Inet, "SecureSession[%p]: Allocated Type:%d LSID:%d", this, to_underlying(mSecureSessionType),
                      mLocalSessionId);
    }

    /**
     * @brief
     *   Activate a pending Secure Session that had been reserved during CASE or
     *   PASE, setting internal state according to the parameters used and
     *   discovered during session establishment.
     */
    void Activate(const ScopedNodeId & localNode, const ScopedNodeId & peerNode, CATValues peerCATs, uint16_t peerSessionId,
                  const ReliableMessageProtocolConfig & config)
    {
        VerifyOrDie(mState == State::kPairing);
        VerifyOrDie(peerNode.GetFabricIndex() == localNode.GetFabricIndex());

        // PASE sessions must always start unassociated with a Fabric!
        VerifyOrDie(!((mSecureSessionType == Type::kPASE) && (peerNode.GetFabricIndex() != kUndefinedFabricIndex)));
        // CASE sessions must always start "associated" a given Fabric!
        VerifyOrDie(!((mSecureSessionType == Type::kCASE) && (peerNode.GetFabricIndex() == kUndefinedFabricIndex)));
        // CASE sessions can only be activated against operational node IDs!
        VerifyOrDie(!((mSecureSessionType == Type::kCASE) &&
                      (!IsOperationalNodeId(peerNode.GetNodeId()) || !IsOperationalNodeId(localNode.GetNodeId()))));

        mPeerNodeId    = peerNode.GetNodeId();
        mLocalNodeId   = localNode.GetNodeId();
        mPeerCATs      = peerCATs;
        mPeerSessionId = peerSessionId;
        mMRPConfig     = config;
        SetFabricIndex(peerNode.GetFabricIndex());

        Retain(); // This ref is released inside MarkForRemoval
        mState = State::kActive;
        ChipLogDetail(Inet, "SecureSession[%p]: Activated - Type:%d LSID:%d", this, to_underlying(mSecureSessionType),
                      mLocalSessionId);
    }
    ~SecureSession() override
    {
        ChipLogDetail(Inet, "SecureSession[%p]: Released - Type:%d LSID:%d", this, to_underlying(mSecureSessionType),
                      mLocalSessionId);
    }

    SecureSession(SecureSession &&)      = delete;
    SecureSession(const SecureSession &) = delete;
    SecureSession & operator=(const SecureSession &) = delete;
    SecureSession & operator=(SecureSession &&) = delete;

    void Retain() override;
    void Release() override;

    bool IsActiveSession() const override { return mState == State::kActive; }
    bool IsPairing() const { return mState == State::kPairing; }
    /// @brief Mark as pending removal, all holders to this session will be cleared, and disallow future grab
    void MarkForRemoval();

    Session::SessionType GetSessionType() const override { return Session::SessionType::kSecure; }
#if CHIP_PROGRESS_LOGGING
    const char * GetSessionTypeString() const override { return "secure"; };
#endif

    ScopedNodeId GetPeer() const override { return ScopedNodeId(mPeerNodeId, GetFabricIndex()); }

    ScopedNodeId GetLocalScopedNodeId() const override { return ScopedNodeId(mLocalNodeId, GetFabricIndex()); }

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
        case Transport::Type::kBle:
            // TODO: Figure out what this should be, but zero is not the right
            // answer.
            return System::Clock::Seconds16(5);
        default:
            break;
        }
        return System::Clock::Timeout();
    }

    const PeerAddress & GetPeerAddress() const { return mPeerAddress; }
    void SetPeerAddress(const PeerAddress & address) { mPeerAddress = address; }

    Type GetSecureSessionType() const { return mSecureSessionType; }
    bool IsCASESession() const { return GetSecureSessionType() == Type::kCASE; }
    bool IsPASESession() const { return GetSecureSessionType() == Type::kPASE; }
    NodeId GetPeerNodeId() const { return mPeerNodeId; }
    NodeId GetLocalNodeId() const { return mLocalNodeId; }

    CATValues GetPeerCATs() const { return mPeerCATs; }

    void SetMRPConfig(const ReliableMessageProtocolConfig & config) { mMRPConfig = config; }

    const ReliableMessageProtocolConfig & GetMRPConfig() const override { return mMRPConfig; }

    uint16_t GetLocalSessionId() const { return mLocalSessionId; }
    uint16_t GetPeerSessionId() const { return mPeerSessionId; }

    // Called when AddNOC has gone through sufficient success that we need to switch the
    // session to reflect a new fabric if it was a PASE session
    CHIP_ERROR AdoptFabricIndex(FabricIndex fabricIndex)
    {
        // It's not legal to augment session type for non-PASE
        if (mSecureSessionType != Type::kPASE)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        SetFabricIndex(fabricIndex);
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

    const CryptoContext & GetCryptoContext() const { return mCryptoContext; }

    SessionMessageCounter & GetSessionMessageCounter() { return mSessionMessageCounter; }

private:
    enum class State : uint8_t
    {
        // Although the stack can tolerate eviction of these (releasing one
        // out from under the holder would exhibit as CHIP_ERROR_INCORRECT_STATE
        // during CASE or PASE), intent is that we should not and would leave
        // these untouched until CASE or PASE complete.
        //
        // During this stage, the reference counter is hold by the PairingSession
        kPairing = 1,

        // The session is active, ready for use. During this stage, the
        // reference counter increased by 1 in Activate, and will be decreased
        // by 1 when MarkForRemoval is called.
        kActive = 2,

        // The session is pending for removal, all SessionHolders are already
        // cleared during MarkForRemoval, no future SessionHolder is able to
        // grab this session, when all SessionHandles go out of scope, the
        // session object will be released automatically.
        kPendingRemoval = 3,
    };

    friend class SecureSessionDeleter;
    SecureSessionTable & mTable;
    State mState;
    const Type mSecureSessionType;
    NodeId mLocalNodeId = kUndefinedNodeId;
    NodeId mPeerNodeId  = kUndefinedNodeId;
    CATValues mPeerCATs = CATValues{};
    const uint16_t mLocalSessionId;
    uint16_t mPeerSessionId = 0;

    PeerAddress mPeerAddress;
    System::Clock::Timestamp mLastActivityTime     = System::SystemClock().GetMonotonicTimestamp(); ///< Timestamp of last tx or rx
    System::Clock::Timestamp mLastPeerActivityTime = System::SystemClock().GetMonotonicTimestamp(); ///< Timestamp of last rx
    ReliableMessageProtocolConfig mMRPConfig       = GetLocalMRPConfig();
    CryptoContext mCryptoContext;
    SessionMessageCounter mSessionMessageCounter;
};

} // namespace Transport
} // namespace chip
