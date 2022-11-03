/*
 * SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <transport/GroupSession.h>
#include <transport/SecureSession.h>
#include <transport/Session.h>
#include <transport/UnauthenticatedSessionTable.h>

namespace chip {
namespace Transport {

SecureSession * Session::AsSecureSession()
{
    VerifyOrDie(GetSessionType() == SessionType::kSecure);
    return static_cast<SecureSession *>(this);
}

UnauthenticatedSession * Session::AsUnauthenticatedSession()
{
    VerifyOrDie(GetSessionType() == SessionType::kUnauthenticated);
    return static_cast<UnauthenticatedSession *>(this);
}

IncomingGroupSession * Session::AsIncomingGroupSession()
{
    VerifyOrDie(GetSessionType() == SessionType::kGroupIncoming);
    return static_cast<IncomingGroupSession *>(this);
}

OutgoingGroupSession * Session::AsOutgoingGroupSession()
{
    VerifyOrDie(GetSessionType() == SessionType::kGroupOutgoing);
    return static_cast<OutgoingGroupSession *>(this);
}

System::Clock::Timeout Session::ComputeRoundTripTimeout(System::Clock::Timeout upperlayerProcessingTimeout)
{
    if (IsGroupSession())
    {
        return System::Clock::kZero;
    }
    return GetAckTimeout() + upperlayerProcessingTimeout;
}

const char * GetSessionTypeString(const SessionHandle & session)
{
    switch (session->GetSessionType())
    {
    case Session::SessionType::kGroupIncoming:
    case Session::SessionType::kGroupOutgoing:
        return "G";
    case Session::SessionType::kSecure:
        return "S";
    case Session::SessionType::kUnauthenticated:
        return "U";
    default:
        return "?";
    }
}

} // namespace Transport
} // namespace chip
