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

// Included for the default AccessControlDelegate logging enables/disables.
// See `chip_access_control_policy_logging_verbosity` in `src/app/BUILD.gn` for
// the levels available.
#include <app/AppConfig.h>

#include "AccessControl.h"

#include <lib/core/Global.h>

namespace chip {
namespace Access {

using chip::CATValues;
using chip::FabricIndex;
using chip::NodeId;

namespace {

Global<AccessControl> defaultAccessControl;
AccessControl * globalAccessControl = nullptr; // lazily defaulted to defaultAccessControl in GetAccessControl

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
    if (IsOperationalNodeId(aNodeId))
    {
        return true;
    }

    if (IsCASEAuthTag(aNodeId) && (GetCASEAuthTagVersion(CASEAuthTagFromNodeId(aNodeId)) != 0))
    {
        return true;
    }

    return false;
}

constexpr bool IsValidGroupNodeId(NodeId aNodeId)
{
    return IsGroupId(aNodeId) && IsValidGroupId(GroupIdFromNodeId(aNodeId));
}

#if CHIP_PROGRESS_LOGGING && CHIP_CONFIG_ACCESS_CONTROL_POLICY_LOGGING_VERBOSITY > 1

char GetAuthModeStringForLogging(AuthMode authMode)
{
    switch (authMode)
    {
    case AuthMode::kNone:
        return 'n';
    case AuthMode::kInternalDeviceAccess:
        return 'i';
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
    static constexpr char fmtWithoutComma[] = "0x%08" PRIX32;
    static constexpr char fmtWithComma[]    = ",0x%08" PRIX32;
    constexpr int countWithoutComma         = 10;
    constexpr int countWithComma            = countWithoutComma + 1;
    bool withComma                          = false;
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

char GetRequestTypeStringForLogging(RequestType requestType)
{
    switch (requestType)
    {
    case RequestType::kAttributeReadRequest:
        return 'r';
    case RequestType::kAttributeWriteRequest:
        return 'w';
    case RequestType::kCommandInvokeRequest:
        return 'i';
    case RequestType::kEventReadRequest:
        return 'e';
    default:
        return '?';
    }
}

#endif // CHIP_PROGRESS_LOGGING && CHIP_CONFIG_ACCESS_CONTROL_POLICY_LOGGING_VERBOSITY > 1

} // namespace

Global<AccessControl::Entry::Delegate> AccessControl::Entry::mDefaultDelegate;
Global<AccessControl::EntryIterator::Delegate> AccessControl::EntryIterator::mDefaultDelegate;

CHIP_ERROR AccessControl::Init(AccessControl::Delegate * delegate, DeviceTypeResolver & deviceTypeResolver)
{
    VerifyOrReturnError(!IsInitialized(), CHIP_ERROR_INCORRECT_STATE);

    ChipLogProgress(DataManagement, "AccessControl: initializing");

    VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    CHIP_ERROR retval = delegate->Init();
    if (retval == CHIP_NO_ERROR)
    {
        mDelegate           = delegate;
        mDeviceTypeResolver = &deviceTypeResolver;
    }

    return retval;
}

void AccessControl::Finish()
{
    VerifyOrReturn(IsInitialized());
    ChipLogProgress(DataManagement, "AccessControl: finishing");
    mDelegate->Finish();
    mDelegate = nullptr;
}

CHIP_ERROR AccessControl::CreateEntry(const SubjectDescriptor * subjectDescriptor, FabricIndex fabric, size_t * index,
                                      const Entry & entry)
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);

    size_t count    = 0;
    size_t maxCount = 0;
    ReturnErrorOnFailure(mDelegate->GetEntryCount(fabric, count));
    ReturnErrorOnFailure(mDelegate->GetMaxEntriesPerFabric(maxCount));

    VerifyOrReturnError((count + 1) <= maxCount, CHIP_ERROR_BUFFER_TOO_SMALL);

    VerifyOrReturnError(IsValid(entry), CHIP_ERROR_INVALID_ARGUMENT);

    size_t i = 0;
    ReturnErrorOnFailure(mDelegate->CreateEntry(&i, entry, &fabric));

    if (index)
    {
        *index = i;
    }

    NotifyEntryChanged(subjectDescriptor, fabric, i, &entry, EntryListener::ChangeType::kAdded);
    return CHIP_NO_ERROR;
}

CHIP_ERROR AccessControl::UpdateEntry(const SubjectDescriptor * subjectDescriptor, FabricIndex fabric, size_t index,
                                      const Entry & entry)
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(IsValid(entry), CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorOnFailure(mDelegate->UpdateEntry(index, entry, &fabric));
    NotifyEntryChanged(subjectDescriptor, fabric, index, &entry, EntryListener::ChangeType::kUpdated);
    return CHIP_NO_ERROR;
}

CHIP_ERROR AccessControl::DeleteEntry(const SubjectDescriptor * subjectDescriptor, FabricIndex fabric, size_t index)
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);
    Entry entry;
    Entry * p = nullptr;
    if (mEntryListener != nullptr && ReadEntry(fabric, index, entry) == CHIP_NO_ERROR)
    {
        p = &entry;
    }
    ReturnErrorOnFailure(mDelegate->DeleteEntry(index, &fabric));
    if (p && p->HasDefaultDelegate())
    {
        // The entry was read prior to deletion so its latest value could be provided
        // to the listener after deletion. If it's been reset to its default delegate,
        // that best effort attempt to retain the latest value failed. This is
        // regrettable but OK.
        p = nullptr;
    }
    NotifyEntryChanged(subjectDescriptor, fabric, index, p, EntryListener::ChangeType::kRemoved);
    return CHIP_NO_ERROR;
}

void AccessControl::AddEntryListener(EntryListener & listener)
{
    if (mEntryListener == nullptr)
    {
        mEntryListener = &listener;
        listener.mNext = nullptr;
        return;
    }

    for (EntryListener * l = mEntryListener; /**/; l = l->mNext)
    {
        if (l == &listener)
        {
            return;
        }

        if (l->mNext == nullptr)
        {
            l->mNext       = &listener;
            listener.mNext = nullptr;
            return;
        }
    }
}

void AccessControl::RemoveEntryListener(EntryListener & listener)
{
    if (mEntryListener == &listener)
    {
        mEntryListener = listener.mNext;
        listener.mNext = nullptr;
        return;
    }

    for (EntryListener * l = mEntryListener; l != nullptr; l = l->mNext)
    {
        if (l->mNext == &listener)
        {
            l->mNext       = listener.mNext;
            listener.mNext = nullptr;
            return;
        }
    }
}

bool AccessControl::IsAccessRestrictionListSupported() const
{
#if CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
    return mAccessRestrictionProvider != nullptr;
#else
    return false;
#endif
}

CHIP_ERROR AccessControl::Check(const SubjectDescriptor & subjectDescriptor, const RequestPath & requestPath,
                                Privilege requestPrivilege)
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);

    CHIP_ERROR result = CheckACL(subjectDescriptor, requestPath, requestPrivilege);

#if CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
    if (result == CHIP_NO_ERROR)
    {
        result = CheckARL(subjectDescriptor, requestPath, requestPrivilege);
    }
#endif

    return result;
}

CHIP_ERROR AccessControl::CheckACL(const SubjectDescriptor & subjectDescriptor, const RequestPath & requestPath,
                                   Privilege requestPrivilege)
{
#if CHIP_PROGRESS_LOGGING && CHIP_CONFIG_ACCESS_CONTROL_POLICY_LOGGING_VERBOSITY > 1
    {
        constexpr size_t kMaxCatsToLog = 6;
        char catLogBuf[kMaxCatsToLog * kCharsPerCatForLogging];
        ChipLogProgress(DataManagement,
                        "AccessControl: checking f=%u a=%c s=0x" ChipLogFormatX64 " t=%s c=" ChipLogFormatMEI " e=%u p=%c r=%c",
                        subjectDescriptor.fabricIndex, GetAuthModeStringForLogging(subjectDescriptor.authMode),
                        ChipLogValueX64(subjectDescriptor.subject),
                        GetCatStringForLogging(catLogBuf, sizeof(catLogBuf), subjectDescriptor.cats),
                        ChipLogValueMEI(requestPath.cluster), requestPath.endpoint, GetPrivilegeStringForLogging(requestPrivilege),
                        GetRequestTypeStringForLogging(requestPath.requestType));
    }
#endif // CHIP_PROGRESS_LOGGING && CHIP_CONFIG_ACCESS_CONTROL_POLICY_LOGGING_VERBOSITY > 1

    {
        CHIP_ERROR result = mDelegate->Check(subjectDescriptor, requestPath, requestPrivilege);
        if (result != CHIP_ERROR_NOT_IMPLEMENTED)
        {
#if CHIP_CONFIG_ACCESS_CONTROL_POLICY_LOGGING_VERBOSITY > 0
            ChipLogProgress(DataManagement, "AccessControl: %s (delegate)",
                            (result == CHIP_NO_ERROR)                  ? "allowed"
                                : (result == CHIP_ERROR_ACCESS_DENIED) ? "denied"
                                                                       : "error");
#else
            if (result != CHIP_NO_ERROR)
            {
                ChipLogProgress(DataManagement, "AccessControl: %s (delegate)",
                                (result == CHIP_ERROR_ACCESS_DENIED) ? "denied" : "error");
            }
#endif // CHIP_CONFIG_ACCESS_CONTROL_POLICY_LOGGING_VERBOSITY > 0

            return result;
        }
    }

    // Operational PASE not supported for v1.0, so PASE implies commissioning, which has highest privilege.
    // Currently, subject descriptor is only PASE if this node is the responder (aka commissionee);
    // if this node is the initiator (aka commissioner) then the subject descriptor remains blank.
    if (subjectDescriptor.authMode == AuthMode::kPase)
    {
#if CHIP_CONFIG_ACCESS_CONTROL_POLICY_LOGGING_VERBOSITY > 1
        ChipLogProgress(DataManagement, "AccessControl: implicit admin (PASE)");
#endif // CHIP_CONFIG_ACCESS_CONTROL_POLICY_LOGGING_VERBOSITY > 1
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
                if (target.flags & Entry::Target::kDeviceType &&
                    !mDeviceTypeResolver->IsDeviceTypeOnEndpoint(target.deviceType, requestPath.endpoint))
                {
                    continue;
                }
                targetMatched = true;
                break;
            }
            if (!targetMatched)
            {
                continue;
            }
        }
        // Entry passed all checks: access is allowed.

#if CHIP_CONFIG_ACCESS_CONTROL_POLICY_LOGGING_VERBOSITY > 0
        ChipLogProgress(DataManagement, "AccessControl: allowed");
#endif // CHIP_CONFIG_ACCESS_CONTROL_POLICY_LOGGING_VERBOSITY > 0

        return CHIP_NO_ERROR;
    }

    // No entry was found which passed all checks: access is denied.
    ChipLogProgress(DataManagement, "AccessControl: denied");
    return CHIP_ERROR_ACCESS_DENIED;
}

#if CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
CHIP_ERROR AccessControl::CheckARL(const SubjectDescriptor & subjectDescriptor, const RequestPath & requestPath,
                                   Privilege requestPrivilege)
{
    CHIP_ERROR result = CHIP_NO_ERROR;

    VerifyOrReturnError(requestPath.requestType != RequestType::kRequestTypeUnknown, CHIP_ERROR_INVALID_ARGUMENT);

    if (!IsAccessRestrictionListSupported())
    {
        // Access Restriction support is compiled in, but not configured/enabled. Nothing to restrict.
        return CHIP_NO_ERROR;
    }

    if (subjectDescriptor.isCommissioning)
    {
        result = mAccessRestrictionProvider->CheckForCommissioning(subjectDescriptor, requestPath);
    }
    else
    {
        result = mAccessRestrictionProvider->Check(subjectDescriptor, requestPath);
    }

    if (result != CHIP_NO_ERROR)
    {
        ChipLogProgress(DataManagement, "AccessControl: %s",
                        (result == CHIP_ERROR_ACCESS_RESTRICTED_BY_ARL) ? "denied (restricted)" : "denied (restriction error)");
        return result;
    }

    return result;
}
#endif

#if CHIP_ACCESS_CONTROL_DUMP_ENABLED
CHIP_ERROR AccessControl::Dump(const Entry & entry)
{
    CHIP_ERROR err;

    ChipLogDetail(DataManagement, "----- BEGIN ENTRY -----");

    {
        FabricIndex fabricIndex;
        SuccessOrExit(err = entry.GetFabricIndex(fabricIndex));
        ChipLogDetail(DataManagement, "fabricIndex: %u", fabricIndex);
    }

    {
        Privilege privilege;
        SuccessOrExit(err = entry.GetPrivilege(privilege));
        ChipLogDetail(DataManagement, "privilege: %d", to_underlying(privilege));
    }

    {
        AuthMode authMode;
        SuccessOrExit(err = entry.GetAuthMode(authMode));
        ChipLogDetail(DataManagement, "authMode: %d", to_underlying(authMode));
    }

    {
        size_t count;
        SuccessOrExit(err = entry.GetSubjectCount(count));
        if (count)
        {
            ChipLogDetail(DataManagement, "subjects: %u", static_cast<unsigned>(count));
            for (size_t i = 0; i < count; ++i)
            {
                NodeId subject;
                SuccessOrExit(err = entry.GetSubject(i, subject));
                ChipLogDetail(DataManagement, "  %u: 0x" ChipLogFormatX64, static_cast<unsigned>(i), ChipLogValueX64(subject));
            }
        }
    }

    {
        size_t count;
        SuccessOrExit(err = entry.GetTargetCount(count));
        if (count)
        {
            ChipLogDetail(DataManagement, "targets: %u", static_cast<unsigned>(count));
            for (size_t i = 0; i < count; ++i)
            {
                Entry::Target target;
                SuccessOrExit(err = entry.GetTarget(i, target));
                if (target.flags & Entry::Target::kCluster)
                {
                    ChipLogDetail(DataManagement, "  %u: cluster: 0x" ChipLogFormatMEI, static_cast<unsigned>(i),
                                  ChipLogValueMEI(target.cluster));
                }
                if (target.flags & Entry::Target::kEndpoint)
                {
                    ChipLogDetail(DataManagement, "  %u: endpoint: %u", static_cast<unsigned>(i), target.endpoint);
                }
                if (target.flags & Entry::Target::kDeviceType)
                {
                    ChipLogDetail(DataManagement, "  %u: deviceType: 0x" ChipLogFormatMEI, static_cast<unsigned>(i),
                                  ChipLogValueMEI(target.deviceType));
                }
            }
        }
    }

    ChipLogDetail(DataManagement, "----- END ENTRY -----");

    return CHIP_NO_ERROR;

exit:
    ChipLogError(DataManagement, "AccessControl: dump failed %" CHIP_ERROR_FORMAT, err.Format());
    return err;
}
#endif

bool AccessControl::IsValid(const Entry & entry)
{
    const char * log = "unexpected error";
    IgnoreUnusedVariable(log); // logging may be disabled

    AuthMode authMode       = AuthMode::kNone;
    FabricIndex fabricIndex = kUndefinedFabricIndex;
    Privilege privilege     = static_cast<Privilege>(0);
    size_t subjectCount     = 0;
    size_t targetCount      = 0;

    CHIP_ERROR err = CHIP_NO_ERROR;
    SuccessOrExit(err = entry.GetAuthMode(authMode));
    SuccessOrExit(err = entry.GetFabricIndex(fabricIndex));
    SuccessOrExit(err = entry.GetPrivilege(privilege));
    SuccessOrExit(err = entry.GetSubjectCount(subjectCount));
    SuccessOrExit(err = entry.GetTargetCount(targetCount));

#if CHIP_CONFIG_ACCESS_CONTROL_POLICY_LOGGING_VERBOSITY > 1
    ChipLogProgress(DataManagement, "AccessControl: validating f=%u p=%c a=%c s=%d t=%d", fabricIndex,
                    GetPrivilegeStringForLogging(privilege), GetAuthModeStringForLogging(authMode), static_cast<int>(subjectCount),
                    static_cast<int>(targetCount));
#endif // CHIP_CONFIG_ACCESS_CONTROL_POLICY_LOGGING_VERBOSITY > 1

    // Fabric index must be defined.
    VerifyOrExit(fabricIndex != kUndefinedFabricIndex, log = "invalid fabric index");

    if (authMode != AuthMode::kCase)
    {
        // Operational PASE not supported for v1.0 (so must be group).
        VerifyOrExit(authMode == AuthMode::kGroup, log = "invalid auth mode");

        // Privilege must not be administer.
        VerifyOrExit(privilege != Privilege::kAdminister, log = "invalid privilege");
    }

    for (size_t i = 0; i < subjectCount; ++i)
    {
        NodeId subject;
        SuccessOrExit(err = entry.GetSubject(i, subject));
        const bool kIsCase  = authMode == AuthMode::kCase;
        const bool kIsGroup = authMode == AuthMode::kGroup;
#if CHIP_CONFIG_ACCESS_CONTROL_POLICY_LOGGING_VERBOSITY > 1
        ChipLogProgress(DataManagement, "  validating subject 0x" ChipLogFormatX64, ChipLogValueX64(subject));
#endif // CHIP_CONFIG_ACCESS_CONTROL_POLICY_LOGGING_VERBOSITY > 1
        VerifyOrExit((kIsCase && IsValidCaseNodeId(subject)) || (kIsGroup && IsValidGroupNodeId(subject)), log = "invalid subject");
    }

    for (size_t i = 0; i < targetCount; ++i)
    {
        Entry::Target target;
        SuccessOrExit(err = entry.GetTarget(i, target));
        const bool kHasCluster    = target.flags & Entry::Target::kCluster;
        const bool kHasEndpoint   = target.flags & Entry::Target::kEndpoint;
        const bool kHasDeviceType = target.flags & Entry::Target::kDeviceType;
        VerifyOrExit((kHasCluster || kHasEndpoint || kHasDeviceType) && !(kHasEndpoint && kHasDeviceType) &&
                         (!kHasCluster || IsValidClusterId(target.cluster)) &&
                         (!kHasEndpoint || IsValidEndpointId(target.endpoint)) &&
                         (!kHasDeviceType || IsValidDeviceTypeId(target.deviceType)),
                     log = "invalid target");
    }

    return true;

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DataManagement, "AccessControl: %s %" CHIP_ERROR_FORMAT, log, err.Format());
    }
    else
    {
        ChipLogError(DataManagement, "AccessControl: %s", log);
    }
    return false;
}

void AccessControl::NotifyEntryChanged(const SubjectDescriptor * subjectDescriptor, FabricIndex fabric, size_t index,
                                       const Entry * entry, EntryListener::ChangeType changeType)
{
    for (EntryListener * listener = mEntryListener; listener != nullptr; listener = listener->mNext)
    {
        listener->OnEntryChanged(subjectDescriptor, fabric, index, entry, changeType);
    }
}

AccessControl & GetAccessControl()
{
    return (globalAccessControl) ? *globalAccessControl : defaultAccessControl.get();
}

void SetAccessControl(AccessControl & accessControl)
{
    ChipLogProgress(DataManagement, "AccessControl: setting");
    globalAccessControl = &accessControl;
}

void ResetAccessControlToDefault()
{
    globalAccessControl = nullptr;
}

} // namespace Access
} // namespace chip
