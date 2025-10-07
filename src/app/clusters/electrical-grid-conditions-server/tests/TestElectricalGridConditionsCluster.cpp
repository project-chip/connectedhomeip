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

#include <app/clusters/electrical-grid-conditions-server/electrical-grid-conditions-server.h>
#include <memory>
#include <platform/CHIPDeviceLayer.h>
#include <pw_unit_test/framework.h>
#include <vector>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ElectricalGridConditions;
using namespace chip::app::Clusters::ElectricalGridConditions::Structs;

// Global variable for mock testing - must be outside anonymous namespace for linkage
bool gMatterReportingCalled = false;

// This replaces the implementation in src/app/reporting/reporting.cpp
void MatterReportingAttributeChangeCallback(uint16_t endpoint, uint32_t clusterId, uint32_t attributeId)
{
    printf("Mocked MatterReportingAttributeChangeCallback(ep=%u, cluster=%u, attr=%u)\n", endpoint, clusterId, attributeId);
    gMatterReportingCalled = true;
}

namespace {

constexpr EndpointId kTestEndpointId = 1;

// Test utility for MatterReporting verification
class MockMatterReporting
{
public:
    static void Reset() { gMatterReportingCalled = false; }

    static bool WasCalled() { return gMatterReportingCalled; }
};

// Helper function to create test conditions
namespace TestHelpers {
ElectricalGridConditionsStruct::Type CreateTestCondition(uint32_t periodStart, Optional<uint32_t> periodEnd = Optional<uint32_t>(),
                                                         Optional<int16_t> gridCarbonIntensity     = Optional<int16_t>(),
                                                         Optional<ThreeLevelEnum> gridCarbonLevel  = Optional<ThreeLevelEnum>(),
                                                         Optional<int16_t> localCarbonIntensity    = Optional<int16_t>(),
                                                         Optional<ThreeLevelEnum> localCarbonLevel = Optional<ThreeLevelEnum>())
{
    ElectricalGridConditionsStruct::Type condition;
    condition.periodStart          = periodStart;
    condition.periodEnd            = periodEnd.HasValue() ? Nullable<uint32_t>(periodEnd.Value()) : Nullable<uint32_t>();
    condition.gridCarbonIntensity  = gridCarbonIntensity.HasValue() ? gridCarbonIntensity.Value() : static_cast<int16_t>(0);
    condition.gridCarbonLevel      = gridCarbonLevel.HasValue() ? gridCarbonLevel.Value() : static_cast<ThreeLevelEnum>(0);
    condition.localCarbonIntensity = localCarbonIntensity.HasValue() ? localCarbonIntensity.Value() : static_cast<int16_t>(0);
    condition.localCarbonLevel     = localCarbonLevel.HasValue() ? localCarbonLevel.Value() : static_cast<ThreeLevelEnum>(0);
    return condition;
}
} // namespace TestHelpers

class TestElectricalGridConditionsCluster : public ::testing::Test
{
public:
    class TestDelegate : public Delegate
    {
    public:
        TestDelegate()  = default;
        ~TestDelegate() = default;

        void SetEndpointId(EndpointId endpointId) { mEndpointId = endpointId; }
        EndpointId GetEndpointId() const { return mEndpointId; }

    private:
        EndpointId mEndpointId = kInvalidEndpointId;
    };

public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }

    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

protected:
    void SetUp() override
    {
        // Reset mock for each test
        MockMatterReporting::Reset();

        mDelegate = std::make_unique<TestDelegate>();
        mInstance = std::make_unique<Instance>(kTestEndpointId, *mDelegate, BitMask<Feature>(Feature::kForecasting));
        EXPECT_EQ(mInstance->Init(), CHIP_NO_ERROR);
    }

    void TearDown() override
    {
        mInstance.reset();
        mDelegate.reset();
    }

    std::unique_ptr<TestDelegate> mDelegate;
    std::unique_ptr<Instance> mInstance;
};

TEST_F(TestElectricalGridConditionsCluster, TestBasicInitialization)
{
    EXPECT_TRUE(mInstance->HasFeature(Feature::kForecasting));
}

TEST_F(TestElectricalGridConditionsCluster, TestLocalGenerationAvailable)
{
    // Test setting to true - should trigger reporting
    Nullable<bool> trueValue(true);
    EXPECT_EQ(mInstance->SetLocalGenerationAvailable(trueValue), CHIP_NO_ERROR);
    EXPECT_TRUE(MockMatterReporting::WasCalled());

    // Reset and set true again - should be no-op (no additional reporting)
    MockMatterReporting::Reset();
    EXPECT_EQ(mInstance->SetLocalGenerationAvailable(trueValue), CHIP_NO_ERROR);
    EXPECT_FALSE(MockMatterReporting::WasCalled()); // No-op: same value, no reporting

    // Test setting to false - should trigger reporting
    Nullable<bool> falseValue(false);
    EXPECT_EQ(mInstance->SetLocalGenerationAvailable(falseValue), CHIP_NO_ERROR);
    EXPECT_TRUE(MockMatterReporting::WasCalled());

    // Reset and set false again - should be no-op (no additional reporting)
    MockMatterReporting::Reset();
    EXPECT_EQ(mInstance->SetLocalGenerationAvailable(falseValue), CHIP_NO_ERROR);
    EXPECT_FALSE(MockMatterReporting::WasCalled()); // No-op: same value, no reporting

    // Test setting to null - should trigger reporting
    Nullable<bool> nullValue;
    EXPECT_EQ(mInstance->SetLocalGenerationAvailable(nullValue), CHIP_NO_ERROR);
    EXPECT_TRUE(MockMatterReporting::WasCalled());

    // Reset and set null again - should be no-op (no additional reporting)
    MockMatterReporting::Reset();
    EXPECT_EQ(mInstance->SetLocalGenerationAvailable(nullValue), CHIP_NO_ERROR);
    EXPECT_FALSE(MockMatterReporting::WasCalled()); // No-op: same value, no reporting
}

TEST_F(TestElectricalGridConditionsCluster, TestLocalGenerationAvailableNoOpBehavior)
{
    Nullable<bool> trueValue(true);
    Nullable<bool> falseValue(false);

    // Initial set to true - should trigger reporting
    EXPECT_EQ(mInstance->SetLocalGenerationAvailable(trueValue), CHIP_NO_ERROR);
    EXPECT_TRUE(MockMatterReporting::WasCalled());

    // Reset mock and set to true again - should be no-op (no additional reporting)
    MockMatterReporting::Reset();
    EXPECT_EQ(mInstance->SetLocalGenerationAvailable(trueValue), CHIP_NO_ERROR);
    EXPECT_FALSE(MockMatterReporting::WasCalled()); // No-op: no reporting triggered

    // Change to false - should trigger reporting
    EXPECT_EQ(mInstance->SetLocalGenerationAvailable(falseValue), CHIP_NO_ERROR);
    EXPECT_TRUE(MockMatterReporting::WasCalled());

    // Reset mock and set to false again - should be no-op (no additional reporting)
    MockMatterReporting::Reset();
    EXPECT_EQ(mInstance->SetLocalGenerationAvailable(falseValue), CHIP_NO_ERROR);
    EXPECT_FALSE(MockMatterReporting::WasCalled()); // No-op: no reporting triggered
}

TEST_F(TestElectricalGridConditionsCluster, TestCurrentConditionsNoOpBehavior)
{
    // Test deep comparison and no-op behavior with MatterReporting verification

    // Create identical conditions
    auto baseCondition      = TestHelpers::CreateTestCondition(1000);
    auto identicalCondition = TestHelpers::CreateTestCondition(1000); // Same values

    Nullable<ElectricalGridConditionsStruct::Type> nullableBase(baseCondition);
    Nullable<ElectricalGridConditionsStruct::Type> nullableIdentical(identicalCondition);

    // Set base condition - should trigger reporting
    EXPECT_EQ(mInstance->SetCurrentConditions(nullableBase), CHIP_NO_ERROR);
    EXPECT_TRUE(MockMatterReporting::WasCalled());

    // Reset mock and set identical condition - should be no-op due to deep comparison
    MockMatterReporting::Reset();
    EXPECT_EQ(mInstance->SetCurrentConditions(nullableIdentical), CHIP_NO_ERROR);
    EXPECT_FALSE(MockMatterReporting::WasCalled()); // No-op: no reporting triggered

    // Reset mock and set same reference again - should be no-op
    MockMatterReporting::Reset();
    EXPECT_EQ(mInstance->SetCurrentConditions(nullableBase), CHIP_NO_ERROR);
    EXPECT_FALSE(MockMatterReporting::WasCalled()); // No-op: no reporting triggered

    // Test null transitions
    Nullable<ElectricalGridConditionsStruct::Type> nullCondition;

    // Set to null - should trigger reporting
    EXPECT_EQ(mInstance->SetCurrentConditions(nullCondition), CHIP_NO_ERROR);
    EXPECT_TRUE(MockMatterReporting::WasCalled());

    // Reset mock and set null again - should be no-op
    MockMatterReporting::Reset();
    EXPECT_EQ(mInstance->SetCurrentConditions(nullCondition), CHIP_NO_ERROR);
    EXPECT_FALSE(MockMatterReporting::WasCalled()); // No-op: no reporting triggered
}

TEST_F(TestElectricalGridConditionsCluster, TestCurrentConditionsDeepComparisonDetectsDifferences)
{
    // Test that our deep comparison correctly identifies different conditions
    // and that different values properly trigger MatterReporting

    // Create base condition
    auto baseCondition = TestHelpers::CreateTestCondition(1000);
    Nullable<ElectricalGridConditionsStruct::Type> nullableBase(baseCondition);

    EXPECT_EQ(mInstance->SetCurrentConditions(nullableBase), CHIP_NO_ERROR);
    EXPECT_TRUE(MockMatterReporting::WasCalled());

    // Reset mock and test different periodStart - should trigger reporting
    MockMatterReporting::Reset();
    auto differentPeriodStart = TestHelpers::CreateTestCondition(1001);
    Nullable<ElectricalGridConditionsStruct::Type> nullableDifferentStart(differentPeriodStart);
    EXPECT_EQ(mInstance->SetCurrentConditions(nullableDifferentStart), CHIP_NO_ERROR);
    EXPECT_TRUE(MockMatterReporting::WasCalled()); // Different value: reporting triggered

    // Reset mock and test different periodEnd - should trigger reporting
    MockMatterReporting::Reset();
    auto differentPeriodEnd = TestHelpers::CreateTestCondition(1000, Optional<uint32_t>(2000));
    Nullable<ElectricalGridConditionsStruct::Type> nullableDifferentEnd(differentPeriodEnd);
    EXPECT_EQ(mInstance->SetCurrentConditions(nullableDifferentEnd), CHIP_NO_ERROR);
    EXPECT_TRUE(MockMatterReporting::WasCalled()); // Different value: reporting triggered

    // Reset mock and set back to base - should trigger reporting (proving it's different from the previous)
    MockMatterReporting::Reset();
    EXPECT_EQ(mInstance->SetCurrentConditions(nullableBase), CHIP_NO_ERROR);
    EXPECT_TRUE(MockMatterReporting::WasCalled()); // Back to different value: reporting triggered
}

TEST_F(TestElectricalGridConditionsCluster, TestCurrentConditionsValidPeriodStartEndCombinations)
{
    // Test valid periodStart/periodEnd combinations that should be accepted

    struct ValidConditionTest
    {
        uint32_t periodStart;
        Optional<uint32_t> periodEnd;
        const char * description;
    };

    std::vector<ValidConditionTest> validTests = {
        // Valid: periodEnd is null (allowed per spec)
        { 1000, Optional<uint32_t>(), "periodEnd is null" },

        // Valid: periodStart < periodEnd (spec requires periodEnd > periodStart)
        { 1000, Optional<uint32_t>(2000), "periodStart < periodEnd (1000 < 2000)" },
        { 0, Optional<uint32_t>(1), "periodStart < periodEnd (0 < 1)" },
        { 100, Optional<uint32_t>(150), "periodStart < periodEnd (100 < 150)" },
        { 4294967000U, Optional<uint32_t>(4294967295U), "periodStart < periodEnd (large values)" },
    };

    for (const auto & test : validTests)
    {
        MockMatterReporting::Reset();

        auto condition = TestHelpers::CreateTestCondition(test.periodStart, test.periodEnd);
        Nullable<ElectricalGridConditionsStruct::Type> nullableCondition(condition);

        EXPECT_EQ(mInstance->SetCurrentConditions(nullableCondition), CHIP_NO_ERROR)
            << "Failed for valid case: " << test.description;
        EXPECT_TRUE(MockMatterReporting::WasCalled()) << "MatterReporting should be called for valid case: " << test.description;
    }
}

TEST_F(TestElectricalGridConditionsCluster, TestCurrentConditionsInvalidPeriodStartEndCombinations)
{
    // Test invalid periodStart/periodEnd combinations that should be rejected per spec
    //
    // SPEC REQUIREMENT: Section 5.2.2 states "PeriodEnd SHALL be greater than PeriodStart"
    // This means periodStart >= periodEnd should be REJECTED

    struct InvalidConditionTest
    {
        uint32_t periodStart;
        uint32_t periodEnd;
        const char * description;
    };

    std::vector<InvalidConditionTest> invalidTests = {
        // Invalid: periodStart >= periodEnd (spec requires periodEnd > periodStart)
        { 1000, 1000, "periodStart == periodEnd (1000 == 1000) - violates spec" },
        { 2000, 1000, "periodStart > periodEnd (2000 > 1000)" },
        { 150, 100, "periodStart > periodEnd (150 > 100)" },
        { 4294967295U, 0, "periodStart > periodEnd (max uint32 > 0)" },
        { 1, 0, "periodStart > periodEnd (1 > 0)" },
        { 0, 0, "periodStart == periodEnd (0 == 0) - violates spec" },
    };

    for (const auto & test : invalidTests)
    {
        MockMatterReporting::Reset();

        auto condition = TestHelpers::CreateTestCondition(test.periodStart, Optional<uint32_t>(test.periodEnd));
        Nullable<ElectricalGridConditionsStruct::Type> nullableCondition(condition);

        // Should return ConstraintError per spec validation
        EXPECT_EQ(mInstance->SetCurrentConditions(nullableCondition), CHIP_IM_GLOBAL_STATUS(ConstraintError))
            << "Should reject invalid case: " << test.description;
        EXPECT_FALSE(MockMatterReporting::WasCalled())
            << "MatterReporting should NOT be called for invalid case: " << test.description;
    }
}

TEST_F(TestElectricalGridConditionsCluster, TestForecastConditionsValidTimeSeries)
{
    // Test valid forecast conditions that meet spec requirements for time ordering

    std::vector<std::vector<std::pair<uint32_t, Optional<uint32_t>>>> validForecastTests = {
        // Single entry with null periodEnd (last entry can have null periodEnd)
        { { 1000, Optional<uint32_t>() } },

        // Single entry with valid periodStart < periodEnd
        { { 1000, Optional<uint32_t>(2000) } },

        // Two entries: properly ordered with gaps allowed
        {
            { 1000, Optional<uint32_t>(2000) }, // First entry: must have non-null periodEnd
            { 2001, Optional<uint32_t>() }      // Last entry: can have null periodEnd, periodStart > previous periodEnd
        },

        // Three entries: properly ordered
        {
            { 1000, Optional<uint32_t>(1500) }, // First: non-null periodEnd
            { 1501, Optional<uint32_t>(2000) }, // Middle: non-null periodEnd, periodStart > previous periodEnd
            { 2001, Optional<uint32_t>() }      // Last: null periodEnd allowed, periodStart > previous periodEnd
        },

        // Multiple entries with exact boundaries (periodStart = previous periodEnd + 1)
        { { 1000, Optional<uint32_t>(1999) },
          { 2000, Optional<uint32_t>(2999) },
          { 3000, Optional<uint32_t>(3999) },
          { 4000, Optional<uint32_t>() } }
    };

    for (size_t testIndex = 0; testIndex < validForecastTests.size(); testIndex++)
    {
        MockMatterReporting::Reset();

        // Build forecast conditions list
        std::vector<ElectricalGridConditionsStruct::Type> forecastList;
        for (const auto & entry : validForecastTests[testIndex])
        {
            forecastList.push_back(TestHelpers::CreateTestCondition(entry.first, entry.second));
        }

        DataModel::List<const ElectricalGridConditionsStruct::Type> forecastConditions(forecastList.data(), forecastList.size());

        EXPECT_EQ(mInstance->SetForecastConditions(forecastConditions), CHIP_NO_ERROR)
            << "Failed for valid forecast test case " << testIndex;
        EXPECT_TRUE(MockMatterReporting::WasCalled())
            << "MatterReporting should be called for valid forecast test case " << testIndex;
    }
}

TEST_F(TestElectricalGridConditionsCluster, TestForecastConditionsInvalidTimeSeries)
{
    // Test invalid forecast conditions that violate spec requirements
    //
    // SPEC REQUIREMENTS:
    // 1. Section 5.2.2: "PeriodEnd SHALL be greater than PeriodStart" (individual entry validation)
    // 2. Section 6.3: "PeriodStart SHALL be greater than the previous entry's PeriodEnd" (time series validation)
    //
    // Current implementation bug: allows periodStart == periodEnd (violates requirement 1)

    struct InvalidForecastTest
    {
        std::vector<std::pair<uint32_t, Optional<uint32_t>>> entries;
        const char * description;
    };

    std::vector<InvalidForecastTest> invalidTests = {
        // Invalid: Individual entry validation - periodStart >= periodEnd (spec violation)
        { { { 2000, Optional<uint32_t>(2000) } }, "Single entry: periodStart == periodEnd - violates spec 5.2.2" },
        { { { 2000, Optional<uint32_t>(1000) } }, "Single entry: periodStart > periodEnd - violates spec 5.2.2" },

        // Invalid: Time series validation - non-last entry has null periodEnd (violates spec 6.3)
        { {
              { 1000, Optional<uint32_t>() }, // First entry with null periodEnd (violates spec 6.3)
              { 2000, Optional<uint32_t>() }  // Last entry with null periodEnd (valid)
          },
          "Non-last entry has null periodEnd - violates spec 6.3" },

        // Invalid: Time series validation - periodStart <= previous periodEnd (violates spec 6.3)
        { {
              { 1000, Optional<uint32_t>(2000) },
              { 2000, Optional<uint32_t>() } // periodStart == previous periodEnd (violates spec 6.3: must be >)
          },
          "periodStart == previous periodEnd - violates spec 6.3" },
        { {
              { 1000, Optional<uint32_t>(2000) },
              { 1999, Optional<uint32_t>() } // periodStart < previous periodEnd (violates spec 6.3)
          },
          "periodStart < previous periodEnd - violates spec 6.3" },

        // Invalid: Multiple violations
        { { { 1000, Optional<uint32_t>(1500) },
            { 1400, Optional<uint32_t>(2000) }, // periodStart < previous periodEnd (violates spec 6.3)
            { 2001, Optional<uint32_t>() } },
          "Multiple entries with time ordering violation - violates spec 6.3" },

        // Invalid: Time series validation - middle entry has null periodEnd (violates spec 6.3)
        { {
              { 1000, Optional<uint32_t>(1500) },
              { 1501, Optional<uint32_t>() }, // Middle entry with null periodEnd (violates spec 6.3)
              { 2000, Optional<uint32_t>() }  // Last entry
          },
          "Middle entry has null periodEnd - violates spec 6.3" }
    };

    for (size_t testIndex = 0; testIndex < invalidTests.size(); testIndex++)
    {
        const auto & test = invalidTests[testIndex];
        MockMatterReporting::Reset();

        // Build forecast conditions list
        std::vector<ElectricalGridConditionsStruct::Type> forecastList;
        for (const auto & entry : test.entries)
        {
            forecastList.push_back(TestHelpers::CreateTestCondition(entry.first, entry.second));
        }

        DataModel::List<const ElectricalGridConditionsStruct::Type> forecastConditions(forecastList.data(), forecastList.size());

        // Should return ConstraintError per spec validation
        EXPECT_EQ(mInstance->SetForecastConditions(forecastConditions), CHIP_IM_GLOBAL_STATUS(ConstraintError))
            << "Should reject invalid case: " << test.description;
        EXPECT_FALSE(MockMatterReporting::WasCalled())
            << "MatterReporting should NOT be called for invalid case: " << test.description;
    }
}

} // namespace
