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
        // The GroupId, which, when received in a message will map to the `endpoint`.
        GroupId group;

        bool operator==(const GroupMapping & other) { return this->endpoint == other.endpoint && this->group == other.group; }
    };

    // A group state maps the group key set to use for encryption/decryption for a given group ID.
    struct GroupState
    {
        // Fabric Index associated with the group state entry's fabric scoping
        chip::FabricIndex fabric_index;
        // Identifies the group within the scope of the given fabric
        chip::GroupId group;
        // References the set of group keys that generate operationa group keys for use with the given group
        uint16_t key_set_index;
    };

    // A operational group key set, usable by many GroupState mappings
    struct KeySet
    {
        enum class SecurityPolicy : uint8_t
        {
            kStandard   = 0,
            kLowLatency = 1
        };

        // The actual keys for the group key set
        EpochKey epoch_keys[3];
        // Logical index provided by the Administrator that configured the entry
        uint16_t key_set_index;
        // Security policy to use for groups that use this keyset
        SecurityPolicy policy;
        // Number of keys present
        uint8_t num_keys_used;
    };

    // Iterator for group mappings under a given endpoint. Associated with
    // Groups cluster logic.
    class GroupMappingIterator
    {
    public:
        virtual ~GroupMappingIterator() = default;
        // Returns the number of entries in total that will be iterated.
        virtual size_t Count() = 0;
        // Returns true if a groupID is found in the iteration.
        virtual bool Next(GroupId & outGroup) = 0;
        // Release the memory allocated by this iterator, if any. Must be called before
        // losing scope of a `GroupMappingIterator *`
        virtual void Release() = 0;

    protected:
        GroupMappingIterator() = default;
    };

    // Iterator for group state information mapping a Group ID to a Group Key Set index,
    // such as reflected by the Groups attribute of the Group Key Management cluster.
    class GroupStateIterator
    {
    public:
        virtual ~GroupStateIterator() = default;
        // Returns the number of entries in total that will be iterated.
        virtual size_t Count() = 0;
        // Returns true if a GroupState is found in the iteration.
        virtual bool Next(GroupState & outEntry) = 0;
        // Release the memory allocated by this iterator, if any. Must be called before
        // losing scope of a `GroupStateIterator *`
        virtual void Release() = 0;

    protected:
        GroupStateIterator() = default;
    };

    // Iterator for the Group Key Sets related under a given Fabric.
    // TODO: Refactor to allow trial decryption and encryption directly, rather than accessing raw keys.
    class KeySetIterator
    {
    public:
        virtual ~KeySetIterator() = default;
        // Returns the number of entries in total that will be iterated.
        virtual size_t Count() = 0;
        // Returns true if a KeySet is found in the iteration.
        virtual bool Next(KeySet & outSet) = 0;
        // Release the memory allocated by this iterator, if any. Must be called before
        // losing scope of a `KeySetIterator *`
        virtual void Release() = 0;

    protected:
        KeySetIterator() = default;
    };

    // Interface for a listener for changes in any Group configuration. Necessary
    // to implement attribute subscription for Group Key Management cluster, and
    // to react to configuration changes that may impact in-progress functional
    // work.
    class GroupListener
    {
    public:
        virtual ~GroupListener() = default;
        /**
         *  Listener callback invoked when a GroupState entry is mutated or added.
         *
         *  @param[in] old_state  GroupState reflecting the previous entry. Set to nullptr on appends.
         *  @param[in] new_state  GroupState reflecting the updated/new entry.
         */
        virtual void OnGroupStateChanged(const GroupState * old_state, const GroupState * new_state) = 0;
        /**
         *  Listener callback invoked when a GroupState entry is removed from the Groups list.
         *
         *  @param[in] removed_state  Copy of GroupState that was just removed. Index included is no longer accessible.
         */
        virtual void OnGroupStateRemoved(const GroupState * removed_state) = 0;
    };

    GroupDataProvider()          = default;
    virtual ~GroupDataProvider() = default;

    // Not copyable
    GroupDataProvider(const GroupDataProvider &) = delete;
    GroupDataProvider & operator=(const GroupDataProvider &) = delete;

    /**
     *  Initialize the GroupDataProvider, including any persistent data store
     *  initialization. Must be called once before any other API succeeds.
     *
     *  @retval #CHIP_ERROR_INCORRECT_STATE if called when already initialized.
     *  @retval #CHIP_NO_ERROR on success.
     */
    virtual CHIP_ERROR Init() = 0;
    virtual void Finish()     = 0;

    // Endpoints
    virtual bool GroupMappingExists(chip::FabricIndex fabric_index, GroupMapping & mapping)                       = 0;
    virtual CHIP_ERROR AddGroupMapping(chip::FabricIndex fabric_index, GroupMapping & mapping, const char * name) = 0;
    virtual CHIP_ERROR RemoveGroupMapping(chip::FabricIndex fabric_index, GroupMapping & mapping)                 = 0;
    virtual CHIP_ERROR RemoveAllGroupMappings(chip::FabricIndex fabric_index)                                     = 0;
    virtual GroupMappingIterator * IterateGroupMappings(chip::FabricIndex fabric_index, EndpointId endpoint)      = 0;

    // States
    virtual CHIP_ERROR SetGroupState(uint16_t state_index, const GroupState & state) = 0;
    virtual CHIP_ERROR GetGroupState(uint16_t state_index, GroupState & state)       = 0;
    virtual CHIP_ERROR RemoveGroupState(uint16_t state_index)                        = 0;
    virtual GroupStateIterator * IterateGroupStates()                                = 0;
    virtual GroupStateIterator * IterateGroupStates(chip::FabricIndex fabric_index)  = 0;

    // Keys
    virtual CHIP_ERROR SetKeySet(chip::FabricIndex fabric_index, uint16_t key_set_index, KeySet & keys) = 0;
    virtual CHIP_ERROR GetKeySet(chip::FabricIndex fabric_index, uint16_t key_set_index, KeySet & keys) = 0;
    virtual CHIP_ERROR RemoveKeySet(chip::FabricIndex fabric_index, uint16_t key_set_index)             = 0;
    virtual KeySetIterator * IterateKeySets(chip::FabricIndex fabric_index)                             = 0;

    void SetListener(GroupListener * listener) { mListener = listener; };
    void RemoveListener() { mListener = nullptr; };

    // TODO: handle fabric deletion (reindex fabric entries!)
protected:
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
