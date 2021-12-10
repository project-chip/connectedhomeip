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

#include <transport/SecureSession.h>
#include <transport/SessionHandle.h>
#include <transport/SessionManager.h>

namespace chip {

using namespace Transport;

using SubjectDescriptor = Access::SubjectDescriptor;

SubjectDescriptor SessionHandle::GetSubjectDescriptor() const
{
    SubjectDescriptor subjectDescriptor = { .fabricIndex = mFabric };
    // TODO: fill subject descriptor with proper fields
    return subjectDescriptor;
}

const PeerAddress * SessionHandle::GetPeerAddress(SessionManager * sessionManager) const
{
    if (IsSecure())
    {
        SecureSession * state = sessionManager->GetSecureSession(*this);
        if (state == nullptr)
        {
            return nullptr;
        }

        return &state->GetPeerAddress();
    }

    return &GetUnauthenticatedSession()->GetPeerAddress();
}

const ReliableMessageProtocolConfig & SessionHandle::GetMRPConfig(SessionManager * sessionManager) const
{
    if (IsSecure())
    {
        SecureSession * secureSession = sessionManager->GetSecureSession(*this);
        if (secureSession == nullptr)
        {
            return gDefaultMRPConfig;
        }
        return secureSession->GetMRPConfig();
    }
    else
    {
        return GetUnauthenticatedSession()->GetMRPConfig();
    }
}

void SessionHandle::SetMRPConfig(SessionManager * sessionManager, const ReliableMessageProtocolConfig & config)
{
    if (IsSecure())
    {
        SecureSession * secureSession = sessionManager->GetSecureSession(*this);
        if (secureSession != nullptr)
        {
            secureSession->SetMRPConfig(config);
        }
    }
    else
    {
        return GetUnauthenticatedSession()->SetMRPConfig(config);
    }
}

} // namespace chip
