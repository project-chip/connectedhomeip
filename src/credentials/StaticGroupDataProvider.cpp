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
#include <lib/core/CHIPConfig.h>
#include <string.h>

using namespace chip::DeviceLayer;

namespace chip {
namespace Credentials {
namespace {

#ifndef CHIP_CONFIG_GROUP_DATA_FABRICS_MAX
#define CHIP_CONFIG_GROUP_DATA_FABRICS_MAX 4
#endif

#ifndef CHIP_CONFIG_GROUP_DATA_MAPPINGS_MAX
#define CHIP_CONFIG_GROUP_DATA_MAPPINGS_MAX 16
#endif

#ifndef CHIP_CONFIG_GROUP_DATA_STATES_MAX
#define CHIP_CONFIG_GROUP_DATA_STATES_MAX 16
#endif

#ifndef CHIP_CONFIG_GROUP_DATA_KEY_SETS_MAX
#define CHIP_CONFIG_GROUP_DATA_KEY_SETS_MAX 16
#endif

static constexpr size_t kGroupFabricsMax    = CHIP_CONFIG_GROUP_DATA_FABRICS_MAX;
static constexpr size_t kEndpointEntriesMax = CHIP_CONFIG_GROUP_DATA_MAPPINGS_MAX;
static constexpr size_t kStateEntriesMax    = CHIP_CONFIG_GROUP_DATA_STATES_MAX;
static constexpr size_t kKeyEntriesMax      = CHIP_CONFIG_GROUP_DATA_KEY_SETS_MAX;

class StaticGroupsProvider : public GroupDataProvider
{
    struct EndpointEntry : public GroupMapping
    {
        bool in_use = false;
    };

    struct StateEntry : public GroupState
    {
        bool in_use = false;
    };

    struct KeysEntry : public KeySet
    {
        bool in_use = false;
    };

    struct Fabric
    {
        bool in_use = false;
        chip::FabricIndex fabric_index;
        EndpointEntry endpoints[kEndpointEntriesMax];
        StateEntry states[kStateEntriesMax];
        KeysEntry keys[kKeyEntriesMax];
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
        bool HasNext() override
        {
            if (!mFirst)
            {
                this->mIndex++;
            }
            while (this->mFabric && this->mIndex < kEndpointEntriesMax)
            {
                EndpointEntry & entry = this->mFabric->endpoints[this->mIndex];
                if (entry.in_use && entry.endpoint == this->mEndpoint)
                {
                    mFirst = false;
                    return true;
                }
                this->mIndex++;
            }
            return false;
        }
        GroupId Next() const override { return kEndpointEntriesMax ? this->mFabric->endpoints[this->mIndex].group : 0; }

    private:
        Fabric * mFabric           = nullptr;
        chip::EndpointId mEndpoint = 0;
        uint16_t mIndex            = 0;
        bool mFirst                = true;
    };

    class StateIterator : public GroupStateIterator
    {
    public:
        StateIterator(Fabric * fabric) : mFabric(fabric) {}
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
        bool HasNext() override
        {
            if (!mFirst)
            {
                this->mIndex++;
            }
            while (this->mFabric && this->mIndex < kStateEntriesMax)
            {
                StateEntry & entry = this->mFabric->states[this->mIndex];
                if (entry.in_use)
                {
                    mFirst = false;
                    return true;
                }
                this->mIndex++;
            }
            return false;
        }
        const GroupState * Next() const override
        {
            return this->mIndex < kStateEntriesMax ? &this->mFabric->states[this->mIndex] : nullptr;
        }

    private:
        Fabric * mFabric = nullptr;
        uint16_t mIndex  = 0;
        bool mFirst      = true;
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
        bool HasNext() override
        {
            if (!mFirst)
            {
                this->mIndex++;
            }
            while (this->mFabric && this->mIndex < kKeyEntriesMax)
            {
                KeysEntry & entry = this->mFabric->keys[this->mIndex];
                if (entry.in_use)
                {
                    mFirst = false;
                    return true;
                }
                this->mIndex++;
            }
            return false;
        }
        const KeySet * Next() const override
        {
            return this->mIndex < kKeyEntriesMax ? &this->mFabric->keys[this->mIndex] : nullptr;
        }

    private:
        Fabric * mFabric = nullptr;
        uint16_t mIndex  = 0;
        bool mFirst      = true;
    };

    Fabric * GetFabric(chip::FabricIndex fabric_index)
    {
        Fabric * fabric = nullptr;
        Fabric * unused = nullptr;
        for (uint16_t i = 0; i < kGroupFabricsMax; i++)
        {
            fabric = &mFabrics[i];
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
        if (unused)
        {
            // Use the first available entry
            unused->fabric_index = fabric_index;
            unused->in_use       = true;
            return unused;
        }
        // Fabric not found, and no unused entries left
        return nullptr;
    }

public:
    CHIP_ERROR Init() override { return CHIP_NO_ERROR; }

    void Finish() override {}

    // Endpoints
    bool ExistsGroupMapping(chip::FabricIndex fabric_index, GroupMapping & mapping) override
    {
        Fabric * fabric = GetFabric(fabric_index);
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
        Fabric * fabric = GetFabric(fabric_index);
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
        Fabric * fabric = GetFabric(fabric_index);
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
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR RemoveGroupAllMappings(chip::FabricIndex fabric_index, EndpointId endpoint) override
    {
        Fabric * fabric = GetFabric(fabric_index);
        // Remove all mappings from fabric
        for (uint16_t i = 0; fabric && i < kEndpointEntriesMax; ++i)
        {
            fabric->endpoints[i].in_use = false;
        }
        return CHIP_NO_ERROR;
    }

    GroupMappingIterator * IterateGroupMappings(chip::FabricIndex fabric_index, EndpointId endpoint) override
    {
        return new EndpointIterator(GetFabric(fabric_index), endpoint);
    }

    // States
    CHIP_ERROR SetGroupState(chip::FabricIndex fabric_index, GroupState & state) override
    {
        Fabric * fabric    = GetFabric(fabric_index);
        StateEntry * entry = nullptr;

        // Search for existing, or unused entry
        for (uint16_t i = 0; fabric && i < kStateEntriesMax; ++i)
        {
            if (fabric->states[i].in_use)
            {
                if (fabric->states[i].state_index == state.state_index)
                {
                    // Reuse existing entry
                    entry = &fabric->states[i];
                    break;
                }
            }
            else if (!entry)
            {
                // Unused entry
                entry = &fabric->states[i];
            }
        }
        if (entry)
        {
            entry->state_index   = state.state_index;
            entry->group         = state.group;
            entry->key_set_index = state.key_set_index;
            entry->in_use        = true;
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_NO_MEMORY;
    }

    CHIP_ERROR GetGroupState(chip::FabricIndex fabric_index, GroupState & state) override
    {
        Fabric * fabric = GetFabric(fabric_index);
        // Search for existing state
        for (uint16_t i = 0; fabric && i < kStateEntriesMax; ++i)
        {
            StateEntry & entry = fabric->states[i];
            if (entry.in_use && entry.state_index == state.state_index)
            {
                // Found
                state.group         = entry.group;
                state.key_set_index = entry.key_set_index;
                return CHIP_NO_ERROR;
            }
        }
        return CHIP_ERROR_KEY_NOT_FOUND;
    }

    CHIP_ERROR RemoveGroupState(chip::FabricIndex fabric_index, uint16_t state_index) override
    {
        Fabric * fabric = GetFabric(fabric_index);
        // Search for existing state
        for (uint16_t i = 0; fabric && i < kStateEntriesMax; ++i)
        {
            StateEntry & entry = fabric->states[i];
            if (entry.in_use && entry.state_index == state_index)
            {
                // Found
                entry.in_use = false;
                return CHIP_NO_ERROR;
            }
        }
        return CHIP_ERROR_KEY_NOT_FOUND;
    }

    GroupStateIterator * IterateGroupStates(chip::FabricIndex fabric_index) override
    {
        return new StateIterator(GetFabric(fabric_index));
    }

    // Keys
    CHIP_ERROR SetKeySet(chip::FabricIndex fabric_index, KeySet & keys) override
    {
        Fabric * fabric   = GetFabric(fabric_index);
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
            memcpy(entry->epoch_keys, keys.epoch_keys, sizeof(entry->epoch_keys));
            entry->in_use = true;
            return CHIP_NO_ERROR;
        }
        return CHIP_ERROR_NO_MEMORY;
    }

    CHIP_ERROR GetKeySet(chip::FabricIndex fabric_index, KeySet & keys) override
    {
        Fabric * fabric = GetFabric(fabric_index);
        // Search for existing keys
        for (uint16_t i = 0; fabric && i < kKeyEntriesMax; ++i)
        {
            KeysEntry & entry = fabric->keys[i];
            if (entry.in_use && entry.key_set_index == keys.key_set_index)
            {
                // Found
                keys.policy = entry.policy;
                memcpy(keys.epoch_keys, entry.epoch_keys, sizeof(keys.epoch_keys));
                return CHIP_NO_ERROR;
            }
        }
        return CHIP_ERROR_KEY_NOT_FOUND;
    }

    CHIP_ERROR RemoveKeySet(chip::FabricIndex fabric_index, uint16_t key_set_index) override
    {
        Fabric * fabric = GetFabric(fabric_index);
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

    KeySetIterator * IterateKeySets(chip::FabricIndex fabric_index) override { return new KeysIterator(GetFabric(fabric_index)); }

private:
    Fabric mFabrics[kGroupFabricsMax];
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
