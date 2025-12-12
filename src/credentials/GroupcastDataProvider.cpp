/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
#include <app/storage/FabricTableImpl.ipp> // nogncheck
#include <credentials/GroupDataProvider.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/logging/CHIPLogging.h>
#include <system/SystemClock.h>

using namespace chip;

struct Tags
{
    static constexpr TLV::Tag GroupId() { return TLV::ContextTag(1); }
    static constexpr TLV::Tag KeyId() { return TLV::ContextTag(2); }
    static constexpr TLV::Tag EndpointCount() { return TLV::ContextTag(10); }
    static constexpr TLV::Tag EndpointList() { return TLV::ContextTag(12); }
    static constexpr TLV::Tag EndpointEntry() { return TLV::ContextTag(13); }
};

using Serializer = chip::app::Storage::DefaultSerializer<Groupcast::GroupDataId, Groupcast::GroupData>;

namespace chip {
namespace app {
namespace Storage {

template <>
StorageKeyName Serializer::EndpointEntryCountKey(EndpointId endpoint_id)
{
    return DefaultStorageKeyAllocator::GroupcastEntryCountKey();
}

template <>
StorageKeyName Serializer::FabricEntryKey(FabricIndex fabric, EndpointId endpoint, uint16_t idx)
{
    return DefaultStorageKeyAllocator::GroupcastFabricEntryKey(fabric, idx);
}

template <>
StorageKeyName Serializer::FabricEntryDataKey(FabricIndex fabric, EndpointId endpoint)
{
    return DefaultStorageKeyAllocator::GroupcastFabricEntryDataKey(fabric);
}

template <>
constexpr size_t Serializer::kEntryMaxBytes()
{
    return Groupcast::kMaxGroupsPerFabric *
        TLV::EstimateStructOverhead(sizeof(GroupId) + sizeof(KeysetId) + sizeof(uint16_t),
                                    Groupcast::kMaxEndpointsPerGroup * (3 + sizeof(EndpointId)));
}

template <>
constexpr uint16_t Serializer::kMaxPerFabric()
{
    return Groupcast::kMaxGroupsPerFabric;
}

template <>
constexpr uint16_t Serializer::kMaxPerEndpoint()
{
    return Groupcast::kMaxEndpointsPerGroup;
}

// A Full fabric serialized TLV length is 88 bytes, 128 bytes gives some slack.  Tested by running writer.GetLengthWritten at the
// end of the Serialize method of FabricGroupData
template <>
constexpr size_t Serializer::kFabricMaxBytes()
{
    return 128;
}

template <>
CHIP_ERROR Serializer::SerializeId(TLV::TLVWriter & writer, const Groupcast::GroupDataId & id)
{
    ReturnErrorOnFailure(writer.Put(Tags::GroupId(), id.mGroupId));
    return CHIP_NO_ERROR;
}

template <>
CHIP_ERROR Serializer::DeserializeId(TLV::TLVReader & reader, Groupcast::GroupDataId & id)
{
    // Scene ID
    ReturnErrorOnFailure(reader.Next(Tags::GroupId()));
    ReturnErrorOnFailure(reader.Get(id.mGroupId));
    return CHIP_NO_ERROR;
}

template <>
CHIP_ERROR Serializer::SerializeData(TLV::TLVWriter & writer, const Groupcast::GroupData & data)
{
    ReturnErrorOnFailure(writer.Put(Tags::GroupId(), static_cast<uint16_t>(data.groupID)));
    ReturnErrorOnFailure(writer.Put(Tags::KeyId(), static_cast<uint32_t>(data.keyID)));
    ReturnErrorOnFailure(writer.Put(Tags::EndpointCount(), static_cast<uint16_t>(data.endpointCount)));
    // Endpoints
    {
        TLV::TLVType array, item;
        ReturnErrorOnFailure(writer.StartContainer(Tags::EndpointList(), TLV::kTLVType_Array, array));
        for (size_t i = 0; i < data.endpointCount; ++i)
        {
            ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, item));
            // Endpoint
            ReturnErrorOnFailure(writer.Put(Tags::EndpointEntry(), data.endpoints[i]));
            ReturnErrorOnFailure(writer.EndContainer(item));
        }
        ReturnErrorOnFailure(writer.EndContainer(array));
    }
    return CHIP_NO_ERROR;
}

template <>
CHIP_ERROR Serializer::DeserializeData(TLV::TLVReader & reader, Groupcast::GroupData & data)
{
    // Group Id
    ReturnErrorOnFailure(reader.Next(Tags::GroupId()));
    ReturnErrorOnFailure(reader.Get(data.groupID));
    // Key Id
    ReturnErrorOnFailure(reader.Next(Tags::KeyId()));
    ReturnErrorOnFailure(reader.Get(data.keyID));
    // Endpoint Count
    ReturnErrorOnFailure(reader.Next(Tags::EndpointCount()));
    ReturnErrorOnFailure(reader.Get(data.endpointCount));
    // Endpoints
    ReturnErrorOnFailure(reader.Next(Tags::EndpointList()));
    VerifyOrReturnError(TLV::kTLVType_Array == reader.GetType(), CHIP_ERROR_INTERNAL);
    {
        TLV::TLVType array, item;
        ReturnErrorOnFailure(reader.EnterContainer(array));
        for (size_t i = 0; i < data.endpointCount; ++i)
        {
            ReturnErrorOnFailure(reader.Next(TLV::AnonymousTag()));
            VerifyOrReturnError(TLV::kTLVType_Structure == reader.GetType(), CHIP_ERROR_INTERNAL);
            ReturnErrorOnFailure(reader.EnterContainer(item));
            // Endpoint
            ReturnErrorOnFailure(reader.Next(Tags::EndpointEntry()));
            ReturnErrorOnFailure(reader.Get(data.endpoints[i]));
            ReturnErrorOnFailure(reader.ExitContainer(item));
        }
        ReturnErrorOnFailure(reader.ExitContainer(array));
    }
    return CHIP_NO_ERROR;
}

template class chip::app::Storage::FabricTableImpl<chip::Groupcast::GroupDataId, chip::Groupcast::GroupData>;

} // namespace Storage
} // namespace app

namespace Groupcast {

//
// DataProvider
//

namespace {
DataProvider * sInstance = nullptr;
}

void DataProvider::SetInstance(DataProvider * instance)
{
    sInstance = instance;
}

DataProvider * DataProvider::Instance()
{
    return sInstance;
}

CHIP_ERROR DataProvider::Initialize(PersistentStorageDelegate * storage, chip::Crypto::SessionKeystore * keystore,
                                    GroupDataProvider * legacy_groups)
{
    VerifyOrReturnError(!IsInitialized(), CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(storage != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(keystore != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(legacy_groups != nullptr, CHIP_ERROR_INCORRECT_STATE);
    mStorage      = storage;
    mKeystore     = keystore;
    mLegacyGroups = legacy_groups;
    return mList.Init(*storage);
}

bool DataProvider::IsInitialized()
{
    return (nullptr != mStorage) && (nullptr != mKeystore) && (nullptr != mLegacyGroups);
}

uint8_t DataProvider::GetMaxMembershipCount()
{
    return kMaxMembershipCount;
}

CHIP_ERROR DataProvider::SetGroup(chip::FabricIndex fabric_index, GroupData & group)
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);
    PersistenceBuffer<Serializer::kEntryMaxBytes()> buffer;
    GroupDataId id(group.groupID);
    return mList.SetTableEntry(fabric_index, id, group, buffer);
}

CHIP_ERROR DataProvider::GetGroup(FabricIndex fabric_index, GroupData & group)
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);
    PersistenceBuffer<Serializer::kEntryMaxBytes()> buffer;
    GroupDataId id(group.groupID);
    return mList.GetTableEntry(fabric_index, id, group, buffer);
}

CHIP_ERROR DataProvider::RemoveGroup(FabricIndex fabric_index, GroupId group_id)
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);
    GroupDataId id(group_id);
    return mList.RemoveTableEntry(fabric_index, id);
}

CHIP_ERROR DataProvider::SetEndpoints(FabricIndex fabric_index, GroupData & group)
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(group.endpointCount <= kMaxEndpointsPerGroup, CHIP_ERROR_INVALID_ARGUMENT);

    PersistenceBuffer<Serializer::kEntryMaxBytes()> buffer;
    GroupDataId id(group.groupID);
    GroupData entry;

    ReturnErrorOnFailure(mList.GetTableEntry(fabric_index, id, entry, buffer));
    entry.endpointCount = group.endpointCount;
    for (size_t i = 0; i < entry.endpointCount; ++i)
    {
        entry.endpoints[i] = group.endpoints[i];
    }
    return mList.SetTableEntry(fabric_index, id, entry, buffer);
}

CHIP_ERROR DataProvider::IterateGroups(FabricIndex fabric, IteratorCallback iterateFn)
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);
    PersistenceBuffer<Serializer::kEntryMaxBytes()> buffer;
    return mList.IterateEntries(fabric, buffer, [&](auto & iter) {
        app::Storage::Data::TableEntryDataConvertingIterator<GroupDataId, GroupData> innerIter(iter);
        return iterateFn(innerIter);
    });
}

chip::Crypto::SymmetricKeyContext * DataProvider::CreateKeyContext(FabricIndex fabric, GroupId groupId)
{
    VerifyOrReturnError(IsInitialized(), nullptr);
    Groupcast::GroupData group;
    VerifyOrReturnError(CHIP_NO_ERROR == GetGroup(fabric, group), nullptr);
    return mLegacyGroups->GetKeysetContext(fabric, group.keyID);
}

CHIP_ERROR DataProvider::FindGroupSession(FabricIndex fabric_index, uint16_t hash, GroupSession & session)
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE);
    KeysetId keyset_id = 0;
    session.Clear();
    session.keyContext = mLegacyGroups->FindGroupContext(fabric_index, hash, keyset_id);
    VerifyOrReturnError(nullptr != session.keyContext, CHIP_ERROR_NOT_FOUND);

    GroupData group;
    auto result = IterateGroups(fabric_index, [&](auto & iter) -> CHIP_ERROR {
        while (iter.Next(group))
        {
            if (keyset_id == group.keyID)
            {
                session.fabric_index = fabric_index;
                session.group_id     = group.groupID;
            }
        }
        return CHIP_ERROR_NOT_FOUND;
    });
    if (CHIP_NO_ERROR != result)
    {
        session.keyContext->Release();
        return result;
    }
    return CHIP_NO_ERROR;
}

} // namespace Groupcast
} // namespace chip
