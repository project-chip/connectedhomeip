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
#include "GroupDataProvider.h"
#include <CHIPProjectConfig.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
// TODO: Start using pool w/ RAII to avoid heap
//#include <lib/support/Pool.h>
#include <string.h>
#include <stdlib.h>

namespace chip {
namespace Credentials {
namespace {

static constexpr size_t kNumFabrics    = CHIP_CONFIG_MAX_DEVICE_ADMINS;
static constexpr size_t kEndpointEntriesMax = CHIP_CONFIG_MAX_GROUP_ENDPOINTS_PER_FABRIC;
static constexpr size_t kStateEntriesMax    = CHIP_CONFIG_MAX_GROUPS_PER_FABRIC;
static constexpr size_t kKeyEntriesMax      = CHIP_CONFIG_MAX_GROUPS_PER_FABRIC;

// TODO: Use VerifyOrReturnError and the likes everywhere possible

class StaticGroupsProvider : public GroupDataProvider
{
protected:
    struct EndpointEntry : public GroupMapping
    {
        bool in_use = false;
    };

    struct StateEntry : public GroupState
    {
        bool in_use = false;
        void Clear()
        {
            in_use = false;
            memset(this, 0, sizeof(*this));
        }
    };

    struct KeysEntry : public KeySet
    {
        bool in_use = false;
    };

    struct Fabric
    {
        // Whether entry is allocated (true), or free to allocate (false)
        bool in_use = false;
        // Node-wide fabric index associated with the entry
        chip::FabricIndex fabric_index;
        // Index within the outer list, used to do index remapping
        uint8_t internal_index;
        // Group to Endpoint mapping for fabric
        EndpointEntry endpoints[kEndpointEntriesMax];
        // Group states for fabric
        StateEntry states[kStateEntriesMax];
        // Group key sets for fabric
        KeysEntry keys[kKeyEntriesMax];
    };

    // Structure to keep mapping between an outer list of entries, and the state
    // distributed across fabric-scoped inner lists.
    struct GroupStateIndex
    {
        // Index within mFabrics list (*not chip::FabricIndex*)
        uint8_t internal_fabric_index;
        // Index within the fabric
        uint8_t entry_index;

        bool IsValid() const
        {
            return (internal_fabric_index < kNumFabrics) && (static_cast<size_t>(entry_index) < kMaxNumGroupStates);
        }

        void Invalidate()
        {
            internal_fabric_index = UINT8_MAX;
            entry_index = 0;
        }

        static_assert(kNumFabrics < 255, "StaticGroupDataProvider only supports up to 255 fabrics");
        static_assert(kStateEntriesMax < 256, "StaticGroupDataProvider only supports up to 255 GroupState mappings per fabric");
    };

    class EndpointIterator : public GroupMappingIterator
    {
    public:
        EndpointIterator(Fabric * fabric, chip::EndpointId endpoint) : mFabric(fabric), mEndpoint(endpoint) {}

        uint16_t Count() override
        {
            uint16_t count = 0;
            for (uint16_t i = 0; this->mFabric && i < kEndpointEntriesMax; ++i)
            {
                EndpointEntry & entry = this->mFabric->endpoints[i];
                if (entry.in_use && entry.endpoint == this->mEndpoint)
                {
                    count++;
                }
            }
            return count;
        }

        GroupId Next() override
        {
            const EndpointEntry * entry = nullptr;
            bool entryFound = false;

            while (!entryFound && (this->mFabric != nullptr) && (this->mIndex < kEndpointEntriesMax))
            {
                entry = &this->mFabric->endpoints[this->mIndex];
                if (entry->in_use && (entry->endpoint == this->mEndpoint))
                {
                    entryFound = true;
                }
                ++this->mIndex;
            }

            return entryFound ? entry->group : 0;
        }

        void Release() override
        {
            chip::Platform::Delete(this);
        }

    private:
        Fabric * mFabric           = nullptr;
        chip::EndpointId mEndpoint = 0;
        uint16_t mIndex            = 0;
    };

    // TODO: Need to add a ListGroupState iterator that allows iteration through the full list of group state

    class FabricGroupStateIterator : public GroupStateIterator
    {
    public:
        FabricGroupStateIterator(Fabric * fabric) : mFabric(fabric) {}

        uint16_t Count() override
        {
            uint16_t count = 0;
            for (uint16_t i = 0; this->mFabric && i < kStateEntriesMax; ++i)
            {
                StateEntry & entry = this->mFabric->states[i];
                if (entry.in_use)
                {
                    count++;
                }
            }
            return count;
        }

        // TODO: Consider changing to `bool Next(GroupStateListEntry &outEntry)` so that caller can provide memory
        const GroupStateListEntry * Next() override
        {
            const StateEntry * entry = nullptr;
            bool entryFound = false;

            while (!entryFound && (this->mFabric != nullptr) && (this->mIndex < kStateEntriesMax))
            {
                entry = &this->mFabric->states[this->mIndex];
                if (entry->in_use)
                {
                    entryFound = true;
                }
                ++this->mIndex;
            }

            if (!entryFound)
            {
                return nullptr;
            }

            // Iterator has data available, copy the contents of the entry to the iterator slot
            mCurrentEntry.fabric_index = mFabric->fabric_index;
            mCurrentEntry.list_index = mOverallListIndex;
            mCurrentEntry.group = entry->group;
            mCurrentEntry.key_set_index = entry->key_set_index;

            ++mOverallListIndex;

            return &mCurrentEntry;
        }

        void Release() override
        {
            chip::Platform::Delete(this);
        }

    private:
        Fabric * mFabric = nullptr;
        uint16_t mIndex  = 0;
        uint16_t mOverallListIndex = 0;
        // Internal storage for current expanded entry (has additional index in addition to GroupState data)
        GroupStateListEntry mCurrentEntry{};
    };

    class KeysIterator : public KeySetIterator
    {
    public:
        KeysIterator(Fabric * fabric) : mFabric(fabric) {}

        uint16_t Count() override
        {
            uint16_t count = 0;
            for (uint16_t i = 0; this->mFabric && i < kKeyEntriesMax; ++i)
            {
                KeysEntry & entry = this->mFabric->keys[i];
                if (entry.in_use)
                {
                    count++;
                }
            }
            return count;
        }

        const KeySet * Next() override
        {
            const KeysEntry * entry = nullptr;
            bool entryFound = false;

            while (!entryFound && (this->mFabric != nullptr) && (this->mIndex < kKeyEntriesMax))
            {
                entry = &this->mFabric->keys[this->mIndex];
                if (entry->in_use)
                {
                    entryFound = true;
                }
                ++this->mIndex;
            }

            return entryFound ? entry : nullptr;
        }

        void Release() override
        {
            chip::Platform::Delete(this);
        }

    private:
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
        uint8_t unused_index = 0;

        for (uint8_t fabric_slot_idx = 0; fabric_slot_idx < kNumFabrics; fabric_slot_idx++)
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
                unused_index = fabric_slot_idx;
            }
        }
        if (unused && allow_allocate)
        {
            // Use the first available entry
            unused->internal_index = unused_index;
            unused->fabric_index = fabric_index;
            unused->in_use       = true;
            return unused;
        }
        // Fabric not found, and not allowed to allocate or out of entry
        return nullptr;
    }

    Fabric * GetExistingFabric(chip::FabricIndex fabric_index)
    {
        return GetFabric(fabric_index, /* allow_allocate= */ false);
    }

    Fabric * GetExistingFabricOrAllocateNew(chip::FabricIndex fabric_index)
    {
        return GetFabric(fabric_index, /* allow_allocate= */ true);
    }

public:
    CHIP_ERROR Init() override {
        // Clear-out all entries of index mapping.
        for (size_t idx; idx < kMaxNumGroupStates; ++idx)
        {
            mGroupStateIndices[idx].Invalidate();
        }
        return CHIP_NO_ERROR;
    }

    void Finish() override {}

    // Endpoints
    bool GroupMappingExists(chip::FabricIndex fabric_index, GroupMapping & mapping) override
    {
        // TODO: Add test for fabric_index lookup failure
        Fabric * fabric = GetExistingFabric(fabric_index);
        if (fabric == nullptr)
        {
            return false;
        }

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
        (void)name; // Unused!

        Fabric * fabric = GetExistingFabricOrAllocateNew(fabric_index);
        if (!fabric)
        {
            return CHIP_ERROR_NO_MEMORY;
        }
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
        Fabric * fabric = GetExistingFabric(fabric_index);
        if (fabric == nullptr)
        {
            return CHIP_ERROR_INVALID_FABRIC_ID;
        }

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

        // It is not an error to ask for a removal of a group mapping.
        // TODO: Determine if this should be CHIP_ERROR_NOT_FOUND.
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR RemoveAllGroupMappings(chip::FabricIndex fabric_index, EndpointId endpoint) override
    {
        Fabric * fabric = GetExistingFabric(fabric_index);
        if (fabric == nullptr)
        {
            return CHIP_ERROR_INVALID_FABRIC_ID;
        }

        // Remove all mappings from fabric
        for (uint16_t i = 0; fabric && i < kEndpointEntriesMax; ++i)
        {
            fabric->endpoints[i].in_use = false;
        }
        return CHIP_NO_ERROR;
    }

    GroupMappingIterator * IterateGroupMappings(chip::FabricIndex fabric_index, EndpointId endpoint) override
    {
        return chip::Platform::New<EndpointIterator>(GetExistingFabric(fabric_index), endpoint);
    }

    // States

    // TODO: add flags to do fabric-limited versus full list access
    // TODO: Consider other internal consistency constraints
    CHIP_ERROR SetGroupState(chip::FabricIndex fabric_index, uint16_t state_index, const GroupState & state) override
    {
        bool appending = false;
        // Append is "add to one past the end". Further than that is not supported.
        if (static_cast<size_t>(state_index) > mGroupStateEntriesUsed)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        else if (static_cast<size_t>(state_index) == mGroupStateEntriesUsed)
        {
            // Need to append to end, which impacts indexing
            appending = true;
        }

        Fabric * fabric = GetExistingFabricOrAllocateNew(fabric_index);
        if (fabric == nullptr)
        {
            return CHIP_ERROR_NO_MEMORY;
        }

        StateEntry * entry = nullptr;
        uint8_t entry_idx;

        // TODO: Add test for append
        // Case 1: appending a new entry altogether
        if (appending)
        {
            // Search for existing, or unused entry in associated fabric
            for (entry_idx = 0; entry_idx < kStateEntriesMax; ++entry_idx)
            {
                if (!fabric->states[entry_idx].in_use)
                {
                    // Found Unused entry
                    entry = &fabric->states[entry_idx];
                    break;
                }
            }
        }
        else
        {
            // TODO: test both correct and incorrect fabric
            // Case 2: replacing an existing entry's contents
            GroupStateIndex candidate_mapping = mGroupStateIndices[state_index];

            if (!candidate_mapping.IsValid())
            {
                // Should not happen that our list is inconsistent!
                return CHIP_ERROR_INTERNAL;
            }
            entry_idx = candidate_mapping.entry_index;

            Fabric * candidate_fabric = &mFabrics[candidate_mapping.internal_fabric_index];
            if (!candidate_fabric->in_use || (candidate_fabric->fabric_index != fabric_index))
            {
                // Tried to overwrite another fabric's entry
                return CHIP_ERROR_ACCESS_DENIED;
            }

            entry = &candidate_fabric->states[entry_idx];
            if (!entry->in_use)
            {
                // Should not happen that our list is inconsistent!
                return CHIP_ERROR_INTERNAL;
            }
        }

        // Now entry and entry_idx both set properly, or entry is null, so we can continue
        if (entry == nullptr)
        {
            return CHIP_ERROR_NO_MEMORY;
        }

        // Set entry contents proper
        entry->group         = state.group;
        entry->key_set_index = state.key_set_index;
        entry->in_use        = true;

        // Record global index mapping in the groups state list
        GroupStateIndex index_mapping;
        index_mapping.internal_fabric_index = fabric->internal_index;
        index_mapping.entry_index = entry_idx;
        mGroupStateIndices[state_index] = index_mapping;
        if (appending)
        {
            ++mGroupStateEntriesUsed;
        }

        // TODO: add listener updates calls
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetGroupState(chip::FabricIndex fabric_index, uint16_t state_index, GroupStateListEntry & state) override
    {
        if (static_cast<size_t>(state_index) >= mGroupStateEntriesUsed)
        {
            return CHIP_ERROR_KEY_NOT_FOUND;
        }

        // TODO: For read, fabric_index is ignored for now, until we have fabric-limited read
        (void)fabric_index;

        // Get entry mapping from index cache
        GroupStateIndex index_mapping = mGroupStateIndices[state_index];

        if (!index_mapping.IsValid())
        {
            // Should not happen that our list is inconsistent!
            return CHIP_ERROR_INTERNAL;
        }

        uint16_t entry_idx = index_mapping.entry_index;
        const Fabric * fabric = &mFabrics[index_mapping.internal_fabric_index];
        const StateEntry & entry = fabric->states[entry_idx];

        if (!fabric->in_use || !entry.in_use)
        {
            // Should not happen that mapped fabric is not allocated!
            return CHIP_ERROR_INTERNAL;
        }

        // Update output mapping entry
        state.group = entry.group;
        state.key_set_index = entry.key_set_index;
        state.fabric_index = fabric->fabric_index;
        state.list_index = state_index;

        return CHIP_NO_ERROR;
    }

    CHIP_ERROR RemoveGroupState(chip::FabricIndex fabric_index, uint16_t state_index) override
    {
        if (static_cast<size_t>(state_index) >= mGroupStateEntriesUsed)
        {
            return CHIP_ERROR_KEY_NOT_FOUND;
        }

        GroupStateIndex candidate_mapping = mGroupStateIndices[state_index];
        uint16_t entry_idx = 0;
        StateEntry * entry = nullptr;
        // TODO: Refactor common checks bellow to a utility
        {
            if (!candidate_mapping.IsValid())
            {
                // Should not happen that our list is inconsistent!
                return CHIP_ERROR_INTERNAL;
            }

            entry_idx = candidate_mapping.entry_index;
            Fabric * fabric = &mFabrics[candidate_mapping.internal_fabric_index];
            if (!fabric->in_use || (fabric->fabric_index != fabric_index))
            {
                // Tried to overwrite another fabric's entry
                return CHIP_ERROR_ACCESS_DENIED;
            }

            entry = &fabric->states[entry_idx];
            if (!entry->in_use)
            {
                // Should not happen that removed entry is not allocated!
                return CHIP_ERROR_INTERNAL;
            }
        }

        // `entry` now points to a concrete entry to mark unused.

        // Shift rest of the list up

        // Mark entry unused in fabric
        entry->Clear();
        mGroupStateIndices[state_index].Invalidate();

        --mGroupStateEntriesUsed;

        size_t num_entries_to_shift = mGroupStateEntriesUsed - state_index;
        if (num_entries_to_shift > 0)
        {
            memmove(&mGroupStateIndices[state_index], &mGroupStateIndices[state_index + 1], num_entries_to_shift * sizeof(mGroupStateIndices[0]));
            // Invalidate entry one past the end of whole used space of table after shift, if any shift occured.
            // This is because the shift left free space at the end of the table which is a copy of the
            // very last entry previously there, and this would break free-space checking assumptions if not
            // fixed-up that all unused entries at the end are invalid
            mGroupStateIndices[mGroupStateEntriesUsed].Invalidate();
        }

        return CHIP_NO_ERROR;
    }

    GroupStateIterator * IterateGroupStates(chip::FabricIndex fabric_index) override
    {
        return chip::Platform::New<FabricGroupStateIterator>(GetExistingFabric(fabric_index));
    }

    // Keys
    CHIP_ERROR SetKeySet(chip::FabricIndex fabric_index, KeySet & keys) override
    {
        Fabric * fabric   = GetExistingFabric(fabric_index);
        KeysEntry * entry = nullptr;

        // Search for existing, or unused entry
        for (uint16_t i = 0; fabric && i < kKeyEntriesMax; ++i)
        {
            if (fabric->keys[i].in_use)
            {
                if (fabric->keys[i].key_set_index == keys.key_set_index)
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
            entry->key_set_index = keys.key_set_index;
            entry->policy        = keys.policy;
            entry->num_keys_used = keys.num_keys_used;
            memcpy(entry->epoch_keys, keys.epoch_keys, sizeof(keys.epoch_keys[0]) * keys.num_keys_used);
            entry->in_use = true;
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_NO_MEMORY;
    }

    CHIP_ERROR GetKeySet(chip::FabricIndex fabric_index, KeySet & keys) override
    {
        Fabric * fabric = GetExistingFabric(fabric_index);
        // Search for existing keys
        for (uint16_t i = 0; fabric && i < kKeyEntriesMax; ++i)
        {
            KeysEntry & entry = fabric->keys[i];
            if (entry.in_use && entry.key_set_index == keys.key_set_index)
            {
                // Found
                keys.policy = entry.policy;
                keys.num_keys_used = entry.num_keys_used;
                memcpy(keys.epoch_keys, entry.epoch_keys, sizeof(keys.epoch_keys[0]) * keys.num_keys_used);
                return CHIP_NO_ERROR;
            }
        }
        return CHIP_ERROR_KEY_NOT_FOUND;
    }

    CHIP_ERROR RemoveKeySet(chip::FabricIndex fabric_index, uint16_t key_set_index) override
    {
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
        return chip::Platform::New<KeysIterator>(GetExistingFabric(fabric_index));
    }

private:
    Fabric mFabrics[kNumFabrics];
    static constexpr size_t kMaxNumGroupStates = kNumFabrics * kStateEntriesMax;
    GroupStateIndex mGroupStateIndices[kMaxNumGroupStates];
    size_t mGroupStateEntriesUsed = 0;
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
