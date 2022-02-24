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

#include <access/AuthMode.h>
#include <transport/SecureSession.h>

namespace chip {
namespace Transport {

Access::Subject SecureSession::GetSubject() const
{
    if (IsOperationalNodeId(mPeerNodeId))
    {
        return Access::Subject::Create<Access::NodeSubject>(GetFabricIndex(), mPeerNodeId);
    }
    else if (IsPAKEKeyId(mPeerNodeId))
    {
        return Access::Subject::CreatePaseSubject();
    }
    else
    {
        VerifyOrDie(false);
        return Access::Subject();
    }
}

bool SecureSession::MatchSubject(const Access::Subject & subject) const
{
    return GetFabricIndex() == subject.GetFabricIndex() && MatchSubject(subject.GetScopedSubject());
}

bool SecureSession::MatchSubject(const Access::ScopedSubject & scopedSubject) const
{
    if (IsOperationalNodeId(mPeerNodeId))
    {
        if (Access::ScopedSubject::Create<Access::NodeSubject>(mPeerNodeId).Match(scopedSubject))
        {
            return true;
        }
    }
    else if (IsPAKEKeyId(mPeerNodeId))
    {
        if (Access::ScopedSubject::Create<Access::PaseSubject>().Match(scopedSubject))
        {
            return true;
        }
    }
    else
    {
        VerifyOrDie(false);
        return false;
    }

    // TODO: match againse CATs
    return false;
}

Access::SubjectDescriptor SecureSession::GetSubjectDescriptor() const
{
    Access::SubjectDescriptor subjectDescriptor;
    if (IsOperationalNodeId(mPeerNodeId))
    {
        subjectDescriptor.authMode    = Access::AuthMode::kCase;
        subjectDescriptor.subject     = mPeerNodeId;
        subjectDescriptor.cats        = mPeerCATs;
        subjectDescriptor.fabricIndex = GetFabricIndex();
    }
    else if (IsPAKEKeyId(mPeerNodeId))
    {
        subjectDescriptor.authMode    = Access::AuthMode::kPase;
        subjectDescriptor.subject     = mPeerNodeId;
        subjectDescriptor.fabricIndex = GetFabricIndex();
    }
    else
    {
        VerifyOrDie(false);
    }
    return subjectDescriptor;
}

} // namespace Transport
} // namespace chip
