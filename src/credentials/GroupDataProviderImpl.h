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

#include <credentials/GroupDataProvider.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/Pool.h>

namespace chip {
namespace Credentials {

class GroupDataProviderImpl : public GroupDataProvider
{
public:
    static constexpr size_t kIteratorsMax = CHIP_CONFIG_MAX_GROUP_CONCURRENT_ITERATORS;

    GroupDataProviderImpl(chip::PersistentStorageDelegate & storage_delegate) : mStorage(storage_delegate) {}
    GroupDataProviderImpl(chip::PersistentStorageDelegate & storage_delegate, uint16_t maxGroupsPerFabric,
                          uint16_t maxGroupKeysPerFabric) :
        GroupDataProvider(maxGroupsPerFabric, maxGroupKeysPerFabric),
        mStorage(storage_delegate)
    {}
    virtual ~GroupDataProviderImpl() {}

    CHIP_ERROR Init() override;
    void Finish() override;

    //
    // Group Info
    //

    // By id
    CHIP_ERROR SetGroupInfo(chip::FabricIndex fabric_index, const GroupInfo & info) override;
    CHIP_ERROR GetGroupInfo(chip::FabricIndex fabric_index, chip::GroupId group_id, GroupInfo & info) override;
    CHIP_ERROR RemoveGroupInfo(chip::FabricIndex fabric_index, chip::GroupId group_id) override;
    // By index
    CHIP_ERROR SetGroupInfoAt(chip::FabricIndex fabric_index, size_t index, const GroupInfo & info) override;
    CHIP_ERROR GetGroupInfoAt(chip::FabricIndex fabric_index, size_t index, GroupInfo & info) override;
    CHIP_ERROR RemoveGroupInfoAt(chip::FabricIndex fabric_index, size_t index) override;
    // Endpoints
    bool HasEndpoint(chip::FabricIndex fabric_index, chip::GroupId group_id, chip::EndpointId endpoint_id) override;
    CHIP_ERROR AddEndpoint(chip::FabricIndex fabric_index, chip::GroupId group_id, chip::EndpointId endpoint_id) override;
    CHIP_ERROR RemoveEndpoint(chip::FabricIndex fabric_index, chip::GroupId group_id, chip::EndpointId endpoint_id) override;
    CHIP_ERROR RemoveEndpoint(chip::FabricIndex fabric_index, chip::EndpointId endpoint_id) override;
    // Iterators
    GroupInfoIterator * IterateGroupInfo(chip::FabricIndex fabric_index) override;
    EndpointIterator * IterateEndpoints(chip::FabricIndex fabric_index) override;

    //
    // Group-Key map
    //

    CHIP_ERROR SetGroupKeyAt(chip::FabricIndex fabric_index, size_t index, const GroupKey & info) override;
    CHIP_ERROR GetGroupKeyAt(chip::FabricIndex fabric_index, size_t index, GroupKey & info) override;
    CHIP_ERROR RemoveGroupKeyAt(chip::FabricIndex fabric_index, size_t index) override;
    CHIP_ERROR RemoveGroupKeys(chip::FabricIndex fabric_index) override;
    GroupKeyIterator * IterateGroupKeys(chip::FabricIndex fabric_index) override;

    //
    // Key Sets
    //

    CHIP_ERROR SetKeySet(chip::FabricIndex fabric_index, const KeySet & keys) override;
    CHIP_ERROR GetKeySet(chip::FabricIndex fabric_index, chip::KeysetId keyset_id, KeySet & keys) override;
    CHIP_ERROR RemoveKeySet(chip::FabricIndex fabric_index, chip::KeysetId keyset_id) override;
    KeySetIterator * IterateKeySets(chip::FabricIndex fabric_index) override;

    // Fabrics
    CHIP_ERROR RemoveFabric(chip::FabricIndex fabric_index) override;

    // General
    CHIP_ERROR Decrypt(PacketHeader packetHeader, PayloadHeader & payloadHeader, System::PacketBufferHandle & msg) override;

private:
    class GroupInfoIteratorImpl : public GroupInfoIterator
    {
    public:
        GroupInfoIteratorImpl(GroupDataProviderImpl & provider, chip::FabricIndex fabric_index);
        size_t Count() override;
        bool Next(GroupInfo & output) override;
        void Release() override;

    private:
        GroupDataProviderImpl & mProvider;
        chip::FabricIndex mFabric = kUndefinedFabricIndex;
        uint16_t mNextId          = 0;
        size_t mCount             = 0;
        size_t mTotal             = 0;
    };

    class GroupKeyIteratorImpl : public GroupKeyIterator
    {
    public:
        GroupKeyIteratorImpl(GroupDataProviderImpl & provider, chip::FabricIndex fabric_index);
        size_t Count() override;
        bool Next(GroupKey & output) override;
        void Release() override;

    private:
        GroupDataProviderImpl & mProvider;
        chip::FabricIndex mFabric = kUndefinedFabricIndex;
        uint16_t mNextId          = 0;
        size_t mCount             = 0;
        size_t mTotal             = 0;
    };

    class EndpointIteratorImpl : public EndpointIterator
    {
    public:
        EndpointIteratorImpl(GroupDataProviderImpl & provider, chip::FabricIndex fabric_index);
        size_t Count() override;
        bool Next(GroupEndpoint & output) override;
        void Release() override;

    private:
        GroupDataProviderImpl & mProvider;
        chip::FabricIndex mFabric = kUndefinedFabricIndex;
        chip::GroupId mFirstGroup = kUndefinedGroupId;
        uint16_t mGroup           = 0;
        size_t mGroupIndex        = 0;
        size_t mGroupCount        = 0;
        uint16_t mEndpoint        = 0;
        size_t mEndpointIndex     = 0;
        size_t mEndpointCount     = 0;
        bool mFirstEndpoint       = true;
    };

    class KeySetIteratorImpl : public KeySetIterator
    {
    public:
        KeySetIteratorImpl(GroupDataProviderImpl & provider, chip::FabricIndex fabric_index);
        size_t Count() override;
        bool Next(KeySet & output) override;
        void Release() override;

    private:
        GroupDataProviderImpl & mProvider;
        chip::FabricIndex mFabric = kUndefinedFabricIndex;
        uint16_t mNextId          = 0;
        size_t mCount             = 0;
        size_t mTotal             = 0;
    };
    CHIP_ERROR RemoveEndpoints(chip::FabricIndex fabric_index, chip::GroupId group_id);

    chip::PersistentStorageDelegate & mStorage;
    bool mInitialized = false;
    BitMapObjectPool<GroupInfoIteratorImpl, kIteratorsMax> mGroupInfoIterators;
    BitMapObjectPool<GroupKeyIteratorImpl, kIteratorsMax> mGroupKeyIterators;
    BitMapObjectPool<EndpointIteratorImpl, kIteratorsMax> mEndpointIterators;
    BitMapObjectPool<KeySetIteratorImpl, kIteratorsMax> mKeySetIterators;
};

} // namespace Credentials
} // namespace chip
