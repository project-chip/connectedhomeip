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
    }
    // Test with all features and no optional attributes & no optional commands
    {
        BitMask<Feature> allFeatures(Feature::kChargingPreferences, Feature::kV2x, Feature::kSoCReporting, Feature::kPlugAndCharge);
        BitMask<OptionalAttributes> optionalAttributes;
        BitMask<OptionalCommands> optionalCommands;

        EnergyEvseCluster cluster(
            EnergyEvseCluster::Config(kTestEndpointId, mockDelegate, allFeatures, optionalAttributes, optionalCommands));
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

    // Read mandatory attributes and verify values
    StateEnum state = StateEnum::kUnknownEnumValue;
    ASSERT_EQ(tester.ReadAttribute(State::Id, state), CHIP_NO_ERROR);
    EXPECT_EQ(state, MockEvseDelegate::kMockState);

    SupplyStateEnum supplyState = SupplyStateEnum::kUnknownEnumValue;
    ASSERT_EQ(tester.ReadAttribute(SupplyState::Id, supplyState), CHIP_NO_ERROR);
    EXPECT_EQ(supplyState, MockEvseDelegate::kMockSupplyState);

    FaultStateEnum faultState = FaultStateEnum::kUnknownEnumValue;
    ASSERT_EQ(tester.ReadAttribute(FaultState::Id, faultState), CHIP_NO_ERROR);
    EXPECT_EQ(faultState, MockEvseDelegate::kMockFaultState);

    DataModel::Nullable<uint32_t> chargingEnabledUntil;
    ASSERT_EQ(tester.ReadAttribute(ChargingEnabledUntil::Id, chargingEnabledUntil), CHIP_NO_ERROR);
    ASSERT_FALSE(chargingEnabledUntil.IsNull());
    EXPECT_EQ(chargingEnabledUntil.Value(), MockEvseDelegate::kMockChargingEnabledUntil);

    int64_t circuitCapacity = 0;
    ASSERT_EQ(tester.ReadAttribute(CircuitCapacity::Id, circuitCapacity), CHIP_NO_ERROR);
    EXPECT_EQ(circuitCapacity, MockEvseDelegate::kMockCircuitCapacity);

    int64_t minimumChargeCurrent = 0;
    ASSERT_EQ(tester.ReadAttribute(MinimumChargeCurrent::Id, minimumChargeCurrent), CHIP_NO_ERROR);
    EXPECT_EQ(minimumChargeCurrent, MockEvseDelegate::kMockMinimumChargeCurrent);

    int64_t maximumChargeCurrent = 0;
    ASSERT_EQ(tester.ReadAttribute(MaximumChargeCurrent::Id, maximumChargeCurrent), CHIP_NO_ERROR);
    EXPECT_EQ(maximumChargeCurrent, MockEvseDelegate::kMockMaximumChargeCurrent);

    DataModel::Nullable<uint32_t> sessionID;
    ASSERT_EQ(tester.ReadAttribute(SessionID::Id, sessionID), CHIP_NO_ERROR);
    ASSERT_FALSE(sessionID.IsNull());
    EXPECT_EQ(sessionID.Value(), MockEvseDelegate::kMockSessionID);

    DataModel::Nullable<uint32_t> sessionDuration;
    ASSERT_EQ(tester.ReadAttribute(SessionDuration::Id, sessionDuration), CHIP_NO_ERROR);
    ASSERT_FALSE(sessionDuration.IsNull());
    EXPECT_EQ(sessionDuration.Value(), MockEvseDelegate::kMockSessionDuration);

    DataModel::Nullable<int64_t> sessionEnergyCharged;
    ASSERT_EQ(tester.ReadAttribute(SessionEnergyCharged::Id, sessionEnergyCharged), CHIP_NO_ERROR);
    ASSERT_FALSE(sessionEnergyCharged.IsNull());
    EXPECT_EQ(sessionEnergyCharged.Value(), MockEvseDelegate::kMockSessionEnergyCharged);

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

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    // Read mandatory attributes
    StateEnum state = StateEnum::kUnknownEnumValue;
    ASSERT_EQ(tester.ReadAttribute(State::Id, state), CHIP_NO_ERROR);
    EXPECT_EQ(state, MockEvseDelegate::kMockState);

    SupplyStateEnum supplyState = SupplyStateEnum::kUnknownEnumValue;
    ASSERT_EQ(tester.ReadAttribute(SupplyState::Id, supplyState), CHIP_NO_ERROR);
    EXPECT_EQ(supplyState, MockEvseDelegate::kMockSupplyState);

    FaultStateEnum faultState = FaultStateEnum::kUnknownEnumValue;
    ASSERT_EQ(tester.ReadAttribute(FaultState::Id, faultState), CHIP_NO_ERROR);
    EXPECT_EQ(faultState, MockEvseDelegate::kMockFaultState);

    DataModel::Nullable<uint32_t> chargingEnabledUntil;
    ASSERT_EQ(tester.ReadAttribute(ChargingEnabledUntil::Id, chargingEnabledUntil), CHIP_NO_ERROR);
    ASSERT_FALSE(chargingEnabledUntil.IsNull());
    EXPECT_EQ(chargingEnabledUntil.Value(), MockEvseDelegate::kMockChargingEnabledUntil);

    int64_t circuitCapacity = 0;
    ASSERT_EQ(tester.ReadAttribute(CircuitCapacity::Id, circuitCapacity), CHIP_NO_ERROR);
    EXPECT_EQ(circuitCapacity, MockEvseDelegate::kMockCircuitCapacity);

    int64_t minimumChargeCurrent = 0;
    ASSERT_EQ(tester.ReadAttribute(MinimumChargeCurrent::Id, minimumChargeCurrent), CHIP_NO_ERROR);
    EXPECT_EQ(minimumChargeCurrent, MockEvseDelegate::kMockMinimumChargeCurrent);

    int64_t maximumChargeCurrent = 0;
    ASSERT_EQ(tester.ReadAttribute(MaximumChargeCurrent::Id, maximumChargeCurrent), CHIP_NO_ERROR);
    EXPECT_EQ(maximumChargeCurrent, MockEvseDelegate::kMockMaximumChargeCurrent);

    // V2x feature attributes
    DataModel::Nullable<uint32_t> dischargingEnabledUntil;
    ASSERT_EQ(tester.ReadAttribute(DischargingEnabledUntil::Id, dischargingEnabledUntil), CHIP_NO_ERROR);
    ASSERT_FALSE(dischargingEnabledUntil.IsNull());
    EXPECT_EQ(dischargingEnabledUntil.Value(), MockEvseDelegate::kMockDischargingEnabledUntil);

    int64_t maximumDischargeCurrent = 0;
    ASSERT_EQ(tester.ReadAttribute(MaximumDischargeCurrent::Id, maximumDischargeCurrent), CHIP_NO_ERROR);
    EXPECT_EQ(maximumDischargeCurrent, MockEvseDelegate::kMockMaximumDischargeCurrent);

    DataModel::Nullable<int64_t> sessionEnergyDischarged;
    ASSERT_EQ(tester.ReadAttribute(SessionEnergyDischarged::Id, sessionEnergyDischarged), CHIP_NO_ERROR);
    ASSERT_FALSE(sessionEnergyDischarged.IsNull());
    EXPECT_EQ(sessionEnergyDischarged.Value(), MockEvseDelegate::kMockSessionEnergyDischarged);

    // ChargingPreferences feature attributes
    DataModel::Nullable<uint32_t> nextChargeStartTime;
    ASSERT_EQ(tester.ReadAttribute(NextChargeStartTime::Id, nextChargeStartTime), CHIP_NO_ERROR);
    ASSERT_FALSE(nextChargeStartTime.IsNull());
    EXPECT_EQ(nextChargeStartTime.Value(), MockEvseDelegate::kMockNextChargeStartTime);

    DataModel::Nullable<uint32_t> nextChargeTargetTime;
    ASSERT_EQ(tester.ReadAttribute(NextChargeTargetTime::Id, nextChargeTargetTime), CHIP_NO_ERROR);
    ASSERT_FALSE(nextChargeTargetTime.IsNull());
    EXPECT_EQ(nextChargeTargetTime.Value(), MockEvseDelegate::kMockNextChargeTargetTime);

    DataModel::Nullable<int64_t> nextChargeRequiredEnergy;
    ASSERT_EQ(tester.ReadAttribute(NextChargeRequiredEnergy::Id, nextChargeRequiredEnergy), CHIP_NO_ERROR);
    ASSERT_FALSE(nextChargeRequiredEnergy.IsNull());
    EXPECT_EQ(nextChargeRequiredEnergy.Value(), MockEvseDelegate::kMockNextChargeRequiredEnergy);

    DataModel::Nullable<Percent> nextChargeTargetSoC;
    ASSERT_EQ(tester.ReadAttribute(NextChargeTargetSoC::Id, nextChargeTargetSoC), CHIP_NO_ERROR);
    ASSERT_FALSE(nextChargeTargetSoC.IsNull());
    EXPECT_EQ(nextChargeTargetSoC.Value(), MockEvseDelegate::kMockNextChargeTargetSoC);

    // SoCReporting feature attributes
    DataModel::Nullable<Percent> stateOfCharge;
    ASSERT_EQ(tester.ReadAttribute(StateOfCharge::Id, stateOfCharge), CHIP_NO_ERROR);
    ASSERT_FALSE(stateOfCharge.IsNull());
    EXPECT_EQ(stateOfCharge.Value(), MockEvseDelegate::kMockStateOfCharge);

    DataModel::Nullable<int64_t> batteryCapacity;
    ASSERT_EQ(tester.ReadAttribute(BatteryCapacity::Id, batteryCapacity), CHIP_NO_ERROR);
    ASSERT_FALSE(batteryCapacity.IsNull());
    EXPECT_EQ(batteryCapacity.Value(), MockEvseDelegate::kMockBatteryCapacity);

    // PlugAndCharge feature attributes
    DataModel::Nullable<CharSpan> vehicleID;
    ASSERT_EQ(tester.ReadAttribute(VehicleID::Id, vehicleID), CHIP_NO_ERROR);
    ASSERT_FALSE(vehicleID.IsNull());
    EXPECT_TRUE(vehicleID.Value().data_equal(CharSpan::fromCharString("MOCK-VIN-12345")));

    // Optional attributes
    int64_t userMaximumChargeCurrent = 0;
    ASSERT_EQ(tester.ReadAttribute(UserMaximumChargeCurrent::Id, userMaximumChargeCurrent), CHIP_NO_ERROR);
    EXPECT_EQ(userMaximumChargeCurrent, MockEvseDelegate::kMockUserMaximumChargeCurrent);

    uint32_t randomizationDelayWindow = 0;
    ASSERT_EQ(tester.ReadAttribute(RandomizationDelayWindow::Id, randomizationDelayWindow), CHIP_NO_ERROR);
    EXPECT_EQ(randomizationDelayWindow, MockEvseDelegate::kMockRandomizationDelayWindow);

    DataModel::Nullable<uint16_t> approximateEVEfficiency;
    ASSERT_EQ(tester.ReadAttribute(ApproximateEVEfficiency::Id, approximateEVEfficiency), CHIP_NO_ERROR);
    ASSERT_FALSE(approximateEVEfficiency.IsNull());
    EXPECT_EQ(approximateEVEfficiency.Value(), MockEvseDelegate::kMockApproximateEVEfficiency);

    // Session attributes
    DataModel::Nullable<uint32_t> sessionID;
    ASSERT_EQ(tester.ReadAttribute(SessionID::Id, sessionID), CHIP_NO_ERROR);
    ASSERT_FALSE(sessionID.IsNull());
    EXPECT_EQ(sessionID.Value(), MockEvseDelegate::kMockSessionID);

    DataModel::Nullable<uint32_t> sessionDuration;
    ASSERT_EQ(tester.ReadAttribute(SessionDuration::Id, sessionDuration), CHIP_NO_ERROR);
    ASSERT_FALSE(sessionDuration.IsNull());
    EXPECT_EQ(sessionDuration.Value(), MockEvseDelegate::kMockSessionDuration);

    DataModel::Nullable<int64_t> sessionEnergyCharged;
    ASSERT_EQ(tester.ReadAttribute(SessionEnergyCharged::Id, sessionEnergyCharged), CHIP_NO_ERROR);
    ASSERT_FALSE(sessionEnergyCharged.IsNull());
    EXPECT_EQ(sessionEnergyCharged.Value(), MockEvseDelegate::kMockSessionEnergyCharged);

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

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    Commands::Disable::Type command;
    EXPECT_TRUE(tester.Invoke(Commands::Disable::Id, command).IsSuccess());

    EXPECT_EQ(mockDelegate.GetSupplyState(), SupplyStateEnum::kDisabled);
    EXPECT_EQ(mockDelegate.GetChargingEnabledUntil().Value(), 0u);
    EXPECT_EQ(mockDelegate.GetDischargingEnabledUntil().Value(), 0u);
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

    mockDelegate.SetFaultState(FaultStateEnum::kGroundFault);
    mockDelegate.SetSupplyState(SupplyStateEnum::kDisabled);
    command.chargingEnabledUntil = 5000;
    command.minimumChargeCurrent = 6000;
    command.maximumChargeCurrent = 32000;
    EXPECT_FALSE(tester.Invoke(Commands::EnableCharging::Id, command).IsSuccess());
    EXPECT_EQ(mockDelegate.GetSupplyState(), SupplyStateEnum::kDisabled);

    // --- Failure with DiagnosticsActive ---

    mockDelegate.SetFaultState(FaultStateEnum::kNoError);
    mockDelegate.SetSupplyState(SupplyStateEnum::kDisabledDiagnostics);
    EXPECT_FALSE(tester.Invoke(Commands::EnableCharging::Id, command).IsSuccess());
    EXPECT_EQ(mockDelegate.GetSupplyState(), SupplyStateEnum::kDisabledDiagnostics);

    // --- Success from Disabled state -> ChargingEnabled ---

    mockDelegate.SetSupplyState(SupplyStateEnum::kDisabled);
    command.chargingEnabledUntil = 5000;
    command.minimumChargeCurrent = 8000;
    command.maximumChargeCurrent = 40000;
    EXPECT_TRUE(tester.Invoke(Commands::EnableCharging::Id, command).IsSuccess());
    EXPECT_EQ(mockDelegate.GetSupplyState(), SupplyStateEnum::kChargingEnabled);
    EXPECT_EQ(mockDelegate.GetChargingEnabledUntil().Value(), 5000u);
    EXPECT_EQ(mockDelegate.GetMinimumChargeCurrent(), 8000);
    EXPECT_EQ(mockDelegate.GetMaximumChargeCurrent(), 40000);

    // --- Success from DisabledError state -> ChargingEnabled ---

    mockDelegate.SetSupplyState(SupplyStateEnum::kDisabledError);
    command.chargingEnabledUntil = 6000;
    command.minimumChargeCurrent = 6000;
    command.maximumChargeCurrent = 32000;
    EXPECT_TRUE(tester.Invoke(Commands::EnableCharging::Id, command).IsSuccess());
    EXPECT_EQ(mockDelegate.GetSupplyState(), SupplyStateEnum::kChargingEnabled);
    EXPECT_EQ(mockDelegate.GetChargingEnabledUntil().Value(), 6000u);

    // --- Success from DischargingEnabled state -> Enabled (both) ---

    mockDelegate.SetSupplyState(SupplyStateEnum::kDischargingEnabled);
    command.chargingEnabledUntil = 7000;
    command.minimumChargeCurrent = 6000;
    command.maximumChargeCurrent = 24000;
    EXPECT_TRUE(tester.Invoke(Commands::EnableCharging::Id, command).IsSuccess());
    EXPECT_EQ(mockDelegate.GetSupplyState(), SupplyStateEnum::kEnabled);
    EXPECT_EQ(mockDelegate.GetChargingEnabledUntil().Value(), 7000u);

    // --- Success with null timestamp (indefinite charging) ---

    mockDelegate.SetSupplyState(SupplyStateEnum::kDisabled);
    command.chargingEnabledUntil.SetNull();
    command.minimumChargeCurrent = 6000;
    command.maximumChargeCurrent = 32000;
    EXPECT_TRUE(tester.Invoke(Commands::EnableCharging::Id, command).IsSuccess());
    EXPECT_EQ(mockDelegate.GetSupplyState(), SupplyStateEnum::kChargingEnabled);
    EXPECT_TRUE(mockDelegate.GetChargingEnabledUntil().IsNull());

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

    EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);
    Commands::EnableDischarging::Type command;

    // --- Constraint error test ---

    // maximumDischargeCurrent < 0 (below kMinimumChargeCurrentLimit)
    command.dischargingEnabledUntil = 5000;
    command.maximumDischargeCurrent = -1;
    EXPECT_FALSE(tester.Invoke(Commands::EnableDischarging::Id, command).IsSuccess());

    // --- Failure with FaultState ---

    mockDelegate.SetFaultState(FaultStateEnum::kGroundFault);
    mockDelegate.SetSupplyState(SupplyStateEnum::kDisabled);
    command.dischargingEnabledUntil = 5000;
    command.maximumDischargeCurrent = 16000;
    EXPECT_FALSE(tester.Invoke(Commands::EnableDischarging::Id, command).IsSuccess());
    EXPECT_EQ(mockDelegate.GetSupplyState(), SupplyStateEnum::kDisabled);

    // --- Failure with DiagnosticsActive ---

    mockDelegate.SetFaultState(FaultStateEnum::kNoError);
    mockDelegate.SetSupplyState(SupplyStateEnum::kDisabledDiagnostics);
    EXPECT_FALSE(tester.Invoke(Commands::EnableDischarging::Id, command).IsSuccess());
    EXPECT_EQ(mockDelegate.GetSupplyState(), SupplyStateEnum::kDisabledDiagnostics);

    // --- Success from Disabled state -> DischargingEnabled ---

    mockDelegate.SetSupplyState(SupplyStateEnum::kDisabled);
    command.dischargingEnabledUntil = 5000;
    command.maximumDischargeCurrent = 16000;
    EXPECT_TRUE(tester.Invoke(Commands::EnableDischarging::Id, command).IsSuccess());
    EXPECT_EQ(mockDelegate.GetSupplyState(), SupplyStateEnum::kDischargingEnabled);
    EXPECT_EQ(mockDelegate.GetDischargingEnabledUntil().Value(), 5000u);
    EXPECT_EQ(mockDelegate.GetMaximumDischargeCurrent(), 16000);

    // --- Success from DisabledError state -> DischargingEnabled ---

    mockDelegate.SetSupplyState(SupplyStateEnum::kDisabledError);
    command.dischargingEnabledUntil = 6000;
    command.maximumDischargeCurrent = 12000;
    EXPECT_TRUE(tester.Invoke(Commands::EnableDischarging::Id, command).IsSuccess());
    EXPECT_EQ(mockDelegate.GetSupplyState(), SupplyStateEnum::kDischargingEnabled);
    EXPECT_EQ(mockDelegate.GetDischargingEnabledUntil().Value(), 6000u);

    // --- Success from ChargingEnabled state -> Enabled (both) ---

    mockDelegate.SetSupplyState(SupplyStateEnum::kChargingEnabled);
    command.dischargingEnabledUntil = 7000;
    command.maximumDischargeCurrent = 10000;
    EXPECT_TRUE(tester.Invoke(Commands::EnableDischarging::Id, command).IsSuccess());
    EXPECT_EQ(mockDelegate.GetSupplyState(), SupplyStateEnum::kEnabled);
    EXPECT_EQ(mockDelegate.GetDischargingEnabledUntil().Value(), 7000u);

    // --- Success with null timestamp (indefinite discharging) ---

    mockDelegate.SetSupplyState(SupplyStateEnum::kDisabled);
    command.dischargingEnabledUntil.SetNull();
    command.maximumDischargeCurrent = 16000;
    EXPECT_TRUE(tester.Invoke(Commands::EnableDischarging::Id, command).IsSuccess());
    EXPECT_EQ(mockDelegate.GetSupplyState(), SupplyStateEnum::kDischargingEnabled);
    EXPECT_TRUE(mockDelegate.GetDischargingEnabledUntil().IsNull());

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

        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        ClusterTester tester(cluster);
        Commands::StartDiagnostics::Type command;

        mockDelegate.SetSupplyState(SupplyStateEnum::kDisabled);
        EXPECT_TRUE(tester.Invoke(Commands::StartDiagnostics::Id, command).IsSuccess());
        EXPECT_EQ(mockDelegate.GetSupplyState(), SupplyStateEnum::kDisabledDiagnostics);

        // --- Failure with DiagnosticsActive ---

        mockDelegate.SetSupplyState(SupplyStateEnum::kDisabledDiagnostics);
        EXPECT_FALSE(tester.Invoke(Commands::StartDiagnostics::Id, command).IsSuccess());

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
    // Test with command not supported
    {
        BitMask<OptionalCommands> optionalCommands;
        EnergyEvseCluster cluster(
            EnergyEvseCluster::Config(kTestEndpointId, mockDelegate, noFeatures, optionalAttributes, optionalCommands));
        EXPECT_EQ(cluster.Startup(context.Get()), CHIP_NO_ERROR);

        ClusterTester tester(cluster);
        Commands::StartDiagnostics::Type command;

        mockDelegate.SetSupplyState(SupplyStateEnum::kDisabled);
        EXPECT_FALSE(tester.Invoke(Commands::StartDiagnostics::Id, command).IsSuccess());
        EXPECT_EQ(mockDelegate.GetSupplyState(), SupplyStateEnum::kDisabled);

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
