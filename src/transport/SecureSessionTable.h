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
                                                          const ReliableMessageProtocolConfig & config)
    {
        if (secureSessionType == SecureSession::Type::kCASE)
        {
            if ((fabricIndex == kUndefinedFabricIndex) || (localNodeId == kUndefinedNodeId) || (peerNodeId == kUndefinedNodeId))
            {
                return Optional<SessionHandle>::Missing();
            }
        }
        else if (secureSessionType == SecureSession::Type::kPASE)
        {
            if ((fabricIndex != kUndefinedFabricIndex) || (localNodeId != kUndefinedNodeId) || (peerNodeId != kUndefinedNodeId))
            {
                // TODO: This secure session type is infeasible! We must fix the tests
                if (false)
                {
                    return Optional<SessionHandle>::Missing();
                }
                else
                {
                    (void) fabricIndex;
                }
            }
        }

        SecureSession * result = mEntries.CreateObject(*this, secureSessionType, localSessionId, localNodeId, peerNodeId, peerCATs,
                                                       peerSessionId, fabricIndex, config);
        return result != nullptr ? MakeOptional<SessionHandle>(*result) : Optional<SessionHandle>::Missing();
    }

    /**
     * Allocate a new secure session out of the internal resource pool with a
     * non-colliding session ID and increments mNextSessionId to give a clue to
     * the allocator for the next allocation.  The secure session session will
     * not become active until the call to SecureSession::Activate.
     *
     * @returns allocated session, or NullOptional on failure
     */
    CHECK_RETURN_VALUE
    Optional<SessionHandle> CreateNewSecureSession(SecureSession::Type secureSessionType)
    {
        Optional<SessionHandle> rv = Optional<SessionHandle>::Missing();
        auto sessionId             = FindUnusedSessionId();
        SecureSession * allocated  = nullptr;
        VerifyOrExit(sessionId.HasValue(), rv = Optional<SessionHandle>::Missing());
        allocated = mEntries.CreateObject(*this, secureSessionType, sessionId.Value());
        VerifyOrExit(allocated != nullptr, rv = Optional<SessionHandle>::Missing());
        rv             = MakeOptional<SessionHandle>(*allocated);
        mNextSessionId = sessionId.Value() == kMaxSessionID ? static_cast<uint16_t>(kUnsecuredSessionId + 1)
                                                            : static_cast<uint16_t>(sessionId.Value() + 1);
    exit:
        return rv;
    }

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
    Optional<SessionHandle> FindSecureSessionByLocalKey(uint16_t localSessionId)
    {
        SecureSession * result = nullptr;
        mEntries.ForEachActiveObject([&](auto session) {
            if (session->GetLocalSessionId() == localSessionId)
            {
                result = session;
                return Loop::Break;
            }
            return Loop::Continue;
        });
        return result != nullptr ? MakeOptional<SessionHandle>(*result) : Optional<SessionHandle>::Missing();
    }

private:
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
    Optional<uint16_t> FindUnusedSessionId()
    {
        uint16_t candidate_base = 0;
        uint64_t candidate_mask = 0;
        for (uint32_t i = 0; i <= kMaxSessionID; i += 64)
        {
            // candidate_base is the base session ID we are searching from.
            // We have a 64-bit mask anchored at this ID and iterate over the
            // whole session table, setting bits in the mask for in-use IDs.
            // If we can iterate through the entire session table and have
            // any bits clear in the mask, we have available session IDs.
            candidate_base = static_cast<uint16_t>(i + mNextSessionId);
            candidate_mask = 0;
            {
                uint16_t shift = static_cast<uint16_t>(kUnsecuredSessionId - candidate_base);
                if (shift <= 63)
                {
                    candidate_mask |= (1ULL << shift); // kUnsecuredSessionId is never available
                }
            }
            mEntries.ForEachActiveObject([&](auto session) {
                uint16_t shift = static_cast<uint16_t>(session->GetLocalSessionId() - candidate_base);
                if (shift <= 63)
                {
                    candidate_mask |= (1ULL << shift);
                }
                if (candidate_mask == UINT64_MAX)
                {
                    return Loop::Break; // No bits clear means this bucket is full.
                }
                return Loop::Continue;
            });
            if (candidate_mask != UINT64_MAX)
            {
                break; // Any bit clear means we have an available ID in this bucket.
            }
        }
        if (candidate_mask != UINT64_MAX)
        {
            uint16_t offset = 0;
            while (candidate_mask & 1)
            {
                candidate_mask >>= 1;
                ++offset;
            }
            uint16_t available = static_cast<uint16_t>(candidate_base + offset);
            return MakeOptional<uint16_t>(available);
        }

        return NullOptional;
    }

    BitMapObjectPool<SecureSession, CHIP_CONFIG_PEER_CONNECTION_POOL_SIZE> mEntries;
    uint16_t mNextSessionId = 0;
};

} // namespace Transport
} // namespace chip
