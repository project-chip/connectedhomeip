/*
 *
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

#include <access/SubjectDescriptor.h>
#include <lib/support/ReferenceCountedHandle.h>
#include <lib/core/Optional.h>

namespace chip {

namespace Transport {
class Session;
} // namespace Transport

class SessionHolder;

/** @brief
 *
 * A shared_ptr like smart pointer to manage the lifetime of a Session object. Like a shared_ptr,
 * this object can start out not tracking any Session and be attached to a Session there-after.
 * The underlying Session is guaranteed to remain active and resident until all references to it from SessionSharedPtr
 * instances in the system have gone away, at which point it invokes its custom destructor.
 *
 * Just because a Session is refcounted does not mean it actually gets destroyed upon reaching a count of 0.
 * UnauthenticatedSession and SecureSession have different logic that gets invoked when the count hits 0.
 *
 * This should really only be used during session setup by the entity setting up the session.
 * Once setup, the session should transfer ownership to the SessionManager at which point,
 * all clients in the system should only be holding SessionWeakPtrs (SessionWeakPtr doesn't exist yet, but once #18399 is complete, SessionHolder
 * will become SessionWeakPtr).
 *
 * This is copy-constructible.
 *
 */
class SessionSharedPtr
{
public:
    SessionSharedPtr() {}
    SessionSharedPtr(Transport::Session & session) {
        mSession.Emplace(session);
    }

    SessionSharedPtr(const SessionSharedPtr & session) {
        Grab(session);
    }

    /*
     * Add ourselves as a shared owner on the passed in session (if it points to
     * a valid session). The passed in
     * session object can now be free'ed safely without impacting the underlying
     * Session.
     */
    void operator=(const SessionSharedPtr & session) {
        Grab(session);
    }

    /*
     * If we're currently pointing to a valid session, remove ourselves
     * as a shared owner of that session. If there are no more shared owners
     * on that session, that session MAY be reclaimed.
     */
    void Release() {
        mSession.ClearValue();
    }

    /*
     * Assume shared ownership of the provided session.
     */
    void Grab(Transport::Session &session) {
        mSession.Emplace(session);
    }

    Transport::Session* operator->() const {
        return Get();
    }

protected:
    Transport::Session* Get() const {
        if (mSession.HasValue()) {
            return &mSession.Value().Get();
        }

        return nullptr;
    }

private:
    void Grab(const SessionSharedPtr & session) {
        auto *underlyingSession = session.Get();
        if (underlyingSession) {
            mSession.Emplace(*underlyingSession);
        }
    }

    Optional<ReferenceCountedHandle<Transport::Session>> mSession;

    // TODO: Remove this once SessionHolder pivots to truly being a weak reference (#18399).
    friend class SessionHolder;
};

} // namespace chip
