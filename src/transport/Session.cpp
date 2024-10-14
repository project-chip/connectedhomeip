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

const SecureSession * Session::AsConstSecureSession() const
{
    VerifyOrDie(GetSessionType() == SessionType::kSecure);
    return static_cast<const SecureSession *>(this);
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

const IncomingGroupSession * Session::AsConstIncomingGroupSession() const
{
    VerifyOrDie(GetSessionType() == SessionType::kGroupIncoming);
    return static_cast<const IncomingGroupSession *>(this);
}

OutgoingGroupSession * Session::AsOutgoingGroupSession()
{
    VerifyOrDie(GetSessionType() == SessionType::kGroupOutgoing);
    return static_cast<OutgoingGroupSession *>(this);
}

const OutgoingGroupSession * Session::AsConstOutgoingGroupSession() const
{
    VerifyOrDie(GetSessionType() == SessionType::kGroupOutgoing);
    return static_cast<const OutgoingGroupSession *>(this);
}

System::Clock::Timeout Session::ComputeRoundTripTimeout(System::Clock::Timeout upperlayerProcessingTimeout)
{
    if (IsGroupSession())
    {
        return System::Clock::kZero;
    }

    // Treat us as active for purposes of GetMessageReceiptTimeout(), since the
    // other side would be responding to our message.
    return GetAckTimeout() + upperlayerProcessingTimeout + GetMessageReceiptTimeout(System::SystemClock().GetMonotonicTimestamp());
}

uint16_t Session::SessionIdForLogging() const
{
    switch (GetSessionType())
    {
    case Session::SessionType::kGroupIncoming:
        return AsConstIncomingGroupSession()->GetGroupId();
    case Session::SessionType::kGroupOutgoing:
        return AsConstOutgoingGroupSession()->GetGroupId();
    case Session::SessionType::kSecure:
        return AsConstSecureSession()->GetLocalSessionId();
    case Session::SessionType::kUnauthenticated:
        return 0;
    default:
        VerifyOrDie(false);
        return 0;
    }
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
