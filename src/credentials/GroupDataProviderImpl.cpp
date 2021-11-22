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
#include <credentials/PersistentStorageKey.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Pool.h>
#include <stdlib.h>
#include <string.h>

namespace chip {
namespace Credentials {

static constexpr uint16_t kPersistentBufferMax = 128;

template <size_t S>
struct PersistentData
{
public:
    virtual ~PersistentData() = default;

    virtual void UpdateKey()                                    = 0;
    virtual void Clear()                                        = 0;
    virtual CHIP_ERROR Serialize(TLV::TLVWriter & writer) const = 0;
    virtual CHIP_ERROR Deserialize(TLV::TLVReader & reader)     = 0;

    CHIP_ERROR Save(chip::PersistentStorageDelegate & storage)
    {
        // Serialize the data
        TLV::TLVWriter writer;
        writer.Init(mBuffer, sizeof(mBuffer));
        ReturnErrorOnFailure(Serialize(writer));

        // Update storage key
        UpdateKey();
        // Save serialized data
        ReturnErrorOnFailure(storage.SyncSetKeyValue(mKey.Value(), mBuffer, static_cast<uint16_t>(writer.GetLengthWritten())));

        // WARNING: This read-back is a walkaround for a bug in the Darwing implementation of KeyValueStoreMgr
        //          See https://github.com/project-chip/connectedhomeip/issues/12174.
        uint16_t size = static_cast<uint16_t>(sizeof(mBuffer));
        ReturnErrorOnFailure(storage.SyncGetKeyValue(mKey.Value(), mBuffer, size));

        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Load(chip::PersistentStorageDelegate & storage)
    {
        // Update storage key
        UpdateKey();
        // Load the serialized data
        uint16_t size = static_cast<uint16_t>(sizeof(mBuffer));
        ReturnErrorOnFailure(storage.SyncGetKeyValue(mKey.Value(), mBuffer, size));

        // Decode serialized data
        Clear();
        TLV::TLVReader reader;
        reader.Init(mBuffer, size);
        ReturnErrorOnFailure(Deserialize(reader));

        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Delete(chip::PersistentStorageDelegate & storage)
    {
        // Update storage key
        UpdateKey();
        // Delete stored data
        return storage.SyncDeleteKeyValue(mKey.Value());
    }

protected:
    PersistentStorageKey mKey;
    uint8_t mBuffer[S] = { 0 };
    char mDebug[512];
};

struct FabricData : public PersistentData<kPersistentBufferMax>
{
    static const TLV::Tag kFirstEndpoint = 1;
    static const TLV::Tag kEndpointCount = 2;
    static const TLV::Tag kFirstKeyset   = 3;
    static const TLV::Tag kKeysetCount   = 4;

    chip::FabricIndex fabric        = 0;
    chip::EndpointId first_endpoint = kInvalidEndpointId;
    uint16_t endpoint_count         = 0;
    uint16_t first_keyset           = 0;
    uint16_t keyset_count           = 0;

    FabricData(chip::FabricIndex fabric_index) : fabric(fabric_index) {}

    void UpdateKey() override { this->mKey.FabricGroups(this->fabric); }

    void Clear() override
    {
        first_endpoint = kInvalidEndpointId;
        first_keyset   = 0;
        keyset_count   = 0;
    }

    CHIP_ERROR Serialize(TLV::TLVWriter & writer) const override
    {
        ReturnErrorOnFailure(writer.Put(kFirstEndpoint, static_cast<uint16_t>(first_endpoint)));
        ReturnErrorOnFailure(writer.Put(kEndpointCount, static_cast<uint16_t>(endpoint_count)));
        ReturnErrorOnFailure(writer.Put(kFirstKeyset, static_cast<uint16_t>(first_keyset)));
        ReturnErrorOnFailure(writer.Put(kKeysetCount, static_cast<uint16_t>(keyset_count)));

        return CHIP_NO_ERROR;
    }
    CHIP_ERROR Deserialize(TLV::TLVReader & reader) override
    {
        // first_endpoint
        ReturnErrorOnFailure(reader.Next(kFirstEndpoint));
        ReturnErrorOnFailure(reader.Get(first_endpoint));
        // endpoint_count
        ReturnErrorOnFailure(reader.Next(kEndpointCount));
        ReturnErrorOnFailure(reader.Get(endpoint_count));
        // first_keyset
        ReturnErrorOnFailure(reader.Next(kFirstKeyset));
        ReturnErrorOnFailure(reader.Get(first_keyset));
        // keyset_count
        ReturnErrorOnFailure(reader.Next(kKeysetCount));
        ReturnErrorOnFailure(reader.Get(keyset_count));

        return CHIP_NO_ERROR;
    }
};

struct EndpointData : public PersistentData<kPersistentBufferMax>
{
    static const TLV::Tag kFirstGroup   = 1;
    static const TLV::Tag kNextEndpoint = 2;

    chip::FabricIndex fabric  = 0;
    chip::EndpointId id       = 0;
    chip::GroupId first_group = kUndefinedGroupId;
    chip::EndpointId next     = kInvalidEndpointId;

    EndpointData() = default;

    EndpointData(chip::FabricIndex fabric_index, chip::EndpointId endpoint_id = kInvalidEndpointId,
                 chip::GroupId first = kUndefinedGroupId, chip::EndpointId next_endpoint = kInvalidEndpointId) :
        fabric(fabric_index),
        id(endpoint_id), first_group(first), next(next_endpoint)
    {}

    void UpdateKey() override { this->mKey.FabricEndpoint(this->fabric, this->id); }

    void Clear() override
    {
        first_group = kUndefinedGroupId;
        next        = kInvalidEndpointId;
    }

    CHIP_ERROR Serialize(TLV::TLVWriter & writer) const override
    {
        ReturnErrorOnFailure(writer.Put(kFirstGroup, static_cast<uint16_t>(first_group)));
        ReturnErrorOnFailure(writer.Put(kNextEndpoint, static_cast<uint16_t>(next)));
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Deserialize(TLV::TLVReader & reader) override
    {
        // first_group
        ReturnErrorOnFailure(reader.Next(kFirstGroup));
        ReturnErrorOnFailure(reader.Get(first_group));
        // next
        ReturnErrorOnFailure(reader.Next(kNextEndpoint));
        ReturnErrorOnFailure(reader.Get(next));
        return CHIP_NO_ERROR;
    }
};

struct GroupData : public GroupDataProvider::GroupMapping, PersistentData<kPersistentBufferMax>
{
    static const TLV::Tag kTagName = 1;
    static const TLV::Tag kTagNext = 2;

    chip::FabricIndex fabric = 0;
    chip::GroupId next       = 0;

    GroupData(chip::FabricIndex fabric_index, chip::EndpointId endpoint_id, chip::GroupId group_id,
              chip::GroupId next_group = kUndefinedGroupId) :
        GroupDataProvider::GroupMapping(endpoint_id, group_id),
        fabric(fabric_index), next(next_group)
    {}

    GroupData(chip::FabricIndex fabric_index, chip::EndpointId endpoint_id, chip::GroupId group_id, const char * group_name,
              chip::GroupId next_group = kUndefinedGroupId) :
        GroupDataProvider::GroupMapping(endpoint_id, group_id, group_name),
        fabric(fabric_index), next(next_group)
    {}

    void UpdateKey() override { this->mKey.FabricEndpointGroup(this->fabric, this->endpoint, this->group); }

    void Clear() override
    {
        name[0] = 0;
        next    = kUndefinedGroupId;
    }

    CHIP_ERROR Serialize(TLV::TLVWriter & writer) const override
    {
        size_t name_size = strnlen(this->name, GroupDataProvider::GroupMapping::kGroupNameMax);
        ReturnErrorOnFailure(writer.PutString(kTagName, this->name, static_cast<uint32_t>(name_size)));
        ReturnErrorOnFailure(writer.Put(kTagNext, static_cast<uint16_t>(this->next)));
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR Deserialize(TLV::TLVReader & reader) override
    {
        // name
        ReturnErrorOnFailure(reader.Next(kTagName));
        ReturnErrorOnFailure(reader.GetString(this->name, sizeof(this->name)));
        // next
        ReturnErrorOnFailure(reader.Next(kTagNext));
        ReturnErrorOnFailure(reader.Get(this->next));
        return CHIP_NO_ERROR;
    }
};

struct StateListData : public PersistentData<kPersistentBufferMax>
{
    static const TLV::Tag kTagFirst = 1;
    static const TLV::Tag kTagCount = 2;

    uint16_t first = 0;
    uint16_t count = 0;

    void UpdateKey() override { this->mKey.GroupStates(); }

    void Clear() override
    {
        first = 0;
        count = 0;
    }

    CHIP_ERROR Serialize(TLV::TLVWriter & writer) const override
    {
        ReturnErrorOnFailure(writer.Put(kTagFirst, static_cast<uint16_t>(first)));
        ReturnErrorOnFailure(writer.Put(kTagCount, static_cast<uint16_t>(count)));
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR Deserialize(TLV::TLVReader & reader) override
    {
        // first
        ReturnErrorOnFailure(reader.Next(kTagFirst));
        ReturnErrorOnFailure(reader.Get(first));
        // count
        ReturnErrorOnFailure(reader.Next(kTagCount));
        ReturnErrorOnFailure(reader.Get(count));
        return CHIP_NO_ERROR;
    }
};

struct StateData : public GroupDataProvider::GroupState, PersistentData<kPersistentBufferMax>
{
    static const TLV::Tag kTagFabric = 1;
    static const TLV::Tag kTagGroup  = 2;
    static const TLV::Tag kTagKeyset = 3;
    static const TLV::Tag kTagNext   = 4;

    // Linked-list index. May be different to state_index
    uint16_t index;
    // Index of the next element of the linked-list
    uint16_t next = 0;

    StateData() = default;
    StateData(uint16_t state_index, chip::FabricIndex fabric = 0, chip::GroupId group_id = kUndefinedGroupId,
              uint16_t key_set = 0) :
        GroupState(fabric, group_id, key_set),
        index(state_index)
    {}

    void UpdateKey() override { this->mKey.GroupState(this->index); }

    void Clear() override
    {
        fabric_index = 0;
        group        = kUndefinedGroupId;
        keyset_index = 0;
        next         = 0;
    }

    CHIP_ERROR Serialize(TLV::TLVWriter & writer) const override
    {
        ReturnErrorOnFailure(writer.Put(kTagFabric, static_cast<uint8_t>(fabric_index)));
        ReturnErrorOnFailure(writer.Put(kTagGroup, static_cast<uint16_t>(group)));
        ReturnErrorOnFailure(writer.Put(kTagKeyset, static_cast<uint16_t>(keyset_index)));
        ReturnErrorOnFailure(writer.Put(kTagNext, static_cast<uint16_t>(next)));

        return CHIP_NO_ERROR;
    }
    CHIP_ERROR Deserialize(TLV::TLVReader & reader) override
    {
        // fabric_index
        ReturnErrorOnFailure(reader.Next(kTagFabric));
        ReturnErrorOnFailure(reader.Get(fabric_index));
        // group
        ReturnErrorOnFailure(reader.Next(kTagGroup));
        ReturnErrorOnFailure(reader.Get(group));
        // keyset_index
        ReturnErrorOnFailure(reader.Next(kTagKeyset));
        ReturnErrorOnFailure(reader.Get(keyset_index));
        // next
        ReturnErrorOnFailure(reader.Next(kTagNext));
        ReturnErrorOnFailure(reader.Get(next));

        return CHIP_NO_ERROR;
    }
};

struct KeysetData : public GroupDataProvider::Keyset, PersistentData<kPersistentBufferMax>
{
    static const TLV::Tag kTagKeysetId  = 1;
    static const TLV::Tag kTagPolicy    = 2;
    static const TLV::Tag kTagNumKeys   = 3;
    static const TLV::Tag kTagEpochKeys = 4;
    static const TLV::Tag kTagStartTime = 5;
    static const TLV::Tag kTagKey       = 6;
    static const TLV::Tag kTagNext      = 7;

    chip::FabricIndex fabric = 0;
    uint16_t next            = 0;

    // KeysetData() = default;
    KeysetData(chip::FabricIndex fabric_index, uint16_t id) : Keyset(id), fabric(fabric_index) {}
    KeysetData(chip::FabricIndex fabric_index, uint16_t id, SecurityPolicy policy_id, uint8_t num_keys) :
        Keyset(id, policy_id, num_keys), fabric(fabric_index)
    {}

    void UpdateKey() override { this->mKey.FabricKeyset(this->fabric, this->keyset_id); }

    void Clear() override
    {
        keyset_id     = 0;
        policy        = Keyset::SecurityPolicy::kStandard;
        num_keys_used = 0;
        next          = 0;
        memset(epoch_keys, 0x00, sizeof(epoch_keys));
    }

    CHIP_ERROR Serialize(TLV::TLVWriter & writer) const override
    {
        // keyset_id
        ReturnErrorOnFailure(writer.Put(kTagKeysetId, static_cast<uint16_t>(keyset_id)));
        // policy
        ReturnErrorOnFailure(writer.Put(kTagPolicy, static_cast<uint16_t>(policy)));
        // num_keys_used
        ReturnErrorOnFailure(writer.Put(kTagNumKeys, static_cast<uint16_t>(num_keys_used)));
        // epoch_keys
        {
            TLV::TLVType array, item;
            writer.StartContainer(kTagEpochKeys, TLV::kTLVType_Array, array);
            for (auto & epoch : this->epoch_keys)
            {
                writer.StartContainer(TLV::AnonymousTag, TLV::kTLVType_Structure, item);
                ReturnErrorOnFailure(writer.Put(TLV::ContextTag(kTagStartTime), static_cast<uint64_t>(epoch.start_time)));
                ReturnErrorOnFailure(
                    writer.PutBytes(TLV::ContextTag(kTagKey), epoch.key, GroupDataProvider::EpochKey::kLengthBytes));
                writer.EndContainer(item);
            }
            writer.EndContainer(array);
        }
        // next keyset
        ReturnErrorOnFailure(writer.Put(kTagNext, static_cast<uint16_t>(next)));

        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Deserialize(TLV::TLVReader & reader) override
    {
        // keyset_id
        ReturnErrorOnFailure(reader.Next(kTagKeysetId));
        ReturnErrorOnFailure(reader.Get(keyset_id));
        // policy
        ReturnErrorOnFailure(reader.Next(kTagPolicy));
        ReturnErrorOnFailure(reader.Get(policy));
        // num_keys_used
        ReturnErrorOnFailure(reader.Next(kTagNumKeys));
        ReturnErrorOnFailure(reader.Get(num_keys_used));
        {
            // epoch_keys
            ReturnErrorOnFailure(reader.Next(kTagEpochKeys));
            VerifyOrReturnError(TLV::kTLVType_Array == reader.GetType(), CHIP_ERROR_INTERNAL);

            TLV::TLVType array, item;
            reader.EnterContainer(array);
            for (auto & epoch : this->epoch_keys)
            {
                ReturnErrorOnFailure(reader.Next(TLV::AnonymousTag));
                VerifyOrReturnError(TLV::kTLVType_Structure == reader.GetType(), CHIP_ERROR_INTERNAL);

                reader.EnterContainer(item);
                // start_time
                ReturnErrorOnFailure(reader.Next(TLV::ContextTag(kTagStartTime)));
                ReturnErrorOnFailure(reader.Get(epoch.start_time));
                // key
                ReturnErrorOnFailure(reader.Next(TLV::ContextTag(kTagKey)));
                ReturnErrorOnFailure(reader.GetBytes(epoch.key, GroupDataProvider::EpochKey::kLengthBytes));
                reader.ExitContainer(item);
            }
            reader.ExitContainer(array);
        }
        // next keyset
        ReturnErrorOnFailure(reader.Next(kTagNext));
        ReturnErrorOnFailure(reader.Get(next));
        return CHIP_NO_ERROR;
    }
};

//
// General
//

CHIP_ERROR GroupDataProviderImpl::Init()
{
    mInitialized = true;
    return CHIP_NO_ERROR;
}

void GroupDataProviderImpl::Finish()
{
    mInitialized = false;
}

//
// Group Mappings
//

bool GroupDataProviderImpl::HasGroupNamesSupport()
{
    return true;
}

bool GroupDataProviderImpl::GroupMappingExists(chip::FabricIndex fabric_index, const GroupMapping & mapping)
{
    VerifyOrReturnError(mInitialized, false);

    FabricData fabric(fabric_index);

    VerifyOrReturnError(CHIP_NO_ERROR == fabric.Load(mStorage), false);
    VerifyOrReturnError(kInvalidEndpointId != fabric.first_endpoint, false);

    // Existing fabric

    EndpointData endpoint(fabric_index, fabric.first_endpoint);
    size_t endpoint_count = 0;

    // Loop through the fabric's endpoints
    do
    {
        VerifyOrReturnError(CHIP_NO_ERROR == endpoint.Load(mStorage), false);
        if (endpoint.id == mapping.endpoint)
        {
            // Target endpoint found
            break;
        }
        endpoint.id = endpoint.next;
    } while ((++endpoint_count < fabric.endpoint_count) && (kInvalidEndpointId != endpoint.id));
    VerifyOrReturnError(endpoint.id == mapping.endpoint, false);

    // Target endpoint found

    GroupData group(fabric_index, mapping.endpoint, endpoint.first_group);

    // Loop through the endpoint's groups
    do
    {
        // Load next group
        VerifyOrReturnError(CHIP_NO_ERROR == group.Load(mStorage), false);
        if (group.group == mapping.group)
        {
            // Target group found
            return true;
        }
        group.group = group.next;
    } while (kUndefinedGroupId != group.group);

    // Not found
    return false;
}

CHIP_ERROR GroupDataProviderImpl::AddGroupMapping(chip::FabricIndex fabric_index, const GroupMapping & mapping)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(kInvalidEndpointId != mapping.endpoint, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(IsFabricGroupId(mapping.group), CHIP_ERROR_INVALID_ARGUMENT);

    FabricData fabric(fabric_index);

    if (CHIP_NO_ERROR != fabric.Load(mStorage) || kInvalidEndpointId == fabric.first_endpoint)
    {
        // New fabric, or new endpoint
        GroupData(fabric_index, mapping.endpoint, mapping.group, mapping.name).Save(mStorage);
        EndpointData(fabric_index, mapping.endpoint, mapping.group).Save(mStorage);
        fabric.first_endpoint = mapping.endpoint;
        fabric.endpoint_count = 1;
        return fabric.Save(mStorage);
    }

    // Existing fabric

    EndpointData endpoint(fabric_index, fabric.first_endpoint);
    size_t endpoint_count = 0;

    // Loop through the fabric's endpoints
    do
    {
        if (CHIP_NO_ERROR != endpoint.Load(mStorage))
        {
            // Endpoint info not found
            break;
        }
        if (endpoint.id == mapping.endpoint)
        {
            // Target endpoint info found
            break;
        }
        endpoint.id = endpoint.next;
    } while ((++endpoint_count < fabric.endpoint_count) && (kInvalidEndpointId != endpoint.id));

    if (endpoint.id != mapping.endpoint)
    {
        // Endpoint not found, create new, and insert first
        GroupData(fabric_index, mapping.endpoint, mapping.group, mapping.name).Save(mStorage);
        EndpointData(fabric_index, mapping.endpoint, mapping.group, fabric.first_endpoint).Save(mStorage);
        fabric.first_endpoint = mapping.endpoint;
        fabric.endpoint_count++;
        return fabric.Save(mStorage);
    }

    // Endpoint info found, loop endpoints

    GroupData group(fabric_index, endpoint.id, endpoint.first_group);

    do
    {
        if (CHIP_NO_ERROR != group.Load(mStorage))
        {
            break;
        }
        if (group.group == mapping.group)
        {
            // Duplicated group
            return CHIP_NO_ERROR;
        }
        group.group = group.next;
    } while (kUndefinedGroupId != group.group);

    // Insert group first, update endpoint
    GroupData(fabric_index, mapping.endpoint, mapping.group, mapping.name, endpoint.first_group).Save(mStorage);
    endpoint.first_group = mapping.group;
    return endpoint.Save(mStorage);
}

CHIP_ERROR GroupDataProviderImpl::RemoveGroupMapping(chip::FabricIndex fabric_index, const GroupMapping & mapping)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(kInvalidEndpointId != mapping.endpoint, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(IsFabricGroupId(mapping.group), CHIP_ERROR_INVALID_ARGUMENT);

    FabricData fabric(fabric_index);

    VerifyOrReturnError(CHIP_NO_ERROR == fabric.Load(mStorage), CHIP_ERROR_INVALID_FABRIC_ID);
    VerifyOrReturnError(kInvalidEndpointId != fabric.first_endpoint, CHIP_ERROR_KEY_NOT_FOUND);

    // Existing fabric

    EndpointData endpoint(fabric_index, fabric.first_endpoint);
    size_t endpoint_count = 0;

    // Loop through the fabric's endpoints
    do
    {
        VerifyOrReturnError(CHIP_NO_ERROR == endpoint.Load(mStorage), CHIP_ERROR_KEY_NOT_FOUND);
        if (endpoint.id == mapping.endpoint)
        {
            // Target endpoint info found
            break;
        }
        endpoint.id = endpoint.next;
    } while ((++endpoint_count < fabric.endpoint_count) && (kInvalidEndpointId != endpoint.id));
    VerifyOrReturnError(endpoint.id == mapping.endpoint, CHIP_ERROR_KEY_NOT_FOUND);

    // Target endpoint found

    GroupData group(fabric_index, endpoint.id, endpoint.first_group);
    chip::GroupId prev_gid = kUndefinedGroupId;

    do
    {
        VerifyOrReturnError(CHIP_NO_ERROR == group.Load(mStorage), CHIP_ERROR_KEY_NOT_FOUND);
        if (group.group == mapping.group)
        {
            // Target group found
            break;
        }
        prev_gid    = group.group;
        group.group = group.next;
    } while (kUndefinedGroupId != group.group);

    VerifyOrReturnError(group.group == mapping.group, CHIP_ERROR_KEY_NOT_FOUND);

    // Target group found, remove
    group.Delete(mStorage);

    if (prev_gid == kUndefinedGroupId)
    {
        // Removing first group, update endpoint
        endpoint.first_group = group.next;
        return endpoint.Save(mStorage);
    }

    // Removing intermediate group, update previous

    GroupData prev_group(fabric_index, endpoint.id, prev_gid);
    ReturnErrorOnFailure(prev_group.Load(mStorage));

    prev_group.next = group.next;
    return prev_group.Save(mStorage);

    return CHIP_ERROR_INTERNAL;
}

CHIP_ERROR GroupDataProviderImpl::RemoveAllGroupMappings(chip::FabricIndex fabric_index, EndpointId endpoint_id)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(kInvalidEndpointId != endpoint_id, CHIP_ERROR_INVALID_ARGUMENT);

    FabricData fabric(fabric_index);

    VerifyOrReturnError(CHIP_NO_ERROR == fabric.Load(mStorage), CHIP_ERROR_INVALID_FABRIC_ID);
    VerifyOrReturnError(kInvalidEndpointId != fabric.first_endpoint, CHIP_ERROR_KEY_NOT_FOUND);

    // Existing fabric

    EndpointData endpoint(fabric_index, fabric.first_endpoint);
    chip::EndpointId prev_eid = kInvalidEndpointId;
    size_t endpoint_count     = 0;

    // Loop through the fabric's endpoints
    do
    {
        VerifyOrReturnError(CHIP_NO_ERROR == endpoint.Load(mStorage), CHIP_ERROR_KEY_NOT_FOUND);
        if (endpoint.id == endpoint_id)
        {
            // Target endpoint info found
            break;
        }
        prev_eid    = endpoint.id;
        endpoint.id = endpoint.next;
    } while ((++endpoint_count < fabric.endpoint_count) && (kInvalidEndpointId != endpoint.id));
    VerifyOrReturnError(endpoint.id == endpoint_id, CHIP_ERROR_KEY_NOT_FOUND);

    // Target endpoint found
    GroupData group(fabric_index, endpoint.id, endpoint.first_group);

    // Remove endpoint's groups
    do
    {
        ReturnErrorOnFailure(group.Load(mStorage));
        group.Delete(mStorage);
        group.group = group.next;
    } while (kUndefinedGroupId != group.group);

    // Remove endpoint

    if (kInvalidEndpointId == prev_eid)
    {
        // First endpoint, update fabric info
        fabric.first_endpoint = endpoint.next;
        fabric.Save(mStorage);
    }
    else
    {
        // Mid endpoint, update previous endpoint's info
        EndpointData prev_endpoint(fabric_index, prev_eid);
        prev_endpoint.Load(mStorage);
        prev_endpoint.next = endpoint.next;
        prev_endpoint.Save(mStorage);
    }

    return endpoint.Delete(mStorage);
}

GroupDataProvider::GroupMappingIterator * GroupDataProviderImpl::IterateGroupMappings(chip::FabricIndex fabric_index,
                                                                                      EndpointId endpoint_id)
{
    VerifyOrReturnError(mInitialized, nullptr);
    return mEndpointIterators.CreateObject(*this, fabric_index, endpoint_id);
}

GroupDataProviderImpl::GroupMappingIteratorImpl::GroupMappingIteratorImpl(GroupDataProviderImpl & provider,
                                                                          chip::FabricIndex fabric_index,
                                                                          chip::EndpointId endpoint_id) :
    mProvider(provider),
    mFabric(fabric_index), mEndpoint(endpoint_id)
{
    FabricData fabric(fabric_index);
    ReturnOnFailure(fabric.Load(provider.mStorage));

    // Existing fabric

    EndpointData endpoint(fabric_index, fabric.first_endpoint);
    uint16_t count = 0;

    // Loop through the fabric's endpoints
    do
    {
        ReturnOnFailure(endpoint.Load(provider.mStorage));
        if (endpoint.id == endpoint_id)
        {
            // Target endpoint found
            mGroup = endpoint.first_group;
            break;
        }
        endpoint.id = endpoint.next;
    } while (++count < fabric.endpoint_count);
}

size_t GroupDataProviderImpl::GroupMappingIteratorImpl::Count()
{
    size_t count = 0;

    GroupData group(mFabric, mEndpoint, mGroup);
    chip::GroupId prev_gid = kUndefinedGroupId;

    while ((kUndefinedGroupId != group.group) && (prev_gid != group.group))
    {
        if (CHIP_NO_ERROR != group.Load(mProvider.mStorage))
        {
            break;
        }
        prev_gid    = group.group;
        group.group = group.next;
        count++;
    }
    return count;
}

bool GroupDataProviderImpl::GroupMappingIteratorImpl::Next(GroupMapping & item)
{
    GroupData group(mFabric, mEndpoint, mGroup);
    CHIP_ERROR err = group.Load(mProvider.mStorage);
    if ((kUndefinedGroupId == mGroup) || (CHIP_NO_ERROR != err))
    {
        return false;
    }
    item.endpoint = mEndpoint;
    item.group    = mGroup;
    strncpy(item.name, group.name, GroupData::kGroupNameMax);
    mGroup = group.next;
    return true;
}

void GroupDataProviderImpl::GroupMappingIteratorImpl::Release()
{
    mProvider.mEndpointIterators.ReleaseObject(this);
}

//
// Group States
//

CHIP_ERROR GroupDataProviderImpl::SetGroupState(size_t state_index, const GroupState & in_state)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(IsFabricGroupId(in_state.group), CHIP_ERROR_INVALID_ARGUMENT);

    StateListData states;

    if (CHIP_NO_ERROR != states.Load(mStorage))
    {
        // First state
        VerifyOrReturnError(0 == state_index, CHIP_ERROR_INVALID_ARGUMENT);
        states.first = 1; // Arbitrary index > 0
        states.count = 1;
        ReturnLogErrorOnFailure(
            StateData(states.first, in_state.fabric_index, in_state.group, in_state.keyset_index).Save(mStorage));
        return states.Save(mStorage);
    }

    // Pseudo-index > 0

    StateData state(states.first);
    StateData prev_state;
    uint16_t actual_index = 0;
    uint16_t new_index    = 1;
    uint16_t prev_index   = 0;
    bool found            = false;

    // Loop until the desired index
    do
    {
        if (CHIP_NO_ERROR != state.Load(mStorage))
        {
            break;
        }
        if (new_index == state.index)
        {
            // Used index, keep looking
            new_index++;
        }
        if (actual_index == state_index)
        {
            // Target index found
            found = true;
            break;
        }
        prev_index  = state.index;
        prev_state  = state;
        state.index = state.next;
    } while (++actual_index < states.count);

    VerifyOrReturnError(state_index <= actual_index, CHIP_ERROR_INVALID_ARGUMENT);

    if (found)
    {
        // Update existing state, must be in the same fabric
        VerifyOrReturnError(state.fabric_index == in_state.fabric_index, CHIP_ERROR_ACCESS_DENIED);
        GroupState old_state = state;
        state.group          = in_state.group;
        state.keyset_index   = in_state.keyset_index;
        ReturnErrorOnFailure(state.Save(mStorage));
        if (nullptr != mListener)
        {
            mListener->OnGroupStateChanged(&old_state, &state);
        }
        return CHIP_NO_ERROR;
    }

    // New state
    ReturnErrorOnFailure(StateData(new_index, in_state.fabric_index, in_state.group, in_state.keyset_index).Save(mStorage));

    if (prev_index > 0)
    {
        // New middle state, update previous
        prev_state.next = new_index;
        ReturnErrorOnFailure(prev_state.Save(mStorage));
    }
    else
    {
        // New first state
        states.first = new_index;
    }
    // Update main list
    states.count = static_cast<uint16_t>(actual_index + 1);
    return states.Save(mStorage);

    return CHIP_ERROR_INTERNAL;
}

CHIP_ERROR GroupDataProviderImpl::GetGroupState(size_t state_index, GroupState & out_state)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INTERNAL);

    StateListData states;
    VerifyOrReturnError(CHIP_NO_ERROR == states.Load(mStorage), CHIP_ERROR_KEY_NOT_FOUND);

    // Fabric info found
    // VerifyOrReturnError(state_index <= fabric.state_count, CHIP_ERROR_INVALID_ARGUMENT);

    StateData state(states.first);
    size_t actual_index = 0;

    // Loop until the desired index
    while (actual_index < states.count)
    {
        if (CHIP_NO_ERROR != state.Load(mStorage))
        {
            break;
        }
        if (actual_index == state_index)
        {
            // Target index found
            out_state.fabric_index = state.fabric_index;
            out_state.group        = state.group;
            out_state.keyset_index = state.keyset_index;
            return CHIP_NO_ERROR;
        }
        state.index = state.next;
        actual_index++;
    }
    return CHIP_ERROR_KEY_NOT_FOUND;
}

CHIP_ERROR GroupDataProviderImpl::RemoveGroupState(size_t state_index)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INTERNAL);

    StateListData states;

    VerifyOrReturnError(CHIP_NO_ERROR == states.Load(mStorage), CHIP_ERROR_KEY_NOT_FOUND);

    StateData state(states.first);
    StateData prev_state;
    size_t actual_index = 0;
    size_t new_index    = 1;
    size_t prev_index   = 0;
    bool found          = false;

    // Loop until the desired index
    while (actual_index < states.count)
    {
        if (CHIP_NO_ERROR != state.Load(mStorage))
        {
            break;
        }
        if (new_index == state.index)
        {
            // Used index, keep looking
            new_index++;
        }
        if (actual_index == state_index)
        {
            // Target index found
            found = true;
            break;
        }
        prev_index  = state.index;
        prev_state  = state;
        state.index = state.next;
        actual_index++;
    }

    VerifyOrReturnError(found, CHIP_ERROR_KEY_NOT_FOUND);

    if (states.count > 0)
    {
        states.count--;
    }

    ReturnErrorOnFailure(state.Delete(mStorage));
    if (0 == prev_index)
    {
        // Remove first state
        states.first = state.next;
    }
    else
    {
        // Remove intermediate state
        prev_state.next = state.next;
        ReturnErrorOnFailure(prev_state.Save(mStorage));
    }
    ReturnErrorOnFailure(states.Save(mStorage));
    if (nullptr != mListener)
    {
        mListener->OnGroupStateRemoved(&state);
    }
    return CHIP_NO_ERROR;
}

GroupDataProvider::GroupStateIterator * GroupDataProviderImpl::IterateGroupStates()
{
    VerifyOrReturnError(mInitialized, nullptr);
    return mAllStatesIterators.CreateObject(*this);
}

GroupDataProvider::GroupStateIterator * GroupDataProviderImpl::IterateGroupStates(chip::FabricIndex fabric_index)
{
    VerifyOrReturnError(mInitialized, nullptr);
    return mFabricStatesIterators.CreateObject(*this, fabric_index);
}

GroupDataProviderImpl::AllStatesIterator::AllStatesIterator(GroupDataProviderImpl & provider) : mProvider(provider)
{
    StateListData states;
    if (CHIP_NO_ERROR == states.Load(provider.mStorage))
    {
        mIndex      = states.first;
        mTotalCount = states.count;
    }
}

size_t GroupDataProviderImpl::AllStatesIterator::Count()
{
    return mTotalCount;
}

bool GroupDataProviderImpl::AllStatesIterator::Next(GroupState & item)
{
    VerifyOrReturnError(mCount < mTotalCount, false);

    StateData state(mIndex);
    VerifyOrReturnError(CHIP_NO_ERROR == state.Load(mProvider.mStorage), false);

    mCount++;
    mIndex            = state.next;
    item.fabric_index = state.fabric_index;
    item.group        = state.group;
    item.keyset_index = state.keyset_index;
    return true;
}

void GroupDataProviderImpl::AllStatesIterator::Release()
{
    mProvider.mAllStatesIterators.ReleaseObject(this);
}

GroupDataProviderImpl::FabricStatesIterator::FabricStatesIterator(GroupDataProviderImpl & provider,
                                                                  chip::FabricIndex fabric_index) :
    mProvider(provider),
    mFabric(fabric_index)
{
    StateListData states;
    if (CHIP_NO_ERROR == states.Load(provider.mStorage))
    {
        mIndex      = states.first;
        mTotalCount = states.count;
    }
}

size_t GroupDataProviderImpl::FabricStatesIterator::Count()
{
    size_t count        = 0;
    size_t actual_index = 0;
    StateData state(mIndex);

    while (actual_index++ < mTotalCount)
    {
        if (CHIP_NO_ERROR != state.Load(mProvider.mStorage))
        {
            break;
        }
        if (state.fabric_index == mFabric)
        {
            count++;
        }
        state.index = state.next;
    }
    return count;
}

bool GroupDataProviderImpl::FabricStatesIterator::Next(GroupState & item)
{
    StateData state(mIndex);

    while (mCount++ < mTotalCount)
    {
        if (CHIP_NO_ERROR != state.Load(mProvider.mStorage))
        {
            mCount = mTotalCount;
            break;
        }
        state.index = state.next;
        if (state.fabric_index == mFabric)
        {
            item.fabric_index = state.fabric_index;
            item.group        = state.group;
            item.keyset_index = state.keyset_index;
            mIndex            = state.index;
            return true;
        }
    }
    return false;
}

void GroupDataProviderImpl::FabricStatesIterator::Release()
{
    mProvider.mFabricStatesIterators.ReleaseObject(this);
}

//
// Key Sets
//

CHIP_ERROR GroupDataProviderImpl::SetKeyset(chip::FabricIndex fabric_index, uint16_t target_id, const Keyset & in_keyset)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INTERNAL);

    FabricData fabric(fabric_index);
    if (CHIP_NO_ERROR != fabric.Load(mStorage))
    {
        // First in_keyset
        fabric.keyset_count = 1;
        fabric.first_keyset = target_id;
        KeysetData keyset(fabric_index, target_id, in_keyset.policy, in_keyset.num_keys_used);
        memcpy(keyset.epoch_keys, in_keyset.epoch_keys, sizeof(in_keyset.epoch_keys));
        ReturnLogErrorOnFailure(keyset.Save(mStorage));
        return fabric.Save(mStorage);
    }

    // Pseudo-index > 0

    KeysetData keyset(fabric_index, fabric.first_keyset);
    size_t keyset_count = 0;
    bool found          = false;

    // Seach for the target in_keyset id
    while (keyset_count < fabric.keyset_count)
    {
        if (CHIP_NO_ERROR != keyset.Load(mStorage))
        {
            break;
        }
        if (keyset.keyset_id == target_id)
        {
            // Target id found
            found = true;
            break;
        }
        keyset.keyset_id = keyset.next;
        keyset_count++;
    }

    keyset.keyset_id     = target_id;
    keyset.policy        = in_keyset.policy;
    keyset.num_keys_used = in_keyset.num_keys_used;
    memcpy(keyset.epoch_keys, in_keyset.epoch_keys, sizeof(in_keyset.epoch_keys));

    if (found)
    {
        // Update existing in_keyset info, keep next
        return keyset.Save(mStorage);
    }

    // New in_keyset, insert first
    keyset.next = fabric.first_keyset;
    ReturnErrorOnFailure(keyset.Save(mStorage));
    // Update fabric info
    fabric.keyset_count++;
    fabric.first_keyset = target_id;
    return fabric.Save(mStorage);
}

CHIP_ERROR GroupDataProviderImpl::GetKeyset(chip::FabricIndex fabric_index, uint16_t target_id, Keyset & out_keyset)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INTERNAL);

    FabricData fabric(fabric_index);
    VerifyOrReturnError(CHIP_NO_ERROR == fabric.Load(mStorage), CHIP_ERROR_INVALID_FABRIC_ID);

    KeysetData keyset(fabric_index, fabric.first_keyset);
    size_t keyset_count = 0;

    // Loop until the desired index
    while (keyset_count < fabric.keyset_count)
    {
        if (CHIP_NO_ERROR != keyset.Load(mStorage))
        {
            break;
        }
        if (keyset.keyset_id == target_id)
        {
            // Target index found
            out_keyset.keyset_id     = keyset.keyset_id;
            out_keyset.policy        = keyset.policy;
            out_keyset.num_keys_used = keyset.num_keys_used;
            memcpy(out_keyset.epoch_keys, keyset.epoch_keys, sizeof(out_keyset.epoch_keys));
            return CHIP_NO_ERROR;
        }
        keyset.keyset_id = keyset.next;
        keyset_count++;
    }

    return CHIP_ERROR_KEY_NOT_FOUND;
}

CHIP_ERROR GroupDataProviderImpl::RemoveKeyset(chip::FabricIndex fabric_index, uint16_t target_id)
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INTERNAL);

    FabricData fabric(fabric_index);
    VerifyOrReturnError(CHIP_NO_ERROR == fabric.Load(mStorage), CHIP_ERROR_KEY_NOT_FOUND);

    KeysetData keyset(fabric_index, fabric.first_keyset);
    uint16_t prev_id    = 0;
    size_t keyset_count = 0;

    // Loop until the desired index
    while (keyset_count < fabric.keyset_count)
    {
        if (CHIP_NO_ERROR != keyset.Load(mStorage))
        {
            break;
        }
        if (keyset.keyset_id == target_id)
        {
            // Target index found
            break;
        }
        prev_id          = keyset.keyset_id;
        keyset.keyset_id = keyset.next;
        keyset_count++;
    }

    VerifyOrReturnError(keyset.keyset_id == target_id, CHIP_ERROR_KEY_NOT_FOUND);

    keyset.Delete(mStorage);
    if (keyset_count > 0)
    {
        // Remove intermediate keyset, update previous
        KeysetData prev_data(fabric_index, prev_id);
        ReturnErrorOnFailure(prev_data.Load(mStorage));
        prev_data.next = keyset.next;
        ReturnErrorOnFailure(prev_data.Save(mStorage));
    }
    else
    {
        // Remove first keyset
        fabric.first_keyset = keyset.next;
    }
    if (fabric.keyset_count > 0)
    {
        fabric.keyset_count--;
    }
    // Update fabric info
    return fabric.Save(mStorage);
}

GroupDataProvider::KeysetIterator * GroupDataProviderImpl::IterateKeysets(chip::FabricIndex fabric_index)
{
    VerifyOrReturnError(mInitialized, nullptr);
    return mKeysetIterators.CreateObject(*this, fabric_index);
}

GroupDataProviderImpl::KeysetIteratorImpl::KeysetIteratorImpl(GroupDataProviderImpl & provider, chip::FabricIndex fabric_index) :
    mProvider(provider), mFabric(fabric_index)
{
    FabricData fabric(fabric_index);
    if (CHIP_NO_ERROR == fabric.Load(provider.mStorage))
    {
        mNextId = fabric.first_keyset;
        mCount  = fabric.keyset_count;
        mIndex  = 0;
    }
}

size_t GroupDataProviderImpl::KeysetIteratorImpl::Count()
{
    return mCount;
}

bool GroupDataProviderImpl::KeysetIteratorImpl::Next(Keyset & item)
{
    VerifyOrReturnError(mIndex < mCount, false);

    KeysetData keyset(mFabric, mNextId);
    VerifyOrReturnError(CHIP_NO_ERROR == keyset.Load(mProvider.mStorage), false);

    mIndex++;
    mNextId            = keyset.next;
    item.keyset_id     = keyset.keyset_id;
    item.policy        = keyset.policy;
    item.num_keys_used = keyset.num_keys_used;
    memcpy(item.epoch_keys, keyset.epoch_keys, sizeof(item.epoch_keys));
    return true;
}

void GroupDataProviderImpl::KeysetIteratorImpl::Release()
{
    mProvider.mKeysetIterators.ReleaseObject(this);
}

//
// Fabrics
//

CHIP_ERROR GroupDataProviderImpl::RemoveFabric(chip::FabricIndex fabric_index)
{
    FabricData fabric(fabric_index);
    VerifyOrReturnError(CHIP_NO_ERROR == fabric.Load(mStorage), CHIP_ERROR_KEY_NOT_FOUND);

    // Remove Group Mappings

    EndpointData endpoint(fabric_index, fabric.first_endpoint);
    size_t endpoint_count = 0;

    do
    {
        if (CHIP_NO_ERROR != endpoint.Load(mStorage))
        {
            break;
        }
        RemoveAllGroupMappings(fabric_index, endpoint.id);
        endpoint.id = endpoint.next;
    } while ((++endpoint_count < fabric.endpoint_count) && (kInvalidEndpointId != endpoint.id));

    // Remove States

    StateListData states;

    // Load state list info
    states.Load(mStorage);

    StateData state(states.first);
    size_t state_index = 0;

    while (state_index < states.count)
    {
        if (CHIP_NO_ERROR != state.Load(mStorage))
        {
            break;
        }
        if (state.fabric_index == fabric_index)
        {
            // Removing the state shifts down the upper states
            RemoveGroupState(state_index);
        }
        else
        {
            state_index++;
        }
        state.index = state.next;
    }

    // Remove Keysets

    KeysetData keyset(fabric_index, fabric.first_keyset);
    size_t keyset_count = 0;

    // Loop until the desired index
    while (keyset_count < fabric.keyset_count)
    {
        if (CHIP_NO_ERROR != keyset.Load(mStorage))
        {
            break;
        }
        RemoveKeyset(fabric_index, keyset.keyset_id);
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
                                          System::PacketBufferHandle && msg)
{
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
