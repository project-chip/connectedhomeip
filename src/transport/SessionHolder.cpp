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

#include <transport/SessionHolder.h>

#include <transport/SecureSession.h>
#include <transport/Session.h>

namespace chip {

SessionHolder::~SessionHolder()
{
    Release();
}

SessionHolder::SessionHolder(const SessionHolder & that) : IntrusiveListNodeBase()
{
    mSession = that.mSession;
    if (mSession.HasValue())
    {
        mSession.Value()->AddHolder(*this);
    }
}

SessionHolder::SessionHolder(SessionHolder && that) : IntrusiveListNodeBase()
{
    mSession = that.mSession;
    if (mSession.HasValue())
    {
        mSession.Value()->AddHolder(*this);
    }

    that.Release();
}

SessionHolder & SessionHolder::operator=(const SessionHolder & that)
{
    Release();

    mSession = that.mSession;
    if (mSession.HasValue())
    {
        mSession.Value()->AddHolder(*this);
    }

    return *this;
}

SessionHolder & SessionHolder::operator=(SessionHolder && that)
{
    Release();

    mSession = that.mSession;
    if (mSession.HasValue())
    {
        mSession.Value()->AddHolder(*this);
    }

    that.Release();

    return *this;
}

bool SessionHolder::GrabPairingSession(const SessionHandle & session)
{
    Release();

    if (!session->IsSecureSession())
        return false;

    if (!session->AsSecureSession()->IsEstablishing())
        return false;

    GrabUnchecked(session);
    return true;
}

bool SessionHolder::Grab(const SessionHandle & session)
{
    Release();

    if (!session->IsActiveSession())
        return false;

    GrabUnchecked(session);
    return true;
}

void SessionHolder::GrabUnchecked(const SessionHandle & session)
{
    VerifyOrDie(!mSession.HasValue());
    mSession.Emplace(session.mSession);
    session->AddHolder(*this);
}

void SessionHolder::Release()
{
    if (mSession.HasValue())
    {
        mSession.Value()->RemoveHolder(*this);
        mSession.ClearValue();
    }
}

} // namespace chip
