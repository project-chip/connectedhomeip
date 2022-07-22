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
#pragma once

#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Pool.h>
#include <lib/support/SortUtils.h>
#include <system/TimeSource.h>
#include <transport/SecureSession.h>

namespace chip {
namespace Transport {

constexpr uint16_t kMaxSessionID       = UINT16_MAX;
constexpr uint16_t kUnsecuredSessionId = 0;

/**
 * Handles a set of sessions.
 *
 * Intended for:
 *   - handle session active time and expiration
 *   - allocate and free space for sessions.
 */
class SecureSessionTable
{
public:
    ~SecureSessionTable() { mEntries.ReleaseAll(); }

    void Init() { mNextSessionId = chip::Crypto::GetRandU16(); }

    /**
     * Allocate a new secure session out of the internal resource pool.
     *
     * @param secureSessionType secure session type
     * @param localSessionId unique identifier for the local node's secure unicast session context
     * @param localNodeId represents the local Node ID for this node
     * @param peerNodeId represents peer Node's ID
     * @param peerCATs represents peer CASE Authenticated Tags
     * @param peerSessionId represents the encryption key ID assigned by peer node
     * @param fabricIndex represents fabric index for the session
     * @param config represents the reliable message protocol configuration
     *
     * @note the newly created state will have an 'active' time set based on the current time source.
     *
     * @returns CHIP_NO_ERROR if state could be initialized. May fail if maximum session count
     *          has been reached (with CHIP_ERROR_NO_MEMORY).
     */
    CHECK_RETURN_VALUE
    Optional<SessionHandle> CreateNewSecureSessionForTest(SecureSession::Type secureSessionType, uint16_t localSessionId,
                                                          NodeId localNodeId, NodeId peerNodeId, CATValues peerCATs,
                                                          uint16_t peerSessionId, FabricIndex fabricIndex,
                                                          const ReliableMessageProtocolConfig & config);
    /**
     * Allocate a new secure session out of the internal resource pool with a
     * non-colliding session ID and increments mNextSessionId to give a clue to
     * the allocator for the next allocation.  The secure session session will
     * not become active until the call to SecureSession::Activate.
     *
     * @returns allocated session, or NullOptional on failure
     */
    CHECK_RETURN_VALUE
    Optional<SessionHandle> CreateNewSecureSession(SecureSession::Type secureSessionType, ScopedNodeId sessionEvictionHint);

    void ReleaseSession(SecureSession * session) { mEntries.ReleaseObject(session); }

    template <typename Function>
    Loop ForEachSession(Function && function)
    {
        return mEntries.ForEachActiveObject(std::forward<Function>(function));
    }

    /**
     * Get a secure session given its session ID.
     *
     * @param localSessionId the identifier of a secure unicast session context within the local node
     *
     * @return the session if found, NullOptional if not found
     */
    CHECK_RETURN_VALUE
    Optional<SessionHandle> FindSecureSessionByLocalKey(uint16_t localSessionId);

    // Select SessionHolders which are pointing to a session with the same peer as the given session. Shift them to the given
    // session.
    // This is an internal API, using raw pointer to a session is allowed here.
    void NewerSessionAvailable(SecureSession * session)
    {
        VerifyOrDie(session->GetSecureSessionType() == SecureSession::Type::kCASE);
        mEntries.ForEachActiveObject([&](SecureSession * oldSession) {
            if (session == oldSession)
                return Loop::Continue;

            SessionHandle ref(*oldSession);

            // This will give all SessionHolders pointing to oldSession a chance to switch to the provided session
            //
            // See documentation for SessionDelegate::GetNewSessionHandlingPolicy about how session auto-shifting works, and how
            // to disable it for a specific SessionHolder in a specific scenario.
            if (oldSession->GetSecureSessionType() == SecureSession::Type::kCASE && oldSession->GetPeer() == session->GetPeer() &&
                oldSession->GetPeerCATs() == session->GetPeerCATs())
            {
                oldSession->NewerSessionAvailable(SessionHandle(*session));
            }

            return Loop::Continue;
        });
    }

private:
    friend class TestSecureSessionTable;

    /**
     * This provides a sortable wrapper for a SecureSession object. A SecureSession
     * isn't directly sortable since it is not swappable (i.e meet criteria for ValueSwappable).
     *
     * However, this wrapper has a stable pointer to a SecureSession while being swappable with
     * another instance of it.
     *
     */
    struct SortableSession
    {
    public:
        void swap(SortableSession & other)
        {
            SortableSession tmp(other);
            other.mSession = mSession;
            mSession       = tmp.mSession;
        }

        const Transport::SecureSession * operator->() const { return mSession; }
        auto GetNumMatchingOnFabric() { return mNumMatchingOnFabric; }
        auto GetNumMatchingOnPeer() { return mNumMatchingOnPeer; }

    private:
        SecureSession * mSession;
        uint16_t mNumMatchingOnFabric;
        uint16_t mNumMatchingOnPeer;

        static_assert(CHIP_CONFIG_SECURE_SESSION_POOL_SIZE <= std::numeric_limits<decltype(mNumMatchingOnFabric)>::max(),
                      "mNumMatchingOnFabric must be able to count up to CHIP_CONFIG_SECURE_SESSION_POOL_SIZE!");
        static_assert(CHIP_CONFIG_SECURE_SESSION_POOL_SIZE <= std::numeric_limits<decltype(mNumMatchingOnPeer)>::max(),
                      "mNumMatchingOnPeer must be able to count up to CHIP_CONFIG_SECURE_SESSION_POOL_SIZE!");

        friend class SecureSessionTable;
    };

    /**
     *
     * Encapsulates all the necessary context for an eviction policy callback
     * to implement its specific policy. The context is provided to the callee
     * with the expectation that it'll call Sort() with a comparator function provided
     * to get the list of sessions sorted in the desired order.
     *
     */
    class EvictionPolicyContext
    {
    public:
        /*
         * Called by the policy implementor to sort the list of sessions given a comparator
         * function. The provided function shall have the following signature:
         *
         * bool CompareFunc(const SortableSession &a, const SortableSession &b);
         *
         * If a is a better candidate than b, true should be returned. Else, return false.
         *
         * NOTE: Sort() can be called multiple times.
         *
         */
        template <typename CompareFunc>
        void Sort(CompareFunc func)
        {
            Sorting::BubbleSort(mSessionList.begin(), mSessionList.size(), func);
        }

        const ScopedNodeId & GetSessionEvictionHint() const { return mSessionEvictionHint; }

    private:
        EvictionPolicyContext(Span<SortableSession> sessionList, ScopedNodeId sessionEvictionHint)
        {
            mSessionList         = sessionList;
            mSessionEvictionHint = sessionEvictionHint;
        }

        friend class SecureSessionTable;
        Span<SortableSession> mSessionList;
        ScopedNodeId mSessionEvictionHint;
    };

    /**
     *
     * This implements an eviction policy by sorting sessions using the following sorting keys and selecting
     * the session that is most ahead as the best candidate for eviction:
     *
     *  - Key1:  Sessions on fabrics that have more sessions in the table are placed ahead of sessions on fabrics
     *           with lesser sessions. We conclusively know that if a particular fabric has more sessions in the table
     *           than another, then that fabric is definitely over minimas (assuming a minimally sized session table
     *           conformant to spec minimas).
     *
     *    Key2:  Sessions that match the eviction hint's fabric are placed ahead of those that don't. This ensures that
     *           if Key1 is even (i.e two fabrics are tied in count), that you attempt to select sessions that match
     *           the eviction hint's fabric to ensure we evict sessions within the fabric that a new session might be about
     *           to be created within. This is essential to preventing cross-fabric denial of service possibilities.
     *
     *    Key3:  Sessions with a higher mNumMatchingOnPeer are placed ahead of those with a lower one. This ensures
     *           we pick sessions that have a higher number of duplicated sessions to a peer over those with lower since
     *           evicting a duplicated session will have less of an impact to that peer.
     *
     *    Key4:  Sessions whose target peer's ScopedNodeId matches the eviction hint are placed ahead of those who don't. This
     * ensures that all things equal, a session that already exists to the peer is refreshed ahead of another to another peer.
     *
     *    Key5:  Sessions that are in defunct state are placed ahead of those in the active state, ahead of any other state.
     *           This ensures that we prioritize evicting defunct sessions (since they have been deemed non-functional anyways)
     *           over active, healthy ones, over those are currently in the process of establishment.
     *
     *    Key6:  Sessions that have a less recent activity time are placed ahead of those with a more recent activity time. This
     *           is the canonical sorting criteria for basic LRU.
     *
     */
    void DefaultEvictionPolicy(EvictionPolicyContext & evictionContext);

    /**
     *
     * Evicts a session from the session table using the DefaultEvictionPolicy implementation.
     *
     */
    SecureSession * EvictAndAllocate(uint16_t localSessionId, SecureSession::Type secureSessionType,
                                     const ScopedNodeId & sessionEvictionHint);

    /**
     * Find an available session ID that is unused in the secure session table.
     *
     * The search algorithm iterates over the session ID space in the outer loop
     * and the session table in the inner loop to locate an available session ID
     * from the starting mNextSessionId clue.
     *
     * The outer-loop considers 64 session IDs in each iteration to give a
     * runtime complexity of O(CHIP_CONFIG_PEER_CONNECTION_POOL_SIZE^2/64).  Speed up could be
     * achieved with a sorted session table or additional storage.
     *
     * @return an unused session ID if any is found, else NullOptional
     */
    CHECK_RETURN_VALUE
    Optional<uint16_t> FindUnusedSessionId();

    bool mRunningEvictionLogic = false;
    ObjectPool<SecureSession, CHIP_CONFIG_SECURE_SESSION_POOL_SIZE> mEntries;

    size_t GetMaxSessionTableSize() const
    {
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
        return mMaxSessionTableSize;
#else
        return CHIP_CONFIG_SECURE_SESSION_POOL_SIZE;
#endif
    }

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    size_t mMaxSessionTableSize = CHIP_CONFIG_SECURE_SESSION_POOL_SIZE;
    void SetMaxSessionTableSize(size_t size) { mMaxSessionTableSize = size; }
#endif

    uint16_t mNextSessionId = 0;
};

} // namespace Transport
} // namespace chip
