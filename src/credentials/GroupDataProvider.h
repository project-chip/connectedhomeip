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
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <stdint.h>
#include <sys/types.h>
#include <transport/raw/MessageHeader.h>

namespace chip {
namespace Credentials {

class GroupDataProvider
{
public:
    // An EpochKey is a single key usable to determine an operational group key
    struct EpochKey
    {
        static constexpr size_t kLengthBytes = Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES;
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
        static constexpr size_t kGroupNameMax = CHIP_CONFIG_MAX_GROUP_NAME_LENGTH;

        // The endpoint to which a GroupId is mapped.
        EndpointId endpoint = kInvalidEndpointId;
        // The GroupId, which, when received in a message will map to the `endpoint`.
        GroupId group = kUndefinedGroupId;
        // Group name
        char name[kGroupNameMax + 1] = { 0 };

        GroupMapping() = default;
        GroupMapping(EndpointId eid, GroupId gid) : GroupMapping(eid, gid, nullptr) {}
        GroupMapping(EndpointId eid, GroupId gid, const char * groupName) : endpoint(eid), group(gid)
        {
            if (nullptr == groupName)
            {
                name[0] = 0;
            }
            else
            {
                size_t size = strnlen(groupName, kGroupNameMax);
                strncpy(name, groupName, size);
                name[size] = 0;
            }
        }
        GroupMapping(EndpointId eid, GroupId gid, const CharSpan & groupName) : endpoint(eid), group(gid)
        {
            if (nullptr == groupName.data())
            {
                name[0] = 0;
            }
            else
            {
                size_t size = std::min(groupName.size(), kGroupNameMax);
                strncpy(name, groupName.data(), size);
                name[size] = 0;
            }
        }
        bool operator==(const GroupMapping & other)
        {
            return (this->endpoint == other.endpoint) && (this->group == other.group) &&
                strncmp(this->name, other.name, kGroupNameMax);
        }
    };

    // A group state maps the group key set to use for encryption/decryption for a given group ID.
    struct GroupState
    {
        GroupState() = default;
        GroupState(chip::FabricIndex fabric, chip::GroupId group_id, uint16_t key_set) :
            fabric_index(fabric), group(group_id), keyset_index(key_set)
        {}
        // Fabric Index associated with the group state entry's fabric scoping
        chip::FabricIndex fabric_index = kUndefinedFabricIndex;
        // Identifies the group within the scope of the given fabric
        chip::GroupId group = kUndefinedGroupId;
        // References the set of group keys that generate operationa group keys for use with the given group
        uint16_t keyset_index = 0;
        bool operator==(const GroupState & other)
        {
            return this->fabric_index == other.fabric_index && this->group == other.group &&
                this->keyset_index == other.keyset_index;
        }
    };

    // A operational group key set, usable by many GroupState mappings
    struct KeySet
    {
        enum class SecurityPolicy : uint8_t
        {
            kStandard   = 0,
            kLowLatency = 1
        };

        KeySet() = default;
        KeySet(uint16_t id) : keyset_id(id) {}
        KeySet(uint16_t id, SecurityPolicy policy_id, uint8_t num_keys) : keyset_id(id), policy(policy_id), num_keys_used(num_keys)
        {}
        KeySet(SecurityPolicy policy_id, uint8_t num_keys) : keyset_id(0), policy(policy_id), num_keys_used(num_keys) {}

        // The actual keys for the group key set
        EpochKey epoch_keys[3];
        // Logical id provided by the Administrator that configured the entry
        uint16_t keyset_id = 0;
        // Security policy to use for groups that use this keyset
        SecurityPolicy policy = SecurityPolicy::kStandard;
        // Number of keys present
        uint8_t num_keys_used = 0;

        bool operator==(const KeySet & other)
        {
            if (this->policy == other.policy && this->num_keys_used == other.num_keys_used)
            {
                return !memcmp(this->epoch_keys, other.epoch_keys, this->num_keys_used * sizeof(EpochKey));
            }
            return false;
        }
    };

    /**
     * Template used to iterate the stored group data
     */
    template <typename T>
    class Iterator
    {
    public:
        virtual ~Iterator() = default;
        /**
         *  @retval The number of entries in total that will be iterated.
         */
        virtual size_t Count() = 0;
        /**
         *   @param[out] item  Value associated with the next element in the iteration.
         *  @retval true if the next entry is successfully retrieved.
         *  @retval false if no more entries can be found.
         */
        virtual bool Next(T & item) = 0;
        /**
         * Release the memory allocated by this iterator.
         * Must be called before the pointer goes out of scope.
         */
        virtual void Release() = 0;

    protected:
        Iterator() = default;
    };

    using GroupMappingIterator = Iterator<GroupMapping>;
    using GroupStateIterator   = Iterator<GroupState>;
    using KeySetIterator       = Iterator<KeySet>;

    /**
     *  Interface for a listener of changes in any Group configuration. Necessary
     *  to implement attribute subscription for Group Key Management cluster, and
     *  to react to configuration changes that may impact in-progress functional work.
     */
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

    //
    // Group Mappings
    //

    virtual bool HasGroupNamesSupport()                                                                 = 0;
    virtual bool GroupMappingExists(chip::FabricIndex fabric_index, const GroupMapping & mapping)       = 0;
    virtual CHIP_ERROR AddGroupMapping(chip::FabricIndex fabric_index, const GroupMapping & mapping)    = 0;
    virtual CHIP_ERROR RemoveGroupMapping(chip::FabricIndex fabric_index, const GroupMapping & mapping) = 0;
    virtual CHIP_ERROR RemoveAllGroupMappings(chip::FabricIndex fabric_index, EndpointId endpoint)      = 0;
    /**
     *  Creates an iterator that may be used to obtain the groups associated with the given fabric and endpoint.
     *  The number of concurrent instances of this iterator is limited. In order to release the allocated memory,
     *  the iterator's Release() method must be called after the iteration is finished.
     *  @retval An instance of GroupMappingIterator on success
     *  @retval nullptr if no iterator instances are available.
     */
    virtual GroupMappingIterator * IterateGroupMappings(chip::FabricIndex fabric_index, EndpointId endpoint) = 0;

    //
    // Group States
    //

    virtual CHIP_ERROR SetGroupState(size_t state_index, const GroupState & state) = 0;
    virtual CHIP_ERROR GetGroupState(size_t state_index, GroupState & state)       = 0;
    virtual CHIP_ERROR RemoveGroupState(size_t state_index)                        = 0;
    /**
     *  Creates an iterator that may be used to obtain the list of group states.
     *  The number of concurrent instances of this iterator is limited. In order to release the allocated memory,
     *  the iterator's Release() method must be called after the iteration is finished.
     *  @retval An instance of GroupStateIterator on success
     *  @retval nullptr if no iterator instances are available.
     */
    virtual GroupStateIterator * IterateGroupStates() = 0;
    /**
     *  Creates an iterator that may be used to obtain the list of group states associated with the given fabric.
     *  The number of concurrent instances of this iterator is limited. In order to release the allocated memory,
     *  the iterator's Release() method must be called after the iteration is finished.
     *  @retval An instance of GroupStateIterator on success
     *  @retval nullptr if no iterator instances are available.
     */
    virtual GroupStateIterator * IterateGroupStates(chip::FabricIndex fabric_index) = 0;

    //
    // Key Sets
    //

    virtual CHIP_ERROR SetKeySet(chip::FabricIndex fabric_index, uint16_t keyset_id, const KeySet & keys) = 0;
    virtual CHIP_ERROR GetKeySet(chip::FabricIndex fabric_index, uint16_t keyset_id, KeySet & keys)       = 0;
    virtual CHIP_ERROR RemoveKeySet(chip::FabricIndex fabric_index, uint16_t keyset_id)                   = 0;
    /**
     *  Creates an iterator that may be used to obtain the list of key sets associated with the given fabric.
     *  The number of concurrent instances of this iterator is limited. In order to release the allocated memory,
     *  the iterator's Release() method must be called after the iteration is finished.
     *  @retval An instance of KeySetIterator on success
     *  @retval nullptr if no iterator instances are available.
     */
    virtual KeySetIterator * IterateKeySets(chip::FabricIndex fabric_index) = 0;

    // Fabrics
    virtual CHIP_ERROR RemoveFabric(chip::FabricIndex fabric_index) = 0;

    // General
    virtual CHIP_ERROR Decrypt(PacketHeader packetHeader, PayloadHeader & payloadHeader, System::PacketBufferHandle & msg) = 0;

    // Listener
    void SetListener(GroupListener * listener) { mListener = listener; };
    void RemoveListener() { mListener = nullptr; };

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
