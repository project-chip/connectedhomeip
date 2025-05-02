/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/CommonPersistentData.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/PersistentData.h>
#include <lib/support/Pool.h>
#include <stdlib.h>

namespace chip {
namespace Credentials {

using GroupInfo     = GroupDataProvider::GroupInfo;
using GroupKey      = GroupDataProvider::GroupKey;
using GroupEndpoint = GroupDataProvider::GroupEndpoint;
using EpochKey      = GroupDataProvider::EpochKey;
using KeySet        = GroupDataProvider::KeySet;
using GroupSession  = GroupDataProvider::GroupSession;

struct FabricList : public CommonPersistentData::FabricList
{
    CHIP_ERROR UpdateKey(StorageKeyName & key) override
    {
        key = DefaultStorageKeyAllocator::GroupFabricList();
        return CHIP_NO_ERROR;
    }
};

constexpr size_t kPersistentBufferMax = 128;

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
    static constexpr TLV::Tag TagNext() { return TLV::ContextTag(7); }

    chip::FabricIndex fabric_index = kUndefinedFabricIndex;
    chip::GroupId first_group      = kUndefinedGroupId;
    uint16_t group_count           = 0;
    uint16_t first_map             = 0;
    uint16_t map_count             = 0;
    chip::KeysetId first_keyset    = kInvalidKeysetId;
    uint16_t keyset_count          = 0;
    chip::FabricIndex next         = kUndefinedFabricIndex;

    FabricData() = default;
    FabricData(chip::FabricIndex fabric) : fabric_index(fabric) {}

    CHIP_ERROR UpdateKey(StorageKeyName & key) override
    {
        VerifyOrReturnError(kUndefinedFabricIndex != fabric_index, CHIP_ERROR_INVALID_FABRIC_INDEX);
        key = DefaultStorageKeyAllocator::FabricGroups(fabric_index);
        return CHIP_NO_ERROR;
    }

    void Clear() override
    {
        first_group  = kUndefinedGroupId;
        group_count  = 0;
        first_keyset = kInvalidKeysetId;
        keyset_count = 0;
        next         = kUndefinedFabricIndex;
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
        ReturnErrorOnFailure(writer.Put(TagNext(), static_cast<uint16_t>(next)));

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
        // next
        ReturnErrorOnFailure(reader.Next(TagNext()));
        ReturnErrorOnFailure(reader.Get(next));

        return reader.ExitContainer(container);
    }

    // Register the fabric in the fabrics' linked-list
    CHIP_ERROR Register(PersistentStorageDelegate * storage)
    {
        FabricList fabric_list;
        CHIP_ERROR err = fabric_list.Load(storage);
        if (CHIP_ERROR_NOT_FOUND == err)
        {
            // New fabric list
            fabric_list.first_entry = fabric_index;
            fabric_list.entry_count = 1;
            return fabric_list.Save(storage);
        }
        ReturnErrorOnFailure(err);

        // Existing fabric list, search for existing entry
        FabricData fabric(fabric_list.first_entry);
        for (size_t i = 0; i < fabric_list.entry_count; i++)
        {
            err = fabric.Load(storage);
            if (CHIP_NO_ERROR != err)
            {
                break;
            }
            if (fabric.fabric_index == this->fabric_index)
            {
                // Fabric already registered
                return CHIP_NO_ERROR;
            }
            fabric.fabric_index = fabric.next;
        }
        // Add this fabric to the fabric list
        this->next              = fabric_list.first_entry;
        fabric_list.first_entry = this->fabric_index;
        fabric_list.entry_count++;
        return fabric_list.Save(storage);
    }

    // Remove the fabric from the fabrics' linked list
    CHIP_ERROR Unregister(PersistentStorageDelegate * storage) const
    {
        FabricList fabric_list;
        CHIP_ERROR err = fabric_list.Load(storage);
        VerifyOrReturnError(CHIP_NO_ERROR == err || CHIP_ERROR_NOT_FOUND == err, err);

        // Existing fabric list, search for existing entry
        FabricData fabric(fabric_list.first_entry);
        FabricData prev;

        for (size_t i = 0; i < fabric_list.entry_count; i++)
        {
            err = fabric.Load(storage);
            if (CHIP_NO_ERROR != err)
            {
                break;
            }
            if (fabric.fabric_index == this->fabric_index)
            {
                // Fabric found
                if (i == 0)
                {
                    // Remove first fabric
                    fabric_list.first_entry = this->next;
                }
                else
                {
                    // Remove intermediate fabric
                    prev.next = this->next;
                    ReturnErrorOnFailure(prev.Save(storage));
                }
                VerifyOrReturnError(fabric_list.entry_count > 0, CHIP_ERROR_INTERNAL);
                fabric_list.entry_count--;
                return fabric_list.Save(storage);
            }
            prev                = fabric;
            fabric.fabric_index = fabric.next;
        }
        // Fabric not in the list
        return CHIP_ERROR_NOT_FOUND;
    }

    // Check the fabric is registered in the fabrics' linked list
    CHIP_ERROR Validate(PersistentStorageDelegate * storage) const
    {
        FabricList fabric_list;
        ReturnErrorOnFailure(fabric_list.Load(storage));

        // Existing fabric list, search for existing entry
        FabricData fabric(fabric_list.first_entry);

        for (size_t i = 0; i < fabric_list.entry_count; i++)
        {
            ReturnErrorOnFailure(fabric.Load(storage));
            if (fabric.fabric_index == this->fabric_index)
            {
                return CHIP_NO_ERROR;
            }
            fabric.fabric_index = fabric.next;
        }
        // Fabric not in the list
        return CHIP_ERROR_NOT_FOUND;
    }

    CHIP_ERROR Save(PersistentStorageDelegate * storage) override
    {
        ReturnErrorOnFailure(Register(storage));
        return PersistentData::Save(storage);
    }

    CHIP_ERROR Delete(PersistentStorageDelegate * storage) override
    {
        ReturnErrorOnFailure(Unregister(storage));
        return PersistentData::Delete(storage);
    }
};

struct GroupData : public GroupDataProvider::GroupInfo, PersistentData<kPersistentBufferMax>
{
    static constexpr TLV::Tag TagName() { return TLV::ContextTag(1); }
    static constexpr TLV::Tag TagFirstEndpoint() { return TLV::ContextTag(2); }
    static constexpr TLV::Tag TagEndpointCount() { return TLV::ContextTag(3); }
    static constexpr TLV::Tag TagNext() { return TLV::ContextTag(4); }

    chip::FabricIndex fabric_index  = kUndefinedFabricIndex;
    chip::EndpointId first_endpoint = kInvalidEndpointId;
    uint16_t endpoint_count         = 0;
    uint16_t index                  = 0;
    chip::GroupId next              = 0;
    chip::GroupId prev              = 0;
    bool first                      = true;

    GroupData() : GroupInfo(nullptr){};
    GroupData(chip::FabricIndex fabric) : fabric_index(fabric) {}
    GroupData(chip::FabricIndex fabric, chip::GroupId group) : GroupInfo(group, nullptr), fabric_index(fabric) {}

    CHIP_ERROR UpdateKey(StorageKeyName & key) override
    {
        VerifyOrReturnError(kUndefinedFabricIndex != fabric_index, CHIP_ERROR_INVALID_FABRIC_INDEX);
        key = DefaultStorageKeyAllocator::FabricGroup(fabric_index, group_id);
        return CHIP_NO_ERROR;
    }

    void Clear() override
    {
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

    bool Get(PersistentStorageDelegate * storage, const FabricData & fabric, size_t target_index)
    {
        fabric_index = fabric.fabric_index;
        group_id     = fabric.first_group;
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

            first    = false;
            prev     = group_id;
            group_id = next;
            index++;
        }

        return false;
    }

    bool Find(PersistentStorageDelegate * storage, const FabricData & fabric, chip::GroupId target_group)
    {
        fabric_index = fabric.fabric_index;
        group_id     = fabric.first_group;
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
            first    = false;
            prev     = group_id;
            group_id = next;
            index++;
        }
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

    KeyMapData(){};
    KeyMapData(chip::FabricIndex fabric, uint16_t link_id = 0, chip::GroupId group = kUndefinedGroupId, chip::KeysetId keyset = 0) :
        GroupKey(group, keyset), LinkedData(link_id), fabric_index(fabric)
    {}

    CHIP_ERROR UpdateKey(StorageKeyName & key) override
    {
        VerifyOrReturnError(kUndefinedFabricIndex != fabric_index, CHIP_ERROR_INVALID_FABRIC_INDEX);
        key = DefaultStorageKeyAllocator::FabricGroupKey(fabric_index, id);
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

    bool Get(PersistentStorageDelegate * storage, const FabricData & fabric, size_t target_index)
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

    bool Find(PersistentStorageDelegate * storage, const FabricData & fabric, const GroupKey & map)
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

    // returns index if the find_id is found, otherwise std::numeric_limits<size_t>::max
    size_t Find(PersistentStorageDelegate * storage, const FabricData & fabric, const KeysetId find_id)
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
            if (keyset_id == find_id)
            {
                // Match found
                return index;
            }
            max_id = std::max(id, max_id);
            first  = false;
            prev   = id;
            id     = next;
            index++;
        }

        id = static_cast<uint16_t>(max_id + 1);
        return std::numeric_limits<size_t>::max();
    }
};

struct EndpointData : GroupDataProvider::GroupEndpoint, PersistentData<kPersistentBufferMax>
{
    static constexpr TLV::Tag TagEndpoint() { return TLV::ContextTag(1); }
    static constexpr TLV::Tag TagNext() { return TLV::ContextTag(2); }

    chip::FabricIndex fabric_index = kUndefinedFabricIndex;
    uint16_t index                 = 0;
    chip::EndpointId next          = 0;
    chip::EndpointId prev          = 0;
    bool first                     = true;

    EndpointData() = default;
    EndpointData(chip::FabricIndex fabric, chip::GroupId group = kUndefinedGroupId,
                 chip::EndpointId endpoint = kInvalidEndpointId) :
        GroupEndpoint(group, endpoint),
        fabric_index(fabric)
    {}

    CHIP_ERROR UpdateKey(StorageKeyName & key) override
    {
        VerifyOrReturnError(kUndefinedFabricIndex != fabric_index, CHIP_ERROR_INVALID_FABRIC_INDEX);
        key = DefaultStorageKeyAllocator::FabricGroupEndpoint(fabric_index, group_id, endpoint_id);
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

    bool Find(PersistentStorageDelegate * storage, const FabricData & fabric, const GroupData & group, chip::EndpointId target_id)
    {
        fabric_index = fabric.fabric_index;
        group_id     = group.group_id;
        endpoint_id  = group.first_endpoint;
        index        = 0;
        first        = true;

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

            first       = false;
            prev        = endpoint_id;
            endpoint_id = next;
            index++;
        }

        return false;
    }
};

struct KeySetData : PersistentData<kPersistentBufferMax>
{
    static constexpr TLV::Tag TagPolicy() { return TLV::ContextTag(1); }
    static constexpr TLV::Tag TagNumKeys() { return TLV::ContextTag(2); }
    static constexpr TLV::Tag TagGroupCredentials() { return TLV::ContextTag(3); }
    static constexpr TLV::Tag TagStartTime() { return TLV::ContextTag(4); }
    static constexpr TLV::Tag TagKeyHash() { return TLV::ContextTag(5); }
    static constexpr TLV::Tag TagKeyValue() { return TLV::ContextTag(6); }
    static constexpr TLV::Tag TagNext() { return TLV::ContextTag(7); }

    chip::FabricIndex fabric_index = kUndefinedFabricIndex;
    chip::KeysetId next            = kInvalidKeysetId;
    chip::KeysetId prev            = kInvalidKeysetId;
    bool first                     = true;

    uint16_t keyset_id                       = 0;
    GroupDataProvider::SecurityPolicy policy = GroupDataProvider::SecurityPolicy::kCacheAndSync;
    uint8_t keys_count                       = 0;
    Crypto::GroupOperationalCredentials operational_keys[KeySet::kEpochKeysMax];

    KeySetData() = default;
    KeySetData(chip::FabricIndex fabric, chip::KeysetId id) : fabric_index(fabric) { keyset_id = id; }
    KeySetData(chip::FabricIndex fabric, chip::KeysetId id, GroupDataProvider::SecurityPolicy policy_id, uint8_t num_keys) :
        fabric_index(fabric), keyset_id(id), policy(policy_id), keys_count(num_keys)
    {}

    CHIP_ERROR UpdateKey(StorageKeyName & key) override
    {
        VerifyOrReturnError(kUndefinedFabricIndex != fabric_index, CHIP_ERROR_INVALID_FABRIC_INDEX);
        VerifyOrReturnError(kInvalidKeysetId != keyset_id, CHIP_ERROR_INVALID_KEY_ID);
        key = DefaultStorageKeyAllocator::FabricKeyset(fabric_index, keyset_id);
        return CHIP_NO_ERROR;
    }

    void Clear() override
    {
        policy     = GroupDataProvider::SecurityPolicy::kCacheAndSync;
        keys_count = 0;
        memset(operational_keys, 0x00, sizeof(operational_keys));
        next = kInvalidKeysetId;
    }

    Crypto::GroupOperationalCredentials * GetCurrentGroupCredentials()
    {
        // An epoch key update SHALL order the keys from oldest to newest,
        // the current epoch key having the second newest time if time
        // synchronization is not achieved or guaranteed.
        switch (this->keys_count)
        {
        case 1:
        case 2:
            return &operational_keys[0];
        case 3:
            return &operational_keys[1];
        default:
            return nullptr;
        }
    }

    CHIP_ERROR Serialize(TLV::TLVWriter & writer) const override
    {
        TLV::TLVType container;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, container));

        // policy
        ReturnErrorOnFailure(writer.Put(TagPolicy(), static_cast<uint16_t>(policy)));
        // keys_count
        ReturnErrorOnFailure(writer.Put(TagNumKeys(), static_cast<uint16_t>(keys_count)));
        // operational_keys
        {
            TLV::TLVType array, item;
            ReturnErrorOnFailure(writer.StartContainer(TagGroupCredentials(), TLV::kTLVType_Array, array));
            uint8_t keyCount   = 0;
            uint64_t startTime = 0;
            uint16_t hash      = 0;
            uint8_t encryptionKey[Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES];
            for (auto & key : operational_keys)
            {
                startTime = 0;
                hash      = 0;
                memset(encryptionKey, 0, sizeof(encryptionKey));
                ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, item));

                if (keyCount++ < keys_count)
                {
                    startTime = key.start_time;
                    hash      = key.hash;
                    memcpy(encryptionKey, key.encryption_key, sizeof(encryptionKey));
                }
                ReturnErrorOnFailure(writer.Put(TagStartTime(), static_cast<uint64_t>(startTime)));
                ReturnErrorOnFailure(writer.Put(TagKeyHash(), hash));
                ReturnErrorOnFailure(writer.Put(TagKeyValue(), ByteSpan(encryptionKey)));

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

        // policy
        ReturnErrorOnFailure(reader.Next(TagPolicy()));
        ReturnErrorOnFailure(reader.Get(policy));
        // keys_count
        ReturnErrorOnFailure(reader.Next(TagNumKeys()));
        ReturnErrorOnFailure(reader.Get(keys_count));
        // TODO(#21614): Enforce maximum number of 3 keys in a keyset
        {
            // operational_keys
            ReturnErrorOnFailure(reader.Next(TagGroupCredentials()));
            VerifyOrReturnError(TLV::kTLVType_Array == reader.GetType(), CHIP_ERROR_INTERNAL);

            TLV::TLVType array, item;
            ReturnErrorOnFailure(reader.EnterContainer(array));
            for (auto & key : operational_keys)
            {
                ReturnErrorOnFailure(reader.Next(TLV::AnonymousTag()));
                VerifyOrReturnError(TLV::kTLVType_Structure == reader.GetType(), CHIP_ERROR_INTERNAL);

                ReturnErrorOnFailure(reader.EnterContainer(item));
                // start_time
                ReturnErrorOnFailure(reader.Next(TagStartTime()));
                ReturnErrorOnFailure(reader.Get(key.start_time));
                // key hash
                ReturnErrorOnFailure(reader.Next(TagKeyHash()));
                ReturnErrorOnFailure(reader.Get(key.hash));
                // key value
                ByteSpan encryption_key;
                ReturnErrorOnFailure(reader.Next(TagKeyValue()));
                ReturnErrorOnFailure(reader.Get(encryption_key));
                VerifyOrReturnError(Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES == encryption_key.size(), CHIP_ERROR_INTERNAL);
                memcpy(key.encryption_key, encryption_key.data(), encryption_key.size());
                // Re-derive privacy key from encryption key when loading from storage to save on storage size.
                MutableByteSpan privacy_key(key.privacy_key);
                ReturnErrorOnFailure(Crypto::DeriveGroupPrivacyKey(encryption_key, privacy_key));
                ReturnErrorOnFailure(reader.ExitContainer(item));
            }
            ReturnErrorOnFailure(reader.ExitContainer(array));
        }
        // next keyset
        ReturnErrorOnFailure(reader.Next(TagNext()));
        ReturnErrorOnFailure(reader.Get(next));

        return reader.ExitContainer(container);
    }

    bool Find(PersistentStorageDelegate * storage, const FabricData & fabric, size_t target_id)
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
    if (mStorage == nullptr || mSessionKeystore == nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return CHIP_NO_ERROR;
}

void GroupDataProviderImpl::Finish()
{
    mGroupInfoIterators.ReleaseAll();
    mGroupKeyIterators.ReleaseAll();
    mEndpointIterators.ReleaseAll();
    mKeySetIterators.ReleaseAll();
    mGroupSessionsIterator.ReleaseAll();
    mGroupKeyContexPool.ReleaseAll();
}

void GroupDataProviderImpl::SetStorageDelegate(PersistentStorageDelegate * storage)
{
    VerifyOrDie(storage != nullptr);
    mStorage = storage;
}

//
// Group Info
//

CHIP_ERROR GroupDataProviderImpl::SetGroupInfo(chip::FabricIndex fabric_index, const GroupInfo & info)
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INTERNAL);

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

    // New group_id
    group.group_id = info.group_id;
    group.SetName(info.name);
    return SetGroupInfoAt(fabric_index, fabric.group_count, group);
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
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INTERNAL);

    FabricData fabric(fabric_index);
    GroupData group;

    // Load fabric, defaults to zero
    CHIP_ERROR err = fabric.Load(mStorage);
    VerifyOrReturnError(CHIP_NO_ERROR == err || CHIP_ERROR_NOT_FOUND == err, err);

    // If the group exists, the index must match
    bool found = group.Find(mStorage, fabric, info.group_id);
    VerifyOrReturnError(!found || (group.index == index), CHIP_ERROR_DUPLICATE_KEY_ID);

    group.group_id       = info.group_id;
    group.endpoint_count = 0;
    group.SetName(info.name);

    if (found)
    {
        // Update existing entry
        return group.Save(mStorage);
    }
    if (index < fabric.group_count)
    {
        // Replace existing entry with a new group
        GroupData old_group;
        old_group.Get(mStorage, fabric, index);
        group.first = old_group.first;
        group.prev  = old_group.prev;
        group.next  = old_group.next;

        ReturnErrorOnFailure(RemoveEndpoints(fabric_index, old_group.group_id));
        ReturnErrorOnFailure(old_group.Delete(mStorage));
        GroupRemoved(fabric_index, old_group);
    }
    else
    {
        // Insert last
        VerifyOrReturnError(fabric.group_count == index, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(fabric.group_count < mMaxGroupsPerFabric, CHIP_ERROR_INVALID_LIST_LENGTH);
        fabric.group_count++;
    }

    ReturnErrorOnFailure(group.Save(mStorage));

    if (group.first)
    {
        // First group, update fabric
        fabric.first_group = group.group_id;
    }
    else
    {
        // Second to last group, update previous
        GroupData prev(fabric_index, group.prev);
        ReturnErrorOnFailure(prev.Load(mStorage));
        prev.next = group.group_id;
        ReturnErrorOnFailure(prev.Save(mStorage));
    }
    // Update fabric
    ReturnErrorOnFailure(fabric.Save(mStorage));
    GroupAdded(fabric_index, group);
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupDataProviderImpl::GetGroupInfoAt(chip::FabricIndex fabric_index, size_t index, GroupInfo & info)
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INTERNAL);

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
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INTERNAL);

    FabricData fabric(fabric_index);
    GroupData group;

    ReturnErrorOnFailure(fabric.Load(mStorage));
    VerifyOrReturnError(group.Get(mStorage, fabric, index), CHIP_ERROR_NOT_FOUND);

    // Remove endpoints
    EndpointData endpoint(fabric_index, group.group_id, group.first_endpoint);
    size_t count = 0;
    while (count++ < group.endpoint_count)
    {
        if (CHIP_NO_ERROR != endpoint.Load(mStorage))
        {
            break;
        }
        endpoint.Delete(mStorage);
        endpoint.endpoint_id = endpoint.next;
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
    GroupRemoved(fabric_index, group);
    return CHIP_NO_ERROR;
}

bool GroupDataProviderImpl::HasEndpoint(chip::FabricIndex fabric_index, chip::GroupId group_id, chip::EndpointId endpoint_id)
{
    VerifyOrReturnError(IsInitialized(), false);

    FabricData fabric(fabric_index);
    GroupData group;
    EndpointData endpoint;

    VerifyOrReturnError(CHIP_NO_ERROR == fabric.Load(mStorage), false);
    VerifyOrReturnError(group.Find(mStorage, fabric, group_id), false);
    return endpoint.Find(mStorage, fabric, group, endpoint_id);
}

CHIP_ERROR GroupDataProviderImpl::AddEndpoint(chip::FabricIndex fabric_index, chip::GroupId group_id, chip::EndpointId endpoint_id)
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INTERNAL);

    FabricData fabric(fabric_index);
    GroupData group;

    // Load fabric data (defaults to zero)
    CHIP_ERROR err = fabric.Load(mStorage);
    VerifyOrReturnError(CHIP_NO_ERROR == err || CHIP_ERROR_NOT_FOUND == err, err);

    if (!group.Find(mStorage, fabric, group_id))
    {
        // New group
        VerifyOrReturnError(fabric.group_count < mMaxGroupsPerFabric, CHIP_ERROR_INVALID_LIST_LENGTH);
        ReturnErrorOnFailure(EndpointData(fabric_index, group_id, endpoint_id).Save(mStorage));
        // Save the new group into the fabric
        group.group_id       = group_id;
        group.name[0]        = 0;
        group.first_endpoint = endpoint_id;
        group.endpoint_count = 1;
        group.next           = fabric.first_group;
        group.prev           = kUndefinedGroupId;
        ReturnErrorOnFailure(group.Save(mStorage));
        // Update fabric
        fabric.first_group = group.group_id;
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
        group.first_endpoint = endpoint.endpoint_id;
    }
    else
    {
        // Previous endpoint(s)
        ReturnErrorOnFailure(endpoint.Save(mStorage));
        EndpointData prev(fabric_index, group.group_id, endpoint.prev);
        ReturnErrorOnFailure(prev.Load(mStorage));
        prev.next = endpoint.endpoint_id;
        ReturnErrorOnFailure(prev.Save(mStorage));
    }
    group.endpoint_count++;
    return group.Save(mStorage);
}

CHIP_ERROR GroupDataProviderImpl::RemoveEndpoint(chip::FabricIndex fabric_index, chip::GroupId group_id,
                                                 chip::EndpointId endpoint_id)
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INTERNAL);

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
        EndpointData prev(fabric_index, group.group_id, endpoint.prev);
        ReturnErrorOnFailure(prev.Load(mStorage));
        prev.next = endpoint.next;
        ReturnErrorOnFailure(prev.Save(mStorage));
    }

    if (group.endpoint_count > 1)
    {
        group.endpoint_count--;
        return group.Save(mStorage);
    }

    // No more endpoints, remove the group
    return RemoveGroupInfoAt(fabric_index, group.index);
}

CHIP_ERROR GroupDataProviderImpl::RemoveEndpoint(chip::FabricIndex fabric_index, chip::EndpointId endpoint_id)
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INTERNAL);

    FabricData fabric(fabric_index);

    ReturnErrorOnFailure(fabric.Load(mStorage));

    GroupData group(fabric_index, fabric.first_group);
    size_t group_index = 0;
    EndpointData endpoint;

    // Loop through all the groups
    while (group_index < fabric.group_count)
    {
        if (CHIP_NO_ERROR != group.Load(mStorage))
        {
            break;
        }
        if (endpoint.Find(mStorage, fabric, group, endpoint_id))
        {
            // Endpoint found in group
            ReturnErrorOnFailure(RemoveEndpoint(fabric_index, group.group_id, endpoint_id));
        }

        group.group_id = group.next;
        group_index++;
    }

    return CHIP_NO_ERROR;
}

GroupDataProvider::GroupInfoIterator * GroupDataProviderImpl::IterateGroupInfo(chip::FabricIndex fabric_index)
{
    VerifyOrReturnError(IsInitialized(), nullptr);
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

GroupDataProvider::EndpointIterator * GroupDataProviderImpl::IterateEndpoints(chip::FabricIndex fabric_index,
                                                                              std::optional<GroupId> group_id)
{
    VerifyOrReturnError(IsInitialized(), nullptr);
    return mEndpointIterators.CreateObject(*this, fabric_index, group_id);
}

GroupDataProviderImpl::EndpointIteratorImpl::EndpointIteratorImpl(GroupDataProviderImpl & provider, chip::FabricIndex fabric_index,
                                                                  std::optional<GroupId> group_id) :
    mProvider(provider),
    mFabric(fabric_index)
{
    FabricData fabric(fabric_index);
    VerifyOrReturn(CHIP_NO_ERROR == fabric.Load(provider.mStorage));

    if (group_id.has_value())
    {
        GroupData group(fabric_index, *group_id);
        VerifyOrReturn(CHIP_NO_ERROR == group.Load(provider.mStorage));

        mGroup         = *group_id;
        mFirstGroup    = *group_id;
        mGroupCount    = 1;
        mEndpoint      = group.first_endpoint;
        mEndpointCount = group.endpoint_count;
    }
    else
    {
        GroupData group(fabric_index, fabric.first_group);
        VerifyOrReturn(CHIP_NO_ERROR == group.Load(provider.mStorage));

        mGroup         = fabric.first_group;
        mFirstGroup    = fabric.first_group;
        mGroupCount    = fabric.group_count;
        mEndpoint      = group.first_endpoint;
        mEndpointCount = group.endpoint_count;
    }
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
        EndpointData endpoint(mFabric, group.group_id, group.first_endpoint);
        while (endpoint_index++ < group.endpoint_count)
        {
            if (CHIP_NO_ERROR != endpoint.Load(mProvider.mStorage))
            {
                break;
            }
            endpoint.endpoint_id = endpoint.next;
            count++;
        }
        group.group_id = group.next;
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
        if (mFirstEndpoint)
        {
            mEndpoint      = group.first_endpoint;
            mEndpointIndex = 0;
            mEndpointCount = group.endpoint_count;
            mFirstEndpoint = false;
        }
        if (mEndpointIndex < mEndpointCount)
        {
            EndpointData endpoint(mFabric, mGroup, mEndpoint);
            if (CHIP_NO_ERROR == endpoint.Load(mProvider.mStorage))
            {
                output.group_id    = group.group_id;
                output.endpoint_id = endpoint.endpoint_id;
                mEndpoint          = endpoint.next;
                mEndpointIndex++;
                return true;
            }
        }
        mGroup = group.next;
        mGroupIndex++;
        mFirstEndpoint = true;
    }
    return false;
}

void GroupDataProviderImpl::EndpointIteratorImpl::Release()
{
    mProvider.mEndpointIterators.ReleaseObject(this);
}

CHIP_ERROR GroupDataProviderImpl::RemoveEndpoints(chip::FabricIndex fabric_index, chip::GroupId group_id)
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INTERNAL);

    FabricData fabric(fabric_index);
    GroupData group;

    VerifyOrReturnError(CHIP_NO_ERROR == fabric.Load(mStorage), CHIP_ERROR_INVALID_FABRIC_INDEX);
    VerifyOrReturnError(group.Find(mStorage, fabric, group_id), CHIP_ERROR_KEY_NOT_FOUND);

    EndpointData endpoint(fabric_index, group.group_id, group.first_endpoint);
    size_t endpoint_index = 0;
    while (endpoint_index < group.endpoint_count)
    {
        ReturnErrorOnFailure(endpoint.Load(mStorage));
        endpoint.Delete(mStorage);
        endpoint.endpoint_id = endpoint.next;
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
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INTERNAL);

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
    VerifyOrReturnError(fabric.map_count < mMaxGroupsPerFabric, CHIP_ERROR_INVALID_LIST_LENGTH);

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
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INTERNAL);

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
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INTERNAL);

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
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INTERNAL);

    FabricData fabric(fabric_index);
    VerifyOrReturnError(CHIP_NO_ERROR == fabric.Load(mStorage), CHIP_ERROR_INVALID_FABRIC_INDEX);

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
    VerifyOrReturnError(IsInitialized(), nullptr);
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

CHIP_ERROR GroupDataProviderImpl::SetKeySet(chip::FabricIndex fabric_index, const ByteSpan & compressed_fabric_id,
                                            const KeySet & in_keyset)
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INTERNAL);

    FabricData fabric(fabric_index);
    KeySetData keyset;

    // Load fabric, defaults to zero
    CHIP_ERROR err = fabric.Load(mStorage);
    VerifyOrReturnError(CHIP_NO_ERROR == err || CHIP_ERROR_NOT_FOUND == err, err);

    // Search existing keyset
    bool found = keyset.Find(mStorage, fabric, in_keyset.keyset_id);

    keyset.keyset_id  = in_keyset.keyset_id;
    keyset.policy     = in_keyset.policy;
    keyset.keys_count = in_keyset.num_keys_used;
    memset(keyset.operational_keys, 0x00, sizeof(keyset.operational_keys));
    keyset.operational_keys[0].start_time = in_keyset.epoch_keys[0].start_time;
    keyset.operational_keys[1].start_time = in_keyset.epoch_keys[1].start_time;
    keyset.operational_keys[2].start_time = in_keyset.epoch_keys[2].start_time;

    // Store the operational keys and hash instead of the epoch keys
    for (size_t i = 0; i < in_keyset.num_keys_used; ++i)
    {
        ByteSpan epoch_key(in_keyset.epoch_keys[i].key, Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES);
        ReturnErrorOnFailure(
            Crypto::DeriveGroupOperationalCredentials(epoch_key, compressed_fabric_id, keyset.operational_keys[i]));
    }

    if (found)
    {
        // Update existing keyset info, keep next
        return keyset.Save(mStorage);
    }

    // New keyset
    VerifyOrReturnError(fabric.keyset_count < mMaxGroupKeysPerFabric, CHIP_ERROR_INVALID_LIST_LENGTH);

    // Insert first
    keyset.next = fabric.first_keyset;
    ReturnErrorOnFailure(keyset.Save(mStorage));
    // Update fabric
    fabric.keyset_count++;
    fabric.first_keyset = in_keyset.keyset_id;
    return fabric.Save(mStorage);
}

CHIP_ERROR GroupDataProviderImpl::GetKeySet(chip::FabricIndex fabric_index, uint16_t target_id, KeySet & out_keyset)
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INTERNAL);

    FabricData fabric(fabric_index);
    KeySetData keyset;

    ReturnErrorOnFailure(fabric.Load(mStorage));
    VerifyOrReturnError(keyset.Find(mStorage, fabric, target_id), CHIP_ERROR_NOT_FOUND);

    // Target keyset found
    out_keyset.ClearKeys();
    out_keyset.keyset_id     = keyset.keyset_id;
    out_keyset.policy        = keyset.policy;
    out_keyset.num_keys_used = keyset.keys_count;
    // Epoch keys are not read back, only start times
    out_keyset.epoch_keys[0].start_time = keyset.operational_keys[0].start_time;
    out_keyset.epoch_keys[1].start_time = keyset.operational_keys[1].start_time;
    out_keyset.epoch_keys[2].start_time = keyset.operational_keys[2].start_time;

    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupDataProviderImpl::RemoveKeySet(chip::FabricIndex fabric_index, uint16_t target_id)
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INTERNAL);

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
    ReturnErrorOnFailure(fabric.Save(mStorage));

    // Removing a key set also removes the associated group mappings
    KeyMapData map;
    uint16_t original_count = fabric.map_count;
    for (uint16_t i = 0; i < original_count; ++i)
    {
        fabric.Load(mStorage);
        size_t idx = map.Find(mStorage, fabric, target_id);
        if (idx == std::numeric_limits<size_t>::max())
        {
            break;
        }
        // NOTE: It's unclear what should happen here if we have removed the key set
        // and possibly some mappings before failing. For now, ignoring errors, but
        // open to suggestsions for the correct behavior.
        RemoveGroupKeyAt(fabric_index, idx);
    }
    return CHIP_NO_ERROR;
}

GroupDataProvider::KeySetIterator * GroupDataProviderImpl::IterateKeySets(chip::FabricIndex fabric_index)
{
    VerifyOrReturnError(IsInitialized(), nullptr);
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
    mNextId = keyset.next;
    output.ClearKeys();
    output.keyset_id     = keyset.keyset_id;
    output.policy        = keyset.policy;
    output.num_keys_used = keyset.keys_count;
    // Epoch keys are not read back, only start times
    output.epoch_keys[0].start_time = keyset.operational_keys[0].start_time;
    output.epoch_keys[1].start_time = keyset.operational_keys[1].start_time;
    output.epoch_keys[2].start_time = keyset.operational_keys[2].start_time;
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
// Cryptography
//

Crypto::SymmetricKeyContext * GroupDataProviderImpl::GetKeyContext(FabricIndex fabric_index, GroupId group_id)
{
    FabricData fabric(fabric_index);
    VerifyOrReturnError(CHIP_NO_ERROR == fabric.Load(mStorage), nullptr);

    KeyMapData mapping(fabric.fabric_index, fabric.first_map);

    // Look for the target group in the fabric's keyset-group pairs
    for (uint16_t i = 0; i < fabric.map_count; ++i, mapping.id = mapping.next)
    {
        VerifyOrReturnError(CHIP_NO_ERROR == mapping.Load(mStorage), nullptr);
        // GroupKeySetID of 0 is reserved for the Identity Protection Key (IPK),
        // it cannot be used for operational group communication.
        if (mapping.keyset_id > 0 && mapping.group_id == group_id)
        {
            // Group found, get the keyset
            KeySetData keyset;
            VerifyOrReturnError(keyset.Find(mStorage, fabric, mapping.keyset_id), nullptr);
            Crypto::GroupOperationalCredentials * creds = keyset.GetCurrentGroupCredentials();
            if (nullptr != creds)
            {
                return mGroupKeyContexPool.CreateObject(*this, creds->encryption_key, creds->hash, creds->privacy_key);
            }
        }
    }
    return nullptr;
}

CHIP_ERROR GroupDataProviderImpl::GetIpkKeySet(FabricIndex fabric_index, KeySet & out_keyset)
{
    FabricData fabric(fabric_index);
    VerifyOrReturnError(CHIP_NO_ERROR == fabric.Load(mStorage), CHIP_ERROR_NOT_FOUND);

    KeyMapData mapping(fabric.fabric_index, fabric.first_map);

    // Fabric found, get the keyset
    KeySetData keyset;
    VerifyOrReturnError(keyset.Find(mStorage, fabric, kIdentityProtectionKeySetId), CHIP_ERROR_NOT_FOUND);

    // If the keyset ID doesn't match, we have a ... problem.
    VerifyOrReturnError(keyset.keyset_id == kIdentityProtectionKeySetId, CHIP_ERROR_INTERNAL);

    out_keyset.keyset_id     = keyset.keyset_id;
    out_keyset.num_keys_used = keyset.keys_count;
    out_keyset.policy        = keyset.policy;

    for (size_t key_idx = 0; key_idx < MATTER_ARRAY_SIZE(out_keyset.epoch_keys); ++key_idx)
    {
        out_keyset.epoch_keys[key_idx].Clear();
        if (key_idx < keyset.keys_count)
        {
            out_keyset.epoch_keys[key_idx].start_time = keyset.operational_keys[key_idx].start_time;
            memcpy(&out_keyset.epoch_keys[key_idx].key[0], keyset.operational_keys[key_idx].encryption_key, EpochKey::kLengthBytes);
        }
    }

    return CHIP_NO_ERROR;
}

void GroupDataProviderImpl::GroupKeyContext::Release()
{
    ReleaseKeys();
    mProvider.mGroupKeyContexPool.ReleaseObject(this);
}

CHIP_ERROR GroupDataProviderImpl::GroupKeyContext::MessageEncrypt(const ByteSpan & plaintext, const ByteSpan & aad,
                                                                  const ByteSpan & nonce, MutableByteSpan & mic,
                                                                  MutableByteSpan & ciphertext) const
{
    uint8_t * output = ciphertext.data();
    return Crypto::AES_CCM_encrypt(plaintext.data(), plaintext.size(), aad.data(), aad.size(), mEncryptionKey, nonce.data(),
                                   nonce.size(), output, mic.data(), mic.size());
}

CHIP_ERROR GroupDataProviderImpl::GroupKeyContext::MessageDecrypt(const ByteSpan & ciphertext, const ByteSpan & aad,
                                                                  const ByteSpan & nonce, const ByteSpan & mic,
                                                                  MutableByteSpan & plaintext) const
{
    uint8_t * output = plaintext.data();
    return Crypto::AES_CCM_decrypt(ciphertext.data(), ciphertext.size(), aad.data(), aad.size(), mic.data(), mic.size(),
                                   mEncryptionKey, nonce.data(), nonce.size(), output);
}

CHIP_ERROR GroupDataProviderImpl::GroupKeyContext::PrivacyEncrypt(const ByteSpan & input, const ByteSpan & nonce,
                                                                  MutableByteSpan & output) const
{
    return Crypto::AES_CTR_crypt(input.data(), input.size(), mPrivacyKey, nonce.data(), nonce.size(), output.data());
}

CHIP_ERROR GroupDataProviderImpl::GroupKeyContext::PrivacyDecrypt(const ByteSpan & input, const ByteSpan & nonce,
                                                                  MutableByteSpan & output) const
{
    return Crypto::AES_CTR_crypt(input.data(), input.size(), mPrivacyKey, nonce.data(), nonce.size(), output.data());
}

GroupDataProviderImpl::GroupSessionIterator * GroupDataProviderImpl::IterateGroupSessions(uint16_t session_id)
{
    VerifyOrReturnError(IsInitialized(), nullptr);
    return mGroupSessionsIterator.CreateObject(*this, session_id);
}

GroupDataProviderImpl::GroupSessionIteratorImpl::GroupSessionIteratorImpl(GroupDataProviderImpl & provider, uint16_t session_id) :
    mProvider(provider), mSessionId(session_id), mGroupKeyContext(provider)
{
    FabricList fabric_list;
    ReturnOnFailure(fabric_list.Load(provider.mStorage));
    mFirstFabric = fabric_list.first_entry;
    mFabric      = fabric_list.first_entry;
    mFabricCount = 0;
    mFabricTotal = fabric_list.entry_count;
    mMapCount    = 0;
    mFirstMap    = true;
}

size_t GroupDataProviderImpl::GroupSessionIteratorImpl::Count()
{
    FabricData fabric(mFirstFabric);
    size_t count = 0;

    for (size_t i = 0; i < mFabricTotal; i++, fabric.fabric_index = fabric.next)
    {
        if (CHIP_NO_ERROR != fabric.Load(mProvider.mStorage))
        {
            break;
        }

        // Iterate key sets
        KeyMapData mapping(fabric.fabric_index, fabric.first_map);

        // Look for the target group in the fabric's keyset-group pairs
        for (uint16_t j = 0; j < fabric.map_count; ++j, mapping.id = mapping.next)
        {
            if (CHIP_NO_ERROR != mapping.Load(mProvider.mStorage))
            {
                break;
            }

            // Group found, get the keyset
            KeySetData keyset;
            if (!keyset.Find(mProvider.mStorage, fabric, mapping.keyset_id))
            {
                break;
            }
            for (uint16_t k = 0; k < keyset.keys_count; ++k)
            {
                if (keyset.operational_keys[k].hash == mSessionId)
                {
                    count++;
                }
            }
        }
    }
    return count;
}

bool GroupDataProviderImpl::GroupSessionIteratorImpl::Next(GroupSession & output)
{
    while (mFabricCount < mFabricTotal)
    {
        FabricData fabric(mFabric);
        VerifyOrReturnError(CHIP_NO_ERROR == fabric.Load(mProvider.mStorage), false);

        if (mMapCount >= fabric.map_count)
        {
            // No more keyset/group mappings on the current fabric, try next fabric
            mFabric = fabric.next;
            mFabricCount++;
            mFirstMap = true;
            mMapCount = 0;
            continue;
        }

        if (mFirstMap)
        {
            mMapping  = fabric.first_map;
            mFirstMap = false;
        }

        KeyMapData mapping(mFabric, mMapping);
        VerifyOrReturnError(CHIP_NO_ERROR == mapping.Load(mProvider.mStorage), false);

        // Group found, get the keyset
        KeySetData keyset;
        VerifyOrReturnError(keyset.Find(mProvider.mStorage, fabric, mapping.keyset_id), false);

        if (mKeyIndex >= keyset.keys_count)
        {
            // No more keys in current keyset, try next
            mMapping = mapping.next;
            mMapCount++;
            mKeyIndex = 0;
            continue;
        }

        Crypto::GroupOperationalCredentials & creds = keyset.operational_keys[mKeyIndex++];
        if (creds.hash == mSessionId)
        {
            mGroupKeyContext.Initialize(creds.encryption_key, mSessionId, creds.privacy_key);
            output.fabric_index    = fabric.fabric_index;
            output.group_id        = mapping.group_id;
            output.security_policy = keyset.policy;
            output.keyContext      = &mGroupKeyContext;
            return true;
        }
    }

    return false;
}

void GroupDataProviderImpl::GroupSessionIteratorImpl::Release()
{
    mGroupKeyContext.ReleaseKeys();
    mProvider.mGroupSessionsIterator.ReleaseObject(this);
}

namespace {

GroupDataProvider * gGroupsProvider = nullptr;

} // namespace

GroupDataProvider * GetGroupDataProvider()
{
    return gGroupsProvider;
}

void SetGroupDataProvider(GroupDataProvider * provider)
{
    gGroupsProvider = provider;
}

} // namespace Credentials
} // namespace chip
