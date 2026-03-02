/*
 *
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

#include "clusters/EnergyEvse/Enums.h"
#include "pw_unit_test/framework.h"
#include <app/clusters/energy-evse-server/Constants.h>
#include <app/clusters/energy-evse-server/EnergyEvseCluster.h>
#include <app/clusters/energy-evse-server/tests/MockEvseDelegate.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/EnergyEvse/Attributes.h>
#include <clusters/EnergyEvse/Commands.h>
#include <clusters/EnergyEvse/Metadata.h>
#include <system/SystemClock.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::EnergyEvse;
using namespace chip::app::Clusters::EnergyEvse::Attributes;
using namespace chip::app::Clusters::EnergyEvse::Commands;
using namespace chip::Testing;

using chip::Testing::IsAcceptedCommandsListEqualTo;
using chip::Testing::IsAttributesListEqualTo;

namespace {

constexpr EndpointId kTestEndpointId = 1;

struct TestEnergyEvseCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    void SetUp() override {}
};

// =============================================================================
// Feature Tests
// =============================================================================

TEST_F(TestEnergyEvseCluster, TestFeatures)
{

    TestServerClusterContext context;
    MockEvseDelegate mockDelegate;
    {
        // Test with minimum features and no optional attributes
        // ChargingPreferences feature is mandatory
        BitMask<Feature> minimumFeatures(Feature::kChargingPreferences);
        BitMask<OptionalAttributes> optionalAttributes;
        BitMask<OptionalCommands> optionalCommands;

        EnergyEvseCluster cluster(
            EnergyEvseCluster::Config(kTestEndpointId, mockDelegate, minimumFeatures, optionalAttributes, optionalCommands));
        mockDelegate.SetCluster(cluster);
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                State::kMetadataEntry,
                                                SupplyState::kMetadataEntry,
                                                FaultState::kMetadataEntry,
                                                ChargingEnabledUntil::kMetadataEntry,
                                                CircuitCapacity::kMetadataEntry,
                                                MinimumChargeCurrent::kMetadataEntry,
                                                MaximumChargeCurrent::kMetadataEntry,
                                                NextChargeStartTime::kMetadataEntry,
                                                NextChargeTargetTime::kMetadataEntry,
                                                NextChargeRequiredEnergy::kMetadataEntry,
                                                NextChargeTargetSoC::kMetadataEntry,
                                                SessionID::kMetadataEntry,
                                                SessionDuration::kMetadataEntry,
                                                SessionEnergyCharged::kMetadataEntry,
                                            }));

        EXPECT_TRUE(IsAcceptedCommandsListEqualTo(
            cluster,
            {
                { Disable::Id,
                  BitMask<chip::app::DataModel::CommandQualityFlags>(chip::app::DataModel::CommandQualityFlags::kTimed) },
                { EnableCharging::Id,
                  BitMask<chip::app::DataModel::CommandQualityFlags>(chip::app::DataModel::CommandQualityFlags::kTimed) },
                { SetTargets::Id,
                  BitMask<chip::app::DataModel::CommandQualityFlags>(chip::app::DataModel::CommandQualityFlags::kTimed) },
                { GetTargets::Id,
                  BitMask<chip::app::DataModel::CommandQualityFlags>(chip::app::DataModel::CommandQualityFlags::kTimed) },
                { ClearTargets::Id,
                  BitMask<chip::app::DataModel::CommandQualityFlags>(chip::app::DataModel::CommandQualityFlags::kTimed) },
            }));

        EXPECT_TRUE(IsGeneratedCommandsListEqualTo(cluster,
                                                   {
                                                       GetTargetsResponse::Id,
                                                   }));
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
    // Test with all features and no optional attributes & no optional commands
    {
        BitMask<Feature> allFeatures(Feature::kChargingPreferences, Feature::kV2x, Feature::kSoCReporting, Feature::kPlugAndCharge);
        BitMask<OptionalAttributes> optionalAttributes;
        BitMask<OptionalCommands> optionalCommands;

        EnergyEvseCluster cluster(
            EnergyEvseCluster::Config(kTestEndpointId, mockDelegate, allFeatures, optionalAttributes, optionalCommands));
        mockDelegate.SetCluster(cluster);
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                State::kMetadataEntry,
                                                SupplyState::kMetadataEntry,
                                                FaultState::kMetadataEntry,
                                                ChargingEnabledUntil::kMetadataEntry,
                                                DischargingEnabledUntil::kMetadataEntry,
                                                CircuitCapacity::kMetadataEntry,
                                                MinimumChargeCurrent::kMetadataEntry,
                                                MaximumChargeCurrent::kMetadataEntry,
                                                MaximumDischargeCurrent::kMetadataEntry,
                                                NextChargeStartTime::kMetadataEntry,
                                                NextChargeTargetTime::kMetadataEntry,
                                                NextChargeRequiredEnergy::kMetadataEntry,
                                                NextChargeTargetSoC::kMetadataEntry,
                                                StateOfCharge::kMetadataEntry,
                                                BatteryCapacity::kMetadataEntry,
                                                VehicleID::kMetadataEntry,
                                                SessionID::kMetadataEntry,
                                                SessionDuration::kMetadataEntry,
                                                SessionEnergyCharged::kMetadataEntry,
                                                SessionEnergyDischarged::kMetadataEntry,
                                            }));

        EXPECT_TRUE(IsAcceptedCommandsListEqualTo(
            cluster,
            {
                { Disable::Id,
                  BitMask<chip::app::DataModel::CommandQualityFlags>(chip::app::DataModel::CommandQualityFlags::kTimed) },
                { EnableCharging::Id,
                  BitMask<chip::app::DataModel::CommandQualityFlags>(chip::app::DataModel::CommandQualityFlags::kTimed) },
                { EnableDischarging::Id,
                  BitMask<chip::app::DataModel::CommandQualityFlags>(chip::app::DataModel::CommandQualityFlags::kTimed) },
                { SetTargets::Id,
                  BitMask<chip::app::DataModel::CommandQualityFlags>(chip::app::DataModel::CommandQualityFlags::kTimed) },
                { GetTargets::Id,
                  BitMask<chip::app::DataModel::CommandQualityFlags>(chip::app::DataModel::CommandQualityFlags::kTimed) },
                { ClearTargets::Id,
                  BitMask<chip::app::DataModel::CommandQualityFlags>(chip::app::DataModel::CommandQualityFlags::kTimed) },
            }));

        EXPECT_TRUE(IsGeneratedCommandsListEqualTo(cluster,
                                                   {
                                                       GetTargetsResponse::Id,
                                                   }));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    // Test with all features and optional attributes & optional commands
    {
        BitMask<Feature> allFeatures(Feature::kChargingPreferences, Feature::kV2x, Feature::kSoCReporting, Feature::kPlugAndCharge);
        BitMask<OptionalAttributes> optionalAttributes(OptionalAttributes::kSupportsUserMaximumChargingCurrent,
                                                       OptionalAttributes::kSupportsRandomizationWindow,
                                                       OptionalAttributes::kSupportsApproximateEvEfficiency);
        BitMask<OptionalCommands> optionalCommands(OptionalCommands::kSupportsStartDiagnostics);

        EnergyEvseCluster cluster(
            EnergyEvseCluster::Config(kTestEndpointId, mockDelegate, allFeatures, optionalAttributes, optionalCommands));
        mockDelegate.SetCluster(cluster);
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                            {
                                                State::kMetadataEntry,
                                                SupplyState::kMetadataEntry,
                                                FaultState::kMetadataEntry,
                                                ChargingEnabledUntil::kMetadataEntry,
                                                DischargingEnabledUntil::kMetadataEntry,
                                                CircuitCapacity::kMetadataEntry,
                                                MinimumChargeCurrent::kMetadataEntry,
                                                MaximumChargeCurrent::kMetadataEntry,
                                                MaximumDischargeCurrent::kMetadataEntry,
                                                NextChargeStartTime::kMetadataEntry,
                                                NextChargeTargetTime::kMetadataEntry,
                                                NextChargeRequiredEnergy::kMetadataEntry,
                                                NextChargeTargetSoC::kMetadataEntry,
                                                ApproximateEVEfficiency::kMetadataEntry,
                                                StateOfCharge::kMetadataEntry,
                                                BatteryCapacity::kMetadataEntry,
                                                VehicleID::kMetadataEntry,
                                                SessionID::kMetadataEntry,
                                                SessionDuration::kMetadataEntry,
                                                SessionEnergyCharged::kMetadataEntry,
                                                SessionEnergyDischarged::kMetadataEntry,
                                                UserMaximumChargeCurrent::kMetadataEntry,
                                                RandomizationDelayWindow::kMetadataEntry,
                                            }));

        EXPECT_TRUE(IsAcceptedCommandsListEqualTo(
            cluster,
            {
                { Disable::Id,
                  BitMask<chip::app::DataModel::CommandQualityFlags>(chip::app::DataModel::CommandQualityFlags::kTimed) },
                { EnableCharging::Id,
                  BitMask<chip::app::DataModel::CommandQualityFlags>(chip::app::DataModel::CommandQualityFlags::kTimed) },
                { EnableDischarging::Id,
                  BitMask<chip::app::DataModel::CommandQualityFlags>(chip::app::DataModel::CommandQualityFlags::kTimed) },
                { StartDiagnostics::Id,
                  BitMask<chip::app::DataModel::CommandQualityFlags>(chip::app::DataModel::CommandQualityFlags::kTimed) },
                { SetTargets::Id,
                  BitMask<chip::app::DataModel::CommandQualityFlags>(chip::app::DataModel::CommandQualityFlags::kTimed) },
                { GetTargets::Id,
                  BitMask<chip::app::DataModel::CommandQualityFlags>(chip::app::DataModel::CommandQualityFlags::kTimed) },
                { ClearTargets::Id,
                  BitMask<chip::app::DataModel::CommandQualityFlags>(chip::app::DataModel::CommandQualityFlags::kTimed) },
            }));

        EXPECT_TRUE(IsGeneratedCommandsListEqualTo(cluster,
                                                   {
                                                       GetTargetsResponse::Id,
                                                   }));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

// =============================================================================
// Startup Tests
// =============================================================================

TEST_F(TestEnergyEvseCluster, TestStartupFailsWithMismatchedEndpointId)
{
    TestServerClusterContext context;
    MockEvseDelegate mockDelegate;
    BitMask<Feature> noFeatures;
    BitMask<OptionalAttributes> optionalAttributes;
    BitMask<OptionalCommands> optionalCommands;

    constexpr EndpointId kClusterEndpointId  = 1;
    constexpr EndpointId kDelegateEndpointId = 2;

    // Create cluster with one endpoint ID
    EnergyEvseCluster cluster(
        EnergyEvseCluster::Config(kClusterEndpointId, mockDelegate, noFeatures, optionalAttributes, optionalCommands));
    mockDelegate.SetCluster(cluster);

    // Set delegate to a different endpoint ID
    mockDelegate.SetEndpointId(kDelegateEndpointId);

    // Startup should fail because endpoint IDs don't match
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(TestEnergyEvseCluster, TestStartupSucceedsWithMatchingEndpointId)
{
    TestServerClusterContext context;
    MockEvseDelegate mockDelegate;
    BitMask<Feature> noFeatures;
    BitMask<OptionalAttributes> optionalAttributes;
    BitMask<OptionalCommands> optionalCommands;
    constexpr EndpointId kEndpointId = 1;

    // Create cluster with endpoint ID
    EnergyEvseCluster cluster(
        EnergyEvseCluster::Config(kEndpointId, mockDelegate, noFeatures, optionalAttributes, optionalCommands));
    mockDelegate.SetCluster(cluster);

    // Delegate endpoint ID is set in constructor, so they should match
    EXPECT_EQ(mockDelegate.GetEndpointId(), kEndpointId);

    // Startup should succeed because endpoint IDs match
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// =============================================================================
// Attribute Tests
// =============================================================================

TEST_F(TestEnergyEvseCluster, TestAttributesMinimalConfig)
{
    // Test with minimal configuration (no features, no optional attributes)
    TestServerClusterContext context;
    MockEvseDelegate mockDelegate;
    BitMask<Feature> noFeatures;
    BitMask<OptionalAttributes> optionalAttributes;
    BitMask<OptionalCommands> optionalCommands;

    EnergyEvseCluster cluster(
        EnergyEvseCluster::Config(kTestEndpointId, mockDelegate, noFeatures, optionalAttributes, optionalCommands));
    mockDelegate.SetCluster(cluster);

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                        {
                                            State::kMetadataEntry,
                                            SupplyState::kMetadataEntry,
                                            FaultState::kMetadataEntry,
                                            ChargingEnabledUntil::kMetadataEntry,
                                            CircuitCapacity::kMetadataEntry,
                                            MinimumChargeCurrent::kMetadataEntry,
                                            MaximumChargeCurrent::kMetadataEntry,
                                            SessionID::kMetadataEntry,
                                            SessionDuration::kMetadataEntry,
                                            SessionEnergyCharged::kMetadataEntry,
                                        }));

    ClusterTester tester(cluster);

    // Read mandatory attributes and verify values - cluster owns the data with defaults
    StateEnum state = StateEnum::kUnknownEnumValue;
    ASSERT_EQ(tester.ReadAttribute(State::Id, state), CHIP_NO_ERROR);
    EXPECT_EQ(state, StateEnum::kNotPluggedIn); // Cluster default

    SupplyStateEnum supplyState = SupplyStateEnum::kUnknownEnumValue;
    ASSERT_EQ(tester.ReadAttribute(SupplyState::Id, supplyState), CHIP_NO_ERROR);
    EXPECT_EQ(supplyState, SupplyStateEnum::kDisabled); // Cluster default

    FaultStateEnum faultState = FaultStateEnum::kUnknownEnumValue;
    ASSERT_EQ(tester.ReadAttribute(FaultState::Id, faultState), CHIP_NO_ERROR);
    EXPECT_EQ(faultState, FaultStateEnum::kNoError); // Cluster default

    DataModel::Nullable<uint32_t> chargingEnabledUntil;
    ASSERT_EQ(tester.ReadAttribute(ChargingEnabledUntil::Id, chargingEnabledUntil), CHIP_NO_ERROR);
    EXPECT_TRUE(chargingEnabledUntil.IsNull()); // Cluster default is null

    int64_t circuitCapacity = -1;
    ASSERT_EQ(tester.ReadAttribute(CircuitCapacity::Id, circuitCapacity), CHIP_NO_ERROR);
    EXPECT_EQ(circuitCapacity, 0); // Cluster default

    int64_t minimumChargeCurrent = -1;
    ASSERT_EQ(tester.ReadAttribute(MinimumChargeCurrent::Id, minimumChargeCurrent), CHIP_NO_ERROR);
    EXPECT_EQ(minimumChargeCurrent, kMinimumChargeCurrent); // Cluster default (6000mA)

    int64_t maximumChargeCurrent = -1;
    ASSERT_EQ(tester.ReadAttribute(MaximumChargeCurrent::Id, maximumChargeCurrent), CHIP_NO_ERROR);
    EXPECT_EQ(maximumChargeCurrent, 0); // Cluster default

    DataModel::Nullable<uint32_t> sessionID;
    ASSERT_EQ(tester.ReadAttribute(SessionID::Id, sessionID), CHIP_NO_ERROR);
    EXPECT_TRUE(sessionID.IsNull()); // Cluster default is null

    DataModel::Nullable<uint32_t> sessionDuration;
    ASSERT_EQ(tester.ReadAttribute(SessionDuration::Id, sessionDuration), CHIP_NO_ERROR);
    EXPECT_TRUE(sessionDuration.IsNull()); // Cluster default is null

    DataModel::Nullable<int64_t> sessionEnergyCharged;
    ASSERT_EQ(tester.ReadAttribute(SessionEnergyCharged::Id, sessionEnergyCharged), CHIP_NO_ERROR);
    EXPECT_TRUE(sessionEnergyCharged.IsNull()); // Cluster default is null

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestEnergyEvseCluster, TestAttributesFullConfig)
{
    // Test with all features and optional attributes
    TestServerClusterContext context;
    MockEvseDelegate mockDelegate;
    BitMask<Feature> allFeatures(Feature::kChargingPreferences, Feature::kV2x, Feature::kSoCReporting, Feature::kPlugAndCharge);
    BitMask<OptionalAttributes> optionalAttributes(OptionalAttributes::kSupportsUserMaximumChargingCurrent,
                                                   OptionalAttributes::kSupportsRandomizationWindow,
                                                   OptionalAttributes::kSupportsApproximateEvEfficiency);
    BitMask<OptionalCommands> optionalCommands;

    EnergyEvseCluster cluster(
        EnergyEvseCluster::Config(kTestEndpointId, mockDelegate, allFeatures, optionalAttributes, optionalCommands));
    mockDelegate.SetCluster(cluster);

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    // Read mandatory attributes - cluster owns the data with defaults
    StateEnum state = StateEnum::kUnknownEnumValue;
    ASSERT_EQ(tester.ReadAttribute(State::Id, state), CHIP_NO_ERROR);
    EXPECT_EQ(state, StateEnum::kNotPluggedIn); // Cluster default

    SupplyStateEnum supplyState = SupplyStateEnum::kUnknownEnumValue;
    ASSERT_EQ(tester.ReadAttribute(SupplyState::Id, supplyState), CHIP_NO_ERROR);
    EXPECT_EQ(supplyState, SupplyStateEnum::kDisabled); // Cluster default

    FaultStateEnum faultState = FaultStateEnum::kUnknownEnumValue;
    ASSERT_EQ(tester.ReadAttribute(FaultState::Id, faultState), CHIP_NO_ERROR);
    EXPECT_EQ(faultState, FaultStateEnum::kNoError); // Cluster default

    DataModel::Nullable<uint32_t> chargingEnabledUntil;
    ASSERT_EQ(tester.ReadAttribute(ChargingEnabledUntil::Id, chargingEnabledUntil), CHIP_NO_ERROR);
    EXPECT_TRUE(chargingEnabledUntil.IsNull()); // Cluster default is null

    int64_t circuitCapacity = -1;
    ASSERT_EQ(tester.ReadAttribute(CircuitCapacity::Id, circuitCapacity), CHIP_NO_ERROR);
    EXPECT_EQ(circuitCapacity, 0); // Cluster default

    int64_t minimumChargeCurrent = -1;
    ASSERT_EQ(tester.ReadAttribute(MinimumChargeCurrent::Id, minimumChargeCurrent), CHIP_NO_ERROR);
    EXPECT_EQ(minimumChargeCurrent, kMinimumChargeCurrent); // Cluster default (6000mA)

    int64_t maximumChargeCurrent = -1;
    ASSERT_EQ(tester.ReadAttribute(MaximumChargeCurrent::Id, maximumChargeCurrent), CHIP_NO_ERROR);
    EXPECT_EQ(maximumChargeCurrent, 0); // Cluster default

    // V2x feature attributes
    DataModel::Nullable<uint32_t> dischargingEnabledUntil;
    ASSERT_EQ(tester.ReadAttribute(DischargingEnabledUntil::Id, dischargingEnabledUntil), CHIP_NO_ERROR);
    EXPECT_TRUE(dischargingEnabledUntil.IsNull()); // Cluster default is null

    int64_t maximumDischargeCurrent = -1;
    ASSERT_EQ(tester.ReadAttribute(MaximumDischargeCurrent::Id, maximumDischargeCurrent), CHIP_NO_ERROR);
    EXPECT_EQ(maximumDischargeCurrent, 0); // Cluster default

    DataModel::Nullable<int64_t> sessionEnergyDischarged;
    ASSERT_EQ(tester.ReadAttribute(SessionEnergyDischarged::Id, sessionEnergyDischarged), CHIP_NO_ERROR);
    EXPECT_TRUE(sessionEnergyDischarged.IsNull()); // Cluster default is null

    // ChargingPreferences feature attributes
    DataModel::Nullable<uint32_t> nextChargeStartTime;
    ASSERT_EQ(tester.ReadAttribute(NextChargeStartTime::Id, nextChargeStartTime), CHIP_NO_ERROR);
    EXPECT_TRUE(nextChargeStartTime.IsNull()); // Cluster default is null

    DataModel::Nullable<uint32_t> nextChargeTargetTime;
    ASSERT_EQ(tester.ReadAttribute(NextChargeTargetTime::Id, nextChargeTargetTime), CHIP_NO_ERROR);
    EXPECT_TRUE(nextChargeTargetTime.IsNull()); // Cluster default is null

    DataModel::Nullable<int64_t> nextChargeRequiredEnergy;
    ASSERT_EQ(tester.ReadAttribute(NextChargeRequiredEnergy::Id, nextChargeRequiredEnergy), CHIP_NO_ERROR);
    EXPECT_TRUE(nextChargeRequiredEnergy.IsNull()); // Cluster default is null

    DataModel::Nullable<Percent> nextChargeTargetSoC;
    ASSERT_EQ(tester.ReadAttribute(NextChargeTargetSoC::Id, nextChargeTargetSoC), CHIP_NO_ERROR);
    EXPECT_TRUE(nextChargeTargetSoC.IsNull()); // Cluster default is null

    // SoCReporting feature attributes
    DataModel::Nullable<Percent> stateOfCharge;
    ASSERT_EQ(tester.ReadAttribute(StateOfCharge::Id, stateOfCharge), CHIP_NO_ERROR);
    EXPECT_TRUE(stateOfCharge.IsNull()); // Cluster default is null

    DataModel::Nullable<int64_t> batteryCapacity;
    ASSERT_EQ(tester.ReadAttribute(BatteryCapacity::Id, batteryCapacity), CHIP_NO_ERROR);
    EXPECT_TRUE(batteryCapacity.IsNull()); // Cluster default is null

    // PlugAndCharge feature attributes
    DataModel::Nullable<CharSpan> vehicleID;
    ASSERT_EQ(tester.ReadAttribute(VehicleID::Id, vehicleID), CHIP_NO_ERROR);
    EXPECT_TRUE(vehicleID.IsNull()); // Cluster default is null

    // Optional attributes - cluster owns the data with initial defaults
    int64_t userMaximumChargeCurrent = -1;
    ASSERT_EQ(tester.ReadAttribute(UserMaximumChargeCurrent::Id, userMaximumChargeCurrent), CHIP_NO_ERROR);
    EXPECT_EQ(userMaximumChargeCurrent, 0); // Cluster's default initial value

    uint32_t randomizationDelayWindow = 0;
    ASSERT_EQ(tester.ReadAttribute(RandomizationDelayWindow::Id, randomizationDelayWindow), CHIP_NO_ERROR);
    EXPECT_EQ(randomizationDelayWindow, 600u); // Cluster's default initial value (600s per spec)

    DataModel::Nullable<uint16_t> approximateEVEfficiency;
    ASSERT_EQ(tester.ReadAttribute(ApproximateEVEfficiency::Id, approximateEVEfficiency), CHIP_NO_ERROR);
    EXPECT_TRUE(approximateEVEfficiency.IsNull()); // Cluster's default initial value is null

    // Session attributes - cluster owns the data with defaults
    DataModel::Nullable<uint32_t> sessionID;
    ASSERT_EQ(tester.ReadAttribute(SessionID::Id, sessionID), CHIP_NO_ERROR);
    EXPECT_TRUE(sessionID.IsNull()); // Cluster default is null

    DataModel::Nullable<uint32_t> sessionDuration;
    ASSERT_EQ(tester.ReadAttribute(SessionDuration::Id, sessionDuration), CHIP_NO_ERROR);
    EXPECT_TRUE(sessionDuration.IsNull()); // Cluster default is null

    DataModel::Nullable<int64_t> sessionEnergyCharged;
    ASSERT_EQ(tester.ReadAttribute(SessionEnergyCharged::Id, sessionEnergyCharged), CHIP_NO_ERROR);
    EXPECT_TRUE(sessionEnergyCharged.IsNull()); // Cluster default is null

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestEnergyEvseCluster, TestWriteAttributes)
{
    // Test writing to writable attributes
    TestServerClusterContext context;
    MockEvseDelegate mockDelegate;
    BitMask<Feature> allFeatures(Feature::kChargingPreferences, Feature::kV2x, Feature::kSoCReporting, Feature::kPlugAndCharge);
    BitMask<OptionalAttributes> optionalAttributes(OptionalAttributes::kSupportsUserMaximumChargingCurrent,
                                                   OptionalAttributes::kSupportsRandomizationWindow,
                                                   OptionalAttributes::kSupportsApproximateEvEfficiency);
    BitMask<OptionalCommands> optionalCommands;

    EnergyEvseCluster cluster(
        EnergyEvseCluster::Config(kTestEndpointId, mockDelegate, allFeatures, optionalAttributes, optionalCommands));
    mockDelegate.SetCluster(cluster);

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    // Test writing UserMaximumChargeCurrent
    int64_t newUserMaximumChargeCurrent = 20000; // 20A in mA
    EXPECT_TRUE(tester.WriteAttribute(UserMaximumChargeCurrent::Id, newUserMaximumChargeCurrent).IsSuccess());

    // Read back and verify
    int64_t readUserMaximumChargeCurrent = 0;
    ASSERT_EQ(tester.ReadAttribute(UserMaximumChargeCurrent::Id, readUserMaximumChargeCurrent), CHIP_NO_ERROR);
    EXPECT_EQ(readUserMaximumChargeCurrent, newUserMaximumChargeCurrent);

    // Test writing RandomizationDelayWindow
    uint32_t newRandomizationDelayWindow = 300; // 5 minutes
    EXPECT_TRUE(tester.WriteAttribute(RandomizationDelayWindow::Id, newRandomizationDelayWindow).IsSuccess());

    // Read back and verify
    uint32_t readRandomizationDelayWindow = 0;
    ASSERT_EQ(tester.ReadAttribute(RandomizationDelayWindow::Id, readRandomizationDelayWindow), CHIP_NO_ERROR);
    EXPECT_EQ(readRandomizationDelayWindow, newRandomizationDelayWindow);

    // Test writing ApproximateEVEfficiency
    DataModel::Nullable<uint16_t> newApproximateEVEfficiency(200); // 200 Wh/km
    EXPECT_TRUE(tester.WriteAttribute(ApproximateEVEfficiency::Id, newApproximateEVEfficiency).IsSuccess());

    // Read back and verify
    DataModel::Nullable<uint16_t> readApproximateEVEfficiency;
    ASSERT_EQ(tester.ReadAttribute(ApproximateEVEfficiency::Id, readApproximateEVEfficiency), CHIP_NO_ERROR);
    ASSERT_FALSE(readApproximateEVEfficiency.IsNull());
    EXPECT_EQ(readApproximateEVEfficiency.Value(), 200);

    // Test writing ApproximateEVEfficiency with null value
    DataModel::Nullable<uint16_t> nullValue;
    nullValue.SetNull();
    EXPECT_TRUE(tester.WriteAttribute(ApproximateEVEfficiency::Id, nullValue).IsSuccess());

    // Read back and verify it's null
    DataModel::Nullable<uint16_t> readNullValue;
    ASSERT_EQ(tester.ReadAttribute(ApproximateEVEfficiency::Id, readNullValue), CHIP_NO_ERROR);
    EXPECT_TRUE(readNullValue.IsNull());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestEnergyEvseCluster, TestWriteAttributesNotifiesChange)
{
    // Test that writing to writable attributes generates attribute change notifications
    TestServerClusterContext context;
    MockEvseDelegate mockDelegate;
    BitMask<Feature> allFeatures(Feature::kChargingPreferences, Feature::kV2x, Feature::kSoCReporting, Feature::kPlugAndCharge);
    BitMask<OptionalAttributes> optionalAttributes(OptionalAttributes::kSupportsUserMaximumChargingCurrent,
                                                   OptionalAttributes::kSupportsRandomizationWindow,
                                                   OptionalAttributes::kSupportsApproximateEvEfficiency);
    BitMask<OptionalCommands> optionalCommands;

    EnergyEvseCluster cluster(
        EnergyEvseCluster::Config(kTestEndpointId, mockDelegate, allFeatures, optionalAttributes, optionalCommands));
    mockDelegate.SetCluster(cluster);

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    auto & dirtyList = context.ChangeListener().DirtyList();

    // --- Test UserMaximumChargeCurrent ---
    dirtyList.clear();
    EXPECT_TRUE(tester.WriteAttribute(UserMaximumChargeCurrent::Id, static_cast<int64_t>(20000)).IsSuccess());
    EXPECT_EQ(dirtyList.size(), 1u);
    EXPECT_EQ(dirtyList[0].mAttributeId, UserMaximumChargeCurrent::Id);

    // --- Test RandomizationDelayWindow ---
    dirtyList.clear();
    EXPECT_TRUE(tester.WriteAttribute(RandomizationDelayWindow::Id, static_cast<uint32_t>(300)).IsSuccess());
    EXPECT_EQ(dirtyList.size(), 1u);
    EXPECT_EQ(dirtyList[0].mAttributeId, RandomizationDelayWindow::Id);

    // --- Test ApproximateEVEfficiency ---
    dirtyList.clear();
    EXPECT_TRUE(tester.WriteAttribute(ApproximateEVEfficiency::Id, DataModel::Nullable<uint16_t>(200)).IsSuccess());
    EXPECT_EQ(dirtyList.size(), 1u);
    EXPECT_EQ(dirtyList[0].mAttributeId, ApproximateEVEfficiency::Id);

    // Write same values again - should not generate notifications
    dirtyList.clear();
    EXPECT_TRUE(tester.WriteAttribute(UserMaximumChargeCurrent::Id, static_cast<int64_t>(20000)).IsSuccess());
    EXPECT_TRUE(tester.WriteAttribute(RandomizationDelayWindow::Id, static_cast<uint32_t>(300)).IsSuccess());
    EXPECT_TRUE(tester.WriteAttribute(ApproximateEVEfficiency::Id, DataModel::Nullable<uint16_t>(200)).IsSuccess());
    EXPECT_TRUE(dirtyList.empty());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestEnergyEvseCluster, TestWriteReadOnlyAttributesFails)
{
    // Test that writing to read-only attributes fails with UnsupportedWrite
    TestServerClusterContext context;
    MockEvseDelegate mockDelegate;
    BitMask<Feature> allFeatures(Feature::kChargingPreferences, Feature::kV2x, Feature::kSoCReporting, Feature::kPlugAndCharge);
    BitMask<OptionalAttributes> optionalAttributes(OptionalAttributes::kSupportsUserMaximumChargingCurrent,
                                                   OptionalAttributes::kSupportsRandomizationWindow,
                                                   OptionalAttributes::kSupportsApproximateEvEfficiency);
    BitMask<OptionalCommands> optionalCommands;

    EnergyEvseCluster cluster(
        EnergyEvseCluster::Config(kTestEndpointId, mockDelegate, allFeatures, optionalAttributes, optionalCommands));
    mockDelegate.SetCluster(cluster);

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    // Test writing to State (read-only)
    EXPECT_FALSE(tester.WriteAttribute(State::Id, StateEnum::kPluggedInCharging).IsSuccess());

    // Test writing to SupplyState (read-only)
    EXPECT_FALSE(tester.WriteAttribute(SupplyState::Id, SupplyStateEnum::kDisabled).IsSuccess());

    // Test writing to FaultState (read-only)
    EXPECT_FALSE(tester.WriteAttribute(FaultState::Id, FaultStateEnum::kGroundFault).IsSuccess());

    // Test writing to ChargingEnabledUntil (read-only)
    EXPECT_FALSE(tester.WriteAttribute(ChargingEnabledUntil::Id, DataModel::Nullable<uint32_t>(5000)).IsSuccess());

    // Test writing to CircuitCapacity (read-only)
    EXPECT_FALSE(tester.WriteAttribute(CircuitCapacity::Id, static_cast<int64_t>(50000)).IsSuccess());

    // Test writing to MinimumChargeCurrent (read-only)
    EXPECT_FALSE(tester.WriteAttribute(MinimumChargeCurrent::Id, static_cast<int64_t>(8000)).IsSuccess());

    // Test writing to MaximumChargeCurrent (read-only)
    EXPECT_FALSE(tester.WriteAttribute(MaximumChargeCurrent::Id, static_cast<int64_t>(40000)).IsSuccess());

    // Test writing to SessionID (read-only)
    EXPECT_FALSE(tester.WriteAttribute(SessionID::Id, DataModel::Nullable<uint32_t>(99999)).IsSuccess());

    // Test writing to SessionDuration (read-only)
    EXPECT_FALSE(tester.WriteAttribute(SessionDuration::Id, DataModel::Nullable<uint32_t>(9999)).IsSuccess());

    // Test writing to SessionEnergyCharged (read-only)
    EXPECT_FALSE(tester.WriteAttribute(SessionEnergyCharged::Id, DataModel::Nullable<int64_t>(99999999)).IsSuccess());

    // Test writing to StateOfCharge (read-only, SoCReporting feature)
    EXPECT_FALSE(tester.WriteAttribute(StateOfCharge::Id, DataModel::Nullable<Percent>(90)).IsSuccess());

    // Test writing to BatteryCapacity (read-only, SoCReporting feature)
    EXPECT_FALSE(tester.WriteAttribute(BatteryCapacity::Id, DataModel::Nullable<int64_t>(80000000)).IsSuccess());

    // Test writing to VehicleID (read-only, PlugAndCharge feature)
    EXPECT_FALSE(
        tester.WriteAttribute(VehicleID::Id, DataModel::Nullable<CharSpan>(CharSpan::fromCharString("NEW-VIN"))).IsSuccess());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// =============================================================================
// Programmatic SetXxx() Method Tests
// =============================================================================

TEST_F(TestEnergyEvseCluster, TestProgrammaticSetAttributes)
{
    // Test setting attributes via cluster's SetXxx() methods (for read-only attributes)
    TestServerClusterContext context;
    MockEvseDelegate mockDelegate;
    BitMask<Feature> allFeatures(Feature::kChargingPreferences, Feature::kV2x, Feature::kSoCReporting, Feature::kPlugAndCharge);
    BitMask<OptionalAttributes> optionalAttributes(OptionalAttributes::kSupportsUserMaximumChargingCurrent,
                                                   OptionalAttributes::kSupportsRandomizationWindow,
                                                   OptionalAttributes::kSupportsApproximateEvEfficiency);
    BitMask<OptionalCommands> optionalCommands;

    EnergyEvseCluster cluster(
        EnergyEvseCluster::Config(kTestEndpointId, mockDelegate, allFeatures, optionalAttributes, optionalCommands));
    mockDelegate.SetCluster(cluster);

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    auto & dirtyList = context.ChangeListener().DirtyList();

    // --- Test SetState (read-only attribute) ---
    dirtyList.clear();
    EXPECT_EQ(cluster.SetState(StateEnum::kPluggedInCharging), CHIP_NO_ERROR);

    // Verify notification was generated
    EXPECT_EQ(dirtyList.size(), 1u);
    EXPECT_EQ(dirtyList[0].mAttributeId, State::Id);

    // Verify delegate callback was called
    EXPECT_EQ(cluster.GetState(), StateEnum::kPluggedInCharging);

    // Verify cluster stores the value
    StateEnum readState = StateEnum::kUnknownEnumValue;
    ASSERT_EQ(tester.ReadAttribute(State::Id, readState), CHIP_NO_ERROR);
    EXPECT_EQ(readState, StateEnum::kPluggedInCharging);

    // --- Test SetSupplyState (read-only attribute) ---
    dirtyList.clear();
    EXPECT_EQ(cluster.SetSupplyState(SupplyStateEnum::kChargingEnabled), CHIP_NO_ERROR);

    EXPECT_EQ(dirtyList.size(), 1u);
    EXPECT_EQ(dirtyList[0].mAttributeId, SupplyState::Id);
    EXPECT_EQ(cluster.GetSupplyState(), SupplyStateEnum::kChargingEnabled);

    SupplyStateEnum readSupplyState = SupplyStateEnum::kUnknownEnumValue;
    ASSERT_EQ(tester.ReadAttribute(SupplyState::Id, readSupplyState), CHIP_NO_ERROR);
    EXPECT_EQ(readSupplyState, SupplyStateEnum::kChargingEnabled);

    // --- Test SetFaultState (read-only attribute) ---
    dirtyList.clear();
    EXPECT_EQ(cluster.SetFaultState(FaultStateEnum::kGroundFault), CHIP_NO_ERROR);

    EXPECT_EQ(dirtyList.size(), 1u);
    EXPECT_EQ(dirtyList[0].mAttributeId, FaultState::Id);
    EXPECT_EQ(cluster.GetFaultState(), FaultStateEnum::kGroundFault);

    // --- Test SetCircuitCapacity (read-only attribute) ---
    dirtyList.clear();
    EXPECT_EQ(cluster.SetCircuitCapacity(48000), CHIP_NO_ERROR);

    EXPECT_EQ(dirtyList.size(), 1u);
    EXPECT_EQ(dirtyList[0].mAttributeId, CircuitCapacity::Id);
    EXPECT_EQ(cluster.GetCircuitCapacity(), 48000);

    int64_t readCircuitCapacity = 0;
    ASSERT_EQ(tester.ReadAttribute(CircuitCapacity::Id, readCircuitCapacity), CHIP_NO_ERROR);
    EXPECT_EQ(readCircuitCapacity, 48000);

    // --- Test SetMinimumChargeCurrent (read-only attribute) ---
    dirtyList.clear();
    EXPECT_EQ(cluster.SetMinimumChargeCurrent(8000), CHIP_NO_ERROR);

    EXPECT_EQ(dirtyList.size(), 1u);
    EXPECT_EQ(dirtyList[0].mAttributeId, MinimumChargeCurrent::Id);
    EXPECT_EQ(cluster.GetMinimumChargeCurrent(), 8000);

    // --- Test SetMaximumChargeCurrent (read-only attribute) ---
    dirtyList.clear();
    EXPECT_EQ(cluster.SetMaximumChargeCurrent(40000), CHIP_NO_ERROR);

    EXPECT_EQ(dirtyList.size(), 1u);
    EXPECT_EQ(dirtyList[0].mAttributeId, MaximumChargeCurrent::Id);
    EXPECT_EQ(cluster.GetMaximumChargeCurrent(), 40000);

    // --- Test SetStateOfCharge (read-only attribute, SoCReporting feature) ---
    dirtyList.clear();
    EXPECT_EQ(cluster.SetStateOfCharge(DataModel::MakeNullable(static_cast<Percent>(75))), CHIP_NO_ERROR);

    EXPECT_EQ(dirtyList.size(), 1u);
    EXPECT_EQ(dirtyList[0].mAttributeId, StateOfCharge::Id);
    ASSERT_FALSE(cluster.GetStateOfCharge().IsNull());
    EXPECT_EQ(cluster.GetStateOfCharge().Value(), 75);

    // --- Test SetBatteryCapacity (read-only attribute, SoCReporting feature) ---
    dirtyList.clear();
    EXPECT_EQ(cluster.SetBatteryCapacity(DataModel::MakeNullable(static_cast<int64_t>(80000000))), CHIP_NO_ERROR);

    EXPECT_EQ(dirtyList.size(), 1u);
    EXPECT_EQ(dirtyList[0].mAttributeId, BatteryCapacity::Id);
    ASSERT_FALSE(cluster.GetBatteryCapacity().IsNull());
    EXPECT_EQ(cluster.GetBatteryCapacity().Value(), 80000000);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestEnergyEvseCluster, TestProgrammaticSetNoOpWhenSameValue)
{
    // Test that setting the same value doesn't generate notifications (no-op behavior)
    TestServerClusterContext context;
    MockEvseDelegate mockDelegate;
    BitMask<Feature> allFeatures(Feature::kChargingPreferences, Feature::kV2x, Feature::kSoCReporting, Feature::kPlugAndCharge);
    BitMask<OptionalAttributes> optionalAttributes;
    BitMask<OptionalCommands> optionalCommands;

    EnergyEvseCluster cluster(
        EnergyEvseCluster::Config(kTestEndpointId, mockDelegate, allFeatures, optionalAttributes, optionalCommands));
    mockDelegate.SetCluster(cluster);

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    auto & dirtyList = context.ChangeListener().DirtyList();

    // First set to a specific value
    dirtyList.clear();
    EXPECT_EQ(cluster.SetState(StateEnum::kPluggedInDemand), CHIP_NO_ERROR);
    EXPECT_EQ(dirtyList.size(), 1u);

    // Set to the same value - should not generate notification
    dirtyList.clear();
    EXPECT_EQ(cluster.SetState(StateEnum::kPluggedInDemand), CHIP_NO_ERROR);
    EXPECT_TRUE(dirtyList.empty());

    // Same test for SupplyState
    dirtyList.clear();
    EXPECT_EQ(cluster.SetSupplyState(SupplyStateEnum::kEnabled), CHIP_NO_ERROR);
    EXPECT_EQ(dirtyList.size(), 1u);

    dirtyList.clear();
    EXPECT_EQ(cluster.SetSupplyState(SupplyStateEnum::kEnabled), CHIP_NO_ERROR);
    EXPECT_TRUE(dirtyList.empty());

    // Same test for CircuitCapacity
    dirtyList.clear();
    EXPECT_EQ(cluster.SetCircuitCapacity(32000), CHIP_NO_ERROR);
    EXPECT_EQ(dirtyList.size(), 1u);

    dirtyList.clear();
    EXPECT_EQ(cluster.SetCircuitCapacity(32000), CHIP_NO_ERROR);
    EXPECT_TRUE(dirtyList.empty());

    // Same test for nullable attribute (StateOfCharge)
    dirtyList.clear();
    EXPECT_EQ(cluster.SetStateOfCharge(DataModel::MakeNullable(static_cast<Percent>(50))), CHIP_NO_ERROR);
    EXPECT_EQ(dirtyList.size(), 1u);

    dirtyList.clear();
    EXPECT_EQ(cluster.SetStateOfCharge(DataModel::MakeNullable(static_cast<Percent>(50))), CHIP_NO_ERROR);
    EXPECT_TRUE(dirtyList.empty());

    // Test setting to null and then same null
    dirtyList.clear();
    EXPECT_EQ(cluster.SetStateOfCharge(DataModel::NullNullable), CHIP_NO_ERROR);
    EXPECT_EQ(dirtyList.size(), 1u);

    dirtyList.clear();
    EXPECT_EQ(cluster.SetStateOfCharge(DataModel::NullNullable), CHIP_NO_ERROR);
    EXPECT_TRUE(dirtyList.empty());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// =============================================================================
// Disable Command Tests
// =============================================================================

TEST_F(TestEnergyEvseCluster, TestDisable)
{
    TestServerClusterContext context;
    MockEvseDelegate mockDelegate;
    BitMask<Feature> noFeatures;
    BitMask<OptionalAttributes> optionalAttributes;
    BitMask<OptionalCommands> optionalCommands;

    EnergyEvseCluster cluster(
        EnergyEvseCluster::Config(kTestEndpointId, mockDelegate, noFeatures, optionalAttributes, optionalCommands));
    mockDelegate.SetCluster(cluster);

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    Commands::Disable::Type command;
    EXPECT_TRUE(tester.Invoke(Commands::Disable::Id, command).IsSuccess());

    EXPECT_EQ(cluster.GetSupplyState(), SupplyStateEnum::kDisabled);
    EXPECT_EQ(cluster.GetChargingEnabledUntil().Value(), 0u);
    EXPECT_EQ(cluster.GetDischargingEnabledUntil().Value(), 0u);
    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// =============================================================================
// EnableCharging Command Tests
// =============================================================================

TEST_F(TestEnergyEvseCluster, TestEnableCharging)
{
    TestServerClusterContext context;
    MockEvseDelegate mockDelegate;
    BitMask<Feature> noFeatures;
    BitMask<OptionalAttributes> optionalAttributes;
    BitMask<OptionalCommands> optionalCommands;

    EnergyEvseCluster cluster(
        EnergyEvseCluster::Config(kTestEndpointId, mockDelegate, noFeatures, optionalAttributes, optionalCommands));
    mockDelegate.SetCluster(cluster);

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    Commands::EnableCharging::Type command;

    // --- Constraint error tests ---

    // minimumChargeCurrent < 0 (below kMinimumChargeCurrentLimit)
    command.chargingEnabledUntil = 5000;
    command.minimumChargeCurrent = -1;
    command.maximumChargeCurrent = 32000;
    EXPECT_FALSE(tester.Invoke(Commands::EnableCharging::Id, command).IsSuccess());

    // maximumChargeCurrent < 0 (below kMinimumChargeCurrentLimit)
    command.minimumChargeCurrent = 6000;
    command.maximumChargeCurrent = -1;
    EXPECT_FALSE(tester.Invoke(Commands::EnableCharging::Id, command).IsSuccess());

    // minimumChargeCurrent > maximumChargeCurrent
    command.minimumChargeCurrent = 32000;
    command.maximumChargeCurrent = 6000;
    EXPECT_FALSE(tester.Invoke(Commands::EnableCharging::Id, command).IsSuccess());

    // --- Failure with FaultState ---

    EXPECT_EQ(cluster.SetFaultState(FaultStateEnum::kGroundFault), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetSupplyState(SupplyStateEnum::kDisabled), CHIP_NO_ERROR);
    command.chargingEnabledUntil = 5000;
    command.minimumChargeCurrent = 6000;
    command.maximumChargeCurrent = 32000;
    EXPECT_FALSE(tester.Invoke(Commands::EnableCharging::Id, command).IsSuccess());
    EXPECT_EQ(cluster.GetSupplyState(), SupplyStateEnum::kDisabled);

    // --- Failure with DiagnosticsActive ---

    EXPECT_EQ(cluster.SetFaultState(FaultStateEnum::kNoError), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetSupplyState(SupplyStateEnum::kDisabledDiagnostics), CHIP_NO_ERROR);
    EXPECT_FALSE(tester.Invoke(Commands::EnableCharging::Id, command).IsSuccess());
    EXPECT_EQ(cluster.GetSupplyState(), SupplyStateEnum::kDisabledDiagnostics);

    // --- Success from Disabled state -> ChargingEnabled ---

    EXPECT_EQ(cluster.SetSupplyState(SupplyStateEnum::kDisabled), CHIP_NO_ERROR);
    command.chargingEnabledUntil = 5000;
    command.minimumChargeCurrent = 8000;
    command.maximumChargeCurrent = 40000;
    EXPECT_TRUE(tester.Invoke(Commands::EnableCharging::Id, command).IsSuccess());
    EXPECT_EQ(cluster.GetSupplyState(), SupplyStateEnum::kChargingEnabled);
    EXPECT_EQ(cluster.GetChargingEnabledUntil().Value(), 5000u);
    EXPECT_EQ(cluster.GetMinimumChargeCurrent(), 8000);
    EXPECT_EQ(cluster.GetMaximumChargeCurrent(), 40000);

    // --- Success from DisabledError state -> ChargingEnabled ---

    EXPECT_EQ(cluster.SetSupplyState(SupplyStateEnum::kDisabledError), CHIP_NO_ERROR);
    command.chargingEnabledUntil = 6000;
    command.minimumChargeCurrent = 6000;
    command.maximumChargeCurrent = 32000;
    EXPECT_TRUE(tester.Invoke(Commands::EnableCharging::Id, command).IsSuccess());
    EXPECT_EQ(cluster.GetSupplyState(), SupplyStateEnum::kChargingEnabled);
    EXPECT_EQ(cluster.GetChargingEnabledUntil().Value(), 6000u);

    // --- Success from DischargingEnabled state -> Enabled (both) ---

    EXPECT_EQ(cluster.SetSupplyState(SupplyStateEnum::kDischargingEnabled), CHIP_NO_ERROR);
    command.chargingEnabledUntil = 7000;
    command.minimumChargeCurrent = 6000;
    command.maximumChargeCurrent = 24000;
    EXPECT_TRUE(tester.Invoke(Commands::EnableCharging::Id, command).IsSuccess());
    EXPECT_EQ(cluster.GetSupplyState(), SupplyStateEnum::kEnabled);
    EXPECT_EQ(cluster.GetChargingEnabledUntil().Value(), 7000u);

    // --- Success with null timestamp (indefinite charging) ---

    EXPECT_EQ(cluster.SetSupplyState(SupplyStateEnum::kDisabled), CHIP_NO_ERROR);
    command.chargingEnabledUntil.SetNull();
    command.minimumChargeCurrent = 6000;
    command.maximumChargeCurrent = 32000;
    EXPECT_TRUE(tester.Invoke(Commands::EnableCharging::Id, command).IsSuccess());
    EXPECT_EQ(cluster.GetSupplyState(), SupplyStateEnum::kChargingEnabled);
    EXPECT_TRUE(cluster.GetChargingEnabledUntil().IsNull());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// =============================================================================
// EnableDischarging Command Tests
// =============================================================================

TEST_F(TestEnergyEvseCluster, TestEnableDischarging)
{
    TestServerClusterContext context;
    MockEvseDelegate mockDelegate;
    BitMask<Feature> features(Feature::kV2x); // V2x feature enables discharging
    BitMask<OptionalAttributes> optionalAttributes;
    BitMask<OptionalCommands> optionalCommands;

    EnergyEvseCluster cluster(
        EnergyEvseCluster::Config(kTestEndpointId, mockDelegate, features, optionalAttributes, optionalCommands));
    mockDelegate.SetCluster(cluster);

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    Commands::EnableDischarging::Type command;

    // --- Constraint error test ---

    // maximumDischargeCurrent < 0 (below kMinimumChargeCurrentLimit)
    command.dischargingEnabledUntil = 5000;
    command.maximumDischargeCurrent = -1;
    EXPECT_FALSE(tester.Invoke(Commands::EnableDischarging::Id, command).IsSuccess());

    // --- Failure with FaultState ---

    EXPECT_EQ(cluster.SetFaultState(FaultStateEnum::kGroundFault), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetSupplyState(SupplyStateEnum::kDisabled), CHIP_NO_ERROR);
    command.dischargingEnabledUntil = 5000;
    command.maximumDischargeCurrent = 16000;
    EXPECT_FALSE(tester.Invoke(Commands::EnableDischarging::Id, command).IsSuccess());
    EXPECT_EQ(cluster.GetSupplyState(), SupplyStateEnum::kDisabled);

    // --- Failure with DiagnosticsActive ---

    EXPECT_EQ(cluster.SetFaultState(FaultStateEnum::kNoError), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetSupplyState(SupplyStateEnum::kDisabledDiagnostics), CHIP_NO_ERROR);
    EXPECT_FALSE(tester.Invoke(Commands::EnableDischarging::Id, command).IsSuccess());
    EXPECT_EQ(cluster.GetSupplyState(), SupplyStateEnum::kDisabledDiagnostics);

    // --- Success from Disabled state -> DischargingEnabled ---

    EXPECT_EQ(cluster.SetSupplyState(SupplyStateEnum::kDisabled), CHIP_NO_ERROR);
    command.dischargingEnabledUntil = 5000;
    command.maximumDischargeCurrent = 16000;
    EXPECT_TRUE(tester.Invoke(Commands::EnableDischarging::Id, command).IsSuccess());
    EXPECT_EQ(cluster.GetSupplyState(), SupplyStateEnum::kDischargingEnabled);
    EXPECT_EQ(cluster.GetDischargingEnabledUntil().Value(), 5000u);
    EXPECT_EQ(cluster.GetMaximumDischargeCurrent(), 16000);

    // --- Success from DisabledError state -> DischargingEnabled ---

    EXPECT_EQ(cluster.SetSupplyState(SupplyStateEnum::kDisabledError), CHIP_NO_ERROR);
    command.dischargingEnabledUntil = 6000;
    command.maximumDischargeCurrent = 12000;
    EXPECT_TRUE(tester.Invoke(Commands::EnableDischarging::Id, command).IsSuccess());
    EXPECT_EQ(cluster.GetSupplyState(), SupplyStateEnum::kDischargingEnabled);
    EXPECT_EQ(cluster.GetDischargingEnabledUntil().Value(), 6000u);

    // --- Success from ChargingEnabled state -> Enabled (both) ---

    EXPECT_EQ(cluster.SetSupplyState(SupplyStateEnum::kChargingEnabled), CHIP_NO_ERROR);
    command.dischargingEnabledUntil = 7000;
    command.maximumDischargeCurrent = 10000;
    EXPECT_TRUE(tester.Invoke(Commands::EnableDischarging::Id, command).IsSuccess());
    EXPECT_EQ(cluster.GetSupplyState(), SupplyStateEnum::kEnabled);
    EXPECT_EQ(cluster.GetDischargingEnabledUntil().Value(), 7000u);

    // --- Success with null timestamp (indefinite discharging) ---

    EXPECT_EQ(cluster.SetSupplyState(SupplyStateEnum::kDisabled), CHIP_NO_ERROR);
    command.dischargingEnabledUntil.SetNull();
    command.maximumDischargeCurrent = 16000;
    EXPECT_TRUE(tester.Invoke(Commands::EnableDischarging::Id, command).IsSuccess());
    EXPECT_EQ(cluster.GetSupplyState(), SupplyStateEnum::kDischargingEnabled);
    EXPECT_TRUE(cluster.GetDischargingEnabledUntil().IsNull());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// =============================================================================
// StartDiagnostics Command Tests
// =============================================================================

TEST_F(TestEnergyEvseCluster, TestStartDiagnostics)
{
    TestServerClusterContext context;
    MockEvseDelegate mockDelegate;
    BitMask<Feature> noFeatures;
    BitMask<OptionalAttributes> optionalAttributes;
    // Test with command supported
    {
        BitMask<OptionalCommands> optionalCommands(OptionalCommands::kSupportsStartDiagnostics);
        EnergyEvseCluster cluster(
            EnergyEvseCluster::Config(kTestEndpointId, mockDelegate, noFeatures, optionalAttributes, optionalCommands));
        mockDelegate.SetCluster(cluster);

        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        ClusterTester tester(cluster);
        Commands::StartDiagnostics::Type command;

        EXPECT_EQ(cluster.SetSupplyState(SupplyStateEnum::kDisabled), CHIP_NO_ERROR);
        EXPECT_TRUE(tester.Invoke(Commands::StartDiagnostics::Id, command).IsSuccess());
        EXPECT_EQ(cluster.GetSupplyState(), SupplyStateEnum::kDisabledDiagnostics);

        // --- Failure with DiagnosticsActive ---

        EXPECT_EQ(cluster.SetSupplyState(SupplyStateEnum::kDisabledDiagnostics), CHIP_NO_ERROR);
        EXPECT_FALSE(tester.Invoke(Commands::StartDiagnostics::Id, command).IsSuccess());

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
    // Test with command not supported
    {
        BitMask<OptionalCommands> optionalCommands;
        EnergyEvseCluster cluster(
            EnergyEvseCluster::Config(kTestEndpointId, mockDelegate, noFeatures, optionalAttributes, optionalCommands));
        mockDelegate.SetCluster(cluster);
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        ClusterTester tester(cluster);
        Commands::StartDiagnostics::Type command;

        EXPECT_EQ(cluster.SetSupplyState(SupplyStateEnum::kDisabled), CHIP_NO_ERROR);
        EXPECT_FALSE(tester.Invoke(Commands::StartDiagnostics::Id, command).IsSuccess());
        EXPECT_EQ(cluster.GetSupplyState(), SupplyStateEnum::kDisabled);

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

// =============================================================================
// SetTargets, GetTargets, ClearTargets Command Tests
// =============================================================================

TEST_F(TestEnergyEvseCluster, TestTargetsCommands)
{
    TestServerClusterContext context;
    MockEvseDelegate mockDelegate;
    // ChargingPreferences feature is required for targets commands
    // SoCReporting feature requires TargetSoC to be present
    BitMask<Feature> features(Feature::kChargingPreferences, Feature::kSoCReporting);
    BitMask<OptionalAttributes> optionalAttributes;
    BitMask<OptionalCommands> optionalCommands;

    EnergyEvseCluster cluster(
        EnergyEvseCluster::Config(kTestEndpointId, mockDelegate, features, optionalAttributes, optionalCommands));
    mockDelegate.SetCluster(cluster);
    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    // --- Setup valid targets for Monday at 8:00 AM (480 minutes) ---

    Structs::ChargingTargetStruct::Type mondayTarget;
    mondayTarget.targetTimeMinutesPastMidnight = 480; // 8:00 AM
    mondayTarget.targetSoC.SetValue(80);              // 80% SoC
    mondayTarget.addedEnergy.SetValue(10000000);      // 10 kWh

    Structs::ChargingTargetScheduleStruct::Type mondaySchedule;
    mondaySchedule.dayOfWeekForSequence = BitMask<TargetDayOfWeekBitmap>(TargetDayOfWeekBitmap::kMonday);
    mondaySchedule.chargingTargets      = DataModel::List<const Structs::ChargingTargetStruct::Type>(&mondayTarget, 1);

    Commands::SetTargets::Type setTargetsCmd;
    setTargetsCmd.chargingTargetSchedules = DataModel::List<const Structs::ChargingTargetScheduleStruct::Type>(&mondaySchedule, 1);

    // --- Success: Set valid targets for Monday ---

    EXPECT_TRUE(tester.Invoke(Commands::SetTargets::Id, setTargetsCmd).IsSuccess());
    EXPECT_EQ(mockDelegate.GetTotalTargetsCount(), 1u);
    EXPECT_EQ(mockDelegate.GetDaysWithTargets(), static_cast<uint8_t>(TargetDayOfWeekBitmap::kMonday));

    // --- ClearTargets: Verify targets are cleared ---

    Commands::ClearTargets::Type clearTargetsCmd;
    EXPECT_TRUE(tester.Invoke(Commands::ClearTargets::Id, clearTargetsCmd).IsSuccess());
    EXPECT_EQ(mockDelegate.GetTotalTargetsCount(), 0u);
    EXPECT_EQ(mockDelegate.GetDaysWithTargets(), 0u);

    // --- Constraint Error: MinutesPastMidnight > 1439 ---

    Structs::ChargingTargetStruct::Type invalidTimeTarget;
    invalidTimeTarget.targetTimeMinutesPastMidnight = 1440; // Invalid: max is 1439
    invalidTimeTarget.targetSoC.SetValue(80);

    Structs::ChargingTargetScheduleStruct::Type invalidTimeSchedule;
    invalidTimeSchedule.dayOfWeekForSequence = BitMask<TargetDayOfWeekBitmap>(TargetDayOfWeekBitmap::kTuesday);
    invalidTimeSchedule.chargingTargets      = DataModel::List<const Structs::ChargingTargetStruct::Type>(&invalidTimeTarget, 1);

    setTargetsCmd.chargingTargetSchedules =
        DataModel::List<const Structs::ChargingTargetScheduleStruct::Type>(&invalidTimeSchedule, 1);
    EXPECT_FALSE(tester.Invoke(Commands::SetTargets::Id, setTargetsCmd).IsSuccess());

    // --- Constraint Error: TargetSoC > 100 ---

    Structs::ChargingTargetStruct::Type invalidSoCTarget;
    invalidSoCTarget.targetTimeMinutesPastMidnight = 480;
    invalidSoCTarget.targetSoC.SetValue(101); // Invalid: max is 100

    Structs::ChargingTargetScheduleStruct::Type invalidSoCSchedule;
    invalidSoCSchedule.dayOfWeekForSequence = BitMask<TargetDayOfWeekBitmap>(TargetDayOfWeekBitmap::kWednesday);
    invalidSoCSchedule.chargingTargets      = DataModel::List<const Structs::ChargingTargetStruct::Type>(&invalidSoCTarget, 1);

    setTargetsCmd.chargingTargetSchedules =
        DataModel::List<const Structs::ChargingTargetScheduleStruct::Type>(&invalidSoCSchedule, 1);
    EXPECT_FALSE(tester.Invoke(Commands::SetTargets::Id, setTargetsCmd).IsSuccess());

    // --- Constraint Error: Duplicate day in multiple schedules ---

    Structs::ChargingTargetStruct::Type target1, target2;
    target1.targetTimeMinutesPastMidnight = 480;
    target1.targetSoC.SetValue(80);
    target2.targetTimeMinutesPastMidnight = 600;
    target2.targetSoC.SetValue(90);

    Structs::ChargingTargetScheduleStruct::Type schedules[2];
    schedules[0].dayOfWeekForSequence = BitMask<TargetDayOfWeekBitmap>(TargetDayOfWeekBitmap::kThursday);
    schedules[0].chargingTargets      = DataModel::List<const Structs::ChargingTargetStruct::Type>(&target1, 1);
    schedules[1].dayOfWeekForSequence = BitMask<TargetDayOfWeekBitmap>(TargetDayOfWeekBitmap::kThursday); // Duplicate!
    schedules[1].chargingTargets      = DataModel::List<const Structs::ChargingTargetStruct::Type>(&target2, 1);

    setTargetsCmd.chargingTargetSchedules = DataModel::List<const Structs::ChargingTargetScheduleStruct::Type>(schedules, 2);
    EXPECT_FALSE(tester.Invoke(Commands::SetTargets::Id, setTargetsCmd).IsSuccess());

    // --- Success: Set targets for multiple days ---

    Structs::ChargingTargetStruct::Type weekdayTarget;
    weekdayTarget.targetTimeMinutesPastMidnight = 420; // 7:00 AM
    weekdayTarget.targetSoC.SetValue(85);

    Structs::ChargingTargetStruct::Type weekendTarget;
    weekendTarget.targetTimeMinutesPastMidnight = 600; // 10:00 AM
    weekendTarget.targetSoC.SetValue(100);

    Structs::ChargingTargetScheduleStruct::Type multiDaySchedules[2];
    // Monday through Friday
    multiDaySchedules[0].dayOfWeekForSequence = BitMask<TargetDayOfWeekBitmap>(
        TargetDayOfWeekBitmap::kMonday, TargetDayOfWeekBitmap::kTuesday, TargetDayOfWeekBitmap::kWednesday,
        TargetDayOfWeekBitmap::kThursday, TargetDayOfWeekBitmap::kFriday);
    multiDaySchedules[0].chargingTargets = DataModel::List<const Structs::ChargingTargetStruct::Type>(&weekdayTarget, 1);
    // Saturday and Sunday
    multiDaySchedules[1].dayOfWeekForSequence =
        BitMask<TargetDayOfWeekBitmap>(TargetDayOfWeekBitmap::kSaturday, TargetDayOfWeekBitmap::kSunday);
    multiDaySchedules[1].chargingTargets = DataModel::List<const Structs::ChargingTargetStruct::Type>(&weekendTarget, 1);

    setTargetsCmd.chargingTargetSchedules =
        DataModel::List<const Structs::ChargingTargetScheduleStruct::Type>(multiDaySchedules, 2);
    EXPECT_TRUE(tester.Invoke(Commands::SetTargets::Id, setTargetsCmd).IsSuccess());
    EXPECT_EQ(mockDelegate.GetTotalTargetsCount(), 7u);  // 7 days, 1 target each
    EXPECT_EQ(mockDelegate.GetDaysWithTargets(), 0x7Fu); // All 7 days

    // --- Success: Replace targets for Saturday only ---

    Structs::ChargingTargetStruct::Type newSaturdayTarget;
    newSaturdayTarget.targetTimeMinutesPastMidnight = 720; // 12:00 PM
    newSaturdayTarget.targetSoC.SetValue(50);

    Structs::ChargingTargetScheduleStruct::Type saturdaySchedule;
    saturdaySchedule.dayOfWeekForSequence = BitMask<TargetDayOfWeekBitmap>(TargetDayOfWeekBitmap::kSaturday);
    saturdaySchedule.chargingTargets      = DataModel::List<const Structs::ChargingTargetStruct::Type>(&newSaturdayTarget, 1);

    setTargetsCmd.chargingTargetSchedules =
        DataModel::List<const Structs::ChargingTargetScheduleStruct::Type>(&saturdaySchedule, 1);
    EXPECT_TRUE(tester.Invoke(Commands::SetTargets::Id, setTargetsCmd).IsSuccess());
    EXPECT_EQ(mockDelegate.GetTotalTargetsCount(), 7u);  // Still 7 targets
    EXPECT_EQ(mockDelegate.GetDaysWithTargets(), 0x7Fu); // All 7 days still

    // --- Success: Clear targets for Sunday by setting empty list ---

    Structs::ChargingTargetScheduleStruct::Type emptySundaySchedule;
    emptySundaySchedule.dayOfWeekForSequence = BitMask<TargetDayOfWeekBitmap>(TargetDayOfWeekBitmap::kSunday);
    emptySundaySchedule.chargingTargets      = DataModel::List<const Structs::ChargingTargetStruct::Type>();

    setTargetsCmd.chargingTargetSchedules =
        DataModel::List<const Structs::ChargingTargetScheduleStruct::Type>(&emptySundaySchedule, 1);
    EXPECT_TRUE(tester.Invoke(Commands::SetTargets::Id, setTargetsCmd).IsSuccess());
    EXPECT_EQ(mockDelegate.GetTotalTargetsCount(), 6u);  // 6 targets now
    EXPECT_EQ(mockDelegate.GetDaysWithTargets(), 0x7Eu); // All except Sunday

    // --- Final ClearTargets ---

    EXPECT_TRUE(tester.Invoke(Commands::ClearTargets::Id, clearTargetsCmd).IsSuccess());
    EXPECT_EQ(mockDelegate.GetTotalTargetsCount(), 0u);
    EXPECT_EQ(mockDelegate.GetDaysWithTargets(), 0u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

} // namespace
