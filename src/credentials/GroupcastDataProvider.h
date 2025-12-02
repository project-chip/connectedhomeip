/*
 *    Copyright (c) 2025 Project CHIP Authors
 *    All rights reserved.
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
#include <credentials/FabricTable.h>
#include <crypto/CHIPCryptoPAL.h>
#include <crypto/SessionKeystore.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/CommonIterator.h>
#include <lib/support/Pool.h>
#include <lib/support/Span.h>
#include <string.h>

namespace chip {
namespace Groupcast {

constexpr size_t kEndpointsMax = 20;
constexpr size_t kIteratorMax  = CHIP_CONFIG_MAX_GROUP_CONCURRENT_ITERATORS;

struct Group
{
    Group() = default;
    Group(GroupId gid) : group_id(gid) {}
    Group(GroupId gid, KeysetId kid) : group_id(gid), key_id(kid) {}

    Group & operator=(const Group & t)
    {
        if (this == &t)
            return *this;
        this->group_id       = t.group_id;
        this->key_id         = t.key_id;
        this->endpoint_count = t.endpoint_count;
        memcpy(this->endpoints, t.endpoints, this->endpoint_count * sizeof(EndpointId));
        return *this;
    }

    bool operator==(const Group & other) const { return this->group_id == other.group_id; }

    GroupId group_id        = kUndefinedGroupId;
    KeysetId key_id         = 0;
    uint16_t endpoint_count = 0;
    EndpointId endpoints[kEndpointsMax];
};

struct DataProvider
{
    using KeyContext = chip::Crypto::SymmetricKeyContext;

    struct GroupIterator : CommonIterator<Group &>
    {
        GroupIterator(DataProvider & provider, FabricIndex fabric);
        size_t Count() override;
        bool Next(Group &) override;
        void Release() override;

    private:
        DataProvider & mProvider;
        FabricIndex mFabric;
        size_t mIndex = 0;
        size_t mCount = 0;
    };

    DataProvider() = default;

    CHIP_ERROR Initialize(PersistentStorageDelegate * storage, chip::Crypto::SessionKeystore * keystore);
    uint8_t GetMaxMembershipCount();
    CHIP_ERROR AddGroup(chip::FabricIndex fabric_idx, Group & grp);
    CHIP_ERROR GetGroup(FabricIndex fabric_idx, Group & grp);
    CHIP_ERROR RemoveGroup(FabricIndex fabric_idx, GroupId group_id);
    CHIP_ERROR SetEndpoints(FabricIndex fabric_idx, Group & grp);
    GroupIterator * IterateGroups(FabricIndex fabric);

private:
    PersistentStorageDelegate * mStorage      = nullptr;
    chip::Crypto::SessionKeystore * mKeystore = nullptr;
    ObjectPool<GroupIterator, kIteratorMax> mGroupIteratorPool;
};

} // namespace Groupcast
} // namespace chip
