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
#include <credentials/GroupDataProvider.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Pool.h>
#include <platform/KeyValueStoreManager.h>
#include <stdlib.h>
#include <string.h>

namespace chip {
namespace Credentials {
namespace {

static constexpr size_t kIteratorsMax      = CHIP_CONFIG_MAX_GROUP_CONCURRENT_ITERATORS;
static constexpr size_t kGroupNameMax      = CHIP_CONFIG_MAX_GROUP_NAME_LENGTH;
static constexpr size_t kStoreKeyLengthMax = 128;
static const char * kStorePrefix           = "gdp";

struct FabricEndpoint
{
    uint32_t group_count;
};

class GroupDataProviderImpl : public GroupDataProvider
{
protected:
    template <typename T, int S>
    class Storage
    {
    public:
        virtual ~Storage() = default;

        CHIP_ERROR Save(const T & value)
        {
            T copy;
            ReturnErrorOnFailure(DeviceLayer::PersistedStorage::KeyValueStoreMgr().Put<T>(this->mKey, value));
            return DeviceLayer::PersistedStorage::KeyValueStoreMgr().Get<T>(this->mKey, &copy);
        }
        CHIP_ERROR Load(T & value)
        {
            Clear(value);
            return DeviceLayer::PersistedStorage::KeyValueStoreMgr().Get<T>(this->mKey, &value);
        }
        CHIP_ERROR Delete() { return DeviceLayer::PersistedStorage::KeyValueStoreMgr().Delete(this->mKey); }

        virtual void Clear(T & value) = 0;

    protected:
        char mKey[S]     = { 0 };
        char mDebug[128] = { 0 };
    };

    struct EndpointInfo
    {
        EndpointInfo() = default;
        EndpointInfo(chip::GroupId first) : first_group(first) {}
        EndpointInfo(chip::GroupId first, chip::EndpointId next_endpoint) : first_group(first), next(next_endpoint) {}
        chip::GroupId first_group = kUndefinedGroupId;
        chip::EndpointId next     = kInvalidEndpointId;
        bool operator==(const EndpointInfo & other) { return this->first_group == other.first_group && this->next == other.next; }
    };

    class EndpointStore : public Storage<EndpointInfo, kStoreKeyLengthMax>
    {
    public:
        EndpointStore(chip::FabricIndex fabric_index, chip::EndpointId endpoint)
        {
            snprintf(this->mKey, sizeof(this->mKey), "%s:map:%02x:%04x", kStorePrefix, fabric_index, endpoint);
        }
        void Clear(EndpointInfo & info) override
        {
            info.first_group = kUndefinedGroupId;
            info.next        = kInvalidEndpointId;
        }
    };

    struct GroupInfo
    {
        GroupInfo() = default;
        GroupInfo(const CharSpan & name)
        {
            this->name_len = std::min(kGroupNameMax, name.size());
            memcpy(this->name, name.data(), this->name_len);
            this->name[this->name_len] = 0;
        }
        bool operator==(const GroupInfo & other)
        {
            return this->name_len == other.name_len && this->next == other.next && !strncmp(this->name, other.name, kGroupNameMax);
        }
        char name[kGroupNameMax + 1];
        size_t name_len    = 0;
        chip::GroupId next = 0;
    };

    class GroupMapStore : public Storage<GroupInfo, kStoreKeyLengthMax>
    {
    public:
        GroupMapStore(chip::FabricIndex fabric_index, chip::EndpointId endpoint, chip::GroupId group)
        {
            snprintf(this->mKey, sizeof(this->mKey), "%s:map:%02x:%04x:%04x", kStorePrefix, fabric_index, endpoint, group);
        }
        void Clear(GroupInfo & info) override
        {
            info.name_len = 0;
            info.name[0]  = 0;
            info.next     = kUndefinedGroupId;
        }
    };

    struct StateListInfo : public GroupState
    {
        // First state's pseudo-index, which may be different from the actual state_index
        size_t first = 0;
        size_t count = 0;

        bool operator==(const StateListInfo & other) { return this->first == other.first && this->count == other.count; }
    };

    class StateListStore : public Storage<StateListInfo, kStoreKeyLengthMax>
    {
    public:
        StateListStore() { snprintf(this->mKey, sizeof(this->mKey), "%s:states", kStorePrefix); }
        void Clear(StateListInfo & info) override
        {
            info.first = 0;
            info.count = 0;
        }
    };

    struct StateInfo : public GroupState
    {
        StateInfo() = default;
        StateInfo(chip::FabricIndex fabric, chip::GroupId group_id, uint16_t key_set) : GroupState(fabric, group_id, key_set) {}
        // Next state's pseudo-index, which may be different from the actual state_index
        size_t next = 0;
        bool operator==(const StateInfo & other)
        {
            return this->fabric_index == other.fabric_index && this->group == other.group &&
                this->keyset_index == other.keyset_index && this->next == other.next;
        }
    };

    class StateStore : public Storage<StateInfo, kStoreKeyLengthMax>
    {
    public:
        StateStore(size_t state_index) { snprintf(this->mKey, sizeof(this->mKey), "%s:state:%04zx", kStorePrefix, state_index); }
        void Clear(StateInfo & info) override
        {
            info.fabric_index = 0;
            info.group        = kUndefinedGroupId;
            info.keyset_index = 0;
            info.next         = 0;
        }
    };

    struct KeySetInfo : public KeySet
    {
        KeySetInfo() = default;
        KeySetInfo(uint16_t id, SecurityPolicy poli, uint8_t num_keys) : KeySet(id, poli, num_keys) {}
        KeySetInfo(KeySet::SecurityPolicy poli, uint8_t num_keys) : KeySet(poli, num_keys) {}
        // Next keyset id
        uint16_t next = 0;
        bool operator==(const KeySetInfo & other)
        {
            if (this->policy == other.policy && this->num_keys_used == other.num_keys_used && this->next == other.next)
            {
                return !memcmp(this->epoch_keys, other.epoch_keys, this->num_keys_used * sizeof(EpochKey));
            }
            return false;
        }
    };

    class KeySetStore : public Storage<KeySetInfo, kStoreKeyLengthMax>
    {
    public:
        KeySetStore(chip::FabricIndex fabric_index, uint16_t keyset_id)
        {
            snprintf(this->mKey, sizeof(this->mKey), "%s:keys:%02x:%04x", kStorePrefix, fabric_index, keyset_id);
        }
        void Clear(KeySetInfo & info) override
        {
            info.keyset_id     = 0;
            info.policy        = KeySet::SecurityPolicy::kStandard;
            info.num_keys_used = 0;
            info.next          = 0;
            memset(info.epoch_keys, 0x00, sizeof(info.epoch_keys));
        }
    };

    struct FabricInfo
    {
        chip::EndpointId first_endpoint = kInvalidEndpointId;
        size_t endpoint_count           = 0;
        uint16_t first_keyset           = 0;
        size_t keyset_count             = 0;

        // The state pseudo-indexes are arbitrary, 0 is reserved for empty list
        bool operator==(const FabricInfo & other) { return this->first_endpoint == other.first_endpoint; }
    };

    class FabricStore : public Storage<FabricInfo, kStoreKeyLengthMax>
    {
    public:
        FabricStore(chip::FabricIndex fabric_index)
        {
            snprintf(this->mKey, sizeof(this->mKey), "%s:fabric:%02x", kStorePrefix, fabric_index);
        }
        void Clear(FabricInfo & info) override
        {
            info.first_endpoint = kInvalidEndpointId;
            info.first_keyset   = 0;
            info.keyset_count   = 0;
        }
    };

    class EndpointIterator : public GroupMappingIterator
    {
    public:
        EndpointIterator(GroupDataProviderImpl & provider, chip::FabricIndex fabric, chip::EndpointId endpoint) :
            mProvider(provider), mFabric(fabric), mEndpoint(endpoint)
        {
            FabricInfo fabric_info;

            ReturnOnFailure(FabricStore(fabric).Load(fabric_info));

            // Existing fabric

            chip::EndpointId next_endpoint = fabric_info.first_endpoint;
            chip::EndpointId endpoint_id   = kInvalidEndpointId;
            EndpointInfo endpoint_info;

            // Loop through the fabric's endpoints
            do
            {
                EndpointStore endpoint_store(fabric, next_endpoint);
                ReturnOnFailure(endpoint_store.Load(endpoint_info));
                endpoint_id   = next_endpoint;
                next_endpoint = endpoint_info.next;
            } while (endpoint_id != endpoint && endpoint_id != next_endpoint);
            VerifyOrReturn(endpoint_id == endpoint);

            // Target endpoint found

            mGroup = endpoint_info.first_group;
        }

        size_t Count() override
        {
            size_t count = 0;

            chip::GroupId group_id   = mGroup;
            chip::GroupId prev_group = kUndefinedGroupId;
            GroupInfo group_info;

            while ((kUndefinedGroupId != group_id) && (prev_group != group_id))
            {
                if (CHIP_NO_ERROR != GroupMapStore(mFabric, mEndpoint, group_id).Load(group_info))
                {
                    break;
                }
                prev_group = group_id;
                group_id   = group_info.next;
                count++;
            }
            return count;
        }

        bool Next(GroupMapping & mapping) override
        {
            if ((kUndefinedGroupId == mGroup) || (CHIP_NO_ERROR != GroupMapStore(mFabric, mEndpoint, mGroup).Load(mGroupInfo)))
            {
                return false;
            }
            mapping.endpoint = mEndpoint;
            mapping.group    = mGroup;
            mapping.name     = CharSpan(mGroupInfo.name, mGroupInfo.name_len);
            mGroup           = mGroupInfo.next;
            return true;
        }

        void Release() override { mProvider.Release(this); }

    private:
        GroupDataProviderImpl & mProvider;
        chip::FabricIndex mFabric  = 0;
        chip::EndpointId mEndpoint = 0;
        chip::GroupId mGroup       = kUndefinedGroupId;
        GroupInfo mGroupInfo;
    };

    class AllStatesIterator : public GroupStateIterator
    {
    public:
        AllStatesIterator(GroupDataProviderImpl & provider) : mProvider(provider)
        {
            StateListInfo states_info;
            if (CHIP_NO_ERROR == StateListStore().Load(states_info))
            {
                mPseudoIndex = states_info.first;
                mCount       = states_info.count;
            }
        }

        size_t Count() override { return mCount; }

        bool Next(GroupState & outEntry) override
        {
            if (mIndex >= mCount)
            {
                return false;
            }
            StateInfo state_info;
            if (CHIP_NO_ERROR != StateStore(mPseudoIndex).Load(state_info))
            {
                mIndex = mCount;
                return false;
            }
            mIndex++;
            mPseudoIndex          = state_info.next;
            outEntry.fabric_index = state_info.fabric_index;
            outEntry.group        = state_info.group;
            outEntry.keyset_index = state_info.keyset_index;
            return true;
        }

        void Release() override { mProvider.Release(this); }

    private:
        GroupDataProviderImpl & mProvider;
        size_t mPseudoIndex = 0;
        size_t mIndex       = 0;
        size_t mCount       = 0;
    };

    class FabricStatesIterator : public GroupStateIterator
    {
    public:
        FabricStatesIterator(GroupDataProviderImpl & provider, chip::FabricIndex fabric_index) :
            mProvider(provider), mFabric(fabric_index)
        {
            StateListInfo states_info;
            if (CHIP_NO_ERROR == StateListStore().Load(states_info))
            {
                mPseudoIndex = states_info.first;
                mTotalCount  = states_info.count;
            }
        }

        size_t Count() override
        {
            size_t count        = 0;
            size_t actual_index = 0;
            size_t pseudo_index = mPseudoIndex;
            StateInfo state_info;

            while (actual_index++ < mTotalCount)
            {
                if (CHIP_NO_ERROR != StateStore(pseudo_index).Load(state_info))
                {
                    break;
                }
                if (state_info.fabric_index == mFabric)
                {
                    count++;
                }
                pseudo_index = state_info.next;
            }
            return count;
        }

        bool Next(GroupState & outEntry) override
        {
            StateInfo state_info;
            while (mIndex++ < mTotalCount)
            {
                if (CHIP_NO_ERROR != StateStore(mPseudoIndex).Load(state_info))
                {
                    mIndex = mTotalCount;
                    break;
                }
                mPseudoIndex = state_info.next;
                if (state_info.fabric_index == mFabric)
                {
                    outEntry.fabric_index = state_info.fabric_index;
                    outEntry.group        = state_info.group;
                    outEntry.keyset_index = state_info.keyset_index;
                    return true;
                }
            }
            return false;
        }

        void Release() override { mProvider.Release(this); }

    private:
        GroupDataProviderImpl & mProvider;
        chip::FabricIndex mFabric = 0;
        size_t mPseudoIndex       = 0;
        size_t mIndex             = 0;
        size_t mTotalCount        = 0;
    };

    class KeySetIteratorImpl : public KeySetIterator
    {
    public:
        KeySetIteratorImpl(GroupDataProviderImpl & provider, chip::FabricIndex fabric_index) :
            mProvider(provider), mFabric(fabric_index)
        {
            FabricStore fabric_store(fabric_index);
            FabricInfo fabric_info;
            if (CHIP_NO_ERROR == fabric_store.Load(fabric_info))
            {
                mNextId = fabric_info.first_keyset;
                mCount  = fabric_info.keyset_count;
                mIndex  = 0;
            }
        }

        size_t Count() override { return mCount; }

        bool Next(KeySet & outEntry) override
        {
            if (mIndex >= mCount)
            {
                return false;
            }

            KeySetInfo keyset_info;
            if (CHIP_NO_ERROR != KeySetStore(mFabric, mNextId).Load(keyset_info))
            {
                return false;
            }
            mIndex++;
            mNextId                = keyset_info.next;
            outEntry.keyset_id     = keyset_info.keyset_id;
            outEntry.policy        = keyset_info.policy;
            outEntry.num_keys_used = keyset_info.num_keys_used;
            memcpy(outEntry.epoch_keys, keyset_info.epoch_keys, sizeof(outEntry.epoch_keys));
            return true;
        }

        void Release() override { mProvider.Release(this); }

    private:
        GroupDataProviderImpl & mProvider;
        chip::FabricIndex mFabric = 0;
        uint16_t mNextId          = 0;
        size_t mCount             = 0;
        size_t mIndex             = 0;
    };

public:
    //
    // Common
    //

    CHIP_ERROR Init() override
    {
#if CHIP_DEVICE_LAYER_TARGET_DARWIN
        ReturnErrorOnFailure(DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().Init("chip.store"));
#elif CHIP_DEVICE_LAYER_TARGET_LINUX
        ReturnErrorOnFailure(DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().Init(CHIP_CONFIG_KVS_PATH));
#endif
        mInitialized = true;
        return CHIP_NO_ERROR;
    }

    void Finish() override { mInitialized = false; }

    //
    // Group Mappings
    //

    bool HasGroupNamesSupport() override { return true; }

    bool GroupMappingExists(chip::FabricIndex fabric_index, const GroupMapping & mapping) override
    {
        VerifyOrReturnError(mInitialized, false);

        FabricStore fabric_store(fabric_index);
        FabricInfo fabric_info;

        CHIP_ERROR err = fabric_store.Load(fabric_info);
        VerifyOrReturnError(CHIP_NO_ERROR == err, false);
        VerifyOrReturnError(kInvalidEndpointId != fabric_info.first_endpoint, false);

        // Existing fabric

        chip::EndpointId endpoint_id = fabric_info.first_endpoint;
        EndpointInfo endpoint_info;
        size_t endpoint_count = 0;

        // Loop through the fabric's endpoints
        do
        {
            EndpointStore endpoint_store(fabric_index, endpoint_id);
            VerifyOrReturnError(CHIP_NO_ERROR == endpoint_store.Load(endpoint_info), false);
            if (endpoint_id == mapping.endpoint)
            {
                // Target endpoint found
                break;
            }
            endpoint_id = endpoint_info.next;
        } while (++endpoint_count < fabric_info.endpoint_count);
        VerifyOrReturnError(endpoint_id == mapping.endpoint, false);

        // Target endpoint found

        chip::GroupId group_id   = endpoint_info.first_group;
        chip::GroupId prev_group = kUndefinedGroupId;
        GroupInfo group_info;

        // Loop through the endpoint's groups
        do
        {
            // Load next group
            VerifyOrReturnError(CHIP_NO_ERROR == GroupMapStore(fabric_index, endpoint_id, group_id).Load(group_info), false);
            if (group_id == mapping.group)
            {
                // Target group found
                return true;
            }
            prev_group = group_id;
            group_id   = group_info.next;
        } while ((kUndefinedGroupId != group_id) && (prev_group != group_id));

        // Not found
        return false;
    }

    CHIP_ERROR AddGroupMapping(chip::FabricIndex fabric_index, const GroupMapping & mapping) override
    {
        VerifyOrReturnError(mInitialized, CHIP_ERROR_INTERNAL);
        VerifyOrReturnError(kInvalidEndpointId != mapping.endpoint, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(IsFabricGroupId(mapping.group), CHIP_ERROR_INVALID_ARGUMENT);

        FabricStore fabric_store(fabric_index);
        FabricInfo fabric_info;

        CHIP_ERROR err = fabric_store.Load(fabric_info);
        if (CHIP_NO_ERROR != err || kInvalidEndpointId == fabric_info.first_endpoint)
        {
            // New fabric, or new endpoint
            GroupMapStore(fabric_index, mapping.endpoint, mapping.group).Save(GroupInfo(mapping.name));
            EndpointStore(fabric_index, mapping.endpoint).Save(EndpointInfo(mapping.group));
            fabric_info.first_endpoint = mapping.endpoint;
            fabric_info.endpoint_count = 1;
            return fabric_store.Save(fabric_info);
        }

        // Existing fabric

        chip::EndpointId endpoint_id   = fabric_info.first_endpoint;
        chip::EndpointId prev_endpoint = kInvalidEndpointId;
        EndpointInfo endpoint_info;
        EndpointInfo prev_end_info;
        size_t endpoint_count = 0;

        // Loop through the fabric's endpoints
        do
        {
            EndpointStore endpoint_store(fabric_index, endpoint_id);
            if (CHIP_NO_ERROR != endpoint_store.Load(endpoint_info))
            {
                // Endpoint info not found
                break;
            }
            if (endpoint_id == mapping.endpoint)
            {
                // Target endpoint info found
                break;
            }
            prev_end_info = endpoint_info;
            prev_endpoint = endpoint_id;
            endpoint_id   = endpoint_info.next;
        } while (++endpoint_count < fabric_info.endpoint_count);

        if (endpoint_id != mapping.endpoint)
        {
            // Endpoint info NOT found, create new
            if (kInvalidEndpointId != prev_endpoint)
            {
                // Link to existing endpoint
                prev_end_info.next = mapping.endpoint;
                EndpointStore(fabric_index, prev_endpoint).Save(prev_end_info);
            }
            // Append new endpoint info
            GroupMapStore(fabric_index, mapping.endpoint, mapping.group).Save(GroupInfo(mapping.name));
            EndpointStore(fabric_index, mapping.endpoint).Save(EndpointInfo(mapping.group));
            fabric_info.endpoint_count++;
            return fabric_store.Save(fabric_info);
        }

        // Endpoint info found, loop endpoints

        chip::GroupId group_id   = endpoint_info.first_group;
        chip::GroupId prev_group = kUndefinedGroupId;
        GroupInfo group_info;

        do
        {
            if (CHIP_NO_ERROR != GroupMapStore(fabric_index, endpoint_id, group_id).Load(group_info))
            {
                break;
            }
            if (group_id == mapping.group)
            {
                // Duplicated group
                return CHIP_NO_ERROR;
            }
            prev_group = group_id;
            group_id   = group_info.next;
        } while ((kUndefinedGroupId != group_id) && (prev_group != group_id));

        GroupMapStore(fabric_index, mapping.endpoint, mapping.group).Save(GroupInfo(mapping.name));
        if (kUndefinedGroupId == prev_group)
        {
            // Adding first group, update endpoint
            endpoint_info.first_group = mapping.group;
            return EndpointStore(fabric_index, mapping.endpoint).Save(endpoint_info);
        }
        else
        {
            // Adding last group, update previous group
            GroupMapStore prev_store(fabric_index, endpoint_id, prev_group);
            GroupInfo prev_info;
            prev_store.Load(prev_info);
            prev_info.next = mapping.group;
            return prev_store.Save(prev_info);
        }
    }

    CHIP_ERROR RemoveGroupMapping(chip::FabricIndex fabric_index, const GroupMapping & mapping) override
    {
        VerifyOrReturnError(mInitialized, CHIP_ERROR_INTERNAL);
        VerifyOrReturnError(kInvalidEndpointId != mapping.endpoint, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(IsFabricGroupId(mapping.group), CHIP_ERROR_INVALID_ARGUMENT);

        FabricStore fabric_store(fabric_index);
        FabricInfo fabric_info;

        VerifyOrReturnError(CHIP_NO_ERROR == fabric_store.Load(fabric_info), CHIP_ERROR_INVALID_FABRIC_ID);
        VerifyOrReturnError(kInvalidEndpointId != fabric_info.first_endpoint, CHIP_ERROR_KEY_NOT_FOUND);

        // Existing fabric

        chip::EndpointId endpoint_id = fabric_info.first_endpoint;
        EndpointInfo endpoint_info;
        size_t endpoint_count = 0;

        // Loop through the fabric's endpoints
        do
        {
            EndpointStore endpoint_store(fabric_index, endpoint_id);
            VerifyOrReturnError(CHIP_NO_ERROR == endpoint_store.Load(endpoint_info), CHIP_ERROR_KEY_NOT_FOUND);
            if (endpoint_id == mapping.endpoint)
            {
                // Target endpoint info found
                break;
            }
            endpoint_id = endpoint_info.next;
        } while (++endpoint_count < fabric_info.endpoint_count);
        VerifyOrReturnError(endpoint_id == mapping.endpoint, CHIP_ERROR_KEY_NOT_FOUND);

        // Target endpoint found

        chip::GroupId group_id   = endpoint_info.first_group;
        chip::GroupId prev_group = kUndefinedGroupId;
        GroupInfo group_info;

        do
        {
            ReturnErrorOnFailure(GroupMapStore(fabric_index, endpoint_id, group_id).Load(group_info));
            // VerifyOrReturnError(CHIP_NO_ERROR == endpoint_store.Load(endpoint_info), CHIP_ERROR_KEY_NOT_FOUND);
            if (group_id == mapping.group)
            {
                // Target group found
                break;
            }
            prev_group = group_id;
            group_id   = group_info.next;
        } while ((kUndefinedGroupId != group_id) && (prev_group != group_id));

        VerifyOrReturnError(group_id == mapping.group, CHIP_ERROR_KEY_NOT_FOUND);

        // Target group found, remove
        GroupMapStore(fabric_index, endpoint_id, group_id).Delete();

        if (prev_group == kUndefinedGroupId)
        {
            // Removing first group, update endpoint
            endpoint_info.first_group = group_info.next;
            return EndpointStore(fabric_index, mapping.endpoint).Save(endpoint_info);
        }

        // Removing intermediate group, update previous

        GroupMapStore prev_store(fabric_index, endpoint_id, prev_group);
        GroupInfo prev_info;
        ReturnErrorOnFailure(prev_store.Load(prev_info));

        prev_info.next = group_info.next;
        return prev_store.Save(prev_info);
    }

    CHIP_ERROR RemoveAllGroupMappings(chip::FabricIndex fabric_index, chip::EndpointId target_endpoint_id) override
    {
        VerifyOrReturnError(mInitialized, CHIP_ERROR_INTERNAL);
        VerifyOrReturnError(kInvalidEndpointId != target_endpoint_id, CHIP_ERROR_INVALID_ARGUMENT);

        FabricStore fabric_store(fabric_index);
        FabricInfo fabric_info;

        CHIP_ERROR err = fabric_store.Load(fabric_info);
        VerifyOrReturnError(CHIP_NO_ERROR == err, CHIP_ERROR_INVALID_FABRIC_ID);
        VerifyOrReturnError(kInvalidEndpointId != fabric_info.first_endpoint, CHIP_ERROR_KEY_NOT_FOUND);

        // Existing fabric

        chip::EndpointId endpoint_id   = fabric_info.first_endpoint;
        chip::EndpointId prev_endpoint = kInvalidEndpointId;
        EndpointInfo endpoint_info;
        EndpointInfo prev_endpoint_info;
        size_t endpoint_count = 0;

        // Loop through the fabric's endpoints
        do
        {
            EndpointStore endpoint_store(fabric_index, endpoint_id);
            VerifyOrReturnError(CHIP_NO_ERROR == endpoint_store.Load(endpoint_info), CHIP_ERROR_KEY_NOT_FOUND);
            if (endpoint_id == target_endpoint_id)
            {
                // Target endpoint info found
                break;
            }
            prev_endpoint_info = endpoint_info;
            prev_endpoint      = endpoint_id;
            endpoint_id        = endpoint_info.next;
        } while (++endpoint_count < fabric_info.endpoint_count);
        VerifyOrReturnError(endpoint_id == target_endpoint_id, CHIP_ERROR_KEY_NOT_FOUND);

        // Target endpoint found

        chip::GroupId group_id   = endpoint_info.first_group;
        chip::GroupId prev_group = kUndefinedGroupId;
        GroupInfo group_info;

        // Remove endpoint's groups
        do
        {
            GroupMapStore group_store(fabric_index, endpoint_id, group_id);
            ReturnErrorOnFailure(group_store.Load(group_info));
            group_store.Delete();
            prev_group = group_id;
            group_id   = group_info.next;
        } while ((kUndefinedGroupId != group_id) && (prev_group != group_id));

        // endpoint_info.first_group = kUndefinedGroupId
        // EndpointStore(fabric_index, endpoint_id).Save(endpoint_info);

        // Remove endpoint info

        if (kInvalidEndpointId == prev_endpoint)
        {
            // First endpoint, update fabric info
            fabric_info.first_endpoint = endpoint_info.next;
            fabric_store.Save(fabric_info);
        }
        else
        {
            // Mid endpoint, update previous endpoint's info
            prev_endpoint_info.next = endpoint_info.next;
            EndpointStore(fabric_index, prev_endpoint).Save(prev_endpoint_info);
        }

        return EndpointStore(fabric_index, target_endpoint_id).Delete();
    }

    GroupMappingIterator * IterateGroupMappings(chip::FabricIndex fabric_index, EndpointId endpoint) override
    {
        VerifyOrReturnError(mInitialized, nullptr);
        return mEndpointIterators.CreateObject(*this, fabric_index, endpoint);
    }

    void Release(EndpointIterator * iterator) { mEndpointIterators.ReleaseObject(iterator); }

    //
    // Group States
    //

    CHIP_ERROR SetGroupState(size_t state_index, const GroupState & state) override
    {
        VerifyOrReturnError(mInitialized, CHIP_ERROR_INTERNAL);
        VerifyOrReturnError(IsFabricGroupId(state.group), CHIP_ERROR_INVALID_ARGUMENT);

        StateListStore states_store;
        StateListInfo states_info;

        if (CHIP_NO_ERROR != states_store.Load(states_info))
        {
            // First state
            VerifyOrReturnError(0 == state_index, CHIP_ERROR_INVALID_ARGUMENT);
            states_info.first = 1; // Arbitrary pseudo-index > 0
            states_info.count = 1;
            ReturnLogErrorOnFailure(
                StateStore(states_info.first).Save(StateInfo(state.fabric_index, state.group, state.keyset_index)));
            return states_store.Save(states_info);
        }

        // Pseudo-index > 0

        size_t pseudo_index = states_info.first;
        size_t actual_index = 0;
        size_t found_index  = 0;
        size_t new_index    = 1;
        size_t prev_index   = 0;
        StateInfo state_info;
        StateInfo prev_state;

        // Loop until the desired index
        while (actual_index < states_info.count)
        {
            if (CHIP_NO_ERROR != StateStore(pseudo_index).Load(state_info))
            {
                break;
            }
            if (new_index == pseudo_index)
            {
                // Used pseudo-index, keep looking
                new_index++;
            }
            if (actual_index == state_index)
            {
                // Target pseudo-index found
                found_index = pseudo_index;
                break;
            }
            prev_index   = pseudo_index;
            prev_state   = state_info;
            pseudo_index = state_info.next;
            actual_index++;
        }
        VerifyOrReturnError(state_index <= actual_index, CHIP_ERROR_INVALID_ARGUMENT);

        if (found_index > 0)
        {
            // Update existing state, must be in the same fabric
            VerifyOrReturnError(state_info.fabric_index == state.fabric_index, CHIP_ERROR_ACCESS_DENIED);
            state_info.group        = state.group;
            state_info.keyset_index = state.keyset_index;
            return StateStore(found_index).Save(state_info);
        }

        // New state
        ReturnErrorOnFailure(StateStore(new_index).Save(StateInfo(state.fabric_index, state.group, state.keyset_index)));
        if (prev_index > 0)
        {
            // New middle state, update previous
            prev_state.next = new_index;
            ReturnErrorOnFailure(StateStore(prev_index).Save(prev_state));
        }
        else
        {
            // New first state
            states_info.first = new_index;
        }
        // Update main list
        states_info.count = actual_index + 1;
        return states_store.Save(states_info);
    }

    CHIP_ERROR GetGroupState(size_t state_index, GroupState & state) override
    {
        VerifyOrReturnError(mInitialized, CHIP_ERROR_INTERNAL);

        StateListInfo states_info;
        VerifyOrReturnError(CHIP_NO_ERROR == StateListStore().Load(states_info), CHIP_ERROR_KEY_NOT_FOUND);

        // Fabric info found
        // VerifyOrReturnError(state_index <= fabric_info.state_count, CHIP_ERROR_INVALID_ARGUMENT);

        size_t pseudo_index = states_info.first;
        size_t actual_index = 0;
        size_t found_index  = 0;
        StateInfo state_info;

        // Loop until the desired index
        while (actual_index < states_info.count)
        {
            if (CHIP_NO_ERROR != StateStore(pseudo_index).Load(state_info))
            {
                break;
            }
            if (actual_index == state_index)
            {
                // Target pseudo-index found
                found_index        = pseudo_index;
                state.fabric_index = state_info.fabric_index;
                state.group        = state_info.group;
                state.keyset_index = state_info.keyset_index;
                return CHIP_NO_ERROR;
            }
            pseudo_index = state_info.next;
            actual_index++;
        }
        return CHIP_ERROR_KEY_NOT_FOUND;
    }

    CHIP_ERROR RemoveGroupState(size_t state_index) override
    {
        VerifyOrReturnError(mInitialized, CHIP_ERROR_INTERNAL);

        StateListStore states_store;
        StateListInfo states_info;

        VerifyOrReturnError(CHIP_NO_ERROR == states_store.Load(states_info), CHIP_ERROR_KEY_NOT_FOUND);

        size_t pseudo_index = states_info.first;
        size_t actual_index = 0;
        size_t found_index  = 0;
        size_t new_index    = 1;
        size_t prev_index   = 0;
        StateInfo state_info;
        StateInfo prev_state;

        // Loop until the desired index
        while (actual_index < states_info.count)
        {
            if (CHIP_NO_ERROR != StateStore(pseudo_index).Load(state_info))
            {
                break;
            }
            if (new_index == pseudo_index)
            {
                // Used pseudo-index, keep looking
                new_index++;
            }
            if (actual_index == state_index)
            {
                // Target pseudo-index found
                found_index = pseudo_index;
                break;
            }
            prev_index   = pseudo_index;
            prev_state   = state_info;
            pseudo_index = state_info.next;
            actual_index++;
        }

        VerifyOrReturnError(found_index > 0, CHIP_ERROR_KEY_NOT_FOUND);

        if (states_info.count > 0)
        {
            states_info.count--;
        }

        ReturnErrorOnFailure(StateStore(found_index).Delete());
        if (0 == prev_index)
        {
            // Remove first state
            states_info.first = state_info.next;
        }
        else
        {
            // Remove intermediate state
            prev_state.next = state_info.next;
            ReturnErrorOnFailure(StateStore(prev_index).Save(prev_state));
        }
        return states_store.Save(states_info);
    }

    GroupStateIterator * IterateGroupStates() override
    {
        VerifyOrReturnError(mInitialized, nullptr);
        return mAllStatesIterators.CreateObject(*this);
    }

    GroupStateIterator * IterateGroupStates(chip::FabricIndex fabric_index) override
    {
        VerifyOrReturnError(mInitialized, nullptr);
        return mFabricStatesIterators.CreateObject(*this, fabric_index);
    }

    void Release(AllStatesIterator * iterator) { mAllStatesIterators.ReleaseObject(iterator); }

    void Release(FabricStatesIterator * iterator) { mFabricStatesIterators.ReleaseObject(iterator); }

    //
    // Key Sets
    //

    CHIP_ERROR SetKeySet(chip::FabricIndex fabric_index, uint16_t target_id, const KeySet & keyset) override
    {
        VerifyOrReturnError(mInitialized, CHIP_ERROR_INTERNAL);

        FabricStore fabric_store(fabric_index);
        FabricInfo fabric_info;
        CHIP_ERROR err = fabric_store.Load(fabric_info);
        if (CHIP_NO_ERROR != err)
        {
            // First keyset
            fabric_info.first_keyset = target_id;
            KeySetInfo keyset_info(target_id, keyset.policy, keyset.num_keys_used);
            memcpy(keyset_info.epoch_keys, keyset.epoch_keys, sizeof(keyset.epoch_keys));
            ReturnLogErrorOnFailure(KeySetStore(fabric_index, target_id).Save(keyset_info));
            return fabric_store.Save(fabric_info);
        }

        // Pseudo-index > 0

        uint16_t keyset_id = fabric_info.first_keyset;
        uint16_t prev_id   = 0;
        KeySetInfo keyset_info;
        KeySetInfo prev_info;
        size_t keyset_count = 0;

        // Seach for the target keyset id
        while (keyset_count < fabric_info.keyset_count)
        {
            if (CHIP_NO_ERROR != KeySetStore(fabric_index, keyset_id).Load(keyset_info))
            {
                break;
            }
            if (keyset_id == target_id)
            {
                // Target id found
                break;
            }
            prev_id   = keyset_id;
            prev_info = keyset_info;
            keyset_id = keyset_info.next;
            keyset_count++;
        }

        keyset_info.keyset_id     = target_id;
        keyset_info.policy        = keyset.policy;
        keyset_info.num_keys_used = keyset.num_keys_used;
        memcpy(keyset_info.epoch_keys, keyset.epoch_keys, sizeof(keyset.epoch_keys));

        if (keyset_id == target_id)
        {
            // Update existing keyset info, keep next
            ReturnErrorOnFailure(KeySetStore(fabric_index, target_id).Save(keyset_info));
        }

        // New keyset

        keyset_info.next = 0;
        if (keyset_count > 0)
        {
            // Insert last
            fabric_info.keyset_count++;
            ReturnErrorOnFailure(KeySetStore(fabric_index, target_id).Save(keyset_info));
            // Update previous
            prev_info.next = target_id;
            ReturnErrorOnFailure(KeySetStore(fabric_index, prev_id).Save(prev_info));
        }
        else
        {
            // Insert first
            fabric_info.keyset_count = 1;
            fabric_info.first_keyset = target_id;
            ReturnErrorOnFailure(KeySetStore(fabric_index, target_id).Save(keyset_info));
            // Update fabric info
        }

        // Update fabric info
        return fabric_store.Save(fabric_info);
    }

    CHIP_ERROR GetKeySet(chip::FabricIndex fabric_index, uint16_t target_id, KeySet & keyset) override
    {
        VerifyOrReturnError(mInitialized, CHIP_ERROR_INTERNAL);

        FabricInfo fabric_info;
        VerifyOrReturnError(CHIP_NO_ERROR == FabricStore(fabric_index).Load(fabric_info), CHIP_ERROR_INVALID_FABRIC_ID);

        KeySetInfo keyset_info;
        uint16_t keyset_id  = fabric_info.first_keyset;
        size_t keyset_count = 0;

        // Loop until the desired index
        while (keyset_count < fabric_info.keyset_count)
        {
            if (CHIP_NO_ERROR != KeySetStore(fabric_index, keyset_id).Load(keyset_info))
            {
                break;
            }
            if (keyset_id == target_id)
            {
                // Target pseudo-index found
                keyset.keyset_id     = keyset_info.keyset_id;
                keyset.policy        = keyset_info.policy;
                keyset.num_keys_used = keyset_info.num_keys_used;
                memcpy(keyset.epoch_keys, keyset_info.epoch_keys, sizeof(keyset.epoch_keys));
                return CHIP_NO_ERROR;
            }
            keyset_id = keyset_info.next;
            keyset_count++;
        }

        return CHIP_ERROR_KEY_NOT_FOUND;
    }

    CHIP_ERROR RemoveKeySet(chip::FabricIndex fabric_index, uint16_t target_id) override
    {
        VerifyOrReturnError(mInitialized, CHIP_ERROR_INTERNAL);

        FabricStore fabric_store(fabric_index);
        FabricInfo fabric_info;

        CHIP_ERROR err = fabric_store.Load(fabric_info);
        VerifyOrReturnError(CHIP_NO_ERROR == err, CHIP_ERROR_KEY_NOT_FOUND);

        uint16_t keyset_id = fabric_info.first_keyset;
        uint16_t prev_id   = 0;
        KeySetInfo keyset_info;
        KeySetInfo prev_info;
        size_t keyset_count = 0;

        // Loop until the desired index
        while (keyset_count < fabric_info.keyset_count)
        {
            if (CHIP_NO_ERROR != KeySetStore(fabric_index, keyset_id).Load(keyset_info))
            {
                break;
            }
            if (keyset_id == target_id)
            {
                // Target pseudo-index found
                break;
            }
            prev_id   = keyset_id;
            prev_info = keyset_info;
            keyset_id = keyset_info.next;
            keyset_count++;
        }

        VerifyOrReturnError(keyset_id == target_id, CHIP_ERROR_KEY_NOT_FOUND);

        KeySetStore(fabric_index, target_id).Delete();
        if (keyset_count > 0)
        {
            // Remove intermediate keyset, update previous
            prev_info.next = keyset_info.next;
            ReturnErrorOnFailure(KeySetStore(fabric_index, prev_id).Save(prev_info));
        }
        else
        {
            // Remove first keyset
            fabric_info.first_keyset = keyset_info.next;
        }
        if (fabric_info.keyset_count > 0)
        {
            fabric_info.keyset_count--;
        }
        // Update fabric info
        return fabric_store.Save(fabric_info);
    }

    KeySetIterator * IterateKeySets(chip::FabricIndex fabric_index) override
    {
        VerifyOrReturnError(mInitialized, nullptr);
        return mKeySetIterators.CreateObject(*this, fabric_index);
    }

    void Release(KeySetIteratorImpl * iterator) { return mKeySetIterators.ReleaseObject(iterator); }

    CHIP_ERROR RemoveFabric(chip::FabricIndex fabric_index) override
    {
        FabricStore fabric_store(fabric_index);
        FabricInfo fabric_info;

        VerifyOrReturnError(CHIP_NO_ERROR == fabric_store.Load(fabric_info), CHIP_ERROR_KEY_NOT_FOUND);

        // Remove Group Mappings

        chip::EndpointId endpoint_id = fabric_info.first_endpoint;
        EndpointInfo endpoint_info;
        size_t endpoint_count = 0;

        do
        {
            EndpointStore endpoint_store(fabric_index, endpoint_id);
            if (CHIP_NO_ERROR != endpoint_store.Load(endpoint_info))
            {
                break;
            }
            RemoveAllGroupMappings(fabric_index, endpoint_id);
            endpoint_id = endpoint_info.next;
        } while (++endpoint_count < fabric_info.endpoint_count);

        // Remove States

        StateListStore states_store;
        StateListInfo states_info;
        StateInfo state_info;
        size_t next_state  = 0;
        size_t state_index = 0;

        // Load state list info
        states_store.Load(states_info);

        state_index = 0;
        next_state  = states_info.first;
        while ((state_index < states_info.count) && (next_state > 0))
        {
            if (CHIP_NO_ERROR != StateStore(next_state).Load(state_info))
            {
                break;
            }
            if (state_info.fabric_index == fabric_index)
            {
                // Removing the state shifts down the upper states
                RemoveGroupState(state_index);
            }
            else
            {
                state_index++;
            }
            next_state = state_info.next;
        }

        // Remove Keysets

        uint16_t keyset_id = fabric_info.first_keyset;
        KeySetInfo keyset_info;
        size_t keyset_count = 0;

        // Loop until the desired index
        while (keyset_count < fabric_info.keyset_count)
        {
            if (CHIP_NO_ERROR != KeySetStore(fabric_index, keyset_id).Load(keyset_info))
            {
                break;
            }
            RemoveKeySet(fabric_index, keyset_id);
            keyset_id = keyset_info.next;
            keyset_count++;
        }

        // Remove fabric
        fabric_store.Delete();

        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Decrypt(PacketHeader packetHeader, PayloadHeader & payloadHeader, System::PacketBufferHandle && msg) override
    {
        // TODO
        return CHIP_NO_ERROR;
    }

private:
    bool mInitialized = false;
    BitMapObjectPool<EndpointIterator, kIteratorsMax> mEndpointIterators;
    BitMapObjectPool<FabricStatesIterator, kIteratorsMax> mFabricStatesIterators;
    BitMapObjectPool<AllStatesIterator, kIteratorsMax> mAllStatesIterators;
    BitMapObjectPool<KeySetIteratorImpl, kIteratorsMax> mKeySetIterators;
};

GroupDataProviderImpl gDefaultGroupsProvider;

GroupDataProvider * gGroupsProvider = &gDefaultGroupsProvider;

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
