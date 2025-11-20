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

} // namespace
