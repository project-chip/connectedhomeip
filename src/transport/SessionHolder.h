/*
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <lib/core/Optional.h>
#include <transport/SessionDelegate.h>
#include <transport/SessionHandle.h>

namespace chip {

/** @brief
 *    Managed session reference. The object is used to store a session, the stored session will be automatically
 *    released when the underlying session is released. One must verify it is available before use. The object can be
 *    created using SessionHandle.Grab()
 *
 *    TODO: release holding session when the session is released. This will be implemented by following PRs
 */
class SessionHolder : public SessionReleaseDelegate
{
public:
    SessionHolder() {}
    SessionHolder(const SessionHandle & session) : mSession(session) {}
    ~SessionHolder() { Release(); }

    SessionHolder(const SessionHolder &);
    SessionHolder operator=(const SessionHolder &);
    SessionHolder(SessionHolder && that);
    SessionHolder operator=(SessionHolder && that);

    void Grab(const SessionHandle & sessionHandle)
    {
        Release();
        mSession.SetValue(sessionHandle);
    }

    void Release() { mSession.ClearValue(); }

    // TODO: call this function when the underlying session is released
    // Implement SessionReleaseDelegate
    void OnSessionReleased(const SessionHandle & session) override { Release(); }

    // Check whether the SessionHolder contains a session matching given session
    bool Contains(const SessionHandle & session) const { return mSession.HasValue() && mSession.Value() == session; }

    operator bool() const { return mSession.HasValue(); }
    const SessionHandle & Get() const { return mSession.Value(); }
    Optional<SessionHandle> ToOptional() const { return mSession; }

private:
    Optional<SessionHandle> mSession;
};

} // namespace chip
