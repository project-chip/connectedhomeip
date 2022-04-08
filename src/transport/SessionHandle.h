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

namespace chip {

namespace Transport {
class Session;
} // namespace Transport

class SessionHolder;

/** @brief
 *    Non-copyable session reference. All SessionHandles are created within SessionManager. SessionHandle is not
 *    reference *counted, hence it is not allowed to store SessionHandle anywhere except for function arguments and
 *    return values. SessionHandle is short-lived as it is only available as stack variable, so it is never dangling. */
class SessionHandle
{
public:
    SessionHandle(Transport::Session & session) : mSession(session) {}
    ~SessionHandle() {}

    SessionHandle(const SessionHandle &) = delete;
    SessionHandle operator=(const SessionHandle &) = delete;
    SessionHandle(SessionHandle &&)                = default;
    SessionHandle & operator=(SessionHandle &&) = delete;

    bool operator==(const SessionHandle & that) const { return &mSession.Get() == &that.mSession.Get(); }

    Transport::Session * operator->() const { return mSession.operator->(); }

private:
    friend class SessionHolder;
    ReferenceCountedHandle<Transport::Session> mSession;
};

} // namespace chip
