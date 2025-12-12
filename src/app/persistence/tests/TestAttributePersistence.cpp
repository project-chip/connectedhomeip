/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/AttributeValueDecoder.h>
#include <app/ConcreteAttributePath.h>
#include <app/data-model-provider/tests/WriteTesting.h>
#include <app/persistence/AttributePersistence.h>
#include <app/persistence/DefaultAttributePersistenceProvider.h>
#include <app/persistence/String.h>
#include <clusters/TimeFormatLocalization/Enums.h>
#include <clusters/TimeFormatLocalization/EnumsCheck.h>
#include <lib/core/CHIPError.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/Span.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <unistd.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Testing;

TEST(TestAttributePersistence, TestLoadAndDecodeAndStoreNativeEndian)
{
    TestPersistentStorageDelegate storageDelegate;
    DefaultAttributePersistenceProvider ramProvider;
    ASSERT_EQ(ramProvider.Init(&storageDelegate), CHIP_NO_ERROR);

    AttributePersistence persistence(ramProvider);

    const ConcreteAttributePath path(1, 2, 3);
    const ConcreteAttributePath wrongPath(1, 2, 4);
    constexpr uint32_t kValueToStore = 42;
    constexpr uint32_t kOtherValue   = 99;

    // Store a fake value
    {
        const uint32_t value = kValueToStore;
        EXPECT_EQ(storageDelegate.SyncSetKeyValue(
                      DefaultStorageKeyAllocator::AttributeValue(path.mEndpointId, path.mClusterId, path.mAttributeId).KeyName(),
                      &value, sizeof(value)),
                  CHIP_NO_ERROR);
    }

    // Test loading a value
    {
        uint32_t valueRead = 0;

        ASSERT_TRUE(persistence.LoadNativeEndianValue(path, valueRead, kOtherValue));
        ASSERT_EQ(valueRead, kValueToStore);
    }

    // Test loading a non-existent value
    {
        uint32_t valueRead = 0;

        ASSERT_FALSE(persistence.LoadNativeEndianValue(wrongPath, valueRead, kOtherValue));
        ASSERT_EQ(valueRead, kOtherValue);
    }

    // Test loading a removed value
    {
        EXPECT_EQ(storageDelegate.SyncDeleteKeyValue(
                      DefaultStorageKeyAllocator::AttributeValue(path.mEndpointId, path.mClusterId, path.mAttributeId).KeyName()),
                  CHIP_NO_ERROR);

        uint32_t valueRead = 0;
        ASSERT_FALSE(persistence.LoadNativeEndianValue(path, valueRead, kOtherValue));
        ASSERT_EQ(valueRead, kOtherValue);
    }
}

TEST(TestAttributePersistence, TestNativeRawValueViaDecoder)
{
    TestPersistentStorageDelegate storageDelegate;
    DefaultAttributePersistenceProvider ramProvider;
    ASSERT_EQ(ramProvider.Init(&storageDelegate), CHIP_NO_ERROR);

    AttributePersistence persistence(ramProvider);

    const ConcreteAttributePath path(1, 2, 3);
    const ConcreteAttributePath wrongPath(1, 2, 4);
    constexpr uint32_t kValueToStore = 0x12345678;
    constexpr uint32_t kOtherValue   = 0x99887766;
    uint32_t valueRead               = 0;

    // Store a value using an encoder (these are a PAIN to create, so use data model provider helpers)
    {
        WriteOperation writeOp(path);
        AttributeValueDecoder decoder = writeOp.DecoderFor(kValueToStore);
        EXPECT_EQ(persistence.DecodeAndStoreNativeEndianValue(path, decoder, valueRead), CHIP_NO_ERROR);
        EXPECT_EQ(valueRead, kValueToStore);
    }

    {
        valueRead = 0;
        ASSERT_TRUE(persistence.LoadNativeEndianValue(path, valueRead, kOtherValue));
        ASSERT_EQ(valueRead, kValueToStore);
    }

    // Try to read non-compatible types (note that size-wise compatible types will work ... wrongly (like u32 and float))
    // this extra check is best-effort
    {
        uint16_t smallValue   = 0;
        const uint16_t kOther = 123u;

        ASSERT_FALSE(persistence.LoadNativeEndianValue(path, smallValue, kOther));
        ASSERT_EQ(smallValue, kOther);
    }
    {
        uint64_t largeValue   = 0;
        const uint64_t kOther = 0x1122334455667788ull;

        ASSERT_FALSE(persistence.LoadNativeEndianValue(path, largeValue, kOther));
        ASSERT_EQ(largeValue, kOther);
    }
}

TEST(TestAttributePersistence, TestStrings)
{
    TestPersistentStorageDelegate storageDelegate;
    DefaultAttributePersistenceProvider ramProvider;
    ASSERT_EQ(ramProvider.Init(&storageDelegate), CHIP_NO_ERROR);

    AttributePersistence persistence(ramProvider);

    const ConcreteAttributePath path(1, 2, 3);
    const ConcreteAttributePath wrongPath(1, 2, 4);

    {
        Storage::String<8> testString;

        ASSERT_TRUE(testString.SetContent("foo"_span));
        ASSERT_EQ(persistence.StoreString(path, testString), CHIP_NO_ERROR);
    }

    {
        Storage::String<16> readString;
        ASSERT_TRUE(persistence.LoadString(path, readString));
        ASSERT_TRUE(readString.Content().data_equal("foo"_span));
        ASSERT_STREQ(readString.c_str(), "foo");
    }

    // fits exactly. Load should succeed
    {
        Storage::String<3> readString;
        ASSERT_TRUE(persistence.LoadString(path, readString));
        ASSERT_TRUE(readString.Content().data_equal("foo"_span));
        ASSERT_STREQ(readString.c_str(), "foo");
    }

    // no space: data is cleared on load error
    {
        Storage::String<2> readString;
        ASSERT_FALSE(persistence.LoadString(path, readString));
        ASSERT_TRUE(readString.Content().empty());

        ASSERT_TRUE(readString.SetContent("xy"_span));
        ASSERT_FALSE(readString.Content().empty());
        ASSERT_FALSE(persistence.LoadString(path, readString));
        ASSERT_TRUE(readString.Content().empty());
        ASSERT_STREQ(readString.c_str(), "");
    }

    // wrong path: data is cleared on load error
    {
        Storage::String<16> readString;

        ASSERT_TRUE(readString.SetContent("xy"_span));
        ASSERT_FALSE(readString.Content().empty());
        ASSERT_FALSE(persistence.LoadString(wrongPath, readString));
        ASSERT_TRUE(readString.Content().empty());
        ASSERT_STREQ(readString.c_str(), "");
    }

    // empty string can be stored and loaded
    {
        Storage::String<8> testString;
        Storage::String<16> readString;

        ASSERT_TRUE(testString.SetContent(""_span));
        ASSERT_EQ(persistence.StoreString(path, testString), CHIP_NO_ERROR);

        ASSERT_TRUE(readString.SetContent("some value"_span));
        ASSERT_TRUE(persistence.LoadString(path, readString));
        ASSERT_TRUE(readString.Content().empty());
        ASSERT_STREQ(readString.c_str(), "");
    }
}

TEST(TestAttributePersistence, TestEnumHandling)
{
    // Using TimeFormatLocalization enums for these tests.
    using namespace chip::app::Clusters;
    using namespace chip::app::Clusters::TimeFormatLocalization;

    TestPersistentStorageDelegate storageDelegate;
    DefaultAttributePersistenceProvider ramProvider;
    ASSERT_EQ(ramProvider.Init(&storageDelegate), CHIP_NO_ERROR);

    AttributePersistence persistence(ramProvider);

    CalendarTypeEnum valueRead = CalendarTypeEnum::kUnknownEnumValue;

    // Test storing and loading a valid enum value
    {
        const ConcreteAttributePath path(1, 2, 3);
        WriteOperation writeOp(path);
        AttributeValueDecoder decoder = writeOp.DecoderFor(CalendarTypeEnum::kGregorian);
        EXPECT_EQ(persistence.DecodeAndStoreNativeEndianValue(path, decoder, valueRead), CHIP_NO_ERROR);
        EXPECT_EQ(valueRead, CalendarTypeEnum::kGregorian);

        // Test loading the stored enum value
        valueRead = CalendarTypeEnum::kUnknownEnumValue;
        EXPECT_TRUE(persistence.LoadNativeEndianValue(path, valueRead, CalendarTypeEnum::kPersian));
        EXPECT_EQ(valueRead, CalendarTypeEnum::kGregorian);
    }

    // Test attempting to store an unknown enum value
    {
        const ConcreteAttributePath path(3, 2, 1);
        const uint8_t testUnknownValue = static_cast<uint8_t>(CalendarTypeEnum::kUnknownEnumValue) + 1;
        ASSERT_EQ(EnsureKnownEnumValue(static_cast<CalendarTypeEnum>(testUnknownValue)), CalendarTypeEnum::kUnknownEnumValue);
        WriteOperation writeOp(path);
        AttributeValueDecoder decoder = writeOp.DecoderFor(testUnknownValue);
        EXPECT_EQ(persistence.DecodeAndStoreNativeEndianValue(path, decoder, valueRead), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    }
}

TEST(TestAttributePersistence, TestNoOpOnSameValueArithmetic)
{
    TestPersistentStorageDelegate storageDelegate;
    DefaultAttributePersistenceProvider ramProvider;
    ASSERT_EQ(ramProvider.Init(&storageDelegate), CHIP_NO_ERROR);

    AttributePersistence persistence(ramProvider);

    const ConcreteAttributePath path(1, 2, 3);
    constexpr uint32_t kInitialValue = 42;

    // Store an initial value
    uint32_t currentValue = kInitialValue;
    {
        currentValue = 0;
        WriteOperation writeOp(path);
        AttributeValueDecoder decoder = writeOp.DecoderFor(kInitialValue);
        EXPECT_EQ(persistence.DecodeAndStoreNativeEndianValue(path, decoder, currentValue), CHIP_NO_ERROR);
        EXPECT_EQ(currentValue, kInitialValue);
    }

    // Attempt to store the same value - should return kWriteSuccessNoOp
    {
        WriteOperation writeOp(path);
        AttributeValueDecoder decoder        = writeOp.DecoderFor(kInitialValue);
        DataModel::ActionReturnStatus status = persistence.DecodeAndStoreNativeEndianValue(path, decoder, currentValue);
        EXPECT_TRUE(status.IsSuccess());
        EXPECT_TRUE(status.IsNoOpSuccess());
        EXPECT_EQ(currentValue, kInitialValue); // Value should remain unchanged
    }

    // Verify the value is still loadable and unchanged
    {
        uint32_t loadedValue = 0;
        EXPECT_TRUE(persistence.LoadNativeEndianValue(path, loadedValue, static_cast<uint32_t>(0)));
        EXPECT_EQ(loadedValue, kInitialValue);
    }
}

TEST(TestAttributePersistence, TestNoOpOnSameValueEnum)
{
    using namespace chip::app::Clusters;
    using namespace chip::app::Clusters::TimeFormatLocalization;

    TestPersistentStorageDelegate storageDelegate;
    DefaultAttributePersistenceProvider ramProvider;
    ASSERT_EQ(ramProvider.Init(&storageDelegate), CHIP_NO_ERROR);

    AttributePersistence persistence(ramProvider);

    const ConcreteAttributePath path(1, 2, 3);
    CalendarTypeEnum currentValue = CalendarTypeEnum::kUnknownEnumValue;

    // Store an initial enum value
    {
        WriteOperation writeOp(path);
        AttributeValueDecoder decoder = writeOp.DecoderFor(CalendarTypeEnum::kGregorian);
        EXPECT_EQ(persistence.DecodeAndStoreNativeEndianValue(path, decoder, currentValue), CHIP_NO_ERROR);
        EXPECT_EQ(currentValue, CalendarTypeEnum::kGregorian);
    }

    // Attempt to store the same enum value - should return kWriteSuccessNoOp
    {
        WriteOperation writeOp(path);
        AttributeValueDecoder decoder        = writeOp.DecoderFor(CalendarTypeEnum::kGregorian);
        DataModel::ActionReturnStatus status = persistence.DecodeAndStoreNativeEndianValue(path, decoder, currentValue);
        EXPECT_TRUE(status.IsSuccess());
        EXPECT_TRUE(status.IsNoOpSuccess());
        EXPECT_EQ(currentValue, CalendarTypeEnum::kGregorian);
    }

    // Verify the value is still loadable and unchanged
    {
        CalendarTypeEnum loadedValue = CalendarTypeEnum::kUnknownEnumValue;
        EXPECT_TRUE(persistence.LoadNativeEndianValue(path, loadedValue, CalendarTypeEnum::kPersian));
        EXPECT_EQ(loadedValue, CalendarTypeEnum::kGregorian);
    }
}

TEST(TestAttributePersistence, TestWriteOnDifferentValueEnum)
{
    using namespace chip::app::Clusters;
    using namespace chip::app::Clusters::TimeFormatLocalization;

    TestPersistentStorageDelegate storageDelegate;
    DefaultAttributePersistenceProvider ramProvider;
    ASSERT_EQ(ramProvider.Init(&storageDelegate), CHIP_NO_ERROR);

    AttributePersistence persistence(ramProvider);

    const ConcreteAttributePath path(1, 2, 3);
    CalendarTypeEnum currentValue = CalendarTypeEnum::kUnknownEnumValue;

    // Store an initial enum value
    {
        WriteOperation writeOp(path);
        AttributeValueDecoder decoder = writeOp.DecoderFor(CalendarTypeEnum::kGregorian);
        EXPECT_EQ(persistence.DecodeAndStoreNativeEndianValue(path, decoder, currentValue), CHIP_NO_ERROR);
        EXPECT_EQ(currentValue, CalendarTypeEnum::kGregorian);
    }

    // Store a different enum value - should perform actual write
    {
        WriteOperation writeOp(path);
        AttributeValueDecoder decoder        = writeOp.DecoderFor(CalendarTypeEnum::kBuddhist);
        DataModel::ActionReturnStatus status = persistence.DecodeAndStoreNativeEndianValue(path, decoder, currentValue);
        EXPECT_TRUE(status.IsSuccess());
        EXPECT_FALSE(status.IsNoOpSuccess());
        EXPECT_EQ(currentValue, CalendarTypeEnum::kBuddhist);
    }

    // Verify the new value is persisted
    {
        CalendarTypeEnum loadedValue = CalendarTypeEnum::kUnknownEnumValue;
        EXPECT_TRUE(persistence.LoadNativeEndianValue(path, loadedValue, CalendarTypeEnum::kPersian));
        EXPECT_EQ(loadedValue, CalendarTypeEnum::kBuddhist);
    }
}

TEST(TestAttributePersistence, TestInvalidPascalLengthStored)
{
    TestPersistentStorageDelegate storageDelegate;
    DefaultAttributePersistenceProvider ramProvider;
    ASSERT_EQ(ramProvider.Init(&storageDelegate), CHIP_NO_ERROR);

    AttributePersistence persistence(ramProvider);
    const ConcreteAttributePath path(1, 2, 3);

    // This string is invalid as stored
    {
        uint8_t buffer[] = { 10, 'h', 'e', 'l', 'l', 'o' }; // length 10, but only 5 chars
        EXPECT_EQ(storageDelegate.SyncSetKeyValue(
                      DefaultStorageKeyAllocator::AttributeValue(path.mEndpointId, path.mClusterId, path.mAttributeId).KeyName(),
                      buffer, sizeof(buffer)),
                  CHIP_NO_ERROR);
    }

    // Load into a buffer that COULD contain the string, but
    // stored string is invalid
    {
        Storage::String<16> readString;

        ASSERT_TRUE(readString.SetContent("some value"_span));
        ASSERT_FALSE(persistence.LoadString(path, readString));
        ASSERT_TRUE(readString.Content().empty());
    }
}

TEST(TestAttributePersistence, TestLoadNativeEndianValueNullable)
{
    TestPersistentStorageDelegate storageDelegate;
    DefaultAttributePersistenceProvider ramProvider;
    ASSERT_EQ(ramProvider.Init(&storageDelegate), CHIP_NO_ERROR);

    AttributePersistence persistence(ramProvider);

    const ConcreteAttributePath path(1, 2, 3);
    const ConcreteAttributePath wrongPath(1, 2, 4);
    constexpr uint32_t kValueToStore = 42;

    // Store a non-null value directly
    {
        typename NumericAttributeTraits<uint32_t>::StorageType storageReadValue;
        NumericAttributeTraits<uint32_t>::WorkingToStorage(kValueToStore, storageReadValue);
        EXPECT_EQ(storageDelegate.SyncSetKeyValue(
                      DefaultStorageKeyAllocator::AttributeValue(path.mEndpointId, path.mClusterId, path.mAttributeId).KeyName(),
                      &storageReadValue, sizeof(storageReadValue)),
                  CHIP_NO_ERROR);
    }

    // Test loading a non-null value into Nullable
    {
        DataModel::Nullable<uint32_t> valueRead;
        DataModel::Nullable<uint32_t> defaultValue = DataModel::MakeNullable<uint32_t>(99);

        ASSERT_TRUE(persistence.LoadNativeEndianValue(path, valueRead, defaultValue));
        ASSERT_FALSE(valueRead.IsNull());
        ASSERT_EQ(valueRead.Value(), kValueToStore);
    }

    // Store a null value
    {
        typename NumericAttributeTraits<uint32_t>::StorageType nullValue;
        NumericAttributeTraits<uint32_t>::SetNull(nullValue);
        EXPECT_EQ(storageDelegate.SyncSetKeyValue(
                      DefaultStorageKeyAllocator::AttributeValue(path.mEndpointId, path.mClusterId, path.mAttributeId).KeyName(),
                      &nullValue, sizeof(nullValue)),
                  CHIP_NO_ERROR);
    }

    // Test loading a null value
    {
        DataModel::Nullable<uint32_t> valueRead;
        DataModel::Nullable<uint32_t> defaultValue = DataModel::MakeNullable<uint32_t>(99);

        ASSERT_TRUE(persistence.LoadNativeEndianValue(path, valueRead, defaultValue));
        ASSERT_TRUE(valueRead.IsNull());
    }

    // Test loading from non-existent path with non-null default
    {
        DataModel::Nullable<uint32_t> valueRead;
        DataModel::Nullable<uint32_t> defaultValue = DataModel::MakeNullable<uint32_t>(123);

        ASSERT_FALSE(persistence.LoadNativeEndianValue(wrongPath, valueRead, defaultValue));
        ASSERT_FALSE(valueRead.IsNull());
        ASSERT_EQ(valueRead.Value(), 123u);
    }

    // Test loading from non-existent path with null default
    {
        DataModel::Nullable<uint32_t> valueRead;
        DataModel::Nullable<uint32_t> defaultValue = DataModel::NullNullable;

        ASSERT_FALSE(persistence.LoadNativeEndianValue(wrongPath, valueRead, defaultValue));
        ASSERT_TRUE(valueRead.IsNull());
    }
}

TEST(TestAttributePersistence, TestDecodeAndStoreNativeEndianValueNullable)
{
    TestPersistentStorageDelegate storageDelegate;
    DefaultAttributePersistenceProvider ramProvider;
    ASSERT_EQ(ramProvider.Init(&storageDelegate), CHIP_NO_ERROR);

    AttributePersistence persistence(ramProvider);

    const ConcreteAttributePath path(1, 2, 3);
    constexpr uint32_t kValueToStore = 0x12345678;

    // Store a non-null value via decoder
    {
        DataModel::Nullable<uint32_t> currentValue;
        WriteOperation writeOp(path);
        AttributeValueDecoder decoder = writeOp.DecoderFor(DataModel::MakeNullable(kValueToStore));
        EXPECT_EQ(persistence.DecodeAndStoreNativeEndianValue(path, decoder, currentValue), CHIP_NO_ERROR);
        ASSERT_FALSE(currentValue.IsNull());
        EXPECT_EQ(currentValue.Value(), kValueToStore);
    }

    // Verify the value can be loaded back
    {
        DataModel::Nullable<uint32_t> valueRead;
        DataModel::Nullable<uint32_t> errorRead;
        ASSERT_TRUE(persistence.LoadNativeEndianValue(path, valueRead, errorRead));
        ASSERT_FALSE(valueRead.IsNull());
        ASSERT_EQ(valueRead.Value(), kValueToStore);
    }

    // Store a null value via decoder
    {
        DataModel::Nullable<uint32_t> currentValue = DataModel::MakeNullable(kValueToStore);
        WriteOperation writeOp(path);
        AttributeValueDecoder decoder = writeOp.DecoderFor(DataModel::Nullable<uint32_t>());
        EXPECT_EQ(persistence.DecodeAndStoreNativeEndianValue(path, decoder, currentValue), CHIP_NO_ERROR);
        ASSERT_TRUE(currentValue.IsNull());
    }

    // Verify the null value can be loaded back
    {
        DataModel::Nullable<uint32_t> valueRead = DataModel::MakeNullable<uint32_t>(999);
        ASSERT_TRUE(persistence.LoadNativeEndianValue(path, valueRead, DataModel::MakeNullable<uint32_t>(0)));
        ASSERT_TRUE(valueRead.IsNull());
    }
}

TEST(TestAttributePersistence, TestNoOpOnSameValueNullable)
{
    TestPersistentStorageDelegate storageDelegate;
    DefaultAttributePersistenceProvider ramProvider;
    ASSERT_EQ(ramProvider.Init(&storageDelegate), CHIP_NO_ERROR);

    AttributePersistence persistence(ramProvider);

    const ConcreteAttributePath path(1, 2, 3);
    constexpr uint32_t kInitialValue = 42;

    // Test no-op for same non-null value
    {
        DataModel::Nullable<uint32_t> currentValue;
        WriteOperation writeOp(path);
        AttributeValueDecoder decoder = writeOp.DecoderFor(DataModel::MakeNullable(kInitialValue));
        EXPECT_EQ(persistence.DecodeAndStoreNativeEndianValue(path, decoder, currentValue), CHIP_NO_ERROR);
    }

    {
        DataModel::Nullable<uint32_t> currentValue = DataModel::MakeNullable(kInitialValue);
        WriteOperation writeOp(path);
        AttributeValueDecoder decoder        = writeOp.DecoderFor(DataModel::MakeNullable(kInitialValue));
        DataModel::ActionReturnStatus status = persistence.DecodeAndStoreNativeEndianValue(path, decoder, currentValue);
        EXPECT_TRUE(status.IsSuccess());
        EXPECT_TRUE(status.IsNoOpSuccess());
    }

    // Test no-op for same null value
    {
        DataModel::Nullable<uint32_t> currentValue = DataModel::MakeNullable(kInitialValue);
        WriteOperation writeOp(path);
        AttributeValueDecoder decoder = writeOp.DecoderFor(DataModel::Nullable<uint32_t>());
        EXPECT_EQ(persistence.DecodeAndStoreNativeEndianValue(path, decoder, currentValue), CHIP_NO_ERROR);
        ASSERT_TRUE(currentValue.IsNull());
    }

    {
        DataModel::Nullable<uint32_t> currentValue; // null
        WriteOperation writeOp(path);
        AttributeValueDecoder decoder        = writeOp.DecoderFor(DataModel::Nullable<uint32_t>());
        DataModel::ActionReturnStatus status = persistence.DecodeAndStoreNativeEndianValue(path, decoder, currentValue);
        EXPECT_TRUE(status.IsSuccess());
        EXPECT_TRUE(status.IsNoOpSuccess());
    }
}

TEST(TestAttributePersistence, TestWriteOnDifferentValueNullable)
{
    TestPersistentStorageDelegate storageDelegate;
    DefaultAttributePersistenceProvider ramProvider;
    ASSERT_EQ(ramProvider.Init(&storageDelegate), CHIP_NO_ERROR);

    AttributePersistence persistence(ramProvider);

    const ConcreteAttributePath path(1, 2, 3);
    constexpr uint32_t kInitialValue = 42;
    constexpr uint32_t kNewValue     = 99;

    // Store initial non-null value
    {
        DataModel::Nullable<uint32_t> currentValue;
        WriteOperation writeOp(path);
        AttributeValueDecoder decoder = writeOp.DecoderFor(DataModel::MakeNullable(kInitialValue));
        EXPECT_EQ(persistence.DecodeAndStoreNativeEndianValue(path, decoder, currentValue), CHIP_NO_ERROR);
        EXPECT_EQ(currentValue.Value(), kInitialValue);
    }

    // Store a different non-null value - should write
    {
        DataModel::Nullable<uint32_t> currentValue = DataModel::MakeNullable(kInitialValue);
        WriteOperation writeOp(path);
        AttributeValueDecoder decoder        = writeOp.DecoderFor(DataModel::MakeNullable(kNewValue));
        DataModel::ActionReturnStatus status = persistence.DecodeAndStoreNativeEndianValue(path, decoder, currentValue);
        EXPECT_TRUE(status.IsSuccess());
        EXPECT_FALSE(status.IsNoOpSuccess());
        EXPECT_EQ(currentValue.Value(), kNewValue);
    }

    // Store null - should write
    {
        DataModel::Nullable<uint32_t> currentValue = DataModel::MakeNullable(kNewValue);
        WriteOperation writeOp(path);
        AttributeValueDecoder decoder        = writeOp.DecoderFor(DataModel::Nullable<uint32_t>());
        DataModel::ActionReturnStatus status = persistence.DecodeAndStoreNativeEndianValue(path, decoder, currentValue);
        EXPECT_TRUE(status.IsSuccess());
        EXPECT_FALSE(status.IsNoOpSuccess());
        EXPECT_TRUE(currentValue.IsNull());
    }

    // Store non-null after null - should write
    {
        DataModel::Nullable<uint32_t> currentValue; // null
        WriteOperation writeOp(path);
        AttributeValueDecoder decoder        = writeOp.DecoderFor(DataModel::MakeNullable(kInitialValue));
        DataModel::ActionReturnStatus status = persistence.DecodeAndStoreNativeEndianValue(path, decoder, currentValue);
        EXPECT_TRUE(status.IsSuccess());
        EXPECT_FALSE(status.IsNoOpSuccess());
        EXPECT_EQ(currentValue.Value(), kInitialValue);
    }
}

TEST(TestAttributePersistence, TestLoadNativeEndianValueNullableEnum)
{
    using namespace chip::app::Clusters;
    using namespace chip::app::Clusters::TimeFormatLocalization;

    TestPersistentStorageDelegate storageDelegate;
    DefaultAttributePersistenceProvider ramProvider;
    ASSERT_EQ(ramProvider.Init(&storageDelegate), CHIP_NO_ERROR);

    AttributePersistence persistence(ramProvider);

    const ConcreteAttributePath path(1, 2, 3);
    const ConcreteAttributePath wrongPath(1, 2, 4);

    // Store a non-null enum value directly
    {
        typename NumericAttributeTraits<CalendarTypeEnum>::StorageType storageValue;
        NumericAttributeTraits<CalendarTypeEnum>::WorkingToStorage(CalendarTypeEnum::kGregorian, storageValue);
        EXPECT_EQ(storageDelegate.SyncSetKeyValue(
                      DefaultStorageKeyAllocator::AttributeValue(path.mEndpointId, path.mClusterId, path.mAttributeId).KeyName(),
                      &storageValue, sizeof(storageValue)),
                  CHIP_NO_ERROR);
    }

    // Test loading a non-null enum value into Nullable
    {
        DataModel::Nullable<CalendarTypeEnum> valueRead;
        DataModel::Nullable<CalendarTypeEnum> defaultValue = DataModel::MakeNullable(CalendarTypeEnum::kPersian);

        ASSERT_TRUE(persistence.LoadNativeEndianValue(path, valueRead, defaultValue));
        ASSERT_FALSE(valueRead.IsNull());
        ASSERT_EQ(valueRead.Value(), CalendarTypeEnum::kGregorian);
    }

    // Store a null value
    {
        typename NumericAttributeTraits<CalendarTypeEnum>::StorageType nullValue;
        NumericAttributeTraits<CalendarTypeEnum>::SetNull(nullValue);
        EXPECT_EQ(storageDelegate.SyncSetKeyValue(
                      DefaultStorageKeyAllocator::AttributeValue(path.mEndpointId, path.mClusterId, path.mAttributeId).KeyName(),
                      &nullValue, sizeof(nullValue)),
                  CHIP_NO_ERROR);
    }

    // Test loading a null value
    {
        DataModel::Nullable<CalendarTypeEnum> valueRead;
        DataModel::Nullable<CalendarTypeEnum> defaultValue = DataModel::MakeNullable(CalendarTypeEnum::kPersian);

        ASSERT_TRUE(persistence.LoadNativeEndianValue(path, valueRead, defaultValue));
        ASSERT_TRUE(valueRead.IsNull());
    }

    // Test loading from non-existent path with non-null default
    {
        DataModel::Nullable<CalendarTypeEnum> valueRead;
        DataModel::Nullable<CalendarTypeEnum> defaultValue = DataModel::MakeNullable(CalendarTypeEnum::kBuddhist);

        ASSERT_FALSE(persistence.LoadNativeEndianValue(wrongPath, valueRead, defaultValue));
        ASSERT_FALSE(valueRead.IsNull());
        ASSERT_EQ(valueRead.Value(), CalendarTypeEnum::kBuddhist);
    }

    // Test loading from non-existent path with null default
    {
        DataModel::Nullable<CalendarTypeEnum> valueRead;
        DataModel::Nullable<CalendarTypeEnum> defaultValue = DataModel::NullNullable;

        ASSERT_FALSE(persistence.LoadNativeEndianValue(wrongPath, valueRead, defaultValue));
        ASSERT_TRUE(valueRead.IsNull());
    }
}

TEST(TestAttributePersistence, TestDecodeAndStoreNativeEndianValueNullableEnum)
{
    using namespace chip::app::Clusters;
    using namespace chip::app::Clusters::TimeFormatLocalization;

    TestPersistentStorageDelegate storageDelegate;
    DefaultAttributePersistenceProvider ramProvider;
    ASSERT_EQ(ramProvider.Init(&storageDelegate), CHIP_NO_ERROR);

    AttributePersistence persistence(ramProvider);

    const ConcreteAttributePath path(1, 2, 3);

    // Store a non-null valid enum value via decoder
    {
        DataModel::Nullable<CalendarTypeEnum> currentValue;
        WriteOperation writeOp(path);
        AttributeValueDecoder decoder = writeOp.DecoderFor(DataModel::MakeNullable(CalendarTypeEnum::kGregorian));
        EXPECT_EQ(persistence.DecodeAndStoreNativeEndianValue(path, decoder, currentValue), CHIP_NO_ERROR);
        ASSERT_FALSE(currentValue.IsNull());
        EXPECT_EQ(currentValue.Value(), CalendarTypeEnum::kGregorian);
    }

    // Verify the value can be loaded back
    {
        DataModel::Nullable<CalendarTypeEnum> valueRead;
        DataModel::Nullable<CalendarTypeEnum> defaultValue = DataModel::MakeNullable(CalendarTypeEnum::kPersian);
        ASSERT_TRUE(persistence.LoadNativeEndianValue(path, valueRead, defaultValue));
        ASSERT_FALSE(valueRead.IsNull());
        ASSERT_EQ(valueRead.Value(), CalendarTypeEnum::kGregorian);
    }

    // Store a null value via decoder
    {
        DataModel::Nullable<CalendarTypeEnum> currentValue = DataModel::MakeNullable(CalendarTypeEnum::kGregorian);
        WriteOperation writeOp(path);
        AttributeValueDecoder decoder = writeOp.DecoderFor(DataModel::Nullable<CalendarTypeEnum>());
        EXPECT_EQ(persistence.DecodeAndStoreNativeEndianValue(path, decoder, currentValue), CHIP_NO_ERROR);
        ASSERT_TRUE(currentValue.IsNull());
    }

    // Verify the null value can be loaded back
    {
        DataModel::Nullable<CalendarTypeEnum> valueRead = DataModel::MakeNullable(CalendarTypeEnum::kBuddhist);
        ASSERT_TRUE(persistence.LoadNativeEndianValue(path, valueRead, DataModel::MakeNullable(CalendarTypeEnum::kPersian)));
        ASSERT_TRUE(valueRead.IsNull());
    }

    // Test that kUnknownEnumValue wrapped in Nullable is rejected
    {
        const ConcreteAttributePath path2(4, 5, 6);
        const uint8_t testUnknownValue = static_cast<uint8_t>(CalendarTypeEnum::kUnknownEnumValue) + 1;
        ASSERT_EQ(EnsureKnownEnumValue(static_cast<CalendarTypeEnum>(testUnknownValue)), CalendarTypeEnum::kUnknownEnumValue);

        DataModel::Nullable<CalendarTypeEnum> currentValue;
        WriteOperation writeOp(path2);
        AttributeValueDecoder decoder = writeOp.DecoderFor(testUnknownValue);
        EXPECT_EQ(persistence.DecodeAndStoreNativeEndianValue(path2, decoder, currentValue),
                  CHIP_IM_GLOBAL_STATUS(ConstraintError));
    }

    // Test that null bypasses kUnknownEnumValue check (null is valid)
    {
        const ConcreteAttributePath path3(7, 8, 9);
        DataModel::Nullable<CalendarTypeEnum> currentValue = DataModel::MakeNullable(CalendarTypeEnum::kGregorian);
        WriteOperation writeOp(path3);
        AttributeValueDecoder decoder = writeOp.DecoderFor(DataModel::Nullable<CalendarTypeEnum>());
        EXPECT_EQ(persistence.DecodeAndStoreNativeEndianValue(path3, decoder, currentValue), CHIP_NO_ERROR);
        ASSERT_TRUE(currentValue.IsNull());
    }
}

TEST(TestAttributePersistence, TestNoOpOnSameValueNullableEnum)
{
    using namespace chip::app::Clusters;
    using namespace chip::app::Clusters::TimeFormatLocalization;

    TestPersistentStorageDelegate storageDelegate;
    DefaultAttributePersistenceProvider ramProvider;
    ASSERT_EQ(ramProvider.Init(&storageDelegate), CHIP_NO_ERROR);

    AttributePersistence persistence(ramProvider);

    const ConcreteAttributePath path(1, 2, 3);

    // Test no-op for same non-null enum value
    {
        DataModel::Nullable<CalendarTypeEnum> currentValue;
        WriteOperation writeOp(path);
        AttributeValueDecoder decoder = writeOp.DecoderFor(DataModel::MakeNullable(CalendarTypeEnum::kGregorian));
        EXPECT_EQ(persistence.DecodeAndStoreNativeEndianValue(path, decoder, currentValue), CHIP_NO_ERROR);
        EXPECT_EQ(currentValue.Value(), CalendarTypeEnum::kGregorian);
    }

    {
        DataModel::Nullable<CalendarTypeEnum> currentValue = DataModel::MakeNullable(CalendarTypeEnum::kGregorian);
        WriteOperation writeOp(path);
        AttributeValueDecoder decoder = writeOp.DecoderFor(DataModel::MakeNullable(CalendarTypeEnum::kGregorian));
        DataModel::ActionReturnStatus status = persistence.DecodeAndStoreNativeEndianValue(path, decoder, currentValue);
        EXPECT_TRUE(status.IsSuccess());
        EXPECT_TRUE(status.IsNoOpSuccess());
        EXPECT_EQ(currentValue.Value(), CalendarTypeEnum::kGregorian);
    }

    // Test no-op for same null value
    {
        DataModel::Nullable<CalendarTypeEnum> currentValue = DataModel::MakeNullable(CalendarTypeEnum::kGregorian);
        WriteOperation writeOp(path);
        AttributeValueDecoder decoder = writeOp.DecoderFor(DataModel::Nullable<CalendarTypeEnum>());
        EXPECT_EQ(persistence.DecodeAndStoreNativeEndianValue(path, decoder, currentValue), CHIP_NO_ERROR);
        ASSERT_TRUE(currentValue.IsNull());
    }

    {
        DataModel::Nullable<CalendarTypeEnum> currentValue; // null
        WriteOperation writeOp(path);
        AttributeValueDecoder decoder = writeOp.DecoderFor(DataModel::Nullable<CalendarTypeEnum>());
        DataModel::ActionReturnStatus status = persistence.DecodeAndStoreNativeEndianValue(path, decoder, currentValue);
        EXPECT_TRUE(status.IsSuccess());
        EXPECT_TRUE(status.IsNoOpSuccess());
        ASSERT_TRUE(currentValue.IsNull());
    }
}

TEST(TestAttributePersistence, TestWriteOnDifferentValueNullableEnum)
{
    using namespace chip::app::Clusters;
    using namespace chip::app::Clusters::TimeFormatLocalization;

    TestPersistentStorageDelegate storageDelegate;
    DefaultAttributePersistenceProvider ramProvider;
    ASSERT_EQ(ramProvider.Init(&storageDelegate), CHIP_NO_ERROR);

    AttributePersistence persistence(ramProvider);

    const ConcreteAttributePath path(1, 2, 3);

    // Store initial non-null enum value
    {
        DataModel::Nullable<CalendarTypeEnum> currentValue;
        WriteOperation writeOp(path);
        AttributeValueDecoder decoder = writeOp.DecoderFor(DataModel::MakeNullable(CalendarTypeEnum::kGregorian));
        EXPECT_EQ(persistence.DecodeAndStoreNativeEndianValue(path, decoder, currentValue), CHIP_NO_ERROR);
        EXPECT_EQ(currentValue.Value(), CalendarTypeEnum::kGregorian);
    }

    // Store a different non-null enum value - should write
    {
        DataModel::Nullable<CalendarTypeEnum> currentValue = DataModel::MakeNullable(CalendarTypeEnum::kGregorian);
        WriteOperation writeOp(path);
        AttributeValueDecoder decoder = writeOp.DecoderFor(DataModel::MakeNullable(CalendarTypeEnum::kBuddhist));
        DataModel::ActionReturnStatus status = persistence.DecodeAndStoreNativeEndianValue(path, decoder, currentValue);
        EXPECT_TRUE(status.IsSuccess());
        EXPECT_FALSE(status.IsNoOpSuccess());
        EXPECT_EQ(currentValue.Value(), CalendarTypeEnum::kBuddhist);
    }

    // Verify the new value is persisted
    {
        DataModel::Nullable<CalendarTypeEnum> loadedValue;
        ASSERT_TRUE(persistence.LoadNativeEndianValue(path, loadedValue, DataModel::MakeNullable(CalendarTypeEnum::kPersian)));
        ASSERT_FALSE(loadedValue.IsNull());
        ASSERT_EQ(loadedValue.Value(), CalendarTypeEnum::kBuddhist);
    }

    // Store null - should write
    {
        DataModel::Nullable<CalendarTypeEnum> currentValue = DataModel::MakeNullable(CalendarTypeEnum::kBuddhist);
        WriteOperation writeOp(path);
        AttributeValueDecoder decoder = writeOp.DecoderFor(DataModel::Nullable<CalendarTypeEnum>());
        DataModel::ActionReturnStatus status = persistence.DecodeAndStoreNativeEndianValue(path, decoder, currentValue);
        EXPECT_TRUE(status.IsSuccess());
        EXPECT_FALSE(status.IsNoOpSuccess());
        EXPECT_TRUE(currentValue.IsNull());
    }

    // Verify null is persisted
    {
        DataModel::Nullable<CalendarTypeEnum> loadedValue = DataModel::MakeNullable(CalendarTypeEnum::kGregorian);
        ASSERT_TRUE(persistence.LoadNativeEndianValue(path, loadedValue, DataModel::MakeNullable(CalendarTypeEnum::kPersian)));
        ASSERT_TRUE(loadedValue.IsNull());
    }

    // Store non-null after null - should write
    {
        DataModel::Nullable<CalendarTypeEnum> currentValue; // null
        WriteOperation writeOp(path);
        AttributeValueDecoder decoder = writeOp.DecoderFor(DataModel::MakeNullable(CalendarTypeEnum::kCoptic));
        DataModel::ActionReturnStatus status = persistence.DecodeAndStoreNativeEndianValue(path, decoder, currentValue);
        EXPECT_TRUE(status.IsSuccess());
        EXPECT_FALSE(status.IsNoOpSuccess());
        EXPECT_EQ(currentValue.Value(), CalendarTypeEnum::kCoptic);
    }

    // Verify the final value is persisted
    {
        DataModel::Nullable<CalendarTypeEnum> loadedValue;
        ASSERT_TRUE(persistence.LoadNativeEndianValue(path, loadedValue, DataModel::MakeNullable(CalendarTypeEnum::kPersian)));
        ASSERT_FALSE(loadedValue.IsNull());
        ASSERT_EQ(loadedValue.Value(), CalendarTypeEnum::kCoptic);
    }
}
} // namespace
