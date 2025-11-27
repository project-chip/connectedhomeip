/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
 *      This file implements a unit test suite for testing different zone
 *      polygons for self-intersection.
 *
 */

#include "pw_unit_test/framework.h"
#include <app/clusters/commodity-tariff-server/CommodityTariffAttrsDataMgmt.h>

#include <cstddef>
#include <cstdint>
#include <mutex>
#include <thread>

#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::CommodityTariffAttrsDataMgmt;

// =================================
//      Unit tests
// =================================

// In TestCommodityTariffBaseDataClass.cpp or a test helper header
namespace chip {
namespace Platform {
namespace Internal {

void AssertChipStackLockedByCurrentThread(const char * file, int line)
{
    // Empty implementation for tests - or add test-specific logic
    // You could use gtest assertions here if you're using Google Test
}

} // namespace Internal
} // namespace Platform
} // namespace chip

namespace chip {
namespace app {

class TestCommodityTariffBaseDataClass : public ::testing::Test
{
public:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}

protected:
    void SetUp() override
    {
        // Initialize CHIP stack if needed
        VerifyOrDie(Platform::MemoryInit() == CHIP_NO_ERROR);
    }

    void TearDown() override { Platform::MemoryShutdown(); }
};

TEST_F(TestCommodityTariffBaseDataClass, BasicConstruction)
{
    // Test construction with different types
    CTC_BaseDataClass<uint32_t> intData(1);
    CTC_BaseDataClass<DataModel::Nullable<uint32_t>> nullableData(2);
    CTC_BaseDataClass<DataModel::List<uint32_t>> listData(3);

    // Verify initial states
    EXPECT_EQ(intData.HasValue(), false);
    EXPECT_EQ(nullableData.GetValue().IsNull(), true); // Nullable starts as null
    EXPECT_EQ(nullableData.HasValue(), false);         // Nullable starts as null
    EXPECT_EQ(listData.GetValue().size() == 0, true);  // List starts as empty
    EXPECT_EQ(listData.HasValue(), false);
}

template <>
CHIP_ERROR CTC_BaseDataClass<uint32_t>::ValidateNewValue()
{
    return CHIP_NO_ERROR;
}

TEST_F(TestCommodityTariffBaseDataClass, ScalarValueUpdateFlow)
{
    CTC_BaseDataClass<uint32_t> data(1);
    constexpr uint32_t data_sample_1 = 0xde;
    constexpr uint32_t data_sample_2 = 0xad;

    // Create new value
    EXPECT_EQ(data.CreateNewSingleValue(), CHIP_NO_ERROR);
    data.GetNewValue() = data_sample_1;
    EXPECT_EQ(data.MarkAsAssigned(), CHIP_NO_ERROR);

    // Validate and commit
    EXPECT_EQ(data.UpdateBegin(nullptr), CHIP_NO_ERROR);
    EXPECT_TRUE(data.UpdateFinish(true));

    EXPECT_EQ(data.GetValue(), data_sample_1);
    EXPECT_TRUE(data.HasValue());

    // Create new value
    EXPECT_EQ(data.SetNewValue(data_sample_2), CHIP_NO_ERROR);
    // Validate and commit
    EXPECT_EQ(data.UpdateBegin(nullptr), CHIP_NO_ERROR);
    EXPECT_TRUE(data.UpdateFinish(true));

    EXPECT_EQ(data.GetValue(), data_sample_2);
    EXPECT_TRUE(data.HasValue());
}

TEST_F(TestCommodityTariffBaseDataClass, ScalarValueNoChangeDetection)
{
    CTC_BaseDataClass<uint32_t> data(1);

    // Set initial value
    EXPECT_EQ(data.SetNewValue(42), CHIP_NO_ERROR);
    EXPECT_EQ(data.UpdateBegin(nullptr), CHIP_NO_ERROR);
    EXPECT_TRUE(data.UpdateFinish(true));

    // Try to set same value again
    EXPECT_EQ(data.SetNewValue(42), CHIP_NO_ERROR);
    EXPECT_EQ(data.UpdateBegin(nullptr), CHIP_NO_ERROR);
    EXPECT_FALSE(data.UpdateFinish(true)); // Should return false (no change)
}

TEST_F(TestCommodityTariffBaseDataClass, NullableValueTransitions)
{
    CTC_BaseDataClass<DataModel::Nullable<uint32_t>> data(1);
    constexpr uint32_t data_sample_1 = 0xde;
    constexpr uint32_t data_sample_2 = 0xad;

    // Set non-null value
    EXPECT_EQ(data.CreateNewSingleValue(), CHIP_NO_ERROR);
    data.GetNewValue().SetNonNull(data_sample_1);
    EXPECT_EQ(data.MarkAsAssigned(), CHIP_NO_ERROR);
    EXPECT_EQ(data.UpdateBegin(nullptr), CHIP_NO_ERROR);
    EXPECT_TRUE(data.UpdateFinish(true));

    EXPECT_FALSE(data.GetValue().IsNull());
    EXPECT_TRUE(data.HasValue());
    EXPECT_EQ(data.GetValue().Value(), data_sample_1);

    EXPECT_EQ(data.SetNewValue(DataModel::MakeNullable(data_sample_2)), CHIP_NO_ERROR);
    EXPECT_EQ(data.UpdateBegin(nullptr), CHIP_NO_ERROR);
    EXPECT_TRUE(data.UpdateFinish(true));

    EXPECT_FALSE(data.GetValue().IsNull());
    EXPECT_TRUE(data.HasValue());
    EXPECT_EQ(data.GetValue().Value(), data_sample_2);

    // Set to null
    EXPECT_EQ(data.SetNewValue(std::nullopt), CHIP_NO_ERROR);
    EXPECT_EQ(data.UpdateBegin(nullptr), CHIP_NO_ERROR);
    EXPECT_TRUE(data.UpdateFinish(true));

    EXPECT_TRUE(data.GetValue().IsNull());
    EXPECT_TRUE(data.HasValue());
}

TEST_F(TestCommodityTariffBaseDataClass, NullableSetNewValue)
{
    CTC_BaseDataClass<DataModel::Nullable<uint32_t>> data(1);
    DataModel::Nullable<uint32_t> newValue;
    constexpr uint32_t data_sample = 200;

    // Test setting null value
    newValue.SetNull();
    EXPECT_EQ(data.SetNewValue(newValue), CHIP_NO_ERROR);
    EXPECT_EQ(data.UpdateBegin(nullptr), CHIP_NO_ERROR);
    EXPECT_FALSE(data.UpdateFinish(true));
    EXPECT_TRUE(data.GetValue().IsNull());

    // Test setting non-null value
    newValue.SetNonNull(data_sample);
    EXPECT_EQ(data.SetNewValue(newValue), CHIP_NO_ERROR);
    EXPECT_EQ(data.UpdateBegin(nullptr), CHIP_NO_ERROR);
    EXPECT_TRUE(data.UpdateFinish(true));
    EXPECT_EQ(data.GetValue().Value(), data_sample);
}

struct MockStruct
{
    uint32_t field1;
    uint16_t field2;
    bool operator!=(const MockStruct & other) const { return field1 != other.field1 || field2 != other.field2; }
};

// Specializations for struct handling
template <>
CHIP_ERROR CTC_BaseDataClass<MockStruct>::CopyData(const StructType & input, StructType & output)
{
    output.field1 = input.field1;
    output.field2 = input.field2;
    return CHIP_NO_ERROR;
}

template <>
CHIP_ERROR CTC_BaseDataClass<MockStruct>::ValidateNewValue()
{
    return CHIP_NO_ERROR;
}

template <>
void CTC_BaseDataClass<MockStruct>::CleanupStruct(StructType & value)
{
    // No special cleanup needed for this simple struct
}

TEST_F(TestCommodityTariffBaseDataClass, StructValueHandling)
{
    CTC_BaseDataClass<MockStruct> data(1);
    constexpr uint32_t sample_field_one = 100;
    constexpr uint16_t sample_field_two = 200;

    MockStruct testStruct = { .field1 = sample_field_one, .field2 = sample_field_two };

    EXPECT_EQ(data.SetNewValue(testStruct), CHIP_NO_ERROR);
    EXPECT_EQ(data.UpdateBegin(nullptr), CHIP_NO_ERROR);
    EXPECT_TRUE(data.UpdateFinish(true));

    EXPECT_EQ(data.GetValue().field1, sample_field_one);
    EXPECT_EQ(data.GetValue().field2, sample_field_two);
}

// Specializations for struct handling
template <>
CHIP_ERROR CTC_BaseDataClass<DataModel::Nullable<MockStruct>>::CopyData(const StructType & input, StructType & output)
{
    output.field1 = input.field1;
    output.field2 = input.field2;
    return CHIP_NO_ERROR;
}

template <>
CHIP_ERROR CTC_BaseDataClass<DataModel::Nullable<MockStruct>>::ValidateNewValue()
{
    return CHIP_NO_ERROR;
}

template <>
void CTC_BaseDataClass<DataModel::Nullable<MockStruct>>::CleanupStruct(StructType & value)
{
    // No special cleanup needed for this simple struct
}

TEST_F(TestCommodityTariffBaseDataClass, NullableStructValueHandling)
{
    CTC_BaseDataClass<DataModel::Nullable<MockStruct>> data(1);
    constexpr uint32_t sample_field_one = 100;
    const uint16_t sample_field_two     = 200;
    MockStruct testStruct               = { sample_field_one, sample_field_two };
    DataModel::Nullable<MockStruct> newValue;

    newValue.SetNonNull(testStruct);

    EXPECT_EQ(data.SetNewValue(newValue), CHIP_NO_ERROR);
    EXPECT_EQ(data.UpdateBegin(nullptr), CHIP_NO_ERROR);
    EXPECT_TRUE(data.UpdateFinish(true));

    EXPECT_EQ(data.GetValue().Value().field1, sample_field_one);
    EXPECT_EQ(data.GetValue().Value().field2, sample_field_two);
}

template <>
CHIP_ERROR CTC_BaseDataClass<DataModel::List<uint32_t>>::ValidateNewValue()
{
    return CHIP_NO_ERROR;
}

TEST_F(TestCommodityTariffBaseDataClass, ListValueCreationAndCleanup)
{
    CTC_BaseDataClass<DataModel::List<uint32_t>> data(1);
    uint32_t SampleArr[]                       = { 10, 20, 30 };
    constexpr uint32_t SampleListLen           = sizeof(SampleArr) / sizeof(uint32_t);
    const DataModel::List<uint32_t> ListSample = DataModel::List<uint32_t>(SampleArr, SampleListLen);

    EXPECT_EQ(data.SetNewValue(ListSample), CHIP_NO_ERROR);
    EXPECT_EQ(data.UpdateBegin(nullptr), CHIP_NO_ERROR);
    EXPECT_TRUE(data.UpdateFinish(true));

    EXPECT_EQ(data.GetValue().size(), SampleListLen);
    EXPECT_EQ(data.GetValue()[0], 10u);
    EXPECT_EQ(data.GetValue()[1], 20u);
    EXPECT_EQ(data.GetValue()[2], 30u);

    data.Cleanup();
    EXPECT_EQ(data.GetValue().data(), nullptr);
}

TEST_F(TestCommodityTariffBaseDataClass, ListValueMemoryManagement)
{
    CTC_BaseDataClass<DataModel::List<uint32_t>> data(1);

    // Test that memory is properly freed
    void * originalPtr = nullptr;

    EXPECT_EQ(data.CreateNewListValue(100), CHIP_NO_ERROR);
    originalPtr = data.GetNewValue().data();
    EXPECT_EQ(data.MarkAsAssigned(), CHIP_NO_ERROR);
    EXPECT_EQ(data.UpdateBegin(nullptr), CHIP_NO_ERROR);
    EXPECT_TRUE(data.UpdateFinish(true));

    // Create new list - should free old memory
    EXPECT_EQ(data.CreateNewListValue(50), CHIP_NO_ERROR);
    // Memory should be different (old was freed)
    EXPECT_NE(data.GetNewValue().data(), originalPtr);

    // Cleanup without commit
    EXPECT_EQ(data.MarkAsAssigned(), CHIP_NO_ERROR);
    EXPECT_EQ(data.UpdateBegin(nullptr), CHIP_NO_ERROR);
    EXPECT_FALSE(data.UpdateFinish(false)); // Don't commit - should cleanup
    EXPECT_EQ(data.GetNewValue().data(), nullptr);
}

TEST_F(TestCommodityTariffBaseDataClass, StateMachineEnforcement)
{
    CTC_BaseDataClass<uint32_t> data(1);

    // Should fail - not in initialized state
    EXPECT_EQ(data.CreateNewSingleValue(), CHIP_NO_ERROR);
    data.GetNewValue() = 0xaau;
    // Should fail - not in assigned state
    EXPECT_EQ(data.UpdateBegin(nullptr), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_FALSE(data.UpdateFinish(false)); // just to cleanup

    // Proper flow
    EXPECT_EQ(data.CreateNewSingleValue(), CHIP_NO_ERROR);
    data.GetNewValue() = 0x55u;
    EXPECT_EQ(data.MarkAsAssigned(), CHIP_NO_ERROR);
    EXPECT_EQ(data.UpdateBegin(nullptr), CHIP_NO_ERROR);
    EXPECT_TRUE(data.UpdateFinish(true));
}

TEST_F(TestCommodityTariffBaseDataClass, DoubleUpdatePrevention)
{
    CTC_BaseDataClass<uint32_t> data(1);

    // Start first update
    EXPECT_EQ(data.CreateNewSingleValue(), CHIP_NO_ERROR);

    // Should fail - already in update
    EXPECT_EQ(data.CreateNewSingleValue(), CHIP_ERROR_INCORRECT_STATE);
}

TEST_F(TestCommodityTariffBaseDataClass, NullableScalarChangeDetection)
{
    CTC_BaseDataClass<DataModel::Nullable<uint32_t>> data(5u);

    // Set initial value
    EXPECT_EQ(data.CreateNewSingleValue(), CHIP_NO_ERROR);
    data.GetNewValue().SetNonNull(50u);
    EXPECT_EQ(data.MarkAsAssigned(), CHIP_NO_ERROR);
    EXPECT_EQ(data.UpdateBegin(nullptr), CHIP_NO_ERROR);
    EXPECT_TRUE(data.UpdateFinish(true));

    // Set same value - should detect no change
    EXPECT_EQ(data.CreateNewSingleValue(), CHIP_NO_ERROR);
    data.GetNewValue().SetNonNull(50u);
    EXPECT_EQ(data.MarkAsAssigned(), CHIP_NO_ERROR);
    EXPECT_EQ(data.UpdateBegin(nullptr), CHIP_NO_ERROR);
    EXPECT_FALSE(data.UpdateFinish(true)); // No change detected

    // Set different value - should detect change
    EXPECT_EQ(data.CreateNewSingleValue(), CHIP_NO_ERROR);
    data.GetNewValue().SetNonNull(100u);
    EXPECT_EQ(data.MarkAsAssigned(), CHIP_NO_ERROR);
    EXPECT_EQ(data.UpdateBegin(nullptr), CHIP_NO_ERROR);
    EXPECT_TRUE(data.UpdateFinish(true)); // Change detected
}

TEST_F(TestCommodityTariffBaseDataClass, ErrorConditions)
{
    CTC_BaseDataClass<DataModel::Nullable<uint32_t>> data(0u);

    // Wrong method call for type
    EXPECT_EQ(data.CreateNewListValue(5), CHIP_ERROR_INTERNAL);

    // Double initialization
    EXPECT_EQ(data.CreateNewSingleValue(), CHIP_NO_ERROR);
    EXPECT_EQ(data.CreateNewSingleValue(), CHIP_ERROR_INCORRECT_STATE);
}

TEST_F(TestCommodityTariffBaseDataClass, UpdateAbort)
{
    CTC_BaseDataClass<DataModel::Nullable<uint32_t>> data(0u);

    // Start update but abort
    EXPECT_EQ(data.CreateNewSingleValue(), CHIP_NO_ERROR);
    data.GetNewValue().SetNonNull(99u);
    EXPECT_EQ(data.MarkAsAssigned(), CHIP_NO_ERROR);

    // Abort without validation
    EXPECT_FALSE(data.UpdateFinish(false));
    EXPECT_FALSE(data.HasNewValue()); // Should not have value after abort
    EXPECT_FALSE(data.HasValue());    // Should not have value after abort
}

// Complex struct with resources that need explicit cleanup
struct TestResourceStruct
{
    uint32_t id;
    CharSpan label;                             // Requires manual memory management
    DataModel::List<const uint32_t> nestedList; // Requires cleanup

    Platform::ScopedMemoryBufferWithSize<char> labelBuffer;
    Platform::ScopedMemoryBufferWithSize<uint32_t> listBuffer;

    CHIP_ERROR fillLabel(const CharSpan & aLabel)
    {
        labelBuffer.CopyFromSpan(aLabel);
        label = CharSpan(labelBuffer.Get(), aLabel.size());
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR fillList(const DataModel::List<const uint32_t> & aList)
    {
        Span<const uint32_t> tmpSpan = Span<const uint32_t>(aList.data(), aList.size());
        listBuffer.CopyFromSpan(tmpSpan);

        nestedList = DataModel::List<const uint32_t>(listBuffer.Get(), tmpSpan.size());
        return CHIP_NO_ERROR;
    }

    void Cleanup()
    {
        if (labelBuffer.Get())
        {
            labelBuffer.Free();
            label = CharSpan();
        }
        if (listBuffer.Get())
        {
            listBuffer.Free();
            nestedList = DataModel::List<const uint32_t>();
        }

        id = 0;
    }

    ~TestResourceStruct() noexcept { Cleanup(); };

    // Proper equality operator
    bool operator==(const TestResourceStruct & other) const
    {
        return id == other.id && this->label.data_equal(other.label) && this->nestedList.data_equal(other.nestedList);
    }

    // Proper inequality operator - should use content comparison, not pointer comparison
    bool operator!=(const TestResourceStruct & other) const { return !(*this == other); }
};

using ComplexType = DataModel::Nullable<DataModel::List<TestResourceStruct>>;

// Specializations for TestResourceStruct handling
template <>
CHIP_ERROR CTC_BaseDataClass<ComplexType>::CopyData(const StructType & input, StructType & output)
{
    output.id = input.id;

    EXPECT_EQ(output.fillLabel(input.label), CHIP_NO_ERROR);
    EXPECT_EQ(output.fillList(input.nestedList), CHIP_NO_ERROR);

    return CHIP_NO_ERROR;
}

template <>
CHIP_ERROR CTC_BaseDataClass<ComplexType>::ValidateNewValue()
{
    return CHIP_NO_ERROR;
}

template <>
void CTC_BaseDataClass<ComplexType>::CleanupStruct(StructType & value)
{
    value.Cleanup();
}

#define TEST_STR_SAMPLE_0 "test1"
#define TEST_STR_SAMPLE_1 "test2"

class TestResourceDataClass : public CTC_BaseDataClass<ComplexType>
{
public:
    TestResourceDataClass() : CTC_BaseDataClass<ComplexType>(1) {}
    ~TestResourceDataClass() override = default;
};

TEST_F(TestCommodityTariffBaseDataClass, NullableListOfResourceStructs_CreationAndCleanup)
{
    constexpr uint32_t IDs[] = { 100, 200 };

    TestResourceStruct ListEntries[] = { {
                                             .id         = 1,
                                             .label      = CharSpan::fromCharString(TEST_STR_SAMPLE_0),
                                             .nestedList = DataModel::List<const uint32_t>(IDs),
                                         },
                                         {
                                             .id         = 2,
                                             .label      = CharSpan::fromCharString(TEST_STR_SAMPLE_1),
                                             .nestedList = DataModel::List<const uint32_t>(),
                                         } };

    ComplexType sourceValue;
    TestResourceDataClass data;

    sourceValue.SetNonNull(ListEntries);

    EXPECT_EQ(data.SetNewValue(sourceValue), CHIP_NO_ERROR);

    EXPECT_EQ(data.UpdateBegin(nullptr), CHIP_NO_ERROR);
    EXPECT_TRUE(data.UpdateFinish(true));

    // Verify data was copied correctly
    EXPECT_FALSE(data.GetValue().IsNull());
    EXPECT_EQ(data.GetValue().Value().size(), 2ul);
    EXPECT_EQ(data.GetValue().Value()[0].id, 1u);

    EXPECT_EQ(data.GetValue().Value()[0].label.size(), strlen(TEST_STR_SAMPLE_0));
    EXPECT_EQ(memcmp(data.GetValue().Value()[0].label.data(), TEST_STR_SAMPLE_0, data.GetValue().Value()[0].label.size()), 0);
    EXPECT_NE(data.GetValue().Value()[0].label.data(), ListEntries[0].label.data());
    EXPECT_EQ(data.GetValue().Value()[0].label.data(), data.GetValue().Value()[0].labelBuffer.Get());

    EXPECT_EQ(data.GetValue().Value()[1].label.size(), strlen(TEST_STR_SAMPLE_1));
    EXPECT_EQ(memcmp(data.GetValue().Value()[1].label.data(), TEST_STR_SAMPLE_1, data.GetValue().Value()[1].label.size()), 0);
    EXPECT_NE(data.GetValue().Value()[1].label.data(), ListEntries[1].label.data());
    EXPECT_EQ(data.GetValue().Value()[1].label.data(), data.GetValue().Value()[1].labelBuffer.Get());

    EXPECT_EQ(data.GetValue().Value()[0].nestedList.size(), 2ul);
    EXPECT_EQ((data.GetValue().Value()[0].nestedList)[0], 100u);
    EXPECT_EQ((data.GetValue().Value()[0].nestedList)[1], 200u);
    EXPECT_NE(data.GetValue().Value()[0].nestedList.data(), ListEntries[0].nestedList.data());
    EXPECT_EQ(data.GetValue().Value()[0].nestedList.data(), data.GetValue().Value()[0].listBuffer.Get());
}

#define TEST_STR_SAMPLE_2 "dynamic_content"

TEST_F(TestCommodityTariffBaseDataClass, NullableListOfResourceStructs_NullTransition)
{
    CTC_BaseDataClass<ComplexType> data(1);
    CharSpan testCharSpan = CharSpan::fromCharString(TEST_STR_SAMPLE_2);

    // First set non-null with resources
    EXPECT_EQ(data.CreateNewListValue(1), CHIP_NO_ERROR);
    data.GetNewValue().Value()[0].id = 1;
    EXPECT_EQ(data.GetNewValue().Value()[0].fillLabel(testCharSpan), CHIP_NO_ERROR);

    EXPECT_EQ(data.MarkAsAssigned(), CHIP_NO_ERROR);
    EXPECT_EQ(data.UpdateBegin(nullptr), CHIP_NO_ERROR);
    EXPECT_TRUE(data.UpdateFinish(true));

    EXPECT_FALSE(data.GetValue().IsNull());

    // Transition to null - should cleanup all resources
    EXPECT_EQ(data.SetNewValue(std::nullopt), CHIP_NO_ERROR);
    EXPECT_EQ(data.UpdateBegin(nullptr), CHIP_NO_ERROR);
    EXPECT_TRUE(data.UpdateFinish(true));

    EXPECT_TRUE(data.GetValue().IsNull());
    // Memory should be properly freed by cleanup
}

TEST_F(TestCommodityTariffBaseDataClass, NullableListOfResourceStructs_UpdateRejectionCleanup)
{
    constexpr uint32_t mItemsCount = 3;
    CTC_BaseDataClass<ComplexType> data(1);

    // Create resource-intensive update but reject it
    EXPECT_EQ(data.CreateNewListValue(3), CHIP_NO_ERROR);

    for (uint32_t i = 0; i < mItemsCount; i++)
    {
        char tmpStrBuf[32]   = { '\0' };
        CharSpan tmpCharSpan = CharSpan::fromCharString(tmpStrBuf);

        data.GetNewValue().Value()[i].id = i;
        sprintf(tmpStrBuf, "resource_%" PRIu32 "", i);
        EXPECT_EQ(data.GetNewValue().Value()[i].fillLabel(tmpCharSpan), CHIP_NO_ERROR);
    }

    EXPECT_EQ(data.MarkAsAssigned(), CHIP_NO_ERROR);

    EXPECT_TRUE(data.HasNewValue());
    EXPECT_FALSE(data.HasValue());

    EXPECT_EQ(data.UpdateBegin(nullptr), CHIP_NO_ERROR);

    EXPECT_FALSE(data.GetNewValue().IsNull());
    EXPECT_TRUE(data.GetValue().IsNull());

    // Reject the update - should cleanup all allocated resources
    EXPECT_FALSE(data.UpdateFinish(false));

    // Verify cleanup happened (no memory leaks)
    EXPECT_TRUE(data.GetNewValue().IsNull());
    EXPECT_TRUE(data.GetValue().IsNull());

    EXPECT_FALSE(data.HasNewValue());
    EXPECT_FALSE(data.HasValue());
}

TEST_F(TestCommodityTariffBaseDataClass, NullableListOfResourceStructs_ZeroSizeList)
{
    CTC_BaseDataClass<ComplexType> data(1);

    // Test empty list creation
    EXPECT_EQ(data.CreateNewListValue(0), CHIP_ERROR_INVALID_LIST_LENGTH);
    EXPECT_TRUE(data.GetNewValue().IsNull()); // Should be null for zero size

    EXPECT_EQ(data.MarkAsAssigned(), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(data.UpdateBegin(nullptr), CHIP_NO_ERROR);
    EXPECT_FALSE(data.UpdateFinish(true));

    EXPECT_TRUE(data.GetValue().IsNull());
}

} // namespace app
} // namespace chip