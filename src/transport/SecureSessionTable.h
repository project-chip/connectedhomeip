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

#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <system/TimeSource.h>
#include <transport/SecureSession.h>

namespace chip {
namespace Transport {

// TODO; use 0xffff to match any key id, this is a temporary solution for
// InteractionModel, where key id is not obtainable. This will be removed when
// InteractionModel is migrated to messaging layer
constexpr const uint16_t kAnyKeyId = 0xffff;

/**
 * Handles a set of sessions.
 *
 * Intended for:
 *   - handle session active time and expiration
 *   - allocate and free space for sessions.
 */
template <size_t kMaxSessionCount, Time::Source kTimeSource = Time::Source::kSystem>
class SecureSessionTable
{
public:
    /**
     * Allocates a new secure session out of the internal resource pool.
     *
     * @param peerNode represents peer Node's ID
     * @param peerSessionId represents the encryption key ID assigned by peer node
     * @param localSessionId represents the encryption key ID assigned by local node
     * @param state [out] will contain the session if one was available. May be null if no return value is desired.
     *
     * @note the newly created state will have an 'active' time set based on the current time source.
     *
     * @returns CHIP_NO_ERROR if state could be initialized. May fail if maximum session count
     *          has been reached (with CHIP_ERROR_NO_MEMORY).
     */
    CHECK_RETURN_VALUE
    CHIP_ERROR CreateNewSecureSession(NodeId peerNode, uint16_t peerSessionId, uint16_t localSessionId, SecureSession ** state)
    {
        CHIP_ERROR err = CHIP_ERROR_NO_MEMORY;

        if (state)
        {
            *state = nullptr;
        }

        for (size_t i = 0; i < kMaxSessionCount; i++)
        {
            if (!mStates[i].IsInitialized())
            {
                mStates[i] = SecureSession();
                mStates[i].SetPeerNodeId(peerNode);
                mStates[i].SetPeerSessionId(peerSessionId);
                mStates[i].SetLocalSessionId(localSessionId);
                mStates[i].SetLastActivityTimeMs(mTimeSource.GetCurrentMonotonicTimeMs());

                if (state)
                {
                    *state = &mStates[i];
                }

                err = CHIP_NO_ERROR;
                break;
            }
        }

        return err;
    }

    /**
     * Get a secure session given a Node Id.
     *
     * @param nodeId is the session to find (based on nodeId).
     * @param begin If a member of the pool, will start search from the next item. Can be nullptr to search from start.
     *
     * @return the state found, nullptr if not found
     */
    CHECK_RETURN_VALUE
    SecureSession * FindSecureSession(NodeId nodeId, SecureSession * begin)
    {
        SecureSession * state = nullptr;
        SecureSession * iter  = &mStates[0];

        if (begin >= iter && begin < &mStates[kMaxSessionCount])
        {
            iter = begin + 1;
        }

        for (; iter < &mStates[kMaxSessionCount]; iter++)
        {
            if (!iter->IsInitialized())
            {
                continue;
            }
            if (iter->GetPeerNodeId() == nodeId)
            {
                state = iter;
                break;
            }
        }
        return state;
    }

    /**
     * Get a secure session given a Node Id and Peer's Encryption Key Id.
     *
     * @param nodeId is the session to find (based on peer nodeId).
     * @param localSessionId Encryption key ID used by the local node.
     * @param begin If a member of the pool, will start search from the next item. Can be nullptr to search from start.
     *
     * @return the state found, nullptr if not found
     */
    CHECK_RETURN_VALUE
    SecureSession * FindSecureSessionByLocalKey(uint16_t localSessionId, SecureSession * begin)
    {
        SecureSession * state = nullptr;
        SecureSession * iter  = &mStates[0];

        if (begin >= iter && begin < &mStates[kMaxSessionCount])
        {
            iter = begin + 1;
        }

        for (; iter < &mStates[kMaxSessionCount]; iter++)
        {
            if (!iter->IsInitialized())
            {
                continue;
            }
            if (iter->GetLocalSessionId() == localSessionId)
            {
                state = iter;
                break;
            }
        }
        return state;
    }

    /**
     * Get the first session that matches the given fabric index.
     *
     * @param fabric The fabric index to match
     *
     * @return the session found, nullptr if not found
     */
    CHECK_RETURN_VALUE
    SecureSession * FindSecureSessionByFabric(FabricIndex fabric)
    {
        for (auto & state : mStates)
        {
            if (!state.IsInitialized())
            {
                continue;
            }
            if (state.GetFabricIndex() == fabric)
            {
                return &state;
            }
        }
        return nullptr;
    }

    /// Convenience method to mark a session as active
    void MarkSessionActive(SecureSession * state) { state->SetLastActivityTimeMs(mTimeSource.GetCurrentMonotonicTimeMs()); }

    /// Convenience method to expired a session and fired the related callback
    template <typename Callback>
    void MarkSessionExpired(SecureSession * state, Callback callback)
    {
        callback(*state);
        *state = SecureSession(PeerAddress::Uninitialized());
    }

    /**
     * Iterates through all active sessions and expires any sessions with an idle time
     * larger than the given amount.
     *
     * Expiring a session involves callback execution and then clearing the internal state.
     */
    template <typename Callback>
    void ExpireInactiveSessions(uint64_t maxIdleTimeMs, Callback callback)
    {
        const uint64_t currentTime = mTimeSource.GetCurrentMonotonicTimeMs();

        for (size_t i = 0; i < kMaxSessionCount; i++)
        {
            if (!mStates[i].IsInitialized())
            {
                continue; // not an active session
            }

            uint64_t sessionActiveTime = mStates[i].GetLastActivityTimeMs();
            if (sessionActiveTime + maxIdleTimeMs >= currentTime)
            {
                continue; // not expired
            }

            MarkSessionExpired(&mStates[i], callback);
        }
    }

    /// Allows access to the underlying time source used for keeping track of session active time
    Time::TimeSource<kTimeSource> & GetTimeSource() { return mTimeSource; }

private:
    Time::TimeSource<kTimeSource> mTimeSource;
    SecureSession mStates[kMaxSessionCount];
};

} // namespace Transport
} // namespace chip
