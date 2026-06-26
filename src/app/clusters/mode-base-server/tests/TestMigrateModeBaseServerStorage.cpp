/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <pw_unit_test/framework.h>

#include <app/DefaultSafeAttributePersistenceProvider.h>
#include <app/clusters/mode-base-server/MigrateModeBaseServerStorage.h>
#include <app/clusters/mode-base-server/mode-base-cluster-objects.h>
#include <app/data-model/Nullable.h>
#include <app/persistence/AttributePersistence.h>
#include <app/persistence/DefaultAttributePersistenceProvider.h>
#include <clusters/DishwasherMode/Metadata.h>
#include <lib/support/TestPersistentStorageDelegate.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ModeBase;
using namespace chip::app::Clusters::ModeBase::Attributes;

constexpr EndpointId kEndpointId = 1;
constexpr ClusterId kClusterId   = DishwasherMode::Id;

ConcreteAttributePath CurrentModePath()
{
    return { kEndpointId, kClusterId, CurrentMode::Id };
}
ConcreteAttributePath StartUpModePath()
{
    return { kEndpointId, kClusterId, StartUpMode::Id };
}
ConcreteAttributePath OnModePath()
{
    return { kEndpointId, kClusterId, OnMode::Id };
}

void ExpectSafeProviderHasNoValue(DefaultSafeAttributePersistenceProvider & safeProvider, const ConcreteAttributePath & path)
{
    uint8_t readBuf[sizeof(NumericAttributeTraits<uint8_t>::StorageType)] = {};
    MutableByteSpan readSpan(readBuf);
    EXPECT_EQ(safeProvider.SafeReadValue(path, readSpan), CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
}

void ExpectDstProviderHasNoValue(DefaultAttributePersistenceProvider & dstProvider, const ConcreteAttributePath & path)
{
    uint8_t readBuf[sizeof(NumericAttributeTraits<uint8_t>::StorageType)] = {};
    MutableByteSpan readSpan(readBuf);
    EXPECT_EQ(dstProvider.ReadValue(path, readSpan), CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
}

void ExpectDstCurrentMode(DefaultAttributePersistenceProvider & dstProvider, uint8_t expectedValue)
{
    AttributePersistence persistence(dstProvider);
    uint8_t currentMode = 0;
    EXPECT_TRUE(persistence.LoadNativeEndianValue(CurrentModePath(), currentMode, static_cast<uint8_t>(0)));
    EXPECT_EQ(currentMode, expectedValue);
}

void ExpectDstNullableUint8(DefaultAttributePersistenceProvider & dstProvider, const ConcreteAttributePath & path,
                            const DataModel::Nullable<uint8_t> & expectedValue)
{
    AttributePersistence persistence(dstProvider);
    DataModel::Nullable<uint8_t> loadedValue;
    DataModel::Nullable<uint8_t> defaultValue;
    defaultValue.SetNull();
    EXPECT_TRUE(persistence.LoadNativeEndianValue(path, loadedValue, defaultValue));
    EXPECT_EQ(loadedValue, expectedValue);
}

struct MigrationTestFixture : public ::testing::Test
{
    void SetUp() override
    {
        storageDelegate.ClearStorage();
        ASSERT_EQ(dstProvider.Init(&storageDelegate), CHIP_NO_ERROR);
        ASSERT_EQ(safeProvider.Init(&storageDelegate), CHIP_NO_ERROR);
    }

    TestPersistentStorageDelegate storageDelegate;
    DefaultAttributePersistenceProvider dstProvider;
    DefaultSafeAttributePersistenceProvider safeProvider;
};

TEST_F(MigrationTestFixture, MigratesAllPersistentAttributes)
{
    constexpr uint8_t kCurrentMode = 2;
    const DataModel::Nullable<uint8_t> kStartUpMode(1);
    const DataModel::Nullable<uint8_t> kOnMode(0);

    ASSERT_EQ(safeProvider.WriteScalarValue(CurrentModePath(), kCurrentMode), CHIP_NO_ERROR);
    ASSERT_EQ(safeProvider.WriteScalarValue(StartUpModePath(), kStartUpMode), CHIP_NO_ERROR);
    ASSERT_EQ(safeProvider.WriteScalarValue(OnModePath(), kOnMode), CHIP_NO_ERROR);

    ASSERT_EQ(MigrateModeBaseServerStorage(kEndpointId, kClusterId, safeProvider, dstProvider), CHIP_NO_ERROR);

    ExpectDstCurrentMode(dstProvider, kCurrentMode);
    ExpectDstNullableUint8(dstProvider, StartUpModePath(), kStartUpMode);
    ExpectDstNullableUint8(dstProvider, OnModePath(), kOnMode);

    ExpectSafeProviderHasNoValue(safeProvider, CurrentModePath());
    ExpectSafeProviderHasNoValue(safeProvider, StartUpModePath());
    ExpectSafeProviderHasNoValue(safeProvider, OnModePath());
}

TEST_F(MigrationTestFixture, MigratesNullableNullValues)
{
    constexpr uint8_t kCurrentMode = 1;
    DataModel::Nullable<uint8_t> nullStartUpMode;
    DataModel::Nullable<uint8_t> nullOnMode;
    nullStartUpMode.SetNull();
    nullOnMode.SetNull();

    ASSERT_EQ(safeProvider.WriteScalarValue(CurrentModePath(), kCurrentMode), CHIP_NO_ERROR);
    ASSERT_EQ(safeProvider.WriteScalarValue(StartUpModePath(), nullStartUpMode), CHIP_NO_ERROR);
    ASSERT_EQ(safeProvider.WriteScalarValue(OnModePath(), nullOnMode), CHIP_NO_ERROR);

    ASSERT_EQ(MigrateModeBaseServerStorage(kEndpointId, kClusterId, safeProvider, dstProvider), CHIP_NO_ERROR);

    ExpectDstCurrentMode(dstProvider, kCurrentMode);
    ExpectDstNullableUint8(dstProvider, StartUpModePath(), nullStartUpMode);
    ExpectDstNullableUint8(dstProvider, OnModePath(), nullOnMode);

    ExpectSafeProviderHasNoValue(safeProvider, StartUpModePath());
    ExpectSafeProviderHasNoValue(safeProvider, OnModePath());
}

TEST_F(MigrationTestFixture, SkipsAbsentNullableAttributes)
{
    constexpr uint8_t kCurrentMode = 1;

    ASSERT_EQ(safeProvider.WriteScalarValue(CurrentModePath(), kCurrentMode), CHIP_NO_ERROR);

    ASSERT_EQ(MigrateModeBaseServerStorage(kEndpointId, kClusterId, safeProvider, dstProvider), CHIP_NO_ERROR);

    ExpectDstCurrentMode(dstProvider, kCurrentMode);
    ExpectDstProviderHasNoValue(dstProvider, StartUpModePath());
    ExpectDstProviderHasNoValue(dstProvider, OnModePath());

    ExpectSafeProviderHasNoValue(safeProvider, CurrentModePath());
}

TEST_F(MigrationTestFixture, SkipsAttributesAlreadyPresentInDestinationProvider)
{
    constexpr uint8_t kSafeCurrentMode = 2;
    constexpr uint8_t kDstCurrentMode  = 5;
    const DataModel::Nullable<uint8_t> kSafeStartUpMode(1);
    const DataModel::Nullable<uint8_t> kDstStartUpMode(3);

    ASSERT_EQ(safeProvider.WriteScalarValue(CurrentModePath(), kSafeCurrentMode), CHIP_NO_ERROR);
    ASSERT_EQ(safeProvider.WriteScalarValue(StartUpModePath(), kSafeStartUpMode), CHIP_NO_ERROR);

    ASSERT_EQ(dstProvider.WriteValue(CurrentModePath(),
                                     ByteSpan(reinterpret_cast<const uint8_t *>(&kDstCurrentMode), sizeof(kDstCurrentMode))),
              CHIP_NO_ERROR);
    NumericAttributeTraits<uint8_t>::StorageType startUpStorageValue;
    DataModel::NullableToStorage(kDstStartUpMode, startUpStorageValue);
    ASSERT_EQ(
        dstProvider.WriteValue(StartUpModePath(),
                               ByteSpan(reinterpret_cast<const uint8_t *>(&startUpStorageValue), sizeof(startUpStorageValue))),
        CHIP_NO_ERROR);

    ASSERT_EQ(MigrateModeBaseServerStorage(kEndpointId, kClusterId, safeProvider, dstProvider), CHIP_NO_ERROR);

    // Destination values must be left untouched.
    ExpectDstCurrentMode(dstProvider, kDstCurrentMode);
    ExpectDstNullableUint8(dstProvider, StartUpModePath(), kDstStartUpMode);

    // Safe values that were skipped remain in the safe provider.
    uint8_t safeCurrentMode = 0;
    ASSERT_EQ(safeProvider.ReadScalarValue(CurrentModePath(), safeCurrentMode), CHIP_NO_ERROR);
    EXPECT_EQ(safeCurrentMode, kSafeCurrentMode);

    DataModel::Nullable<uint8_t> safeStartUpMode;
    ASSERT_EQ(safeProvider.ReadScalarValue(StartUpModePath(), safeStartUpMode), CHIP_NO_ERROR);
    EXPECT_EQ(safeStartUpMode, kSafeStartUpMode);
}

} // namespace
