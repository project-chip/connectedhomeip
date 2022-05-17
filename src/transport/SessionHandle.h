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
#include <lib/core/Optional.h>
#include <lib/support/ReferenceCountedHandle.h>
#include <transport/SessionSharedPtr.h>

namespace chip {

namespace Transport {
class Session;
} // namespace Transport

class SessionHolder;

/** @brief
 *
 *    A non-copyable version of SessionSharedPtr that is always guaranteed to point to a valid Session. This is always
 *    created within SessionManager.
 *
 *    Since this is a strong reference, this should be short-lived and only used as a stack variable.
 */
class SessionHandle : protected SessionSharedPtr
{
public:
    SessionHandle(Transport::Session & session) : SessionSharedPtr(session) {}
    ~SessionHandle() {}

    SessionHandle(const SessionHandle &) = delete;
    SessionHandle operator=(const SessionHandle &) = delete;
    SessionHandle(SessionHandle &&)                = default;
    SessionHandle & operator=(SessionHandle &&) = delete;

    bool operator==(const SessionHandle & that) const { return Get() == that.Get(); }

    Transport::Session * operator->() const { return SessionSharedPtr::operator->(); }

private:
    // TODO: Remove this once SessionHolder pivots to truly being a weak reference (#18399).
    friend class SessionHolder;
};

} // namespace chip
