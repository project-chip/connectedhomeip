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

#include <app/server/DefaultAclStorage.h>

#include <lib/support/DefaultStorageKeyAllocator.h>

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
        ChipLogError(DataManagement, "DefaultAclStorage: failed %" CHIP_ERROR_FORMAT, err.Format());
    }

    // Must initialize before use.
    void Init(PersistentStorageDelegate & persistentStorage) { mPersistentStorage = &persistentStorage; }

private:
    PersistentStorageDelegate * mPersistentStorage = nullptr;

} sEntryListener;

} // namespace

namespace chip {
namespace app {

CHIP_ERROR DefaultAclStorage::Init(PersistentStorageDelegate & persistentStorage, ConstFabricIterator first,
                                   ConstFabricIterator last)
{
    ChipLogProgress(DataManagement, "DefaultAclStorage: initializing");

    CHIP_ERROR err;

    DefaultStorageKeyAllocator key;

    size_t count = 0;

    for (auto it = first; it != last; ++it)
    {
        auto fabric = it->GetFabricIndex();
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

    ChipLogProgress(DataManagement, "DefaultAclStorage: %u entries loaded", (unsigned) count);

    sEntryListener.Init(persistentStorage);
    GetAccessControl().AddEntryListener(sEntryListener);

    return CHIP_NO_ERROR;

exit:
    ChipLogError(DataManagement, "DefaultAclStorage: failed %" CHIP_ERROR_FORMAT, err.Format());
    return err;
}

} // namespace app
} // namespace chip
