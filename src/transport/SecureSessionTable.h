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
#include <lib/support/Pool.h>
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
     * @param localSessionId represents the encryption key ID assigned by local node
     * @param peerNodeId represents peer Node's ID
     * @param peerSessionId represents the encryption key ID assigned by peer node
     * @param fabric represents fabric ID for the session
     * @param state [out] will contain the session if one was available. May be null if no return value is desired.
     *
     * @note the newly created state will have an 'active' time set based on the current time source.
     *
     * @returns CHIP_NO_ERROR if state could be initialized. May fail if maximum session count
     *          has been reached (with CHIP_ERROR_NO_MEMORY).
     */
    CHECK_RETURN_VALUE
    SecureSession * CreateNewSecureSession(uint16_t localSessionId, NodeId peerNodeId, uint16_t peerSessionId, FabricIndex fabric)
    {
        return mEntries.CreateObject(localSessionId, peerNodeId, peerSessionId, fabric, mTimeSource.GetCurrentMonotonicTimeMs());
    }

    void ReleaseSession(SecureSession * session)
    {
        mEntries.ReleaseObject(session);
    }

    template <typename Function>
    bool ForEachSession(Function && function)
    {
        return mEntries.ForEachActiveObject(std::forward<Function>(function));
    }

    /**
     * Get a secure session given a Node Id and Peer's Encryption Key Id.
     *
     * @param localSessionId Encryption key ID used by the local node.
     * @param begin If a member of the pool, will start search from the next item. Can be nullptr to search from start.
     *
     * @return the state found, nullptr if not found
     */
    CHECK_RETURN_VALUE
    SecureSession * FindSecureSessionByLocalKey(uint16_t localSessionId)
    {
        SecureSession * result = nullptr;
        mEntries.ForEachActiveObject([&] (auto session) {
            if (session->GetLocalSessionId() == localSessionId)
            {
                result = session;
                return false;
            }
            return true;
        });
        return result;
    }

    /// Convenience method to mark a session as active
    void MarkSessionActive(SecureSession * state) { state->SetLastActivityTimeMs(mTimeSource.GetCurrentMonotonicTimeMs()); }

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
        mEntries.ForEachActiveObject([&] (auto session) {
            if (session->GetLastActivityTimeMs() + maxIdleTimeMs < currentTime)
            {
                callback(*session);
                ReleaseSession(session);
            }
            return true;
        });
    }

    /// Allows access to the underlying time source used for keeping track of session active time
    Time::TimeSource<kTimeSource> & GetTimeSource() { return mTimeSource; }

private:
    Time::TimeSource<kTimeSource> mTimeSource;
    BitMapObjectPool<SecureSession, kMaxSessionCount> mEntries;
};

} // namespace Transport
} // namespace chip
