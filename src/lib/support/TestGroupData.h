/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <credentials/GroupDataProviderImpl.h>
#include <lib/support/TestPersistentStorageDelegate.h>

namespace {

constexpr uint16_t kMaxGroupsPerFabric    = 5;
constexpr uint16_t kMaxGroupKeysPerFabric = 8;

static chip::TestPersistentStorageDelegate sDeviceStorage;
static chip::Credentials::GroupDataProviderImpl sGroupsProvider(sDeviceStorage, kMaxGroupsPerFabric, kMaxGroupKeysPerFabric);

static const chip::FabricIndex kFabric1 = 1;
static const chip::GroupId kGroup1      = 0x1234;
static const chip::GroupId kGroup2      = 0x0001;
static const chip::KeysetId kKeySet1    = 0x0101;

}

namespace chip {

namespace GroupTesting {

    CHIP_ERROR InitGroupData()
    {
        ReturnErrorOnFailure(sGroupsProvider.Init());
        chip::Credentials::SetGroupDataProvider(&sGroupsProvider);

        const chip::Credentials::GroupDataProvider::GroupInfo group1(kGroup1, "Group #1");
        ReturnErrorOnFailure(sGroupsProvider.SetGroupInfo(kFabric1, group1));
        ReturnErrorOnFailure(sGroupsProvider.AddEndpoint(kFabric1, group1.group_id, 1));

        const chip::Credentials::GroupDataProvider::GroupInfo group2(kGroup2, "Group #2");
        ReturnErrorOnFailure(sGroupsProvider.SetGroupInfo(kFabric1, group2));
        ReturnErrorOnFailure(sGroupsProvider.AddEndpoint(kFabric1, group2.group_id, 0));

        chip::Credentials::GroupDataProvider::KeySet keyset1(
            kKeySet1, chip::Credentials::GroupDataProvider::KeySet::SecurityPolicy::kStandard, 3);
        const chip::Credentials::GroupDataProvider::EpochKey epoch_keys[] = {
            { 0xaaaaaaaaaaaaaaaa, { 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf } },
            { 0xbbbbbbbbbbbbbbbb, { 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf } },
            { 0xcccccccccccccccc, { 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf } },
        };
        memcpy(keyset1.epoch_keys, epoch_keys, sizeof(epoch_keys));
        ReturnErrorOnFailure(sGroupsProvider.SetKeySet(kFabric1, keyset1));

        sGroupsProvider.SetGroupKeyAt(kFabric1, 0, chip::Credentials::GroupDataProvider::GroupKey(kGroup1, kKeySet1));
        sGroupsProvider.SetGroupKeyAt(kFabric1, 1, chip::Credentials::GroupDataProvider::GroupKey(kGroup2, kKeySet1));

        return CHIP_NO_ERROR;
    }

} // GroupTesting

} // namespace chip
