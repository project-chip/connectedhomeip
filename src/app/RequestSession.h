/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <optional>

#include <transport/Session.h>

namespace chip {
namespace app {

/**
 * Captures the destination used to encode an Invoke or Write Request.
 * Unicast sessions are retained without shifting to newer sessions until Release() or destruction.
 * Group destinations are stored by group ID and fabric index without retaining the group session.
 */
class RequestSession final : private SessionDelegate
{
public:
    explicit RequestSession(const OptionalSessionHandleRef & session);

    RequestSession(const RequestSession &)             = delete;
    RequestSession & operator=(const RequestSession &) = delete;

    bool IsGroupSession() const { return mKind == Kind::kGroupcast; }

    /// Returns the peer interaction model revision reported by the construction session. When the session reports none
    /// (groupcast, no construction session, or a peer that reported none), returns the revision assumed by the client, if any.
    std::optional<uint16_t> GetPeerInteractionModelRevision() const
    {
        return mPeerInteractionModelRevision.has_value() ? mPeerInteractionModelRevision : mFallbackPeerInteractionModelRevision;
    }

    /// Sets the revision the client assumes for the peer. GetPeerInteractionModelRevision() uses it only when the
    /// construction session reports none.
    void SetFallbackPeerInteractionModelRevision(std::optional<uint16_t> aFallbackRevision)
    {
        mFallbackPeerInteractionModelRevision = aFallbackRevision;
    }

    /// Returns CHIP_ERROR_INCORRECT_STATE if the session was not available in the constructor or has since been released.
    CHIP_ERROR Validate() const;

    /// Check that the provided session matches we were constructed, with as follows:
    ///
    /// 1) If we were constructed without a session, any session is allowed here.
    /// 2) If we were constructed with a unicast session, the session being checked must be the same session.
    /// 3) If we were constructed with a groupcast session, the session being checked must also be a groupcast session that has the
    /// same group ID and fabric.
    CHIP_ERROR CheckSession(const SessionHandle & session) const;

    /// Releases the retained session and prevents further use of this request destination.
    void Release();

private:
    enum class Kind : uint8_t
    {
        kUnspecified, // No session was supplied; use legacy encoding and allow any session at send time.
        kUnicast,     // Retain the captured unicast session and require that same session when sending.
        kGroupcast,   // Capture the group ID and fabric index and require that destination when sending.
        kUnavailable, // Capture failed, the session was released, or Release() was called; further use fails.
    };

    NewSessionHandlingPolicy GetNewSessionHandlingPolicy() override { return NewSessionHandlingPolicy::kStayAtOldSession; }
    void OnSessionReleased() override;

    SessionHolderWithDelegate mUnicastSession{ *this };
    std::optional<uint16_t> mPeerInteractionModelRevision;
    std::optional<uint16_t> mFallbackPeerInteractionModelRevision;
    GroupId mGroupId         = kUndefinedGroupId;
    FabricIndex mFabricIndex = kUndefinedFabricIndex;
    Kind mKind               = Kind::kUnspecified;
};

} // namespace app
} // namespace chip
