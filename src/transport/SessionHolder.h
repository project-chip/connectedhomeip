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
#include <lib/support/IntrusiveList.h>
#include <transport/SessionDelegate.h>
#include <transport/SessionHandle.h>

namespace chip {

/** @brief
 *    Managed session reference. The object is used to store a session, the stored session will be automatically
 *    released when the underlying session is released. One must verify it is available before use. The object can be
 *    created using SessionHandle.Grab()
 */
class SessionHolder : public SessionReleaseDelegate, public IntrusiveListNodeBase
{
public:
    SessionHolder() {}
    ~SessionHolder() override;

    SessionHolder(const SessionHolder &);
    SessionHolder(SessionHolder && that);
    SessionHolder & operator=(const SessionHolder &);
    SessionHolder & operator=(SessionHolder && that);

    // Implement SessionReleaseDelegate
    void OnSessionReleased() override { Release(); }

    bool Contains(const SessionHandle & session) const
    {
        return mSession.HasValue() && &mSession.Value().Get() == &session.mSession.Get();
    }

    void Grab(const SessionHandle & session);
    void Release();

    operator bool() const { return mSession.HasValue(); }
    SessionHandle Get() const { return SessionHandle{ mSession.Value().Get() }; }
    Optional<SessionHandle> ToOptional() const
    {
        return mSession.HasValue() ? chip::MakeOptional<SessionHandle>(Get()) : chip::Optional<SessionHandle>::Missing();
    }

    Transport::Session * operator->() const { return &mSession.Value().Get(); }

private:
    Optional<ReferenceCountedHandle<Transport::Session>> mSession;
};

// @brief Extends SessionHolder to allow propagate OnSessionReleased event to an extra given destination
class SessionHolderWithDelegate : public SessionHolder
{
public:
    SessionHolderWithDelegate(SessionReleaseDelegate & delegate) : mDelegate(delegate) {}
    SessionHolderWithDelegate(const SessionHandle & handle, SessionReleaseDelegate & delegate) : mDelegate(delegate)
    {
        Grab(handle);
    }
    operator bool() const { return SessionHolder::operator bool(); }

    void OnSessionReleased() override
    {
        Release();

        // Note, the session is already cleared during mDelegate.OnSessionReleased
        mDelegate.OnSessionReleased();
    }

private:
    SessionReleaseDelegate & mDelegate;
};

} // namespace chip
