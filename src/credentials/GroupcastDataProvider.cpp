/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "GroupcastDataProvider.h"
#include <credentials/GroupDataProvider.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/PersistentArray.h>
#include <lib/support/logging/CHIPLogging.h>
#include <system/SystemClock.h>

using namespace chip;

namespace chip {
namespace Groupcast {

struct Tags
{
    static constexpr TLV::Tag GroupId() { return TLV::ContextTag(1); }
    static constexpr TLV::Tag KeyId() { return TLV::ContextTag(2); }
    static constexpr TLV::Tag EndpointCount() { return TLV::ContextTag(10); }
    static constexpr TLV::Tag EndpointList() { return TLV::ContextTag(12); }
    static constexpr TLV::Tag EndpointEntry() { return TLV::ContextTag(13); }
};

//
// GroupList
//

static constexpr size_t kMaxMembershipCount = CHIP_CONFIG_MAX_GROUPCAST_MEMBERSHIP_COUNT;
constexpr size_t kEndpointEntrySize         = 4; // TLV::AnonymousTag(1) + Endpoint(2) + EndContainer(1)
constexpr size_t kPersistentBufferMax =
    4 + kEndpointsMax * kEndpointEntrySize; // GroupId(2) + KeyId(2) + kEndpointsMax(8) * kEndpointEntrySize(4);

struct GroupList : public PersistentArray<kMaxMembershipCount, kPersistentBufferMax, Group>
{
    GroupList(FabricIndex fabric, PersistentStorageDelegate * storage) :
        PersistentArray<kMaxMembershipCount, kPersistentBufferMax, Group>(storage), mFabric(fabric)
    {}

    CHIP_ERROR UpdateKey(StorageKeyName & key) const override
    {
        VerifyOrReturnError(kUndefinedFabricIndex != mFabric, CHIP_ERROR_INVALID_FABRIC_INDEX);
        key = DefaultStorageKeyAllocator::GroupcastMembership(mFabric);
        return CHIP_NO_ERROR;
    }

    void ClearEntry(Group & entry) override
    {
        entry.group_id       = 0;
        entry.endpoint_count = 0;
        memset(entry.endpoints, 0x00, sizeof(entry.endpoints));
    }

    CHIP_ERROR Serialize(TLV::TLVWriter & writer, const Group & entry) const override
    {
        ReturnErrorOnFailure(writer.Put(Tags::GroupId(), static_cast<uint16_t>(entry.group_id)));
        ReturnErrorOnFailure(writer.Put(Tags::KeyId(), static_cast<uint32_t>(entry.key_id)));
        ReturnErrorOnFailure(writer.Put(Tags::EndpointCount(), static_cast<uint16_t>(entry.endpoint_count)));
        // Endpoints
        {
            TLV::TLVType array, item;
            ReturnErrorOnFailure(writer.StartContainer(Tags::EndpointList(), TLV::kTLVType_Array, array));
            for (size_t i = 0; i < kEndpointsMax; ++i)
            {
                ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, item));
                // Endpoint
                ReturnErrorOnFailure(writer.Put(Tags::EndpointEntry(), entry.endpoints[i]));
                ReturnErrorOnFailure(writer.EndContainer(item));
            }
            ReturnErrorOnFailure(writer.EndContainer(array));
        }
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Deserialize(TLV::TLVReader & reader, Group & entry) override
    {
        // Group Id
        ReturnErrorOnFailure(reader.Next(Tags::GroupId()));
        ReturnErrorOnFailure(reader.Get(entry.group_id));
        // Key Id
        ReturnErrorOnFailure(reader.Next(Tags::KeyId()));
        ReturnErrorOnFailure(reader.Get(entry.key_id));
        // Endpoint Count
        ReturnErrorOnFailure(reader.Next(Tags::EndpointCount()));
        ReturnErrorOnFailure(reader.Get(entry.endpoint_count));
        // Endpoints
        ReturnErrorOnFailure(reader.Next(Tags::EndpointList()));
        VerifyOrReturnError(TLV::kTLVType_Array == reader.GetType(), CHIP_ERROR_INTERNAL);
        {
            TLV::TLVType array, item;
            ReturnErrorOnFailure(reader.EnterContainer(array));
            for (size_t i = 0; i < kEndpointsMax; ++i)
            {
                ReturnErrorOnFailure(reader.Next(TLV::AnonymousTag()));
                VerifyOrReturnError(TLV::kTLVType_Structure == reader.GetType(), CHIP_ERROR_INTERNAL);
                ReturnErrorOnFailure(reader.EnterContainer(item));
                // Endpoint
                ReturnErrorOnFailure(reader.Next(Tags::EndpointEntry()));
                ReturnErrorOnFailure(reader.Get(entry.endpoints[i]));
                ReturnErrorOnFailure(reader.ExitContainer(item));
            }
            ReturnErrorOnFailure(reader.ExitContainer(array));
        }
        return CHIP_NO_ERROR;
    }

    FabricIndex mFabric;
};

//
// Group Iterator
//

DataProvider::GroupIterator::GroupIterator(DataProvider & group_data, FabricIndex fabric) : mProvider(group_data), mFabric(fabric)
{
    chip::Groupcast::GroupList list(fabric, group_data.mStorage);
    list.Load();
    mCount = list.Count();
}

size_t DataProvider::GroupIterator::Count()
{
    return mCount;
}

bool DataProvider::GroupIterator::Next(Group & out)
{
    VerifyOrReturnValue(mIndex < mCount, false);
    Group entry;
    chip::Groupcast::GroupList list(mFabric, mProvider.mStorage);
    list.Load();
    VerifyOrReturnValue(CHIP_NO_ERROR == list.Get(mIndex++, entry), false);
    out = entry;
    return true;
}

void DataProvider::GroupIterator::Release()
{
    mProvider.mGroupIteratorPool.ReleaseObject(this);
}

//
// DataProvider
//

namespace {
DataProvider sInstance;
}

DataProvider & DataProvider::DataProvider::Instance()
{
    return sInstance;
}

CHIP_ERROR DataProvider::Initialize(PersistentStorageDelegate * storage, chip::Crypto::SessionKeystore * keystore)
{
    VerifyOrReturnError(storage != nullptr, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(keystore != nullptr, CHIP_ERROR_INTERNAL);
    mStorage  = storage;
    mKeystore = keystore;
    return CHIP_NO_ERROR;
}

uint8_t DataProvider::GetMaxMembershipCount()
{
    return kMaxMembershipCount;
}

CHIP_ERROR DataProvider::AddGroup(chip::FabricIndex fabric_idx, Group & grp)
{
    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mKeystore != nullptr, CHIP_ERROR_INCORRECT_STATE);

    // Insert entry
    {
        chip::Groupcast::GroupList list(fabric_idx, mStorage);
        Group entry(grp.group_id, grp.key_id);
        size_t index = 0;

        // Load current entry, if any
        list.Find(entry, index);

        // Endpoints
        entry.endpoint_count = grp.endpoint_count;
        memcpy(entry.endpoints, grp.endpoints, entry.endpoint_count * sizeof(EndpointId));

        return list.Add(entry, true);
    }
}

CHIP_ERROR DataProvider::GetGroup(FabricIndex fabric_idx, Group & grp)
{
    chip::Groupcast::GroupList list(fabric_idx, mStorage);
    Group entry(grp.group_id);
    size_t index = 0;
    // Find grp in NVM
    ReturnErrorOnFailure(list.Find(entry, index));
    // Copy to output
    grp = entry;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DataProvider::SetEndpoints(FabricIndex fabric_idx, Group & grp)
{
    chip::Groupcast::GroupList list(fabric_idx, mStorage);
    Group entry(grp.group_id);
    size_t index = 0;
    // Find group in NVM
    ReturnErrorOnFailure(list.Find(entry, index));
    entry.endpoint_count = grp.endpoint_count;
    for (size_t i = 0; i < entry.endpoint_count; ++i)
    {
        entry.endpoints[i] = grp.endpoints[i];
    }
    return list.Save();
}

CHIP_ERROR DataProvider::RemoveGroup(FabricIndex fabric_idx, GroupId group_id)
{
    chip::Groupcast::GroupList list(fabric_idx, mStorage);
    ReturnErrorOnFailure(list.Remove(Group(group_id)));
    return list.Save();
}

DataProvider::GroupIterator * DataProvider::IterateGroups(FabricIndex fabric)
{
    return mGroupIteratorPool.CreateObject(*this, fabric);
}

} // namespace Groupcast
} // namespace chip
