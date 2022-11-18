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

#include <ble/BleConfig.h>
#include <lib/core/CHIPError.h>
#include <lib/core/ReferenceCounted.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Pool.h>
#include <messaging/ReliableMessageProtocolConfig.h>
#include <system/TimeSource.h>
#include <transport/PeerMessageCounter.h>
#include <transport/Session.h>
#include <transport/raw/PeerAddress.h>

namespace chip {
namespace Transport {

/**
 * @brief
 *   An UnauthenticatedSession stores the binding of TransportAddress, and message counters.
 */
class UnauthenticatedSession : public Session,
                               public ReferenceCounted<UnauthenticatedSession, NoopDeletor<UnauthenticatedSession>, 0>
{
public:
    enum class SessionRole
    {
        kInitiator,
        kResponder,
    };

    UnauthenticatedSession(SessionRole sessionRole, NodeId ephemeralInitiatorNodeID, const ReliableMessageProtocolConfig & config) :
        mEphemeralInitiatorNodeId(ephemeralInitiatorNodeID), mSessionRole(sessionRole),
        mLastActivityTime(System::SystemClock().GetMonotonicTimestamp()),
        mLastPeerActivityTime(System::Clock::kZero), // Start at zero to default to IDLE state
        mRemoteMRPConfig(config)
    {}
    ~UnauthenticatedSession() override { VerifyOrDie(GetReferenceCount() == 0); }

    UnauthenticatedSession(const UnauthenticatedSession &) = delete;
    UnauthenticatedSession & operator=(const UnauthenticatedSession &) = delete;
    UnauthenticatedSession(UnauthenticatedSession &&)                  = delete;
    UnauthenticatedSession & operator=(UnauthenticatedSession &&) = delete;

    System::Clock::Timestamp GetLastActivityTime() const { return mLastActivityTime; }
    System::Clock::Timestamp GetLastPeerActivityTime() const { return mLastPeerActivityTime; }
    void MarkActive() { mLastActivityTime = System::SystemClock().GetMonotonicTimestamp(); }
    void MarkActiveRx()
    {
        mLastPeerActivityTime = System::SystemClock().GetMonotonicTimestamp();
        MarkActive();
    }

    Session::SessionType GetSessionType() const override { return Session::SessionType::kUnauthenticated; }

    void Retain() override { ReferenceCounted<UnauthenticatedSession, NoopDeletor<UnauthenticatedSession>, 0>::Retain(); }
    void Release() override { ReferenceCounted<UnauthenticatedSession, NoopDeletor<UnauthenticatedSession>, 0>::Release(); }

    bool IsActiveSession() const override { return true; }

    ScopedNodeId GetPeer() const override { return ScopedNodeId(GetPeerNodeId(), kUndefinedFabricIndex); }
    ScopedNodeId GetLocalScopedNodeId() const override { return ScopedNodeId(kUndefinedNodeId, kUndefinedFabricIndex); }

    Access::SubjectDescriptor GetSubjectDescriptor() const override
    {
        return Access::SubjectDescriptor(); // return an empty ISD for unauthenticated session.
    }

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

    NodeId GetPeerNodeId() const
    {
        if (mSessionRole == SessionRole::kInitiator)
        {
            return kUndefinedNodeId;
        }

        return mEphemeralInitiatorNodeId;
    }

    SessionRole GetSessionRole() const { return mSessionRole; }
    NodeId GetEphemeralInitiatorNodeID() const { return mEphemeralInitiatorNodeId; }
    const PeerAddress & GetPeerAddress() const { return mPeerAddress; }
    void SetPeerAddress(const PeerAddress & peerAddress) { mPeerAddress = peerAddress; }

    bool IsPeerActive() const
    {
        return ((System::SystemClock().GetMonotonicTimestamp() - GetLastPeerActivityTime()) < kMinActiveTime);
    }

    System::Clock::Timestamp GetMRPBaseTimeout() const override
    {
        return IsPeerActive() ? GetRemoteMRPConfig().mActiveRetransTimeout : GetRemoteMRPConfig().mIdleRetransTimeout;
    }

    void SetRemoteMRPConfig(const ReliableMessageProtocolConfig & config) { mRemoteMRPConfig = config; }

    const ReliableMessageProtocolConfig & GetRemoteMRPConfig() const override { return mRemoteMRPConfig; }

    PeerMessageCounter & GetPeerMessageCounter() { return mPeerMessageCounter; }

private:
    const NodeId mEphemeralInitiatorNodeId;
    const SessionRole mSessionRole;
    PeerAddress mPeerAddress;
    System::Clock::Timestamp mLastActivityTime;     ///< Timestamp of last tx or rx
    System::Clock::Timestamp mLastPeerActivityTime; ///< Timestamp of last rx
    ReliableMessageProtocolConfig mRemoteMRPConfig;
    PeerMessageCounter mPeerMessageCounter;
};

/*
 * @brief
 *   An table which manages UnauthenticatedSessions
 *
 *   The UnauthenticatedSession entries are rotated using LRU, but entry can be hold by using SessionHandle or
 *   SessionHolder, which increase the reference count by 1. If the reference count is not 0, the entry won't be pruned.
 */
template <size_t kMaxSessionCount>
class UnauthenticatedSessionTable
{
public:
    ~UnauthenticatedSessionTable() { mEntries.ReleaseAll(); }

    /**
     * Get a responder session with the given ephemeralInitiatorNodeID. If the session doesn't exist in the cache, allocate a new
     * entry for it.
     *
     * @return the session found or allocated, or Optional::Missing if not found and allocation failed.
     */
    CHECK_RETURN_VALUE
    Optional<SessionHandle> FindOrAllocateResponder(NodeId ephemeralInitiatorNodeID, const ReliableMessageProtocolConfig & config)
    {
        UnauthenticatedSession * result = FindEntry(UnauthenticatedSession::SessionRole::kResponder, ephemeralInitiatorNodeID);
        if (result != nullptr)
            return MakeOptional<SessionHandle>(*result);

        CHIP_ERROR err = AllocEntry(UnauthenticatedSession::SessionRole::kResponder, ephemeralInitiatorNodeID, config, result);
        if (err == CHIP_NO_ERROR)
        {
            return MakeOptional<SessionHandle>(*result);
        }

        return Optional<SessionHandle>::Missing();
    }

    CHECK_RETURN_VALUE Optional<SessionHandle> FindInitiator(NodeId ephemeralInitiatorNodeID)
    {
        UnauthenticatedSession * result = FindEntry(UnauthenticatedSession::SessionRole::kInitiator, ephemeralInitiatorNodeID);
        if (result != nullptr)
        {
            return MakeOptional<SessionHandle>(*result);
        }

        return Optional<SessionHandle>::Missing();
    }

    CHECK_RETURN_VALUE Optional<SessionHandle> AllocInitiator(NodeId ephemeralInitiatorNodeID, const PeerAddress & peerAddress,
                                                              const ReliableMessageProtocolConfig & config)
    {
        UnauthenticatedSession * result = nullptr;
        CHIP_ERROR err = AllocEntry(UnauthenticatedSession::SessionRole::kInitiator, ephemeralInitiatorNodeID, config, result);
        if (err == CHIP_NO_ERROR)
        {
            result->SetPeerAddress(peerAddress);
            return MakeOptional<SessionHandle>(*result);
        }

        return Optional<SessionHandle>::Missing();
    }

private:
    /**
     * Allocates a new session out of the internal resource pool.
     *
     * @returns CHIP_NO_ERROR if new session created. May fail if maximum session count has been reached (with
     * CHIP_ERROR_NO_MEMORY).
     */
    CHECK_RETURN_VALUE
    CHIP_ERROR AllocEntry(UnauthenticatedSession::SessionRole sessionRole, NodeId ephemeralInitiatorNodeID,
                          const ReliableMessageProtocolConfig & config, UnauthenticatedSession *& entry)
    {
        entry = mEntries.CreateObject(sessionRole, ephemeralInitiatorNodeID, config);
        if (entry != nullptr)
            return CHIP_NO_ERROR;

        entry = FindLeastRecentUsedEntry();
        if (entry == nullptr)
        {
            return CHIP_ERROR_NO_MEMORY;
        }

        mEntries.ResetObject(entry, sessionRole, ephemeralInitiatorNodeID, config);
        return CHIP_NO_ERROR;
    }

    CHECK_RETURN_VALUE UnauthenticatedSession * FindEntry(UnauthenticatedSession::SessionRole sessionRole,
                                                          NodeId ephemeralInitiatorNodeID)
    {
        UnauthenticatedSession * result = nullptr;
        mEntries.ForEachActiveObject([&](UnauthenticatedSession * entry) {
            if (entry->GetSessionRole() == sessionRole && entry->GetEphemeralInitiatorNodeID() == ephemeralInitiatorNodeID)
            {
                result = entry;
                return Loop::Break;
            }
            return Loop::Continue;
        });
        return result;
    }

    UnauthenticatedSession * FindLeastRecentUsedEntry()
    {
        UnauthenticatedSession * result     = nullptr;
        System::Clock::Timestamp oldestTime = System::Clock::Timestamp(std::numeric_limits<System::Clock::Timestamp::rep>::max());

        mEntries.ForEachActiveObject([&](UnauthenticatedSession * entry) {
            if (entry->GetReferenceCount() == 0 && entry->GetLastActivityTime() < oldestTime)
            {
                result     = entry;
                oldestTime = entry->GetLastActivityTime();
            }
            return Loop::Continue;
        });

        return result;
    }

    ObjectPool<UnauthenticatedSession, kMaxSessionCount> mEntries;
};

} // namespace Transport
} // namespace chip
