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
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Pool.h>
#include <stdlib.h>
#include <string.h>

namespace chip {
namespace Credentials {
namespace {

static constexpr size_t kNumFabrics         = CHIP_CONFIG_MAX_DEVICE_ADMINS;
static constexpr size_t kEndpointEntriesMax = CHIP_CONFIG_MAX_GROUP_ENDPOINTS_PER_FABRIC;
static constexpr size_t kStateEntriesMax    = CHIP_CONFIG_MAX_GROUPS_PER_FABRIC;
static constexpr size_t kKeyEntriesMax      = CHIP_CONFIG_MAX_GROUPS_PER_FABRIC;
static constexpr size_t kIteratorsMax       = CHIP_CONFIG_MAX_GROUP_CONCURRENT_ITERATORS;

class StaticGroupsProvider : public GroupDataProvider
{
protected:
    struct EndpointEntry : public GroupMapping
    {
        bool in_use = false;
    };

    struct KeysEntry : public KeySet
    {
        bool in_use = false;
        uint16_t key_set_index;
    };

    struct Fabric
    {
        // Whether entry is allocated (true), or free to allocate (false)
        bool in_use = false;
        // Node-wide fabric index associated with the entry
        chip::FabricIndex fabric_index;
        // Group to Endpoint mapping for fabric
        EndpointEntry endpoints[kEndpointEntriesMax];
        // Number of Group states for this fabric
        size_t states_count;
        // Group key sets for fabric
        KeysEntry keys[kKeyEntriesMax];
    };

    struct StateEntry : public GroupState
    {
        Fabric * fabric = nullptr;
        bool in_use     = false;
        void Clear()
        {
            fabric_index  = 0;
            group         = 0;
            key_set_index = 0;
            fabric        = nullptr;
            in_use        = false;
        }
    };

    class EndpointIterator : public GroupMappingIterator
    {
    public:
        EndpointIterator(StaticGroupsProvider & provider, Fabric * fabric, chip::EndpointId endpoint) :
            mProvider(provider), mFabric(fabric), mEndpoint(endpoint)
        {}

        size_t Count() override
        {
            size_t count = 0;
            for (size_t i = 0; this->mFabric && i < kEndpointEntriesMax; ++i)
            {
                const EndpointEntry & entry = this->mFabric->endpoints[i];
                if (entry.in_use && entry.endpoint == this->mEndpoint)
                {
                    count++;
                }
            }
            return count;
        }

        bool Next(GroupId & outGroup) override
        {
            while ((this->mFabric != nullptr) && (this->mIndex < kEndpointEntriesMax))
            {
                const EndpointEntry & entry = this->mFabric->endpoints[this->mIndex++];
                if (entry.in_use && (entry.endpoint == this->mEndpoint))
                {
                    outGroup = entry.group;
                    return true;
                }
            }

            return false;
        }

        void Release() override { mProvider.Release(this); }

    private:
        StaticGroupsProvider & mProvider;
        Fabric * mFabric           = nullptr;
        chip::EndpointId mEndpoint = 0;
        uint16_t mIndex            = 0;
    };

    class FabricGroupStateIterator : public GroupStateIterator
    {
    public:
        FabricGroupStateIterator(StaticGroupsProvider & provider, Fabric * fabric) : mProvider(provider), mFabric(fabric) {}

        size_t Count() override
        {
            size_t count = 0;
            for (size_t i = 0; i < kMaxNumGroupStates && this->mProvider.mGroupStatesCount; ++i)
            {
                const StateEntry & entry = this->mProvider.mGroupStates[i];
                if (entry.in_use && entry.fabric == mFabric)
                {
                    count++;
                }
            }
            return count;
        }

        bool Next(GroupState & outEntry) override
        {
            while ((this->mFabric != nullptr) && (mIndex < kMaxNumGroupStates) && (mIndex < this->mProvider.mGroupStatesCount))
            {
                const StateEntry & entry = this->mProvider.mGroupStates[mIndex++];
                if (entry.in_use && entry.fabric == mFabric)
                {
                    // Iterator has data available, copy the contents of the entry to the output
                    outEntry = entry;
                    return true;
                }
            }
            return false;
        }

        void Release() override { mProvider.Release(this); }

    private:
        StaticGroupsProvider & mProvider;
        Fabric * mFabric = nullptr;
        uint16_t mIndex  = 0;
    };

    class AllGroupStateIterator : public GroupStateIterator
    {
    public:
        AllGroupStateIterator(StaticGroupsProvider & provider) : mProvider(provider) {}

        size_t Count() override
        {
            size_t count = 0;
            for (size_t i = 0; i < kMaxNumGroupStates && this->mProvider.mGroupStatesCount; ++i)
            {
                const StateEntry & entry = this->mProvider.mGroupStates[i];
                if (entry.in_use)
                {
                    count++;
                }
            }
            return count;
        }

        bool Next(GroupState & outEntry) override
        {
            while ((mIndex < kMaxNumGroupStates) && (mIndex < this->mProvider.mGroupStatesCount))
            {
                const StateEntry & entry = this->mProvider.mGroupStates[mIndex++];
                if (entry.in_use)
                {
                    // Iterator has data available, copy the contents of the entry to the output
                    outEntry = entry;
                    return true;
                }
            }
            return false;
        }

        void Release() override { mProvider.Release(this); }

    private:
        StaticGroupsProvider & mProvider;
        uint16_t mIndex = 0;
    };

    class KeysIterator : public KeySetIterator
    {
    public:
        KeysIterator(StaticGroupsProvider & provider, Fabric * fabric) : mProvider(provider), mFabric(fabric) {}

        size_t Count() override
        {
            size_t count = 0;
            for (size_t i = 0; this->mFabric && i < kKeyEntriesMax; ++i)
            {
                const KeysEntry & entry = this->mFabric->keys[i];
                if (entry.in_use)
                {
                    count++;
                }
            }
            return count;
        }

        bool Next(KeySet & outSet) override
        {
            while ((this->mFabric != nullptr) && (this->mIndex < kKeyEntriesMax))
            {
                const KeysEntry & entry = this->mFabric->keys[this->mIndex++];
                if (entry.in_use)
                {
                    outSet.key_set_index = entry.key_set_index;
                    outSet.policy        = entry.policy;
                    outSet.num_keys_used = entry.num_keys_used;
                    memcpy(outSet.epoch_keys, entry.epoch_keys, sizeof(outSet.epoch_keys));
                    return true;
                }
            }
            return false;
        }

        void Release() override { mProvider.Release(this); }

    private:
        StaticGroupsProvider & mProvider;
        Fabric * mFabric = nullptr;
        uint16_t mIndex  = 0;
    };

    // Get the fabric-scoped dataset slot for the given `fabric_index`.
    // If `allow_allocate` is true, a fabric index not found will mark a slot as allocated
    // to that given fabric_index, otherwise only lookup of existing fabric slots is possible.
    // If no slot is found matching the `fabric_index`, nullptr is returned.
    Fabric * GetFabric(chip::FabricIndex fabric_index, bool allow_allocate)
    {
        Fabric * fabric = nullptr;
        Fabric * unused = nullptr;

        for (chip::FabricIndex fabric_slot_idx = 0; fabric_slot_idx < kNumFabrics; fabric_slot_idx++)
        {
            fabric = &mFabrics[fabric_slot_idx];
            if (fabric->in_use)
            {
                if (fabric->fabric_index == fabric_index)
                {
                    // Fabric in use
                    return fabric;
                }
            }
            else if (nullptr == unused)
            {
                // Remember the first unused entry
                unused = fabric;
            }
        }
        if (unused && allow_allocate)
        {
            // Use the first available entry
            unused->fabric_index = fabric_index;
            unused->in_use       = true;
            return unused;
        }
        // Fabric not found, and not allowed to allocate or out of entry
        return nullptr;
    }

    Fabric * GetExistingFabric(chip::FabricIndex fabric_index) { return GetFabric(fabric_index, /* allow_allocate= */ false); }

    Fabric * GetExistingFabricOrAllocateNew(chip::FabricIndex fabric_index)
    {
        return GetFabric(fabric_index, /* allow_allocate= */ true);
    }

public:
    CHIP_ERROR Init() override
    {
        // Clear-out all entries of index mapping.
        for (size_t i = 0; i < kMaxNumGroupStates; ++i)
        {
            mGroupStates[i].Clear();
        }
        mInitialized = true;
        return CHIP_NO_ERROR;
    }

    void Finish() override { mInitialized = false; }

    // Endpoints
    bool GroupMappingExists(chip::FabricIndex fabric_index, GroupMapping & mapping) override
    {
        VerifyOrReturnError(mInitialized, false);

        Fabric * fabric = GetExistingFabric(fabric_index);
        VerifyOrReturnError(fabric, false);

        for (uint16_t i = 0; fabric && i < kEndpointEntriesMax; ++i)
        {
            EndpointEntry & entry = fabric->endpoints[i];
            if (entry.in_use && (entry == mapping))
            {
                return true;
            }
        }
        return false;
    }

    CHIP_ERROR AddGroupMapping(chip::FabricIndex fabric_index, GroupMapping & mapping, const char * name) override
    {
        VerifyOrReturnError(mInitialized, CHIP_ERROR_INTERNAL);

        (void) name; // Unused!

        Fabric * fabric = GetExistingFabricOrAllocateNew(fabric_index);
        VerifyOrReturnError(fabric, CHIP_ERROR_NO_MEMORY);

        // Search for existing mapping
        for (uint16_t i = 0; i < kEndpointEntriesMax; ++i)
        {
            EndpointEntry & entry = fabric->endpoints[i];
            if (entry.in_use && (entry == mapping))
            {
                // Duplicated
                return CHIP_NO_ERROR;
            }
        }
        // New mapping
        for (uint16_t i = 0; i < kEndpointEntriesMax; ++i)
        {
            EndpointEntry & entry = fabric->endpoints[i];
            if (!entry.in_use)
            {
                entry.group    = mapping.group;
                entry.endpoint = mapping.endpoint;
                entry.in_use   = true;
                return CHIP_NO_ERROR;
            }
        }
        return CHIP_ERROR_NO_MEMORY;
    }

    CHIP_ERROR RemoveGroupMapping(chip::FabricIndex fabric_index, GroupMapping & mapping) override
    {
        VerifyOrReturnError(mInitialized, CHIP_ERROR_INTERNAL);

        Fabric * fabric = GetExistingFabric(fabric_index);
        VerifyOrReturnError(fabric, CHIP_ERROR_INVALID_FABRIC_ID);

        // Search for existing mapping
        for (uint16_t i = 0; fabric && i < kEndpointEntriesMax; ++i)
        {
            EndpointEntry & entry = fabric->endpoints[i];
            if (entry.in_use && (entry == mapping))
            {
                // Found
                entry.in_use = false;
                return CHIP_NO_ERROR;
            }
        }

        return CHIP_ERROR_KEY_NOT_FOUND;
    }

    CHIP_ERROR RemoveAllGroupMappings(chip::FabricIndex fabric_index) override
    {
        VerifyOrReturnError(mInitialized, CHIP_ERROR_INTERNAL);

        Fabric * fabric = GetExistingFabric(fabric_index);
        VerifyOrReturnError(fabric, CHIP_ERROR_INVALID_FABRIC_ID);

        // Remove all mappings from fabric
        for (uint16_t i = 0; fabric && i < kEndpointEntriesMax; ++i)
        {
            fabric->endpoints[i].in_use = false;
        }
        return CHIP_NO_ERROR;
    }

    GroupMappingIterator * IterateGroupMappings(chip::FabricIndex fabric_index, EndpointId endpoint) override
    {
        VerifyOrReturnError(mInitialized, nullptr);
        return mEndpointIterators.CreateObject(*this, GetExistingFabric(fabric_index), endpoint);
    }

    void Release(EndpointIterator * iterator) { mEndpointIterators.ReleaseObject(iterator); }

    // States

    CHIP_ERROR SetGroupState(uint16_t state_index, const GroupState & state) override
    {
        VerifyOrReturnError(mInitialized, CHIP_ERROR_INTERNAL);

        // Append is "add to one past the end". Further than that is not supported.
        VerifyOrReturnError(static_cast<size_t>(state_index) <= mGroupStatesCount, CHIP_ERROR_INVALID_ARGUMENT);

        Fabric * fabric = GetExistingFabricOrAllocateNew(state.fabric_index);
        VerifyOrReturnError(fabric, CHIP_ERROR_NO_MEMORY);

        StateEntry & entry = mGroupStates[state_index];
        bool appending     = static_cast<size_t>(state_index) == mGroupStatesCount;

        if (appending)
        {
            // New entry, append at the end, limiting the number of entries per fabric
            VerifyOrReturnError(fabric->states_count < kStateEntriesMax, CHIP_ERROR_NO_MEMORY);
            mGroupStatesCount++;
            fabric->states_count++;
        }
        else
        {
            // Existing entry, avoid overwrite another fabric's entry
            VerifyOrReturnError(mGroupStates[state_index].fabric_index == state.fabric_index, CHIP_ERROR_ACCESS_DENIED);
        }

        GroupState old_entry = entry;
        entry.fabric_index   = state.fabric_index;
        entry.group          = state.group;
        entry.key_set_index  = state.key_set_index;
        entry.fabric         = fabric;
        entry.in_use         = true;

        if (mListener)
        {
            mListener->OnGroupStateChanged((appending ? nullptr : &old_entry), &entry);
        }

        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetGroupState(uint16_t state_index, GroupState & state) override
    {
        VerifyOrReturnError(mInitialized, CHIP_ERROR_INTERNAL);
        VerifyOrReturnError(static_cast<size_t>(state_index) < mGroupStatesCount, CHIP_ERROR_KEY_NOT_FOUND);

        StateEntry & entry = mGroupStates[state_index];

        // Should not happen that mapped fabric is not allocated!
        VerifyOrReturnError(entry.fabric && entry.fabric->in_use && entry.in_use, CHIP_ERROR_INTERNAL);

        // Update output mapping entry
        state = mGroupStates[state_index];

        return CHIP_NO_ERROR;
    }

    CHIP_ERROR RemoveGroupState(uint16_t state_index) override
    {
        VerifyOrReturnError(mInitialized, CHIP_ERROR_INTERNAL);
        VerifyOrReturnError(static_cast<size_t>(state_index) < mGroupStatesCount, CHIP_ERROR_KEY_NOT_FOUND);

        StateEntry & entry = mGroupStates[state_index];

        // Should not happen that mapped fabric is not allocated!
        VerifyOrReturnError(entry.fabric && entry.fabric->in_use && entry.fabric->states_count > 0 && entry.in_use,
                            CHIP_ERROR_INTERNAL);

        GroupState old_entry = entry;

        // Shift rest of the list up

        // Mark entry unused in fabric
        entry.fabric->states_count--;
        entry.Clear();

        --mGroupStatesCount;

        size_t num_entries_to_shift = mGroupStatesCount - state_index;
        if (num_entries_to_shift > 0)
        {
            memmove(&mGroupStates[state_index], &mGroupStates[state_index + 1], num_entries_to_shift * sizeof(mGroupStates[0]));
            // Invalidate entry one past the end of whole used space of table after shift, if any shift occured.
            // This is because the shift left free space at the end of the table which is a copy of the
            // very last entry previously there, and this would break free-space checking assumptions if not
            // fixed-up that all unused entries at the end are invalid
            mGroupStates[mGroupStatesCount].Clear();
        }

        if (mListener)
        {
            mListener->OnGroupStateRemoved(&old_entry);
        }
        return CHIP_NO_ERROR;
    }

    GroupStateIterator * IterateGroupStates() override
    {
        VerifyOrReturnError(mInitialized, nullptr);
        return mAllStateIterators.CreateObject(*this);
    }

    GroupStateIterator * IterateGroupStates(chip::FabricIndex fabric_index) override
    {
        VerifyOrReturnError(mInitialized, nullptr);
        return mFabricStateIterators.CreateObject(*this, GetExistingFabric(fabric_index));
    }

    void Release(FabricGroupStateIterator * iterator) { mFabricStateIterators.ReleaseObject(iterator); }

    void Release(AllGroupStateIterator * iterator) { mAllStateIterators.ReleaseObject(iterator); }

    // Keys
    CHIP_ERROR SetKeySet(chip::FabricIndex fabric_index, uint16_t key_set_index, KeySet & keys) override
    {
        VerifyOrReturnError(mInitialized, CHIP_ERROR_INTERNAL);

        Fabric * fabric   = GetExistingFabric(fabric_index);
        KeysEntry * entry = nullptr;

        // Search for existing, or unused entry
        for (uint16_t i = 0; fabric && i < kKeyEntriesMax; ++i)
        {
            if (fabric->keys[i].in_use)
            {
                if (fabric->keys[i].key_set_index == key_set_index)
                {
                    // Reuse existing entry
                    entry = &fabric->keys[i];
                    break;
                }
            }
            else if (!entry)
            {
                // Unused entry
                entry = &fabric->keys[i];
            }
        }
        if (entry)
        {
            entry->key_set_index = key_set_index;
            entry->policy        = keys.policy;
            entry->num_keys_used = keys.num_keys_used;
            memcpy(entry->epoch_keys, keys.epoch_keys, sizeof(keys.epoch_keys[0]) * keys.num_keys_used);
            entry->in_use = true;
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_NO_MEMORY;
    }

    CHIP_ERROR GetKeySet(chip::FabricIndex fabric_index, uint16_t key_set_index, KeySet & keys) override
    {
        VerifyOrReturnError(mInitialized, CHIP_ERROR_INTERNAL);
        Fabric * fabric = GetExistingFabric(fabric_index);
        // Search for existing keys
        for (uint16_t i = 0; fabric && i < kKeyEntriesMax; ++i)
        {
            KeysEntry & entry = fabric->keys[i];
            if (entry.in_use && entry.key_set_index == key_set_index)
            {
                // Found
                keys.policy        = entry.policy;
                keys.num_keys_used = entry.num_keys_used;
                memcpy(keys.epoch_keys, entry.epoch_keys, sizeof(keys.epoch_keys[0]) * keys.num_keys_used);
                return CHIP_NO_ERROR;
            }
        }
        return CHIP_ERROR_KEY_NOT_FOUND;
    }

    CHIP_ERROR RemoveKeySet(chip::FabricIndex fabric_index, uint16_t key_set_index) override
    {
        VerifyOrReturnError(mInitialized, CHIP_ERROR_INTERNAL);
        Fabric * fabric = GetExistingFabric(fabric_index);
        // Search for existing keys
        for (uint16_t i = 0; fabric && i < kKeyEntriesMax; ++i)
        {
            KeysEntry & entry = fabric->keys[i];
            if (entry.in_use && entry.key_set_index == key_set_index)
            {
                // Found
                entry.in_use = false;
                return CHIP_NO_ERROR;
            }
        }
        return CHIP_ERROR_KEY_NOT_FOUND;
    }

    KeySetIterator * IterateKeySets(chip::FabricIndex fabric_index) override
    {
        VerifyOrReturnError(mInitialized, nullptr);
        return mKeyIterators.CreateObject(*this, GetExistingFabric(fabric_index));
    }

    void Release(KeysIterator * iterator) { return mKeyIterators.ReleaseObject(iterator); }

private:
    bool mInitialized = false;
    Fabric mFabrics[kNumFabrics];
    BitMapObjectPool<KeysIterator, kIteratorsMax> mKeyIterators;
    BitMapObjectPool<EndpointIterator, kIteratorsMax> mEndpointIterators;
    BitMapObjectPool<FabricGroupStateIterator, kIteratorsMax> mFabricStateIterators;
    BitMapObjectPool<AllGroupStateIterator, kIteratorsMax> mAllStateIterators;
    static constexpr size_t kMaxNumGroupStates = kNumFabrics * kStateEntriesMax;
    StateEntry mGroupStates[kMaxNumGroupStates];
    size_t mGroupStatesCount = 0;
};

StaticGroupsProvider gDefaultGroupsProvider;

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
