/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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

#include <set>
#include <string.h>
#include <tuple>
#include <utility>

#include <pw_unit_test/framework.h>

#include <credentials/GroupDataProviderImpl.h>
#include <crypto/DefaultSessionKeystore.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/core/TLV.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <platform/KeyValueStoreManager.h>

using namespace chip::Credentials;
using GroupInfo      = GroupDataProvider::GroupInfo;
using GroupKey       = GroupDataProvider::GroupKey;
using GroupEndpoint  = GroupDataProvider::GroupEndpoint;
using EpochKey       = GroupDataProvider::EpochKey;
using KeySet         = GroupDataProvider::KeySet;
using GroupSession   = GroupDataProvider::GroupSession;
using SecurityPolicy = GroupDataProvider::SecurityPolicy;

namespace chip {
namespace app {
namespace TestGroups {

static const char kKey1[]   = "abc/def";
static const char kValue1[] = "abc/def";
static const char kValue2[] = "abc/ghi/xyz";
static const size_t kSize1  = strlen(kValue1) + 1;
static const size_t kSize2  = strlen(kValue2) + 1;

constexpr uint16_t kMaxGroupsPerFabric    = 5;
constexpr uint16_t kMaxGroupKeysPerFabric = 4;

// If test cases covering more than 2 fabrics are added, update `ResetProvider` function.
constexpr chip::FabricIndex kFabric1 = 1;
constexpr chip::FabricIndex kFabric2 = 7;

// Currently unused constants that are useful for context
#if 0
static const uint8_t kExampleOperationalRootPublicKey[65] = {
    0x04, 0x4a, 0x9f, 0x42, 0xb1, 0xca, 0x48, 0x40, 0xd3, 0x72, 0x92, 0xbb, 0xc7, 0xf6, 0xa7, 0xe1, 0x1e,
    0x22, 0x20, 0x0c, 0x97, 0x6f, 0xc9, 0x00, 0xdb, 0xc9, 0x8a, 0x7a, 0x38, 0x3a, 0x64, 0x1c, 0xb8, 0x25,
    0x4a, 0x2e, 0x56, 0xd4, 0xe2, 0x95, 0xa8, 0x47, 0x94, 0x3b, 0x4e, 0x38, 0x97, 0xc4, 0xa7, 0x73, 0xe9,
    0x30, 0x27, 0x7b, 0x4d, 0x9f, 0xbe, 0xde, 0x8a, 0x05, 0x26, 0x86, 0xbf, 0xac, 0xfa,
};
static const ByteSpan kExampleOperationalRootPublicKeySpan{ kExampleOperationalRootPublicKey };

constexpr chip::FabricId kFabricId1               = 0x2906C908D115D362;
constexpr chip::FabricId kFabricId2               = 0x5E1C0F1B2C813C7A;
#endif

// kFabricId1/kCompressedFabricIdBuffer1 matches the Compressed Fabric Identifier
// example of spec section `4.3.2.2. Compressed Fabric Identifier`. It is based on
// the public key in `kExampleOperationalRootPublicKey`.
static const uint8_t kCompressedFabricIdBuffer1[] = { 0x87, 0xe1, 0xb0, 0x04, 0xe2, 0x35, 0xa1, 0x30 };
constexpr ByteSpan kCompressedFabricId1(kCompressedFabricIdBuffer1);

static const uint8_t kCompressedFabricIdBuffer2[] = { 0x3f, 0xaa, 0xe2, 0x90, 0x93, 0xd5, 0xaf, 0x45 };
constexpr ByteSpan kCompressedFabricId2(kCompressedFabricIdBuffer2);

constexpr chip::GroupId kGroup1 = kMinApplicationGroupId;
constexpr chip::GroupId kGroup2 = 0x2222;
constexpr chip::GroupId kGroup3 = kMaxApplicationGroupId;
constexpr chip::GroupId kGroup4 = 0x4444;
constexpr chip::GroupId kGroup5 = 0x5555;

constexpr chip::EndpointId kEndpointId0 = 0xee00;
constexpr chip::EndpointId kEndpointId1 = 0xee01;
constexpr chip::EndpointId kEndpointId2 = 0xee02;
constexpr chip::EndpointId kEndpointId3 = 0xee03;
constexpr chip::EndpointId kEndpointId4 = 0xee04;

constexpr uint16_t kKeysetId0 = 0x0;
constexpr uint16_t kKeysetId1 = 0x1111;
constexpr uint16_t kKeysetId2 = 0x2222;
constexpr uint16_t kKeysetId3 = 0x3333;
constexpr uint16_t kKeysetId4 = 0x4444;

static const GroupInfo kGroupInfo1_1(kGroup1, "Group-1.1");
static const GroupInfo kGroupInfo1_2(kGroup2, "Group-1.2");
static const GroupInfo kGroupInfo1_3(kGroup3, "Group-1.3");
static const GroupInfo kGroupInfo2_1(kGroup1, "Group-2.1");
static const GroupInfo kGroupInfo2_2(kGroup2, "Group-2.2");
static const GroupInfo kGroupInfo2_3(kGroup3, "Group-2.3");
static const GroupInfo kGroupInfo3_1(kGroup1, "Group-3.1");
static const GroupInfo kGroupInfo3_2(kGroup2, "Group-3.2");
static const GroupInfo kGroupInfo3_3(kGroup3, "Group-3.3");
static const GroupInfo kGroupInfo3_4(kGroup4, "Group-3.4");
static const GroupInfo kGroupInfo3_5(kGroup4, "Group-3.5");

static const GroupKey kGroup1Keyset0(kGroup1, kKeysetId0);
static const GroupKey kGroup1Keyset1(kGroup1, kKeysetId1);
static const GroupKey kGroup1Keyset2(kGroup1, kKeysetId2);
static const GroupKey kGroup1Keyset3(kGroup1, kKeysetId3);
static const GroupKey kGroup2Keyset0(kGroup2, kKeysetId0);
static const GroupKey kGroup2Keyset1(kGroup2, kKeysetId1);
static const GroupKey kGroup2Keyset2(kGroup2, kKeysetId2);
static const GroupKey kGroup2Keyset3(kGroup2, kKeysetId3);
static const GroupKey kGroup3Keyset0(kGroup3, kKeysetId0);
static const GroupKey kGroup3Keyset1(kGroup3, kKeysetId1);
static const GroupKey kGroup3Keyset2(kGroup3, kKeysetId2);
static const GroupKey kGroup3Keyset3(kGroup3, kKeysetId3);

static KeySet kKeySet0(kKeysetId0, SecurityPolicy::kCacheAndSync, 3);
static KeySet kKeySet1(kKeysetId1, SecurityPolicy::kTrustFirst, 1);
static KeySet kKeySet2(kKeysetId2, SecurityPolicy::kTrustFirst, 2);
static KeySet kKeySet3(kKeysetId3, SecurityPolicy::kCacheAndSync, 3);
static KeySet kKeySet4(kKeysetId4, SecurityPolicy::kTrustFirst, 1);

uint8_t kZeroKey[EpochKey::kLengthBytes] = { 0 };

class TestListener : public GroupDataProvider::GroupListener
{
public:
    chip::FabricIndex fabric_index = kUndefinedFabricIndex;
    GroupInfo latest;
    size_t added_count   = 0;
    size_t removed_count = 0;

    void Reset()
    {
        fabric_index  = kUndefinedFabricIndex;
        latest        = GroupInfo();
        added_count   = 0;
        removed_count = 0;
    }

    void OnGroupAdded(chip::FabricIndex fabric, const GroupInfo & new_group) override
    {
        fabric_index = fabric;
        latest       = new_group;
        added_count++;
    }
    void OnGroupRemoved(chip::FabricIndex fabric, const GroupInfo & old_group) override
    {
        fabric_index = fabric;
        latest       = old_group;
        removed_count++;
    }
};
static TestListener sListener;

void ResetProvider(GroupDataProvider * provider)
{
    provider->RemoveFabric(kFabric1);
    provider->RemoveFabric(kFabric2);
}

bool CompareKeySets(const KeySet & retrievedKeySet, const KeySet & keyset2)
{
    VerifyOrReturnError(retrievedKeySet.policy == keyset2.policy, false);
    VerifyOrReturnError(retrievedKeySet.num_keys_used == keyset2.num_keys_used, false);

    for (int i = 0; i < 3; i++)
    {
        if (i < retrievedKeySet.num_keys_used)
        {
            VerifyOrReturnError(retrievedKeySet.epoch_keys[i].start_time == keyset2.epoch_keys[i].start_time, false);
        }
        else
        {
            VerifyOrReturnError(retrievedKeySet.epoch_keys[i].start_time == 0, false);
        }

        VerifyOrReturnError(0 == memcmp(kZeroKey, retrievedKeySet.epoch_keys[i].key, EpochKey::kLengthBytes), false);
    }
    return true;
}

struct TestGroupDataProvider : public ::testing::Test
{

    static chip::TestPersistentStorageDelegate sDelegate;
    static chip::Crypto::DefaultSessionKeystore sSessionKeystore;
    static GroupDataProviderImpl sProvider;

    constexpr static EpochKey kEpochKeys0[] = {
        { 0x0000000000000000, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
        { 0x1111111111111111, { 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f } },
        { 0x2222222222222222, { 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f } }
    };
    constexpr static EpochKey kEpochKeys1[] = {
        { 0x3333333333333333, { 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f } },
        { 0x4444444444444444, { 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f } },
        { 0x5555555555555555, { 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f } },
    };
    constexpr static EpochKey kEpochKeys2[] = {
        { 0xaaaaaaaaaaaaaaaa, { 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf } },
        { 0xbbbbbbbbbbbbbbbb, { 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf } },
        { 0xcccccccccccccccc, { 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf } },
    };
    constexpr static EpochKey kEpochKeys3[] = {
        { 0xdddddddddddddddd, { 0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf } },
        { 0xeeeeeeeeeeeeeeee, { 0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef } },
        { 0xffffffffffffffff, { 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff } },
    };
    static void SetUpTestSuite()
    {

        EXPECT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);

        // Initialize Group Data Provider
        sProvider.SetStorageDelegate(&sDelegate);
        sProvider.SetSessionKeystore(&sSessionKeystore);
        sProvider.SetListener(&chip::app::TestGroups::sListener);
        EXPECT_EQ(sProvider.Init(), CHIP_NO_ERROR);
        SetGroupDataProvider(&sProvider);

        memcpy(chip::app::TestGroups::kKeySet0.epoch_keys, kEpochKeys0, sizeof(kEpochKeys0));
        memcpy(chip::app::TestGroups::kKeySet1.epoch_keys, kEpochKeys1, sizeof(kEpochKeys1));
        memcpy(chip::app::TestGroups::kKeySet2.epoch_keys, kEpochKeys2, sizeof(kEpochKeys2));
        memcpy(chip::app::TestGroups::kKeySet3.epoch_keys, kEpochKeys3, sizeof(kEpochKeys3));
    }
    static void TearDownTestSuite()
    {
        GroupDataProvider * provider = GetGroupDataProvider();
        if (nullptr != provider)
        {
            provider->Finish();
        }
        chip::Platform::MemoryShutdown();
    }
};

chip::TestPersistentStorageDelegate TestGroupDataProvider::sDelegate;
chip::Crypto::DefaultSessionKeystore TestGroupDataProvider::sSessionKeystore;
GroupDataProviderImpl TestGroupDataProvider::sProvider(kMaxGroupsPerFabric, kMaxGroupKeysPerFabric);

TEST_F(TestGroupDataProvider, TestStorageDelegate)
{
    chip::TestPersistentStorageDelegate delegate;

    char out[128];
    uint16_t size = static_cast<uint16_t>(sizeof(out));

    EXPECT_EQ(CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND, delegate.SyncGetKeyValue(kKey1, out, size));

    size = static_cast<uint16_t>(kSize1);
    EXPECT_EQ(delegate.SyncSetKeyValue(kKey1, kValue1, size), CHIP_NO_ERROR);

    size = static_cast<uint16_t>(sizeof(out));
    EXPECT_EQ(delegate.SyncGetKeyValue(kKey1, out, size), CHIP_NO_ERROR);
    EXPECT_EQ(size, kSize1);
    EXPECT_FALSE(memcmp(out, kValue1, kSize1));

    size = static_cast<uint16_t>(kSize2);
    EXPECT_EQ(delegate.SyncSetKeyValue(kKey1, kValue2, size), CHIP_NO_ERROR);

    size = static_cast<uint16_t>(sizeof(out));
    EXPECT_EQ(delegate.SyncGetKeyValue(kKey1, out, size), CHIP_NO_ERROR);
    EXPECT_EQ(size, kSize2);
    EXPECT_FALSE(memcmp(out, kValue2, kSize2));

    EXPECT_EQ(delegate.SyncDeleteKeyValue(kKey1), CHIP_NO_ERROR);

    size = static_cast<uint16_t>(sizeof(out));
    EXPECT_EQ(CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND, delegate.SyncGetKeyValue(kKey1, out, size));
}

TEST_F(TestGroupDataProvider, TestGroupInfo)
{
    GroupDataProvider * provider = GetGroupDataProvider();
    EXPECT_TRUE(provider);

    // Reset test
    ResetProvider(provider);

    GroupInfo group;

    // Set Group Info

    sListener.Reset();

    // Out-of-order
    EXPECT_EQ(CHIP_ERROR_INVALID_ARGUMENT, provider->SetGroupInfoAt(kFabric1, 2, kGroupInfo1_1));

    EXPECT_EQ(provider->SetGroupInfoAt(kFabric1, 0, kGroupInfo1_1), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetGroupInfoAt(kFabric1, 1, kGroupInfo1_2), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetGroupInfoAt(kFabric1, 2, kGroupInfo1_3), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetGroupInfoAt(kFabric2, 0, kGroupInfo2_1), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetGroupInfoAt(kFabric2, 1, kGroupInfo2_2), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetGroupInfoAt(kFabric2, 2, kGroupInfo2_3), CHIP_NO_ERROR);

    // Duplicated
    EXPECT_EQ(CHIP_ERROR_DUPLICATE_KEY_ID, provider->SetGroupInfoAt(kFabric1, 3, kGroupInfo1_1));
    EXPECT_EQ(CHIP_ERROR_DUPLICATE_KEY_ID, provider->SetGroupInfoAt(kFabric2, 3, kGroupInfo2_3));

    // Get Group Info

    EXPECT_EQ(CHIP_ERROR_INVALID_FABRIC_INDEX, provider->GetGroupInfoAt(kUndefinedFabricIndex, 0, group));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, provider->GetGroupInfoAt(kFabric2, 999, group));

    EXPECT_EQ(sListener.latest, kGroupInfo2_3);
    EXPECT_EQ(sListener.added_count, 6u);
    EXPECT_EQ(sListener.removed_count, 0u);

    EXPECT_EQ(provider->GetGroupInfoAt(kFabric2, 2, group), CHIP_NO_ERROR);
    EXPECT_EQ(group, kGroupInfo2_3);
    EXPECT_EQ(provider->GetGroupInfoAt(kFabric2, 1, group), CHIP_NO_ERROR);
    EXPECT_EQ(group, kGroupInfo2_2);
    EXPECT_EQ(provider->GetGroupInfoAt(kFabric2, 0, group), CHIP_NO_ERROR);
    EXPECT_EQ(group, kGroupInfo2_1);
    EXPECT_EQ(provider->GetGroupInfoAt(kFabric1, 1, group), CHIP_NO_ERROR);
    EXPECT_EQ(group, kGroupInfo1_2);
    EXPECT_EQ(provider->GetGroupInfoAt(kFabric1, 0, group), CHIP_NO_ERROR);
    EXPECT_EQ(group, kGroupInfo1_1);
    EXPECT_EQ(provider->GetGroupInfoAt(kFabric1, 2, group), CHIP_NO_ERROR);
    EXPECT_EQ(group, kGroupInfo1_3);

    // Remove Groups

    EXPECT_EQ(provider->RemoveGroupInfo(kFabric1, kGroup3), CHIP_NO_ERROR);
    EXPECT_EQ(provider->RemoveGroupInfoAt(kFabric2, 0), CHIP_NO_ERROR);
    EXPECT_EQ(sListener.latest, kGroupInfo2_1);
    EXPECT_EQ(sListener.added_count, 6u);
    EXPECT_EQ(sListener.removed_count, 2u);

    // Remaining entries shift up

    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, provider->GetGroupInfoAt(kFabric2, 2, group));
    EXPECT_EQ(provider->GetGroupInfoAt(kFabric2, 1, group), CHIP_NO_ERROR);
    EXPECT_EQ(group, kGroupInfo2_3);
    EXPECT_EQ(provider->GetGroupInfoAt(kFabric2, 0, group), CHIP_NO_ERROR);
    EXPECT_EQ(group, kGroupInfo2_2);
    EXPECT_EQ(provider->GetGroupInfoAt(kFabric1, 1, group), CHIP_NO_ERROR);
    EXPECT_EQ(group, kGroupInfo1_2);
    EXPECT_EQ(provider->GetGroupInfoAt(kFabric1, 0, group), CHIP_NO_ERROR);
    EXPECT_EQ(group, kGroupInfo1_1);
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, provider->GetGroupInfoAt(kFabric1, 3, group));

    // Overwrite with new group

    EXPECT_EQ(provider->SetGroupInfoAt(kFabric1, 2, kGroupInfo3_4), CHIP_NO_ERROR);
    // Replace existing group (implicit group remove)
    EXPECT_EQ(provider->SetGroupInfoAt(kFabric2, 0, kGroupInfo3_4), CHIP_NO_ERROR);

    EXPECT_EQ(provider->GetGroupInfoAt(kFabric1, 2, group), CHIP_NO_ERROR);
    EXPECT_EQ(group, kGroupInfo3_4);

    EXPECT_EQ(provider->GetGroupInfoAt(kFabric2, 0, group), CHIP_NO_ERROR);
    EXPECT_EQ(group, kGroupInfo3_4);
    EXPECT_EQ(sListener.latest, kGroupInfo3_4);
    EXPECT_EQ(sListener.added_count, 8u);
    EXPECT_EQ(sListener.removed_count, 3u);

    // Overwrite existing group, index must match

    EXPECT_EQ(CHIP_ERROR_DUPLICATE_KEY_ID, provider->SetGroupInfoAt(kFabric1, 1, kGroupInfo1_1));
    EXPECT_EQ(provider->SetGroupInfoAt(kFabric1, 1, kGroupInfo2_2), CHIP_NO_ERROR);

    EXPECT_EQ(CHIP_ERROR_DUPLICATE_KEY_ID, provider->SetGroupInfoAt(kFabric2, 1, kGroupInfo3_4));
    EXPECT_EQ(provider->SetGroupInfoAt(kFabric2, 1, kGroupInfo1_3), CHIP_NO_ERROR);

    EXPECT_EQ(provider->GetGroupInfoAt(kFabric1, 1, group), CHIP_NO_ERROR);
    EXPECT_EQ(group, kGroupInfo2_2);

    EXPECT_EQ(provider->GetGroupInfoAt(kFabric2, 1, group), CHIP_NO_ERROR);
    EXPECT_EQ(group, kGroupInfo1_3);
    EXPECT_EQ(sListener.latest, kGroupInfo3_4);
    EXPECT_EQ(sListener.added_count, 8u);
    EXPECT_EQ(sListener.removed_count, 3u);

    // By group_id

    // Override existing
    EXPECT_EQ(provider->SetGroupInfo(kFabric1, kGroupInfo3_5), CHIP_NO_ERROR);
    // New group
    EXPECT_EQ(provider->SetGroupInfo(kFabric2, kGroupInfo3_2), CHIP_NO_ERROR);
    // Not found
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, provider->GetGroupInfo(kFabric2, kGroup5, group));
    // Existing
    EXPECT_EQ(provider->GetGroupInfo(kFabric2, kGroup2, group), CHIP_NO_ERROR);
    EXPECT_EQ(group, kGroupInfo3_2);
    EXPECT_EQ(sListener.latest, kGroupInfo3_2);
    EXPECT_EQ(sListener.added_count, 9u);
    EXPECT_EQ(sListener.removed_count, 3u);
}

TEST_F(TestGroupDataProvider, TestGroupInfoIterator)
{
    GroupDataProvider * provider = GetGroupDataProvider();
    EXPECT_TRUE(provider);

    // Reset test
    ResetProvider(provider);

    GroupInfo group;

    // Set Group Info

    EXPECT_EQ(provider->SetGroupInfoAt(kFabric1, 0, kGroupInfo1_3), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetGroupInfoAt(kFabric1, 1, kGroupInfo1_2), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetGroupInfoAt(kFabric1, 2, kGroupInfo1_1), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetGroupInfoAt(kFabric2, 0, kGroupInfo2_1), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetGroupInfoAt(kFabric2, 1, kGroupInfo2_3), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetGroupInfoAt(kFabric2, 2, kGroupInfo2_2), CHIP_NO_ERROR);

    // Iterate fabric 1

    GroupInfo expected_f1[]  = { kGroupInfo1_3, kGroupInfo1_2, kGroupInfo1_1 };
    size_t expected_f1_count = sizeof(expected_f1) / sizeof(GroupInfo);

    auto it  = provider->IterateGroupInfo(kFabric1);
    size_t i = 0;
    ASSERT_TRUE(it);
    EXPECT_EQ(expected_f1_count, it->Count());
    while (it->Next(group) && i < expected_f1_count)
    {
        EXPECT_EQ(expected_f1[i++], group);
    }
    EXPECT_EQ(i, it->Count());
    it->Release();

    // Iterate fabric 2

    GroupInfo expected_f2[]  = { kGroupInfo2_1, kGroupInfo2_3, kGroupInfo2_2 };
    size_t expected_f2_count = sizeof(expected_f2) / sizeof(GroupInfo);

    it = provider->IterateGroupInfo(kFabric2);
    ASSERT_TRUE(it);
    i = 0;
    EXPECT_EQ(expected_f2_count, it->Count());
    while (it->Next(group) && i < expected_f2_count)
    {
        EXPECT_EQ(expected_f2[i++], group);
    }
    EXPECT_EQ(i, it->Count());
    it->Release();
}

TEST_F(TestGroupDataProvider, TestEndpoints)
{
    GroupDataProvider * provider = GetGroupDataProvider();
    EXPECT_TRUE(provider);

    // Reset test
    ResetProvider(provider);

    GroupInfo group;

    // Existing groups

    EXPECT_EQ(provider->SetGroupInfoAt(kFabric1, 0, kGroupInfo1_1), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetGroupInfoAt(kFabric1, 1, kGroupInfo1_2), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetGroupInfoAt(kFabric1, 2, kGroupInfo1_3), CHIP_NO_ERROR);

    EXPECT_FALSE(provider->HasEndpoint(kFabric1, kGroup1, kEndpointId0));
    EXPECT_FALSE(provider->HasEndpoint(kFabric1, kGroup1, kEndpointId1));
    EXPECT_FALSE(provider->HasEndpoint(kFabric1, kGroup2, kEndpointId2));
    EXPECT_FALSE(provider->HasEndpoint(kFabric1, kGroup2, kEndpointId3));

    EXPECT_EQ(provider->AddEndpoint(kFabric1, kGroup1, kEndpointId0), CHIP_NO_ERROR);
    EXPECT_EQ(provider->AddEndpoint(kFabric1, kGroup1, kEndpointId1), CHIP_NO_ERROR);
    EXPECT_EQ(provider->AddEndpoint(kFabric1, kGroup2, kEndpointId2), CHIP_NO_ERROR);
    EXPECT_EQ(provider->AddEndpoint(kFabric1, kGroup2, kEndpointId3), CHIP_NO_ERROR);

    EXPECT_TRUE(provider->HasEndpoint(kFabric1, kGroup2, kEndpointId3));
    EXPECT_TRUE(provider->HasEndpoint(kFabric1, kGroup2, kEndpointId2));
    EXPECT_TRUE(provider->HasEndpoint(kFabric1, kGroup1, kEndpointId1));
    EXPECT_TRUE(provider->HasEndpoint(kFabric1, kGroup1, kEndpointId0));

    // New groups

    EXPECT_FALSE(provider->HasEndpoint(kFabric2, kGroup1, kEndpointId0));
    EXPECT_FALSE(provider->HasEndpoint(kFabric2, kGroup2, kEndpointId1));
    EXPECT_FALSE(provider->HasEndpoint(kFabric2, kGroup3, kEndpointId2));
    EXPECT_FALSE(provider->HasEndpoint(kFabric2, kGroup4, kEndpointId3));

    EXPECT_EQ(provider->AddEndpoint(kFabric2, kGroup1, kEndpointId0), CHIP_NO_ERROR);
    EXPECT_EQ(provider->AddEndpoint(kFabric2, kGroup2, kEndpointId1), CHIP_NO_ERROR);
    EXPECT_EQ(provider->AddEndpoint(kFabric2, kGroup3, kEndpointId2), CHIP_NO_ERROR);
    EXPECT_EQ(provider->AddEndpoint(kFabric2, kGroup4, kEndpointId3), CHIP_NO_ERROR);

    EXPECT_TRUE(provider->HasEndpoint(kFabric2, kGroup1, kEndpointId0));
    EXPECT_TRUE(provider->HasEndpoint(kFabric2, kGroup2, kEndpointId1));
    EXPECT_TRUE(provider->HasEndpoint(kFabric2, kGroup3, kEndpointId2));
    EXPECT_TRUE(provider->HasEndpoint(kFabric2, kGroup4, kEndpointId3));

    // Remove

    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, provider->RemoveEndpoint(kFabric1, kGroup1, kEndpointId4));
    EXPECT_EQ(provider->RemoveEndpoint(kFabric1, kGroup1, kEndpointId0), CHIP_NO_ERROR);
    EXPECT_EQ(provider->RemoveEndpoint(kFabric1, kGroup2, kEndpointId3), CHIP_NO_ERROR);

    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, provider->RemoveEndpoint(kFabric2, kGroup5, kEndpointId0));
    EXPECT_EQ(provider->RemoveEndpoint(kFabric2, kGroup2, kEndpointId1), CHIP_NO_ERROR);
    EXPECT_EQ(provider->RemoveEndpoint(kFabric2, kGroup3, kEndpointId2), CHIP_NO_ERROR);

    // Check removed

    EXPECT_FALSE(provider->HasEndpoint(kFabric1, kGroup1, kEndpointId0));
    EXPECT_TRUE(provider->HasEndpoint(kFabric1, kGroup1, kEndpointId1));
    EXPECT_TRUE(provider->HasEndpoint(kFabric1, kGroup2, kEndpointId2));
    EXPECT_FALSE(provider->HasEndpoint(kFabric1, kGroup2, kEndpointId3));

    EXPECT_TRUE(provider->HasEndpoint(kFabric2, kGroup1, kEndpointId0));
    EXPECT_FALSE(provider->HasEndpoint(kFabric2, kGroup2, kEndpointId1));
    EXPECT_FALSE(provider->HasEndpoint(kFabric2, kGroup3, kEndpointId2));
    EXPECT_TRUE(provider->HasEndpoint(kFabric2, kGroup4, kEndpointId3));

    // Remove All

    EXPECT_EQ(provider->AddEndpoint(kFabric1, kGroup1, kEndpointId3), CHIP_NO_ERROR);
    EXPECT_EQ(provider->AddEndpoint(kFabric1, kGroup2, kEndpointId3), CHIP_NO_ERROR);
    EXPECT_EQ(provider->AddEndpoint(kFabric1, kGroup3, kEndpointId3), CHIP_NO_ERROR);
    EXPECT_EQ(provider->AddEndpoint(kFabric1, kGroup4, kEndpointId3), CHIP_NO_ERROR);

    EXPECT_TRUE(provider->HasEndpoint(kFabric1, kGroup1, kEndpointId1));
    EXPECT_TRUE(provider->HasEndpoint(kFabric1, kGroup1, kEndpointId3));
    EXPECT_TRUE(provider->HasEndpoint(kFabric1, kGroup2, kEndpointId3));
    EXPECT_TRUE(provider->HasEndpoint(kFabric1, kGroup2, kEndpointId2));
    EXPECT_TRUE(provider->HasEndpoint(kFabric1, kGroup3, kEndpointId3));
    EXPECT_TRUE(provider->HasEndpoint(kFabric1, kGroup4, kEndpointId3));

    EXPECT_EQ(provider->RemoveEndpoint(kFabric1, kEndpointId3), CHIP_NO_ERROR);

    EXPECT_TRUE(provider->HasEndpoint(kFabric1, kGroup1, kEndpointId1));
    EXPECT_FALSE(provider->HasEndpoint(kFabric1, kGroup1, kEndpointId3));
    EXPECT_FALSE(provider->HasEndpoint(kFabric1, kGroup2, kEndpointId3));
    EXPECT_TRUE(provider->HasEndpoint(kFabric1, kGroup2, kEndpointId2));
    EXPECT_FALSE(provider->HasEndpoint(kFabric1, kGroup3, kEndpointId3));
    EXPECT_FALSE(provider->HasEndpoint(kFabric1, kGroup4, kEndpointId3));
}

TEST_F(TestGroupDataProvider, TestEndpointIterator)
{
    GroupDataProvider * provider = GetGroupDataProvider();
    EXPECT_TRUE(provider);

    // Reset test
    ResetProvider(provider);

    GroupInfo group;

    // Set Endpoints

    EXPECT_EQ(provider->AddEndpoint(kFabric1, kGroup1, kEndpointId0), CHIP_NO_ERROR);
    EXPECT_EQ(provider->AddEndpoint(kFabric1, kGroup1, kEndpointId2), CHIP_NO_ERROR);
    EXPECT_EQ(provider->AddEndpoint(kFabric1, kGroup1, kEndpointId4), CHIP_NO_ERROR);
    EXPECT_EQ(provider->AddEndpoint(kFabric1, kGroup2, kEndpointId1), CHIP_NO_ERROR);
    EXPECT_EQ(provider->AddEndpoint(kFabric1, kGroup2, kEndpointId2), CHIP_NO_ERROR);
    EXPECT_EQ(provider->AddEndpoint(kFabric1, kGroup2, kEndpointId3), CHIP_NO_ERROR);

    EXPECT_EQ(provider->AddEndpoint(kFabric2, kGroup3, kEndpointId0), CHIP_NO_ERROR);
    EXPECT_EQ(provider->AddEndpoint(kFabric2, kGroup3, kEndpointId1), CHIP_NO_ERROR);
    EXPECT_EQ(provider->AddEndpoint(kFabric2, kGroup3, kEndpointId2), CHIP_NO_ERROR);
    EXPECT_EQ(provider->AddEndpoint(kFabric2, kGroup3, kEndpointId3), CHIP_NO_ERROR);
    EXPECT_EQ(provider->AddEndpoint(kFabric2, kGroup3, kEndpointId4), CHIP_NO_ERROR);

    // Iterate fabric 1

    std::set<std::pair<GroupId, EndpointId>> expected_f1 = {
        { kGroup1, kEndpointId0 }, { kGroup1, kEndpointId2 }, { kGroup1, kEndpointId4 },
        { kGroup2, kEndpointId1 }, { kGroup2, kEndpointId2 }, { kGroup2, kEndpointId3 },
    };

    auto it      = provider->IterateEndpoints(kFabric1);
    size_t count = 0;
    ASSERT_TRUE(it);
    GroupEndpoint output;
    EXPECT_EQ(expected_f1.size(), it->Count());
    while (it->Next(output) && count < expected_f1.size())
    {
        std::pair<chip::GroupId, chip::EndpointId> mapping(output.group_id, output.endpoint_id);
        EXPECT_GT(expected_f1.count(mapping), 0u);
        count++;
    }
    EXPECT_EQ(count, it->Count());
    it->Release();

    // Iterate fabric 2

    std::set<std::pair<GroupId, EndpointId>> expected_f2 = {
        { kGroup3, kEndpointId0 }, { kGroup3, kEndpointId1 }, { kGroup3, kEndpointId2 },
        { kGroup3, kEndpointId3 }, { kGroup3, kEndpointId4 },
    };

    it = provider->IterateEndpoints(kFabric2);
    ASSERT_TRUE(it);
    count = 0;
    EXPECT_EQ(expected_f2.size(), it->Count());
    while (it->Next(output) && count < expected_f2.size())
    {
        std::pair<chip::GroupId, chip::EndpointId> mapping(output.group_id, output.endpoint_id);
        EXPECT_GT(expected_f2.count(mapping), 0u);
        count++;
    }
    EXPECT_EQ(count, it->Count());
    it->Release();
}

TEST_F(TestGroupDataProvider, TestGroupKeys)
{
    GroupDataProvider * provider = GetGroupDataProvider();
    EXPECT_TRUE(provider);

    // Reset test
    ResetProvider(provider);

    GroupKey pair;

    // Set Group Info

    // Out-of-order
    EXPECT_EQ(CHIP_ERROR_INVALID_ARGUMENT, provider->SetGroupKeyAt(kFabric1, 2, kGroup1Keyset0));

    EXPECT_EQ(provider->SetGroupKeyAt(kFabric1, 0, kGroup1Keyset0), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetGroupKeyAt(kFabric1, 1, kGroup1Keyset1), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetGroupKeyAt(kFabric1, 2, kGroup1Keyset2), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetGroupKeyAt(kFabric1, 3, kGroup1Keyset3), CHIP_NO_ERROR);
    // Duplicated
    EXPECT_EQ(CHIP_ERROR_DUPLICATE_KEY_ID, provider->SetGroupKeyAt(kFabric1, 4, kGroup1Keyset2));

    EXPECT_EQ(provider->SetGroupKeyAt(kFabric2, 0, kGroup2Keyset0), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetGroupKeyAt(kFabric2, 1, kGroup2Keyset1), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetGroupKeyAt(kFabric2, 2, kGroup2Keyset2), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetGroupKeyAt(kFabric2, 3, kGroup2Keyset3), CHIP_NO_ERROR);
    // Duplicated
    EXPECT_EQ(CHIP_ERROR_DUPLICATE_KEY_ID, provider->SetGroupKeyAt(kFabric2, 4, kGroup2Keyset0));

    // Get Group Info

    EXPECT_EQ(CHIP_ERROR_INVALID_FABRIC_INDEX, provider->GetGroupKeyAt(kUndefinedFabricIndex, 0, pair));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, provider->GetGroupKeyAt(kFabric2, 999, pair));

    EXPECT_EQ(provider->GetGroupKeyAt(kFabric1, 3, pair), CHIP_NO_ERROR);
    EXPECT_EQ(pair, kGroup1Keyset3);
    EXPECT_EQ(provider->GetGroupKeyAt(kFabric1, 2, pair), CHIP_NO_ERROR);
    EXPECT_EQ(pair, kGroup1Keyset2);
    EXPECT_EQ(provider->GetGroupKeyAt(kFabric1, 1, pair), CHIP_NO_ERROR);
    EXPECT_EQ(pair, kGroup1Keyset1);
    EXPECT_EQ(provider->GetGroupKeyAt(kFabric1, 0, pair), CHIP_NO_ERROR);
    EXPECT_EQ(pair, kGroup1Keyset0);

    EXPECT_EQ(provider->GetGroupKeyAt(kFabric2, 3, pair), CHIP_NO_ERROR);
    EXPECT_EQ(pair, kGroup2Keyset3);
    EXPECT_EQ(provider->GetGroupKeyAt(kFabric2, 2, pair), CHIP_NO_ERROR);
    EXPECT_EQ(pair, kGroup2Keyset2);
    EXPECT_EQ(provider->GetGroupKeyAt(kFabric2, 1, pair), CHIP_NO_ERROR);
    EXPECT_EQ(pair, kGroup2Keyset1);
    EXPECT_EQ(provider->GetGroupKeyAt(kFabric2, 0, pair), CHIP_NO_ERROR);
    EXPECT_EQ(pair, kGroup2Keyset0);

    // Remove Groups (remaining entries shift up)

    EXPECT_EQ(provider->RemoveGroupKeyAt(kFabric1, 2), CHIP_NO_ERROR);
    EXPECT_EQ(provider->RemoveGroupKeyAt(kFabric2, 0), CHIP_NO_ERROR);

    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, provider->GetGroupKeyAt(kFabric1, 3, pair));
    EXPECT_EQ(provider->GetGroupKeyAt(kFabric1, 2, pair), CHIP_NO_ERROR);
    EXPECT_EQ(pair, kGroup1Keyset3);
    EXPECT_EQ(provider->GetGroupKeyAt(kFabric1, 1, pair), CHIP_NO_ERROR);
    EXPECT_EQ(pair, kGroup1Keyset1);
    EXPECT_EQ(provider->GetGroupKeyAt(kFabric1, 0, pair), CHIP_NO_ERROR);
    EXPECT_EQ(pair, kGroup1Keyset0);

    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, provider->GetGroupKeyAt(kFabric2, 3, pair));
    EXPECT_EQ(provider->GetGroupKeyAt(kFabric2, 2, pair), CHIP_NO_ERROR);
    EXPECT_EQ(pair, kGroup2Keyset3);
    EXPECT_EQ(provider->GetGroupKeyAt(kFabric2, 1, pair), CHIP_NO_ERROR);
    EXPECT_EQ(pair, kGroup2Keyset2);
    EXPECT_EQ(provider->GetGroupKeyAt(kFabric2, 0, pair), CHIP_NO_ERROR);
    EXPECT_EQ(pair, kGroup2Keyset1);

    // Overwrite, (group_id, keyset_id) must be unique

    EXPECT_EQ(CHIP_ERROR_DUPLICATE_KEY_ID, provider->SetGroupKeyAt(kFabric1, 2, kGroup1Keyset0));
    EXPECT_EQ(provider->SetGroupKeyAt(kFabric1, 2, kGroup3Keyset0), CHIP_NO_ERROR);
    EXPECT_EQ(CHIP_ERROR_DUPLICATE_KEY_ID, provider->SetGroupKeyAt(kFabric2, 0, kGroup2Keyset2));
    EXPECT_EQ(provider->SetGroupKeyAt(kFabric2, 0, kGroup3Keyset1), CHIP_NO_ERROR);

    EXPECT_EQ(provider->GetGroupKeyAt(kFabric1, 2, pair), CHIP_NO_ERROR);
    EXPECT_EQ(pair, kGroup3Keyset0);

    EXPECT_EQ(provider->GetGroupKeyAt(kFabric2, 0, pair), CHIP_NO_ERROR);
    EXPECT_EQ(pair, kGroup3Keyset1);

    // Remove all

    EXPECT_EQ(provider->RemoveGroupKeys(kFabric1), CHIP_NO_ERROR);

    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, provider->GetGroupKeyAt(kFabric1, 3, pair));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, provider->GetGroupKeyAt(kFabric1, 2, pair));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, provider->GetGroupKeyAt(kFabric1, 1, pair));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, provider->GetGroupKeyAt(kFabric1, 0, pair));

    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, provider->GetGroupKeyAt(kFabric2, 3, pair));
    EXPECT_EQ(provider->GetGroupKeyAt(kFabric2, 2, pair), CHIP_NO_ERROR);
    EXPECT_EQ(pair, kGroup2Keyset3);
    EXPECT_EQ(provider->GetGroupKeyAt(kFabric2, 1, pair), CHIP_NO_ERROR);
    EXPECT_EQ(pair, kGroup2Keyset2);
    EXPECT_EQ(provider->GetGroupKeyAt(kFabric2, 0, pair), CHIP_NO_ERROR);
    EXPECT_EQ(pair, kGroup3Keyset1);
}

TEST_F(TestGroupDataProvider, TestGroupKeyIterator)
{
    GroupDataProvider * provider = GetGroupDataProvider();
    EXPECT_TRUE(provider);

    // Reset test
    ResetProvider(provider);

    GroupKey pair;

    // Set Group Info

    EXPECT_EQ(provider->SetGroupKeyAt(kFabric1, 0, kGroup3Keyset0), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetGroupKeyAt(kFabric1, 1, kGroup3Keyset1), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetGroupKeyAt(kFabric1, 2, kGroup3Keyset2), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetGroupKeyAt(kFabric1, 3, kGroup3Keyset3), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetGroupKeyAt(kFabric1, 4, kGroup1Keyset0), CHIP_NO_ERROR);
    EXPECT_NE(CHIP_NO_ERROR, provider->SetGroupKeyAt(kFabric1, 5, kGroup1Keyset1));

    EXPECT_EQ(provider->SetGroupKeyAt(kFabric2, 0, kGroup2Keyset0), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetGroupKeyAt(kFabric2, 1, kGroup2Keyset1), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetGroupKeyAt(kFabric2, 2, kGroup2Keyset2), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetGroupKeyAt(kFabric2, 3, kGroup2Keyset3), CHIP_NO_ERROR);

    // Iterate fabric 1

    GroupKey expected_f1[]   = { kGroup3Keyset0, kGroup3Keyset1, kGroup3Keyset2, kGroup3Keyset3, kGroup1Keyset0 };
    size_t expected_f1_count = sizeof(expected_f1) / sizeof(GroupKey);

    auto it      = provider->IterateGroupKeys(kFabric1);
    size_t count = 0;
    ASSERT_TRUE(it);
    EXPECT_EQ(expected_f1_count, it->Count());
    while (it->Next(pair) && count < expected_f1_count)
    {
        EXPECT_EQ(expected_f1[count++], pair);
    }
    EXPECT_EQ(count, it->Count());
    it->Release();

    // Iterate fabric 2

    GroupKey expected_f2[]   = { kGroup2Keyset0, kGroup2Keyset1, kGroup2Keyset2, kGroup2Keyset3 };
    size_t expected_f2_count = sizeof(expected_f2) / sizeof(GroupKey);

    it = provider->IterateGroupKeys(kFabric2);
    ASSERT_TRUE(it);
    count = 0;
    EXPECT_EQ(expected_f2_count, it->Count());
    while (it->Next(pair) && count < expected_f2_count)
    {
        EXPECT_EQ(expected_f2[count++], pair);
    }
    EXPECT_EQ(count, it->Count());
    it->Release();
}

TEST_F(TestGroupDataProvider, TestKeySets)
{
    GroupDataProvider * provider = GetGroupDataProvider();
    EXPECT_TRUE(provider);

    // Reset test
    ResetProvider(provider);

    KeySet keyset;

    // Add KeySets

    EXPECT_EQ(provider->SetKeySet(kFabric1, kCompressedFabricId1, kKeySet1), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetKeySet(kFabric1, kCompressedFabricId1, kKeySet0), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetKeySet(kFabric1, kCompressedFabricId1, kKeySet2), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetKeySet(kFabric1, kCompressedFabricId1, kKeySet3), CHIP_NO_ERROR);
    EXPECT_NE(CHIP_NO_ERROR, provider->SetKeySet(kFabric1, kCompressedFabricId1, kKeySet4));
    EXPECT_EQ(provider->SetKeySet(kFabric2, kCompressedFabricId2, kKeySet3), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetKeySet(kFabric2, kCompressedFabricId2, kKeySet0), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetKeySet(kFabric2, kCompressedFabricId2, kKeySet2), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetKeySet(kFabric2, kCompressedFabricId2, kKeySet1), CHIP_NO_ERROR);

    // Get KeySets

    EXPECT_EQ(provider->GetKeySet(kFabric1, kKeysetId3, keyset), CHIP_NO_ERROR);
    EXPECT_TRUE(CompareKeySets(keyset, kKeySet3));

    EXPECT_EQ(provider->GetKeySet(kFabric1, kKeysetId1, keyset), CHIP_NO_ERROR);
    EXPECT_TRUE(CompareKeySets(keyset, kKeySet1));

    EXPECT_EQ(provider->GetKeySet(kFabric1, kKeysetId0, keyset), CHIP_NO_ERROR);
    EXPECT_TRUE(CompareKeySets(keyset, kKeySet0));

    EXPECT_EQ(provider->GetKeySet(kFabric1, kKeysetId2, keyset), CHIP_NO_ERROR);
    EXPECT_TRUE(CompareKeySets(keyset, kKeySet2));

    EXPECT_EQ(provider->GetKeySet(kFabric2, kKeysetId3, keyset), CHIP_NO_ERROR);
    EXPECT_TRUE(CompareKeySets(keyset, kKeySet3));

    EXPECT_EQ(provider->GetKeySet(kFabric2, kKeysetId2, keyset), CHIP_NO_ERROR);
    EXPECT_TRUE(CompareKeySets(keyset, kKeySet2));

    EXPECT_EQ(provider->GetKeySet(kFabric2, kKeysetId1, keyset), CHIP_NO_ERROR);
    EXPECT_TRUE(CompareKeySets(keyset, kKeySet1));

    EXPECT_EQ(provider->GetKeySet(kFabric2, kKeysetId0, keyset), CHIP_NO_ERROR);
    EXPECT_TRUE(CompareKeySets(keyset, kKeySet0));

    // Remove Keysets

    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, provider->RemoveKeySet(kFabric1, 0xffff));

    EXPECT_EQ(provider->RemoveKeySet(kFabric1, kKeysetId1), CHIP_NO_ERROR); // First
    EXPECT_EQ(provider->RemoveKeySet(kFabric1, kKeysetId3), CHIP_NO_ERROR); // Last
    EXPECT_EQ(provider->RemoveKeySet(kFabric2, kKeysetId2), CHIP_NO_ERROR); // Middle

    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, provider->GetKeySet(kFabric1, kKeysetId3, keyset));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, provider->GetKeySet(kFabric1, kKeysetId1, keyset));
    EXPECT_EQ(provider->GetKeySet(kFabric1, kKeysetId0, keyset), CHIP_NO_ERROR);
    EXPECT_TRUE(CompareKeySets(keyset, kKeySet0));

    EXPECT_EQ(provider->GetKeySet(kFabric1, kKeysetId2, keyset), CHIP_NO_ERROR);
    EXPECT_TRUE(CompareKeySets(keyset, kKeySet2));

    EXPECT_EQ(provider->GetKeySet(kFabric2, kKeysetId3, keyset), CHIP_NO_ERROR);
    EXPECT_TRUE(CompareKeySets(keyset, kKeySet3));

    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, provider->GetKeySet(kFabric2, kKeysetId2, keyset));
    EXPECT_EQ(provider->GetKeySet(kFabric2, kKeysetId1, keyset), CHIP_NO_ERROR);
    EXPECT_TRUE(CompareKeySets(keyset, kKeySet1));

    EXPECT_EQ(provider->GetKeySet(kFabric2, kKeysetId0, keyset), CHIP_NO_ERROR);
    EXPECT_TRUE(CompareKeySets(keyset, kKeySet0));

    // Remove all

    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, provider->RemoveKeySet(kFabric1, kKeysetId3));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, provider->RemoveKeySet(kFabric1, kKeysetId1));
    EXPECT_EQ(provider->RemoveKeySet(kFabric1, kKeysetId0), CHIP_NO_ERROR);
    EXPECT_EQ(provider->RemoveKeySet(kFabric1, kKeysetId2), CHIP_NO_ERROR);
    EXPECT_EQ(provider->RemoveKeySet(kFabric2, kKeysetId3), CHIP_NO_ERROR);
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, provider->RemoveKeySet(kFabric2, kKeysetId2));
    EXPECT_EQ(provider->RemoveKeySet(kFabric2, kKeysetId1), CHIP_NO_ERROR);
    EXPECT_EQ(provider->RemoveKeySet(kFabric2, kKeysetId0), CHIP_NO_ERROR);

    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, provider->GetKeySet(kFabric1, kKeysetId3, keyset));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, provider->GetKeySet(kFabric1, kKeysetId1, keyset));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, provider->GetKeySet(kFabric1, kKeysetId0, keyset));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, provider->GetKeySet(kFabric1, kKeysetId2, keyset));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, provider->GetKeySet(kFabric2, kKeysetId3, keyset));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, provider->GetKeySet(kFabric2, kKeysetId2, keyset));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, provider->GetKeySet(kFabric2, kKeysetId1, keyset));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, provider->GetKeySet(kFabric2, kKeysetId0, keyset));
}

TEST_F(TestGroupDataProvider, TestIpk)
{
    GroupDataProvider * provider = GetGroupDataProvider();
    EXPECT_TRUE(provider);

    // Reset test
    ResetProvider(provider);

    // Make sure IPK set is not found on a fresh provider
    KeySet ipkOperationalKeySet;
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, provider->GetIpkKeySet(kFabric1, ipkOperationalKeySet));

    // Add a non-IPK key, make sure the IPK set is not found
    EXPECT_EQ(provider->SetKeySet(kFabric1, kCompressedFabricId1, kKeySet3), CHIP_NO_ERROR);
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, provider->GetIpkKeySet(kFabric1, ipkOperationalKeySet));

    const uint8_t kIpkEpochKeyFromSpec[] = { 0x23, 0x5b, 0xf7, 0xe6, 0x28, 0x23, 0xd3, 0x58,
                                             0xdc, 0xa4, 0xba, 0x50, 0xb1, 0x53, 0x5f, 0x4b };

    KeySet fabric1KeySet0(kKeysetId0, SecurityPolicy::kTrustFirst, 1);
    fabric1KeySet0.epoch_keys[0].start_time = 1234;
    memcpy(&fabric1KeySet0.epoch_keys[0].key, &kIpkEpochKeyFromSpec[0], sizeof(kIpkEpochKeyFromSpec));

    // Set a single IPK, validate key derivation follows spec
    EXPECT_EQ(provider->SetKeySet(kFabric1, kCompressedFabricId1, fabric1KeySet0), CHIP_NO_ERROR);
    EXPECT_EQ(provider->GetIpkKeySet(kFabric1, ipkOperationalKeySet), CHIP_NO_ERROR);

    // Make sure the derived key matches spec test vector
    const uint8_t kExpectedIpkFromSpec[] = { 0xa6, 0xf5, 0x30, 0x6b, 0xaf, 0x6d, 0x05, 0x0a,
                                             0xf2, 0x3b, 0xa4, 0xbd, 0x6b, 0x9d, 0xd9, 0x60 };

    EXPECT_EQ(ipkOperationalKeySet.keyset_id, 0u);
    EXPECT_EQ(ipkOperationalKeySet.num_keys_used, 1u);
    EXPECT_EQ(SecurityPolicy::kTrustFirst, ipkOperationalKeySet.policy);
    EXPECT_EQ(ipkOperationalKeySet.epoch_keys[0].start_time, 1234u);
    EXPECT_EQ(memcmp(ipkOperationalKeySet.epoch_keys[0].key, kExpectedIpkFromSpec, sizeof(kExpectedIpkFromSpec)), 0);

    // Remove IPK, verify removal
    EXPECT_EQ(provider->RemoveKeySet(kFabric1, kKeysetId0), CHIP_NO_ERROR);
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, provider->GetIpkKeySet(kFabric1, ipkOperationalKeySet));

    // Set a single IPK with the SetSingleIpkEpochKey helper, validate key derivation follows spec
    EXPECT_EQ(chip::Credentials::SetSingleIpkEpochKey(provider, kFabric1, ByteSpan(kIpkEpochKeyFromSpec), kCompressedFabricId1),
              CHIP_NO_ERROR);
    EXPECT_EQ(provider->GetIpkKeySet(kFabric1, ipkOperationalKeySet), CHIP_NO_ERROR);

    EXPECT_EQ(ipkOperationalKeySet.keyset_id, 0u);
    EXPECT_EQ(ipkOperationalKeySet.num_keys_used, 1u);
    EXPECT_EQ(SecurityPolicy::kTrustFirst, ipkOperationalKeySet.policy);
    EXPECT_EQ(ipkOperationalKeySet.epoch_keys[0].start_time, 0u); // default time is zero for SetSingleIpkEpochKey
    EXPECT_EQ(memcmp(ipkOperationalKeySet.epoch_keys[0].key, kExpectedIpkFromSpec, sizeof(kExpectedIpkFromSpec)), 0);
}

TEST_F(TestGroupDataProvider, TestKeySetIterator)
{
    GroupDataProvider * provider = GetGroupDataProvider();
    EXPECT_TRUE(provider);

    // Reset test
    ResetProvider(provider);

    // Add data to iterate
    EXPECT_EQ(provider->SetKeySet(kFabric1, kCompressedFabricId1, kKeySet1), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetKeySet(kFabric1, kCompressedFabricId1, kKeySet0), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetKeySet(kFabric1, kCompressedFabricId1, kKeySet2), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetKeySet(kFabric1, kCompressedFabricId1, kKeySet3), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetKeySet(kFabric2, kCompressedFabricId2, kKeySet3), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetKeySet(kFabric2, kCompressedFabricId2, kKeySet2), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetKeySet(kFabric2, kCompressedFabricId2, kKeySet1), CHIP_NO_ERROR);

    // Iterate Fabric 1

    KeySet keyset;

    std::map<uint16_t, const KeySet> expected_f1{
        { kKeysetId0, kKeySet0 }, { kKeysetId1, kKeySet1 }, { kKeysetId2, kKeySet2 }, { kKeysetId3, kKeySet3 }
    };

    auto it = provider->IterateKeySets(kFabric1);
    ASSERT_TRUE(it);
    size_t count = 0;
    EXPECT_EQ(expected_f1.size(), it->Count());

    while (it->Next(keyset) && count < expected_f1.size())
    {
        EXPECT_GT(expected_f1.count(keyset.keyset_id), 0u);
        EXPECT_TRUE(CompareKeySets(keyset, expected_f1[keyset.keyset_id]));
        count++;
    }
    EXPECT_EQ(count, expected_f1.size());
    it->Release();

    // Iterate Fabric 2

    std::map<uint16_t, const KeySet> expected_f2{ { kKeysetId1, kKeySet1 }, { kKeysetId2, kKeySet2 }, { kKeysetId3, kKeySet3 } };

    it = provider->IterateKeySets(kFabric2);
    ASSERT_TRUE(it);
    count = 0;
    EXPECT_EQ(expected_f2.size(), it->Count());

    while (it->Next(keyset) && count < expected_f2.size())
    {
        EXPECT_GT(expected_f2.count(keyset.keyset_id), 0u);
        EXPECT_TRUE(CompareKeySets(keyset, expected_f2[keyset.keyset_id]));
        count++;
    }
    EXPECT_EQ(count, expected_f2.size());
    it->Release();
}

TEST_F(TestGroupDataProvider, TestPerFabricData)
{
    GroupDataProvider * provider = GetGroupDataProvider();
    EXPECT_TRUE(provider);

    // Reset test
    ResetProvider(provider);

    // Group Info
    GroupInfo group;

    EXPECT_EQ(provider->SetGroupInfoAt(kFabric1, 0, kGroupInfo1_3), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetGroupInfoAt(kFabric1, 1, kGroupInfo1_2), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetGroupInfoAt(kFabric1, 2, kGroupInfo1_1), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetGroupInfoAt(kFabric2, 0, kGroupInfo2_1), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetGroupInfoAt(kFabric2, 1, kGroupInfo2_3), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetGroupInfoAt(kFabric2, 2, kGroupInfo2_2), CHIP_NO_ERROR);

    EXPECT_EQ(provider->GetGroupInfoAt(kFabric1, 0, group), CHIP_NO_ERROR);
    EXPECT_EQ(group, kGroupInfo1_3);
    EXPECT_EQ(provider->GetGroupInfoAt(kFabric1, 1, group), CHIP_NO_ERROR);
    EXPECT_EQ(group, kGroupInfo1_2);
    EXPECT_EQ(provider->GetGroupInfoAt(kFabric1, 2, group), CHIP_NO_ERROR);
    EXPECT_EQ(group, kGroupInfo1_1);
    EXPECT_EQ(provider->GetGroupInfoAt(kFabric2, 0, group), CHIP_NO_ERROR);
    EXPECT_EQ(group, kGroupInfo2_1);
    EXPECT_EQ(provider->GetGroupInfoAt(kFabric2, 1, group), CHIP_NO_ERROR);
    EXPECT_EQ(group, kGroupInfo2_3);
    EXPECT_EQ(provider->GetGroupInfoAt(kFabric2, 2, group), CHIP_NO_ERROR);
    EXPECT_EQ(group, kGroupInfo2_2);

    // Endpoints

    EXPECT_EQ(provider->AddEndpoint(kFabric1, kGroup1, kEndpointId0), CHIP_NO_ERROR);
    EXPECT_EQ(provider->AddEndpoint(kFabric1, kGroup1, kEndpointId1), CHIP_NO_ERROR);
    EXPECT_EQ(provider->AddEndpoint(kFabric1, kGroup1, kEndpointId2), CHIP_NO_ERROR);
    EXPECT_EQ(provider->AddEndpoint(kFabric1, kGroup2, kEndpointId1), CHIP_NO_ERROR);
    EXPECT_EQ(provider->AddEndpoint(kFabric1, kGroup2, kEndpointId2), CHIP_NO_ERROR);
    EXPECT_EQ(provider->AddEndpoint(kFabric1, kGroup2, kEndpointId3), CHIP_NO_ERROR);
    EXPECT_EQ(provider->AddEndpoint(kFabric2, kGroup3, kEndpointId0), CHIP_NO_ERROR);
    EXPECT_EQ(provider->AddEndpoint(kFabric2, kGroup3, kEndpointId1), CHIP_NO_ERROR);
    EXPECT_EQ(provider->AddEndpoint(kFabric2, kGroup3, kEndpointId2), CHIP_NO_ERROR);
    EXPECT_EQ(provider->AddEndpoint(kFabric2, kGroup3, kEndpointId3), CHIP_NO_ERROR);
    EXPECT_EQ(provider->AddEndpoint(kFabric2, kGroup3, kEndpointId4), CHIP_NO_ERROR);

    EXPECT_TRUE(provider->HasEndpoint(kFabric1, kGroup1, kEndpointId0));
    EXPECT_TRUE(provider->HasEndpoint(kFabric1, kGroup1, kEndpointId1));
    EXPECT_TRUE(provider->HasEndpoint(kFabric1, kGroup1, kEndpointId2));
    EXPECT_TRUE(provider->HasEndpoint(kFabric1, kGroup2, kEndpointId1));
    EXPECT_TRUE(provider->HasEndpoint(kFabric1, kGroup2, kEndpointId2));
    EXPECT_TRUE(provider->HasEndpoint(kFabric1, kGroup2, kEndpointId3));
    EXPECT_TRUE(provider->HasEndpoint(kFabric2, kGroup3, kEndpointId0));
    EXPECT_TRUE(provider->HasEndpoint(kFabric2, kGroup3, kEndpointId1));
    EXPECT_TRUE(provider->HasEndpoint(kFabric2, kGroup3, kEndpointId2));
    EXPECT_TRUE(provider->HasEndpoint(kFabric2, kGroup3, kEndpointId3));
    EXPECT_TRUE(provider->HasEndpoint(kFabric2, kGroup3, kEndpointId4));

    // Keys

    KeySet keyset;

    EXPECT_EQ(provider->SetKeySet(kFabric2, kCompressedFabricId2, kKeySet0), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetKeySet(kFabric1, kCompressedFabricId1, kKeySet2), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetKeySet(kFabric2, kCompressedFabricId2, kKeySet1), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetKeySet(kFabric1, kCompressedFabricId1, kKeySet1), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetKeySet(kFabric2, kCompressedFabricId2, kKeySet2), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetKeySet(kFabric1, kCompressedFabricId1, kKeySet0), CHIP_NO_ERROR);

    EXPECT_EQ(provider->GetKeySet(kFabric2, kKeysetId0, keyset), CHIP_NO_ERROR);
    EXPECT_TRUE(CompareKeySets(keyset, kKeySet0));

    EXPECT_EQ(provider->GetKeySet(kFabric2, kKeysetId1, keyset), CHIP_NO_ERROR);
    EXPECT_TRUE(CompareKeySets(keyset, kKeySet1));

    EXPECT_EQ(provider->GetKeySet(kFabric2, kKeysetId2, keyset), CHIP_NO_ERROR);
    EXPECT_TRUE(CompareKeySets(keyset, kKeySet2));

    EXPECT_EQ(provider->GetKeySet(kFabric1, kKeysetId2, keyset), CHIP_NO_ERROR);
    EXPECT_TRUE(CompareKeySets(keyset, kKeySet2));

    EXPECT_EQ(provider->GetKeySet(kFabric1, kKeysetId1, keyset), CHIP_NO_ERROR);
    EXPECT_TRUE(CompareKeySets(keyset, kKeySet1));

    EXPECT_EQ(provider->GetKeySet(kFabric1, kKeysetId0, keyset), CHIP_NO_ERROR);
    EXPECT_TRUE(CompareKeySets(keyset, kKeySet0));

    //
    // Remove Fabric
    //

    EXPECT_EQ(provider->RemoveFabric(kFabric1), CHIP_NO_ERROR);

    // Endpoints

    EXPECT_FALSE(provider->HasEndpoint(kFabric1, kGroup1, kEndpointId0));
    EXPECT_FALSE(provider->HasEndpoint(kFabric1, kGroup1, kEndpointId1));
    EXPECT_FALSE(provider->HasEndpoint(kFabric1, kGroup1, kEndpointId2));
    EXPECT_FALSE(provider->HasEndpoint(kFabric1, kGroup2, kEndpointId1));
    EXPECT_FALSE(provider->HasEndpoint(kFabric1, kGroup2, kEndpointId2));
    EXPECT_FALSE(provider->HasEndpoint(kFabric1, kGroup2, kEndpointId3));
    EXPECT_TRUE(provider->HasEndpoint(kFabric2, kGroup3, kEndpointId0));
    EXPECT_TRUE(provider->HasEndpoint(kFabric2, kGroup3, kEndpointId1));
    EXPECT_TRUE(provider->HasEndpoint(kFabric2, kGroup3, kEndpointId2));
    EXPECT_TRUE(provider->HasEndpoint(kFabric2, kGroup3, kEndpointId3));
    EXPECT_TRUE(provider->HasEndpoint(kFabric2, kGroup3, kEndpointId4));

    // Group Info

    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, provider->GetGroupInfoAt(kFabric1, 0, group));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, provider->GetGroupInfoAt(kFabric1, 1, group));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, provider->GetGroupInfoAt(kFabric1, 2, group));
    EXPECT_EQ(provider->GetGroupInfoAt(kFabric2, 0, group), CHIP_NO_ERROR);
    EXPECT_EQ(provider->GetGroupInfoAt(kFabric2, 1, group), CHIP_NO_ERROR);
    EXPECT_EQ(provider->GetGroupInfoAt(kFabric2, 2, group), CHIP_NO_ERROR);

    // Keys

    EXPECT_EQ(provider->GetKeySet(kFabric2, kKeysetId1, keyset), CHIP_NO_ERROR);
    EXPECT_TRUE(CompareKeySets(keyset, kKeySet1));

    EXPECT_EQ(provider->GetKeySet(kFabric2, kKeysetId0, keyset), CHIP_NO_ERROR);
    EXPECT_TRUE(CompareKeySets(keyset, kKeySet0));

    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, provider->GetKeySet(kFabric1, 202, keyset));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, provider->GetKeySet(kFabric1, 404, keyset));
    EXPECT_EQ(CHIP_ERROR_NOT_FOUND, provider->GetKeySet(kFabric1, 606, keyset));
}

TEST_F(TestGroupDataProvider, TestGroupDecryption)
{
    GroupDataProvider * provider = GetGroupDataProvider();
    EXPECT_TRUE(provider);

    // Reset test
    ResetProvider(provider);

    EXPECT_EQ(provider->SetGroupInfoAt(kFabric1, 0, kGroupInfo1_3), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetGroupInfoAt(kFabric1, 1, kGroupInfo1_2), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetGroupInfoAt(kFabric1, 2, kGroupInfo1_1), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetGroupInfoAt(kFabric2, 0, kGroupInfo2_1), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetGroupInfoAt(kFabric2, 1, kGroupInfo2_3), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetGroupInfoAt(kFabric2, 2, kGroupInfo2_2), CHIP_NO_ERROR);

    EXPECT_EQ(provider->AddEndpoint(kFabric1, kGroup1, kEndpointId0), CHIP_NO_ERROR);
    EXPECT_EQ(provider->AddEndpoint(kFabric1, kGroup1, kEndpointId2), CHIP_NO_ERROR);
    EXPECT_EQ(provider->AddEndpoint(kFabric1, kGroup1, kEndpointId4), CHIP_NO_ERROR);
    EXPECT_EQ(provider->AddEndpoint(kFabric1, kGroup2, kEndpointId1), CHIP_NO_ERROR);
    EXPECT_EQ(provider->AddEndpoint(kFabric1, kGroup2, kEndpointId2), CHIP_NO_ERROR);
    EXPECT_EQ(provider->AddEndpoint(kFabric1, kGroup2, kEndpointId3), CHIP_NO_ERROR);

    EXPECT_EQ(provider->AddEndpoint(kFabric2, kGroup3, kEndpointId0), CHIP_NO_ERROR);
    EXPECT_EQ(provider->AddEndpoint(kFabric2, kGroup3, kEndpointId1), CHIP_NO_ERROR);
    EXPECT_EQ(provider->AddEndpoint(kFabric2, kGroup3, kEndpointId2), CHIP_NO_ERROR);
    EXPECT_EQ(provider->AddEndpoint(kFabric2, kGroup3, kEndpointId3), CHIP_NO_ERROR);
    EXPECT_EQ(provider->AddEndpoint(kFabric2, kGroup3, kEndpointId4), CHIP_NO_ERROR);

    EXPECT_EQ(provider->SetKeySet(kFabric1, kCompressedFabricId1, kKeySet0), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetKeySet(kFabric1, kCompressedFabricId1, kKeySet2), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetKeySet(kFabric2, kCompressedFabricId2, kKeySet1), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetKeySet(kFabric2, kCompressedFabricId2, kKeySet3), CHIP_NO_ERROR);

    EXPECT_EQ(provider->SetGroupKeyAt(kFabric1, 0, kGroup1Keyset0), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetGroupKeyAt(kFabric1, 1, kGroup1Keyset2), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetGroupKeyAt(kFabric1, 2, kGroup3Keyset0), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetGroupKeyAt(kFabric1, 3, kGroup3Keyset2), CHIP_NO_ERROR);

    EXPECT_EQ(provider->SetGroupKeyAt(kFabric2, 0, kGroup2Keyset1), CHIP_NO_ERROR);
    EXPECT_EQ(provider->SetGroupKeyAt(kFabric2, 1, kGroup2Keyset3), CHIP_NO_ERROR);

    const size_t kMessageLength            = 10;
    const uint8_t kMessage[kMessageLength] = { 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9 };
    const uint8_t nonce[13]                = { 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x18, 0x1a, 0x1b, 0x1c };
    const uint8_t aad[40]                  = { 0x0a, 0x1a, 0x2a, 0x3a, 0x4a, 0x5a, 0x6a, 0x7a, 0x8a, 0x9a, 0x0b, 0x1b, 0x2b, 0x3b,
                                               0x4b, 0x5b, 0x6b, 0x7b, 0x8b, 0x9b, 0x0c, 0x1c, 0x2c, 0x3c, 0x4c, 0x5c, 0x6c, 0x7c,
                                               0x8c, 0x9c, 0x0d, 0x1d, 0x2d, 0x3d, 0x4d, 0x5d, 0x6d, 0x7d, 0x8d, 0x9d };
    uint8_t mic[16]                        = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };

    uint8_t ciphertext_buffer[kMessageLength];
    uint8_t plaintext_buffer[kMessageLength];
    MutableByteSpan ciphertext(ciphertext_buffer, sizeof(ciphertext_buffer));
    MutableByteSpan plaintext(plaintext_buffer, sizeof(plaintext_buffer));
    MutableByteSpan tag(mic, sizeof(mic));

    //
    // Encrypt
    //

    // Load the plaintext to encrypt
    memcpy(plaintext_buffer, kMessage, sizeof(kMessage));

    // Get the key context
    Crypto::SymmetricKeyContext * key_context = provider->GetKeyContext(kFabric2, kGroup2);
    ASSERT_NE(nullptr, key_context);
    uint16_t session_id = key_context->GetKeyHash();

    // Encrypt the message
    EXPECT_EQ(key_context->MessageEncrypt(plaintext, ByteSpan(aad, sizeof(aad)), ByteSpan(nonce, sizeof(nonce)), tag, ciphertext),
              CHIP_NO_ERROR);

    // The ciphertext must be different to the original message
    EXPECT_TRUE(memcmp(ciphertext.data(), kMessage, sizeof(kMessage)));
    key_context->Release();

    //
    // Decrypt
    //

    const std::set<std::pair<FabricIndex, GroupId>> expected = { { kFabric2, kGroup2 } };

    // Iterate all keys that matches the incoming session
    GroupSession session;
    auto it      = provider->IterateGroupSessions(session_id);
    size_t count = 0, total = 0;

    ASSERT_TRUE(it);
    total = it->Count();
    EXPECT_EQ(expected.size(), total);
    while (it->Next(session))
    {
        std::pair<FabricIndex, GroupId> found(session.fabric_index, session.group_id);
        EXPECT_GT(expected.count(found), 0u);
        ASSERT_NE(session.keyContext, nullptr);
        // Assert aboves doesn't actually exit, we call continue so that we can call it->Release() outside of
        // loop.
        if (session.keyContext == nullptr)
        {
            continue;
        }

        // Decrypt the ciphertext
        EXPECT_EQ(session.keyContext->MessageDecrypt(ciphertext, ByteSpan(aad, sizeof(aad)), ByteSpan(nonce, sizeof(nonce)), tag,
                                                     plaintext),
                  CHIP_NO_ERROR);

        // The new plaintext must match the original message
        EXPECT_EQ(memcmp(plaintext.data(), kMessage, sizeof(kMessage)), 0);
        count++;
    }
    EXPECT_EQ(count, total);
    it->Release();
}

} // namespace TestGroups
} // namespace app
} // namespace chip

namespace {} // namespace
