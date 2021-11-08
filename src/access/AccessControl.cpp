/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
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

#include "AccessControl.h"

namespace {

using chip::FabricIndex;
using namespace chip::Access;

AccessControl defaultAccessControl;
AccessControl * globalAccessControl = &defaultAccessControl;

static_assert((int(Privilege::kAdminister) & int(Privilege::kManage)) == 0);
static_assert((int(Privilege::kAdminister) & int(Privilege::kOperate)) == 0);
static_assert((int(Privilege::kAdminister) & int(Privilege::kView)) == 0);
static_assert((int(Privilege::kAdminister) & int(Privilege::kProxyView)) == 0);
static_assert((int(Privilege::kManage) & int(Privilege::kOperate)) == 0);
static_assert((int(Privilege::kManage) & int(Privilege::kView)) == 0);
static_assert((int(Privilege::kManage) & int(Privilege::kProxyView)) == 0);
static_assert((int(Privilege::kOperate) & int(Privilege::kView)) == 0);
static_assert((int(Privilege::kOperate) & int(Privilege::kProxyView)) == 0);
static_assert((int(Privilege::kView) & int(Privilege::kProxyView)) == 0);

int GetGrantedPrivileges(Privilege privilege)
{
    switch (privilege)
    {
        case Privilege::kView:
            return int(Privilege::kView);
        case Privilege::kProxyView:
            return int(Privilege::kProxyView) | int(Privilege::kView);
        case Privilege::kOperate:
            return int(Privilege::kOperate) | int(Privilege::kView);
        case Privilege::kManage:
            return int(Privilege::kManage) | int(Privilege::kOperate) | int(Privilege::kView);
        case Privilege::kAdminister:
            return int(Privilege::kAdminister) | int(Privilege::kManage) | int(Privilege::kOperate) | int(Privilege::kView) | int(Privilege::kProxyView);
    }
    return 0;
}

} // namespace

namespace chip {
namespace Access {

AccessControl::Entry::Delegate AccessControl::Entry::mDefaultDelegate;
AccessControl::EntryIterator::Delegate AccessControl::EntryIterator::mDefaultDelegate;
AccessControl::Delegate AccessControl::mDefaultDelegate;

CHIP_ERROR AccessControl::Init()
{
    ChipLogDetail(DataManagement, "AccessControl::Init");
    return mDelegate.Init();
}

CHIP_ERROR AccessControl::Finish()
{
    ChipLogDetail(DataManagement, "AccessControl::Finish");
    return mDelegate.Finish();
}

CHIP_ERROR AccessControl::Check(const SubjectDescriptor & subjectDescriptor, const RequestPath & requestPath, Privilege requestPrivilege)
{
    EntryIterator iterator;
    ReturnErrorOnFailure(Entries(iterator, &subjectDescriptor.fabricIndex));

    Entry entry;
    while (iterator.Next(entry) == CHIP_NO_ERROR)
    {
        ChipLogDetail(DataManagement, "got an entry");

        AuthMode authMode;
        ReturnErrorOnFailure(entry.GetAuthMode(authMode));
        if (authMode != subjectDescriptor.authMode)
        {
            continue;
        }

        Privilege privilege;
        ReturnErrorOnFailure(entry.GetPrivilege(privilege));
        int grantedPrivileges = GetGrantedPrivileges(privilege);
        if ((grantedPrivileges & int(requestPrivilege)) == 0)
        {
            continue;
        }

        size_t subjectCount;
        NodeId subject;
        ReturnErrorOnFailure(entry.GetSubjectCount(subjectCount));
        if (subjectCount > 0)
        {
            for (size_t i = 0; i < subjectCount; ++i)
            {
                ReturnErrorOnFailure(entry.GetSubject(i, subject));
                if (subject == subjectDescriptor.subjects[0])
                {
                    goto subjectMatched;
                }
                // TODO: check against CATs in subject descriptor
            }
            continue;
        }
        subjectMatched:

        size_t targetCount;
        Entry::Target target;
        ReturnErrorOnFailure(entry.GetTargetCount(targetCount));
        if (targetCount > 0)
        {
            for (size_t i = 0; i < targetCount; ++i)
            {
                ReturnErrorOnFailure(entry.GetTarget(i, target));
                if ((target.flags & Entry::Target::kCluster) && target.cluster != requestPath.cluster)
                {
                    continue;
                }
                if ((target.flags & Entry::Target::kEndpoint) && target.endpoint != requestPath.endpoint)
                {
                    continue;
                }
                // TODO: check against target.deviceType (requires lookup)
                goto targetMatched;
            }
            continue;
        }
        targetMatched:

        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_ACCESS_DENIED;
}

AccessControl & GetAccessControl()
{
    return *globalAccessControl;
}

void ResetAccessControl()
{
    globalAccessControl = &defaultAccessControl;
}

void SetAccessControl(AccessControl & accessControl)
{
    globalAccessControl = &accessControl;
}

} // namespace Access
} // namespace chip
