/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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
#include <lib/support/CHIPMemString.h>
#include <lib/support/CommonIterator.h>

namespace chip {
namespace Credentials {

class GroupDataProvider
{
public:
    using SecurityPolicy                                  = app::Clusters::GroupKeyManagement::GroupKeySecurityPolicyEnum;
    static constexpr KeysetId kIdentityProtectionKeySetId = 0;

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
                Platform::CopyString(name, groupName);
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
                Platform::CopyString(name, groupName);
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
        bool operator==(const GroupKey & other) const
        {
            return this->group_id == other.group_id && this->keyset_id == other.keyset_id;
        }
    };

    struct GroupEndpoint
    {
        GroupEndpoint() = default;
        GroupEndpoint(GroupId group, EndpointId endpoint) : group_id(group), endpoint_id(endpoint) {}
        // Identifies group within the scope of the given Fabric
        GroupId group_id = kUndefinedGroupId;
        // Endpoint on the Node to which messages to this group may be forwarded
        EndpointId endpoint_id = kInvalidEndpointId;

        bool operator==(const GroupEndpoint & other) const
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
        Crypto::SymmetricKeyContext * keyContext = nullptr;
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

        void Clear()
        {
            start_time = 0;
            Crypto::ClearSecretData(&key[0], sizeof(key));
        }
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
        SecurityPolicy policy = SecurityPolicy::kCacheAndSync;
        // Number of keys present
        uint8_t num_keys_used = 0;

        bool operator==(const KeySet & other)
        {
            VerifyOrReturnError(this->policy == other.policy && this->num_keys_used == other.num_keys_used, false);
            return !memcmp(this->epoch_keys, other.epoch_keys, this->num_keys_used * sizeof(EpochKey));
        }

        void ClearKeys()
        {
            for (size_t key_idx = 0; key_idx < kEpochKeysMax; ++key_idx)
            {
                epoch_keys[key_idx].Clear();
            }
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
         *  @param[in] old_group  GroupInfo structure of the removed group.
         */
        virtual void OnGroupRemoved(FabricIndex fabric_index, const GroupInfo & old_group) = 0;
    };

    using GroupInfoIterator    = CommonIterator<GroupInfo>;
    using GroupKeyIterator     = CommonIterator<GroupKey>;
    using EndpointIterator     = CommonIterator<GroupEndpoint>;
    using KeySetIterator       = CommonIterator<KeySet>;
    using GroupSessionIterator = CommonIterator<GroupSession>;

    GroupDataProvider(uint16_t maxGroupsPerFabric    = CHIP_CONFIG_MAX_GROUPS_PER_FABRIC,
                      uint16_t maxGroupKeysPerFabric = CHIP_CONFIG_MAX_GROUP_KEYS_PER_FABRIC) :
        mMaxGroupsPerFabric(maxGroupsPerFabric),
        mMaxGroupKeysPerFabric(maxGroupKeysPerFabric)
    {}

    virtual ~GroupDataProvider() = default;

    // Not copyable
    GroupDataProvider(const GroupDataProvider &) = delete;
    GroupDataProvider & operator=(const GroupDataProvider &) = delete;

    uint16_t GetMaxGroupsPerFabric() const { return mMaxGroupsPerFabric; }
    uint16_t GetMaxGroupKeysPerFabric() const { return mMaxGroupKeysPerFabric; }

    /**
     *  Initialize the GroupDataProvider, including possibly any persistent
     *  data store initialization done by the implementation. Must be called once
     *  before any other API succeeds.
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
     *  If you wish to iterate only the endpoints of a particular group id you can provide the optional `group_id` to do so.
     *  @retval An instance of EndpointIterator on success
     *  @retval nullptr if no iterator instances are available.
     */
    virtual EndpointIterator * IterateEndpoints(FabricIndex fabric_index, Optional<GroupId> group_id = NullOptional) = 0;

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

    virtual CHIP_ERROR SetKeySet(FabricIndex fabric_index, const ByteSpan & compressed_fabric_id, const KeySet & keys) = 0;
    virtual CHIP_ERROR GetKeySet(FabricIndex fabric_index, KeysetId keyset_id, KeySet & keys)                          = 0;
    virtual CHIP_ERROR RemoveKeySet(FabricIndex fabric_index, KeysetId keyset_id)                                      = 0;

    /**
     * @brief Obtain the actual operational Identity Protection Key (IPK) keyset for a given
     *        fabric. These keys are used by the CASE protocol, and do not participate in
     *        any direct traffic encryption. Since the identity protection operational keyset
     *        is used in multiple key derivations and procedures, it cannot be hidden behind a
     *        SymmetricKeyContext, and must be obtainable by value.
     *
     * @param fabric_index - Fabric index for which to get the IPK operational keyset
     * @param out_keyset - Reference to a KeySet where the IPK keys will be stored on success
     * @return CHIP_NO_ERROR on success, CHIP_ERROR_NOT_FOUND if the IPK keyset is somehow unavailable
     *         or another CHIP_ERROR value if an internal storage error occurs.
     */
    virtual CHIP_ERROR GetIpkKeySet(FabricIndex fabric_index, KeySet & out_keyset) = 0;

    /**
     *  Creates an iterator that may be used to obtain the list of key sets associated with the given fabric.
     *  In order to release the allocated memory, the Release() method must be called after the iteration is finished.
     *  Modifying the key sets table during the iteration is currently not supported, and may yield unexpected behaviour.
     *
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
 * @brief Utility Set the IPK Epoch key on a GroupDataProvider assuming a single IPK
 *
 * This utility replaces having to call `GroupDataProvider::SetKeySet` for the simple situation of a
 * single IPK for a fabric, if a single epoch key is used. Start time will be set to 0 ("was always valid")
 *
 * @param provider - pointer to GroupDataProvider on which to set the IPK
 * @param fabric_index - fabric index within the GroupDataProvider for which to set the IPK
 * @param ipk_epoch_span - Span containing the IPK epoch key
 * @param compressed_fabric_id - Compressed fabric ID associated with the fabric, for key derivation
 * @return CHIP_NO_ERROR on success, CHIP_ERROR_INVALID_ARGUMENT on any bad argument, other CHIP_ERROR values
 *         from implementation on other errors
 */
inline CHIP_ERROR SetSingleIpkEpochKey(GroupDataProvider * provider, FabricIndex fabric_index, const ByteSpan & ipk_epoch_span,
                                       const ByteSpan & compressed_fabric_id)
{
    GroupDataProvider::KeySet ipkKeySet(GroupDataProvider::kIdentityProtectionKeySetId,
                                        GroupDataProvider::SecurityPolicy::kTrustFirst, 1);

    VerifyOrReturnError(provider != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(ipk_epoch_span.size() == sizeof(ipkKeySet.epoch_keys[0].key), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(compressed_fabric_id.size() == sizeof(uint64_t), CHIP_ERROR_INVALID_ARGUMENT);

    ipkKeySet.epoch_keys[0].start_time = 0;
    memcpy(&ipkKeySet.epoch_keys[0].key, ipk_epoch_span.data(), ipk_epoch_span.size());

    // Set a single IPK, validate key derivation follows spec
    return provider->SetKeySet(fabric_index, compressed_fabric_id, ipkKeySet);
}

/**
 * Instance getter for the global GroupDataProvider.
 *
 * Callers have to externally synchronize usage of this function.
 *
 * @return The global Group Data Provider
 */
GroupDataProvider * GetGroupDataProvider();

/**
 * Instance setter for the global GroupDataProvider.
 *
 * Callers have to externally synchronize usage of this function.
 *
 * The `provider` can be set to nullptr if the owner is done with it fully.
 *
 * @param[in] provider pointer to the Group Data Provider global isntance to use
 */
void SetGroupDataProvider(GroupDataProvider * provider);

} // namespace Credentials
} // namespace chip
