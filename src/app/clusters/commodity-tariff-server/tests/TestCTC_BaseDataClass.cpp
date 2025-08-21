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

#include <vector>

#include <app/clusters/zone-management-server/zone-geometry.h>
#include <app/clusters/zone-management-server/zone-management-server.h>

#include <pw_unit_test/framework.h>

#include <lib/support/CodeUtils.h>

using namespace chip;
using namespace chip::Test;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::CommodityTariffAttrsDataMgmt;

// =================================
//      Unit tests
// =================================

namespace chip {
namespace app {

class TestCTC_BaseDataClass : public ::testing::Test
{
public:
    static void SetUpTestSuite() {}
    static void TearDownTestSuite() {}
};

TEST_F(TestCTC_BaseDataClass, BasicConstruction)
{
    // Test construction with different types
    CTC_BaseDataClass<uint32_t> intData(1);
    CTC_BaseDataClass<DataModel::Nullable<uint32_t>> nullableData(2);
    CTC_BaseDataClass<DataModel::List<uint32_t>> listData(3);
    
    // Verify initial states
    EXPECT_EQ(intData.HasValue(), false);
    EXPECT_EQ(nullableData.GetValue().IsNull(), true); // Nullable starts as null  
    EXPECT_EQ(nullableData.HasValue(), false);  // Nullable starts as null
    EXPECT_EQ(listData.GetValue().size() == 0, true); // List starts as empty
    EXPECT_EQ(listData.HasValue(), false); 
}

TEST_F(TestCTC_BaseDataClass, TypeTraitsDetection)
{
    // Test type categorization
    EXPECT_TRUE(CTC_BaseDataClass<uint32_t>::IsValueScalar());
    EXPECT_TRUE(CTC_BaseDataClass<DataModel::Nullable<uint32_t>>::IsValueNullable());
    EXPECT_TRUE(CTC_BaseDataClass<DataModel::List<uint32_t>>::IsValueList());
}

TEST_F(TestCTC_BaseDataClass, ScalarValueUpdateFlow)
{
    CTC_BaseDataClass<uint32_t> data(1);
    
    // Create new value
    EXPECT_EQ(data.CreateNewSingleValue(), CHIP_NO_ERROR);
    data.GetNewValue() = 42;
    EXPECT_EQ(data.MarkAsAssigned(), CHIP_NO_ERROR);
    
    // Validate and commit
    EXPECT_EQ(data.UpdateBegin(nullptr), CHIP_NO_ERROR);
    EXPECT_TRUE(data.UpdateFinish(true));
    
    EXPECT_EQ(data.GetValue(), 42);
    EXPECT_TRUE(data.HasValue());

    // Create new value
    EXPECT_EQ(data.SetNewValue(42), CHIP_NO_ERROR);
    // Validate and commit
    EXPECT_EQ(data.UpdateBegin(nullptr), CHIP_NO_ERROR);
    EXPECT_TRUE(data.UpdateFinish(true));
    
    EXPECT_EQ(data.GetValue(), 42);
    EXPECT_TRUE(data.HasValue());
}

TEST_F(TestCTC_BaseDataClass, ScalarValueNoChangeDetection)
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

} // namespace app
} // namespace chip
