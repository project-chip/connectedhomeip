/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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

#include <array>
#include <cinttypes>
#include <cstdint>
#include <cstring>
#include <string>

#include <pw_unit_test/framework.h>

#include <lib/core/CHIPKeyIds.h>

using namespace chip;

using KeyID = uint32_t;

constexpr uint32_t kFlag_UseCurrentEpochKey = 0x80000000;

TEST(TestCHIPKeyIds, TestGroupKeyTypes)
{
    KeyID static_key = ChipKeyId::kType_AppStaticKey | 0x0FFF;

    ASSERT_TRUE(ChipKeyId::IsAppGroupKey(static_key));

    KeyID rotating_key = ChipKeyId::kType_AppRotatingKey | 0x0FFF;

    ASSERT_TRUE(ChipKeyId::IsAppGroupKey(rotating_key));
}

TEST(TestCHIPKeyIds, UsesCurrentEpochKey)
{
    KeyID key = 0x0FFF5FFF;

    // assert that the key incorporates epoch key
    ASSERT_TRUE(ChipKeyId::IncorporatesEpochKey(key));

    // assert that the key doesn't use current epoch key
    ASSERT_FALSE(ChipKeyId::UsesCurrentEpochKey(key));

    KeyID current_epoch_key = kFlag_UseCurrentEpochKey | 0x0FFF5FFF; // Test key part 0x0FFF5FFF with current epoch flag

    // assert that the key incorporates epoch key
    ASSERT_TRUE(ChipKeyId::IncorporatesEpochKey(current_epoch_key));

    // assert that the key uses current epoch key
    ASSERT_TRUE(ChipKeyId::UsesCurrentEpochKey(current_epoch_key));
}

TEST(TestCHIPKeyIds, IncorporatesRootKey)
{
    KeyID static_key = 0x00004FFF;

    ASSERT_TRUE(ChipKeyId::IncorporatesRootKey(static_key));

    KeyID rotating_key = 0x00005FFF;

    ASSERT_TRUE(ChipKeyId::IncorporatesRootKey(rotating_key));

    KeyID root_key = 0x00010FFF;

    ASSERT_TRUE(ChipKeyId::IncorporatesRootKey(root_key));

    KeyID intermediate_key = 0x00011FFF;

    ASSERT_TRUE(ChipKeyId::IncorporatesRootKey(intermediate_key));

    KeyID non_root_key = 0xFFF00FFF;

    ASSERT_FALSE(ChipKeyId::IncorporatesRootKey(non_root_key));
}

TEST(TestCHIPKeyIds, IncorporatesAppGroupMasterKey)
{
    KeyID static_key = 0x00004FFF;

    ASSERT_TRUE(ChipKeyId::IncorporatesAppGroupMasterKey(static_key));

    KeyID rotating_key = 0x00005FFF;

    ASSERT_TRUE(ChipKeyId::IncorporatesAppGroupMasterKey(rotating_key));

    KeyID group_master_key = 0x00030FFF;

    ASSERT_TRUE(ChipKeyId::IncorporatesAppGroupMasterKey(group_master_key));

    KeyID non_group_master_key = 0xFFF00FFF;

    ASSERT_FALSE(ChipKeyId::IncorporatesAppGroupMasterKey(non_group_master_key));
}

TEST(TestCHIPKeyIds, IsAppGroupKey)
{
    KeyID static_key = 0x00004FFF;

    ASSERT_TRUE(ChipKeyId::IsAppGroupKey(static_key));

    KeyID rotating_key = 0x00005FFF;

    ASSERT_TRUE(ChipKeyId::IsAppGroupKey(rotating_key));

    KeyID non_group_key = 0xFFF00FFF;

    ASSERT_FALSE(ChipKeyId::IsAppGroupKey(non_group_key));
}

TEST(TestCHIPKeyIds, MakesAppKeyId)
{
    KeyID key = ChipKeyId::kType_General;

    ASSERT_FALSE(ChipKeyId::UsesCurrentEpochKey(key));
    ASSERT_EQ(ChipKeyId::GetRootKeyId(key), ChipKeyId::kType_AppRootKey);
    ASSERT_EQ(ChipKeyId::GetEpochKeyId(key), ChipKeyId::kType_AppEpochKey);
    ASSERT_EQ(ChipKeyId::ConvertToCurrentAppKeyId(key),
              kFlag_UseCurrentEpochKey | ChipKeyId::kType_General); // (keyId & ~kMask_EpochKeyNumber) | kFlag_UseCurrentEpochKey

    key = ChipKeyId::MakeAppKeyId(key, ChipKeyId::kServiceRootKey, 0x00000380, ChipKeyId::kNone, true);

    ASSERT_TRUE(ChipKeyId::UsesCurrentEpochKey(key));
    ASSERT_EQ(ChipKeyId::GetRootKeyNumber(key), uint8_t{ 2 });
    ASSERT_EQ(ChipKeyId::GetEpochKeyNumber(key), uint8_t{ 0 });
    ASSERT_EQ(ChipKeyId::ConvertToCurrentAppKeyId(key), key);
}

TEST(TestCHIPKeyIds, MakesAppIntermediateKeyId)
{
    KeyID key = ChipKeyId::kType_General;

    ASSERT_FALSE(ChipKeyId::UsesCurrentEpochKey(key));
    ASSERT_EQ(ChipKeyId::GetRootKeyId(key), ChipKeyId::kType_AppRootKey);
    ASSERT_EQ(ChipKeyId::GetEpochKeyId(key), ChipKeyId::kType_AppEpochKey);
    ASSERT_EQ(ChipKeyId::ConvertToCurrentAppKeyId(key), 0x80001000); // (keyId & ~kMask_EpochKeyNumber) | kFlag_UseCurrentEpochKey

    key = ChipKeyId::MakeAppIntermediateKeyId(ChipKeyId::kServiceRootKey, 0x00000380, ChipKeyId::kNone);

    ASSERT_FALSE(ChipKeyId::UsesCurrentEpochKey(key));
    ASSERT_EQ(ChipKeyId::GetRootKeyNumber(key), uint8_t{ 2 });
    ASSERT_EQ(ChipKeyId::GetEpochKeyNumber(key), uint8_t{ 7 });
    ASSERT_EQ(ChipKeyId::ConvertToCurrentAppKeyId(key),
              kFlag_UseCurrentEpochKey | (ChipKeyId::kType_AppIntermediateKey | (static_cast<uint32_t>(2) << 10)));
    ASSERT_EQ(ChipKeyId::GetType(key), ChipKeyId::kType_AppIntermediateKey);
}

TEST(TestCHIPKeyIds, MakesAppRotatingKeyId)
{
    KeyID key = ChipKeyId::kType_General;

    ASSERT_FALSE(ChipKeyId::UsesCurrentEpochKey(key));
    ASSERT_EQ(ChipKeyId::GetRootKeyId(key), ChipKeyId::kType_AppRootKey);
    ASSERT_EQ(ChipKeyId::GetEpochKeyId(key), ChipKeyId::kType_AppEpochKey);
    ASSERT_EQ(ChipKeyId::ConvertToCurrentAppKeyId(key), 0x80001000); // (keyId & ~kMask_EpochKeyNumber) | kFlag_UseCurrentEpochKey
    ASSERT_EQ(ChipKeyId::GetType(key), ChipKeyId::kType_General);

    key = ChipKeyId::MakeAppRotatingKeyId(ChipKeyId::kServiceRootKey, 0x00000380, ChipKeyId::kNone, true);

    ASSERT_TRUE(ChipKeyId::UsesCurrentEpochKey(key));
    ASSERT_EQ(ChipKeyId::GetRootKeyNumber(key), uint8_t{ 2 });
    ASSERT_EQ(ChipKeyId::GetEpochKeyNumber(key), uint8_t{ 0 });
    ASSERT_EQ(ChipKeyId::ConvertToCurrentAppKeyId(key), key);
    ASSERT_EQ(ChipKeyId::GetType(key), ChipKeyId::kType_AppRotatingKey);
}

TEST(TestCHIPKeyIds, MakesAppStaticKeyId)
{
    KeyID key = ChipKeyId::kType_General;

    ASSERT_FALSE(ChipKeyId::UsesCurrentEpochKey(key));
    ASSERT_EQ(ChipKeyId::GetRootKeyId(key), ChipKeyId::kType_AppRootKey);
    ASSERT_EQ(ChipKeyId::GetEpochKeyId(key), ChipKeyId::kType_AppEpochKey);
    ASSERT_EQ(ChipKeyId::ConvertToCurrentAppKeyId(key), 0x80001000); // (keyId & ~kMask_EpochKeyNumber) | kFlag_UseCurrentEpochKey
    ASSERT_EQ(ChipKeyId::GetType(key), ChipKeyId::kType_General);

    key = ChipKeyId::MakeAppStaticKeyId(ChipKeyId::kServiceRootKey, 0x00000380);

    ASSERT_FALSE(ChipKeyId::UsesCurrentEpochKey(key));
    ASSERT_EQ(ChipKeyId::GetRootKeyNumber(key), uint8_t{ 2 });
    ASSERT_EQ(ChipKeyId::GetEpochKeyNumber(key), uint8_t{ 0 });
    ASSERT_EQ(ChipKeyId::ConvertToCurrentAppKeyId(key),
              kFlag_UseCurrentEpochKey | (ChipKeyId::kType_AppStaticKey | (static_cast<uint32_t>(2) << 10)));
    ASSERT_EQ(ChipKeyId::GetType(key), ChipKeyId::kType_AppStaticKey);
}

TEST(TestCHIPKeyIds, ConvertsToStaticKeyId)
{
    KeyID key = ChipKeyId::kType_AppRotatingKey;

    ASSERT_FALSE(ChipKeyId::UsesCurrentEpochKey(key));

    key = ChipKeyId::ConvertToStaticAppKeyId(key);

    ASSERT_FALSE(ChipKeyId::UsesCurrentEpochKey(key));
}

TEST(TestCHIPKeyIds, IsValidKeyId)
{
    KeyID key = ChipKeyId::kType_General;

    ASSERT_TRUE(ChipKeyId::IsValidKeyId(key));

    key = ChipKeyId::kType_None;

    ASSERT_FALSE(ChipKeyId::IsValidKeyId(key));

    key = ChipKeyId::kType_AppStaticKey;

    ASSERT_TRUE(ChipKeyId::IsValidKeyId(key));

    key = ChipKeyId::kType_AppRotatingKey;

    ASSERT_TRUE(ChipKeyId::IsValidKeyId(key));

    key = ChipKeyId::kType_AppRootKey;

    ASSERT_TRUE(ChipKeyId::IsValidKeyId(key));

    key = ChipKeyId::kType_AppIntermediateKey;

    ASSERT_TRUE(ChipKeyId::IsValidKeyId(key));

    key = ChipKeyId::kType_AppEpochKey;

    ASSERT_TRUE(ChipKeyId::IsValidKeyId(key));

    key = ChipKeyId::kType_AppGroupMasterKey;

    ASSERT_TRUE(ChipKeyId::IsValidKeyId(key));

    key = 0xFFFFFFFF; // Invalid key ID

    ASSERT_FALSE(ChipKeyId::IsValidKeyId(key));
}

TEST(TestCHIPKeyIds, DetectsSameKeyOrGroup)
{
    KeyID key1 = ChipKeyId::kType_General | 0x00000001;
    KeyID key2 = ChipKeyId::kType_General | 0x00000001;
    KeyID key3 = ChipKeyId::kType_General | 0x00000002;

    ASSERT_TRUE(ChipKeyId::IsSameKeyOrGroup(key1, key2));
    ASSERT_FALSE(ChipKeyId::IsSameKeyOrGroup(key1, key3));
    ASSERT_TRUE(ChipKeyId::IsSameKeyOrGroup(key1, key1));                           // same key
    ASSERT_FALSE(ChipKeyId::IsSameKeyOrGroup(key2, key3));                          // different keys
    ASSERT_FALSE(ChipKeyId::IsSameKeyOrGroup(key1, ChipKeyId::kType_AppStaticKey)); // different key types
}

TEST(TestCHIPKeyIds, DetectsMessageSessionId)
{
    ASSERT_TRUE(ChipKeyId::IsMessageSessionId(ChipKeyId::kType_Session, false));
    ASSERT_TRUE(ChipKeyId::IsMessageSessionId(ChipKeyId::kType_Session, true));
    ASSERT_FALSE(ChipKeyId::IsMessageSessionId(ChipKeyId::kType_General, false));
    ASSERT_TRUE(ChipKeyId::IsMessageSessionId(ChipKeyId::kType_AppStaticKey, false));
    ASSERT_TRUE(ChipKeyId::IsMessageSessionId(ChipKeyId::kType_AppStaticKey, true));
    ASSERT_TRUE(ChipKeyId::IsMessageSessionId(ChipKeyId::kType_AppRotatingKey, false));
    ASSERT_TRUE(ChipKeyId::IsMessageSessionId(ChipKeyId::kType_AppRotatingKey, true));
    ASSERT_FALSE(ChipKeyId::IsMessageSessionId(ChipKeyId::kType_AppRootKey, false));
    ASSERT_FALSE(ChipKeyId::IsMessageSessionId(ChipKeyId::kType_AppRootKey, true));
    ASSERT_FALSE(ChipKeyId::IsMessageSessionId(ChipKeyId::kType_AppEpochKey, false));
    ASSERT_FALSE(ChipKeyId::IsMessageSessionId(ChipKeyId::kType_AppEpochKey, true));
    ASSERT_FALSE(ChipKeyId::IsMessageSessionId(ChipKeyId::kType_General, true));
}

TEST(TestCHIPKeyIds, DescribesKey)
{
    ASSERT_STREQ(ChipKeyId::DescribeKey(ChipKeyId::kType_General), "Other General Key");
    ASSERT_STREQ(ChipKeyId::DescribeKey(ChipKeyId::kFabricSecret), "Fabric Secret");
    ASSERT_STREQ(ChipKeyId::DescribeKey(ChipKeyId::kFabricRootKey), "Fabric Root Key");
    ASSERT_STREQ(ChipKeyId::DescribeKey(ChipKeyId::kClientRootKey), "Client Root Key");
    ASSERT_STREQ(ChipKeyId::DescribeKey(ChipKeyId::kServiceRootKey), "Service Root Key");
    ASSERT_STREQ(ChipKeyId::DescribeKey(ChipKeyId::kType_AppStaticKey), "Application Static Key");
    ASSERT_STREQ(ChipKeyId::DescribeKey(ChipKeyId::kType_AppRotatingKey), "Application Rotating Key");
    ASSERT_STREQ(ChipKeyId::DescribeKey(ChipKeyId::kType_AppRootKey), "Fabric Root Key");
    ASSERT_STREQ(ChipKeyId::DescribeKey(ChipKeyId::kType_AppIntermediateKey), "Application Intermediate Key");
    ASSERT_STREQ(ChipKeyId::DescribeKey(ChipKeyId::kType_AppEpochKey), "Application Epoch Key");
    ASSERT_STREQ(ChipKeyId::DescribeKey(ChipKeyId::kType_AppGroupMasterKey), "Application Group Master Key");
    ASSERT_STREQ(ChipKeyId::DescribeKey(ChipKeyId::kType_None), "No Key");
    ASSERT_STREQ(ChipKeyId::DescribeKey(ChipKeyId::kType_Session), "Session Key");
    ASSERT_STREQ(ChipKeyId::DescribeKey(0xFFFFFFFF), "Unknown Key Type");
}

TEST(TestCHIPKeyIds, GetType)
{
    KeyID key = ChipKeyId::kType_General;

    ASSERT_EQ(ChipKeyId::GetType(key), ChipKeyId::kType_General);

    key = ChipKeyId::kType_AppStaticKey;

    ASSERT_EQ(ChipKeyId::GetType(key), ChipKeyId::kType_AppStaticKey);

    key = ChipKeyId::kType_AppRotatingKey;

    ASSERT_EQ(ChipKeyId::GetType(key), ChipKeyId::kType_AppRotatingKey);

    key = ChipKeyId::kType_AppRootKey;

    ASSERT_EQ(ChipKeyId::GetType(key), ChipKeyId::kType_AppRootKey);
}

TEST(TestCHIPKeyIds, UpdateEpochKeyId)
{
    // 10 bits shift for root key number, 7 bits shift for epoch key number
    KeyID key = ChipKeyId::kType_AppRotatingKey | (1 << 10) | (2 << 7);

    ASSERT_EQ(key, uint32_t{ 0x00005500 });
    ASSERT_EQ(ChipKeyId::UpdateEpochKeyId(key, 0), uint32_t{ 0x00005400 });
}
