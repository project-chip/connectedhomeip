/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include <credentials/GroupDataProviderImpl.h>
#include <lib/core/CHIPTLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/Pool.h>
#include <stdlib.h>
#include <string.h>

namespace chip {
namespace Credentials {

static constexpr size_t kPersistentBufferMax = 128;

template <size_t kMaxSerializedSize>
struct PersistentData
{
    virtual ~PersistentData() = default;

    virtual CHIP_ERROR UpdateKey(DefaultStorageKeyAllocator & key) = 0;
    virtual CHIP_ERROR Serialize(TLV::TLVWriter & writer) const    = 0;
    virtual CHIP_ERROR Deserialize(TLV::TLVReader & reader)        = 0;
    virtual void Clear()                                           = 0;

    CHIP_ERROR Save(chip::PersistentStorageDelegate & storage)
    {
        uint8_t buffer[kMaxSerializedSize] = { 0 };
        DefaultStorageKeyAllocator key;
        // Update storage key
        ReturnErrorOnFailure(UpdateKey(key));

        // Serialize the data
        TLV::TLVWriter writer;
        writer.Init(buffer, sizeof(buffer));
        ReturnErrorOnFailure(Serialize(writer));

        // Save serialized data
        return storage.SyncSetKeyValue(key.KeyName(), buffer, static_cast<uint16_t>(writer.GetLengthWritten()));
    }

    CHIP_ERROR Load(chip::PersistentStorageDelegate & storage)
    {
        uint8_t buffer[kMaxSerializedSize] = { 0 };
        DefaultStorageKeyAllocator key;

        // Set data to defaults
        Clear();

        // Update storage key
        ReturnErrorOnFailure(UpdateKey(key));

        // Load the serialized data
        uint16_t size  = static_cast<uint16_t>(sizeof(buffer));
        CHIP_ERROR err = storage.SyncGetKeyValue(key.KeyName(), buffer, size);
        VerifyOrReturnError(CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND != err, CHIP_ERROR_NOT_FOUND);
        ReturnErrorOnFailure(err);

        // Decode serialized data
        TLV::TLVReader reader;
        reader.Init(buffer, size);
        return Deserialize(reader);
    }

    CHIP_ERROR Delete(chip::PersistentStorageDelegate & storage)
    {
        DefaultStorageKeyAllocator key;
        // Update storage key
        ReturnErrorOnFailure(UpdateKey(key));
        // Delete stored data
        return storage.SyncDeleteKeyValue(key.KeyName());
    }
};

struct LinkedData : public PersistentData<kPersistentBufferMax>
{
    static constexpr uint16_t kMinLinkId = 1;

    LinkedData() = default;
    LinkedData(uint16_t linked_id) : id(linked_id) {}
    uint16_t id     = kMinLinkId;
    uint16_t index  = 0;
    uint16_t next   = 0;
    uint16_t prev   = 0;
    uint16_t max_id = 0;
    bool first      = true;
};

struct FabricData : public PersistentData<kPersistentBufferMax>
{
    static constexpr TLV::Tag TagFirstGroup() { return TLV::ContextTag(1); }
    static constexpr TLV::Tag TagGroupCount() { return TLV::ContextTag(2); }
    static constexpr TLV::Tag TagFirstMap() { return TLV::ContextTag(3); }
    static constexpr TLV::Tag TagMapCount() { return TLV::ContextTag(4); }
    static constexpr TLV::Tag TagFirstKeyset() { return TLV::ContextTag(5); }
    static constexpr TLV::Tag TagKeysetCount() { return TLV::ContextTag(6); }

    chip::FabricIndex fabric_index = kUndefinedFabricIndex;
    chip::GroupId first_group      = kUndefinedGroupId;
    uint16_t group_count           = 0;
    uint16_t first_map             = 0;
    uint16_t map_count             = 0;
    chip::KeysetId first_keyset    = 0xffff;
    uint16_t keyset_count          = 0;

    FabricData() = default;
    FabricData(chip::FabricIndex fabric) : fabric_index(fabric) {}

    CHIP_ERROR UpdateKey(DefaultStorageKeyAllocator & key) override
    {
        VerifyOrReturnError(kUndefinedFabricIndex != fabric_index, CHIP_ERROR_INVALID_FABRIC_ID);
        key.FabricGroups(fabric_index);
        return CHIP_NO_ERROR;
    }

    void Clear() override
    {
        first_group  = kUndefinedGroupId;
        group_count  = 0;
        first_keyset = 0xffff;
        keyset_count = 0;
    }

    CHIP_ERROR Serialize(TLV::TLVWriter & writer) const override
    {
        TLV::TLVType container;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, container));

        ReturnErrorOnFailure(writer.Put(TagFirstGroup(), static_cast<uint16_t>(first_group)));
        ReturnErrorOnFailure(writer.Put(TagGroupCount(), static_cast<uint16_t>(group_count)));
        ReturnErrorOnFailure(writer.Put(TagFirstMap(), static_cast<uint16_t>(first_map)));
        ReturnErrorOnFailure(writer.Put(TagMapCount(), static_cast<uint16_t>(map_count)));
        ReturnErrorOnFailure(writer.Put(TagFirstKeyset(), static_cast<uint16_t>(first_keyset)));
        ReturnErrorOnFailure(writer.Put(TagKeysetCount(), static_cast<uint16_t>(keyset_count)));

        return writer.EndContainer(container);
    }
    CHIP_ERROR Deserialize(TLV::TLVReader & reader) override
    {
        ReturnErrorOnFailure(reader.Next(TLV::AnonymousTag()));
        VerifyOrReturnError(TLV::kTLVType_Structure == reader.GetType(), CHIP_ERROR_INTERNAL);

        TLV::TLVType container;
        ReturnErrorOnFailure(reader.EnterContainer(container));

        // first_group
        ReturnErrorOnFailure(reader.Next(TagFirstGroup()));
        ReturnErrorOnFailure(reader.Get(first_group));
        // group_count
        ReturnErrorOnFailure(reader.Next(TagGroupCount()));
        ReturnErrorOnFailure(reader.Get(group_count));
        // first_map
        ReturnErrorOnFailure(reader.Next(TagFirstMap()));
        ReturnErrorOnFailure(reader.Get(first_map));
        // map_count
        ReturnErrorOnFailure(reader.Next(TagMapCount()));
        ReturnErrorOnFailure(reader.Get(map_count));
        // first_keyset
        ReturnErrorOnFailure(reader.Next(TagFirstKeyset()));
        ReturnErrorOnFailure(reader.Get(first_keyset));
        // keyset_count
        ReturnErrorOnFailure(reader.Next(TagKeysetCount()));
        ReturnErrorOnFailure(reader.Get(keyset_count));

        return reader.ExitContainer(container);
    }
};

struct GroupData : public GroupDataProvider::GroupInfo, LinkedData
{
    static constexpr TLV::Tag TagGroupId() { return TLV::ContextTag(1); }
    static constexpr TLV::Tag TagName() { return TLV::ContextTag(2); }
    static constexpr TLV::Tag TagFirstEndpoint() { return TLV::ContextTag(3); }
    static constexpr TLV::Tag TagEndpointCount() { return TLV::ContextTag(4); }
    static constexpr TLV::Tag TagNext() { return TLV::ContextTag(5); }

    chip::FabricIndex fabric_index  = kUndefinedFabricIndex;
    chip::EndpointId first_endpoint = kInvalidEndpointId;
    uint16_t endpoint_count         = 0;

    GroupData() : GroupInfo(nullptr){};
    GroupData(chip::FabricIndex fabric) : GroupInfo(), LinkedData(), fabric_index(fabric) {}
    GroupData(chip::FabricIndex fabric, uint16_t link_id) : GroupInfo(), LinkedData(link_id), fabric_index(fabric) {}

    CHIP_ERROR UpdateKey(DefaultStorageKeyAllocator & key) override
    {
        VerifyOrReturnError(kUndefinedFabricIndex != fabric_index, CHIP_ERROR_INVALID_FABRIC_ID);
        key.FabricGroup(fabric_index, id);
        return CHIP_NO_ERROR;
    }

    void Clear() override
    {
        group_id = kUndefinedGroupId;
        SetName(CharSpan());
        first_endpoint = kInvalidEndpointId;
        endpoint_count = 0;
        next           = 0;
    }

    CHIP_ERROR Serialize(TLV::TLVWriter & writer) const override
    {
        TLV::TLVType container;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, container));

        size_t name_size = strnlen(name, GroupDataProvider::GroupInfo::kGroupNameMax);
        ReturnErrorOnFailure(writer.Put(TagGroupId(), static_cast<uint16_t>(group_id)));
        ReturnErrorOnFailure(writer.PutString(TagName(), name, static_cast<uint32_t>(name_size)));
        ReturnErrorOnFailure(writer.Put(TagFirstEndpoint(), static_cast<uint16_t>(first_endpoint)));
        ReturnErrorOnFailure(writer.Put(TagEndpointCount(), static_cast<uint16_t>(endpoint_count)));
        ReturnErrorOnFailure(writer.Put(TagNext(), static_cast<uint16_t>(next)));
        return writer.EndContainer(container);
    }
    CHIP_ERROR Deserialize(TLV::TLVReader & reader) override
    {
        ReturnErrorOnFailure(reader.Next(TLV::AnonymousTag()));
        VerifyOrReturnError(TLV::kTLVType_Structure == reader.GetType(), CHIP_ERROR_INTERNAL);

        TLV::TLVType container;
        ReturnErrorOnFailure(reader.EnterContainer(container));

        // group_id
        ReturnErrorOnFailure(reader.Next(TagGroupId()));
        ReturnErrorOnFailure(reader.Get(group_id));
        // name
        ReturnErrorOnFailure(reader.Next(TagName()));
        ReturnErrorOnFailure(reader.GetString(name, sizeof(name)));
        size_t size = strnlen(name, kGroupNameMax);
        name[size]  = 0;
        // first_endpoint
        ReturnErrorOnFailure(reader.Next(TagFirstEndpoint()));
        ReturnErrorOnFailure(reader.Get(first_endpoint));
        // endpoint_count
        ReturnErrorOnFailure(reader.Next(TagEndpointCount()));
        ReturnErrorOnFailure(reader.Get(endpoint_count));
        // next
        ReturnErrorOnFailure(reader.Next(TagNext()));
        ReturnErrorOnFailure(reader.Get(next));

        return reader.ExitContainer(container);
    }

    bool Get(chip::PersistentStorageDelegate & storage, const FabricData & fabric, size_t target_index)
    {
        fabric_index = fabric.fabric_index;
        id           = fabric.first_group;
        max_id       = kMinLinkId;
        index        = 0;
        first        = true;

        while (index < fabric.group_count)
        {
            if (CHIP_NO_ERROR != Load(storage))
            {
                break;
            }
            if (index == target_index)
            {
                // Target index found
                return true;
            }

            max_id = std::max(id, max_id);
            first  = false;
            prev   = id;
            id     = next;
            index++;
        }

        id = static_cast<uint16_t>(max_id + 1);
        return false;
    }

    bool Find(chip::PersistentStorageDelegate & storage, const FabricData & fabric, chip::GroupId target_group)
    {
        fabric_index = fabric.fabric_index;
        id           = fabric.first_group;
        max_id       = 0;
        index        = 0;
        first        = true;

        while (index < fabric.group_count)
        {
            if (CHIP_NO_ERROR != Load(storage))
            {
                break;
            }
            if (group_id == target_group)
            {
                // Target index found
                return true;
            }
            max_id = std::max(id, max_id);
            first  = false;
            prev   = id;
            id     = next;
            index++;
        }
        id = static_cast<uint16_t>(max_id + 1);
        return false;
    }
};

struct KeyMapData : public GroupDataProvider::GroupKey, LinkedData
{
    static constexpr TLV::Tag TagGroupId() { return TLV::ContextTag(1); }
    static constexpr TLV::Tag TagKeysetId() { return TLV::ContextTag(2); }
    static constexpr TLV::Tag TagNext() { return TLV::ContextTag(3); }

    chip::FabricIndex fabric_index = kUndefinedFabricIndex;
    chip::GroupId group_id         = kUndefinedGroupId;
    chip::KeysetId keyset_id       = 0;

    KeyMapData() : GroupKey(){};
    KeyMapData(chip::FabricIndex fabric, uint16_t link_id = 0, chip::GroupId group = kUndefinedGroupId, chip::KeysetId keyset = 0) :
        GroupKey(group, keyset), LinkedData(link_id), fabric_index(fabric)
    {}

    CHIP_ERROR UpdateKey(DefaultStorageKeyAllocator & key) override
    {
        VerifyOrReturnError(kUndefinedFabricIndex != fabric_index, CHIP_ERROR_INVALID_FABRIC_ID);
        key.FabricGroupKey(fabric_index, id);
        return CHIP_NO_ERROR;
    }

    void Clear() override {}

    CHIP_ERROR Serialize(TLV::TLVWriter & writer) const override
    {
        TLV::TLVType container;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, container));

        ReturnErrorOnFailure(writer.Put(TagGroupId(), static_cast<uint16_t>(group_id)));
        ReturnErrorOnFailure(writer.Put(TagKeysetId(), static_cast<uint16_t>(keyset_id)));
        ReturnErrorOnFailure(writer.Put(TagNext(), static_cast<uint16_t>(next)));
        return writer.EndContainer(container);
    }

    CHIP_ERROR Deserialize(TLV::TLVReader & reader) override
    {
        ReturnErrorOnFailure(reader.Next(TLV::AnonymousTag()));
        VerifyOrReturnError(TLV::kTLVType_Structure == reader.GetType(), CHIP_ERROR_INTERNAL);

        TLV::TLVType container;
        ReturnErrorOnFailure(reader.EnterContainer(container));

        // first_endpoint
        ReturnErrorOnFailure(reader.Next(TagGroupId()));
        ReturnErrorOnFailure(reader.Get(group_id));
        // endpoint_count
        ReturnErrorOnFailure(reader.Next(TagKeysetId()));
        ReturnErrorOnFailure(reader.Get(keyset_id));
        // next
        ReturnErrorOnFailure(reader.Next(TagNext()));
        ReturnErrorOnFailure(reader.Get(next));

        return reader.ExitContainer(container);
    }

    bool Get(chip::PersistentStorageDelegate & storage, const FabricData & fabric, size_t target_index)
    {
        fabric_index = fabric.fabric_index;
        id           = fabric.first_map;
        max_id       = 0;
        index        = 0;
        first        = true;

        while (index < fabric.map_count)
        {
            if (CHIP_NO_ERROR != Load(storage))
            {
                break;
            }
            if (index == target_index)
            {
                // Target index found
                return true;
            }
            max_id = std::max(id, max_id);
            first  = false;
            prev   = id;
            id     = next;
            index++;
        }

        id = static_cast<uint16_t>(max_id + 1);
        return false;
    }

    bool Find(chip::PersistentStorageDelegate & storage, const FabricData & fabric, const GroupKey & map)
    {
        fabric_index = fabric.fabric_index;
        id           = fabric.first_map;
        max_id       = 0;
        index        = 0;
        first        = true;

        while (index < fabric.map_count)
        {
            if (CHIP_NO_ERROR != Load(storage))
            {
                break;
            }
            if ((group_id == map.group_id) && (keyset_id == map.keyset_id))
            {
                // Match found
                return true;
            }
            max_id = std::max(id, max_id);
            first  = false;
            prev   = id;
            id     = next;
            index++;
        }

        id = static_cast<uint16_t>(max_id + 1);
        return false;
    }
};

struct EndpointData : GroupDataProvider::GroupEndpoint, LinkedData
{
    static constexpr TLV::Tag TagEndpoint() { return TLV::ContextTag(1); }
    static constexpr TLV::Tag TagNext() { return TLV::ContextTag(2); }

    chip::FabricIndex fabric_index = kUndefinedFabricIndex;
    uint16_t group_link_id         = 0;

    EndpointData() = default;
    EndpointData(chip::FabricIndex fabric, uint16_t group_linked_id, uint16_t link_id = kMinLinkId,
                 chip::GroupId group = kUndefinedGroupId, chip::EndpointId endpoint = kInvalidEndpointId) :
        GroupEndpoint(group, endpoint),
        LinkedData(link_id), fabric_index(fabric), group_link_id(group_linked_id)
    {}

    CHIP_ERROR UpdateKey(DefaultStorageKeyAllocator & key) override
    {
        VerifyOrReturnError(kUndefinedFabricIndex != fabric_index, CHIP_ERROR_INVALID_FABRIC_ID);
        key.FabricGroupEndpoint(fabric_index, group_link_id, id);
        return CHIP_NO_ERROR;
    }

    void Clear() override { next = kInvalidEndpointId; }

    CHIP_ERROR Serialize(TLV::TLVWriter & writer) const override
    {
        TLV::TLVType container;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, container));

        ReturnErrorOnFailure(writer.Put(TagEndpoint(), static_cast<uint16_t>(endpoint_id)));
        ReturnErrorOnFailure(writer.Put(TagNext(), static_cast<uint16_t>(next)));

        return writer.EndContainer(container);
    }
    CHIP_ERROR Deserialize(TLV::TLVReader & reader) override
    {
        ReturnErrorOnFailure(reader.Next(TLV::AnonymousTag()));
        VerifyOrReturnError(TLV::kTLVType_Structure == reader.GetType(), CHIP_ERROR_INTERNAL);

        TLV::TLVType container;
        ReturnErrorOnFailure(reader.EnterContainer(container));

        // endpoint_id
        ReturnErrorOnFailure(reader.Next(TagEndpoint()));
        ReturnErrorOnFailure(reader.Get(endpoint_id));
        // next
        ReturnErrorOnFailure(reader.Next(TagNext()));
        ReturnErrorOnFailure(reader.Get(next));

        return reader.ExitContainer(container);
    }

    bool Find(chip::PersistentStorageDelegate & storage, const FabricData & fabric, const GroupData & group,
              chip::EndpointId target_id)
    {
        fabric_index  = fabric.fabric_index;
        group_link_id = group.id;
        group_id      = group.group_id;
        id            = group.first_endpoint;
        max_id        = 0;
        index         = 0;
        first         = true;

        while (index < group.endpoint_count)
        {
            if (CHIP_NO_ERROR != Load(storage))
            {
                break;
            }
            if (this->endpoint_id == target_id)
            {
                // Match found
                return true;
            }

            max_id = std::max(id, max_id);
            first  = false;
            prev   = id;
            id     = next;
            index++;
        }

        id = static_cast<uint16_t>(max_id + 1);
        return false;
    }
};

struct KeySetData : public GroupDataProvider::KeySet, PersistentData<kPersistentBufferMax>
{
    static constexpr TLV::Tag TagKeySetId() { return TLV::ContextTag(1); }
    static constexpr TLV::Tag TagPolicy() { return TLV::ContextTag(2); }
    static constexpr TLV::Tag TagNumKeys() { return TLV::ContextTag(3); }
    static constexpr TLV::Tag TagEpochKeys() { return TLV::ContextTag(4); }
    static constexpr TLV::Tag TagStartTime() { return TLV::ContextTag(5); }
    static constexpr TLV::Tag TagKey() { return TLV::ContextTag(6); }
    static constexpr TLV::Tag TagNext() { return TLV::ContextTag(7); }

    chip::FabricIndex fabric_index = kUndefinedFabricIndex;
    chip::KeysetId next            = 0xffff;
    chip::KeysetId prev            = 0xffff;
    bool first                     = true;

    KeySetData() = default;
    KeySetData(chip::FabricIndex fabric, chip::KeysetId id) : fabric_index(fabric) { keyset_id = id; }
    KeySetData(chip::FabricIndex fabric, chip::KeysetId id, SecurityPolicy policy_id, uint8_t num_keys) :
        KeySet(id, policy_id, num_keys), fabric_index(fabric)
    {}

    CHIP_ERROR UpdateKey(DefaultStorageKeyAllocator & key) override
    {
        VerifyOrReturnError(kUndefinedFabricIndex != fabric_index, CHIP_ERROR_INVALID_FABRIC_ID);
        VerifyOrReturnError(kUndefinedFabricIndex != fabric_index, CHIP_ERROR_INVALID_KEY_ID);
        key.FabricKeyset(fabric_index, keyset_id);
        return CHIP_NO_ERROR;
    }

    void Clear() override
    {
        policy        = KeySet::SecurityPolicy::kStandard;
        num_keys_used = 0;
        memset(epoch_keys, 0x00, sizeof(epoch_keys));
        next = 0xffff;
    }

    CHIP_ERROR Serialize(TLV::TLVWriter & writer) const override
    {
        TLV::TLVType container;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, container));

        // keyset_id
        ReturnErrorOnFailure(writer.Put(TagKeySetId(), static_cast<uint16_t>(keyset_id)));
        // policy
        ReturnErrorOnFailure(writer.Put(TagPolicy(), static_cast<uint16_t>(policy)));
        // num_keys_used
        ReturnErrorOnFailure(writer.Put(TagNumKeys(), static_cast<uint16_t>(num_keys_used)));
        // epoch_keys
        {
            TLV::TLVType array, item;
            ReturnErrorOnFailure(writer.StartContainer(TagEpochKeys(), TLV::kTLVType_Array, array));
            for (auto & epoch : epoch_keys)
            {
                ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, item));
                ReturnErrorOnFailure(writer.Put(TagStartTime(), static_cast<uint64_t>(epoch.start_time)));
                ReturnErrorOnFailure(writer.Put(TagKey(), ByteSpan(epoch.key, GroupDataProvider::EpochKey::kLengthBytes)));
                ReturnErrorOnFailure(writer.EndContainer(item));
            }
            ReturnErrorOnFailure(writer.EndContainer(array));
        }
        // next keyset
        ReturnErrorOnFailure(writer.Put(TagNext(), static_cast<uint16_t>(next)));

        return writer.EndContainer(container);
    }

    CHIP_ERROR Deserialize(TLV::TLVReader & reader) override
    {
        ReturnErrorOnFailure(reader.Next(TLV::AnonymousTag()));
        VerifyOrReturnError(TLV::kTLVType_Structure == reader.GetType(), CHIP_ERROR_INTERNAL);

        TLV::TLVType container;
        ReturnErrorOnFailure(reader.EnterContainer(container));

        // keyset_id
        ReturnErrorOnFailure(reader.Next(TagKeySetId()));
        ReturnErrorOnFailure(reader.Get(keyset_id));
        // policy
        ReturnErrorOnFailure(reader.Next(TagPolicy()));
        ReturnErrorOnFailure(reader.Get(policy));
        // num_keys_used
        ReturnErrorOnFailure(reader.Next(TagNumKeys()));
        ReturnErrorOnFailure(reader.Get(num_keys_used));
        {
            // epoch_keys
            ReturnErrorOnFailure(reader.Next(TagEpochKeys()));
            VerifyOrReturnError(TLV::kTLVType_Array == reader.GetType(), CHIP_ERROR_INTERNAL);

            TLV::TLVType array, item;
            ReturnErrorOnFailure(reader.EnterContainer(array));
            for (auto & epoch : epoch_keys)
            {
                ReturnErrorOnFailure(reader.Next(TLV::AnonymousTag()));
                VerifyOrReturnError(TLV::kTLVType_Structure == reader.GetType(), CHIP_ERROR_INTERNAL);

                ReturnErrorOnFailure(reader.EnterContainer(item));
                // start_time
                ReturnErrorOnFailure(reader.Next(TagStartTime()));
                ReturnErrorOnFailure(reader.Get(epoch.start_time));
                // key
                ByteSpan key; // epoch.key,
                ReturnErrorOnFailure(reader.Next(TagKey()));
                ReturnErrorOnFailure(reader.Get(key));
                VerifyOrReturnError(GroupDataProvider::EpochKey::kLengthBytes == key.size(), CHIP_ERROR_INTERNAL);
                memcpy(epoch.key, key.data(), GroupDataProvider::EpochKey::kLengthBytes);
                ReturnErrorOnFailure(reader.ExitContainer(item));
            }
            ReturnErrorOnFailure(reader.ExitContainer(array));
        }
        // next keyset
        ReturnErrorOnFailure(reader.Next(TagNext()));
        ReturnErrorOnFailure(reader.Get(next));

        return reader.ExitContainer(container);
    }

    bool Find(chip::PersistentStorageDelegate & storage, const FabricData & fabric, size_t target_id)
    {
        uint16_t count = 0;

        fabric_index = fabric.fabric_index;
        keyset_id    = fabric.first_keyset;
        first        = true;

        while (count++ < fabric.keyset_count)
        {
            if (CHIP_NO_ERROR != Load(storage))
            {
                break;
            }
            if (keyset_id == target_id)
            {
                // Target id found
                return true;
            }

            first     = false;
            prev      = keyset_id;
            keyset_id = next;
        }

        return false;
    }
};

//
// General
//

constexpr size_t GroupDataProvider::GroupInfo::kGroupNameMax;
constexpr size_t GroupDataProviderImpl::kIteratorsMax;

CHIP_ERROR GroupDataProviderImpl::Init()
{
    mInitialized = true;
    return CHIP_NO_ERROR;
}

void GroupDataProviderImpl::Finish()
{
    mInitialized = false;
    mGroupInfoIterators.ReleaseAll();
    mGroupKeyIterators.ReleaseAll();
    mEndpointIterators.ReleaseAll();
    mKeySetIterators.ReleaseAll();
}

//
// Group Info
//

CHIP_ERROR GroupDataProviderImpl::SetGroupInfo(chip::FabricIndex fabric_index, const GroupInfo & info)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INTERNAL);

    FabricData fabric(fabric_index);
    GroupData group;

    // Load fabric data (defaults to zero)
    CHIP_ERROR err = fabric.Load(mStorage);
    VerifyOrReturnError(CHIP_NO_ERROR == err || CHIP_ERROR_NOT_FOUND == err, err);

    if (group.Find(mStorage, fabric, info.group_id))
    {
        // Existing group_id
        group.SetName(info.name);
        return group.Save(mStorage);
    }
    else
    {
        // New group_id
        group.group_id = info.group_id;
        group.SetName(info.name);
        return SetGroupInfoAt(fabric_index, fabric.group_count, group);
    }
}

CHIP_ERROR GroupDataProviderImpl::GetGroupInfo(chip::FabricIndex fabric_index, chip::GroupId group_id, GroupInfo & info)
{
    FabricData fabric(fabric_index);
    GroupData group;

    ReturnErrorOnFailure(fabric.Load(mStorage));
    VerifyOrReturnError(group.Find(mStorage, fabric, group_id), CHIP_ERROR_NOT_FOUND);

    info.group_id = group_id;
    info.SetName(group.name);
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupDataProviderImpl::RemoveGroupInfo(chip::FabricIndex fabric_index, chip::GroupId group_id)
{
    FabricData fabric(fabric_index);
    GroupData group;

    ReturnErrorOnFailure(fabric.Load(mStorage));
    VerifyOrReturnError(group.Find(mStorage, fabric, group_id), CHIP_ERROR_NOT_FOUND);

    return RemoveGroupInfoAt(fabric_index, group.index);
}

CHIP_ERROR GroupDataProviderImpl::SetGroupInfoAt(chip::FabricIndex fabric_index, size_t index, const GroupInfo & info)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INTERNAL);

    FabricData fabric(fabric_index);
    GroupData group;

    // Load fabric, defaults to zero
    CHIP_ERROR err = fabric.Load(mStorage);
    VerifyOrReturnError(CHIP_NO_ERROR == err || CHIP_ERROR_NOT_FOUND == err, err);

    // If the group exists, the index must match
    bool found = group.Find(mStorage, fabric, info.group_id);
    VerifyOrReturnError(!found || (group.index == index), CHIP_ERROR_DUPLICATE_KEY_ID);

    found                = group.Get(mStorage, fabric, index);
    const bool new_group = (group.group_id != info.group_id);
    group.group_id       = info.group_id;
    group.SetName(info.name);

    if (found)
    {
        // Update existing entry
        if (new_group)
        {
            // New group, clear endpoints
            RemoveEndpoints(fabric_index, group.group_id);
        }
        ReturnErrorOnFailure(group.Save(mStorage));
        if (new_group)
        {
            GroupAdded(fabric_index, group);
        }
        return CHIP_NO_ERROR;
    }

    // Insert last
    VerifyOrReturnError(fabric.group_count == index, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(fabric.group_count < mMaxGroupsPerFabric, CHIP_ERROR_INVALID_LIST_LENGTH);

    group.group_id = info.group_id;
    group.next     = 0;
    ReturnErrorOnFailure(group.Save(mStorage));

    if (group.first)
    {
        // First group, update fabric
        fabric.first_group = group.id;
    }
    else
    {
        // Last group, update previous
        GroupData prev(fabric_index, group.prev);
        ReturnErrorOnFailure(prev.Load(mStorage));
        prev.next = group.id;
        ReturnErrorOnFailure(prev.Save(mStorage));
    }
    // Update fabric
    fabric.group_count++;
    ReturnErrorOnFailure(fabric.Save(mStorage));
    GroupAdded(fabric_index, group);
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupDataProviderImpl::GetGroupInfoAt(chip::FabricIndex fabric_index, size_t index, GroupInfo & info)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INTERNAL);

    FabricData fabric(fabric_index);
    GroupData group;

    ReturnErrorOnFailure(fabric.Load(mStorage));
    VerifyOrReturnError(group.Get(mStorage, fabric, index), CHIP_ERROR_NOT_FOUND);

    // Target group found
    info.group_id = group.group_id;
    info.SetName(group.name);
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupDataProviderImpl::RemoveGroupInfoAt(chip::FabricIndex fabric_index, size_t index)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INTERNAL);

    FabricData fabric(fabric_index);
    GroupData group;

    ReturnErrorOnFailure(fabric.Load(mStorage));
    VerifyOrReturnError(group.Get(mStorage, fabric, index), CHIP_ERROR_NOT_FOUND);

    // Remove endpoints
    EndpointData endpoint(fabric_index, group.id, group.first_endpoint);
    size_t count = 0;
    while (count++ < group.endpoint_count)
    {
        if (CHIP_NO_ERROR != endpoint.Load(mStorage))
        {
            break;
        }
        endpoint.Delete(mStorage);
        endpoint.id = endpoint.next;
    }

    ReturnErrorOnFailure(group.Delete(mStorage));
    if (group.first)
    {
        // Remove first group
        fabric.first_group = group.next;
    }
    else
    {
        // Remove intermediate group, update previous
        GroupData prev_data(fabric_index, group.prev);
        ReturnErrorOnFailure(prev_data.Load(mStorage));
        prev_data.next = group.next;
        ReturnErrorOnFailure(prev_data.Save(mStorage));
    }
    if (fabric.group_count > 0)
    {
        fabric.group_count--;
    }
    // Update fabric info
    ReturnErrorOnFailure(fabric.Save(mStorage));
    if (mListener)
    {
        mListener->OnGroupRemoved(fabric_index, group);
    }
    return CHIP_NO_ERROR;
}

bool GroupDataProviderImpl::HasEndpoint(chip::FabricIndex fabric_index, chip::GroupId group_id, chip::EndpointId endpoint_id)
{
    VerifyOrReturnError(mInitialized, false);

    FabricData fabric(fabric_index);
    GroupData group;
    EndpointData endpoint;

    VerifyOrReturnError(CHIP_NO_ERROR == fabric.Load(mStorage), false);
    VerifyOrReturnError(group.Find(mStorage, fabric, group_id), false);
    return endpoint.Find(mStorage, fabric, group, endpoint_id);
}

CHIP_ERROR GroupDataProviderImpl::AddEndpoint(chip::FabricIndex fabric_index, chip::GroupId group_id, chip::EndpointId endpoint_id)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INTERNAL);

    FabricData fabric(fabric_index);
    GroupData group;

    // Load fabric data (defaults to zero)
    CHIP_ERROR err = fabric.Load(mStorage);
    VerifyOrReturnError(CHIP_NO_ERROR == err || CHIP_ERROR_NOT_FOUND == err, err);

    if (!group.Find(mStorage, fabric, group_id))
    {
        // New group
        EndpointData endpoint(fabric_index, group.id);
        endpoint.endpoint_id = endpoint_id;
        ReturnErrorOnFailure(endpoint.Save(mStorage));
        // Save the new group into the fabric
        group.group_id       = group_id;
        group.first_endpoint = endpoint.id;
        group.endpoint_count = 1;
        group.next           = fabric.first_group;
        group.prev           = kUndefinedGroupId;
        ReturnErrorOnFailure(group.Save(mStorage));
        // Update fabric
        fabric.first_group = group.id;
        fabric.group_count++;
        ReturnErrorOnFailure(fabric.Save(mStorage));
        GroupAdded(fabric_index, group);
        return CHIP_NO_ERROR;
    }

    // Existing group
    EndpointData endpoint;
    VerifyOrReturnError(!endpoint.Find(mStorage, fabric, group, endpoint_id), CHIP_NO_ERROR);

    // New endpoint, insert last
    endpoint.endpoint_id = endpoint_id;
    ReturnErrorOnFailure(endpoint.Save(mStorage));
    if (endpoint.first)
    {
        // First endpoint of group
        group.first_endpoint = endpoint.id;
    }
    else
    {
        // Previous endpoint(s)
        ReturnErrorOnFailure(endpoint.Save(mStorage));
        EndpointData prev(fabric_index, group.id, endpoint.prev);
        ReturnErrorOnFailure(prev.Load(mStorage));
        prev.next = endpoint.id;
        ReturnErrorOnFailure(prev.Save(mStorage));
    }
    group.endpoint_count++;
    return group.Save(mStorage);
}

CHIP_ERROR GroupDataProviderImpl::RemoveEndpoint(chip::FabricIndex fabric_index, chip::GroupId group_id,
                                                 chip::EndpointId endpoint_id)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INTERNAL);

    FabricData fabric(fabric_index);
    GroupData group;
    EndpointData endpoint;

    ReturnErrorOnFailure(fabric.Load(mStorage));
    VerifyOrReturnError(group.Find(mStorage, fabric, group_id), CHIP_ERROR_NOT_FOUND);
    VerifyOrReturnError(endpoint.Find(mStorage, fabric, group, endpoint_id), CHIP_ERROR_NOT_FOUND);

    // Existing endpoint
    endpoint.Delete(mStorage);

    if (endpoint.first)
    {
        // Remove first
        group.first_endpoint = endpoint.next;
    }
    else
    {
        // Remove middle
        EndpointData prev(fabric_index, group.id, endpoint.prev);
        ReturnErrorOnFailure(prev.Load(mStorage));
        prev.next = endpoint.next;
        ReturnErrorOnFailure(prev.Save(mStorage));
    }
    if (group.endpoint_count > 0)
    {
        group.endpoint_count--;
    }
    return group.Save(mStorage);
}

CHIP_ERROR GroupDataProviderImpl::RemoveEndpoint(chip::FabricIndex fabric_index, chip::EndpointId endpoint_id)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INTERNAL);

    FabricData fabric(fabric_index);

    ReturnErrorOnFailure(fabric.Load(mStorage));

    GroupData group(fabric_index, fabric.first_group);
    size_t group_index = 0;

    // Loop through all the groups
    while (group_index < fabric.group_count)
    {
        if (CHIP_NO_ERROR != group.Load(mStorage))
        {
            break;
        }
        EndpointData endpoint(fabric_index, group.id, group.first_endpoint);
        EndpointData prev_endpoint;
        size_t endpoint_index = 0;
        while (endpoint_index < group.endpoint_count)
        {
            if (CHIP_NO_ERROR != endpoint.Load(mStorage))
            {
                break;
            }
            if (endpoint.endpoint_id == endpoint_id)
            {
                // Remove endpoint from curent group
                if (0 == endpoint_index)
                {
                    // Remove first
                    group.first_endpoint = endpoint.next;
                }
                else
                {
                    prev_endpoint.next = endpoint.next;
                    ReturnErrorOnFailure(prev_endpoint.Save(mStorage));
                }
                endpoint.Delete(mStorage);
                if (group.endpoint_count > 0)
                {
                    group.endpoint_count--;
                }
                ReturnErrorOnFailure(group.Save(mStorage));
            }
            prev_endpoint = endpoint;
            endpoint.id   = endpoint.next;
            endpoint_index++;
        }
        group.id = group.next;
        group_index++;
    }

    return CHIP_NO_ERROR;
}

GroupDataProvider::GroupInfoIterator * GroupDataProviderImpl::IterateGroupInfo(chip::FabricIndex fabric_index)
{
    VerifyOrReturnError(mInitialized, nullptr);
    return mGroupInfoIterators.CreateObject(*this, fabric_index);
}

GroupDataProviderImpl::GroupInfoIteratorImpl::GroupInfoIteratorImpl(GroupDataProviderImpl & provider,
                                                                    chip::FabricIndex fabric_index) :
    mProvider(provider),
    mFabric(fabric_index)
{
    FabricData fabric(fabric_index);
    if (CHIP_NO_ERROR == fabric.Load(provider.mStorage))
    {
        mNextId = fabric.first_group;
        mTotal  = fabric.group_count;
        mCount  = 0;
    }
}

size_t GroupDataProviderImpl::GroupInfoIteratorImpl::Count()
{
    return mTotal;
}

bool GroupDataProviderImpl::GroupInfoIteratorImpl::Next(GroupInfo & output)
{
    VerifyOrReturnError(mCount < mTotal, false);

    GroupData group(mFabric, mNextId);
    VerifyOrReturnError(CHIP_NO_ERROR == group.Load(mProvider.mStorage), false);

    mCount++;
    mNextId         = group.next;
    output.group_id = group.group_id;
    output.SetName(group.name);
    return true;
}

void GroupDataProviderImpl::GroupInfoIteratorImpl::Release()
{
    mProvider.mGroupInfoIterators.ReleaseObject(this);
}

GroupDataProvider::EndpointIterator * GroupDataProviderImpl::IterateEndpoints(chip::FabricIndex fabric_index)
{
    VerifyOrReturnError(mInitialized, nullptr);
    return mEndpointIterators.CreateObject(*this, fabric_index);
}

GroupDataProviderImpl::EndpointIteratorImpl::EndpointIteratorImpl(GroupDataProviderImpl & provider,
                                                                  chip::FabricIndex fabric_index) :
    mProvider(provider),
    mFabric(fabric_index)
{
    FabricData fabric(fabric_index);
    VerifyOrReturn(CHIP_NO_ERROR == fabric.Load(provider.mStorage));

    GroupData group(fabric_index, fabric.first_group);
    VerifyOrReturn(CHIP_NO_ERROR == group.Load(provider.mStorage));

    mGroup         = fabric.first_group;
    mFirstGroup    = fabric.first_group;
    mGroupCount    = fabric.group_count;
    mEndpoint      = group.first_endpoint;
    mEndpointCount = group.endpoint_count;
}

size_t GroupDataProviderImpl::EndpointIteratorImpl::Count()
{
    GroupData group(mFabric, mFirstGroup);
    size_t group_index    = 0;
    size_t endpoint_index = 0;
    size_t count          = 0;

    while (group_index++ < mGroupCount)
    {
        if (CHIP_NO_ERROR != group.Load(mProvider.mStorage))
        {
            break;
        }
        EndpointData endpoint_data(mFabric, group.id, group.first_endpoint);
        while (endpoint_index++ < group.endpoint_count)
        {
            if (CHIP_NO_ERROR != endpoint_data.Load(mProvider.mStorage))
            {
                break;
            }
            endpoint_data.id = endpoint_data.next;
            count++;
        }
        group.id       = group.next;
        endpoint_index = 0;
    }
    return count;
}

bool GroupDataProviderImpl::EndpointIteratorImpl::Next(GroupEndpoint & output)
{
    while (mGroupIndex < mGroupCount)
    {
        GroupData group(mFabric, mGroup);
        if (CHIP_NO_ERROR != group.Load(mProvider.mStorage))
        {
            mGroupIndex = mGroupCount;
            return false;
        }
        if (mEndpointIndex < mEndpointCount)
        {
            EndpointData endpoint_data(mFabric, mGroup, mEndpoint);
            if (CHIP_NO_ERROR == endpoint_data.Load(mProvider.mStorage))
            {
                output.group_id    = group.group_id;
                output.endpoint_id = endpoint_data.endpoint_id;
                mEndpoint          = endpoint_data.next;
                mEndpointIndex++;
                return true;
            }
        }

        mGroup = group.next;
        mGroupIndex++;
        mEndpoint      = group.first_endpoint;
        mEndpointIndex = 0;
        mEndpointCount = group.endpoint_count;
    }
    return false;
}

void GroupDataProviderImpl::EndpointIteratorImpl::Release()
{
    mProvider.mEndpointIterators.ReleaseObject(this);
}

CHIP_ERROR GroupDataProviderImpl::RemoveEndpoints(chip::FabricIndex fabric_index, chip::GroupId group_id)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INTERNAL);

    FabricData fabric(fabric_index);
    GroupData group;

    VerifyOrReturnError(CHIP_NO_ERROR == fabric.Load(mStorage), CHIP_ERROR_INVALID_FABRIC_ID);
    VerifyOrReturnError(group.Find(mStorage, fabric, group_id), CHIP_ERROR_KEY_NOT_FOUND);

    EndpointData endpoint(fabric_index, group.id, group.first_endpoint);
    size_t endpoint_index = 0;
    while (endpoint_index < group.endpoint_count)
    {
        ReturnErrorOnFailure(endpoint.Load(mStorage));
        endpoint.Delete(mStorage);
        endpoint.id = endpoint.next;
        endpoint_index++;
    }
    group.first_endpoint = kInvalidEndpointId;
    group.endpoint_count = 0;
    ReturnErrorOnFailure(group.Save(mStorage));

    return CHIP_NO_ERROR;
}

//
// Group-Key map
//

CHIP_ERROR GroupDataProviderImpl::SetGroupKeyAt(chip::FabricIndex fabric_index, size_t index, const GroupKey & in_map)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INTERNAL);

    FabricData fabric(fabric_index);
    KeyMapData map(fabric_index);

    // Load fabric, defaults to zero
    CHIP_ERROR err = fabric.Load(mStorage);
    VerifyOrReturnError(CHIP_NO_ERROR == err || CHIP_ERROR_NOT_FOUND == err, err);

    // If the group exists, the index must match
    bool found = map.Find(mStorage, fabric, in_map);
    VerifyOrReturnError(!found || (map.index == index), CHIP_ERROR_DUPLICATE_KEY_ID);

    found         = map.Get(mStorage, fabric, index);
    map.group_id  = in_map.group_id;
    map.keyset_id = in_map.keyset_id;

    if (found)
    {
        // Update existing map
        return map.Save(mStorage);
    }

    // Insert last
    VerifyOrReturnError(fabric.map_count == index, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(fabric.map_count < mMaxGroupKeysPerFabric, CHIP_ERROR_INVALID_LIST_LENGTH);

    map.next = 0;
    ReturnErrorOnFailure(map.Save(mStorage));

    if (map.first)
    {
        // First map, update fabric
        fabric.first_map = map.id;
    }
    else
    {
        // Last map, update previous
        KeyMapData prev(fabric_index, map.prev);
        ReturnErrorOnFailure(prev.Load(mStorage));
        prev.next = map.id;
        ReturnErrorOnFailure(prev.Save(mStorage));
    }
    // Update fabric
    fabric.map_count++;
    return fabric.Save(mStorage);
}

CHIP_ERROR GroupDataProviderImpl::GetGroupKeyAt(chip::FabricIndex fabric_index, size_t index, GroupKey & out_map)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INTERNAL);

    FabricData fabric(fabric_index);
    KeyMapData map;

    ReturnErrorOnFailure(fabric.Load(mStorage));
    VerifyOrReturnError(map.Get(mStorage, fabric, index), CHIP_ERROR_NOT_FOUND);

    // Target map found
    out_map.group_id  = map.group_id;
    out_map.keyset_id = map.keyset_id;
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupDataProviderImpl::RemoveGroupKeyAt(chip::FabricIndex fabric_index, size_t index)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INTERNAL);

    FabricData fabric(fabric_index);
    KeyMapData map;

    ReturnErrorOnFailure(fabric.Load(mStorage));
    VerifyOrReturnError(map.Get(mStorage, fabric, index), CHIP_ERROR_NOT_FOUND);

    ReturnErrorOnFailure(map.Delete(mStorage));
    if (map.first)
    {
        // Remove first map
        fabric.first_map = map.next;
    }
    else
    {
        // Remove intermediate map, update previous
        KeyMapData prev_data(fabric_index, map.prev);
        ReturnErrorOnFailure(prev_data.Load(mStorage));
        prev_data.next = map.next;
        ReturnErrorOnFailure(prev_data.Save(mStorage));
    }
    if (fabric.map_count > 0)
    {
        fabric.map_count--;
    }
    // Update fabric
    return fabric.Save(mStorage);
}

CHIP_ERROR GroupDataProviderImpl::RemoveGroupKeys(chip::FabricIndex fabric_index)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INTERNAL);

    FabricData fabric(fabric_index);
    VerifyOrReturnError(CHIP_NO_ERROR == fabric.Load(mStorage), CHIP_ERROR_INVALID_FABRIC_ID);

    size_t count = 0;
    KeyMapData map(fabric_index, fabric.first_map);
    while (count++ < fabric.map_count)
    {
        if (CHIP_NO_ERROR != map.Load(mStorage))
        {
            break;
        }
        map.Delete(mStorage);
        map.id = map.next;
    }

    // Update fabric
    fabric.first_map = 0;
    fabric.map_count = 0;
    return fabric.Save(mStorage);
}

GroupDataProvider::GroupKeyIterator * GroupDataProviderImpl::IterateGroupKeys(chip::FabricIndex fabric_index)
{
    VerifyOrReturnError(mInitialized, nullptr);
    return mGroupKeyIterators.CreateObject(*this, fabric_index);
}

GroupDataProviderImpl::GroupKeyIteratorImpl::GroupKeyIteratorImpl(GroupDataProviderImpl & provider,
                                                                  chip::FabricIndex fabric_index) :
    mProvider(provider),
    mFabric(fabric_index)
{
    FabricData fabric(fabric_index);
    if (CHIP_NO_ERROR == fabric.Load(provider.mStorage))
    {
        mNextId = fabric.first_map;
        mTotal  = fabric.map_count;
        mCount  = 0;
    }
}

size_t GroupDataProviderImpl::GroupKeyIteratorImpl::Count()
{
    return mTotal;
}

bool GroupDataProviderImpl::GroupKeyIteratorImpl::Next(GroupKey & output)
{
    VerifyOrReturnError(mCount < mTotal, false);

    KeyMapData map(mFabric, mNextId);
    VerifyOrReturnError(CHIP_NO_ERROR == map.Load(mProvider.mStorage), false);

    mCount++;
    mNextId          = map.next;
    output.group_id  = map.group_id;
    output.keyset_id = map.keyset_id;
    return true;
}

void GroupDataProviderImpl::GroupKeyIteratorImpl::Release()
{
    mProvider.mGroupKeyIterators.ReleaseObject(this);
}

//
// Key Sets
//

constexpr size_t GroupDataProvider::EpochKey::kLengthBytes;

CHIP_ERROR GroupDataProviderImpl::SetKeySet(chip::FabricIndex fabric_index, const KeySet & in_keyset)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INTERNAL);

    FabricData fabric(fabric_index);
    KeySetData keyset;

    // Load fabric, defaults to zero
    CHIP_ERROR err = fabric.Load(mStorage);
    VerifyOrReturnError(CHIP_NO_ERROR == err || CHIP_ERROR_NOT_FOUND == err, err);

    // Search existing keyset
    bool found = keyset.Find(mStorage, fabric, in_keyset.keyset_id);

    keyset.keyset_id     = in_keyset.keyset_id;
    keyset.policy        = in_keyset.policy;
    keyset.num_keys_used = in_keyset.num_keys_used;
    memcpy(keyset.epoch_keys, in_keyset.epoch_keys, sizeof(in_keyset.epoch_keys));

    if (found)
    {
        // Update existing keyset info, keep next
        return keyset.Save(mStorage);
    }
    else
    {
        // New keyset, insert first
        keyset.next = fabric.first_keyset;
        ReturnErrorOnFailure(keyset.Save(mStorage));
        // Update fabric
        fabric.keyset_count++;
        fabric.first_keyset = in_keyset.keyset_id;
        return fabric.Save(mStorage);
    }
}

CHIP_ERROR GroupDataProviderImpl::GetKeySet(chip::FabricIndex fabric_index, uint16_t target_id, KeySet & out_keyset)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INTERNAL);

    FabricData fabric(fabric_index);
    KeySetData keyset;

    ReturnErrorOnFailure(fabric.Load(mStorage));
    VerifyOrReturnError(keyset.Find(mStorage, fabric, target_id), CHIP_ERROR_NOT_FOUND);

    VerifyOrReturnError(keyset.Find(mStorage, fabric, target_id), CHIP_ERROR_NOT_FOUND);

    // Target keyset found
    out_keyset.policy        = keyset.policy;
    out_keyset.num_keys_used = keyset.num_keys_used;
    memcpy(out_keyset.epoch_keys, keyset.epoch_keys, sizeof(out_keyset.epoch_keys));
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupDataProviderImpl::RemoveKeySet(chip::FabricIndex fabric_index, uint16_t target_id)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INTERNAL);

    FabricData fabric(fabric_index);
    KeySetData keyset;

    ReturnErrorOnFailure(fabric.Load(mStorage));
    VerifyOrReturnError(keyset.Find(mStorage, fabric, target_id), CHIP_ERROR_NOT_FOUND);
    ReturnErrorOnFailure(keyset.Delete(mStorage));

    if (keyset.first)
    {
        // Remove first keyset
        fabric.first_keyset = keyset.next;
    }
    else
    {
        // Remove intermediate keyset, update previous
        KeySetData prev_data(fabric_index, keyset.prev);
        ReturnErrorOnFailure(prev_data.Load(mStorage));
        prev_data.next = keyset.next;
        ReturnErrorOnFailure(prev_data.Save(mStorage));
    }
    if (fabric.keyset_count > 0)
    {
        fabric.keyset_count--;
    }
    // Update fabric info
    return fabric.Save(mStorage);
}

GroupDataProvider::KeySetIterator * GroupDataProviderImpl::IterateKeySets(chip::FabricIndex fabric_index)
{
    VerifyOrReturnError(mInitialized, nullptr);
    return mKeySetIterators.CreateObject(*this, fabric_index);
}

GroupDataProviderImpl::KeySetIteratorImpl::KeySetIteratorImpl(GroupDataProviderImpl & provider, chip::FabricIndex fabric_index) :
    mProvider(provider), mFabric(fabric_index)
{
    FabricData fabric(fabric_index);
    if (CHIP_NO_ERROR == fabric.Load(provider.mStorage))
    {
        mNextId = fabric.first_keyset;
        mTotal  = fabric.keyset_count;
        mCount  = 0;
    }
}

size_t GroupDataProviderImpl::KeySetIteratorImpl::Count()
{
    return mTotal;
}

bool GroupDataProviderImpl::KeySetIteratorImpl::Next(KeySet & output)
{
    VerifyOrReturnError(mCount < mTotal, false);

    KeySetData keyset(mFabric, mNextId);
    VerifyOrReturnError(CHIP_NO_ERROR == keyset.Load(mProvider.mStorage), false);

    mCount++;
    mNextId              = keyset.next;
    output.keyset_id     = keyset.keyset_id;
    output.policy        = keyset.policy;
    output.num_keys_used = keyset.num_keys_used;
    memcpy(output.epoch_keys, keyset.epoch_keys, sizeof(output.epoch_keys));
    return true;
}

void GroupDataProviderImpl::KeySetIteratorImpl::Release()
{
    mProvider.mKeySetIterators.ReleaseObject(this);
}

//
// Fabrics
//

CHIP_ERROR GroupDataProviderImpl::RemoveFabric(chip::FabricIndex fabric_index)
{
    FabricData fabric(fabric_index);

    // Fabric data defaults to zero, so if not entry is found, no mappings, or keys are removed
    // However, states has a separate list, and needs to be removed regardless
    CHIP_ERROR err = fabric.Load(mStorage);
    VerifyOrReturnError(CHIP_NO_ERROR == err || CHIP_ERROR_NOT_FOUND == err, err);

    // Remove Group mappings

    for (size_t i = 0; i < fabric.map_count; i++)
    {
        RemoveGroupKeyAt(fabric_index, fabric.map_count - i - 1);
    }

    // Remove group info

    for (size_t i = 0; i < fabric.group_count; i++)
    {
        RemoveGroupInfoAt(fabric_index, fabric.group_count - i - 1);
    }

    // Remove Keysets

    KeySetData keyset(fabric_index, fabric.first_keyset);
    size_t keyset_count = 0;

    // Loop the keysets associated with the target fabric
    while (keyset_count < fabric.keyset_count)
    {
        if (CHIP_NO_ERROR != keyset.Load(mStorage))
        {
            break;
        }
        RemoveKeySet(fabric_index, keyset.keyset_id);
        keyset.keyset_id = keyset.next;
        keyset_count++;
    }

    // Remove fabric
    return fabric.Delete(mStorage);
}

//
// General
//

CHIP_ERROR GroupDataProviderImpl::Decrypt(PacketHeader packetHeader, PayloadHeader & payloadHeader,
                                          System::PacketBufferHandle & msg)
{
    (void) packetHeader;
    (void) payloadHeader;
    (void) msg;
    return CHIP_NO_ERROR;
}

namespace {

GroupDataProvider * gGroupsProvider = nullptr;

} // namespace

/**
 * Instance getter for the global GroupDataProvider.
 *
 * Callers have to externally synchronize usage of this function.
 *
 * @return The global device attestation credentials provider. Assume never null.
 */
GroupDataProvider * GetGroupDataProvider()
{
    return gGroupsProvider;
}

/**
 * Instance setter for the global GroupDataProvider.
 *
 * Callers have to externally synchronize usage of this function.
 *
 * If the `provider` is nullptr, no change is done.
 *
 * @param[in] provider the GroupDataProvider to start returning with the getter
 */
void SetGroupDataProvider(GroupDataProvider * provider)
{
    if (provider)
    {
        gGroupsProvider = provider;
    }
}

} // namespace Credentials
} // namespace chip
