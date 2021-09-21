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

// TODO: test insertion in more than 1 fabric
void TestEndpoints(nlTestSuite * apSuite, void * apContext)
{
    GroupDataProvider * groups              = GetGroupDataProvider();
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->Init());

    chip::FabricIndex kFabricIndex1         = 1;
    GroupDataProvider::GroupMapping group1a = { .endpoint = 1, .group = 1 };
    GroupDataProvider::GroupMapping group1b = { .endpoint = 1, .group = 2 };
    GroupDataProvider::GroupMapping group1c = { .endpoint = 1, .group = 3 };
    CHIP_ERROR err                          = CHIP_NO_ERROR;
    bool exists                             = false;

    NL_TEST_ASSERT(apSuite, groups);

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

    err = groups->RemoveAllGroupMappings(kFabricIndex1, 1);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    exists = groups->GroupMappingExists(kFabricIndex1, group1a);
    NL_TEST_ASSERT(apSuite, !exists);

    exists = groups->GroupMappingExists(kFabricIndex1, group1b);
    NL_TEST_ASSERT(apSuite, !exists);

    exists = groups->GroupMappingExists(kFabricIndex1, group1c);
    NL_TEST_ASSERT(apSuite, !exists);
}

void TestEndpointIterator(nlTestSuite * apSuite, void * apContext)
{
    GroupDataProvider * groups              = GetGroupDataProvider();
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->Init());

    chip::FabricIndex kFabricIndex          = 1;
    // TODO: Address the fact that this requires CHIP_CONFIG_MAX_GROUP_ENDPOINTS_PER_FABRIC to be >= 7.
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

    uint16_t count1 = it->Count();
    uint16_t count2 = 0;
    NL_TEST_ASSERT(apSuite, 2 == count1);
    while ((gid = it->Next()) != 0)
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
    while ((gid = it->Next()) != 0)
    {
        count2++;
        NL_TEST_ASSERT(apSuite, gid > 0 && gid < 4);
    }
    NL_TEST_ASSERT(apSuite, count2 == count1);
    it->Release();
    it = nullptr;
}

// TODO: Test overwrite of an entry
void TestStates(nlTestSuite * apSuite, void * apContext)
{
    GroupDataProvider * groups            = GetGroupDataProvider();
    NL_TEST_ASSERT(apSuite, groups);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->Init());

    GroupDataProvider::GroupState state0a = { .group = 1, .key_set_index = 1 };

    GroupDataProvider::GroupStateListEntry state0b{};
    state0b.group = 10;
    state0b.key_set_index = 11;

    GroupDataProvider::GroupState state1a = { .group = 1, .key_set_index = 2 };
    GroupDataProvider::GroupStateListEntry state1b{};
    state1b.group = 10;
    state1b.key_set_index = 12;

    GroupDataProvider::GroupStateListEntry state3b{};
    state3b.group = 10;
    state3b.key_set_index = 13;

    chip::FabricIndex fabric_index        = 1;
    CHIP_ERROR err                        = CHIP_NO_ERROR;

    // First append
    err = groups->SetGroupState(fabric_index, 0, state0a);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    // Second append
    err = groups->SetGroupState(fabric_index, 1, state1a);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    // Attempt to set past the append slot
    err = groups->SetGroupState(fabric_index, 3, state0a);
    NL_TEST_ASSERT(apSuite, CHIP_ERROR_INVALID_ARGUMENT == err);

    auto * it = groups->IterateGroupStates(fabric_index);
    NL_TEST_ASSERT(apSuite, it != nullptr);
    NL_TEST_ASSERT(apSuite, 2 == it->Count());
    it->Release();
    it = nullptr;

    err = groups->GetGroupState(fabric_index, 0, state0b);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(apSuite, state0a.group == state0b.group);
    NL_TEST_ASSERT(apSuite, state0a.key_set_index == state0b.key_set_index);
    NL_TEST_ASSERT(apSuite, fabric_index == state0b.fabric_index);
    NL_TEST_ASSERT(apSuite, 0 == state0b.list_index);

    err = groups->GetGroupState(fabric_index, 1, state1b);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(apSuite, state1a.group == state1b.group);
    NL_TEST_ASSERT(apSuite, state1a.key_set_index == state1b.key_set_index);
    NL_TEST_ASSERT(apSuite, fabric_index == state1b.fabric_index);
    NL_TEST_ASSERT(apSuite, 1 == state1b.list_index);

    err = groups->GetGroupState(fabric_index, 2, state3b);
    NL_TEST_ASSERT(apSuite, CHIP_ERROR_KEY_NOT_FOUND == err);

    err = groups->RemoveGroupState(fabric_index, 0);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    // Entry 1 should remain, now at slot 0
    state1b.group = 10;
    state1b.key_set_index = 12;
    state1b.list_index = 13;
    state1b.fabric_index = 14;
    err = groups->GetGroupState(fabric_index, 0, state1b);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(apSuite, state1a.group == state1b.group);
    NL_TEST_ASSERT(apSuite, state1a.key_set_index == state1b.key_set_index);
    NL_TEST_ASSERT(apSuite, 0 == state1b.list_index);
    NL_TEST_ASSERT(apSuite, fabric_index == state1b.fabric_index);

    state1b.group = 10;
    state1b.key_set_index = 12;
    state1b.list_index = 13;
    state1b.fabric_index = 14;
    err = groups->GetGroupState(fabric_index, 1, state1b);
    NL_TEST_ASSERT(apSuite, CHIP_ERROR_KEY_NOT_FOUND == err);

    err = groups->RemoveGroupState(fabric_index, 0);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->GetGroupState(fabric_index, 0, state1b);
    NL_TEST_ASSERT(apSuite, CHIP_ERROR_KEY_NOT_FOUND == err);

    // err = groups->GetStateCount(fabric, count);
    // NL_TEST_ASSERT(apSuite, 1 == count);
}

void TestStateIterator(nlTestSuite * apSuite, void * apContext)
{
    GroupDataProvider * groups           = GetGroupDataProvider();
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->Init());

    chip::FabricIndex kFabricIndex1      = 1;
    chip::FabricIndex kFabricIndex2      = 2;
    GroupDataProvider::GroupState state0 = { .group = 1, .key_set_index = 1 }; // Fabric1
    GroupDataProvider::GroupState state1 = { .group = 2, .key_set_index = 1 }; // Fabric1
    GroupDataProvider::GroupState state2 = { .group = 2, .key_set_index = 2 }; // Fabric2
    GroupDataProvider::GroupState state3 = { .group = 3, .key_set_index = 1 }; // Fabric1
    CHIP_ERROR err                       = CHIP_NO_ERROR;

    NL_TEST_ASSERT(apSuite, groups);

    err = groups->SetGroupState(kFabricIndex1, 0, state0);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->SetGroupState(kFabricIndex1, 1, state1);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->SetGroupState(kFabricIndex2, 2, state2);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->SetGroupState(kFabricIndex1, 3, state3);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    {
        // Fabric Index 1 has 3 entries
        GroupDataProvider::GroupStateIterator * it = groups->IterateGroupStates(kFabricIndex1);
        NL_TEST_ASSERT(apSuite, it != nullptr);

        uint16_t count1 = it->Count();
        uint16_t count2 = 0;
        NL_TEST_ASSERT(apSuite, 3 == count1);
        const GroupDataProvider::GroupStateListEntry *state = nullptr;
        while ((state = it->Next()) != nullptr)
        {
            NL_TEST_ASSERT(apSuite, (state != nullptr));
            NL_TEST_ASSERT(apSuite, (state->group > 0 && state->group < 4) && (state->key_set_index == 1));
            // List index must be fabric-relative (no gaps)
            NL_TEST_ASSERT(apSuite, (state->list_index == count2));
            NL_TEST_ASSERT(apSuite, (state->fabric_index == kFabricIndex1));
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

        uint16_t count1 = it->Count();
        NL_TEST_ASSERT(apSuite, 1 == count1);
        const GroupDataProvider::GroupStateListEntry *state = it->Next();
        NL_TEST_ASSERT(apSuite, (state != nullptr));

        NL_TEST_ASSERT(apSuite, (state->group > 0 && state->group < 4) && (state->key_set_index == 2));
        NL_TEST_ASSERT(apSuite, (state->list_index == 0));
        NL_TEST_ASSERT(apSuite, (state->fabric_index == kFabricIndex2));

        NL_TEST_ASSERT(apSuite, it->Next() == nullptr);

        it->Release();
        it = nullptr;
    }
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
    GroupDataProvider * groups       = GetGroupDataProvider();
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->Init());

    // Pairs keys0[a|b], keys1[a|b] have different values. [b] is used as Get target, so it
    // should get overwritten with the values from [a].
    KeySet keys0a = { .key_set_index = 0, .policy = KeySet::SecurityPolicy::kStandard, .num_keys_used = 3 };
    KeySet keys0b = { .key_set_index = 0, .policy = KeySet::SecurityPolicy::kLowLatency, .num_keys_used = 2 };
    KeySet keys1a = { .key_set_index = 1, .policy = KeySet::SecurityPolicy::kLowLatency, .num_keys_used = 3 };
    KeySet keys1b = { .key_set_index = 1, .policy = KeySet::SecurityPolicy::kStandard, .num_keys_used = 2 };
    KeySet keys3  = { .key_set_index = 3, .policy = KeySet::SecurityPolicy::kStandard, .num_keys_used = 2 };
    chip::FabricIndex kFabricIndex   = 1;
    CHIP_ERROR err                   = CHIP_NO_ERROR;

    NL_TEST_ASSERT(apSuite, groups);

    memcpy(keys0a.epoch_keys, epoch_keys1, sizeof(epoch_keys1));
    memcpy(keys0b.epoch_keys, epoch_keys0, sizeof(epoch_keys0));
    memcpy(keys1a.epoch_keys, epoch_keys1, sizeof(epoch_keys1));
    memcpy(keys1b.epoch_keys, epoch_keys0, sizeof(epoch_keys0));
    memcpy(keys3.epoch_keys, epoch_keys2, sizeof(epoch_keys2));

    err = groups->SetKeySet(kFabricIndex, keys0a);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->SetKeySet(kFabricIndex, keys1a);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    auto * it = groups->IterateKeySets(kFabricIndex);
    // err = groups->GetStateCount(fabric, count);
    NL_TEST_ASSERT(apSuite, it != nullptr);
    NL_TEST_ASSERT(apSuite, it->Count() == 2);

    err = groups->GetKeySet(kFabricIndex, keys0b);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(apSuite, keys0a.policy == keys0b.policy);
    // TODO: Make sure we have `num_keys_used` handled properly
    NL_TEST_ASSERT(apSuite, keys0a.num_keys_used == keys0b.num_keys_used);
    NL_TEST_ASSERT(apSuite, !memcmp(keys0a.epoch_keys, keys0b.epoch_keys, sizeof(keys0a.epoch_keys[0]) * keys0a.num_keys_used));

    err = groups->GetKeySet(kFabricIndex, keys1b);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(apSuite, keys1a.policy == keys1b.policy);
    NL_TEST_ASSERT(apSuite, keys1a.num_keys_used == keys1b.num_keys_used);
    NL_TEST_ASSERT(apSuite, !memcmp(keys1a.epoch_keys, keys1b.epoch_keys, sizeof(keys1a.epoch_keys[0]) * keys1a.num_keys_used));

    err = groups->GetKeySet(kFabricIndex, keys3);
    NL_TEST_ASSERT(apSuite, CHIP_ERROR_KEY_NOT_FOUND == err);

    err = groups->RemoveKeySet(kFabricIndex, 0);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->GetKeySet(kFabricIndex, keys0b);
    NL_TEST_ASSERT(apSuite, CHIP_ERROR_KEY_NOT_FOUND == err);

    err = groups->GetKeySet(kFabricIndex, keys1b);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);
}

void TestKeysIterator(nlTestSuite * apSuite, void * apContext)
{
    GroupDataProvider * groups      = GetGroupDataProvider();
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->Init());

    KeySet keys0 = { .key_set_index = 0, .policy = KeySet::SecurityPolicy::kStandard, .num_keys_used = 3 };
    KeySet keys1 = { .key_set_index = 1, .policy = KeySet::SecurityPolicy::kStandard, .num_keys_used = 2 };
    KeySet keys2 = { .key_set_index = 2, .policy = KeySet::SecurityPolicy::kStandard, .num_keys_used = 3 };
    chip::FabricIndex kFabricIndex  = 1;
    CHIP_ERROR err                  = CHIP_NO_ERROR;

    NL_TEST_ASSERT(apSuite, groups);

    memcpy(keys0.epoch_keys, epoch_keys1, sizeof(epoch_keys1));
    memcpy(keys1.epoch_keys, epoch_keys2, sizeof(epoch_keys2));
    memcpy(keys2.epoch_keys, epoch_keys1, sizeof(epoch_keys1));

    NL_TEST_ASSERT(apSuite, groups);

    err = groups->SetKeySet(kFabricIndex, keys2);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->SetKeySet(kFabricIndex, keys0);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->SetKeySet(kFabricIndex, keys1);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    GroupDataProvider::KeySetIterator * it = groups->IterateKeySets(kFabricIndex);
    NL_TEST_ASSERT(apSuite, it);

    uint16_t count1 = it->Count();
    uint16_t count2 = 0;
    NL_TEST_ASSERT(apSuite, 3 == count1);
    const GroupDataProvider::KeySet *keys = nullptr;

    uint16_t last_key_set_index = UINT16_MAX;

    while ((keys = it->Next()) != nullptr)
    {
        NL_TEST_ASSERT(apSuite, keys && keys->key_set_index >= 0 && keys->key_set_index <= 2);
        NL_TEST_ASSERT(apSuite, keys->key_set_index != last_key_set_index);
        last_key_set_index = keys->key_set_index;
        count2++;
    }
    NL_TEST_ASSERT(apSuite, count2 == count1);
    it->Release();
    it = nullptr;
}

} // namespace TestGroups
} // namespace Credentials
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
}

int TestGroups()
{
    // TODO: Use .initialize/.finalize to setup the groups data provider for each test.
    nlTestSuite theSuite = { "GroupDataProvider", &sTests[0], &Test_Setup, &Test_Teardown };

    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestGroups)
