/*
 *    Copyright (c) 2022 Project CHIP Authors
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
#include <lib/support/ReferenceCountedHandle.h>

namespace chip {

namespace Transport {
class Session;
} // namespace Transport

class SessionHolder;

/** @brief
 *    A shared_ptr like smart pointer to manage the lifetime of a Session object. Like a shared_ptr,
 *    this object can start out not tracking any Session and be attached to a Session there-after.
 *    The underlying Session is guaranteed to remain active and resident until all references to it from SessionSharedPtr
 *    instances in the system have gone away, at which point it invokes its custom destructor.
 *
 *    Just because a Session is refcounted does not mean it actually gets destroyed upon reaching a count of 0.
 *    UnauthenticatedSession and SecureSession have different logic that gets invoked when the count hits 0.
 *
 *    This should really only be used during session setup by the entity setting up the session.
 *    Once setup, the session should transfer ownership to the SessionManager at which point,
 *    all clients in the system should only be holding SessionWeakPtrs (SessionWeakPtr doesn't exist yet, but once
 *    #18399 is complete, SessionHolder will become SessionWeakPtr).
 *
 *    This is copy-constructible.
 */
class SessionSharedPtr
{
public:
    SessionSharedPtr() {}
    SessionSharedPtr(Transport::Session & session) : mSession(InPlace, session) {}

    SessionSharedPtr(const SessionSharedPtr &) = default;
    SessionSharedPtr & operator=(const SessionSharedPtr &) = default;

    /*
     * If we're currently pointing to a valid session, remove ourselves
     * as a shared owner of that session. If there are no more shared owners
     * on that session, that session MAY be reclaimed.
     */
    void Release() { mSession.ClearValue(); }

    // Transport::Session * operator->() const { return Get(); }

private:
    Optional<ReferenceCountedHandle<Transport::Session>> mSession;
};

} // namespace chip
