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
#pragma once

#include <app/util/basic-types.h>
#include <lib/core/CHIPError.h>
#include <stdint.h>
#include <sys/types.h>

namespace chip {
namespace Credentials {

class GroupDataProvider
{
public:
    // An EpochKey is a single key usable to determine an operational group key
    struct EpochKey
    {
        static constexpr size_t kLengthBytes = (128 / 8);
        // Validity start time in microseconds since 2000-01-01T00:00:00 UTC ("the Epoch")
        uint64_t start_time;
        // Actual key bits. Depending on context, it may be a raw epoch key (as seen within `SetKeySet` calls)
        // or it may be the derived operational group key (as seen in any other usage).
        uint8_t key[kLengthBytes];
    };

    // A GroupMapping maps a controlling GroupId to a given EndpointId. There may be
    // multiple GroupMapping having the same `group` value, but each with different
    // `endpoint` value.
    struct GroupMapping
    {
        // The endpoint to which a GroupId is mapped.
        EndpointId endpoint;
        // The GroupId, which, when received in a message will map the the `endpoint`.
        GroupId group;

        bool operator==(const GroupMapping & other) { return this->endpoint == other.endpoint && this->group == other.group; }

        GroupMapping & operator=(const GroupMapping & other)
        {
            if (this == &other)
                return *this; // Guard self assignment
            this->endpoint = other.endpoint;
            this->group    = other.group;
            return *this;
        }
    };

    // A group state maps the group key set to use for encryption/decryption for a given group.
    struct GroupState
    {
        chip::GroupId group;
        uint16_t key_set_index;

        bool operator==(const GroupState & other)
        {
            return this->group == other.group && this->key_set_index == other.key_set_index;
        }
    };

    // A group state list entry has the data for a list read operation, including during iteration.
    struct GroupStateListEntry : public GroupState
    {
        // Fabric Index associated with the group state entry's fabric scoping
        chip::FabricIndex fabric_index;
        // Positional index within the overall list.
        uint16_t list_index;
    };

    // A operational group key set, usable by many GroupState mappings
    struct KeySet
    {
        enum class SecurityPolicy : uint8_t
        {
            kStandard   = 0,
            kLowLatency = 1
        };

        // Logical index provided by the Administrator that configured the entry
        uint16_t key_set_index;
        // Security policy to use for groups that use this keyset
        SecurityPolicy policy;
        // The actual keys for the group key set
        EpochKey epoch_keys[3];
        // TODO: Move fields around to reduce padding loss
        // Number of keys present
        uint8_t num_keys_used;
    };

    class GroupMappingIterator
    {
    public:
        virtual ~GroupMappingIterator() = default;
        virtual uint16_t Count()        = 0;
        virtual GroupId Next()          = 0;
        virtual void Release()          = 0;

    protected:
        GroupMappingIterator() = default;
    };

    class GroupStateIterator
    {
    public:
        virtual ~GroupStateIterator()                    = default;
        virtual uint16_t Count()                         = 0;
        virtual const GroupStateListEntry * Next()       = 0;
        virtual void Release()                           = 0;

    protected:
        GroupStateIterator() = default;
    };

    class KeySetIterator
    {
    public:
        virtual ~KeySetIterator()           = default;
        virtual uint16_t Count()            = 0;
        virtual const KeySet * Next()       = 0;
        virtual void Release()              = 0;

    protected:
        KeySetIterator() = default;
    };

    class GroupListener
    {
        virtual ~GroupListener() = default;
        virtual void OnGroupStateChanged(const GroupState & old_state, const GroupState & new_state) = 0;
        virtual void OnGroupStateRemoved(const GroupState & removed_state) = 0;
    };

    GroupDataProvider()          = default;
    virtual ~GroupDataProvider() = default;

    // Not copyable
    GroupDataProvider(const GroupDataProvider &) = delete;
    GroupDataProvider & operator=(const GroupDataProvider &) = delete;

    virtual CHIP_ERROR Init() = 0;
    virtual void Finish()     = 0;

    // Endpoints
    virtual bool GroupMappingExists(chip::FabricIndex fabric_index, GroupMapping & mapping)                       = 0;
    virtual CHIP_ERROR AddGroupMapping(chip::FabricIndex fabric_index, GroupMapping & mapping, const char * name) = 0;
    virtual CHIP_ERROR RemoveGroupMapping(chip::FabricIndex fabric_index, GroupMapping & mapping)                 = 0;
    virtual CHIP_ERROR RemoveAllGroupMappings(chip::FabricIndex fabric_index, EndpointId endpoint)                = 0;
    virtual GroupMappingIterator * IterateGroupMappings(chip::FabricIndex fabric_index, EndpointId endpoint)      = 0;

    // States
    virtual CHIP_ERROR SetGroupState(chip::FabricIndex fabric_index, uint16_t state_index, const GroupState & state)    = 0;
    virtual CHIP_ERROR GetGroupState(chip::FabricIndex fabric_index, uint16_t state_index, GroupStateListEntry & state) = 0;
    virtual CHIP_ERROR RemoveGroupState(chip::FabricIndex fabric_index, uint16_t state_index)                           = 0;
    virtual GroupStateIterator * IterateGroupStates(chip::FabricIndex fabric_index)                                     = 0;

    // Keys
    virtual CHIP_ERROR SetKeySet(chip::FabricIndex fabric_index, KeySet & keys)             = 0;
    virtual CHIP_ERROR GetKeySet(chip::FabricIndex fabric_index, KeySet & keys)             = 0;
    virtual CHIP_ERROR RemoveKeySet(chip::FabricIndex fabric_index, uint16_t key_set_index) = 0;
    virtual KeySetIterator * IterateKeySets(chip::FabricIndex fabric_index)                 = 0;

    void SetListener(GroupListener * listener) { mListener = listener; };
    void RemoveListener() { mListener = nullptr; };

    // TODO: handle fabric deletion (reindex fabric entries!)
private:
    GroupListener * mListener = nullptr;
};

/**
 * Instance getter for the global GroupDataProvider.
 *
 * Callers have to externally synchronize usage of this function.
 *
 * @return The global Group Data Provider. Assume never null.
 */
GroupDataProvider * GetGroupDataProvider();

/**
 * Instance setter for the global GroupDataProvider.
 *
 * Callers have to externally synchronize usage of this function.
 *
 * If the `provider` is nullptr, no change is done.
 *
 * @param[in] provider the Group Data Provider
 */
void SetGroupDataProvider(GroupDataProvider * provider);

} // namespace Credentials
} // namespace chip
