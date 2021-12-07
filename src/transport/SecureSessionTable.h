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
     * @param sessionType secure session type
     * @param localSessionId represents the encryption key ID assigned by local node
     * @param peerNodeId represents peer Node's ID
     * @param peerCATs represents peer CASE Authenticated Tags
     * @param peerSessionId represents the encryption key ID assigned by peer node
     * @param fabric represents fabric ID for the session
     * @param config represents the reliable message protocol configuration
     *
     * @note the newly created state will have an 'active' time set based on the current time source.
     *
     * @returns CHIP_NO_ERROR if state could be initialized. May fail if maximum session count
     *          has been reached (with CHIP_ERROR_NO_MEMORY).
     */
    CHECK_RETURN_VALUE
    SecureSession * CreateNewSecureSession(SecureSession::Type secureSessionType, uint16_t localSessionId, NodeId peerNodeId,
                                           Credentials::CATValues peerCATs, uint16_t peerSessionId, FabricIndex fabric,
                                           const ReliableMessageProtocolConfig & config)
    {
        return mEntries.CreateObject(secureSessionType, localSessionId, peerNodeId, peerCATs, peerSessionId, fabric, config,
                                     mTimeSource.GetMonotonicTimestamp());
    }

    void ReleaseSession(SecureSession * session) { mEntries.ReleaseObject(session); }

    template <typename Function>
    Loop ForEachSession(Function && function)
    {
        return mEntries.ForEachActiveObject(std::forward<Function>(function));
    }

    /**
     * Get a secure session given a Node Id and Peer's Encryption Key Id.
     *
     * @param localSessionId Encryption key ID used by the local node.
     *
     * @return the state found, nullptr if not found
     */
    CHECK_RETURN_VALUE
    SecureSession * FindSecureSessionByLocalKey(uint16_t localSessionId)
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
        return result;
    }

    /// Convenience method to mark a session as active
    void MarkSessionActive(SecureSession * state) { state->SetLastActivityTime(mTimeSource.GetMonotonicTimestamp()); }

    /**
     * Iterates through all active sessions and expires any sessions with an idle time
     * larger than the given amount.
     *
     * Expiring a session involves callback execution and then clearing the internal state.
     */
    template <typename Callback>
    void ExpireInactiveSessions(System::Clock::Timestamp maxIdleTime, Callback callback)
    {
        const System::Clock::Timestamp currentTime = mTimeSource.GetMonotonicTimestamp();
        mEntries.ForEachActiveObject([&](auto session) {
            if (session->GetLastActivityTime() + maxIdleTime < currentTime)
            {
                callback(*session);
                ReleaseSession(session);
            }
            return Loop::Continue;
        });
    }

    /// Allows access to the underlying time source used for keeping track of session active time
    Time::TimeSource<kTimeSource> & GetTimeSource() { return mTimeSource; }

private:
    Time::TimeSource<kTimeSource> mTimeSource;
    BitMapObjectPool<SecureSession, kMaxSessionCount, OnObjectPoolDestruction::IgnoreUnsafeDoNotUseInNewCode> mEntries;
};

} // namespace Transport
} // namespace chip
