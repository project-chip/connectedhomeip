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

using AuthMode          = Access::AuthMode;
using SubjectDescriptor = Access::SubjectDescriptor;

SubjectDescriptor SessionHandle::GetSubjectDescriptor() const
{
    SubjectDescriptor subjectDescriptor;
    if (IsSecure())
    {
        if (IsOperationalNodeId(mPeerNodeId))
        {
            subjectDescriptor.authMode    = AuthMode::kCase;
            subjectDescriptor.subject     = mPeerNodeId;
            subjectDescriptor.fabricIndex = mFabric;
            // TODO(#10243): add CATs
        }
        else if (IsPAKEKeyId(mPeerNodeId))
        {
            subjectDescriptor.authMode = AuthMode::kPase;
            subjectDescriptor.subject  = mPeerNodeId;
            // TODO(#10242): PASE *can* have fabric in some situations
        }
        else if (mGroupId.HasValue())
        {
            subjectDescriptor.authMode = AuthMode::kGroup;
            subjectDescriptor.subject  = NodeIdFromGroupId(mGroupId.Value());
        }
    }
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
