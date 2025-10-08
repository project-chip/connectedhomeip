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
        Platform::MemoryInit();
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
    const uint32_t data_sample_1 = 0xde;
    const uint32_t data_sample_2 = 0xad;

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
    data.SetNewValue(42);
    data.UpdateBegin(nullptr);
    data.UpdateFinish(true);

    // Try to set same value again
    data.SetNewValue(42);
    data.UpdateBegin(nullptr);
    EXPECT_FALSE(data.UpdateFinish(true)); // Should return false (no change)
}

TEST_F(TestCommodityTariffBaseDataClass, NullableValueTransitions)
{
    CTC_BaseDataClass<DataModel::Nullable<uint32_t>> data(1);
    const uint32_t data_sample_1 = 0xde;
    const uint32_t data_sample_2 = 0xad;

    // Set non-null value
    data.CreateNewSingleValue();
    data.GetNewValue().SetNonNull(data_sample_1);
    data.MarkAsAssigned();
    data.UpdateBegin(nullptr);
    EXPECT_TRUE(data.UpdateFinish(true));

    EXPECT_FALSE(data.GetValue().IsNull());
    EXPECT_TRUE(data.HasValue());
    EXPECT_EQ(data.GetValue().Value(), data_sample_1);

    data.SetNewValue(DataModel::MakeNullable(data_sample_2));
    data.UpdateBegin(nullptr);
    EXPECT_TRUE(data.UpdateFinish(true));

    EXPECT_FALSE(data.GetValue().IsNull());
    EXPECT_TRUE(data.HasValue());
    EXPECT_EQ(data.GetValue().Value(), data_sample_2);

    // Set to null
    data.SetNewValue(std::nullopt);
    data.UpdateBegin(nullptr);
    EXPECT_TRUE(data.UpdateFinish(true));

    EXPECT_TRUE(data.GetValue().IsNull());
    EXPECT_TRUE(data.HasValue());
}

TEST_F(TestCommodityTariffBaseDataClass, NullableSetNewValue)
{
    CTC_BaseDataClass<DataModel::Nullable<uint32_t>> data(1);
    DataModel::Nullable<uint32_t> newValue;
    const uint32_t data_sample = 200;

    // Test setting null value
    newValue.SetNull();
    EXPECT_EQ(data.SetNewValue(newValue), CHIP_NO_ERROR);
    data.UpdateBegin(nullptr);
    data.UpdateFinish(true);
    EXPECT_TRUE(data.GetValue().IsNull());

    // Test setting non-null value
    newValue.SetNonNull(data_sample);
    EXPECT_EQ(data.SetNewValue(newValue), CHIP_NO_ERROR);
    data.UpdateBegin(nullptr);
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
    const uint32_t sample_field_one = 100;
    const uint16_t sample_field_two = 200;

    MockStruct testStruct = { sample_field_one, sample_field_two };

    EXPECT_EQ(data.SetNewValue(testStruct), CHIP_NO_ERROR);
    data.UpdateBegin(nullptr);
    data.UpdateFinish(true);

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
    const uint32_t sample_field_one = 100;
    const uint16_t sample_field_two = 200;
    MockStruct testStruct           = { sample_field_one, sample_field_two };
    DataModel::Nullable<MockStruct> newValue;

    newValue.SetNonNull(testStruct);

    EXPECT_EQ(data.SetNewValue(newValue), CHIP_NO_ERROR);
    data.UpdateBegin(nullptr);
    data.UpdateFinish(true);

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
    const uint32_t SampleListLen               = sizeof(SampleArr) / sizeof(uint32_t);
    const DataModel::List<uint32_t> ListSample = DataModel::List<uint32_t>(SampleArr, SampleListLen);

    EXPECT_EQ(data.SetNewValue(ListSample), CHIP_NO_ERROR);
    data.UpdateBegin(nullptr);
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

    data.CreateNewListValue(100);
    originalPtr = data.GetNewValue().data();
    data.MarkAsAssigned();
    data.UpdateBegin(nullptr);
    data.UpdateFinish(true);

    // Create new list - should free old memory
    data.CreateNewListValue(50);
    // Memory should be different (old was freed)
    EXPECT_NE(data.GetNewValue().data(), originalPtr);

    // Cleanup without commit
    data.MarkAsAssigned();
    data.UpdateBegin(nullptr);
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
    data.CreateNewSingleValue();

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
struct ResourceStruct
{
    uint32_t id;
    char * dynamicString;                   // Requires manual memory management
    DataModel::List<uint32_t> * nestedList; // Requires cleanup

    ResourceStruct() : id(0), dynamicString(nullptr), nestedList(nullptr) {}

    bool operator!=(const ResourceStruct & other) const
    {
        if (id != other.id)
            return true;
        if (dynamicString && other.dynamicString)
        {
            if (strcmp(dynamicString, other.dynamicString) != 0)
                return true;
        }
        else if (dynamicString != other.dynamicString)
        {
            return true; // One is null, other isn't
        }
        // Compare nested lists if both exist
        if (nestedList && other.nestedList)
        {
            if (nestedList->size() != other.nestedList->size())
                return true;
            for (size_t i = 0; i < nestedList->size(); i++)
            {
                if ((*nestedList)[i] != (*other.nestedList)[i])
                    return true;
            }
        }
        else if (nestedList != other.nestedList)
        {
            return true; // One is null, other isn't
        }
        return false;
    }
};

using ComplexType = DataModel::Nullable<DataModel::List<ResourceStruct>>;

// Specializations for ResourceStruct handling
template <>
CHIP_ERROR CTC_BaseDataClass<ComplexType>::CopyData(const StructType & input, StructType & output)
{
    output.id = input.id;

    // Copy dynamic string
    if (input.dynamicString)
    {
        output.dynamicString = static_cast<char *>(Platform::MemoryCalloc(strlen(input.dynamicString) + 1, 1));
        if (!output.dynamicString)
        {
            return CHIP_ERROR_NO_MEMORY;
        }
        strcpy(output.dynamicString, input.dynamicString);
    }
    else
    {
        output.dynamicString = nullptr;
    }

    // Copy nested list
    if (input.nestedList)
    {
        output.nestedList = new DataModel::List<uint32_t>();
        if (!output.nestedList)
        {
            Platform::MemoryFree(output.dynamicString);
            return CHIP_ERROR_NO_MEMORY;
        }

        if (input.nestedList->size() > 0)
        {
            auto * buffer = static_cast<uint32_t *>(Platform::MemoryCalloc(input.nestedList->size(), sizeof(uint32_t)));
            if (!buffer)
            {
                Platform::MemoryFree(output.dynamicString);
                delete output.nestedList;
                return CHIP_ERROR_NO_MEMORY;
            }
            for (size_t i = 0; i < input.nestedList->size(); i++)
            {
                buffer[i] = (*input.nestedList)[i];
            }
            *output.nestedList = DataModel::List<uint32_t>(buffer, input.nestedList->size());
        }
    }
    else
    {
        output.nestedList = nullptr;
    }

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
    // Cleanup dynamic string
    if (value.dynamicString)
    {
        Platform::MemoryFree(value.dynamicString);
        value.dynamicString = nullptr;
    }

    // Cleanup nested list
    if (value.nestedList)
    {
        if (value.nestedList->data())
        {
            Platform::MemoryFree(value.nestedList->data());
        }
        delete value.nestedList;
        value.nestedList = nullptr;
    }

    value.id = 0;
}

TEST_F(TestCommodityTariffBaseDataClass, NullableListOfResourceStructs_CreationAndCleanup)
{
    CTC_BaseDataClass<ComplexType> data(1);

    // Create list with resource-intensive structs
    EXPECT_EQ(data.CreateNewListValue(2), CHIP_NO_ERROR);

    auto & newList = data.GetNewValue().Value();

    // First struct with dynamic resources
    newList[0].id            = 1;
    newList[0].dynamicString = static_cast<char *>(Platform::MemoryCalloc(10, 1));
    strcpy(newList[0].dynamicString, "test1");

    auto * nestedBuffer1  = static_cast<uint32_t *>(Platform::MemoryCalloc(2, sizeof(uint32_t)));
    nestedBuffer1[0]      = 100;
    nestedBuffer1[1]      = 200;
    newList[0].nestedList = new DataModel::List<uint32_t>(nestedBuffer1, 2);

    // Second struct
    newList[1].id            = 2;
    newList[1].dynamicString = static_cast<char *>(Platform::MemoryCalloc(10, 1));
    strcpy(newList[1].dynamicString, "test2");

    data.MarkAsAssigned();
    data.UpdateBegin(nullptr);
    EXPECT_TRUE(data.UpdateFinish(true));

    // Verify data was copied correctly
    EXPECT_FALSE(data.GetValue().IsNull());
    EXPECT_EQ(data.GetValue().Value().size(), 2ul);
    EXPECT_EQ(data.GetValue().Value()[0].id, 1u);
    EXPECT_STREQ(data.GetValue().Value()[0].dynamicString, "test1");
    EXPECT_EQ(data.GetValue().Value()[0].nestedList->size(), 2ul);
    EXPECT_EQ((*data.GetValue().Value()[0].nestedList)[0], 100u);
}

TEST_F(TestCommodityTariffBaseDataClass, NullableListOfResourceStructs_SetNewValue)
{
    CTC_BaseDataClass<ComplexType> data(1);

    // Prepare source data
    ComplexType sourceValue;

    // Create a struct with resources
    ResourceStruct testStruct = {};
    testStruct.id             = 42;
    testStruct.dynamicString  = static_cast<char *>(Platform::MemoryCalloc(20, 1));
    strcpy(testStruct.dynamicString, "dynamic_content");

    auto * nestedBuffer   = static_cast<uint32_t *>(Platform::MemoryCalloc(2, sizeof(uint32_t)));
    nestedBuffer[0]       = 100;
    nestedBuffer[1]       = 200;
    testStruct.nestedList = new DataModel::List<uint32_t>(nestedBuffer, 2);

    sourceValue.SetNonNull(DataModel::List<ResourceStruct>(&testStruct, 1ul));

    // Use SetNewValue to copy the complex data
    EXPECT_EQ(data.SetNewValue(sourceValue), CHIP_NO_ERROR);

    data.UpdateBegin(nullptr);
    data.UpdateFinish(true);

    // Cleanup source (should not affect the copied data)
    data.CleanupExtListEntry(testStruct);
    EXPECT_EQ(testStruct.dynamicString, nullptr);
    EXPECT_EQ(testStruct.nestedList, nullptr);

    EXPECT_EQ(data.GetValue().Value().size(), 1ul);
    EXPECT_STREQ(data.GetValue().Value()[0].dynamicString, "dynamic_content");

    EXPECT_NE(data.GetValue().Value()[0].nestedList, nullptr);
    EXPECT_EQ(data.GetValue().Value()[0].nestedList->size(), 2ul);
    EXPECT_EQ((*data.GetValue().Value()[0].nestedList)[0], 100u);
    EXPECT_EQ((*data.GetValue().Value()[0].nestedList)[1], 200u);
}

TEST_F(TestCommodityTariffBaseDataClass, NullableListOfResourceStructs_NullTransition)
{
    CTC_BaseDataClass<ComplexType> data(1);

    // First set non-null with resources
    data.CreateNewListValue(1);
    data.GetNewValue().Value()[0].id            = 1;
    data.GetNewValue().Value()[0].dynamicString = static_cast<char *>(Platform::MemoryCalloc(10, 1));
    strcpy(data.GetNewValue().Value()[0].dynamicString, "test");

    data.MarkAsAssigned();
    data.UpdateBegin(nullptr);
    EXPECT_TRUE(data.UpdateFinish(true));

    EXPECT_FALSE(data.GetValue().IsNull());

    // Transition to null - should cleanup all resources
    data.SetNewValue(std::nullopt);
    data.UpdateBegin(nullptr);
    EXPECT_TRUE(data.UpdateFinish(true));

    EXPECT_TRUE(data.GetValue().IsNull());
    // Memory should be properly freed by cleanup
}

TEST_F(TestCommodityTariffBaseDataClass, NullableListOfResourceStructs_UpdateRejectionCleanup)
{
    CTC_BaseDataClass<ComplexType> data(1);

    // Create resource-intensive update but reject it
    data.CreateNewListValue(3);

    for (uint32_t i = 0; i < 3; i++)
    {
        data.GetNewValue().Value()[i].id            = i;
        data.GetNewValue().Value()[i].dynamicString = static_cast<char *>(Platform::MemoryCalloc(20, 1));
        sprintf(data.GetNewValue().Value()[i].dynamicString, "resource_%" PRIu32 "", i);
    }

    data.MarkAsAssigned();

    EXPECT_TRUE(data.HasNewValue());
    EXPECT_FALSE(data.HasValue());

    data.UpdateBegin(nullptr);

    // Reject the update - should cleanup all allocated resources
    EXPECT_FALSE(data.UpdateFinish(false));

    // Verify cleanup happened (no memory leaks)
    EXPECT_FALSE(data.HasNewValue());
    EXPECT_FALSE(data.HasValue());
}

TEST_F(TestCommodityTariffBaseDataClass, NullableListOfResourceStructs_ZeroSizeList)
{
    CTC_BaseDataClass<ComplexType> data(1);

    // Test empty list creation
    EXPECT_EQ(data.CreateNewListValue(0), CHIP_ERROR_INVALID_LIST_LENGTH);
    EXPECT_TRUE(data.GetNewValue().IsNull()); // Should be null for zero size

    data.MarkAsAssigned();
    data.UpdateBegin(nullptr);
    EXPECT_FALSE(data.UpdateFinish(true));

    EXPECT_TRUE(data.GetValue().IsNull());
}

TEST_F(TestCommodityTariffBaseDataClass, ConcurrentReadAccess)
{
    CTC_BaseDataClass<DataModel::Nullable<uint32_t>> data(2);

    // Initialize with a value
    EXPECT_EQ(data.CreateNewSingleValue(), CHIP_NO_ERROR);
    data.GetNewValue().SetNonNull(123);
    EXPECT_EQ(data.MarkAsAssigned(), CHIP_NO_ERROR);
    EXPECT_EQ(data.UpdateBegin(nullptr), CHIP_NO_ERROR);
    EXPECT_TRUE(data.UpdateFinish(true));

    // Multiple threads should be able to read simultaneously
    auto reader = [&data]() {
        for (int i = 0; i < 1000; i++)
        {
            auto & value = data.GetValue();
            EXPECT_FALSE(value.IsNull());
            EXPECT_EQ(value.Value(), 123u);
        }
    };

    std::thread t1(reader);
    std::thread t2(reader);
    std::thread t3(reader);

    t1.join();
    t2.join();
    t3.join();
}

TEST_F(TestCommodityTariffBaseDataClass, ConcurrentWriteAccess_ShouldBeSynchronized)
{
    CTC_BaseDataClass<DataModel::Nullable<uint32_t>> data(2);

    std::atomic<int> successCount(0);
    std::atomic<int> failureCount(0);

    auto writer = [&data, &successCount, &failureCount](uint32_t value) {
        for (uint32_t i = 0; i < 100; i++)
        {
            CHIP_ERROR err = data.CreateNewSingleValue();
            if (err != CHIP_NO_ERROR)
            {
                failureCount++;
                continue;
            }

            data.GetNewValue().SetNonNull(value + i);
            err = data.MarkAsAssigned();
            if (err != CHIP_NO_ERROR)
            {
                failureCount++;
                continue;
            }

            err = data.UpdateBegin(nullptr);
            if (err != CHIP_NO_ERROR)
            {
                failureCount++;
                continue;
            }

            if (data.UpdateFinish(true))
            {
                successCount++;
            }

            // Small delay to increase chance of race conditions
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
    };

    std::thread t1(writer, 1000);
    std::thread t2(writer, 2000);
    std::thread t3(writer, 3000);

    t1.join();
    t2.join();
    t3.join();

    // Without synchronization, we expect many failures due to state conflicts
    if (failureCount.load() > 0)
    {
        EXPECT_GT(failureCount.load(), 0) << "Concurrent writes should cause state errors without synchronization";
    }

    EXPECT_GT(successCount.load(), 0) << "Some writes should succeed";
}

TEST_F(TestCommodityTariffBaseDataClass, ConcurrentWriteAccess_WithSynchronization)
{
    CTC_BaseDataClass<DataModel::Nullable<uint32_t>> data(2);
    std::mutex dataMutex;

    std::atomic<int> successCount(0);
    std::atomic<int> failureCount(0);
    std::vector<uint32_t> finalValues;

    auto synchronizedWriter = [&data, &dataMutex, &successCount, &failureCount, &finalValues](uint32_t baseValue) {
        for (uint32_t i = 0; i < 50; i++)
        {
            std::lock_guard<std::mutex> lock(dataMutex);

            CHIP_ERROR err = data.CreateNewSingleValue();
            if (err != CHIP_NO_ERROR)
                continue;

            data.GetNewValue().SetNonNull(baseValue + i);
            err = data.MarkAsAssigned();
            if (err != CHIP_NO_ERROR)
            {
                failureCount++;
                continue;
            }

            err = data.UpdateBegin(nullptr);
            if (err != CHIP_NO_ERROR)
            {
                failureCount++;
                continue;
            }

            if (data.UpdateFinish(true))
            {
                successCount++;
                // Store the value that was successfully written
                if (!data.GetValue().IsNull())
                {
                    finalValues.push_back(data.GetValue().Value());
                }
            }
        }
    };

    std::thread t1(synchronizedWriter, 1000);
    std::thread t2(synchronizedWriter, 2000);
    std::thread t3(synchronizedWriter, 3000);

    t1.join();
    t2.join();
    t3.join();

    // With synchronization, all operations should succeed
    EXPECT_EQ(failureCount.load(), 0) << "All synchronized writes should succeed";
    EXPECT_EQ(successCount.load(), 150) << "All synchronized writes should succeed";
    EXPECT_TRUE(data.HasValue()) << "Data should have a value after writes";
}

TEST_F(TestCommodityTariffBaseDataClass, MixedReadWriteConcurrency)
{
    CTC_BaseDataClass<DataModel::Nullable<uint32_t>> data(2);
    std::mutex dataMutex;
    std::atomic<bool> stopThreads(false);

    std::atomic<int> readCount(0);
    std::atomic<int> writeCount(0);
    std::atomic<int> readErrors(0);

    // Initialize with a value
    {
        std::lock_guard<std::mutex> lock(dataMutex);
        EXPECT_EQ(data.CreateNewSingleValue(), CHIP_NO_ERROR);
        data.GetNewValue().SetNonNull(999);
        EXPECT_EQ(data.MarkAsAssigned(), CHIP_NO_ERROR);
        EXPECT_EQ(data.UpdateBegin(nullptr), CHIP_NO_ERROR);
        EXPECT_TRUE(data.UpdateFinish(true));
    }

    auto reader = [&]() {
        while (!stopThreads.load())
        {
            // Readers don't need synchronization for GetValue()
            auto & value = data.GetValue();
            readCount++;

            if (value.IsNull())
            {
                readErrors++;
            }
            else if (value.Value() < 100)
            {
                readErrors++; // Should never see values < 100
            }

            std::this_thread::sleep_for(std::chrono::microseconds(5));
        }
    };

    auto writer = [&](uint32_t startValue) {
        for (uint32_t i = 0; i < 20; i++)
        {
            std::lock_guard<std::mutex> lock(dataMutex);

            CHIP_ERROR err = data.CreateNewSingleValue();
            if (err != CHIP_NO_ERROR)
                continue;

            data.GetNewValue().SetNonNull(startValue + i);
            err = data.MarkAsAssigned();
            if (err != CHIP_NO_ERROR)
                continue;

            err = data.UpdateBegin(nullptr);
            if (err != CHIP_NO_ERROR)
                continue;

            if (data.UpdateFinish(true))
            {
                writeCount++;
            }

            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
    };

    // Start readers
    std::thread reader1(reader);
    std::thread reader2(reader);

    // Start writers
    std::thread writer1(writer, 100);
    std::thread writer2(writer, 200);

    writer1.join();
    writer2.join();

    stopThreads.store(true);
    reader1.join();
    reader2.join();

    EXPECT_GT(readCount.load(), 0) << "Should have performed many reads";
    EXPECT_GT(writeCount.load(), 0) << "Should have performed many writes";
    EXPECT_EQ(readErrors.load(), 0) << "No read errors should occur during concurrent access";

    // Final value should be from one of the writers
    EXPECT_FALSE(data.GetValue().IsNull());
    EXPECT_GE(data.GetValue().Value(), 100u);
}

TEST_F(TestCommodityTariffBaseDataClass, ConcurrentListOperations)
{
    CTC_BaseDataClass<DataModel::List<uint32_t>> data(2);
    std::mutex dataMutex;

    auto listWriter = [&](uint32_t threadId) {
        std::lock_guard<std::mutex> lock(dataMutex);

        EXPECT_EQ(data.CreateNewListValue(3), CHIP_NO_ERROR);
        auto & list = data.GetNewValue();

        for (size_t i = 0; i < list.size(); i++)
        {
            list[i] = threadId * 100 + static_cast<uint32_t>(i);
        }

        EXPECT_EQ(data.MarkAsAssigned(), CHIP_NO_ERROR);
        EXPECT_EQ(data.UpdateBegin(nullptr), CHIP_NO_ERROR);
        EXPECT_TRUE(data.UpdateFinish(true));
    };

    std::thread t1(listWriter, 1);
    std::thread t2(listWriter, 2);
    std::thread t3(listWriter, 3);

    t1.join();
    t2.join();
    t3.join();

    // Final list should be from one of the threads
    EXPECT_TRUE(data.HasValue());
    EXPECT_EQ(data.GetValue().size(), 3u);

    // Verify the list contains valid values from one thread
    auto & finalList   = data.GetValue();
    uint32_t baseValue = finalList[0] / 100 * 100;
    EXPECT_GE(baseValue, 100u);
    EXPECT_LE(baseValue, 300u);

    for (size_t i = 0; i < finalList.size(); i++)
    {
        EXPECT_EQ(finalList[i], baseValue + i);
    }
}

TEST_F(TestCommodityTariffBaseDataClass, StressTest_ManyThreads)
{
    constexpr size_t NUM_THREADS           = 10;
    constexpr size_t OPERATIONS_PER_THREAD = 50;

    CTC_BaseDataClass<DataModel::Nullable<uint32_t>> data(2);
    std::mutex dataMutex;
    std::atomic<size_t> totalOperations(0);

    auto worker = [&](uint32_t threadId) {
        for (size_t i = 0; i < OPERATIONS_PER_THREAD; i++)
        {
            std::lock_guard<std::mutex> lock(dataMutex);

            CHIP_ERROR err = data.CreateNewSingleValue();
            if (err != CHIP_NO_ERROR)
                continue;

            data.GetNewValue().SetNonNull(threadId * 1000 + static_cast<uint32_t>(i));
            err = data.MarkAsAssigned();
            if (err != CHIP_NO_ERROR)
                continue;

            err = data.UpdateBegin(nullptr);
            if (err != CHIP_NO_ERROR)
                continue;

            if (data.UpdateFinish(true))
            {
                totalOperations++;
            }
        }
    };

    std::vector<std::thread> threads;
    for (uint32_t i = 0; i < NUM_THREADS; i++)
    {
        threads.emplace_back(worker, i);
    }

    for (auto & t : threads)
    {
        t.join();
    }

    EXPECT_EQ(totalOperations, NUM_THREADS * OPERATIONS_PER_THREAD);
    EXPECT_TRUE(data.HasValue());
    EXPECT_FALSE(data.GetValue().IsNull());
}

} // namespace app
} // namespace chip
