/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <credentials/GroupDataProvider.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>
#include <string.h>

static_assert(CHIP_CONFIG_MAX_GROUPS_PER_FABRIC >= 5, "Test expects CHIP_CONFIG_MAX_GROUPS_PER_FABRIC >= 5");
static_assert(CHIP_CONFIG_MAX_GROUP_ENDPOINTS_PER_FABRIC >= 8, "Test expects CHIP_CONFIG_MAX_GROUP_ENDPOINTS_PER_FABRIC >= 8");

using namespace chip::Credentials;

namespace chip {
namespace app {
namespace TestGroups {

using KeySet = chip::Credentials::GroupDataProvider::KeySet;

void TestEndpoints(nlTestSuite * apSuite, void * apContext)
{
    GroupDataProvider * groups = GetGroupDataProvider();
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->Init());

    chip::FabricIndex kFabricIndex1 = 1;
    chip::FabricIndex kFabricIndex2 = 7;

    GroupDataProvider::GroupMapping group1a = { .endpoint = 1, .group = 1 };
    GroupDataProvider::GroupMapping group1b = { .endpoint = 1, .group = 2 };
    GroupDataProvider::GroupMapping group1c = { .endpoint = 1, .group = 3 };
    CHIP_ERROR err                          = CHIP_NO_ERROR;
    bool exists                             = false;

    NL_TEST_ASSERT(apSuite, groups);

    exists = groups->GroupMappingExists(0xff, group1a);
    NL_TEST_ASSERT(apSuite, !exists);

    exists = groups->GroupMappingExists(kFabricIndex1, group1a);
    NL_TEST_ASSERT(apSuite, !exists);

    err = groups->AddGroupMapping(kFabricIndex1, group1a, "Group 1.1");
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->AddGroupMapping(kFabricIndex1, group1c, "Group 1.3");
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    exists = groups->GroupMappingExists(kFabricIndex1, group1a);
    NL_TEST_ASSERT(apSuite, exists);

    exists = groups->GroupMappingExists(kFabricIndex1, group1b);
    NL_TEST_ASSERT(apSuite, !exists);

    exists = groups->GroupMappingExists(kFabricIndex1, group1c);
    NL_TEST_ASSERT(apSuite, exists);

    err = groups->RemoveGroupMapping(kFabricIndex1, group1a);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    exists = groups->GroupMappingExists(kFabricIndex1, group1a);
    NL_TEST_ASSERT(apSuite, !exists);

    exists = groups->GroupMappingExists(kFabricIndex1, group1b);
    NL_TEST_ASSERT(apSuite, !exists);

    exists = groups->GroupMappingExists(kFabricIndex1, group1c);
    NL_TEST_ASSERT(apSuite, exists);

    err = groups->AddGroupMapping(kFabricIndex1, group1a, "Group 1.1b");
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->AddGroupMapping(kFabricIndex1, group1b, "Group 1.2");
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    exists = groups->GroupMappingExists(kFabricIndex1, group1a);
    NL_TEST_ASSERT(apSuite, exists);

    exists = groups->GroupMappingExists(kFabricIndex1, group1b);
    NL_TEST_ASSERT(apSuite, exists);

    exists = groups->GroupMappingExists(kFabricIndex1, group1c);
    NL_TEST_ASSERT(apSuite, exists);

    err = groups->RemoveAllGroupMappings(kFabricIndex1);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    exists = groups->GroupMappingExists(kFabricIndex1, group1a);
    NL_TEST_ASSERT(apSuite, !exists);

    exists = groups->GroupMappingExists(kFabricIndex1, group1b);
    NL_TEST_ASSERT(apSuite, !exists);

    exists = groups->GroupMappingExists(kFabricIndex1, group1c);
    NL_TEST_ASSERT(apSuite, !exists);

    // Test multiple fabrics

    NL_TEST_ASSERT(apSuite, groups);

    exists = groups->GroupMappingExists(0xff, group1a);
    NL_TEST_ASSERT(apSuite, !exists);

    exists = groups->GroupMappingExists(kFabricIndex2, group1a);
    NL_TEST_ASSERT(apSuite, !exists);

    err = groups->AddGroupMapping(kFabricIndex1, group1a, "Group 1.1");
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->AddGroupMapping(kFabricIndex1, group1c, "Group 1.3");
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->AddGroupMapping(kFabricIndex2, group1a, "Group 1.1");
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->AddGroupMapping(kFabricIndex2, group1c, "Group 1.3");
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    exists = groups->GroupMappingExists(kFabricIndex2, group1a);
    NL_TEST_ASSERT(apSuite, exists);

    exists = groups->GroupMappingExists(kFabricIndex2, group1b);
    NL_TEST_ASSERT(apSuite, !exists);

    exists = groups->GroupMappingExists(kFabricIndex2, group1c);
    NL_TEST_ASSERT(apSuite, exists);

    err = groups->RemoveGroupMapping(kFabricIndex2, group1a);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    exists = groups->GroupMappingExists(kFabricIndex2, group1a);
    NL_TEST_ASSERT(apSuite, !exists);

    exists = groups->GroupMappingExists(kFabricIndex2, group1b);
    NL_TEST_ASSERT(apSuite, !exists);

    exists = groups->GroupMappingExists(kFabricIndex2, group1c);
    NL_TEST_ASSERT(apSuite, exists);

    err = groups->AddGroupMapping(kFabricIndex2, group1a, "Group 1.1b");
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->AddGroupMapping(kFabricIndex2, group1b, "Group 1.2");
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    exists = groups->GroupMappingExists(kFabricIndex2, group1a);
    NL_TEST_ASSERT(apSuite, exists);

    exists = groups->GroupMappingExists(kFabricIndex2, group1b);
    NL_TEST_ASSERT(apSuite, exists);

    exists = groups->GroupMappingExists(kFabricIndex2, group1c);
    NL_TEST_ASSERT(apSuite, exists);

    err = groups->RemoveAllGroupMappings(kFabricIndex2);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    exists = groups->GroupMappingExists(kFabricIndex2, group1a);
    NL_TEST_ASSERT(apSuite, !exists);

    exists = groups->GroupMappingExists(kFabricIndex2, group1b);
    NL_TEST_ASSERT(apSuite, !exists);

    exists = groups->GroupMappingExists(kFabricIndex2, group1c);
    NL_TEST_ASSERT(apSuite, !exists);

    groups->Finish();
}

void TestEndpointIterator(nlTestSuite * apSuite, void * apContext)
{
    GroupDataProvider * groups = GetGroupDataProvider();
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->Init());

    chip::FabricIndex kFabricIndex          = 1;
    GroupDataProvider::GroupMapping group1a = { .endpoint = 1, .group = 1 };
    GroupDataProvider::GroupMapping group1b = { .endpoint = 1, .group = 3 };
    GroupDataProvider::GroupMapping group2a = { .endpoint = 2, .group = 2 };
    GroupDataProvider::GroupMapping group3a = { .endpoint = 3, .group = 1 };
    GroupDataProvider::GroupMapping group3b = { .endpoint = 3, .group = 2 };
    GroupDataProvider::GroupMapping group3c = { .endpoint = 3, .group = 3 };
    CHIP_ERROR err                          = CHIP_NO_ERROR;

    NL_TEST_ASSERT(apSuite, groups);

    err = groups->AddGroupMapping(kFabricIndex, group3b, "Group 3.2");
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->AddGroupMapping(kFabricIndex, group2a, "Group 2.2");
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->AddGroupMapping(kFabricIndex, group1b, "Group 1.3");
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->AddGroupMapping(kFabricIndex, group3a, "Group 3.1");
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->AddGroupMapping(kFabricIndex, group1a, "Group 1.1");
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->AddGroupMapping(kFabricIndex, group3c, "Group 3.3");
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->AddGroupMapping(kFabricIndex, group3a, "Group 3.1");
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    GroupId gid = 0;

    // Endpoint 1
    EndpointId endpoint                          = 1;
    GroupDataProvider::GroupMappingIterator * it = groups->IterateGroupMappings(kFabricIndex, endpoint);
    NL_TEST_ASSERT(apSuite, it);

    size_t count1 = it->Count();
    size_t count2 = 0;
    NL_TEST_ASSERT(apSuite, 2 == count1);
    while (it->Next(gid))
    {
        count2++;
        NL_TEST_ASSERT(apSuite, 1 == gid || 3 == gid);
    }
    NL_TEST_ASSERT(apSuite, count2 == count1);
    it->Release();
    it = nullptr;

    // Endpoint 3
    endpoint = 3;
    it       = groups->IterateGroupMappings(kFabricIndex, endpoint);
    NL_TEST_ASSERT(apSuite, it);

    count1 = it->Count();
    count2 = 0;
    NL_TEST_ASSERT(apSuite, 3 == count1);
    while (it->Next(gid))
    {
        count2++;
        NL_TEST_ASSERT(apSuite, gid > 0 && gid < 4);
    }
    NL_TEST_ASSERT(apSuite, count2 == count1);
    it->Release();
    it = nullptr;

    groups->Finish();
}

void TestStates(nlTestSuite * apSuite, void * apContext)
{
    GroupDataProvider * groups = GetGroupDataProvider();
    NL_TEST_ASSERT(apSuite, groups);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->Init());

    GroupDataProvider::GroupState state0a = { .fabric_index = 1, .group = 1, .key_set_index = 1 };

    GroupDataProvider::GroupState state0b = { .fabric_index = 0, .group = 10, .key_set_index = 11 };

    GroupDataProvider::GroupState state1a = { .fabric_index = 1, .group = 1, .key_set_index = 2 };
    GroupDataProvider::GroupState state1b = { .fabric_index = 0, .group = 10, .key_set_index = 12 };

    GroupDataProvider::GroupState state3b = { .fabric_index = 0, .group = 10, .key_set_index = 13 };
    state3b.group                         = 10;
    state3b.key_set_index                 = 13;

    GroupDataProvider::GroupState state4a = { .fabric_index = 1, .group = 5, .key_set_index = 3 };
    GroupDataProvider::GroupState state4b = { .fabric_index = 0, .group = 10, .key_set_index = 14 };

    chip::FabricIndex fabric_index = 1;
    CHIP_ERROR err                 = CHIP_NO_ERROR;

    // First append
    err = groups->SetGroupState(0, state0a);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    // Second append
    err = groups->SetGroupState(1, state1a);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    // Attempt to set past the append slot
    err = groups->SetGroupState(3, state0a);
    NL_TEST_ASSERT(apSuite, CHIP_ERROR_INVALID_ARGUMENT == err);

    auto * it = groups->IterateGroupStates(fabric_index);
    NL_TEST_ASSERT(apSuite, it != nullptr);
    NL_TEST_ASSERT(apSuite, 2 == it->Count());
    it->Release();
    it = nullptr;

    err = groups->GetGroupState(0, state0b);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(apSuite, state0a.group == state0b.group);
    NL_TEST_ASSERT(apSuite, state0a.key_set_index == state0b.key_set_index);
    NL_TEST_ASSERT(apSuite, fabric_index == state0b.fabric_index);

    err = groups->GetGroupState(1, state1b);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(apSuite, state1a.group == state1b.group);
    NL_TEST_ASSERT(apSuite, state1a.key_set_index == state1b.key_set_index);
    NL_TEST_ASSERT(apSuite, fabric_index == state1b.fabric_index);

    err = groups->GetGroupState(2, state3b);
    NL_TEST_ASSERT(apSuite, CHIP_ERROR_KEY_NOT_FOUND == err);

    err = groups->RemoveGroupState(0);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    // Entry 1 should remain, now at slot 0
    state1b.group         = 10;
    state1b.key_set_index = 12;
    state1b.fabric_index  = 14;
    err                   = groups->GetGroupState(0, state1b);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(apSuite, state1a.group == state1b.group);
    NL_TEST_ASSERT(apSuite, state1a.key_set_index == state1b.key_set_index);
    NL_TEST_ASSERT(apSuite, fabric_index == state1b.fabric_index);

    state1b.group         = 10;
    state1b.key_set_index = 12;
    state1b.fabric_index  = 14;
    err                   = groups->GetGroupState(1, state1b);
    NL_TEST_ASSERT(apSuite, CHIP_ERROR_KEY_NOT_FOUND == err);

    err = groups->RemoveGroupState(0);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->GetGroupState(0, state1b);
    NL_TEST_ASSERT(apSuite, CHIP_ERROR_KEY_NOT_FOUND == err);

    // Test Override

    err = groups->SetGroupState(0, state0a);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->SetGroupState(0, state4a);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->GetGroupState(0, state4b);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(apSuite, state4a.group == state4b.group);
    NL_TEST_ASSERT(apSuite, state4a.key_set_index == state4b.key_set_index);
    NL_TEST_ASSERT(apSuite, state4a.fabric_index == state4b.fabric_index);

    // Incorrect fabric

    state4a.fabric_index = 3;
    err                  = groups->SetGroupState(0, state4a);
    NL_TEST_ASSERT(apSuite, CHIP_ERROR_ACCESS_DENIED == err);

    err = groups->RemoveGroupState(0);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    groups->Finish();
}

void TestStateIterator(nlTestSuite * apSuite, void * apContext)
{
    GroupDataProvider * groups = GetGroupDataProvider();
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->Init());

    chip::FabricIndex kFabricIndex1      = 1;
    chip::FabricIndex kFabricIndex2      = 2;
    GroupDataProvider::GroupState state0 = { .fabric_index = kFabricIndex1, .group = 1, .key_set_index = 1 }; // Fabric1
    GroupDataProvider::GroupState state1 = { .fabric_index = kFabricIndex1, .group = 2, .key_set_index = 1 }; // Fabric1
    GroupDataProvider::GroupState state2 = { .fabric_index = kFabricIndex2, .group = 2, .key_set_index = 2 }; // Fabric2
    GroupDataProvider::GroupState state3 = { .fabric_index = kFabricIndex1, .group = 3, .key_set_index = 1 }; // Fabric1
    CHIP_ERROR err                       = CHIP_NO_ERROR;

    NL_TEST_ASSERT(apSuite, groups);

    err = groups->SetGroupState(0, state0);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->SetGroupState(1, state1);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->SetGroupState(2, state2);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->SetGroupState(3, state3);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    {
        // Fabric Index 1 has 3 entries
        GroupDataProvider::GroupStateIterator * it = groups->IterateGroupStates(kFabricIndex1);
        NL_TEST_ASSERT(apSuite, it != nullptr);

        size_t count1 = it->Count();
        size_t count2 = 0;
        NL_TEST_ASSERT(apSuite, 3 == count1);
        GroupDataProvider::GroupState state;
        while (it->Next(state))
        {
            NL_TEST_ASSERT(apSuite, (state.group > 0 && state.group < 4) && (state.key_set_index == 1));
            NL_TEST_ASSERT(apSuite, (state.fabric_index == kFabricIndex1));
            count2++;
        }
        NL_TEST_ASSERT(apSuite, count2 == count1);
        it->Release();
        it = nullptr;
    }

    {
        // Fabric Index 2 has 1 entry
        GroupDataProvider::GroupStateIterator * it = groups->IterateGroupStates(kFabricIndex2);
        NL_TEST_ASSERT(apSuite, it != nullptr);

        size_t count1 = it->Count();
        NL_TEST_ASSERT(apSuite, 1 == count1);
        GroupDataProvider::GroupState state;
        NL_TEST_ASSERT(apSuite, it->Next(state));

        NL_TEST_ASSERT(apSuite, (state.group > 0 && state.group < 4) && (state.key_set_index == 2));
        NL_TEST_ASSERT(apSuite, (state.fabric_index == kFabricIndex2));

        NL_TEST_ASSERT(apSuite, !it->Next(state));

        it->Release();
        it = nullptr;
    }

    {
        // Fabric Index 1 has 3 entries + Fabric Index 2 has 1 entry
        GroupDataProvider::GroupStateIterator * it = groups->IterateGroupStates();
        NL_TEST_ASSERT(apSuite, it != nullptr);

        size_t count1 = it->Count();
        size_t count2 = 0;
        NL_TEST_ASSERT(apSuite, 4 == count1);
        GroupDataProvider::GroupState state;
        while (it->Next(state))
        {
            NL_TEST_ASSERT(apSuite, (state.fabric_index == kFabricIndex1 || state.fabric_index == kFabricIndex2));
            NL_TEST_ASSERT(apSuite, (state.group > 0 && state.group < 4) && (state.key_set_index == 1 || state.key_set_index == 2));
            count2++;
        }
        NL_TEST_ASSERT(apSuite, count2 == count1);
        it->Release();
        it = nullptr;
    }

    groups->Finish();
}

static GroupDataProvider::EpochKey epoch_keys0[3] = {
    { 0x0000000000000000, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
    { 0x0000000000000000, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
    { 0x0000000000000000, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } }
};
static GroupDataProvider::EpochKey epoch_keys1[3] = {
    { 0x1000000000000000, { 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f } },
    { 0x2000000000000000, { 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f } },
    { 0x3000000000000000, { 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f } },
};
static GroupDataProvider::EpochKey epoch_keys2[2] = {
    { 0xa000000000000000, { 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf } },
    { 0xb000000000000000, { 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf } },
};

void TestKeys(nlTestSuite * apSuite, void * apContext)
{
    GroupDataProvider * groups = GetGroupDataProvider();
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->Init());

    // Pairs keys0[a|b], keys1[a|b] have different values. [b] is used as Get target, so it
    // should get overwritten with the values from [a].
    KeySet keys0a                  = { .policy = KeySet::SecurityPolicy::kStandard, .num_keys_used = 3 };
    KeySet keys0b                  = { .policy = KeySet::SecurityPolicy::kLowLatency, .num_keys_used = 2 };
    KeySet keys1a                  = { .policy = KeySet::SecurityPolicy::kLowLatency, .num_keys_used = 3 };
    KeySet keys1b                  = { .policy = KeySet::SecurityPolicy::kStandard, .num_keys_used = 2 };
    KeySet keys3                   = { .policy = KeySet::SecurityPolicy::kStandard, .num_keys_used = 2 };
    chip::FabricIndex kFabricIndex = 1;
    CHIP_ERROR err                 = CHIP_NO_ERROR;

    NL_TEST_ASSERT(apSuite, groups);

    memcpy(keys0a.epoch_keys, epoch_keys1, sizeof(epoch_keys1));
    memcpy(keys0b.epoch_keys, epoch_keys0, sizeof(epoch_keys0));
    memcpy(keys1a.epoch_keys, epoch_keys1, sizeof(epoch_keys1));
    memcpy(keys1b.epoch_keys, epoch_keys0, sizeof(epoch_keys0));
    memcpy(keys3.epoch_keys, epoch_keys2, sizeof(epoch_keys2));

    err = groups->SetKeySet(kFabricIndex, 0, keys0a);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->SetKeySet(kFabricIndex, 1, keys1a);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    auto * it = groups->IterateKeySets(kFabricIndex);
    // err = groups->GetStateCount(fabric, count);
    NL_TEST_ASSERT(apSuite, it != nullptr);
    NL_TEST_ASSERT(apSuite, it->Count() == 2);

    err = groups->GetKeySet(kFabricIndex, 0, keys0b);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(apSuite, keys0a.policy == keys0b.policy);
    NL_TEST_ASSERT(apSuite, keys0a.num_keys_used == keys0b.num_keys_used);
    NL_TEST_ASSERT(apSuite, !memcmp(keys0a.epoch_keys, keys0b.epoch_keys, sizeof(keys0a.epoch_keys[0]) * keys0a.num_keys_used));

    err = groups->GetKeySet(kFabricIndex, 1, keys1b);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(apSuite, keys1a.policy == keys1b.policy);
    NL_TEST_ASSERT(apSuite, keys1a.num_keys_used == keys1b.num_keys_used);
    NL_TEST_ASSERT(apSuite, !memcmp(keys1a.epoch_keys, keys1b.epoch_keys, sizeof(keys1a.epoch_keys[0]) * keys1a.num_keys_used));

    err = groups->GetKeySet(kFabricIndex, 3, keys3);
    NL_TEST_ASSERT(apSuite, CHIP_ERROR_KEY_NOT_FOUND == err);

    err = groups->RemoveKeySet(kFabricIndex, 0);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->GetKeySet(kFabricIndex, 0, keys0b);
    NL_TEST_ASSERT(apSuite, CHIP_ERROR_KEY_NOT_FOUND == err);

    err = groups->GetKeySet(kFabricIndex, 1, keys1b);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    groups->Finish();
}

void TestKeysIterator(nlTestSuite * apSuite, void * apContext)
{
    GroupDataProvider * groups = GetGroupDataProvider();
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->Init());

    KeySet keys0                   = { .policy = KeySet::SecurityPolicy::kStandard, .num_keys_used = 3 };
    KeySet keys1                   = { .policy = KeySet::SecurityPolicy::kStandard, .num_keys_used = 2 };
    KeySet keys2                   = { .policy = KeySet::SecurityPolicy::kStandard, .num_keys_used = 3 };
    chip::FabricIndex kFabricIndex = 1;
    CHIP_ERROR err                 = CHIP_NO_ERROR;

    NL_TEST_ASSERT(apSuite, groups);

    memcpy(keys0.epoch_keys, epoch_keys1, sizeof(epoch_keys1));
    memcpy(keys1.epoch_keys, epoch_keys2, sizeof(epoch_keys2));
    memcpy(keys2.epoch_keys, epoch_keys1, sizeof(epoch_keys1));

    NL_TEST_ASSERT(apSuite, groups);

    err = groups->SetKeySet(kFabricIndex, 2, keys2);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->SetKeySet(kFabricIndex, 0, keys0);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->SetKeySet(kFabricIndex, 1, keys1);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    GroupDataProvider::KeySetIterator * it = groups->IterateKeySets(kFabricIndex);
    NL_TEST_ASSERT(apSuite, it);

    size_t count1 = it->Count();
    size_t count2 = 0;
    NL_TEST_ASSERT(apSuite, 3 == count1);
    GroupDataProvider::KeySet keys;

    uint16_t last_key_set_index = UINT16_MAX;

    while (it->Next(keys))
    {
        NL_TEST_ASSERT(apSuite, keys.key_set_index >= 0 && keys.key_set_index <= 2);
        NL_TEST_ASSERT(apSuite, keys.key_set_index != last_key_set_index);
        last_key_set_index = keys.key_set_index;
        count2++;
    }
    NL_TEST_ASSERT(apSuite, count2 == count1);
    it->Release();
    it = nullptr;

    groups->Finish();
}

} // namespace TestGroups
} // namespace app
} // namespace chip

namespace {

/**
 *  Set up the test suite.
 */
int Test_Setup(void * inContext)
{
    CHIP_ERROR error = chip::Platform::MemoryInit();

    if (error != CHIP_NO_ERROR)
    {
        return FAILURE;
    }

    return SUCCESS;
}

/**
 *  Tear down the test suite.
 */
int Test_Teardown(void * inContext)
{
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

const nlTest sTests[] = { NL_TEST_DEF("TestEndpoints", chip::app::TestGroups::TestEndpoints),
                          NL_TEST_DEF("TestEndpointIterator", chip::app::TestGroups::TestEndpointIterator),
                          NL_TEST_DEF("TestStates", chip::app::TestGroups::TestStates),
                          NL_TEST_DEF("TestStateIterator", chip::app::TestGroups::TestStateIterator),
                          NL_TEST_DEF("TestKeys", chip::app::TestGroups::TestKeys),
                          NL_TEST_DEF("TestKeysIterator", chip::app::TestGroups::TestKeysIterator),
                          NL_TEST_SENTINEL() };
} // namespace

int TestGroups()
{
    nlTestSuite theSuite = { "GroupDataProvider", &sTests[0], &Test_Setup, &Test_Teardown };

    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestGroups)
