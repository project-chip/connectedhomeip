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
    static constexpr size_t kGroupNameLengthMax = 16;
    static constexpr size_t kGroupEndpointsMax  = 16;

    struct EpochKey
    {
        static constexpr size_t kLengthBytes = 16;
        uint64_t start_time;
        uint8_t key[kLengthBytes];
    };

    struct GroupMapping
    {
        EndpointId endpoint;
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

    struct GroupState
    {
        uint16_t state_index;
        chip::GroupId group;
        uint16_t key_set_index;
        bool operator==(const GroupState & other)
        {
            return this->state_index == other.state_index && this->group == other.group &&
                this->key_set_index == other.key_set_index;
        }
    };

    struct KeySet
    {
        enum class SecurityPolicy : uint8_t
        {
            kStandard   = 0,
            kLowLatency = 1
        };

        uint16_t key_set_index;
        SecurityPolicy policy;
        EpochKey epoch_keys[3];
    };

    class GroupMappingIterator
    {
    public:
        virtual ~GroupMappingIterator() = default;
        virtual uint16_t Count()        = 0;
        virtual bool HasNext()          = 0;
        virtual GroupId Next() const    = 0;

    protected:
        GroupMappingIterator() = default;
    };

    class GroupStateIterator
    {
    public:
        virtual ~GroupStateIterator()           = default;
        virtual uint16_t Count()                = 0;
        virtual bool HasNext()                  = 0;
        virtual const GroupState * Next() const = 0;

    protected:
        GroupStateIterator() = default;
    };

    class KeySetIterator
    {
    public:
        virtual ~KeySetIterator()           = default;
        virtual uint16_t Count()            = 0;
        virtual bool HasNext()              = 0;
        virtual const KeySet * Next() const = 0;

    protected:
        KeySetIterator() = default;
    };

    struct GroupListener
    {
        void OnStateChanged(GroupState & old_state, GroupState & new_state);
    };

    GroupDataProvider()          = default;
    virtual ~GroupDataProvider() = default;

    // Not copyable
    GroupDataProvider(const GroupDataProvider &) = delete;
    GroupDataProvider & operator=(const GroupDataProvider &) = delete;

    virtual CHIP_ERROR Init() = 0;
    virtual void Finish()     = 0;

    // Endpoints
    virtual bool ExistsGroupMapping(chip::FabricIndex fabric, GroupMapping & mapping)                       = 0;
    virtual CHIP_ERROR AddGroupMapping(chip::FabricIndex fabric, GroupMapping & mapping, const char * name) = 0;
    virtual CHIP_ERROR RemoveGroupMapping(chip::FabricIndex fabric, GroupMapping & mapping)                 = 0;
    virtual CHIP_ERROR RemoveGroupAllMappings(chip::FabricIndex fabric, EndpointId endpoint)                = 0;
    virtual GroupMappingIterator * IterateGroupMappings(chip::FabricIndex fabric, EndpointId endpoint)      = 0;

    // States
    virtual CHIP_ERROR SetGroupState(chip::FabricIndex fabric, GroupState & state)      = 0;
    virtual CHIP_ERROR GetGroupState(chip::FabricIndex fabric, GroupState & state)      = 0;
    virtual CHIP_ERROR RemoveGroupState(chip::FabricIndex fabric, uint16_t state_index) = 0;
    virtual GroupStateIterator * IterateGroupStates(chip::FabricIndex fabric)           = 0;

    // Keys
    virtual CHIP_ERROR SetKeySet(chip::FabricIndex fabric, KeySet & keys)             = 0;
    virtual CHIP_ERROR GetKeySet(chip::FabricIndex fabric, KeySet & keys)             = 0;
    virtual CHIP_ERROR RemoveKeySet(chip::FabricIndex fabric, uint16_t key_set_index) = 0;
    virtual KeySetIterator * IterateKeySets(chip::FabricIndex fabric)                 = 0;

    void SetListener(GroupListener * listener) { mListener = listener; };
    void RemoveListener() { mListener = nullptr; };

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
