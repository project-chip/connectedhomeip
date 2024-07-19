/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app/server/DefaultArlStorage.h>

#include <access/AccessControl.h>
#include <lib/support/DefaultStorageKeyAllocator.h>

using namespace chip;
using namespace chip::app;
using namespace chip::Access;

using EncodableEntry     = ArlStorage::EncodableEntry;
using Entry              = AccessRestriction::Entry;
using EntryListener      = AccessRestriction::EntryListener;
using StagingRestriction = Clusters::AccessControl::Structs::AccessRestrictionEntryStruct::Type;
using Restriction        = AccessRestriction::Restriction;

namespace {

/*
Size calculation for TLV encoded entry.

Because EncodeForWrite is used without an accessing fabric, the fabric index is
not encoded. However, let's assume it is. This yields 17 bytes total overhead,
but it's wise to add a few more for safety.

Each restriction requires 11 bytes.

DATA                                    C   T   L   V   NOTES
structure (anonymous)                   1               0x15
  field 0 endpoint                      1   1       1
  field 1 cluster                       1   1       4
  field 2 restrictions                  1   1
    structure (anonymous)               1               per restriction
      field 0 type                      1   1       1
      field 1 id                        1   1       4
    end structure                       1
  end list                              1               0x18
  field 254 fabric index                1   1       1   not written
end structure                           1               0x18
*/

constexpr int kEncodedEntryOverheadBytes    = 17 + 8;
constexpr int kEncodedEntryRestrictionBytes = 11 * CHIP_CONFIG_ACCESS_RESTRICTION_MAX_RESTRICTIONS_PER_ENTRY;
constexpr int kEncodedEntryTotalBytes       = kEncodedEntryOverheadBytes + kEncodedEntryRestrictionBytes;

class : public EntryListener
{
public:
    void OnEntryChanged(FabricIndex fabricIndex, size_t index, SharedPtr<Entry> entry, ChangeType changeType) override
    {
        CHIP_ERROR err;

        uint8_t buffer[kEncodedEntryTotalBytes] = { 0 };

        VerifyOrExit(mPersistentStorage != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

        if (changeType == ChangeType::kRemoved)
        {
            // Shuffle down entries past index, then delete entry at last index.
            while (true)
            {
                uint16_t size = static_cast<uint16_t>(sizeof(buffer));
                err           = mPersistentStorage->SyncGetKeyValue(
                    DefaultStorageKeyAllocator::AccessControlArlEntry(fabricIndex, index + 1).KeyName(), buffer, size);
                if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
                {
                    break;
                }
                SuccessOrExit(err);
                SuccessOrExit(err = mPersistentStorage->SyncSetKeyValue(
                                  DefaultStorageKeyAllocator::AccessControlArlEntry(fabricIndex, index).KeyName(), buffer, size));
                index++;
            }
            SuccessOrExit(err = mPersistentStorage->SyncDeleteKeyValue(
                              DefaultStorageKeyAllocator::AccessControlArlEntry(fabricIndex, index).KeyName()));
        }
        else
        {
            // Write added/updated entry at index.
            VerifyOrExit(entry != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
            TLV::TLVWriter writer;
            writer.Init(buffer);
            EncodableEntry encodableEntry(entry);
            SuccessOrExit(err = encodableEntry.EncodeForWrite(writer, TLV::AnonymousTag()));
            SuccessOrExit(err = mPersistentStorage->SyncSetKeyValue(
                              DefaultStorageKeyAllocator::AccessControlArlEntry(fabricIndex, index).KeyName(), buffer,
                              static_cast<uint16_t>(writer.GetLengthWritten())));
        }

        return;

    exit:
        ChipLogError(DataManagement, "DefaultArlStorage: failed %" CHIP_ERROR_FORMAT, err.Format());
    }

    void OnFabricRestrictionReviewUpdate(FabricIndex fabricIndex, uint64_t token, const char * instruction, const char * redirectUrl) override {}

    // Must initialize before use.
    void Init(PersistentStorageDelegate & persistentStorage) { mPersistentStorage = &persistentStorage; }

private:
    PersistentStorageDelegate * mPersistentStorage = nullptr;

} sEntryListener;

} // namespace

namespace chip {
namespace app {

CHIP_ERROR DefaultArlStorage::Init(PersistentStorageDelegate & persistentStorage, ConstFabricIterator first,
                                   ConstFabricIterator last)
{
    ChipLogProgress(DataManagement, "DefaultArlStorage: initializing");

    CHIP_ERROR err;

    [[maybe_unused]] size_t count = 0;

    for (auto it = first; it != last; ++it)
    {
        auto fabric = it->GetFabricIndex();
        for (size_t index = 0; /**/; ++index)
        {
            uint8_t buffer[kEncodedEntryTotalBytes] = { 0 };
            uint16_t size                           = static_cast<uint16_t>(sizeof(buffer));
            err = persistentStorage.SyncGetKeyValue(DefaultStorageKeyAllocator::AccessControlArlEntry(fabric, index).KeyName(),
                                                    buffer, size);
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
            entry.fabricIndex = fabric;

            SuccessOrExit(err = GetAccessControl().GetAccessRestriction()->CreateEntry(nullptr, entry, fabric));
            count++;
        }
    }

    ChipLogProgress(DataManagement, "DefaultArlStorage: %u entries loaded", (unsigned) count);

    sEntryListener.Init(persistentStorage);
    GetAccessControl().GetAccessRestriction()->AddListener(sEntryListener);

    return CHIP_NO_ERROR;

exit:
    ChipLogError(DataManagement, "DefaultArlStorage: failed %" CHIP_ERROR_FORMAT, err.Format());
    return err;
}

} // namespace app
} // namespace chip
