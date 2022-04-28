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

#include <app/server/Server.h>

using namespace chip;
using namespace chip::app;
using namespace chip::Access;

using EncodableEntry   = AclStorage::EncodableEntry;
using Entry            = AccessControl::Entry;
using EntryListener    = AccessControl::EntryListener;
using StagingAuthMode  = Clusters::AccessControl::AuthMode;
using StagingPrivilege = Clusters::AccessControl::Privilege;
using StagingTarget    = Clusters::AccessControl::Structs::Target::Type;
using Target           = AccessControl::Entry::Target;

namespace {

/*
Size calculation for TLV encoded entry.

Because EncodeForWrite is used without an accessing fabric, the fabric index is
not encoded. However, let's assume it is. This yields 17 bytes total overhead,
but it's wise to add a few more for safety.

Each subject may require up to 9 bytes. Each target may require up to 14 bytes
(only one of endpoint or device type will be encoded).

DATA                                    C   T   L   V   NOTES
structure (anonymous)                   1               0x15
  field 1 privilege                     1   1       1
  field 2 authmode                      1   1       1
  field 3 subjects                      1   1
    uint64                              1           8   per subject
  end list                              1               0x18
  field 4 targets                       1   1
    structure (anonymous)               1               per target
      field 0 cluster                   1   1       4
      field 1 endpoint                  1   1       2   only field 1 or 2
      field 2 devicetype                1   1       4   only field 1 or 2
    end structure                       1
  end list                              1               0x18
  field 254 fabric index                1   1       1   not written
end structure                           1               0x18
*/

// TODO(#14455): get actual values for max subjects/targets
constexpr int kEncodedEntryOverheadBytes = 17 + 8;
constexpr int kEncodedEntrySubjectBytes  = 9 * CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_MAX_SUBJECTS_PER_ENTRY;
constexpr int kEncodedEntryTargetBytes   = 14 * CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_MAX_TARGETS_PER_ENTRY;
constexpr int kEncodedEntryTotalBytes    = kEncodedEntryOverheadBytes + kEncodedEntrySubjectBytes + kEncodedEntryTargetBytes;

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

class : public EntryListener
{
public:
    void OnEntryChanged(const SubjectDescriptor * subjectDescriptor, FabricIndex fabric, size_t index, const Entry * entry,
                        ChangeType changeType) override
    {
        CHIP_ERROR err;

        DefaultStorageKeyAllocator key;

        uint8_t buffer[kEncodedEntryTotalBytes] = { 0 };

        VerifyOrExit(mPersistentStorage != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

        if (changeType == ChangeType::kRemoved)
        {
            // Shuffle down entries past index, then delete entry at last index.
            while (true)
            {
                uint16_t size = static_cast<uint16_t>(sizeof(buffer));
                err           = mPersistentStorage->SyncGetKeyValue(key.AccessControlAclEntry(fabric, index + 1), buffer, size);
                if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
                {
                    break;
                }
                SuccessOrExit(err);
                SuccessOrExit(err = mPersistentStorage->SyncSetKeyValue(key.AccessControlAclEntry(fabric, index), buffer, size));
                index++;
            }
            SuccessOrExit(err = mPersistentStorage->SyncDeleteKeyValue(key.AccessControlAclEntry(fabric, index)));
        }
        else
        {
            // Write added/updated entry at index.
            VerifyOrExit(entry != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
            TLV::TLVWriter writer;
            writer.Init(buffer);
            EncodableEntry encodableEntry(*entry);
            SuccessOrExit(err = encodableEntry.EncodeForWrite(writer, TLV::AnonymousTag()));
            SuccessOrExit(err = mPersistentStorage->SyncSetKeyValue(key.AccessControlAclEntry(fabric, index), buffer,
                                                                    static_cast<uint16_t>(writer.GetLengthWritten())));
        }

        return;

    exit:
        ChipLogError(DataManagement, "AclStorage: failed %" CHIP_ERROR_FORMAT, err.Format());
    }

    // Must initialize before use.
    void Init(PersistentStorageDelegate & persistentStorage) { mPersistentStorage = &persistentStorage; }

private:
    PersistentStorageDelegate * mPersistentStorage = nullptr;

} sEntryListener;

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

CHIP_ERROR AclStorage::Init(PersistentStorageDelegate & persistentStorage, const FabricTable & fabricTable)
{
    ChipLogProgress(DataManagement, "AclStorage: initializing");

    CHIP_ERROR err;

    DefaultStorageKeyAllocator key;

    size_t count = 0;

    for (auto & info : fabricTable)
    {
        auto fabric = info.GetFabricIndex();
        for (size_t index = 0; /**/; ++index)
        {
            uint8_t buffer[kEncodedEntryTotalBytes] = { 0 };
            uint16_t size                           = static_cast<uint16_t>(sizeof(buffer));
            err = persistentStorage.SyncGetKeyValue(key.AccessControlAclEntry(fabric, index), buffer, size);
            if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
            {
                break;
            }
            SuccessOrExit(err);

            TLV::TLVReader reader;
            reader.Init(buffer, size);
            SuccessOrExit(err = reader.Next());

            DecodableEntry decodableEntry;
            SuccessOrExit(err = decodableEntry.Decode(reader));

            Entry & entry = decodableEntry.GetEntry();
            SuccessOrExit(err = entry.SetFabricIndex(fabric));

            SuccessOrExit(err = GetAccessControl().CreateEntry(nullptr, fabric, nullptr, entry));
            count++;
        }
    }

    ChipLogProgress(DataManagement, "AclStorage: %u entries loaded", (unsigned) count);

    sEntryListener.Init(persistentStorage);
    GetAccessControl().AddEntryListener(sEntryListener);

    return CHIP_NO_ERROR;

exit:
    ChipLogError(DataManagement, "AclStorage: failed %" CHIP_ERROR_FORMAT, err.Format());
    return err;
}

} // namespace app
} // namespace chip
