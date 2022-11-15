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
#include <ble/BleConfig.h>
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
        mState(State::kEstablishing), mSecureSessionType(secureSessionType), mLocalNodeId(localNodeId), mPeerNodeId(peerNodeId),
        mPeerCATs(peerCATs), mLocalSessionId(localSessionId), mPeerSessionId(peerSessionId), mRemoteMRPConfig(config)
    {
        MoveToState(State::kActive);
        Retain(); // Put the test session in Active state. This ref is released inside MarkForEviction
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
        mTable(table), mState(State::kEstablishing), mSecureSessionType(secureSessionType), mLocalSessionId(localSessionId)
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
                  const ReliableMessageProtocolConfig & config);

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
    bool IsEstablishing() const { return mState == State::kEstablishing; }
    bool IsPendingEviction() const { return mState == State::kPendingEviction; }
    bool IsDefunct() const { return mState == State::kDefunct; }
    const char * GetStateStr() const { return StateToString(mState); }

    /*
     * This marks the session for eviction. It will first detach all SessionHolders attached to this
     * session by calling 'OnSessionReleased' on each of them. This will force them to release their reference
     * to the session. If there are no more references left, the session will then be de-allocated.
     *
     * Once marked for eviction, the session SHALL NOT ever become active again.
     *
     */
    void MarkForEviction();

    /*
     * This marks a previously active session as defunct to temporarily prevent it from being used with
     * new exchanges to send or receive messages on this session. This should be called when there is suspicion of
     * a loss-of-sync with the session state on the associated peer. This could arise if there is evidence
     * of transport failure.
     *
     * If messages are received thereafter on this session, the session SHALL be put back into the Active state.
     *
     * This SHALL only be callable on an active session.
     * This SHALL NOT detach any existing SessionHolders.
     *
     */
    void MarkAsDefunct();

    Session::SessionType GetSessionType() const override { return Session::SessionType::kSecure; }

    ScopedNodeId GetPeer() const override { return ScopedNodeId(mPeerNodeId, GetFabricIndex()); }

    ScopedNodeId GetLocalScopedNodeId() const override { return ScopedNodeId(mLocalNodeId, GetFabricIndex()); }

    Access::SubjectDescriptor GetSubjectDescriptor() const override;

    bool RequireMRP() const override { return GetPeerAddress().GetTransportType() == Transport::Type::kUdp; }

    System::Clock::Milliseconds32 GetAckTimeout() const override
    {
        switch (mPeerAddress.GetTransportType())
        {
        case Transport::Type::kUdp:
            return GetRetransmissionTimeout(mRemoteMRPConfig.mActiveRetransTimeout, mRemoteMRPConfig.mIdleRetransTimeout,
                                            GetLastPeerActivityTime(), kMinActiveTime);
        case Transport::Type::kTcp:
            return System::Clock::Seconds16(30);
        case Transport::Type::kBle:
            return System::Clock::Milliseconds32(BTP_ACK_TIMEOUT_MS);
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

    const CATValues & GetPeerCATs() const { return mPeerCATs; }

    void SetRemoteMRPConfig(const ReliableMessageProtocolConfig & config) { mRemoteMRPConfig = config; }

    const ReliableMessageProtocolConfig & GetRemoteMRPConfig() const override { return mRemoteMRPConfig; }

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

        if (mState == State::kDefunct)
        {
            MoveToState(State::kActive);
        }
    }

    bool IsPeerActive() const
    {
        return ((System::SystemClock().GetMonotonicTimestamp() - GetLastPeerActivityTime()) < kMinActiveTime);
    }

    System::Clock::Timestamp GetMRPBaseTimeout() const override
    {
        return IsPeerActive() ? GetRemoteMRPConfig().mActiveRetransTimeout : GetRemoteMRPConfig().mIdleRetransTimeout;
    }

    CryptoContext & GetCryptoContext() { return mCryptoContext; }

    const CryptoContext & GetCryptoContext() const { return mCryptoContext; }

    SessionMessageCounter & GetSessionMessageCounter() { return mSessionMessageCounter; }

    // This should be a private API, only meant to be called by SecureSessionTable
    // Session holders to this session may shift to the target session regarding SessionDelegate::GetNewSessionHandlingPolicy.
    // It requires that the target sessoin is also a CASE session, having the same peer and CATs as this session.
    void NewerSessionAvailable(const SessionHandle & session);

private:
    enum class State : uint8_t
    {
        //
        // Denotes a secure session object that is internally
        // reserved by the stack before and during session establishment.
        //
        // Although the stack can tolerate eviction of these (releasing one
        // out from under the holder would exhibit as CHIP_ERROR_INCORRECT_STATE
        // during CASE or PASE), intent is that we should not and would leave
        // these untouched until CASE or PASE complete.
        //
        // In this state, the reference count is held by the PairingSession.
        //
        kEstablishing = 1,

        //
        // The session is active, ready for use. When transitioning to this state via Activate, the
        // reference count is incremented by 1, and will subsequently be decremented
        // by 1 when MarkForEviction is called. This ensures the session remains resident
        // and active for future use even if there currently are no references to it.
        //
        kActive = 2,

        //
        // The session is temporarily disabled due to suspicion of a loss of synchronization
        // with the session state on the peer (e.g transport failure).
        // In this state, no new outbound exchanges can be created. However, if we receive valid messages
        // again on this session, we CAN mark this session as being active again.
        //
        // Transitioning to this state does not detach any existing SessionHolders.
        //
        // In addition to any existing SessionHolders holding a reference to this session, the SessionManager
        // maintains a reference as well to the session that will only be relinquished when MarkForEviction is called.
        //
        kDefunct = 3,

        //
        // The session has been marked for eviction and is pending deallocation. All SessionHolders would have already
        // been detached in a previous call to MarkForEviction. Future SessionHolders will not be able to attach to
        // this session.
        //
        // When all SessionHandles go out of scope, the session will be released automatically.
        //
        kPendingEviction = 4,
    };

    const char * StateToString(State state) const;
    void MoveToState(State targetState);

    friend class SecureSessionDeleter;
    friend class TestSecureSessionTable;

    SecureSessionTable & mTable;
    State mState;
    const Type mSecureSessionType;
    NodeId mLocalNodeId = kUndefinedNodeId;
    NodeId mPeerNodeId  = kUndefinedNodeId;
    CATValues mPeerCATs = CATValues{};
    const uint16_t mLocalSessionId;
    uint16_t mPeerSessionId = 0;

    PeerAddress mPeerAddress;

    /// Timestamp of last tx or rx. @see SessionTimestamp in the spec
    System::Clock::Timestamp mLastActivityTime = System::SystemClock().GetMonotonicTimestamp();

    /// Timestamp of last rx. @see ActiveTimestamp in the spec
    System::Clock::Timestamp mLastPeerActivityTime = System::SystemClock().GetMonotonicTimestamp();

    ReliableMessageProtocolConfig mRemoteMRPConfig = GetDefaultMRPConfig();
    CryptoContext mCryptoContext;
    SessionMessageCounter mSessionMessageCounter;
};

} // namespace Transport
} // namespace chip
