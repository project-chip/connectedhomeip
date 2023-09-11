/*
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
