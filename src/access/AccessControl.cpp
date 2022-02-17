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

using chip::CATValues;
using chip::FabricIndex;
using chip::NodeId;
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

constexpr bool IsValidCaseNodeId(NodeId aNodeId)
{
    return chip::IsOperationalNodeId(aNodeId) || (chip::IsCASEAuthTag(aNodeId) && ((aNodeId & chip::kTagVersionMask) != 0));
}

constexpr bool IsValidGroupNodeId(NodeId aNodeId)
{
    return chip::IsGroupId(aNodeId) && chip::IsValidGroupId(chip::GroupIdFromNodeId(aNodeId));
}

#if CHIP_PROGRESS_LOGGING

char GetAuthModeStringForLogging(AuthMode authMode)
{
    switch (authMode)
    {
    case AuthMode::kNone:
        return 'n';
    case AuthMode::kPase:
        return 'p';
    case AuthMode::kCase:
        return 'c';
    case AuthMode::kGroup:
        return 'g';
    }
    return 'u';
}

constexpr int kCharsPerCatForLogging = 11; // including final null terminator

char * GetCatStringForLogging(char * buf, size_t size, const CATValues & cats)
{
    if (size == 0)
    {
        return nullptr;
    }
    char * p         = buf;
    char * const end = buf + size;
    *p               = '\0';
    // Format string chars needed:
    //   1 for comma (optional)
    //   2 for 0x prefix
    //   8 for 32-bit hex value
    //   1 for null terminator (at end)
    constexpr char fmtWithoutComma[] = "0x%08" PRIX32;
    constexpr char fmtWithComma[]    = ",0x%08" PRIX32;
    constexpr int countWithoutComma  = 10;
    constexpr int countWithComma     = countWithoutComma + 1;
    bool withComma                   = false;
    for (auto cat : cats.values)
    {
        if (cat == chip::kUndefinedCAT)
        {
            break;
        }
        snprintf(p, static_cast<size_t>(end - p), withComma ? fmtWithComma : fmtWithoutComma, cat);
        p += withComma ? countWithComma : countWithoutComma;
        if (p >= end)
        {
            // Output was truncated.
            p = end - ((size < 4) ? size : 4);
            while (*p)
            {
                // Indicate truncation if possible.
                *p++ = '.';
            }
            break;
        }
        withComma = true;
    }
    return buf;
}

char GetPrivilegeStringForLogging(Privilege privilege)
{
    switch (privilege)
    {
    case Privilege::kView:
        return 'v';
    case Privilege::kProxyView:
        return 'p';
    case Privilege::kOperate:
        return 'o';
    case Privilege::kManage:
        return 'm';
    case Privilege::kAdminister:
        return 'a';
    }
    return 'u';
}

#endif // CHIP_PROGRESS_LOGGING

} // namespace

namespace chip {
namespace Access {

AccessControl::Entry::Delegate AccessControl::Entry::mDefaultDelegate;
AccessControl::EntryIterator::Delegate AccessControl::EntryIterator::mDefaultDelegate;
AccessControl::Delegate AccessControl::mDefaultDelegate;

CHIP_ERROR AccessControl::Init()
{
    ChipLogProgress(DataManagement, "AccessControl: initializing");
    return mDelegate.Init();
}

CHIP_ERROR AccessControl::Finish()
{
    ChipLogProgress(DataManagement, "AccessControl: finishing");
    return mDelegate.Finish();
}

CHIP_ERROR AccessControl::Check(const SubjectDescriptor & subjectDescriptor, const RequestPath & requestPath,
                                Privilege requestPrivilege)
{
#if CHIP_PROGRESS_LOGGING
    {
        char buf[6 * kCharsPerCatForLogging];
        ChipLogProgress(DataManagement,
                        "AccessControl: checking f=%u a=%c s=0x" ChipLogFormatX64 " t=%s c=" ChipLogFormatMEI " e=%" PRIu16 " p=%c",
                        subjectDescriptor.fabricIndex, GetAuthModeStringForLogging(subjectDescriptor.authMode),
                        ChipLogValueX64(subjectDescriptor.subject),
                        GetCatStringForLogging(buf, sizeof(buf), subjectDescriptor.cats), ChipLogValueMEI(requestPath.cluster),
                        requestPath.endpoint, GetPrivilegeStringForLogging(requestPrivilege));
    }
#endif

    // TODO(#13867): this will go away
    if (mDelegate.TemporaryCheckOverride())
    {
        ChipLogProgress(DataManagement, "AccessControl: temporary check override (this will go away)");
        return CHIP_NO_ERROR;
    }

    // Operational PASE not supported for v1.0, so PASE implies commissioning, which has highest privilege.
    if (subjectDescriptor.authMode == AuthMode::kPase)
    {
        ChipLogProgress(DataManagement, "AccessControl: implicit admin (PASE)");
        return CHIP_NO_ERROR;
    }

    EntryIterator iterator;
    ReturnErrorOnFailure(Entries(iterator, &subjectDescriptor.fabricIndex));

    Entry entry;
    while (iterator.Next(entry) == CHIP_NO_ERROR)
    {
        AuthMode authMode = AuthMode::kNone;
        ReturnErrorOnFailure(entry.GetAuthMode(authMode));
        // Operational PASE not supported for v1.0.
        VerifyOrReturnError(authMode == AuthMode::kCase || authMode == AuthMode::kGroup, CHIP_ERROR_INCORRECT_STATE);
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
                if (IsOperationalNodeId(subject))
                {
                    VerifyOrReturnError(authMode == AuthMode::kCase, CHIP_ERROR_INCORRECT_STATE);
                    if (subject == subjectDescriptor.subject)
                    {
                        subjectMatched = true;
                        break;
                    }
                }
                else if (IsCASEAuthTag(subject))
                {
                    VerifyOrReturnError(authMode == AuthMode::kCase, CHIP_ERROR_INCORRECT_STATE);
                    if (subjectDescriptor.cats.CheckSubjectAgainstCATs(subject))
                    {
                        subjectMatched = true;
                        break;
                    }
                }
                else if (IsGroupId(subject))
                {
                    VerifyOrReturnError(authMode == AuthMode::kGroup, CHIP_ERROR_INCORRECT_STATE);
                    if (subject == subjectDescriptor.subject)
                    {
                        subjectMatched = true;
                        break;
                    }
                }
                else
                {
                    // Operational PASE not supported for v1.0.
                    return CHIP_ERROR_INCORRECT_STATE;
                }
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
                // TODO(#14431): device type target not yet supported (add lookup/match when supported)
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
    ChipLogProgress(DataManagement, "AccessControl: denied");
    return CHIP_ERROR_ACCESS_DENIED;
}

bool AccessControl::IsValid(const Entry & entry)
{
    const char * log = "unexpected error";
    IgnoreUnusedVariable(log); // logging may be disabled

    AuthMode authMode;
    FabricIndex fabricIndex;
    Privilege privilege;
    size_t subjectCount = 0;
    size_t targetCount  = 0;

    SuccessOrExit(entry.GetAuthMode(authMode));
    SuccessOrExit(entry.GetFabricIndex(fabricIndex));
    SuccessOrExit(entry.GetPrivilege(privilege));
    SuccessOrExit(entry.GetSubjectCount(subjectCount));
    SuccessOrExit(entry.GetTargetCount(targetCount));

    ChipLogProgress(DataManagement, "AccessControl: validating f=%u p=%c a=%c s=%d t=%d", fabricIndex,
                    GetPrivilegeStringForLogging(privilege), GetAuthModeStringForLogging(authMode), static_cast<int>(subjectCount),
                    static_cast<int>(targetCount));

    // Fabric index must be defined.
    VerifyOrExit(fabricIndex != kUndefinedFabricIndex, log = "invalid fabric index");

    if (authMode != AuthMode::kCase)
    {
        // Operational PASE not supported for v1.0 (so must be group).
        VerifyOrExit(authMode == AuthMode::kGroup, log = "invalid auth mode");

        // Privilege must not be administer.
        VerifyOrExit(privilege != Privilege::kAdminister, log = "invalid privilege");

        // Subject must be present.
        VerifyOrExit(subjectCount > 0, log = "invalid subject count");
    }

    for (size_t i = 0; i < subjectCount; ++i)
    {
        NodeId subject;
        SuccessOrExit(entry.GetSubject(i, subject));
        const bool kIsCase  = authMode == AuthMode::kCase;
        const bool kIsGroup = authMode == AuthMode::kGroup;
        ChipLogProgress(DataManagement, "  validating subject 0x" ChipLogFormatX64, ChipLogValueX64(subject));
        VerifyOrExit((kIsCase && IsValidCaseNodeId(subject)) || (kIsGroup && IsValidGroupNodeId(subject)), log = "invalid subject");
    }

    for (size_t i = 0; i < targetCount; ++i)
    {
        Entry::Target target;
        SuccessOrExit(entry.GetTarget(i, target));
        const bool kHasCluster    = target.flags & Entry::Target::kCluster;
        const bool kHasEndpoint   = target.flags & Entry::Target::kEndpoint;
        const bool kHasDeviceType = target.flags & Entry::Target::kDeviceType;
        VerifyOrExit((kHasCluster || kHasEndpoint || kHasDeviceType) && !(kHasEndpoint && kHasDeviceType) &&
                         (!kHasCluster || IsValidClusterId(target.cluster)) &&
                         (!kHasEndpoint || IsValidEndpointId(target.endpoint)) &&
                         (!kHasDeviceType || IsValidDeviceTypeId(target.deviceType)),
                     log = "invalid target");
        // TODO(#14431): device type target not yet supported (remove check when supported)
        VerifyOrExit(!kHasDeviceType, log = "device type target not yet supported");
    }

    return true;

exit:
    ChipLogError(DataManagement, "AccessControl: %s", log);
    return false;
}

AccessControl & GetAccessControl()
{
    return *globalAccessControl;
}

void SetAccessControl(AccessControl & accessControl)
{
    ChipLogProgress(DataManagement, "AccessControl: setting");
    globalAccessControl = &accessControl;
}

} // namespace Access
} // namespace chip
