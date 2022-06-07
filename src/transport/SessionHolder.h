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
class SessionHolder : public SessionDelegate, public IntrusiveListNodeBase
{
public:
    SessionHolder() {}
    ~SessionHolder() override;

    SessionHolder(const SessionHolder &);
    SessionHolder(SessionHolder && that);
    SessionHolder & operator=(const SessionHolder &);
    SessionHolder & operator=(SessionHolder && that);

    // Implement SessionDelegate
    void OnSessionReleased() override { Release(); }

    bool Contains(const SessionHandle & session) const
    {
        return mSession.HasValue() && &mSession.Value().Get() == &session.mSession.Get();
    }

    bool GrabPairingSession(const SessionHandle & session); // Should be only used inside CASE/PASE pairing.
    bool Grab(const SessionHandle & session);
    void Release();

    operator bool() const { return mSession.HasValue(); }
    Optional<SessionHandle> Get() const
    {
        //
        // We cannot return mSession directly even if Optional<SessionHandle> is internally composed of the same bits,
        // since they are not actually equivalent type-wise, and SessionHandle does not permit copy-construction.
        //
        // So, construct a new Optional<SessionHandle> from the underlying Transport::Session reference.
        //
        return mSession.HasValue() ? chip::MakeOptional<SessionHandle>(mSession.Value().Get())
                                   : chip::Optional<SessionHandle>::Missing();
    }

    Transport::Session * operator->() const { return &mSession.Value().Get(); }

    // There is not delegate, nothing to do here
    virtual void DispatchSessionEvent(SessionDelegate::Event event) {}

private:
    Optional<ReferenceCountedHandle<Transport::Session>> mSession;
};

/// @brief Extends SessionHolder to allow propagate SessionDelegate::* events to a given destination
class SessionHolderWithDelegate : public SessionHolder
{
public:
    SessionHolderWithDelegate(SessionDelegate & delegate) : mDelegate(delegate) {}
    SessionHolderWithDelegate(const SessionHandle & handle, SessionDelegate & delegate) : mDelegate(delegate) { Grab(handle); }
    operator bool() const { return SessionHolder::operator bool(); }

    void OnSessionReleased() override
    {
        Release();

        // Note, the session is already cleared during mDelegate.OnSessionReleased
        mDelegate.OnSessionReleased();
    }

    void DispatchSessionEvent(SessionDelegate::Event event) override { (mDelegate.*event)(); }

private:
    SessionDelegate & mDelegate;
};

} // namespace chip
