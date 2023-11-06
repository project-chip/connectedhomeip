/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <app/server/AclStorage.h>

#include <lib/support/DefaultStorageKeyAllocator.h>

using namespace chip;
using namespace chip::app;
using namespace chip::Access;

using Entry            = AccessControl::Entry;
using EntryListener    = AccessControl::EntryListener;
using StagingAuthMode  = Clusters::AccessControl::AccessControlEntryAuthModeEnum;
using StagingPrivilege = Clusters::AccessControl::AccessControlEntryPrivilegeEnum;
using StagingTarget    = Clusters::AccessControl::Structs::AccessControlTargetStruct::Type;
using Target           = AccessControl::Entry::Target;

namespace {

struct StagingSubject
{
    NodeId nodeId;
    StagingAuthMode authMode;
};

CHIP_ERROR Convert(AuthMode from, StagingAuthMode & to)
{
    switch (from)
    {
    case AuthMode::kPase:
        to = StagingAuthMode::kPase;
        break;
    case AuthMode::kCase:
        to = StagingAuthMode::kCase;
        break;
    case AuthMode::kGroup:
        to = StagingAuthMode::kGroup;
        break;
    default:
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR Convert(StagingAuthMode from, AuthMode & to)
{
    switch (from)
    {
    case StagingAuthMode::kPase:
        to = AuthMode::kPase;
        break;
    case StagingAuthMode::kCase:
        to = AuthMode::kCase;
        break;
    case StagingAuthMode::kGroup:
        to = AuthMode::kGroup;
        break;
    default:
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR Convert(Privilege from, StagingPrivilege & to)
{
    switch (from)
    {
    case Privilege::kView:
        to = StagingPrivilege::kView;
        break;
    case Privilege::kProxyView:
        to = StagingPrivilege::kProxyView;
        break;
    case Privilege::kOperate:
        to = StagingPrivilege::kOperate;
        break;
    case Privilege::kManage:
        to = StagingPrivilege::kManage;
        break;
    case Privilege::kAdminister:
        to = StagingPrivilege::kAdminister;
        break;
    default:
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR Convert(StagingPrivilege from, Privilege & to)
{
    switch (from)
    {
    case StagingPrivilege::kView:
        to = Privilege::kView;
        break;
    case StagingPrivilege::kProxyView:
        to = Privilege::kProxyView;
        break;
    case StagingPrivilege::kOperate:
        to = Privilege::kOperate;
        break;
    case StagingPrivilege::kManage:
        to = Privilege::kManage;
        break;
    case StagingPrivilege::kAdminister:
        to = Privilege::kAdminister;
        break;
    default:
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR Convert(NodeId from, StagingSubject & to)
{
    if (IsOperationalNodeId(from) || IsCASEAuthTag(from))
    {
        to = { .nodeId = from, .authMode = StagingAuthMode::kCase };
    }
    else if (IsGroupId(from))
    {
        to = { .nodeId = GroupIdFromNodeId(from), .authMode = StagingAuthMode::kGroup };
    }
    else if (IsPAKEKeyId(from))
    {
        to = { .nodeId = PAKEKeyIdFromNodeId(from), .authMode = StagingAuthMode::kPase };
    }
    else
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR Convert(StagingSubject from, NodeId & to)
{
    switch (from.authMode)
    {
    case StagingAuthMode::kPase:
        ReturnErrorCodeIf((from.nodeId & ~kMaskPAKEKeyId) != 0, CHIP_ERROR_INVALID_ARGUMENT);
        to = NodeIdFromPAKEKeyId(static_cast<PasscodeId>(from.nodeId));
        break;
    case StagingAuthMode::kCase:
        to = from.nodeId;
        break;
    case StagingAuthMode::kGroup:
        ReturnErrorCodeIf((from.nodeId & ~kMaskGroupId) != 0, CHIP_ERROR_INVALID_ARGUMENT);
        to = NodeIdFromGroupId(static_cast<GroupId>(from.nodeId));
        break;
    default:
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR Convert(const Target & from, StagingTarget & to)
{
    if ((from.flags & Target::kCluster) != 0)
    {
        to.cluster.SetNonNull(from.cluster);
    }
    else
    {
        to.cluster.SetNull();
    }
    if ((from.flags & Target::kEndpoint) != 0)
    {
        to.endpoint.SetNonNull(from.endpoint);
    }
    else
    {
        to.endpoint.SetNull();
    }
    if ((from.flags & Target::kDeviceType) != 0)
    {
        to.deviceType.SetNonNull(from.deviceType);
    }
    else
    {
        to.deviceType.SetNull();
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR Convert(const StagingTarget & from, Target & to)
{
    to.flags = 0;
    if (!from.cluster.IsNull())
    {
        to.flags |= Target::kCluster;
        to.cluster = from.cluster.Value();
    }
    if (!from.endpoint.IsNull())
    {
        to.flags |= Target::kEndpoint;
        to.endpoint = from.endpoint.Value();
    }
    if (!from.deviceType.IsNull())
    {
        to.flags |= Target::kDeviceType;
        to.deviceType = from.deviceType.Value();
    }
    return CHIP_NO_ERROR;
}

} // namespace

namespace chip {
namespace app {

CHIP_ERROR AclStorage::DecodableEntry::Decode(TLV::TLVReader & reader)
{
    ReturnErrorOnFailure(mStagingEntry.Decode(reader));
    ReturnErrorOnFailure(Unstage());
    return CHIP_NO_ERROR;
}

CHIP_ERROR AclStorage::DecodableEntry::Unstage()
{
    ReturnErrorOnFailure(GetAccessControl().PrepareEntry(mEntry));

    ReturnErrorOnFailure(mEntry.SetFabricIndex(mStagingEntry.fabricIndex));

    {
        Privilege privilege;
        ReturnErrorOnFailure(Convert(mStagingEntry.privilege, privilege));
        ReturnErrorOnFailure(mEntry.SetPrivilege(privilege));
    }

    {
        AuthMode authMode;
        ReturnErrorOnFailure(Convert(mStagingEntry.authMode, authMode));
        ReturnErrorOnFailure(mEntry.SetAuthMode(authMode));
    }

    if (!mStagingEntry.subjects.IsNull())
    {
        auto iterator = mStagingEntry.subjects.Value().begin();
        while (iterator.Next())
        {
            StagingSubject tmp = { .nodeId = iterator.GetValue(), .authMode = mStagingEntry.authMode };
            NodeId subject;
            ReturnErrorOnFailure(Convert(tmp, subject));
            ReturnErrorOnFailure(mEntry.AddSubject(nullptr, subject));
        }
        ReturnErrorOnFailure(iterator.GetStatus());
    }

    if (!mStagingEntry.targets.IsNull())
    {
        auto iterator = mStagingEntry.targets.Value().begin();
        while (iterator.Next())
        {
            Target target;
            ReturnErrorOnFailure(Convert(iterator.GetValue(), target));
            ReturnErrorOnFailure(mEntry.AddTarget(nullptr, target));
        }
        ReturnErrorOnFailure(iterator.GetStatus());
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR AclStorage::EncodableEntry::EncodeForRead(TLV::TLVWriter & writer, TLV::Tag tag, FabricIndex fabric) const
{
    ReturnErrorOnFailure(Stage());
    ReturnErrorOnFailure(mStagingEntry.EncodeForRead(writer, tag, fabric));
    return CHIP_NO_ERROR;
}

CHIP_ERROR AclStorage::EncodableEntry::EncodeForWrite(TLV::TLVWriter & writer, TLV::Tag tag) const
{
    ReturnErrorOnFailure(Stage());
    ReturnErrorOnFailure(mStagingEntry.EncodeForWrite(writer, tag));
    return CHIP_NO_ERROR;
}

CHIP_ERROR AclStorage::EncodableEntry::Stage() const
{
    ReturnErrorOnFailure(mEntry.GetFabricIndex(mStagingEntry.fabricIndex));

    {
        Privilege privilege;
        ReturnErrorOnFailure(mEntry.GetPrivilege(privilege));
        ReturnErrorOnFailure(Convert(privilege, mStagingEntry.privilege));
    }

    {
        AuthMode authMode;
        ReturnErrorOnFailure(mEntry.GetAuthMode(authMode));
        ReturnErrorOnFailure(Convert(authMode, mStagingEntry.authMode));
    }

    {
        size_t count;
        ReturnErrorOnFailure(mEntry.GetSubjectCount(count));
        if (count > 0)
        {
            for (size_t i = 0; i < count; ++i)
            {
                NodeId subject;
                ReturnErrorOnFailure(mEntry.GetSubject(i, subject));
                StagingSubject tmp;
                ReturnErrorOnFailure(Convert(subject, tmp));
                mStagingSubjects[i] = tmp.nodeId;
            }
            mStagingEntry.subjects.SetNonNull(mStagingSubjects, count);
        }
        else
        {
            mStagingEntry.subjects.SetNull();
        }
    }

    {
        size_t count;
        ReturnErrorOnFailure(mEntry.GetTargetCount(count));
        if (count > 0)
        {
            for (size_t i = 0; i < count; ++i)
            {
                Target target;
                ReturnErrorOnFailure(mEntry.GetTarget(i, target));
                ReturnErrorOnFailure(Convert(target, mStagingTargets[i]));
            }
            mStagingEntry.targets.SetNonNull(mStagingTargets, count);
        }
        else
        {
            mStagingEntry.targets.SetNull();
        }
    }

    return CHIP_NO_ERROR;
}

} // namespace app
} // namespace chip
