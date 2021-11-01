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

#include <lib/support/Variant.h>
#include <transport/Session.h>
#include <transport/UnauthenticatedSessionTable.h>

namespace chip {

class SessionManager;

/*
 * @brief A handle to all types of sessions
 *
 * A SessionHandle always referring to a valid session, it will never dangling.
 */
class SessionHandle
{
public:
    SessionHandle(SessionManager & sessionManager, uint16_t localSessionId) :
        mSession(InPlaceTemplate<VariantSecureSession>, sessionManager, localSessionId)
    {}
    SessionHandle(const Transport::UnauthenticatedSessionHandle & session) :
        mSession(InPlaceTemplate<VariantUnauthenticatedSession>, session)
    {}

    bool operator==(const SessionHandle & that) const { return mSession == that.mSession; }

    // Access the abstract session interface
    Session * operator->() const;

    // Access underlying actual session object, it is not recommended to access the underlying object, please use the abstract
    // session interface when possible
    Transport::SecureSession * AsSecureSession() const;
    Transport::UnauthenticatedSessionHandle AsUnauthenticatedSession() const;

private:
    friend class SecureSessionMgr;

    struct VariantSecureSession
    {
        VariantSecureSession(SessionManager & sessionManager, uint16_t localSessionId) :
            mSessionManager(sessionManager), mLocalSessionId(localSessionId)
        {}
        bool operator==(const VariantSecureSession & that) const { return mLocalSessionId == that.mLocalSessionId; }

        // Include a ref to session manager to make the handler easy to use.
        // Replace these field with a raw pointer to the actual secure session, once we have figured out how to prevent dangling
        // session.
        SessionManager & mSessionManager;
        uint16_t mLocalSessionId;
    };

    struct VariantUnauthenticatedSession
    {
        VariantUnauthenticatedSession(const Transport::UnauthenticatedSessionHandle & unauthenticatedSessionHandle) :
            mUnauthenticatedSessionHandle(unauthenticatedSessionHandle)
        {}
        bool operator==(const VariantUnauthenticatedSession & that) const
        {
            return mUnauthenticatedSessionHandle == that.mUnauthenticatedSessionHandle;
        }
        Transport::UnauthenticatedSessionHandle mUnauthenticatedSessionHandle;
    };

    Variant<VariantSecureSession, VariantUnauthenticatedSession> mSession;
};

} // namespace chip
