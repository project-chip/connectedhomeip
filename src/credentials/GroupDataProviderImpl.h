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

static constexpr size_t kIteratorsMax = CHIP_CONFIG_MAX_GROUP_CONCURRENT_ITERATORS;

class GroupDataProviderImpl : public GroupDataProvider
{
public:
    GroupDataProviderImpl(chip::PersistentStorageDelegate & storage_delegate) : mStorage(storage_delegate) {}
    virtual ~GroupDataProviderImpl() {}

    CHIP_ERROR Init() override;
    void Finish() override;

    //
    // Group Mappings
    //

    bool HasGroupNamesSupport() override;
    bool GroupMappingExists(chip::FabricIndex fabric_index, const GroupMapping & mapping) override;
    CHIP_ERROR AddGroupMapping(chip::FabricIndex fabric_index, const GroupMapping & mapping) override;
    CHIP_ERROR RemoveGroupMapping(chip::FabricIndex fabric_index, const GroupMapping & mapping) override;
    CHIP_ERROR RemoveAllGroupMappings(chip::FabricIndex fabric_index, EndpointId endpoint) override;
    GroupMappingIterator * IterateGroupMappings(chip::FabricIndex fabric_index, EndpointId endpoint) override;

    //
    // Group States
    //

    CHIP_ERROR SetGroupState(size_t state_index, const GroupState & state) override;
    CHIP_ERROR GetGroupState(size_t state_index, GroupState & state) override;
    CHIP_ERROR RemoveGroupState(size_t state_index) override;
    GroupStateIterator * IterateGroupStates() override;
    GroupStateIterator * IterateGroupStates(chip::FabricIndex fabric_index) override;

    //
    // Key Sets
    //

    CHIP_ERROR SetKeySet(chip::FabricIndex fabric_index, uint16_t keyset_id, const KeySet & keys) override;
    CHIP_ERROR GetKeySet(chip::FabricIndex fabric_index, uint16_t keyset_id, KeySet & keys) override;
    CHIP_ERROR RemoveKeySet(chip::FabricIndex fabric_index, uint16_t keyset_id) override;
    KeySetIterator * IterateKeySets(chip::FabricIndex fabric_index) override;

    // Fabrics
    CHIP_ERROR RemoveFabric(chip::FabricIndex fabric_index) override;

    // General
    CHIP_ERROR Decrypt(PacketHeader packetHeader, PayloadHeader & payloadHeader, System::PacketBufferHandle & msg) override;

private:
    class GroupMappingIteratorImpl : public GroupMappingIterator
    {
    public:
        GroupMappingIteratorImpl(GroupDataProviderImpl & provider, chip::FabricIndex fabric, chip::EndpointId endpoint);
        size_t Count() override;
        bool Next(GroupMapping & item) override;
        void Release() override;

    private:
        GroupDataProviderImpl & mProvider;
        chip::FabricIndex mFabric  = 0;
        chip::EndpointId mEndpoint = 0;
        chip::GroupId mGroup       = kUndefinedGroupId;
    };

    class AllStatesIterator : public GroupStateIterator
    {
    public:
        AllStatesIterator(GroupDataProviderImpl & provider);
        size_t Count() override;
        bool Next(GroupState & item) override;
        void Release() override;

    private:
        GroupDataProviderImpl & mProvider;
        uint16_t mIndex    = 0;
        size_t mCount      = 0;
        size_t mTotalCount = 0;
    };

    class FabricStatesIterator : public GroupStateIterator
    {
    public:
        FabricStatesIterator(GroupDataProviderImpl & provider, chip::FabricIndex fabric_index);
        size_t Count() override;
        bool Next(GroupState & item) override;
        void Release() override;

    private:
        GroupDataProviderImpl & mProvider;
        chip::FabricIndex mFabric = 0;
        uint16_t mIndex           = 0;
        size_t mCount             = 0;
        size_t mTotalCount        = 0;
    };

    class KeySetIteratorImpl : public KeySetIterator
    {
    public:
        KeySetIteratorImpl(GroupDataProviderImpl & provider, chip::FabricIndex fabric_index);
        size_t Count() override;
        bool Next(KeySet & item) override;
        void Release() override;

    private:
        GroupDataProviderImpl & mProvider;
        chip::FabricIndex mFabric = 0;
        uint16_t mNextId          = 0;
        size_t mCount             = 0;
        size_t mIndex             = 0;
    };

    chip::PersistentStorageDelegate & mStorage;
    bool mInitialized = false;
    BitMapObjectPool<GroupMappingIteratorImpl, kIteratorsMax> mEndpointIterators;
    BitMapObjectPool<AllStatesIterator, kIteratorsMax> mAllStatesIterators;
    BitMapObjectPool<FabricStatesIterator, kIteratorsMax> mFabricStatesIterators;
    BitMapObjectPool<KeySetIteratorImpl, kIteratorsMax, OnObjectPoolDestruction::IgnoreUnsafeDoNotUseInNewCode> mKeySetIterators;
};

} // namespace Credentials
} // namespace chip
