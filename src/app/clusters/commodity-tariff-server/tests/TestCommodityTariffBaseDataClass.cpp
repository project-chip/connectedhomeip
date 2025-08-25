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

#include <app/clusters/commodity-tariff-server/CommodityTariffAttrsDataMgmt.h>

#include <cstdint>
#include <pw_unit_test/framework.h>

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
    data.UpdateFinish(true);

    EXPECT_FALSE(data.GetValue().IsNull());
    EXPECT_TRUE(data.HasValue());
    EXPECT_EQ(data.GetValue().Value(), data_sample_1);

    data.SetNewValue(DataModel::MakeNullable(data_sample_2));
    data.UpdateBegin(nullptr);
    data.UpdateFinish(true);

    EXPECT_FALSE(data.GetValue().IsNull());
    EXPECT_TRUE(data.HasValue());
    EXPECT_EQ(data.GetValue().Value(), data_sample_2);

    // Set to null
    data.SetNewValue(std::nullopt);
    data.UpdateBegin(nullptr);
    data.UpdateFinish(true);

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
    data.UpdateFinish(true);
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
    data.UpdateFinish(false); // Don't commit - should cleanup
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

TEST_F(TestCommodityTariffBaseDataClass, ChangeDetection) {
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

TEST_F(TestCommodityTariffBaseDataClass, ErrorConditions) {
    CTC_BaseDataClass<DataModel::Nullable<uint32_t>> data(0u);
    
    // Wrong method call for type
    EXPECT_EQ(data.CreateNewListValue(5), CHIP_ERROR_INTERNAL);
    
    // Double initialization
    EXPECT_EQ(data.CreateNewSingleValue(), CHIP_NO_ERROR);
    EXPECT_EQ(data.CreateNewSingleValue(), CHIP_ERROR_INCORRECT_STATE);
    
    // Memory allocation failure (mock test)
    // This would require mocking Platform::MemoryCalloc to return nullptr
}

TEST_F(TestCommodityTariffBaseDataClass, UpdateAbort) {
    CTC_BaseDataClass<DataModel::Nullable<uint32_t>> data(0u);
    
    // Start update but abort
    EXPECT_EQ(data.CreateNewSingleValue(), CHIP_NO_ERROR);
    data.GetNewValue().SetNonNull(99u);
    EXPECT_EQ(data.MarkAsAssigned(), CHIP_NO_ERROR);
    
    // Abort without validation
    EXPECT_FALSE(data.UpdateFinish(false));
    EXPECT_FALSE(data.HasValue()); // Should not have value after abort
}

// Complex struct with resources that need explicit cleanup
struct ResourceStruct {
    uint32_t id;
    char* dynamicString;          // Requires manual memory management
    DataModel::List<uint32_t>* nestedList; // Requires cleanup
    
    ResourceStruct() : id(0), dynamicString(nullptr), nestedList(nullptr) {}
    
    bool operator!=(const ResourceStruct& other) const {
        if (id != other.id) return true;
        if (dynamicString && other.dynamicString) {
            if (strcmp(dynamicString, other.dynamicString) != 0) return true;
        } else if (dynamicString != other.dynamicString) {
            return true; // One is null, other isn't
        }
        // Compare nested lists if both exist
        if (nestedList && other.nestedList) {
            if (nestedList->size() != other.nestedList->size()) return true;
            for (size_t i = 0; i < nestedList->size(); i++) {
                if ((*nestedList)[i] != (*other.nestedList)[i]) return true;
            }
        } else if (nestedList != other.nestedList) {
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
    if (input.dynamicString) {
        output.dynamicString = static_cast<char*>(Platform::MemoryCalloc(strlen(input.dynamicString) + 1, 1));
        if (!output.dynamicString) {
            return CHIP_ERROR_NO_MEMORY;
        }
        strcpy(output.dynamicString, input.dynamicString);
    } else {
        output.dynamicString = nullptr;
    }
    
    // Copy nested list
    if (input.nestedList) {
        output.nestedList = static_cast<DataModel::List<uint32_t>*>(
            Platform::MemoryCalloc(1, sizeof(DataModel::List<uint32_t>)));
        if (!output.nestedList) {
            Platform::MemoryFree(output.dynamicString);
            return CHIP_ERROR_NO_MEMORY;
        }
        
        output.nestedList->~List<uint32_t>(); // Properly destruct before placement new
        new (output.nestedList) DataModel::List<uint32_t>();
        
        if (input.nestedList->size() > 0) {
            auto* buffer = static_cast<uint32_t*>(
                Platform::MemoryCalloc(input.nestedList->size(), sizeof(uint32_t)));
            if (!buffer) {
                Platform::MemoryFree(output.dynamicString);
                Platform::MemoryFree(output.nestedList);
                return CHIP_ERROR_NO_MEMORY;
            }
            for (size_t i = 0; i < input.nestedList->size(); i++) {
                buffer[i] = (*input.nestedList)[i];
            }
            *output.nestedList = DataModel::List<uint32_t>(buffer, input.nestedList->size());
        }
    } else {
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
    if (value.dynamicString) {
        Platform::MemoryFree(value.dynamicString);
        value.dynamicString = nullptr;
    }
    
    // Cleanup nested list
    if (value.nestedList) {
        if (value.nestedList->data()) {
            Platform::MemoryFree(value.nestedList->data());
        }
        Platform::MemoryFree(value.nestedList);
        value.nestedList = nullptr;
    }
    
    value.id = 0;
}

TEST_F(TestCommodityTariffBaseDataClass, NullableListOfResourceStructs_CreationAndCleanup)
{
    CTC_BaseDataClass<ComplexType> data(1);
    
    // Create list with resource-intensive structs
    EXPECT_EQ(data.CreateNewListValue(2), CHIP_NO_ERROR);
    
    auto& newList = data.GetNewValue().Value();
    
    // First struct with dynamic resources
    newList[0].id = 1;
    newList[0].dynamicString = static_cast<char*>(Platform::MemoryCalloc(10, 1));
    strcpy(newList[0].dynamicString, "test1");
    
    newList[0].nestedList = static_cast<DataModel::List<uint32_t>*>(
        Platform::MemoryCalloc(1, sizeof(DataModel::List<uint32_t>)));
    new (newList[0].nestedList) DataModel::List<uint32_t>();
    auto* nestedBuffer1 = static_cast<uint32_t*>(Platform::MemoryCalloc(2, sizeof(uint32_t)));
    nestedBuffer1[0] = 100;
    nestedBuffer1[1] = 200;
    *newList[0].nestedList = DataModel::List<uint32_t>(nestedBuffer1, 2);
    
    // Second struct
    newList[1].id = 2;
    newList[1].dynamicString = static_cast<char*>(Platform::MemoryCalloc(10, 1));
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
    sourceValue.SetNonNull(DataModel::List<ResourceStruct>());
    
    // Create a struct with resources
    ResourceStruct testStruct;
    testStruct.id = 42;
    testStruct.dynamicString = static_cast<char*>(Platform::MemoryCalloc(20, 1));
    strcpy(testStruct.dynamicString, "dynamic_content");
    
    // Use SetNewValue to copy the complex data
    EXPECT_EQ(data.SetNewValue(sourceValue), CHIP_NO_ERROR);
    
    data.UpdateBegin(nullptr);
    data.UpdateFinish(true);
    
    // Cleanup source (should not affect the copied data)
    //data.CleanupExtListEntry(testStruct);
    //EXPECT_EQ(data.GetValue().Value().size(), 1ul);
    //EXPECT_STREQ(data.GetValue().Value()[0].dynamicString, "dynamic_content");
}
/*
TEST_F(TestCommodityTariffBaseDataClass, NullableListOfResourceStructs_NullTransition)
{
    CTC_BaseDataClass<ComplexType> data(1);
    
    // First set non-null with resources
    data.CreateNewListValue(1);
    data.GetNewValue().Value()[0].id = 1;
    data.GetNewValue().Value()[0].dynamicString = static_cast<char*>(Platform::MemoryCalloc(10, 1));
    strcpy(data.GetNewValue().Value()[0].dynamicString, "test");
    
    data.MarkAsAssigned();
    data.UpdateBegin(nullptr);
    data.UpdateFinish(true);
    
    EXPECT_FALSE(data.GetValue().IsNull());
    
    // Transition to null - should cleanup all resources
    data.CreateNewSingleValue();
    data.GetNewValue().SetNull();
    data.MarkAsAssigned();
    data.UpdateBegin(nullptr);
    data.UpdateFinish(true);
    
    EXPECT_TRUE(data.GetValue().IsNull());
    // Memory should be properly freed by cleanup
}

TEST_F(TestCommodityTariffBaseDataClass, NullableListOfResourceStructs_UpdateRejectionCleanup)
{
    CTC_BaseDataClass<ComplexType> data(1);
    
    // Create resource-intensive update but reject it
    data.CreateNewListValue(3);
    
    for (int i = 0; i < 3; i++) {
        data.GetNewValue().Value()[i].id = i;
        data.GetNewValue().Value()[i].dynamicString = static_cast<char*>(Platform::MemoryCalloc(20, 1));
        sprintf(data.GetNewValue().Value()[i].dynamicString, "resource_%d", i);
    }
    
    data.MarkAsAssigned();
    data.UpdateBegin(nullptr);
    
    // Reject the update - should cleanup all allocated resources
    EXPECT_FALSE(data.UpdateFinish(false));
    
    // Verify cleanup happened (no memory leaks)
    EXPECT_FALSE(data.HasNewValue());
}

TEST_F(TestCommodityTariffBaseDataClass, NullableListOfResourceStructs_ZeroSizeList)
{
    CTC_BaseDataClass<ComplexType> data(1);
    
    // Test empty list creation
    EXPECT_EQ(data.CreateNewListValue(0), CHIP_NO_ERROR);
    EXPECT_TRUE(data.GetNewValue().IsNull()); // Should be null for zero size
    
    data.MarkAsAssigned();
    data.UpdateBegin(nullptr);
    data.UpdateFinish(true);
    
    EXPECT_TRUE(data.GetValue().IsNull());
}

TEST_F(TestCommodityTariffBaseDataClass, NullableListOfResourceStructs_MemoryAllocationFailure)
{
    CTC_BaseDataClass<ComplexType> data(1);
    
    // Mock memory allocation failure scenario
    // (This would typically use a memory allocator mock)
    
    // Test that failed allocation leaves object in consistent state

    EXPECT_EQ(data.CreateNewListValue(1000000), CHIP_ERROR_NO_MEMORY);

    EXPECT_EQ(data.MarkAsAssigned(), CHIP_ERROR_INCORRECT_STATE);
}*/

} // namespace app
} // namespace chip
