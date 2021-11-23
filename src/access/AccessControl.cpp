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

static_assert(((unsigned(Privilege::kAdminister) & unsigned(Privilege::kManage)) == 0) &&
                  ((unsigned(Privilege::kAdminister) & unsigned(Privilege::kOperate)) == 0) &&
                  ((unsigned(Privilege::kAdminister) & unsigned(Privilege::kView)) == 0) &&
                  ((unsigned(Privilege::kAdminister) & unsigned(Privilege::kProxyView)) == 0) &&
                  ((unsigned(Privilege::kManage) & unsigned(Privilege::kOperate)) == 0) &&
                  ((unsigned(Privilege::kManage) & unsigned(Privilege::kView)) == 0) &&
                  ((unsigned(Privilege::kManage) & unsigned(Privilege::kProxyView)) == 0) &&
                  ((unsigned(Privilege::kOperate) & unsigned(Privilege::kView)) == 0) &&
                  ((unsigned(Privilege::kOperate) & unsigned(Privilege::kProxyView)) == 0) &&
                  ((unsigned(Privilege::kView) & unsigned(Privilege::kProxyView)) == 0),
              "Privilege bits must be unique");

bool CheckRequestPrivilegeAgainstEntryPrivilege(Privilege requestPrivilege, Privilege entryPrivilege)
{
    switch (entryPrivilege)
    {
    case Privilege::kView:
        return requestPrivilege == Privilege::kView;
    case Privilege::kProxyView:
        return requestPrivilege == Privilege::kProxyView || requestPrivilege == Privilege::kView;
    case Privilege::kOperate:
        return requestPrivilege == Privilege::kOperate || requestPrivilege == Privilege::kView;
    case Privilege::kManage:
        return requestPrivilege == Privilege::kManage || requestPrivilege == Privilege::kOperate ||
            requestPrivilege == Privilege::kView;
    case Privilege::kAdminister:
        return requestPrivilege == Privilege::kAdminister || requestPrivilege == Privilege::kManage ||
            requestPrivilege == Privilege::kOperate || requestPrivilege == Privilege::kView ||
            requestPrivilege == Privilege::kProxyView;
    }
    return false;
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

CHIP_ERROR AccessControl::Check(const SubjectDescriptor & subjectDescriptor, const RequestPath & requestPath,
                                Privilege requestPrivilege)
{
    // During development, allow access if delegate is transitional
    ReturnErrorCodeIf(mDelegate.IsTransitional(), CHIP_NO_ERROR);

    EntryIterator iterator;
    ReturnErrorOnFailure(Entries(iterator, &subjectDescriptor.fabricIndex));

    Entry entry;
    while (iterator.Next(entry) == CHIP_NO_ERROR)
    {
        AuthMode authMode = AuthMode::kNone;
        ReturnErrorOnFailure(entry.GetAuthMode(authMode));
        if (authMode != subjectDescriptor.authMode)
        {
            continue;
        }

        Privilege privilege = Privilege::kView;
        ReturnErrorOnFailure(entry.GetPrivilege(privilege));
        if (!CheckRequestPrivilegeAgainstEntryPrivilege(requestPrivilege, privilege))
        {
            continue;
        }

        size_t subjectCount = 0;
        ReturnErrorOnFailure(entry.GetSubjectCount(subjectCount));
        if (subjectCount > 0)
        {
            bool subjectMatched = false;
            for (size_t i = 0; i < subjectCount; ++i)
            {
                NodeId subject = kUndefinedNodeId;
                ReturnErrorOnFailure(entry.GetSubject(i, subject));
                if (subject == subjectDescriptor.subjects[0])
                {
                    subjectMatched = true;
                    break;
                }
                // TODO: check against CATs in subject descriptor
            }
            if (!subjectMatched)
            {
                continue;
            }
        }

        size_t targetCount = 0;
        ReturnErrorOnFailure(entry.GetTargetCount(targetCount));
        if (targetCount > 0)
        {
            bool targetMatched = false;
            for (size_t i = 0; i < targetCount; ++i)
            {
                Entry::Target target;
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
                targetMatched = true;
                break;
            }
            if (!targetMatched)
            {
                continue;
            }
        }

        // Entry passed all checks: access is allowed.
        return CHIP_NO_ERROR;
    }

    // No entry was found which passed all checks: access is denied.
    return CHIP_ERROR_ACCESS_DENIED;
}

AccessControl & GetAccessControl()
{
    return *globalAccessControl;
}

void SetAccessControl(AccessControl & accessControl)
{
    globalAccessControl = &accessControl;
}

} // namespace Access
} // namespace chip
