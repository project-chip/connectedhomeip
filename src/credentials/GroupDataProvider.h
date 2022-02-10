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

#include <algorithm>
#include <stdint.h>
#include <sys/types.h>

#include <app-common/zap-generated/cluster-objects.h>
#include <app/util/basic-types.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>

namespace chip {
namespace Credentials {

class GroupDataProvider
{
public:
    using SecurityPolicy = app::Clusters::GroupKeyManagement::GroupKeySecurityPolicy;

    struct GroupInfo
    {
        static constexpr size_t kGroupNameMax = CHIP_CONFIG_MAX_GROUP_NAME_LENGTH;

        // Identifies group within the scope of the given Fabric
        GroupId group_id = kUndefinedGroupId;
        // Lastest group name written for a given GroupId on any Endpoint via the Groups cluster
        char name[kGroupNameMax + 1] = { 0 };

        GroupInfo() { SetName(nullptr); }
        GroupInfo(const char * groupName) { SetName(groupName); }
        GroupInfo(const CharSpan & groupName) { SetName(groupName); }
        GroupInfo(GroupId id, const char * groupName) : group_id(id) { SetName(groupName); }
        GroupInfo(GroupId id, const CharSpan & groupName) : group_id(id) { SetName(groupName); }
        void SetName(const char * groupName)
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
        void SetName(const CharSpan & groupName)
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
        bool operator==(const GroupInfo & other)
        {
            return (this->group_id == other.group_id) && !strncmp(this->name, other.name, kGroupNameMax);
        }
    };

    struct GroupKey
    {
        GroupKey() = default;
        GroupKey(GroupId group, KeysetId keyset) : group_id(group), keyset_id(keyset) {}
        // Identifies group within the scope of the given Fabric
        GroupId group_id = kUndefinedGroupId;
        // Set of group keys that generate operational group keys for use with this group
        KeysetId keyset_id = 0;
        bool operator==(const GroupKey & other) { return this->group_id == other.group_id && this->keyset_id == other.keyset_id; }
    };

    struct GroupEndpoint
    {
        GroupEndpoint() = default;
        GroupEndpoint(GroupId group, EndpointId endpoint) : group_id(group), endpoint_id(endpoint) {}
        // Identifies group within the scope of the given Fabric
        GroupId group_id = kUndefinedGroupId;
        // Endpoint on the Node to which messages to this group may be forwarded
        EndpointId endpoint_id = kInvalidEndpointId;

        bool operator==(const GroupEndpoint & other)
        {
            return this->group_id == other.group_id && this->endpoint_id == other.endpoint_id;
        }
    };

    struct GroupSession
    {
        GroupSession()   = default;
        GroupId group_id = kUndefinedGroupId;
        FabricIndex fabric_index;
        SecurityPolicy security_policy;
        Crypto::SymmetricKeyContext * key = nullptr;
    };

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

    // A operational group key set, usable by many GroupState mappings
    struct KeySet
    {
        static constexpr size_t kEpochKeysMax = 3;

        KeySet() = default;
        KeySet(uint16_t id, SecurityPolicy policy_id, uint8_t num_keys) : keyset_id(id), policy(policy_id), num_keys_used(num_keys)
        {}

        // The actual keys for the group key set
        EpochKey epoch_keys[kEpochKeysMax];
        // Logical id provided by the Administrator that configured the entry
        uint16_t keyset_id = 0;
        // Security policy to use for groups that use this keyset
        SecurityPolicy policy = SecurityPolicy::kStandard;
        // Number of keys present
        uint8_t num_keys_used = 0;

        bool operator==(const KeySet & other)
        {
            VerifyOrReturnError(this->policy == other.policy && this->num_keys_used == other.num_keys_used, false);
            return !memcmp(this->epoch_keys, other.epoch_keys, this->num_keys_used * sizeof(EpochKey));
        }
    };

    /**
     *  Interface to listen for changes in the Group info.
     */
    class GroupListener
    {
    public:
        virtual ~GroupListener() = default;
        /**
         *  Callback invoked when a new group is added.
         *
         *  @param[in] new_group  GroupInfo structure of the new group.
         */
        virtual void OnGroupAdded(FabricIndex fabric_index, const GroupInfo & new_group) = 0;
        /**
         *  Callback invoked when an existing group is removed.
         *
         *  @param[in] removed_state  GroupInfo structure of the removed group.
         */
        virtual void OnGroupRemoved(FabricIndex fabric_index, const GroupInfo & old_group) = 0;
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

    using GroupInfoIterator    = Iterator<GroupInfo>;
    using GroupKeyIterator     = Iterator<GroupKey>;
    using EndpointIterator     = Iterator<GroupEndpoint>;
    using KeySetIterator       = Iterator<KeySet>;
    using GroupSessionIterator = Iterator<GroupSession>;

    GroupDataProvider(uint16_t maxGroupsPerFabric    = CHIP_CONFIG_MAX_GROUPS_PER_FABRIC,
                      uint16_t maxGroupKeysPerFabric = CHIP_CONFIG_MAX_GROUP_KEYS_PER_FABRIC) :
        mMaxGroupsPerFabric(maxGroupsPerFabric),
        mMaxGroupKeysPerFabric(maxGroupKeysPerFabric)
    {}

    virtual ~GroupDataProvider() = default;

    // Not copyable
    GroupDataProvider(const GroupDataProvider &) = delete;
    GroupDataProvider & operator=(const GroupDataProvider &) = delete;

    uint16_t GetMaxGroupsPerFabric() { return mMaxGroupsPerFabric; }
    uint16_t GetMaxGroupKeysPerFabric() { return mMaxGroupKeysPerFabric; }

    /**
     *  Initialize the GroupDataProvider, including any persistent data store
     *  initialization. Must be called once before any other API succeeds.
     *
     *  @retval #CHIP_ERROR_INCORRECT_STATE if called when already initialized.
     *  @retval #CHIP_NO_ERROR on success
     */
    virtual CHIP_ERROR Init() = 0;
    virtual void Finish()     = 0;

    //
    // Group Table
    //

    // By id
    virtual CHIP_ERROR SetGroupInfo(FabricIndex fabric_index, const GroupInfo & info)             = 0;
    virtual CHIP_ERROR GetGroupInfo(FabricIndex fabric_index, GroupId group_id, GroupInfo & info) = 0;
    virtual CHIP_ERROR RemoveGroupInfo(FabricIndex fabric_index, GroupId group_id)                = 0;
    // By index
    virtual CHIP_ERROR SetGroupInfoAt(FabricIndex fabric_index, size_t index, const GroupInfo & info) = 0;
    virtual CHIP_ERROR GetGroupInfoAt(FabricIndex fabric_index, size_t index, GroupInfo & info)       = 0;
    virtual CHIP_ERROR RemoveGroupInfoAt(FabricIndex fabric_index, size_t index)                      = 0;
    // Endpoints
    virtual bool HasEndpoint(FabricIndex fabric_index, GroupId group_id, EndpointId endpoint_id)          = 0;
    virtual CHIP_ERROR AddEndpoint(FabricIndex fabric_index, GroupId group_id, EndpointId endpoint_id)    = 0;
    virtual CHIP_ERROR RemoveEndpoint(FabricIndex fabric_index, GroupId group_id, EndpointId endpoint_id) = 0;
    virtual CHIP_ERROR RemoveEndpoint(FabricIndex fabric_index, EndpointId endpoint_id)                   = 0;
    // Iterators
    /**
     *  Creates an iterator that may be used to obtain the list of groups associated with the given fabric.
     *  In order to release the allocated memory, the Release() method must be called after the iteration is finished.
     *  Modifying the group table during the iteration is currently not supported, and may yield unexpected behaviour.
     *  @retval An instance of EndpointIterator on success
     *  @retval nullptr if no iterator instances are available.
     */
    virtual GroupInfoIterator * IterateGroupInfo(FabricIndex fabric_index) = 0;
    /**
     *  Creates an iterator that may be used to obtain the list of (group, endpoint) pairs associated with the given fabric.
     *  In order to release the allocated memory, the Release() method must be called after the iteration is finished.
     *  Modifying the group table during the iteration is currently not supported, and may yield unexpected behaviour.
     *  @retval An instance of EndpointIterator on success
     *  @retval nullptr if no iterator instances are available.
     */
    virtual EndpointIterator * IterateEndpoints(FabricIndex fabric_index) = 0;

    //
    // Group-Key map
    //

    virtual CHIP_ERROR SetGroupKeyAt(FabricIndex fabric_index, size_t index, const GroupKey & info) = 0;
    virtual CHIP_ERROR GetGroupKeyAt(FabricIndex fabric_index, size_t index, GroupKey & info)       = 0;
    virtual CHIP_ERROR RemoveGroupKeyAt(FabricIndex fabric_index, size_t index)                     = 0;
    virtual CHIP_ERROR RemoveGroupKeys(FabricIndex fabric_index)                                    = 0;

    /**
     *  Creates an iterator that may be used to obtain the list of (group, keyset) pairs associated with the given fabric.
     *  In order to release the allocated memory, the Release() method must be called after the iteration is finished.
     *  Modifying the keyset mappings during the iteration is currently not supported, and may yield unexpected behaviour.
     *  @retval An instance of GroupKeyIterator on success
     *  @retval nullptr if no iterator instances are available.
     */
    virtual GroupKeyIterator * IterateGroupKeys(FabricIndex fabric_index) = 0;

    //
    // Key Sets
    //

    virtual CHIP_ERROR SetKeySet(FabricIndex fabric_index, const KeySet & keys)               = 0;
    virtual CHIP_ERROR GetKeySet(FabricIndex fabric_index, KeysetId keyset_id, KeySet & keys) = 0;
    virtual CHIP_ERROR RemoveKeySet(FabricIndex fabric_index, KeysetId keyset_id)             = 0;
    /**
     *  Creates an iterator that may be used to obtain the list of key sets associated with the given fabric.
     *  In order to release the allocated memory, the Release() method must be called after the iteration is finished.
     *  Modifying the key sets table during the iteration is currently not supported, and may yield unexpected behaviour.
     *  @retval An instance of KeySetIterator on success
     *  @retval nullptr if no iterator instances are available.
     */
    virtual KeySetIterator * IterateKeySets(FabricIndex fabric_index) = 0;

    // Fabrics
    virtual CHIP_ERROR RemoveFabric(FabricIndex fabric_index) = 0;

    // Decryption
    virtual GroupSessionIterator * IterateGroupSessions(uint16_t session_id)                        = 0;
    virtual Crypto::SymmetricKeyContext * GetKeyContext(FabricIndex fabric_index, GroupId group_id) = 0;

    // Listener
    void SetListener(GroupListener * listener) { mListener = listener; };
    void RemoveListener() { mListener = nullptr; };

protected:
    void GroupAdded(FabricIndex fabric_index, const GroupInfo & new_group)
    {
        if (mListener)
        {
            mListener->OnGroupAdded(fabric_index, new_group);
        }
    }
    void GroupRemoved(FabricIndex fabric_index, const GroupInfo & old_group)
    {
        if (mListener)
        {
            mListener->OnGroupRemoved(fabric_index, old_group);
        }
    }
    const uint16_t mMaxGroupsPerFabric;
    const uint16_t mMaxGroupKeysPerFabric;
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
