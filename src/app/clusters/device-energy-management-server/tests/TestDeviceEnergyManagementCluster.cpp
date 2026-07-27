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

#include "DeviceEnergyManagementMockDelegate.h"
#include <app/clusters/device-energy-management-server/DeviceEnergyManagementCluster.h>
#include <app/clusters/device-energy-management-server/tests/DeviceEnergyManagementMockDelegate.h>
#include <pw_unit_test/framework.h>

#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/DeviceEnergyManagement/Attributes.h>
#include <clusters/DeviceEnergyManagement/Commands.h>
#include <clusters/DeviceEnergyManagement/Metadata.h>
#include <system/SystemClock.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::DeviceEnergyManagement;
using namespace chip::app::Clusters::DeviceEnergyManagement::Attributes;
using namespace chip::app::Clusters::DeviceEnergyManagement::Commands;
using namespace chip::Testing;

using chip::Testing::IsAcceptedCommandsListEqualTo;
using chip::Testing::IsAttributesListEqualTo;

namespace {

constexpr EndpointId kTestEndpointId = 1;

// Helper to check if real-time clock is available (required for time-based validations)
bool IsRealTimeClockAvailable()
{
    uint32_t epoch = 0;
    return chip::System::Clock::GetClock_MatterEpochS(epoch) == CHIP_NO_ERROR;
}

struct TestDeviceEnergyManagementCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    void SetUp() override {}
};

// =============================================================================
// Feature Tests
// =============================================================================

TEST_F(TestDeviceEnergyManagementCluster, TestFeatures)
{
    TestServerClusterContext context;
    DeviceEnergyManagementMockDelegate mockDelegate;

    // Test 1: No features - only mandatory attributes
    {
        BitMask<Feature> noFeatures;
        DeviceEnergyManagementCluster cluster(DeviceEnergyManagementCluster::Config(kTestEndpointId, noFeatures, mockDelegate));
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                ESAType::kMetadataEntry,
                                                ESACanGenerate::kMetadataEntry,
                                                ESAState::kMetadataEntry,
                                                AbsMinPower::kMetadataEntry,
                                                AbsMaxPower::kMetadataEntry,
                                            }));

        EXPECT_TRUE(IsAcceptedCommandsListEqualTo(cluster, {}));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    // Test 2: PowerAdjustment feature - PowerAdjustmentCapability and OptOutState attributes
    {
        BitMask<Feature> features(Feature::kPowerAdjustment);
        DeviceEnergyManagementCluster cluster(DeviceEnergyManagementCluster::Config(kTestEndpointId, features, mockDelegate));
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                ESAType::kMetadataEntry,
                                                ESACanGenerate::kMetadataEntry,
                                                ESAState::kMetadataEntry,
                                                AbsMinPower::kMetadataEntry,
                                                AbsMaxPower::kMetadataEntry,
                                                PowerAdjustmentCapability::kMetadataEntry,
                                                OptOutState::kMetadataEntry,
                                            }));

        EXPECT_TRUE(IsAcceptedCommandsListEqualTo(cluster,
                                                  {
                                                      PowerAdjustRequest::kMetadataEntry,
                                                      CancelPowerAdjustRequest::kMetadataEntry,
                                                  }));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    // Test 3: PowerForecastReporting feature - Forecast attribute
    {
        BitMask<Feature> features(Feature::kPowerForecastReporting);
        DeviceEnergyManagementCluster cluster(DeviceEnergyManagementCluster::Config(kTestEndpointId, features, mockDelegate));
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                ESAType::kMetadataEntry,
                                                ESACanGenerate::kMetadataEntry,
                                                ESAState::kMetadataEntry,
                                                AbsMinPower::kMetadataEntry,
                                                AbsMaxPower::kMetadataEntry,
                                                Forecast::kMetadataEntry,
                                            }));

        EXPECT_TRUE(IsAcceptedCommandsListEqualTo(cluster, {}));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    // Test 4: StateForecastReporting feature - Forecast attribute
    {
        BitMask<Feature> features(Feature::kStateForecastReporting);
        DeviceEnergyManagementCluster cluster(DeviceEnergyManagementCluster::Config(kTestEndpointId, features, mockDelegate));
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                ESAType::kMetadataEntry,
                                                ESACanGenerate::kMetadataEntry,
                                                ESAState::kMetadataEntry,
                                                AbsMinPower::kMetadataEntry,
                                                AbsMaxPower::kMetadataEntry,
                                                Forecast::kMetadataEntry,
                                            }));

        EXPECT_TRUE(IsAcceptedCommandsListEqualTo(cluster, {}));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    // Test 5: Pausable feature - PauseRequest and ResumeRequest commands, OptOutState attribute
    {
        BitMask<Feature> features(Feature::kPausable);
        DeviceEnergyManagementCluster cluster(DeviceEnergyManagementCluster::Config(kTestEndpointId, features, mockDelegate));
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                ESAType::kMetadataEntry,
                                                ESACanGenerate::kMetadataEntry,
                                                ESAState::kMetadataEntry,
                                                AbsMinPower::kMetadataEntry,
                                                AbsMaxPower::kMetadataEntry,
                                                OptOutState::kMetadataEntry,
                                            }));

        EXPECT_TRUE(IsAcceptedCommandsListEqualTo(cluster,
                                                  {
                                                      PauseRequest::kMetadataEntry,
                                                      ResumeRequest::kMetadataEntry,
                                                  }));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    // Test 6: ForecastAdjustment feature - ModifyForecastRequest command, OptOutState attribute, CancelRequest command
    {
        BitMask<Feature> features(Feature::kForecastAdjustment);
        DeviceEnergyManagementCluster cluster(DeviceEnergyManagementCluster::Config(kTestEndpointId, features, mockDelegate));
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                ESAType::kMetadataEntry,
                                                ESACanGenerate::kMetadataEntry,
                                                ESAState::kMetadataEntry,
                                                AbsMinPower::kMetadataEntry,
                                                AbsMaxPower::kMetadataEntry,
                                                OptOutState::kMetadataEntry,
                                            }));

        EXPECT_TRUE(IsAcceptedCommandsListEqualTo(cluster,
                                                  {
                                                      ModifyForecastRequest::kMetadataEntry,
                                                      CancelRequest::kMetadataEntry,
                                                  }));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    // Test 7: StartTimeAdjustment feature - StartTimeAdjustRequest command, OptOutState attribute, CancelRequest command
    {
        BitMask<Feature> features(Feature::kStartTimeAdjustment);
        DeviceEnergyManagementCluster cluster(DeviceEnergyManagementCluster::Config(kTestEndpointId, features, mockDelegate));
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                ESAType::kMetadataEntry,
                                                ESACanGenerate::kMetadataEntry,
                                                ESAState::kMetadataEntry,
                                                AbsMinPower::kMetadataEntry,
                                                AbsMaxPower::kMetadataEntry,
                                                OptOutState::kMetadataEntry,
                                            }));

        EXPECT_TRUE(IsAcceptedCommandsListEqualTo(cluster,
                                                  {
                                                      StartTimeAdjustRequest::kMetadataEntry,
                                                      CancelRequest::kMetadataEntry,
                                                  }));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    // Test 8: ConstraintBasedAdjustment feature - RequestConstraintBasedForecast command, OptOutState attribute, CancelRequest
    {
        BitMask<Feature> features(Feature::kConstraintBasedAdjustment);
        DeviceEnergyManagementCluster cluster(DeviceEnergyManagementCluster::Config(kTestEndpointId, features, mockDelegate));
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                ESAType::kMetadataEntry,
                                                ESACanGenerate::kMetadataEntry,
                                                ESAState::kMetadataEntry,
                                                AbsMinPower::kMetadataEntry,
                                                AbsMaxPower::kMetadataEntry,
                                                OptOutState::kMetadataEntry,
                                            }));

        EXPECT_TRUE(IsAcceptedCommandsListEqualTo(cluster,
                                                  {
                                                      RequestConstraintBasedForecast::kMetadataEntry,
                                                      CancelRequest::kMetadataEntry,
                                                  }));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    // Test 9: All features - all optional attributes, all commands
    {
        BitMask<Feature> allFeatures(Feature::kPowerAdjustment, Feature::kPowerForecastReporting, Feature::kStateForecastReporting,
                                     Feature::kStartTimeAdjustment, Feature::kPausable, Feature::kForecastAdjustment,
                                     Feature::kConstraintBasedAdjustment);
        DeviceEnergyManagementCluster cluster(DeviceEnergyManagementCluster::Config(kTestEndpointId, allFeatures, mockDelegate));
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                ESAType::kMetadataEntry,
                                                ESACanGenerate::kMetadataEntry,
                                                ESAState::kMetadataEntry,
                                                AbsMinPower::kMetadataEntry,
                                                AbsMaxPower::kMetadataEntry,
                                                PowerAdjustmentCapability::kMetadataEntry,
                                                Forecast::kMetadataEntry,
                                                OptOutState::kMetadataEntry,
                                            }));

        EXPECT_TRUE(IsAcceptedCommandsListEqualTo(cluster,
                                                  {
                                                      PowerAdjustRequest::kMetadataEntry,
                                                      CancelPowerAdjustRequest::kMetadataEntry,
                                                      StartTimeAdjustRequest::kMetadataEntry,
                                                      PauseRequest::kMetadataEntry,
                                                      ResumeRequest::kMetadataEntry,
                                                      ModifyForecastRequest::kMetadataEntry,
                                                      RequestConstraintBasedForecast::kMetadataEntry,
                                                      CancelRequest::kMetadataEntry,
                                                  }));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

// =============================================================================
// Startup Tests
// =============================================================================

TEST_F(TestDeviceEnergyManagementCluster, TestStartupFailsWithMismatchedEndpointId)
{
    TestServerClusterContext context;
    DeviceEnergyManagementMockDelegate mockDelegate;
    BitMask<Feature> noFeatures;

    constexpr EndpointId kClusterEndpointId  = 1;
    constexpr EndpointId kDelegateEndpointId = 2;

    // Create cluster with one endpoint ID
    DeviceEnergyManagementCluster cluster(DeviceEnergyManagementCluster::Config(kClusterEndpointId, noFeatures, mockDelegate));

    // Set delegate to a different endpoint ID
    mockDelegate.SetEndpointId(kDelegateEndpointId);

    // Startup should fail because endpoint IDs don't match
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(TestDeviceEnergyManagementCluster, TestStartupSucceedsWithMatchingEndpointId)
{
    TestServerClusterContext context;
    DeviceEnergyManagementMockDelegate mockDelegate;
    BitMask<Feature> noFeatures;

    constexpr EndpointId kEndpointId = 1;

    // Create cluster with endpoint ID
    DeviceEnergyManagementCluster cluster(DeviceEnergyManagementCluster::Config(kEndpointId, noFeatures, mockDelegate));

    // Delegate endpoint ID is set in constructor, so they should match
    EXPECT_EQ(mockDelegate.GetEndpointId(), kEndpointId);

    // Startup should succeed because endpoint IDs match
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// =============================================================================
// Attribute Tests
// =============================================================================

TEST_F(TestDeviceEnergyManagementCluster, TestMandatoryAttributes)
{
    TestServerClusterContext context;
    DeviceEnergyManagementMockDelegate mockDelegate;
    BitMask<Feature> noFeatures;

    DeviceEnergyManagementCluster cluster(DeviceEnergyManagementCluster::Config(kTestEndpointId, noFeatures, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    ESATypeEnum esaType = ESATypeEnum::kUnknownEnumValue;
    ASSERT_EQ(tester.ReadAttribute(ESAType::Id, esaType), CHIP_NO_ERROR);
    EXPECT_EQ(esaType, ESATypeEnum::kEvse);

    bool esaCanGenerate = false;
    ASSERT_EQ(tester.ReadAttribute(ESACanGenerate::Id, esaCanGenerate), CHIP_NO_ERROR);
    EXPECT_EQ(esaCanGenerate, false);

    ESAStateEnum esaState = ESAStateEnum::kUnknownEnumValue;
    ASSERT_EQ(tester.ReadAttribute(ESAState::Id, esaState), CHIP_NO_ERROR);
    EXPECT_EQ(esaState, ESAStateEnum::kOnline);

    int64_t absMinPower = 0;
    ASSERT_EQ(tester.ReadAttribute(AbsMinPower::Id, absMinPower), CHIP_NO_ERROR);
    EXPECT_EQ(absMinPower, DeviceEnergyManagementMockDelegate::kAbsMinPower);

    int64_t absMaxPower = 0;
    ASSERT_EQ(tester.ReadAttribute(AbsMaxPower::Id, absMaxPower), CHIP_NO_ERROR);
    EXPECT_EQ(absMaxPower, DeviceEnergyManagementMockDelegate::kAbsMaxPower);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// =============================================================================
// PowerAdjustRequest Command Tests
// =============================================================================

TEST_F(TestDeviceEnergyManagementCluster, TestPowerAdjustRequest)
{
    TestServerClusterContext context;
    DeviceEnergyManagementMockDelegate mockDelegate;
    BitMask<Feature> features(Feature::kPowerAdjustment);

    DeviceEnergyManagementCluster cluster(DeviceEnergyManagementCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    Commands::PowerAdjustRequest::Type command;
    command.power    = 1000;
    command.duration = 60;
    command.cause    = AdjustmentCauseEnum::kLocalOptimization;

    // Power out of range - rejected
    command.power = DeviceEnergyManagementMockDelegate::kAbsMaxPower + 1000;
    EXPECT_FALSE(tester.Invoke(Commands::PowerAdjustRequest::Id, command).IsSuccess());
    EXPECT_EQ(mockDelegate.GetESAState(), ESAStateEnum::kOnline);

    // Valid power adjustment - succeeds
    command.power = 1000;
    EXPECT_TRUE(tester.Invoke(Commands::PowerAdjustRequest::Id, command).IsSuccess());
    EXPECT_EQ(mockDelegate.GetESAState(), ESAStateEnum::kPowerAdjustActive);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// =============================================================================
// CancelPowerAdjustRequest Command Tests
// =============================================================================

TEST_F(TestDeviceEnergyManagementCluster, TestCancelPowerAdjustRequest)
{
    TestServerClusterContext context;
    DeviceEnergyManagementMockDelegate mockDelegate;
    BitMask<Feature> features(Feature::kPowerAdjustment);

    DeviceEnergyManagementCluster cluster(DeviceEnergyManagementCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    Commands::CancelPowerAdjustRequest::Type cancelCommand;

    // Cancel without active adjustment - fails (InvalidInState)
    EXPECT_FALSE(tester.Invoke(Commands::CancelPowerAdjustRequest::Id, cancelCommand).IsSuccess());

    // Start a power adjustment first
    Commands::PowerAdjustRequest::Type powerAdjustCommand;
    powerAdjustCommand.power    = 1000;
    powerAdjustCommand.duration = 60;
    powerAdjustCommand.cause    = AdjustmentCauseEnum::kLocalOptimization;
    EXPECT_TRUE(tester.Invoke(Commands::PowerAdjustRequest::Id, powerAdjustCommand).IsSuccess());
    EXPECT_EQ(mockDelegate.GetESAState(), ESAStateEnum::kPowerAdjustActive);

    // Cancel - succeeds
    EXPECT_TRUE(tester.Invoke(Commands::CancelPowerAdjustRequest::Id, cancelCommand).IsSuccess());
    EXPECT_EQ(mockDelegate.GetESAState(), ESAStateEnum::kOnline);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// =============================================================================
// StartTimeAdjustRequest Command Tests
// =============================================================================

TEST_F(TestDeviceEnergyManagementCluster, TestStartTimeAdjustRequest)
{
    TestServerClusterContext context;
    DeviceEnergyManagementMockDelegate mockDelegate;
    BitMask<Feature> features(Feature::kStartTimeAdjustment);

    DeviceEnergyManagementCluster cluster(DeviceEnergyManagementCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    Commands::StartTimeAdjustRequest::Type command;
    command.cause = AdjustmentCauseEnum::kLocalOptimization;

    // Start time before earliestStartTime (900) - rejected
    command.requestedStartTime = 800;
    EXPECT_FALSE(tester.Invoke(Commands::StartTimeAdjustRequest::Id, command).IsSuccess());

    // Start time that would push end beyond latestEndTime (2100) - rejected
    // Forecast duration is 1000, so start at 1200 ends at 2200 > 2100
    command.requestedStartTime = 1200;
    EXPECT_FALSE(tester.Invoke(Commands::StartTimeAdjustRequest::Id, command).IsSuccess());

    // Valid start time within range - succeeds
    command.requestedStartTime = 1000;
    EXPECT_TRUE(tester.Invoke(Commands::StartTimeAdjustRequest::Id, command).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// =============================================================================
// PauseRequest Command Tests
// =============================================================================

TEST_F(TestDeviceEnergyManagementCluster, TestPauseRequest)
{
    TestServerClusterContext context;
    DeviceEnergyManagementMockDelegate mockDelegate;
    BitMask<Feature> features(Feature::kPausable);

    DeviceEnergyManagementCluster cluster(DeviceEnergyManagementCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    Commands::PauseRequest::Type command;
    command.duration = 60;
    command.cause    = AdjustmentCauseEnum::kLocalOptimization;

    // Duration out of range - rejected
    command.duration = 4000; // Exceeds maxPauseDuration (3600)
    EXPECT_FALSE(tester.Invoke(Commands::PauseRequest::Id, command).IsSuccess());
    EXPECT_EQ(mockDelegate.GetESAState(), ESAStateEnum::kOnline);

    // OptOut rejects matching cause
    command.duration = 60;
    mockDelegate.SetOptOutState(OptOutStateEnum::kLocalOptOut);
    EXPECT_FALSE(tester.Invoke(Commands::PauseRequest::Id, command).IsSuccess());
    mockDelegate.SetOptOutState(OptOutStateEnum::kNoOptOut);

    // Force set the ESA in an invalid state - rejected
    EXPECT_EQ(mockDelegate.SetESAState(ESAStateEnum::kFault), CHIP_NO_ERROR);
    EXPECT_FALSE(tester.Invoke(Commands::PauseRequest::Id, command).IsSuccess());

    // Valid pause - succeeds
    EXPECT_EQ(mockDelegate.SetESAState(ESAStateEnum::kOnline), CHIP_NO_ERROR);
    EXPECT_TRUE(tester.Invoke(Commands::PauseRequest::Id, command).IsSuccess());
    EXPECT_EQ(mockDelegate.GetESAState(), ESAStateEnum::kPaused);
    EXPECT_EQ(mockDelegate.GetForecast().Value().forecastUpdateReason, ForecastUpdateReasonEnum::kLocalOptimization);

    // Further pause while paused with valid duration - succeeds
    command.cause = AdjustmentCauseEnum::kGridOptimization;
    EXPECT_TRUE(tester.Invoke(Commands::PauseRequest::Id, command).IsSuccess());
    EXPECT_EQ(mockDelegate.GetForecast().Value().forecastUpdateReason, ForecastUpdateReasonEnum::kGridOptimization);

    // Further pause while paused with invalid duration - rejected but stays paused
    command.duration = 5000;
    EXPECT_FALSE(tester.Invoke(Commands::PauseRequest::Id, command).IsSuccess());
    EXPECT_EQ(mockDelegate.GetESAState(), ESAStateEnum::kPaused);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// =============================================================================
// ResumeRequest Command Tests
// =============================================================================

TEST_F(TestDeviceEnergyManagementCluster, TestResumeRequest)
{
    TestServerClusterContext context;
    DeviceEnergyManagementMockDelegate mockDelegate;
    BitMask<Feature> features(Feature::kPausable);

    DeviceEnergyManagementCluster cluster(DeviceEnergyManagementCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    Commands::ResumeRequest::Type resumeCommand;

    // Resume without being paused - fails (InvalidInState)
    EXPECT_FALSE(tester.Invoke(Commands::ResumeRequest::Id, resumeCommand).IsSuccess());

    // Pause first
    Commands::PauseRequest::Type pauseCommand;
    pauseCommand.duration = 60;
    pauseCommand.cause    = AdjustmentCauseEnum::kLocalOptimization;
    EXPECT_TRUE(tester.Invoke(Commands::PauseRequest::Id, pauseCommand).IsSuccess());
    EXPECT_EQ(mockDelegate.GetESAState(), ESAStateEnum::kPaused);

    // Resume - succeeds
    EXPECT_TRUE(tester.Invoke(Commands::ResumeRequest::Id, resumeCommand).IsSuccess());
    EXPECT_EQ(mockDelegate.GetESAState(), ESAStateEnum::kOnline);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// =============================================================================
// ModifyForecastRequest Command Tests
// =============================================================================

TEST_F(TestDeviceEnergyManagementCluster, TestModifyForecastRequest)
{
    TestServerClusterContext context;
    DeviceEnergyManagementMockDelegate mockDelegate;
    BitMask<Feature> features(Feature::kForecastAdjustment);

    DeviceEnergyManagementCluster cluster(DeviceEnergyManagementCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    Commands::ModifyForecastRequest::Type command;
    command.cause = AdjustmentCauseEnum::kLocalOptimization;

    // Wrong forecast ID - rejected
    command.forecastID = 9999;
    EXPECT_FALSE(tester.Invoke(Commands::ModifyForecastRequest::Id, command).IsSuccess());

    // Correct forecast ID - succeeds
    command.forecastID = mockDelegate.GetForecast().Value().forecastID;
    EXPECT_TRUE(tester.Invoke(Commands::ModifyForecastRequest::Id, command).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// =============================================================================
// RequestConstraintBasedForecast Command Tests
// =============================================================================

TEST_F(TestDeviceEnergyManagementCluster, TestRequestConstraintBasedForecast)
{
    if (!IsRealTimeClockAvailable())
    {
        GTEST_SKIP() << "Real-time clock not available";
    }

    TestServerClusterContext context;
    DeviceEnergyManagementMockDelegate mockDelegate;
    BitMask<Feature> features(Feature::kConstraintBasedAdjustment);

    DeviceEnergyManagementCluster cluster(DeviceEnergyManagementCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    Commands::RequestConstraintBasedForecast::Type command;
    command.cause = AdjustmentCauseEnum::kLocalOptimization;

    ClusterTester tester(cluster);
    auto result = tester.Invoke(Commands::RequestConstraintBasedForecast::Id, command);
    EXPECT_TRUE(result.IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// =============================================================================
// CancelRequest Command Tests
// =============================================================================

TEST_F(TestDeviceEnergyManagementCluster, TestCancelRequest)
{
    TestServerClusterContext context;
    DeviceEnergyManagementMockDelegate mockDelegate;
    BitMask<Feature> features(Feature::kStartTimeAdjustment);

    DeviceEnergyManagementCluster cluster(DeviceEnergyManagementCluster::Config(kTestEndpointId, features, mockDelegate));
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    Commands::CancelRequest::Type cancelCommand;

    // Mock starts with kLocalOptimization - cancel succeeds
    EXPECT_EQ(mockDelegate.GetForecast().Value().forecastUpdateReason, ForecastUpdateReasonEnum::kLocalOptimization);
    EXPECT_TRUE(tester.Invoke(Commands::CancelRequest::Id, cancelCommand).IsSuccess());
    // ESAState reset to Online, ForecastUpdateReason reset to InternalOptimization
    EXPECT_EQ(mockDelegate.GetESAState(), ESAStateEnum::kOnline);
    EXPECT_EQ(mockDelegate.GetForecast().Value().forecastUpdateReason, ForecastUpdateReasonEnum::kInternalOptimization);

    // Cancel when already InternalOptimization - fails (InvalidInState)
    EXPECT_FALSE(tester.Invoke(Commands::CancelRequest::Id, cancelCommand).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

} // namespace
