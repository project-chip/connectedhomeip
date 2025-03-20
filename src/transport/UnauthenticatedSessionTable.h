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

#include <ble/Ble.h>
#include <lib/core/CHIPError.h>
#include <lib/core/ReferenceCounted.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Pool.h>
#include <messaging/ReliableMessageProtocolConfig.h>
#include <system/SystemConfig.h>
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
class UnauthenticatedSession : public Session, public ReferenceCounted<UnauthenticatedSession, UnauthenticatedSession, 0>
{
public:
    enum class SessionRole
    {
        kInitiator,
        kResponder,
    };

protected:
    UnauthenticatedSession(SessionRole sessionRole, NodeId ephemeralInitiatorNodeID, const Transport::PeerAddress & peerAddress,
                           const ReliableMessageProtocolConfig & config) :
        mEphemeralInitiatorNodeId(ephemeralInitiatorNodeID),
        mSessionRole(sessionRole), mPeerAddress(peerAddress), mLastActivityTime(System::SystemClock().GetMonotonicTimestamp()),
        mLastPeerActivityTime(System::Clock::kZero), // Start at zero to default to IDLE state
        mRemoteSessionParams(config)
    {}
    ~UnauthenticatedSession() override { VerifyOrDie(GetReferenceCount() == 0); }

public:
    UnauthenticatedSession(const UnauthenticatedSession &)             = delete;
    UnauthenticatedSession & operator=(const UnauthenticatedSession &) = delete;
    UnauthenticatedSession(UnauthenticatedSession &&)                  = delete;
    UnauthenticatedSession & operator=(UnauthenticatedSession &&)      = delete;

    System::Clock::Timestamp GetLastActivityTime() const { return mLastActivityTime; }
    System::Clock::Timestamp GetLastPeerActivityTime() const { return mLastPeerActivityTime; }
    void MarkActive() { mLastActivityTime = System::SystemClock().GetMonotonicTimestamp(); }
    void MarkActiveRx()
    {
        mLastPeerActivityTime = System::SystemClock().GetMonotonicTimestamp();
        MarkActive();
    }

    Session::SessionType GetSessionType() const override { return Session::SessionType::kUnauthenticated; }

    void Retain() override { ReferenceCounted<UnauthenticatedSession, UnauthenticatedSession, 0>::Retain(); }
    void Release() override { ReferenceCounted<UnauthenticatedSession, UnauthenticatedSession, 0>::Release(); }

    bool IsActiveSession() const override { return true; }

    ScopedNodeId GetPeer() const override { return ScopedNodeId(GetPeerNodeId(), kUndefinedFabricIndex); }
    ScopedNodeId GetLocalScopedNodeId() const override { return ScopedNodeId(kUndefinedNodeId, kUndefinedFabricIndex); }

    Access::SubjectDescriptor GetSubjectDescriptor() const override
    {
        return Access::SubjectDescriptor(); // return an empty ISD for unauthenticated session.
    }

    bool AllowsMRP() const override { return GetPeerAddress().GetTransportType() == Transport::Type::kUdp; }

    bool AllowsLargePayload() const override { return GetPeerAddress().GetTransportType() == Transport::Type::kTcp; }

    System::Clock::Milliseconds32 GetAckTimeout(bool isFirstMessageOnExchange) const override
    {
        switch (mPeerAddress.GetTransportType())
        {
        case Transport::Type::kUdp: {
            const ReliableMessageProtocolConfig & remoteMRPConfig = mRemoteSessionParams.GetMRPConfig();
            return GetRetransmissionTimeout(remoteMRPConfig.mActiveRetransTimeout, remoteMRPConfig.mIdleRetransTimeout,
                                            GetLastPeerActivityTime(), remoteMRPConfig.mActiveThresholdTime,
                                            isFirstMessageOnExchange);
        }
        case Transport::Type::kTcp:
            return System::Clock::Seconds16(30);
        case Transport::Type::kBle:
            return System::Clock::Milliseconds32(BTP_ACK_TIMEOUT_MS);
        default:
            break;
        }
        return System::Clock::Timeout();
    }

    System::Clock::Milliseconds32 GetMessageReceiptTimeout(System::Clock::Timestamp ourLastActivity,
                                                           bool isFirstMessageOnExchange) const override
    {
        switch (mPeerAddress.GetTransportType())
        {
        case Transport::Type::kUdp: {
            const auto & maybeLocalMRPConfig = GetLocalMRPConfig();
            const auto & defaultMRRPConfig   = GetDefaultMRPConfig();
            const auto & localMRPConfig      = maybeLocalMRPConfig.ValueOr(defaultMRRPConfig);
            return GetRetransmissionTimeout(localMRPConfig.mActiveRetransTimeout, localMRPConfig.mIdleRetransTimeout,
                                            ourLastActivity, localMRPConfig.mActiveThresholdTime, isFirstMessageOnExchange);
        }
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
        return ((System::SystemClock().GetMonotonicTimestamp() - GetLastPeerActivityTime()) <
                GetRemoteMRPConfig().mActiveThresholdTime);
    }

    System::Clock::Timestamp GetMRPBaseTimeout() const override
    {
        return IsPeerActive() ? GetRemoteMRPConfig().mActiveRetransTimeout : GetRemoteMRPConfig().mIdleRetransTimeout;
    }

    void SetRemoteSessionParameters(const SessionParameters & sessionParams) { mRemoteSessionParams = sessionParams; }

    const SessionParameters & GetRemoteSessionParameters() const override { return mRemoteSessionParams; }

    PeerMessageCounter & GetPeerMessageCounter() { return mPeerMessageCounter; }

    static void Release(UnauthenticatedSession * obj)
    {
        // When using heap pools, we need to make sure to release ourselves back to
        // the pool.  When not using heap pools, we don't want the extra cost of the
        // table pointer here, and the table itself handles entry reuse and cleanup
        // as needed.
#if CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
        obj->ReleaseSelfToPool();
#else
        // Just do nothing.
#endif // CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
    }

#if CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
    virtual void ReleaseSelfToPool() = 0;
#endif // CHIP_SYSTEM_CONFIG_POOL_USE_HEAP

private:
    const NodeId mEphemeralInitiatorNodeId;
    const SessionRole mSessionRole;
    PeerAddress mPeerAddress;
    System::Clock::Timestamp mLastActivityTime;     ///< Timestamp of last tx or rx
    System::Clock::Timestamp mLastPeerActivityTime; ///< Timestamp of last rx
    SessionParameters mRemoteSessionParams;
    PeerMessageCounter mPeerMessageCounter;
};

template <size_t kMaxSessionCount>
class UnauthenticatedSessionTable;

namespace detail {

template <size_t kMaxSessionCount>
class UnauthenticatedSessionPoolEntry : public UnauthenticatedSession
{
public:
    UnauthenticatedSessionPoolEntry(SessionRole sessionRole, NodeId ephemeralInitiatorNodeID,
                                    const Transport::PeerAddress & peerAddress, const ReliableMessageProtocolConfig & config,
                                    UnauthenticatedSessionTable<kMaxSessionCount> & sessionTable) :
        UnauthenticatedSession(sessionRole, ephemeralInitiatorNodeID, peerAddress, config)
#if CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
        ,
        mSessionTable(sessionTable)
#endif // CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
    {}

private:
#if CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
    virtual void ReleaseSelfToPool();

    UnauthenticatedSessionTable<kMaxSessionCount> & mSessionTable;
#endif // CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
};

} // namespace detail

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
    ~UnauthenticatedSessionTable()
    {
#if !CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
        // When not using heap pools, our entries never actually get released
        // back to the pool (which lets us make the entries 4 bytes smaller by
        // not storing a reference to the table in them) and we LRU reuse ones
        // that have 0 refcount.  But we should release them all here, to ensure
        // that we don't hit fatal asserts in our pool destructor.
        mEntries.ReleaseAll();
#endif // CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
    }

    /**
     * Get a responder session with the given ephemeralInitiatorNodeID. If the session doesn't exist in the cache, allocate a new
     * entry for it.
     *
     * @return the session found or allocated, or Optional::Missing if not found and allocation failed.
     */
    CHECK_RETURN_VALUE
    Optional<SessionHandle> FindOrAllocateResponder(NodeId ephemeralInitiatorNodeID, const ReliableMessageProtocolConfig & config,
                                                    const Transport::PeerAddress & peerAddress)
    {
        UnauthenticatedSession * result =
            FindEntry(UnauthenticatedSession::SessionRole::kResponder, ephemeralInitiatorNodeID, peerAddress);
        if (result != nullptr)
            return MakeOptional<SessionHandle>(*result);

        CHIP_ERROR err =
            AllocEntry(UnauthenticatedSession::SessionRole::kResponder, ephemeralInitiatorNodeID, peerAddress, config, result);
        if (err == CHIP_NO_ERROR)
        {
            return MakeOptional<SessionHandle>(*result);
        }

        return Optional<SessionHandle>::Missing();
    }

    CHECK_RETURN_VALUE Optional<SessionHandle> FindInitiator(NodeId ephemeralInitiatorNodeID,
                                                             const Transport::PeerAddress & peerAddress)
    {
        UnauthenticatedSession * result =
            FindEntry(UnauthenticatedSession::SessionRole::kInitiator, ephemeralInitiatorNodeID, peerAddress);
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
        CHIP_ERROR err =
            AllocEntry(UnauthenticatedSession::SessionRole::kInitiator, ephemeralInitiatorNodeID, peerAddress, config, result);
        if (err == CHIP_NO_ERROR)
        {
            result->SetPeerAddress(peerAddress);
            return MakeOptional<SessionHandle>(*result);
        }

        return Optional<SessionHandle>::Missing();
    }

private:
    using EntryType = detail::UnauthenticatedSessionPoolEntry<kMaxSessionCount>;
    friend EntryType;

    /**
     * Allocates a new session out of the internal resource pool.
     *
     * @returns CHIP_NO_ERROR if new session created. May fail if maximum session count has been reached (with
     * CHIP_ERROR_NO_MEMORY).
     */
    CHECK_RETURN_VALUE
    CHIP_ERROR AllocEntry(UnauthenticatedSession::SessionRole sessionRole, NodeId ephemeralInitiatorNodeID,
                          const PeerAddress & peerAddress, const ReliableMessageProtocolConfig & config,
                          UnauthenticatedSession *& entry)
    {
        auto entryToUse = mEntries.CreateObject(sessionRole, ephemeralInitiatorNodeID, peerAddress, config, *this);
        if (entryToUse != nullptr)
        {
            entry = entryToUse;
            return CHIP_NO_ERROR;
        }

#if CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
        // permanent failure if heap was insufficient
        return CHIP_ERROR_NO_MEMORY;
#else
        entryToUse = FindLeastRecentUsedEntry();
        VerifyOrReturnError(entryToUse != nullptr, CHIP_ERROR_NO_MEMORY);

        // Drop the least recent entry to allow for a new alloc.
        mEntries.ReleaseObject(entryToUse);
        entryToUse = mEntries.CreateObject(sessionRole, ephemeralInitiatorNodeID, peerAddress, config, *this);

        if (entryToUse == nullptr)
        {
            // this is NOT expected: we freed an object to have space
            return CHIP_ERROR_INTERNAL;
        }

        entry = entryToUse;
        return CHIP_NO_ERROR;
#endif // CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
    }

    CHECK_RETURN_VALUE UnauthenticatedSession * FindEntry(UnauthenticatedSession::SessionRole sessionRole,
                                                          NodeId ephemeralInitiatorNodeID,
                                                          const Transport::PeerAddress & peerAddress)
    {
        UnauthenticatedSession * result = nullptr;
        mEntries.ForEachActiveObject([&](UnauthenticatedSession * entry) {
            if (entry->GetSessionRole() == sessionRole && entry->GetEphemeralInitiatorNodeID() == ephemeralInitiatorNodeID &&
                entry->GetPeerAddress().GetTransportType() == peerAddress.GetTransportType())
            {
                result = entry;
                return Loop::Break;
            }
            return Loop::Continue;
        });
        return result;
    }

    EntryType * FindLeastRecentUsedEntry()
    {
        EntryType * result                  = nullptr;
        System::Clock::Timestamp oldestTime = System::Clock::Timestamp(std::numeric_limits<System::Clock::Timestamp::rep>::max());

        mEntries.ForEachActiveObject([&](EntryType * entry) {
            if (entry->GetReferenceCount() == 0 && entry->GetLastActivityTime() < oldestTime)
            {
                result     = entry;
                oldestTime = entry->GetLastActivityTime();
            }
            return Loop::Continue;
        });

        return result;
    }

    void ReleaseEntry(EntryType * entry) { mEntries.ReleaseObject(entry); }

    ObjectPool<EntryType, kMaxSessionCount> mEntries;
};

#if CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
template <size_t kMaxSessionCount>
void detail::UnauthenticatedSessionPoolEntry<kMaxSessionCount>::ReleaseSelfToPool()
{
    mSessionTable.ReleaseEntry(this);
}
#endif // CHIP_SYSTEM_CONFIG_POOL_USE_HEAP

} // namespace Transport
} // namespace chip
