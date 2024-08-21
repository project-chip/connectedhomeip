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
using Entry              = AccessRestrictionProvider::Entry;
using Listener           = AccessRestrictionProvider::Listener;
using StagingRestriction = Clusters::AccessControl::Structs::AccessRestrictionEntryStruct::Type;
using Restriction        = AccessRestrictionProvider::Restriction;

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

class : public Listener
{
public:
    void CommissioningRestrictionListChanged() override
    {
        // first clear out any existing entries
        CHIP_ERROR err;
        for (size_t index = 0; /**/; ++index)
        {
            err = mPersistentStorage->SyncDeleteKeyValue(
                DefaultStorageKeyAllocator::AccessControlCommissioningArlEntry(index).KeyName());
            if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
            {
                break;
            }
        }

        auto entries = GetAccessControl().GetAccessRestrictionProvider()->GetCommissioningEntries();
        size_t index = 0;
        for (auto & entry : entries)
        {
            uint8_t buffer[kEncodedEntryTotalBytes] = { 0 };
            TLV::TLVWriter writer;
            writer.Init(buffer);
            EncodableEntry encodableEntry(entry);
            SuccessOrExit(err = encodableEntry.EncodeForWrite(writer, TLV::AnonymousTag()));
            SuccessOrExit(err = mPersistentStorage->SyncSetKeyValue(
                              DefaultStorageKeyAllocator::AccessControlCommissioningArlEntry(index++).KeyName(), buffer,
                              static_cast<uint16_t>(writer.GetLengthWritten())));
        }

        return;

    exit:
        ChipLogError(DataManagement, "DefaultArlStorage: failed %" CHIP_ERROR_FORMAT, err.Format());
    }

    void RestrictionListChanged(FabricIndex fabricIndex) override
    {
        // first clear out any existing entries
        CHIP_ERROR err;
        size_t index = 0;
        for (index = 0; /**/; ++index)
        {
            err = mPersistentStorage->SyncDeleteKeyValue(
                DefaultStorageKeyAllocator::AccessControlArlEntry(fabricIndex, index).KeyName());
            if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
            {
                break;
            }
        }

        const std::vector<Entry> * entries;
        SuccessOrExit(err = GetAccessControl().GetAccessRestrictionProvider()->GetEntries(fabricIndex, entries));
        VerifyOrExit(entries != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
        index = 0;
        for (auto & entry : *entries)
        {
            uint8_t buffer[kEncodedEntryTotalBytes] = { 0 };
            TLV::TLVWriter writer;
            writer.Init(buffer);
            EncodableEntry encodableEntry(entry);
            SuccessOrExit(err = encodableEntry.EncodeForWrite(writer, TLV::AnonymousTag()));
            SuccessOrExit(err = mPersistentStorage->SyncSetKeyValue(
                              DefaultStorageKeyAllocator::AccessControlArlEntry(fabricIndex, index++).KeyName(), buffer,
                              static_cast<uint16_t>(writer.GetLengthWritten())));
        }

        return;

    exit:
        ChipLogError(DataManagement, "DefaultArlStorage: failed %" CHIP_ERROR_FORMAT, err.Format());
    }

    void OnFabricRestrictionReviewUpdate(FabricIndex fabricIndex, uint64_t token, const char * instruction,
                                         const char * redirectUrl) override
    {}

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
    size_t commissioningCount = 0;
    size_t count              = 0;

    std::vector<Entry> commissioningEntries;
    for (size_t index = 0; /**/; ++index)
    {
        uint8_t buffer[kEncodedEntryTotalBytes] = { 0 };
        uint16_t size                           = static_cast<uint16_t>(sizeof(buffer));
        err = persistentStorage.SyncGetKeyValue(DefaultStorageKeyAllocator::AccessControlCommissioningArlEntry(index).KeyName(),
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

        commissioningEntries.push_back(decodableEntry.GetEntry());
        commissioningCount++;
    }
    GetAccessControl().GetAccessRestrictionProvider()->SetCommissioningEntries(commissioningEntries);

    for (auto it = first; it != last; ++it)
    {
        std::vector<Entry> entries;
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

            entries.push_back(decodableEntry.GetEntry());
            count++;
        }
        GetAccessControl().GetAccessRestrictionProvider()->SetEntries(fabric, entries);
    }

    ChipLogProgress(DataManagement, "DefaultArlStorage: %u commissioning entries loaded, %u fabric entries loaded",
                    (unsigned) commissioningCount, (unsigned) count);

    sEntryListener.Init(persistentStorage);
    GetAccessControl().GetAccessRestrictionProvider()->AddListener(sEntryListener);

    return CHIP_NO_ERROR;

exit:
    ChipLogError(DataManagement, "DefaultArlStorage: failed %" CHIP_ERROR_FORMAT, err.Format());
    return err;
}

} // namespace app
} // namespace chip
