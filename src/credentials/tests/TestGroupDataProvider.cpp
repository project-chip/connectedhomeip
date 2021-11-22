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

#include "TestPersistentStorageDelegate.h"
#include <credentials/GroupDataProviderImpl.h>
#include <lib/core/CHIPTLV.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>
#include <platform/KeyValueStoreManager.h>
#include <set>
#include <string.h>

using namespace chip::Credentials;
using GroupMapping = GroupDataProvider::GroupMapping;
using Keyset       = GroupDataProvider::Keyset;
using GroupState   = GroupDataProvider::GroupState;
using EpochKey     = GroupDataProvider::EpochKey;

namespace chip {
namespace app {
namespace TestGroups {

static const char * kKey1   = "abc/def";
static const char * kValue1 = "abc/def";
static const char * kValue2 = "abc/ghi/xyz";
static const size_t kSize1  = strlen(kValue1) + 1;
static const size_t kSize2  = strlen(kValue2) + 1;

void TestStorageDelegate(nlTestSuite * apSuite, void * apContext)
{
    chip::TestPersistentStorageDelegate delegate;

    char out[128];
    uint16_t size = static_cast<uint16_t>(sizeof(out));

    NL_TEST_ASSERT(apSuite, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND == delegate.SyncGetKeyValue(kKey1, out, size));

    size = static_cast<uint16_t>(kSize1);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == delegate.SyncSetKeyValue(kKey1, kValue1, size));

    size = static_cast<uint16_t>(sizeof(out));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == delegate.SyncGetKeyValue(kKey1, out, size));
    NL_TEST_ASSERT(apSuite, size == kSize1);
    NL_TEST_ASSERT(apSuite, !memcmp(out, kValue1, kSize1));

    size = static_cast<uint16_t>(kSize2);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == delegate.SyncSetKeyValue(kKey1, kValue2, size));

    size = static_cast<uint16_t>(sizeof(out));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == delegate.SyncGetKeyValue(kKey1, out, size));
    NL_TEST_ASSERT(apSuite, size == kSize2);
    NL_TEST_ASSERT(apSuite, !memcmp(out, kValue2, kSize2));

    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == delegate.SyncDeleteKeyValue(kKey1));

    size = static_cast<uint16_t>(sizeof(out));
    NL_TEST_ASSERT(apSuite, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND == delegate.SyncGetKeyValue(kKey1, out, size));
}

constexpr chip::FabricIndex kFabric1  = 1;
constexpr chip::FabricIndex kFabric2  = 7;
constexpr chip::EndpointId kEndpoint1 = 1;
constexpr chip::EndpointId kEndpoint2 = 0xabcd;
constexpr chip::EndpointId kEndpoint3 = 0xfffe;
constexpr chip::GroupId kGroup1       = kMinFabricGroupId;
constexpr chip::GroupId kGroup2       = 0x2222;
constexpr chip::GroupId kGroup3       = kMaxFabricGroupId;

static const GroupMapping endpoint1group1(kEndpoint1, kGroup1, "Group 1.1");
static const GroupMapping endpoint1group2(kEndpoint1, kGroup2, "Group 1.2");
static const GroupMapping endpoint1group3(kEndpoint1, kGroup3, "Group 1.3");

static const GroupMapping endpoint2group1(kEndpoint2, kGroup1, "Group 2.1");
static const GroupMapping endpoint2group2(kEndpoint2, kGroup2, "Group 2.2");
static const GroupMapping endpoint2group3(kEndpoint2, kGroup3, "Group 2.3");

static const GroupMapping endpoint3group1(kEndpoint3, kGroup1, "Group 3.1");
static const GroupMapping endpoint3group2(kEndpoint3, kGroup2, "Group 3.2");
static const GroupMapping endpoint3group3(kEndpoint3, kGroup3, "Group 3.3");

void TestGroupMappings(nlTestSuite * apSuite, void * apContext)
{
    GroupDataProvider * groups = GetGroupDataProvider();
    NL_TEST_ASSERT(apSuite, groups);

    // Reset test

    groups->RemoveAllGroupMappings(kFabric1, kEndpoint1);
    groups->RemoveAllGroupMappings(kFabric1, kEndpoint2);
    groups->RemoveAllGroupMappings(kFabric1, kEndpoint3);
    groups->RemoveAllGroupMappings(kFabric2, kEndpoint1);
    groups->RemoveAllGroupMappings(kFabric2, kEndpoint2);
    groups->RemoveAllGroupMappings(kFabric2, kEndpoint3);

    // Test initial conditions

    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric1, endpoint1group1));
    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric1, endpoint1group2));
    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric1, endpoint1group3));

    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric1, endpoint2group1));
    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric1, endpoint2group2));
    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric1, endpoint2group3));

    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric1, endpoint3group1));
    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric1, endpoint3group2));
    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric1, endpoint3group3));

    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric2, endpoint1group1));
    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric2, endpoint1group2));
    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric2, endpoint1group3));

    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric2, endpoint2group1));
    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric2, endpoint2group2));
    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric2, endpoint2group3));

    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric2, endpoint3group1));
    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric2, endpoint3group2));
    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric2, endpoint3group3));

    // Add Group (new)

    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->AddGroupMapping(kFabric1, endpoint1group1));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->AddGroupMapping(kFabric2, endpoint3group2));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->AddGroupMapping(kFabric1, endpoint3group3));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->AddGroupMapping(kFabric2, endpoint2group2));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->AddGroupMapping(kFabric2, endpoint1group2));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->AddGroupMapping(kFabric2, endpoint3group1));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->AddGroupMapping(kFabric2, endpoint2group3));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->AddGroupMapping(kFabric1, endpoint3group1));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->AddGroupMapping(kFabric2, endpoint1group3));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->AddGroupMapping(kFabric1, endpoint1group3));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->AddGroupMapping(kFabric1, endpoint2group1));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->AddGroupMapping(kFabric1, endpoint2group2));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->AddGroupMapping(kFabric2, endpoint2group1));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->AddGroupMapping(kFabric1, endpoint2group3));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->AddGroupMapping(kFabric1, endpoint3group2));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->AddGroupMapping(kFabric2, endpoint1group1));
    // Keep these out to check for unexisting groups:
    // NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->AddGroupMapping(kFabric1, endpoint1group2));
    // NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->AddGroupMapping(kFabric2, endpoint3group3));

    NL_TEST_ASSERT(apSuite, groups->GroupMappingExists(kFabric1, endpoint1group1));
    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric1, endpoint1group2));
    NL_TEST_ASSERT(apSuite, groups->GroupMappingExists(kFabric1, endpoint1group3));
    NL_TEST_ASSERT(apSuite, groups->GroupMappingExists(kFabric1, endpoint2group1));
    NL_TEST_ASSERT(apSuite, groups->GroupMappingExists(kFabric1, endpoint2group2));
    NL_TEST_ASSERT(apSuite, groups->GroupMappingExists(kFabric1, endpoint2group3));
    NL_TEST_ASSERT(apSuite, groups->GroupMappingExists(kFabric1, endpoint3group1));
    NL_TEST_ASSERT(apSuite, groups->GroupMappingExists(kFabric1, endpoint3group2));
    NL_TEST_ASSERT(apSuite, groups->GroupMappingExists(kFabric1, endpoint3group3));
    NL_TEST_ASSERT(apSuite, groups->GroupMappingExists(kFabric2, endpoint1group1));
    NL_TEST_ASSERT(apSuite, groups->GroupMappingExists(kFabric2, endpoint1group2));
    NL_TEST_ASSERT(apSuite, groups->GroupMappingExists(kFabric2, endpoint1group3));
    NL_TEST_ASSERT(apSuite, groups->GroupMappingExists(kFabric2, endpoint2group1));
    NL_TEST_ASSERT(apSuite, groups->GroupMappingExists(kFabric2, endpoint2group2));
    NL_TEST_ASSERT(apSuite, groups->GroupMappingExists(kFabric2, endpoint2group3));
    NL_TEST_ASSERT(apSuite, groups->GroupMappingExists(kFabric2, endpoint3group1));
    NL_TEST_ASSERT(apSuite, groups->GroupMappingExists(kFabric2, endpoint3group2));
    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric2, endpoint3group3));

    // Add Group (duplicated)

    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->AddGroupMapping(kFabric1, endpoint3group3));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->AddGroupMapping(kFabric2, endpoint1group2));

    // Remove Group (invalid)

    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR != groups->RemoveGroupMapping(kFabric1, GroupMapping(kInvalidEndpointId, 1, nullptr)));
    NL_TEST_ASSERT(apSuite,
                   CHIP_NO_ERROR != groups->RemoveGroupMapping(kFabric1, GroupMapping(kEndpoint1, kUndefinedGroupId, nullptr)));

    // Remove Group (existing)

    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->RemoveGroupMapping(kFabric1, endpoint1group1)); // First
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->RemoveGroupMapping(kFabric2, endpoint2group1)); // Last
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->RemoveGroupMapping(kFabric2, endpoint2group2)); // Middle

    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric1, endpoint1group1));
    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric1, endpoint1group2));
    NL_TEST_ASSERT(apSuite, groups->GroupMappingExists(kFabric1, endpoint1group3));
    NL_TEST_ASSERT(apSuite, groups->GroupMappingExists(kFabric1, endpoint2group1));
    NL_TEST_ASSERT(apSuite, groups->GroupMappingExists(kFabric1, endpoint2group2));
    NL_TEST_ASSERT(apSuite, groups->GroupMappingExists(kFabric1, endpoint2group3));
    NL_TEST_ASSERT(apSuite, groups->GroupMappingExists(kFabric1, endpoint3group1));
    NL_TEST_ASSERT(apSuite, groups->GroupMappingExists(kFabric1, endpoint3group2));
    NL_TEST_ASSERT(apSuite, groups->GroupMappingExists(kFabric1, endpoint3group3));
    NL_TEST_ASSERT(apSuite, groups->GroupMappingExists(kFabric2, endpoint1group1));
    NL_TEST_ASSERT(apSuite, groups->GroupMappingExists(kFabric2, endpoint1group2));
    NL_TEST_ASSERT(apSuite, groups->GroupMappingExists(kFabric2, endpoint1group3));
    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric2, endpoint2group1));
    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric2, endpoint2group2));
    NL_TEST_ASSERT(apSuite, groups->GroupMappingExists(kFabric2, endpoint2group3));
    NL_TEST_ASSERT(apSuite, groups->GroupMappingExists(kFabric2, endpoint3group1));
    NL_TEST_ASSERT(apSuite, groups->GroupMappingExists(kFabric2, endpoint3group2));
    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric2, endpoint3group3));

    // Remove Group (already removed)

    NL_TEST_ASSERT(apSuite, CHIP_ERROR_KEY_NOT_FOUND == groups->RemoveGroupMapping(kFabric1, endpoint1group1));
    NL_TEST_ASSERT(apSuite, CHIP_ERROR_KEY_NOT_FOUND == groups->RemoveGroupMapping(kFabric2, endpoint2group1));
    NL_TEST_ASSERT(apSuite, CHIP_ERROR_KEY_NOT_FOUND == groups->RemoveGroupMapping(kFabric2, endpoint2group2));

    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric1, endpoint1group1));
    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric2, endpoint2group1));
    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric2, endpoint2group2));

    // Remove All

    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->RemoveAllGroupMappings(kFabric2, kEndpoint1));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->RemoveAllGroupMappings(kFabric1, kEndpoint3));

    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric1, endpoint1group1));
    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric1, endpoint1group2));
    NL_TEST_ASSERT(apSuite, groups->GroupMappingExists(kFabric1, endpoint1group3));
    NL_TEST_ASSERT(apSuite, groups->GroupMappingExists(kFabric1, endpoint2group1));
    NL_TEST_ASSERT(apSuite, groups->GroupMappingExists(kFabric1, endpoint2group2));
    NL_TEST_ASSERT(apSuite, groups->GroupMappingExists(kFabric1, endpoint2group3));
    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric1, endpoint3group1));
    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric1, endpoint3group2));
    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric1, endpoint3group3));
    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric2, endpoint1group1));
    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric2, endpoint1group2));
    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric2, endpoint1group3));
    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric2, endpoint2group1));
    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric2, endpoint2group2));
    NL_TEST_ASSERT(apSuite, groups->GroupMappingExists(kFabric2, endpoint2group3));
    NL_TEST_ASSERT(apSuite, groups->GroupMappingExists(kFabric2, endpoint3group1));
    NL_TEST_ASSERT(apSuite, groups->GroupMappingExists(kFabric2, endpoint3group2));
    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric2, endpoint3group3));

    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->RemoveAllGroupMappings(kFabric1, kEndpoint1));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->RemoveAllGroupMappings(kFabric2, kEndpoint2));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->RemoveAllGroupMappings(kFabric2, kEndpoint3));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->RemoveAllGroupMappings(kFabric1, kEndpoint2));

    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric1, endpoint1group1));
    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric1, endpoint1group2));
    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric1, endpoint1group3));
    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric1, endpoint2group1));
    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric1, endpoint2group2));
    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric1, endpoint2group3));
    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric1, endpoint3group1));
    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric1, endpoint3group2));
    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric1, endpoint3group3));
    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric2, endpoint1group1));
    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric2, endpoint1group2));
    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric2, endpoint1group3));
    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric2, endpoint2group1));
    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric2, endpoint2group2));
    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric2, endpoint2group3));
    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric2, endpoint3group1));
    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric2, endpoint3group2));
    NL_TEST_ASSERT(apSuite, !groups->GroupMappingExists(kFabric2, endpoint3group3));
}

void TestGroupMappingIterator(nlTestSuite * apSuite, void * apContext)
{
    GroupDataProvider * groups = GetGroupDataProvider();
    NL_TEST_ASSERT(apSuite, groups);

    // Reset test

    groups->RemoveAllGroupMappings(kFabric1, kEndpoint1);
    groups->RemoveAllGroupMappings(kFabric1, kEndpoint2);
    groups->RemoveAllGroupMappings(kFabric1, kEndpoint3);
    groups->RemoveAllGroupMappings(kFabric2, kEndpoint1);
    groups->RemoveAllGroupMappings(kFabric2, kEndpoint2);
    groups->RemoveAllGroupMappings(kFabric2, kEndpoint3);

    // Add Groups

    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->AddGroupMapping(kFabric1, endpoint1group1));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->AddGroupMapping(kFabric1, endpoint1group2));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->AddGroupMapping(kFabric1, endpoint1group3));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->AddGroupMapping(kFabric1, endpoint2group1));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->AddGroupMapping(kFabric1, endpoint3group1));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->AddGroupMapping(kFabric1, endpoint2group2));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->AddGroupMapping(kFabric1, endpoint3group2));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->AddGroupMapping(kFabric1, endpoint2group3));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->AddGroupMapping(kFabric1, endpoint3group3));

    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->AddGroupMapping(kFabric2, endpoint1group1));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->AddGroupMapping(kFabric2, endpoint2group2));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->AddGroupMapping(kFabric2, endpoint2group1));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->AddGroupMapping(kFabric2, endpoint3group3));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->AddGroupMapping(kFabric2, endpoint3group2));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->AddGroupMapping(kFabric2, endpoint3group1));

    // Fabric 1

    GroupMapping mapping;

    constexpr size_t endpoints_count                = 3;
    constexpr EndpointId endpoints[endpoints_count] = { kEndpoint1, kEndpoint2, kEndpoint3 };

    std::set<GroupId> expected_f1{ kGroup1, kGroup2, kGroup3 };
    std::map<GroupId, int> group_map = { { kGroup1, 1 }, { kGroup2, 2 }, { kGroup3, 3 } };
    std::map<GroupId, int> end_map   = { { kEndpoint1, 1 }, { kEndpoint2, 2 }, { kEndpoint3, 3 } };
    char expected_name[64];
    size_t j = 0;

    for (size_t i = 0; i < endpoints_count; i++)
    {
        auto it = groups->IterateGroupMappings(kFabric1, endpoints[i]);
        NL_TEST_ASSERT(apSuite, it);
        if (it)
        {
            size_t count = it->Count();
            NL_TEST_ASSERT(apSuite, expected_f1.size() == count);
            j = 0;
            while (it->Next(mapping) && j < expected_f1.size())
            {
                sprintf(expected_name, "Group %d.%d", end_map[mapping.endpoint], group_map[mapping.group]);
                NL_TEST_ASSERT(apSuite, expected_f1.count(mapping.group) > 0);
                NL_TEST_ASSERT(apSuite, !strcmp(expected_name, mapping.name));
                j++;
            }
            NL_TEST_ASSERT(apSuite, j == expected_f1.size());
            it->Release();
        }
    }

    // // Fabric 2

    std::set<GroupId> expected_f2[3]                     = { { kGroup1, kUndefinedGroupId, kUndefinedGroupId },
                                         { kGroup2, kGroup1, kUndefinedGroupId },
                                         { kGroup3, kGroup2, kGroup1 } };
    constexpr size_t expected_f2_counts[endpoints_count] = { 1, 2, 3 };

    for (size_t i = 0; i < endpoints_count; i++)
    {
        auto it = groups->IterateGroupMappings(kFabric2, endpoints[i]);
        NL_TEST_ASSERT(apSuite, it);
        if (it)
        {
            size_t expected_count = expected_f2_counts[i];
            size_t count          = it->Count();

            NL_TEST_ASSERT(apSuite, expected_count == count);
            j = 0;
            while (it->Next(mapping) && j < expected_count)
            {
                sprintf(expected_name, "Group %d.%d", end_map[mapping.endpoint], group_map[mapping.group]);
                NL_TEST_ASSERT(apSuite, mapping.endpoint == endpoints[i]);
                NL_TEST_ASSERT(apSuite, expected_f2[i].count(mapping.group) > 0);
                NL_TEST_ASSERT(apSuite, !strcmp(expected_name, mapping.name));
                j++;
            }
            NL_TEST_ASSERT(apSuite, j == expected_count);
            it->Release();
        }
    }
}

constexpr uint16_t kKeyset1 = 101;
constexpr uint16_t kKeyset2 = 102;
constexpr uint16_t kKeyset3 = 103;

static const GroupState fabric0group0set0(0, 0, 0);

static const GroupState fabric1group1set1(kFabric1, kGroup1, kKeyset1);
static const GroupState fabric1group1set2(kFabric1, kGroup1, kKeyset2);
static const GroupState fabric1group1set3(kFabric1, kGroup1, kKeyset3);
static const GroupState fabric1group2set1(kFabric1, kGroup2, kKeyset1);
static const GroupState fabric1group2set2(kFabric1, kGroup2, kKeyset2);
static const GroupState fabric1group2set3(kFabric1, kGroup2, kKeyset3);

static const GroupState fabric2group1set1(kFabric2, kGroup1, kKeyset1);
static const GroupState fabric2group1set2(kFabric2, kGroup1, kKeyset2);
static const GroupState fabric2group1set3(kFabric2, kGroup1, kKeyset3);
static const GroupState fabric2group2set1(kFabric2, kGroup2, kKeyset1);
static const GroupState fabric2group2set2(kFabric2, kGroup2, kKeyset2);
static const GroupState fabric2group2set3(kFabric2, kGroup2, kKeyset3);

void TestGroupStates(nlTestSuite * apSuite, void * apContext)
{
    GroupDataProvider * groups = GetGroupDataProvider();
    NL_TEST_ASSERT(apSuite, groups);

    // Reset test

    groups->RemoveGroupState(5);
    groups->RemoveGroupState(4);
    groups->RemoveGroupState(3);
    groups->RemoveGroupState(2);
    groups->RemoveGroupState(1);
    groups->RemoveGroupState(0);

    // Add States

    GroupState state(0, 0, 0);

    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->SetGroupState(0, fabric1group1set1));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->SetGroupState(1, fabric1group1set2));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->SetGroupState(2, fabric1group1set3));

    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->GetGroupState(0, state));
    NL_TEST_ASSERT(apSuite, state == fabric1group1set1);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->GetGroupState(1, state));
    NL_TEST_ASSERT(apSuite, state == fabric1group1set2);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->GetGroupState(2, state));
    NL_TEST_ASSERT(apSuite, state == fabric1group1set3);

    // Override
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->SetGroupState(2, fabric1group2set1));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->SetGroupState(1, fabric1group2set2));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->SetGroupState(0, fabric1group2set3));

    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->GetGroupState(0, state));
    NL_TEST_ASSERT(apSuite, state == fabric1group2set3);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->GetGroupState(1, state));
    NL_TEST_ASSERT(apSuite, state == fabric1group2set2);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->GetGroupState(2, state));
    NL_TEST_ASSERT(apSuite, state == fabric1group2set1);
    // Invalid
    NL_TEST_ASSERT(apSuite, CHIP_ERROR_INVALID_ARGUMENT == groups->SetGroupState(4, fabric1group1set1));

    // Remove States

    NL_TEST_ASSERT(apSuite, CHIP_ERROR_KEY_NOT_FOUND == groups->RemoveGroupState(3));

    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->RemoveGroupState(0));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->GetGroupState(0, state));
    NL_TEST_ASSERT(apSuite, state == fabric1group2set2);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->GetGroupState(1, state));
    NL_TEST_ASSERT(apSuite, state == fabric1group2set1);

    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->RemoveGroupState(1));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->GetGroupState(0, state));
    NL_TEST_ASSERT(apSuite, state == fabric1group2set2);
    NL_TEST_ASSERT(apSuite, CHIP_ERROR_INVALID_ARGUMENT != groups->GetGroupState(1, state));

    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->RemoveGroupState(0));
    NL_TEST_ASSERT(apSuite, CHIP_ERROR_INVALID_ARGUMENT != groups->GetGroupState(0, state));
    NL_TEST_ASSERT(apSuite, CHIP_ERROR_INVALID_ARGUMENT != groups->GetGroupState(1, state));
    NL_TEST_ASSERT(apSuite, CHIP_ERROR_INVALID_ARGUMENT != groups->GetGroupState(3, state));

    // Multiple fabrics

    // Invalid index
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR != groups->SetGroupState(1, fabric2group1set1));

    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->SetGroupState(0, fabric1group1set3));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->SetGroupState(1, fabric2group1set1));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->SetGroupState(2, fabric1group1set1));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->SetGroupState(3, fabric2group1set2));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->SetGroupState(4, fabric2group1set3));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->SetGroupState(5, fabric1group1set2));
    // Incorrect fabric
    NL_TEST_ASSERT(apSuite, CHIP_ERROR_ACCESS_DENIED == groups->SetGroupState(1, fabric1group1set1));

    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->GetGroupState(2, state));
    NL_TEST_ASSERT(apSuite, state == fabric1group1set1);

    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->GetGroupState(5, state));
    NL_TEST_ASSERT(apSuite, state == fabric1group1set2);

    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->GetGroupState(0, state));
    NL_TEST_ASSERT(apSuite, state == fabric1group1set3);

    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->GetGroupState(4, state));
    NL_TEST_ASSERT(apSuite, state == fabric2group1set3);

    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->GetGroupState(1, state));
    NL_TEST_ASSERT(apSuite, state == fabric2group1set1);

    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->GetGroupState(3, state));
    NL_TEST_ASSERT(apSuite, state == fabric2group1set2);
}

void TestGroupStateIterator(nlTestSuite * apSuite, void * apContext)
{
    GroupDataProvider * groups = GetGroupDataProvider();
    NL_TEST_ASSERT(apSuite, groups);

    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->SetGroupState(0, fabric1group1set3));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->SetGroupState(1, fabric2group1set1));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->SetGroupState(2, fabric1group1set1));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->SetGroupState(3, fabric2group1set2));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->SetGroupState(4, fabric2group1set3));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->SetGroupState(5, fabric1group1set2));

    // Iterate All Fabrics

    constexpr size_t expected_count           = 6;
    const GroupState expected[expected_count] = { fabric1group1set3, fabric2group1set1, fabric1group1set1,
                                                  fabric2group1set2, fabric2group1set3, fabric1group1set2 };
    GroupState state(0, 0, 0);

    auto it_all = groups->IterateGroupStates();
    NL_TEST_ASSERT(apSuite, it_all);
    if (it_all)
    {
        size_t i = 0;
        NL_TEST_ASSERT(apSuite, expected_count == it_all->Count());

        while (it_all->Next(state) && i < expected_count)
        {
            NL_TEST_ASSERT(apSuite, state == expected[i]);
            i++;
        }
        NL_TEST_ASSERT(apSuite, i == expected_count);
        it_all->Release();
    }

    // Iterate Fabric 1 only

    constexpr size_t expected_count_f1              = 3;
    const GroupState expected_f1[expected_count_f1] = { fabric1group1set3, fabric1group1set1, fabric1group1set2 };

    auto it_f1 = groups->IterateGroupStates(kFabric1);
    NL_TEST_ASSERT(apSuite, it_f1);
    if (it_f1)
    {
        size_t i = 0;
        NL_TEST_ASSERT(apSuite, expected_count_f1 == it_f1->Count());

        while (it_f1->Next(state) && i < expected_count_f1)
        {
            NL_TEST_ASSERT(apSuite, state == expected_f1[i]);
            i++;
        }
        NL_TEST_ASSERT(apSuite, i == expected_count_f1);
        it_f1->Release();
    }

    // Iterate Fabric 2 only

    constexpr size_t expected_count_f2              = 3;
    const GroupState expected_f2[expected_count_f2] = { fabric2group1set1, fabric2group1set2, fabric2group1set3 };

    auto it_f2 = groups->IterateGroupStates(kFabric2);
    NL_TEST_ASSERT(apSuite, it_f2);
    if (it_f2)
    {
        size_t i = 0;
        NL_TEST_ASSERT(apSuite, expected_count_f2 == it_f2->Count());

        while (it_f2->Next(state) && i < expected_count_f2)
        {
            NL_TEST_ASSERT(apSuite, state == expected_f2[i]);
            i++;
        }
        NL_TEST_ASSERT(apSuite, i == expected_count_f2);
        it_f2->Release();
    }
}

static EpochKey epoch_keys0[3] = {
    { 0x1111111111111111, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
    { 0x2222222222222222, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
    { 0x3333333333333333, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } }
};
static EpochKey epoch_keys1[3] = {
    { 0xaaaaaaaaaaaaaaaa, { 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f } },
    { 0xbbbbbbbbbbbbbbbb, { 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f } },
    { 0xcccccccccccccccc, { 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f } },
};
static EpochKey epoch_keys2[2] = {
    { 0xeeeeeeeeeeeeeeee, { 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf } },
    { 0xffffffffffffffff, { 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf } },
};

constexpr uint16_t kKeysetId0 = 0x0;
constexpr uint16_t kKeysetId1 = 0x1111;
constexpr uint16_t kKeysetId2 = 0x2222;
constexpr uint16_t kKeysetId3 = 0x3333;

void TestKeysets(nlTestSuite * apSuite, void * apContext)
{
    GroupDataProvider * groups = GetGroupDataProvider();
    NL_TEST_ASSERT(apSuite, groups);

    // Reset test

    groups->RemoveKeyset(kFabric1, 0);
    groups->RemoveKeyset(kFabric1, 1);
    groups->RemoveKeyset(kFabric2, 0);
    groups->RemoveKeyset(kFabric2, 1);

    Keyset keyset0(Keyset::SecurityPolicy::kStandard, 3);
    Keyset keyset1(Keyset::SecurityPolicy::kStandard, 1);
    Keyset keyset2(Keyset::SecurityPolicy::kLowLatency, 2);
    Keyset keyset3(Keyset::SecurityPolicy::kStandard, 3);
    Keyset keyset;

    memcpy(keyset0.epoch_keys, epoch_keys0, sizeof(epoch_keys1));
    memcpy(keyset1.epoch_keys, epoch_keys1, sizeof(epoch_keys1));
    memcpy(keyset2.epoch_keys, epoch_keys2, sizeof(epoch_keys2));
    memcpy(keyset3.epoch_keys, epoch_keys1, sizeof(epoch_keys1));

    // Add Keysets

    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->SetKeyset(kFabric1, kKeysetId1, keyset1));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->SetKeyset(kFabric1, kKeysetId0, keyset0));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->SetKeyset(kFabric1, kKeysetId2, keyset2));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->SetKeyset(kFabric1, kKeysetId3, keyset3));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->SetKeyset(kFabric2, kKeysetId1, keyset3));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->SetKeyset(kFabric2, kKeysetId0, keyset0));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->SetKeyset(kFabric2, kKeysetId2, keyset1));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->SetKeyset(kFabric2, kKeysetId3, keyset2));

    // Get Keysets

    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->GetKeyset(kFabric1, kKeysetId3, keyset));
    NL_TEST_ASSERT(apSuite, keyset == keyset3);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->GetKeyset(kFabric1, kKeysetId1, keyset));
    NL_TEST_ASSERT(apSuite, keyset == keyset1);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->GetKeyset(kFabric1, kKeysetId0, keyset));
    NL_TEST_ASSERT(apSuite, keyset == keyset0);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->GetKeyset(kFabric1, kKeysetId2, keyset));
    NL_TEST_ASSERT(apSuite, keyset == keyset2);

    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->GetKeyset(kFabric2, kKeysetId3, keyset));
    NL_TEST_ASSERT(apSuite, keyset == keyset2);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->GetKeyset(kFabric2, kKeysetId2, keyset));
    NL_TEST_ASSERT(apSuite, keyset == keyset1);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->GetKeyset(kFabric2, kKeysetId1, keyset));
    NL_TEST_ASSERT(apSuite, keyset == keyset3);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->GetKeyset(kFabric2, kKeysetId0, keyset));
    NL_TEST_ASSERT(apSuite, keyset == keyset0);

    // Remove Keyseyts

    NL_TEST_ASSERT(apSuite, CHIP_ERROR_KEY_NOT_FOUND == groups->RemoveKeyset(kFabric1, 0xffff));

    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->RemoveKeyset(kFabric1, kKeysetId1)); // First
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->RemoveKeyset(kFabric1, kKeysetId3)); // Last
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->RemoveKeyset(kFabric2, kKeysetId2)); // Middle

    NL_TEST_ASSERT(apSuite, CHIP_ERROR_KEY_NOT_FOUND == groups->GetKeyset(kFabric1, kKeysetId3, keyset));
    NL_TEST_ASSERT(apSuite, CHIP_ERROR_KEY_NOT_FOUND == groups->GetKeyset(kFabric1, kKeysetId1, keyset));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->GetKeyset(kFabric1, kKeysetId0, keyset));
    NL_TEST_ASSERT(apSuite, keyset == keyset0);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->GetKeyset(kFabric1, kKeysetId2, keyset));
    NL_TEST_ASSERT(apSuite, keyset == keyset2);

    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->GetKeyset(kFabric2, kKeysetId3, keyset));
    NL_TEST_ASSERT(apSuite, keyset == keyset2);
    NL_TEST_ASSERT(apSuite, CHIP_ERROR_KEY_NOT_FOUND == groups->GetKeyset(kFabric2, kKeysetId2, keyset));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->GetKeyset(kFabric2, kKeysetId1, keyset));
    NL_TEST_ASSERT(apSuite, keyset == keyset3);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->GetKeyset(kFabric2, kKeysetId0, keyset));
    NL_TEST_ASSERT(apSuite, keyset == keyset0);

    // Remove all

    NL_TEST_ASSERT(apSuite, CHIP_ERROR_KEY_NOT_FOUND == groups->RemoveKeyset(kFabric1, kKeysetId3));
    NL_TEST_ASSERT(apSuite, CHIP_ERROR_KEY_NOT_FOUND == groups->RemoveKeyset(kFabric1, kKeysetId1));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->RemoveKeyset(kFabric1, kKeysetId0));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->RemoveKeyset(kFabric1, kKeysetId2));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->RemoveKeyset(kFabric2, kKeysetId3));
    NL_TEST_ASSERT(apSuite, CHIP_ERROR_KEY_NOT_FOUND == groups->RemoveKeyset(kFabric2, kKeysetId2));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->RemoveKeyset(kFabric2, kKeysetId1));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->RemoveKeyset(kFabric2, kKeysetId0));

    NL_TEST_ASSERT(apSuite, CHIP_ERROR_KEY_NOT_FOUND == groups->GetKeyset(kFabric1, kKeysetId3, keyset));
    NL_TEST_ASSERT(apSuite, CHIP_ERROR_KEY_NOT_FOUND == groups->GetKeyset(kFabric1, kKeysetId1, keyset));
    NL_TEST_ASSERT(apSuite, CHIP_ERROR_KEY_NOT_FOUND == groups->GetKeyset(kFabric1, kKeysetId0, keyset));
    NL_TEST_ASSERT(apSuite, CHIP_ERROR_KEY_NOT_FOUND == groups->GetKeyset(kFabric1, kKeysetId2, keyset));
    NL_TEST_ASSERT(apSuite, CHIP_ERROR_KEY_NOT_FOUND == groups->GetKeyset(kFabric2, kKeysetId3, keyset));
    NL_TEST_ASSERT(apSuite, CHIP_ERROR_KEY_NOT_FOUND == groups->GetKeyset(kFabric2, kKeysetId2, keyset));
    NL_TEST_ASSERT(apSuite, CHIP_ERROR_KEY_NOT_FOUND == groups->GetKeyset(kFabric2, kKeysetId1, keyset));
    NL_TEST_ASSERT(apSuite, CHIP_ERROR_KEY_NOT_FOUND == groups->GetKeyset(kFabric2, kKeysetId0, keyset));
}

void TestKeysetIterator(nlTestSuite * apSuite, void * apContext)
{
    GroupDataProvider * groups = GetGroupDataProvider();
    NL_TEST_ASSERT(apSuite, groups);

    // Reset test

    groups->RemoveKeyset(kFabric1, kKeysetId0);
    groups->RemoveKeyset(kFabric1, kKeysetId1);
    groups->RemoveKeyset(kFabric1, kKeysetId2);
    groups->RemoveKeyset(kFabric1, kKeysetId3);
    groups->RemoveKeyset(kFabric2, kKeysetId0);
    groups->RemoveKeyset(kFabric2, kKeysetId1);
    groups->RemoveKeyset(kFabric2, kKeysetId2);
    groups->RemoveKeyset(kFabric2, kKeysetId3);

    Keyset keyset0(Keyset::SecurityPolicy::kStandard, 3);
    Keyset keyset1(Keyset::SecurityPolicy::kStandard, 1);
    Keyset keyset2(Keyset::SecurityPolicy::kLowLatency, 2);
    Keyset keyset3(Keyset::SecurityPolicy::kStandard, 3);
    Keyset keyset;

    memcpy(keyset0.epoch_keys, epoch_keys0, sizeof(epoch_keys1));
    memcpy(keyset1.epoch_keys, epoch_keys1, sizeof(epoch_keys1));
    memcpy(keyset2.epoch_keys, epoch_keys2, sizeof(epoch_keys2));
    memcpy(keyset3.epoch_keys, epoch_keys1, sizeof(epoch_keys1));

    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->SetKeyset(kFabric1, kKeysetId1, keyset1));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->SetKeyset(kFabric1, kKeysetId0, keyset0));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->SetKeyset(kFabric1, kKeysetId2, keyset2));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->SetKeyset(kFabric1, kKeysetId3, keyset3));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->SetKeyset(kFabric2, kKeysetId1, keyset3));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->SetKeyset(kFabric2, kKeysetId2, keyset1));
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == groups->SetKeyset(kFabric2, kKeysetId3, keyset2));

    // Iterate Fabric 1

    std::map<uint16_t, const Keyset> expected_f1{
        { kKeysetId1, keyset1 }, { kKeysetId0, keyset0 }, { kKeysetId2, keyset2 }, { kKeysetId3, keyset3 }
    };

    auto it_f1 = groups->IterateKeysets(kFabric1);
    NL_TEST_ASSERT(apSuite, it_f1);
    if (it_f1)
    {
        size_t i = 0;
        NL_TEST_ASSERT(apSuite, expected_f1.size() == it_f1->Count());

        while (it_f1->Next(keyset) && i < expected_f1.size())
        {
            NL_TEST_ASSERT(apSuite, expected_f1.count(keyset.keyset_id) > 0);
            NL_TEST_ASSERT(apSuite, keyset == expected_f1[keyset.keyset_id]);
            i++;
        }
        NL_TEST_ASSERT(apSuite, i == expected_f1.size());
        it_f1->Release();
    }

    // Iterate Fabric 2

    std::map<uint16_t, const Keyset> expected_f2{ { kKeysetId3, keyset2 }, { kKeysetId1, keyset3 }, { kKeysetId2, keyset1 } };

    auto it_f2 = groups->IterateKeysets(kFabric2);
    NL_TEST_ASSERT(apSuite, it_f2);
    if (it_f2)
    {
        size_t i = 0;
        NL_TEST_ASSERT(apSuite, expected_f2.size() == it_f2->Count());

        while (it_f2->Next(keyset) && i < expected_f2.size())
        {
            NL_TEST_ASSERT(apSuite, expected_f2.count(keyset.keyset_id) > 0);
            NL_TEST_ASSERT(apSuite, keyset == expected_f2[keyset.keyset_id]);
            i++;
        }
        NL_TEST_ASSERT(apSuite, i == expected_f2.size());
        it_f2->Release();
    }
}

void TestEndpoints(nlTestSuite * apSuite, void * apContext)
{
    GroupDataProvider * groups = GetGroupDataProvider();
    NL_TEST_ASSERT(apSuite, groups);

    // Reset test

    groups->RemoveAllGroupMappings(kFabric1, kEndpoint1);
    groups->RemoveAllGroupMappings(kFabric1, kEndpoint2);
    groups->RemoveAllGroupMappings(kFabric1, kEndpoint3);
    groups->RemoveAllGroupMappings(kFabric2, kEndpoint1);
    groups->RemoveAllGroupMappings(kFabric2, kEndpoint2);
    groups->RemoveAllGroupMappings(kFabric2, kEndpoint3);

    CHIP_ERROR err = CHIP_NO_ERROR;
    bool exists    = false;

    NL_TEST_ASSERT(apSuite, groups);

    exists = groups->GroupMappingExists(0xff, endpoint1group1);
    NL_TEST_ASSERT(apSuite, !exists);

    exists = groups->GroupMappingExists(kFabric1, endpoint1group1);
    NL_TEST_ASSERT(apSuite, !exists);

    err = groups->AddGroupMapping(kFabric1, endpoint1group1);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->AddGroupMapping(kFabric1, endpoint1group3);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    exists = groups->GroupMappingExists(kFabric1, endpoint1group1);
    NL_TEST_ASSERT(apSuite, exists);

    exists = groups->GroupMappingExists(kFabric1, endpoint1group2);
    NL_TEST_ASSERT(apSuite, !exists);

    exists = groups->GroupMappingExists(kFabric1, endpoint1group3);
    NL_TEST_ASSERT(apSuite, exists);

    err = groups->RemoveGroupMapping(kFabric1, endpoint1group1);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    exists = groups->GroupMappingExists(kFabric1, endpoint1group1);
    NL_TEST_ASSERT(apSuite, !exists);

    exists = groups->GroupMappingExists(kFabric1, endpoint1group2);
    NL_TEST_ASSERT(apSuite, !exists);

    exists = groups->GroupMappingExists(kFabric1, endpoint1group3);
    NL_TEST_ASSERT(apSuite, exists);

    err = groups->AddGroupMapping(kFabric1, endpoint1group1);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->AddGroupMapping(kFabric1, endpoint1group2);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    exists = groups->GroupMappingExists(kFabric1, endpoint1group1);
    NL_TEST_ASSERT(apSuite, exists);

    exists = groups->GroupMappingExists(kFabric1, endpoint1group2);
    NL_TEST_ASSERT(apSuite, exists);

    exists = groups->GroupMappingExists(kFabric1, endpoint1group3);
    NL_TEST_ASSERT(apSuite, exists);

    err = groups->RemoveAllGroupMappings(kFabric1, 1);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    exists = groups->GroupMappingExists(kFabric1, endpoint1group1);
    NL_TEST_ASSERT(apSuite, !exists);

    exists = groups->GroupMappingExists(kFabric1, endpoint1group2);
    NL_TEST_ASSERT(apSuite, !exists);

    exists = groups->GroupMappingExists(kFabric1, endpoint1group3);
    NL_TEST_ASSERT(apSuite, !exists);

    // Test multiple fabrics

    NL_TEST_ASSERT(apSuite, groups);

    exists = groups->GroupMappingExists(0xff, endpoint1group1);
    NL_TEST_ASSERT(apSuite, !exists);

    exists = groups->GroupMappingExists(kFabric2, endpoint1group1);
    NL_TEST_ASSERT(apSuite, !exists);

    err = groups->AddGroupMapping(kFabric1, endpoint1group1);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->AddGroupMapping(kFabric1, endpoint1group3);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->AddGroupMapping(kFabric2, endpoint1group1);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->AddGroupMapping(kFabric2, endpoint1group3);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    exists = groups->GroupMappingExists(kFabric2, endpoint1group1);
    NL_TEST_ASSERT(apSuite, exists);

    exists = groups->GroupMappingExists(kFabric2, endpoint1group2);
    NL_TEST_ASSERT(apSuite, !exists);

    exists = groups->GroupMappingExists(kFabric2, endpoint1group3);
    NL_TEST_ASSERT(apSuite, exists);

    err = groups->RemoveGroupMapping(kFabric2, endpoint1group1);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    exists = groups->GroupMappingExists(kFabric2, endpoint1group1);
    NL_TEST_ASSERT(apSuite, !exists);

    exists = groups->GroupMappingExists(kFabric2, endpoint1group2);
    NL_TEST_ASSERT(apSuite, !exists);

    exists = groups->GroupMappingExists(kFabric2, endpoint1group3);
    NL_TEST_ASSERT(apSuite, exists);

    err = groups->AddGroupMapping(kFabric2, endpoint1group1);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->AddGroupMapping(kFabric2, endpoint1group2);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    exists = groups->GroupMappingExists(kFabric2, endpoint1group1);
    NL_TEST_ASSERT(apSuite, exists);

    exists = groups->GroupMappingExists(kFabric2, endpoint1group2);
    NL_TEST_ASSERT(apSuite, exists);

    exists = groups->GroupMappingExists(kFabric2, endpoint1group3);
    NL_TEST_ASSERT(apSuite, exists);

    err = groups->RemoveAllGroupMappings(kFabric2, 1);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    exists = groups->GroupMappingExists(kFabric2, endpoint1group1);
    NL_TEST_ASSERT(apSuite, !exists);

    exists = groups->GroupMappingExists(kFabric2, endpoint1group2);
    NL_TEST_ASSERT(apSuite, !exists);

    exists = groups->GroupMappingExists(kFabric2, endpoint1group3);
    NL_TEST_ASSERT(apSuite, !exists);
}

void TestEndpointIterator(nlTestSuite * apSuite, void * apContext)
{
    GroupDataProvider * groups = GetGroupDataProvider();
    NL_TEST_ASSERT(apSuite, groups);

    CHIP_ERROR err = CHIP_NO_ERROR;

    // Reset test

    groups->RemoveAllGroupMappings(kFabric1, kEndpoint1);
    groups->RemoveAllGroupMappings(kFabric1, kEndpoint2);
    groups->RemoveAllGroupMappings(kFabric1, kEndpoint3);

    NL_TEST_ASSERT(apSuite, groups);

    err = groups->AddGroupMapping(kFabric1, endpoint3group2);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->AddGroupMapping(kFabric1, endpoint2group2);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->AddGroupMapping(kFabric1, endpoint1group2);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->AddGroupMapping(kFabric1, endpoint3group1);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->AddGroupMapping(kFabric1, endpoint1group1);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->AddGroupMapping(kFabric1, endpoint3group3);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    // Endpoint 1

    auto * it = groups->IterateGroupMappings(kFabric1, kEndpoint1);
    NL_TEST_ASSERT(apSuite, it);

    GroupDataProvider::GroupMapping mapping;
    size_t count1 = it->Count();
    size_t count2 = 0;
    NL_TEST_ASSERT(apSuite, 2 == count1);
    while (it->Next(mapping))
    {
        count2++;
        NL_TEST_ASSERT(apSuite, kGroup1 == mapping.group || kGroup2 == mapping.group);
    }
    NL_TEST_ASSERT(apSuite, count2 == count1);
    it->Release();
    it = nullptr;

    // Endpoint 3

    it = groups->IterateGroupMappings(kFabric1, kEndpoint3);
    NL_TEST_ASSERT(apSuite, it);

    count1 = it->Count();
    count2 = 0;
    NL_TEST_ASSERT(apSuite, 3 == count1);
    while (it->Next(mapping))
    {
        count2++;
        NL_TEST_ASSERT(apSuite, kGroup1 == mapping.group || kGroup2 == mapping.group || kGroup3 == mapping.group);
    }
    NL_TEST_ASSERT(apSuite, count2 == count1);
    it->Release();
    it = nullptr;
}

void TestStates(nlTestSuite * apSuite, void * apContext)
{
    GroupDataProvider * groups = GetGroupDataProvider();
    NL_TEST_ASSERT(apSuite, groups);

    // Reset test

    groups->RemoveGroupState(5);
    groups->RemoveGroupState(4);
    groups->RemoveGroupState(3);
    groups->RemoveGroupState(2);
    groups->RemoveGroupState(1);
    groups->RemoveGroupState(0);

    GroupDataProvider::GroupState state0a(1, 1, 1);
    GroupDataProvider::GroupState state0b(0, 10, 11);
    GroupDataProvider::GroupState state1a(1, 1, 2);
    GroupDataProvider::GroupState state1b(0, 10, 12);
    GroupDataProvider::GroupState state3b(0, 10, 13);
    GroupDataProvider::GroupState state4a(1, 5, 3);
    GroupDataProvider::GroupState state4b(0, 10, 14);

    CHIP_ERROR err = CHIP_NO_ERROR;

    // First append
    err = groups->SetGroupState(0, state0a);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    // Second append
    err = groups->SetGroupState(1, state1a);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    // Attempt to set past the append slot
    err = groups->SetGroupState(3, state0a);
    NL_TEST_ASSERT(apSuite, CHIP_ERROR_INVALID_ARGUMENT == err);

    auto * it = groups->IterateGroupStates(kFabric1);
    NL_TEST_ASSERT(apSuite, it != nullptr);
    NL_TEST_ASSERT(apSuite, 2 == it->Count());
    it->Release();
    it = nullptr;

    err = groups->GetGroupState(0, state0b);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(apSuite, state0a.group == state0b.group);
    NL_TEST_ASSERT(apSuite, state0a.keyset_index == state0b.keyset_index);
    NL_TEST_ASSERT(apSuite, kFabric1 == state0b.fabric_index);

    err = groups->GetGroupState(1, state1b);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(apSuite, state1a.group == state1b.group);
    NL_TEST_ASSERT(apSuite, state1a.keyset_index == state1b.keyset_index);
    NL_TEST_ASSERT(apSuite, kFabric1 == state1b.fabric_index);

    err = groups->GetGroupState(2, state3b);
    NL_TEST_ASSERT(apSuite, CHIP_ERROR_KEY_NOT_FOUND == err);

    err = groups->RemoveGroupState(0);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    // Entry 1 should remain, now at slot 0
    state1b.group        = 10;
    state1b.keyset_index = 12;
    state1b.fabric_index = 14;
    err                  = groups->GetGroupState(0, state1b);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(apSuite, state1a.group == state1b.group);
    NL_TEST_ASSERT(apSuite, state1a.keyset_index == state1b.keyset_index);
    NL_TEST_ASSERT(apSuite, kFabric1 == state1b.fabric_index);

    state1b.group        = 10;
    state1b.keyset_index = 12;
    state1b.fabric_index = 14;
    err                  = groups->GetGroupState(1, state1b);
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
    NL_TEST_ASSERT(apSuite, state4a.keyset_index == state4b.keyset_index);
    NL_TEST_ASSERT(apSuite, state4a.fabric_index == state4b.fabric_index);

    // Incorrect fabric

    state4a.fabric_index = 3;
    err                  = groups->SetGroupState(0, state4a);
    NL_TEST_ASSERT(apSuite, CHIP_ERROR_ACCESS_DENIED == err);

    err = groups->RemoveGroupState(0);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);
}

void TestStateIterator(nlTestSuite * apSuite, void * apContext)
{
    GroupDataProvider * groups = GetGroupDataProvider();
    NL_TEST_ASSERT(apSuite, groups);

    GroupDataProvider::GroupState state0(kFabric1, 1, 1);
    GroupDataProvider::GroupState state1(kFabric1, 2, 1);
    GroupDataProvider::GroupState state2(kFabric2, 2, 2);
    GroupDataProvider::GroupState state3(kFabric1, 3, 1);
    CHIP_ERROR err = CHIP_NO_ERROR;

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
        auto * it = groups->IterateGroupStates(kFabric1);
        NL_TEST_ASSERT(apSuite, it != nullptr);

        size_t count1 = it->Count();
        size_t count2 = 0;
        NL_TEST_ASSERT(apSuite, 3 == count1);
        GroupDataProvider::GroupState state;
        while (it->Next(state))
        {
            NL_TEST_ASSERT(apSuite, (state.group > 0 && state.group < 4) && (state.keyset_index == 1));
            NL_TEST_ASSERT(apSuite, (state.fabric_index == kFabric1));
            count2++;
        }
        NL_TEST_ASSERT(apSuite, count2 == count1);
        it->Release();
        it = nullptr;
    }

    {
        // Fabric Index 2 has 1 entry
        auto * it = groups->IterateGroupStates(kFabric2);
        NL_TEST_ASSERT(apSuite, it != nullptr);

        size_t count1 = it->Count();
        NL_TEST_ASSERT(apSuite, 1 == count1);
        GroupDataProvider::GroupState state;
        NL_TEST_ASSERT(apSuite, it->Next(state));

        NL_TEST_ASSERT(apSuite, (state.group > 0 && state.group < 4) && (state.keyset_index == 2));
        NL_TEST_ASSERT(apSuite, (state.fabric_index == kFabric2));

        NL_TEST_ASSERT(apSuite, !it->Next(state));

        it->Release();
        it = nullptr;
    }

    {
        // Fabric Index 1 has 3 entries + Fabric Index 2 has 1 entry
        auto * it = groups->IterateGroupStates();
        NL_TEST_ASSERT(apSuite, it != nullptr);

        size_t count1 = it->Count();
        size_t count2 = 0;
        NL_TEST_ASSERT(apSuite, 4 == count1);
        GroupDataProvider::GroupState state;
        while (it->Next(state))
        {
            NL_TEST_ASSERT(apSuite, (state.fabric_index == kFabric1 || state.fabric_index == kFabric2));
            NL_TEST_ASSERT(apSuite, (state.group > 0 && state.group < 4) && (state.keyset_index == 1 || state.keyset_index == 2));
            count2++;
        }
        NL_TEST_ASSERT(apSuite, count2 == count1);
        it->Release();
        it = nullptr;
    }
}

void TestKeys(nlTestSuite * apSuite, void * apContext)
{
    GroupDataProvider * groups = GetGroupDataProvider();
    NL_TEST_ASSERT(apSuite, groups);

    // Reset test
    groups->RemoveKeyset(kFabric1, kKeysetId0);
    groups->RemoveKeyset(kFabric1, kKeysetId1);
    groups->RemoveKeyset(kFabric1, kKeysetId2);
    groups->RemoveKeyset(kFabric1, kKeysetId3);

    // Pairs keys0[a|b], keys1[a|b] have different values. [b] is used as Get target, so it
    // should get overwritten with the values from [a].
    Keyset keys0a(0, Keyset::SecurityPolicy::kStandard, 3);
    Keyset keys0b(0, Keyset::SecurityPolicy::kStandard, 2);
    Keyset keys1a(0, Keyset::SecurityPolicy::kStandard, 3);
    Keyset keys1b(0, Keyset::SecurityPolicy::kStandard, 2);
    Keyset keys3(0, Keyset::SecurityPolicy::kStandard, 2);
    CHIP_ERROR err = CHIP_NO_ERROR;

    NL_TEST_ASSERT(apSuite, groups);

    memcpy(keys0a.epoch_keys, epoch_keys1, sizeof(epoch_keys1));
    memcpy(keys0b.epoch_keys, epoch_keys0, sizeof(epoch_keys0));
    memcpy(keys1a.epoch_keys, epoch_keys1, sizeof(epoch_keys1));
    memcpy(keys1b.epoch_keys, epoch_keys0, sizeof(epoch_keys0));
    memcpy(keys3.epoch_keys, epoch_keys2, sizeof(epoch_keys2));

    err = groups->SetKeyset(kFabric1, kKeysetId0, keys0a);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->SetKeyset(kFabric1, kKeysetId1, keys1a);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    auto * it = groups->IterateKeysets(kFabric1);
    NL_TEST_ASSERT(apSuite, it != nullptr);
    NL_TEST_ASSERT(apSuite, it->Count() == 2);

    err = groups->GetKeyset(kFabric1, kKeysetId0, keys0b);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(apSuite, keys0a.policy == keys0b.policy);
    NL_TEST_ASSERT(apSuite, keys0a.num_keys_used == keys0b.num_keys_used);
    NL_TEST_ASSERT(apSuite, 0 == memcmp(keys0a.epoch_keys, keys0b.epoch_keys, sizeof(keys0a.epoch_keys[0]) * keys0a.num_keys_used));

    err = groups->GetKeyset(kFabric1, kKeysetId1, keys1b);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(apSuite, keys1a.policy == keys1b.policy);
    NL_TEST_ASSERT(apSuite, keys1a.num_keys_used == keys1b.num_keys_used);
    NL_TEST_ASSERT(apSuite, 0 == memcmp(keys1a.epoch_keys, keys1b.epoch_keys, sizeof(keys1a.epoch_keys[0]) * keys1a.num_keys_used));

    err = groups->GetKeyset(kFabric1, kKeysetId3, keys3);
    NL_TEST_ASSERT(apSuite, CHIP_ERROR_KEY_NOT_FOUND == err);

    err = groups->RemoveKeyset(kFabric1, kKeysetId0);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->GetKeyset(kFabric1, kKeysetId1, keys1b);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->GetKeyset(kFabric1, kKeysetId0, keys0b);
    NL_TEST_ASSERT(apSuite, CHIP_ERROR_KEY_NOT_FOUND == err);
}

void TestKeysIterator(nlTestSuite * apSuite, void * apContext)
{
    GroupDataProvider * groups = GetGroupDataProvider();
    NL_TEST_ASSERT(apSuite, groups);

    // Reset test
    groups->RemoveKeyset(kFabric1, kKeysetId0);
    groups->RemoveKeyset(kFabric1, kKeysetId1);
    groups->RemoveKeyset(kFabric1, kKeysetId2);
    groups->RemoveKeyset(kFabric1, kKeysetId3);

    Keyset keys0(0, Keyset::SecurityPolicy::kStandard, 3);
    Keyset keys1(0, Keyset::SecurityPolicy::kStandard, 2);
    Keyset keys2(0, Keyset::SecurityPolicy::kStandard, 3);
    CHIP_ERROR err = CHIP_NO_ERROR;

    NL_TEST_ASSERT(apSuite, groups);

    memcpy(keys0.epoch_keys, epoch_keys1, sizeof(epoch_keys1));
    memcpy(keys1.epoch_keys, epoch_keys2, sizeof(epoch_keys2));
    memcpy(keys2.epoch_keys, epoch_keys1, sizeof(epoch_keys1));

    NL_TEST_ASSERT(apSuite, groups);

    err = groups->SetKeyset(kFabric1, kKeysetId2, keys2);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->SetKeyset(kFabric1, kKeysetId0, keys0);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->SetKeyset(kFabric1, kKeysetId1, keys1);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    auto * it = groups->IterateKeysets(kFabric1);
    NL_TEST_ASSERT(apSuite, it);

    size_t count1 = it->Count();
    size_t count2 = 0;
    NL_TEST_ASSERT(apSuite, 3 == count1);
    GroupDataProvider::Keyset keys;

    uint16_t last_keyset_id = UINT16_MAX;

    while (it->Next(keys))
    {
        NL_TEST_ASSERT(apSuite, keys.keyset_id == kKeysetId0 || keys.keyset_id == kKeysetId1 || keys.keyset_id == kKeysetId2);
        NL_TEST_ASSERT(apSuite, keys.keyset_id != last_keyset_id);
        last_keyset_id = keys.keyset_id;
        count2++;
    }
    NL_TEST_ASSERT(apSuite, count2 == count1);
    it->Release();
    it = nullptr;
}

void TestPerFabricData(nlTestSuite * apSuite, void * apContext)
{
    GroupDataProvider * groups = GetGroupDataProvider();
    NL_TEST_ASSERT(apSuite, groups);

    // Reset test

    groups->RemoveAllGroupMappings(kFabric1, kEndpoint1);
    groups->RemoveAllGroupMappings(kFabric1, kEndpoint2);
    groups->RemoveAllGroupMappings(kFabric1, kEndpoint3);
    groups->RemoveAllGroupMappings(kFabric2, kEndpoint1);
    groups->RemoveAllGroupMappings(kFabric2, kEndpoint2);
    groups->RemoveAllGroupMappings(kFabric2, kEndpoint3);

    groups->RemoveGroupState(5);
    groups->RemoveGroupState(4);
    groups->RemoveGroupState(3);
    groups->RemoveGroupState(2);
    groups->RemoveGroupState(1);
    groups->RemoveGroupState(0);

    groups->RemoveKeyset(kFabric1, kKeysetId0);
    groups->RemoveKeyset(kFabric1, kKeysetId1);
    groups->RemoveKeyset(kFabric1, kKeysetId2);
    groups->RemoveKeyset(kFabric1, kKeysetId3);
    groups->RemoveKeyset(kFabric2, kKeysetId0);
    groups->RemoveKeyset(kFabric2, kKeysetId1);
    groups->RemoveKeyset(kFabric2, kKeysetId2);
    groups->RemoveKeyset(kFabric2, kKeysetId3);

    CHIP_ERROR err = CHIP_NO_ERROR;
    bool exists    = false;

    // Mappings

    err = groups->AddGroupMapping(kFabric1, endpoint1group1);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->AddGroupMapping(kFabric1, endpoint1group2);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->AddGroupMapping(kFabric1, endpoint1group3);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->AddGroupMapping(kFabric2, endpoint1group1);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->AddGroupMapping(kFabric2, endpoint1group3);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    exists = groups->GroupMappingExists(kFabric1, endpoint1group1);
    NL_TEST_ASSERT(apSuite, exists);

    exists = groups->GroupMappingExists(kFabric1, endpoint1group2);
    NL_TEST_ASSERT(apSuite, exists);

    exists = groups->GroupMappingExists(kFabric1, endpoint1group3);
    NL_TEST_ASSERT(apSuite, exists);

    exists = groups->GroupMappingExists(kFabric2, endpoint1group1);
    NL_TEST_ASSERT(apSuite, exists);

    exists = groups->GroupMappingExists(kFabric2, endpoint1group2);
    NL_TEST_ASSERT(apSuite, !exists);

    exists = groups->GroupMappingExists(kFabric2, endpoint1group3);
    NL_TEST_ASSERT(apSuite, exists);

    // States

    const GroupDataProvider::GroupState state0a(kFabric1, kGroup1, 101);
    GroupDataProvider::GroupState state0b(0, 0, 0);

    const GroupDataProvider::GroupState state1a(kFabric2, kGroup1, 102);
    GroupDataProvider::GroupState state1b(0, 0, 0);

    const GroupDataProvider::GroupState state2a(kFabric2, kGroup2, 101);
    GroupDataProvider::GroupState state2b(0, 0, 0);

    const GroupDataProvider::GroupState state3a(kFabric1, kGroup2, 102);
    GroupDataProvider::GroupState state4b(0, 0, 0);

    err = groups->SetGroupState(0, state0a);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->SetGroupState(1, state1a);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->SetGroupState(2, state2a);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->SetGroupState(3, state3a);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->GetGroupState(0, state0b);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(apSuite, state0a.fabric_index == state0b.fabric_index);
    NL_TEST_ASSERT(apSuite, state0a.group == state0b.group);
    NL_TEST_ASSERT(apSuite, state0a.keyset_index == state0b.keyset_index);

    err = groups->GetGroupState(1, state1b);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(apSuite, state1a.fabric_index == state1b.fabric_index);
    NL_TEST_ASSERT(apSuite, state1a.group == state1b.group);
    NL_TEST_ASSERT(apSuite, state1a.keyset_index == state1b.keyset_index);

    err = groups->GetGroupState(2, state2b);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(apSuite, state2a.fabric_index == state2b.fabric_index);
    NL_TEST_ASSERT(apSuite, state2a.group == state2b.group);
    NL_TEST_ASSERT(apSuite, state2a.keyset_index == state2b.keyset_index);

    err = groups->GetGroupState(4, state4b);
    NL_TEST_ASSERT(apSuite, CHIP_ERROR_KEY_NOT_FOUND == err);

    // Keys

    Keyset keys0a(0, Keyset::SecurityPolicy::kStandard, 3);
    Keyset keys1a(0, Keyset::SecurityPolicy::kLowLatency, 3);
    Keyset keys_out(0, Keyset::SecurityPolicy::kStandard, 0);

    NL_TEST_ASSERT(apSuite, groups);

    memcpy(keys0a.epoch_keys, epoch_keys1, sizeof(epoch_keys1));
    memcpy(keys1a.epoch_keys, epoch_keys1, sizeof(epoch_keys1));
    memcpy(keys_out.epoch_keys, epoch_keys0, sizeof(epoch_keys0));

    err = groups->SetKeyset(kFabric2, kKeysetId0, keys0a);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->SetKeyset(kFabric1, kKeysetId0, keys0a);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->SetKeyset(kFabric2, kKeysetId1, keys1a);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->SetKeyset(kFabric1, kKeysetId1, keys1a);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->SetKeyset(kFabric2, kKeysetId2, keys0a);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->SetKeyset(kFabric1, kKeysetId2, keys0a);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    err = groups->GetKeyset(kFabric2, kKeysetId0, keys_out);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(apSuite, keys0a.policy == keys_out.policy);
    NL_TEST_ASSERT(apSuite, keys0a.num_keys_used == keys_out.num_keys_used);
    NL_TEST_ASSERT(apSuite,
                   0 == memcmp(keys0a.epoch_keys, keys_out.epoch_keys, sizeof(keys0a.epoch_keys[0]) * keys0a.num_keys_used));

    err = groups->GetKeyset(kFabric2, kKeysetId1, keys_out);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(apSuite, keys1a.policy == keys_out.policy);
    NL_TEST_ASSERT(apSuite, keys1a.num_keys_used == keys_out.num_keys_used);
    NL_TEST_ASSERT(apSuite,
                   0 == memcmp(keys1a.epoch_keys, keys_out.epoch_keys, sizeof(keys1a.epoch_keys[0]) * keys1a.num_keys_used));

    err = groups->GetKeyset(kFabric2, kKeysetId2, keys_out);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(apSuite, keys0a.policy == keys_out.policy);
    NL_TEST_ASSERT(apSuite, keys0a.num_keys_used == keys_out.num_keys_used);
    NL_TEST_ASSERT(apSuite,
                   0 == memcmp(keys0a.epoch_keys, keys_out.epoch_keys, sizeof(keys0a.epoch_keys[0]) * keys0a.num_keys_used));

    err = groups->GetKeyset(kFabric1, kKeysetId2, keys_out);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(apSuite, keys0a.policy == keys_out.policy);
    NL_TEST_ASSERT(apSuite, keys0a.num_keys_used == keys_out.num_keys_used);
    NL_TEST_ASSERT(apSuite,
                   0 == memcmp(keys0a.epoch_keys, keys_out.epoch_keys, sizeof(keys0a.epoch_keys[0]) * keys0a.num_keys_used));

    err = groups->GetKeyset(kFabric1, kKeysetId1, keys_out);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(apSuite, keys1a.policy == keys_out.policy);
    NL_TEST_ASSERT(apSuite, keys1a.num_keys_used == keys_out.num_keys_used);
    NL_TEST_ASSERT(apSuite,
                   0 == memcmp(keys1a.epoch_keys, keys_out.epoch_keys, sizeof(keys1a.epoch_keys[0]) * keys1a.num_keys_used));

    err = groups->GetKeyset(kFabric1, kKeysetId0, keys_out);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(apSuite, keys0a.policy == keys_out.policy);
    NL_TEST_ASSERT(apSuite, keys0a.num_keys_used == keys_out.num_keys_used);
    NL_TEST_ASSERT(apSuite,
                   0 == memcmp(keys0a.epoch_keys, keys_out.epoch_keys, sizeof(keys0a.epoch_keys[0]) * keys0a.num_keys_used));

    //
    // Remove Fabric
    //

    err = groups->RemoveFabric(kFabric1);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);

    // Mappings

    exists = groups->GroupMappingExists(kFabric1, endpoint1group1);
    NL_TEST_ASSERT(apSuite, !exists);

    exists = groups->GroupMappingExists(kFabric1, endpoint1group2);
    NL_TEST_ASSERT(apSuite, !exists);

    exists = groups->GroupMappingExists(kFabric1, endpoint1group3);
    NL_TEST_ASSERT(apSuite, !exists);

    exists = groups->GroupMappingExists(kFabric2, endpoint1group1);
    NL_TEST_ASSERT(apSuite, exists);

    exists = groups->GroupMappingExists(kFabric2, endpoint1group2);
    NL_TEST_ASSERT(apSuite, !exists);

    exists = groups->GroupMappingExists(kFabric2, endpoint1group3);
    NL_TEST_ASSERT(apSuite, exists);

    // States: Removing the fabric shift the remaining groups states to a lower index

    err = groups->GetGroupState(0, state0b);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(apSuite, state1a.fabric_index == state0b.fabric_index);
    NL_TEST_ASSERT(apSuite, state1a.group == state0b.group);
    NL_TEST_ASSERT(apSuite, state1a.keyset_index == state0b.keyset_index);

    err = groups->GetGroupState(1, state1b);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(apSuite, state2a.fabric_index == state1b.fabric_index);
    NL_TEST_ASSERT(apSuite, state2a.group == state1b.group);
    NL_TEST_ASSERT(apSuite, state2a.keyset_index == state1b.keyset_index);

    err = groups->GetGroupState(2, state2b);
    NL_TEST_ASSERT(apSuite, CHIP_ERROR_KEY_NOT_FOUND == err);

    // Keys

    err = groups->GetKeyset(kFabric2, kKeysetId0, keys_out);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(apSuite, keys0a.policy == keys_out.policy);
    NL_TEST_ASSERT(apSuite, keys0a.num_keys_used == keys_out.num_keys_used);
    NL_TEST_ASSERT(apSuite,
                   0 == memcmp(keys0a.epoch_keys, keys_out.epoch_keys, sizeof(keys0a.epoch_keys[0]) * keys0a.num_keys_used));

    err = groups->GetKeyset(kFabric2, kKeysetId1, keys_out);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(apSuite, keys1a.policy == keys_out.policy);
    NL_TEST_ASSERT(apSuite, keys1a.num_keys_used == keys_out.num_keys_used);
    NL_TEST_ASSERT(apSuite,
                   0 == memcmp(keys1a.epoch_keys, keys_out.epoch_keys, sizeof(keys1a.epoch_keys[0]) * keys1a.num_keys_used));

    err = groups->GetKeyset(kFabric2, kKeysetId0, keys_out);
    NL_TEST_ASSERT(apSuite, CHIP_NO_ERROR == err);
    NL_TEST_ASSERT(apSuite, keys0a.policy == keys_out.policy);
    NL_TEST_ASSERT(apSuite, keys0a.num_keys_used == keys_out.num_keys_used);
    NL_TEST_ASSERT(apSuite,
                   0 == memcmp(keys0a.epoch_keys, keys_out.epoch_keys, sizeof(keys0a.epoch_keys[0]) * keys0a.num_keys_used));

    err = groups->GetKeyset(kFabric1, 202, keys_out);
    NL_TEST_ASSERT(apSuite, CHIP_ERROR_INVALID_FABRIC_ID == err);

    err = groups->GetKeyset(kFabric1, 404, keys_out);
    NL_TEST_ASSERT(apSuite, CHIP_ERROR_INVALID_FABRIC_ID == err);

    err = groups->GetKeyset(kFabric1, 606, keys_out);
    NL_TEST_ASSERT(apSuite, CHIP_ERROR_INVALID_FABRIC_ID == err);
}

} // namespace TestGroups
} // namespace app
} // namespace chip

namespace {

static chip::TestPersistentStorageDelegate sDelegate;
static GroupDataProviderImpl sProvider(sDelegate);

/**
 *  Set up the test suite.
 */
int Test_Setup(void * inContext)
{
    // #if CHIP_DEVICE_LAYER_TARGET_DARWIN
    //     VerifyOrReturnError(CHIP_NO_ERROR == chip::DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().Init("chip.store"),
    //     FAILURE);
    // #elif CHIP_DEVICE_LAYER_TARGET_LINUX
    //     VerifyOrReturnError(CHIP_NO_ERROR ==
    //     chip::DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().Init(CHIP_CONFIG_KVS_PATH),
    //                         FAILURE);
    // #endif
    SetGroupDataProvider(&sProvider);
    VerifyOrReturnError(CHIP_NO_ERROR == chip::Platform::MemoryInit(), FAILURE);
    VerifyOrReturnError(CHIP_NO_ERROR == sProvider.Init(), FAILURE);
    return SUCCESS;
}

/**
 *  Tear down the test suite.
 */
int Test_Teardown(void * inContext)
{
    chip::Platform::MemoryShutdown();
    GroupDataProvider * groups = GetGroupDataProvider();
    if (nullptr != groups)
    {
        groups->Finish();
    }
    return SUCCESS;
}

const nlTest sTests[] = {

    NL_TEST_DEF("TestStorageDelegate", chip::app::TestGroups::TestStorageDelegate),
    NL_TEST_DEF("TestGroupMappings", chip::app::TestGroups::TestGroupMappings),
    NL_TEST_DEF("TestGroupMappingIterator", chip::app::TestGroups::TestGroupMappingIterator),
    NL_TEST_DEF("TestGroupStates", chip::app::TestGroups::TestGroupStates),
    NL_TEST_DEF("TestGroupStateIterator", chip::app::TestGroups::TestGroupStateIterator),
    NL_TEST_DEF("TestKeysets", chip::app::TestGroups::TestKeysets),
    NL_TEST_DEF("TestKeysetIterator", chip::app::TestGroups::TestKeysetIterator),
    // Old Tests
    NL_TEST_DEF("TestEndpoints", chip::app::TestGroups::TestEndpoints),
    NL_TEST_DEF("TestEndpointIterator", chip::app::TestGroups::TestEndpointIterator),
    NL_TEST_DEF("TestStates", chip::app::TestGroups::TestStates),
    NL_TEST_DEF("TestStateIterator", chip::app::TestGroups::TestStateIterator),
    NL_TEST_DEF("TestKeys", chip::app::TestGroups::TestKeys),
    NL_TEST_DEF("TestKeysIterator", chip::app::TestGroups::TestKeysIterator),
    NL_TEST_DEF("TestPerFabricData", chip::app::TestGroups::TestPerFabricData), NL_TEST_SENTINEL()
};
} // namespace

int TestGroups()
{
    nlTestSuite theSuite = { "GroupDataProvider", &sTests[0], &Test_Setup, &Test_Teardown };

    nlTestRunner(&theSuite, nullptr);
    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestGroups)
