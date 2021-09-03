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

/**
 *    @file
 *      This file implements unit tests for CommandPathParams
 *
 */

#include <credentials/GroupDataProvider.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>
#include <string.h>

using namespace chip::Credentials;

namespace chip {
namespace app {
namespace TestGroups {

void TestEndpoints(nlTestSuite * apSuite, void * apContext)
{
    GroupDataProvider * groups              = GetGroupDataProvider();
    chip::FabricIndex fabric                = 1;
    GroupDataProvider::GroupMapping group1a = { .endpoint = 1, .group = 1 };
    GroupDataProvider::GroupMapping group1b = { .endpoint = 1, .group = 2 };
    GroupDataProvider::GroupMapping group1c = { .endpoint = 1, .group = 3 };
    CHIP_ERROR err                          = CHIP_NO_ERROR;
    bool exists                             = false;

    NL_TEST_ASSERT(apSuite, groups);

    exists = groups->ExistsGroupMapping(fabric, group1a);
    NL_TEST_ASSERT(apSuite, !exists);

    err = groups->AddGroupMapping(fabric, group1a, "Group 1.1");
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->AddGroupMapping(fabric, group1c, "Group 1.3");
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    exists = groups->ExistsGroupMapping(fabric, group1a);
    NL_TEST_ASSERT(apSuite, exists);

    exists = groups->ExistsGroupMapping(fabric, group1b);
    NL_TEST_ASSERT(apSuite, !exists);

    exists = groups->ExistsGroupMapping(fabric, group1c);
    NL_TEST_ASSERT(apSuite, exists);

    err = groups->RemoveGroupMapping(fabric, group1a);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    exists = groups->ExistsGroupMapping(fabric, group1a);
    NL_TEST_ASSERT(apSuite, !exists);

    exists = groups->ExistsGroupMapping(fabric, group1b);
    NL_TEST_ASSERT(apSuite, !exists);

    exists = groups->ExistsGroupMapping(fabric, group1c);
    NL_TEST_ASSERT(apSuite, exists);

    err = groups->AddGroupMapping(fabric, group1a, "Group 1.1b");
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->AddGroupMapping(fabric, group1b, "Group 1.2");
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    exists = groups->ExistsGroupMapping(fabric, group1a);
    NL_TEST_ASSERT(apSuite, exists);

    exists = groups->ExistsGroupMapping(fabric, group1b);
    NL_TEST_ASSERT(apSuite, exists);

    exists = groups->ExistsGroupMapping(fabric, group1c);
    NL_TEST_ASSERT(apSuite, exists);

    err = groups->RemoveGroupAllMappings(fabric, 1);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    exists = groups->ExistsGroupMapping(fabric, group1a);
    NL_TEST_ASSERT(apSuite, !exists);

    exists = groups->ExistsGroupMapping(fabric, group1b);
    NL_TEST_ASSERT(apSuite, !exists);

    exists = groups->ExistsGroupMapping(fabric, group1c);
    NL_TEST_ASSERT(apSuite, !exists);
}

void TestEndpointIterator(nlTestSuite * apSuite, void * apContext)
{
    GroupDataProvider * groups              = GetGroupDataProvider();
    chip::FabricIndex fabric                = 1;
    GroupDataProvider::GroupMapping group1a = { .endpoint = 1, .group = 1 };
    GroupDataProvider::GroupMapping group1b = { .endpoint = 1, .group = 3 };
    GroupDataProvider::GroupMapping group2a = { .endpoint = 2, .group = 2 };
    GroupDataProvider::GroupMapping group3a = { .endpoint = 3, .group = 1 };
    GroupDataProvider::GroupMapping group3b = { .endpoint = 3, .group = 2 };
    GroupDataProvider::GroupMapping group3c = { .endpoint = 3, .group = 3 };
    CHIP_ERROR err                          = CHIP_NO_ERROR;

    NL_TEST_ASSERT(apSuite, groups);

    err = groups->AddGroupMapping(fabric, group3b, "Group 3.2");
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->AddGroupMapping(fabric, group2a, "Group 2.2");
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->AddGroupMapping(fabric, group1b, "Group 1.3");
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->AddGroupMapping(fabric, group3a, "Group 3.1");
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->AddGroupMapping(fabric, group1a, "Group 1.1");
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->AddGroupMapping(fabric, group3c, "Group 3.3");
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->AddGroupMapping(fabric, group3a, "Group 3.1");
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    EndpointId endpoint                          = 1;
    GroupDataProvider::GroupMappingIterator * it = groups->IterateGroupMappings(fabric, endpoint);
    NL_TEST_ASSERT(apSuite, it);

    // Endpoint 1
    uint16_t count1 = it->Count();
    uint16_t count2 = 0;
    NL_TEST_ASSERT(apSuite, 2 == count1);
    while (it->HasNext())
    {
        count2++;
        GroupId gid = it->Next();
        NL_TEST_ASSERT(apSuite, 1 == gid || 3 == gid);
    }
    NL_TEST_ASSERT(apSuite, count2 == count1);
    delete it;

    // Endpoint 3
    endpoint = 3;
    it       = groups->IterateGroupMappings(fabric, endpoint);
    NL_TEST_ASSERT(apSuite, it);

    count1 = it->Count();
    count2 = 0;
    NL_TEST_ASSERT(apSuite, 3 == count1);
    while (it->HasNext())
    {
        count2++;
        GroupId gid = it->Next();
        NL_TEST_ASSERT(apSuite, gid > 0 && gid < 4);
    }
    NL_TEST_ASSERT(apSuite, count2 == count1);
    delete it;
}

void TestStates(nlTestSuite * apSuite, void * apContext)
{
    GroupDataProvider * groups            = GetGroupDataProvider();
    GroupDataProvider::GroupState state0a = { .state_index = 0, .group = 1, .key_set_index = 1 };
    GroupDataProvider::GroupState state0b = { .state_index = 0, .group = 0, .key_set_index = 0 };
    GroupDataProvider::GroupState state1a = { .state_index = 1, .group = 1, .key_set_index = 2 };
    GroupDataProvider::GroupState state1b = { .state_index = 1, .group = 0, .key_set_index = 0 };
    GroupDataProvider::GroupState state3  = { .state_index = 3, .group = 1, .key_set_index = 3 };
    chip::FabricIndex fabric              = 1;
    CHIP_ERROR err                        = CHIP_NO_ERROR;

    NL_TEST_ASSERT(apSuite, groups);

    err = groups->SetGroupState(fabric, state0a);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->SetGroupState(fabric, state1a);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    // err = groups->GetStateCount(fabric, count);
    // NL_TEST_ASSERT(apSuite, 2 == count);

    err = groups->GetGroupState(fabric, state0b);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(apSuite, state0a.group == state0b.group);
    NL_TEST_ASSERT(apSuite, state0a.key_set_index == state0b.key_set_index);

    err = groups->GetGroupState(fabric, state1b);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(apSuite, state1a.group == state1b.group);
    NL_TEST_ASSERT(apSuite, state1a.key_set_index == state1b.key_set_index);

    err = groups->GetGroupState(fabric, state3);
    NL_TEST_ASSERT(apSuite, CHIP_ERROR_KEY_NOT_FOUND == err);

    err = groups->RemoveGroupState(fabric, 1);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->GetGroupState(fabric, state0b);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->GetGroupState(fabric, state1b);
    NL_TEST_ASSERT(apSuite, CHIP_ERROR_KEY_NOT_FOUND == err);

    // err = groups->GetStateCount(fabric, count);
    // NL_TEST_ASSERT(apSuite, 1 == count);
}

void TestStateIterator(nlTestSuite * apSuite, void * apContext)
{
    GroupDataProvider * groups           = GetGroupDataProvider();
    chip::FabricIndex fabric             = 1;
    GroupDataProvider::GroupState state0 = { .state_index = 0, .group = 1, .key_set_index = 1 };
    GroupDataProvider::GroupState state1 = { .state_index = 1, .group = 1, .key_set_index = 2 };
    GroupDataProvider::GroupState state2 = { .state_index = 2, .group = 2, .key_set_index = 1 };
    GroupDataProvider::GroupState state3 = { .state_index = 3, .group = 3, .key_set_index = 3 };
    CHIP_ERROR err                       = CHIP_NO_ERROR;

    NL_TEST_ASSERT(apSuite, groups);

    err = groups->SetGroupState(fabric, state2);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->SetGroupState(fabric, state3);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->SetGroupState(fabric, state0);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->SetGroupState(fabric, state1);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    GroupDataProvider::GroupStateIterator * it = groups->IterateGroupStates(fabric);
    NL_TEST_ASSERT(apSuite, it);

    // Endpoint 1
    uint16_t count1 = it->Count();
    uint16_t count2 = 0;
    NL_TEST_ASSERT(apSuite, 4 == count1);
    while (it->HasNext())
    {
        count2++;
        const GroupDataProvider::GroupState * state = it->Next();
        NL_TEST_ASSERT(apSuite, state && state->group > 0 && state->group < 4);
    }
    NL_TEST_ASSERT(apSuite, count2 == count1);
    delete it;
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
static GroupDataProvider::EpochKey epoch_keys2[3] = {
    { 0xa000000000000000, { 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf } },
    { 0xb000000000000000, { 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf } },
    { 0xc000000000000000, { 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf } },
};

void TestKeys(nlTestSuite * apSuite, void * apContext)
{
    GroupDataProvider * groups       = GetGroupDataProvider();
    GroupDataProvider::KeySet keys0a = { .key_set_index = 0, .policy = GroupDataProvider::KeySet::SecurityPolicy::kStandard };
    GroupDataProvider::KeySet keys0b = { .key_set_index = 0, .policy = GroupDataProvider::KeySet::SecurityPolicy::kStandard };
    GroupDataProvider::KeySet keys1a = { .key_set_index = 1, .policy = GroupDataProvider::KeySet::SecurityPolicy::kStandard };
    GroupDataProvider::KeySet keys1b = { .key_set_index = 1, .policy = GroupDataProvider::KeySet::SecurityPolicy::kStandard };
    GroupDataProvider::KeySet keys3  = { .key_set_index = 3, .policy = GroupDataProvider::KeySet::SecurityPolicy::kStandard };
    chip::FabricIndex fabric         = 1;
    CHIP_ERROR err                   = CHIP_NO_ERROR;

    NL_TEST_ASSERT(apSuite, groups);

    memcpy(keys0a.epoch_keys, epoch_keys1, sizeof(epoch_keys1));
    memcpy(keys0b.epoch_keys, epoch_keys0, sizeof(epoch_keys0));
    memcpy(keys1a.epoch_keys, epoch_keys1, sizeof(epoch_keys1));
    memcpy(keys1b.epoch_keys, epoch_keys0, sizeof(epoch_keys0));
    memcpy(keys3.epoch_keys, epoch_keys2, sizeof(epoch_keys2));

    err = groups->SetKeySet(fabric, keys0a);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->SetKeySet(fabric, keys1a);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    // err = groups->GetStateCount(fabric, count);
    // NL_TEST_ASSERT(apSuite, 2 == count);

    err = groups->GetKeySet(fabric, keys0b);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(apSuite, keys0a.policy == keys0b.policy);
    NL_TEST_ASSERT(apSuite, !memcmp(keys0a.epoch_keys, keys0b.epoch_keys, sizeof(keys0a.epoch_keys)));

    err = groups->GetKeySet(fabric, keys1b);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(apSuite, keys1a.policy == keys1b.policy);
    NL_TEST_ASSERT(apSuite, !memcmp(keys1a.epoch_keys, keys1b.epoch_keys, sizeof(keys1a.epoch_keys)));

    err = groups->GetKeySet(fabric, keys3);
    NL_TEST_ASSERT(apSuite, CHIP_ERROR_KEY_NOT_FOUND == err);

    err = groups->RemoveKeySet(fabric, 1);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->GetKeySet(fabric, keys0b);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->GetKeySet(fabric, keys1b);
    NL_TEST_ASSERT(apSuite, CHIP_ERROR_KEY_NOT_FOUND == err);
}

void TestKeysIterator(nlTestSuite * apSuite, void * apContext)
{
    GroupDataProvider * groups      = GetGroupDataProvider();
    GroupDataProvider::KeySet keys0 = { .key_set_index = 0, .policy = GroupDataProvider::KeySet::SecurityPolicy::kStandard };
    GroupDataProvider::KeySet keys1 = { .key_set_index = 1, .policy = GroupDataProvider::KeySet::SecurityPolicy::kStandard };
    GroupDataProvider::KeySet keys2 = { .key_set_index = 2, .policy = GroupDataProvider::KeySet::SecurityPolicy::kStandard };
    GroupDataProvider::KeySet keys3 = { .key_set_index = 3, .policy = GroupDataProvider::KeySet::SecurityPolicy::kStandard };
    GroupDataProvider::KeySet keys4 = { .key_set_index = 4, .policy = GroupDataProvider::KeySet::SecurityPolicy::kStandard };
    chip::FabricIndex fabric        = 1;
    CHIP_ERROR err                  = CHIP_NO_ERROR;

    NL_TEST_ASSERT(apSuite, groups);

    memcpy(keys0.epoch_keys, epoch_keys1, sizeof(epoch_keys1));
    memcpy(keys1.epoch_keys, epoch_keys2, sizeof(epoch_keys2));
    memcpy(keys2.epoch_keys, epoch_keys1, sizeof(epoch_keys1));
    memcpy(keys3.epoch_keys, epoch_keys2, sizeof(epoch_keys2));
    memcpy(keys4.epoch_keys, epoch_keys1, sizeof(epoch_keys1));

    NL_TEST_ASSERT(apSuite, groups);

    err = groups->SetKeySet(fabric, keys2);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->SetKeySet(fabric, keys3);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->SetKeySet(fabric, keys0);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->SetKeySet(fabric, keys4);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->SetKeySet(fabric, keys1);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    GroupDataProvider::KeySetIterator * it = groups->IterateKeySets(fabric);
    NL_TEST_ASSERT(apSuite, it);

    uint16_t count1 = it->Count();
    uint16_t count2 = 0;
    NL_TEST_ASSERT(apSuite, 5 == count1);
    while (it->HasNext())
    {
        count2++;
        const GroupDataProvider::KeySet * keys = it->Next();
        NL_TEST_ASSERT(apSuite, keys && keys->key_set_index >= 0 && keys->key_set_index <= 4);
    }
    NL_TEST_ASSERT(apSuite, count2 == count1);
    delete it;
}

} // namespace TestGroups
} // namespace app
} // namespace chip

namespace {
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
    nlTestSuite theSuite = { "GroupDataProvider", &sTests[0], nullptr, nullptr };

    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestGroups)
