/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "ThermostatTestCommon.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Commands.h>
#include <lib/support/TimeUtils.h>
#include <system/RAIIMockClock.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace chip::app::Clusters::Thermostat::Commands;
using namespace chip::Protocols::InteractionModel;
using namespace chip::System::Clock;
using namespace chip::Testing;

namespace {

// An arbitrary post-2000 Unix timestamp, so it maps to a valid (positive) Matter epoch time.
constexpr uint64_t kMockUnixTimeMicroseconds = 1700000000ULL * 1000000ULL;

void EnableAllSetpointChangeAttributes(OptionalAttributes & optionalAttributes)
{
    optionalAttributes.SetpointChangeSource          = true;
    optionalAttributes.SetpointChangeAmount          = true;
    optionalAttributes.SetpointChangeSourceTimestamp = true;
}

TEST_F(ThermostatTestFixture, TestAttributesNotAdvertisedWhenDisabled)
{
    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling);
    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_FALSE(HasAttribute(cluster, SetpointChangeSource::Id));
    EXPECT_FALSE(HasAttribute(cluster, SetpointChangeAmount::Id));
    EXPECT_FALSE(HasAttribute(cluster, SetpointChangeSourceTimestamp::Id));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestDefaultValuesBeforeAnySetpointChange)
{
    EnableAllSetpointChangeAttributes(mOptionalAttributes);

    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling);
    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(HasAttribute(cluster, SetpointChangeSource::Id));
    EXPECT_TRUE(HasAttribute(cluster, SetpointChangeAmount::Id));
    EXPECT_TRUE(HasAttribute(cluster, SetpointChangeSourceTimestamp::Id));

    SetpointChangeSourceEnum source = SetpointChangeSourceEnum::kExternal;
    EXPECT_EQ(tester.ReadAttribute(SetpointChangeSource::Id, source), Status::Success);
    EXPECT_EQ(source, SetpointChangeSourceEnum::kManual);

    DataModel::Nullable<int16_t> amount(static_cast<int16_t>(42));
    EXPECT_EQ(tester.ReadAttribute(SetpointChangeAmount::Id, amount), Status::Success);
    EXPECT_TRUE(amount.IsNull());

    uint32_t timestamp = 12345;
    EXPECT_EQ(tester.ReadAttribute(SetpointChangeSourceTimestamp::Id, timestamp), Status::Success);
    EXPECT_EQ(timestamp, 0u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestOperationalSetpointWriteUpdatesTrackingAttributes)
{
    EnableAllSetpointChangeAttributes(mOptionalAttributes);

    // Mock the clock to a known synced real time. Some platforms (e.g. Zephyr's native_sim) start with an
    // unsynced clock by default, which would otherwise make GetClock_MatterEpochS() fail and this test flaky
    // across platforms; see TestUnsyncedClockLeavesTimestampStaleButStillUpdatesSourceAndAmount for that case.
    System::Clock::Internal::RAIIMockClock mockClock;
    ASSERT_EQ(mockClock.SetClock_RealTime(Microseconds64(kMockUnixTimeMicroseconds)), CHIP_NO_ERROR);

    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling);
    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // mHeatingDelegate.mOccupiedHeatingSetpoint starts at 2000.
    EXPECT_EQ(tester.WriteAttribute(OccupiedHeatingSetpoint::Id, static_cast<temperature>(2100)), Status::Success);

    EXPECT_TRUE(tester.IsAttributeDirty(SetpointChangeSource::Id));
    EXPECT_TRUE(tester.IsAttributeDirty(SetpointChangeAmount::Id));
    EXPECT_TRUE(tester.IsAttributeDirty(SetpointChangeSourceTimestamp::Id));

    SetpointChangeSourceEnum source = SetpointChangeSourceEnum::kExternal;
    EXPECT_EQ(tester.ReadAttribute(SetpointChangeSource::Id, source), Status::Success);
    EXPECT_EQ(source, SetpointChangeSourceEnum::kManual);

    DataModel::Nullable<int16_t> amount;
    EXPECT_EQ(tester.ReadAttribute(SetpointChangeAmount::Id, amount), Status::Success);
    ASSERT_FALSE(amount.IsNull());
    EXPECT_EQ(amount.Value(), 100);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestSetpointChangeSourceReflectsDelegate)
{
    EnableAllSetpointChangeAttributes(mOptionalAttributes);
    mThermostatDelegate.mSetpointChangeSource = SetpointChangeSourceEnum::kSchedule;

    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling);
    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_EQ(tester.WriteAttribute(OccupiedCoolingSetpoint::Id, static_cast<temperature>(2700)), Status::Success);

    SetpointChangeSourceEnum source = SetpointChangeSourceEnum::kManual;
    EXPECT_EQ(tester.ReadAttribute(SetpointChangeSource::Id, source), Status::Success);
    EXPECT_EQ(source, SetpointChangeSourceEnum::kSchedule);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestSetpointChangeAmountIsClampedToInt16Range)
{
    EnableAllSetpointChangeAttributes(mOptionalAttributes);
    // Widen the heating limits so that a full int16_t swing is accepted by the setpoint write validation;
    // only the SetpointChangeAmount clamping behavior is under test here.
    mOptionalAttributes.AbsMinHeatSetpointLimit = true;
    mOptionalAttributes.AbsMaxHeatSetpointLimit = true;
    mOptionalAttributes.MinHeatSetpointLimit    = true;
    mOptionalAttributes.MaxHeatSetpointLimit    = true;
    mHeatingDelegate.mAbsMinHeat                = INT16_MIN;
    mHeatingDelegate.mAbsMaxHeat                = INT16_MAX;
    mHeatingDelegate.mMinHeat                   = INT16_MIN;
    mHeatingDelegate.mMaxHeat                   = INT16_MAX;
    mHeatingDelegate.mOccupiedHeatingSetpoint   = INT16_MIN;

    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling);
    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // A swing from INT16_MIN to INT16_MAX overflows int16_t; the reported amount must clamp to INT16_MAX.
    EXPECT_EQ(tester.WriteAttribute(OccupiedHeatingSetpoint::Id, static_cast<temperature>(INT16_MAX)), Status::Success);

    DataModel::Nullable<int16_t> amount;
    EXPECT_EQ(tester.ReadAttribute(SetpointChangeAmount::Id, amount), Status::Success);
    ASSERT_FALSE(amount.IsNull());
    EXPECT_EQ(amount.Value(), INT16_MAX);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestNonOperationalSetpointWriteDoesNotUpdateTrackingAttributes)
{
    EnableAllSetpointChangeAttributes(mOptionalAttributes);
    mOptionalAttributes.MinHeatSetpointLimit = true;

    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling);
    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Writing a setpoint *limit* is not a setpoint value change and must not affect the tracking attributes.
    EXPECT_EQ(tester.WriteAttribute(MinHeatSetpointLimit::Id, static_cast<temperature>(900)), Status::Success);

    EXPECT_FALSE(tester.IsAttributeDirty(SetpointChangeSource::Id));
    EXPECT_FALSE(tester.IsAttributeDirty(SetpointChangeAmount::Id));
    EXPECT_FALSE(tester.IsAttributeDirty(SetpointChangeSourceTimestamp::Id));

    DataModel::Nullable<int16_t> amount(static_cast<int16_t>(7));
    EXPECT_EQ(tester.ReadAttribute(SetpointChangeAmount::Id, amount), Status::Success);
    EXPECT_TRUE(amount.IsNull());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestIndependentOptionalAttributeGating)
{
    // Only SetpointChangeAmount is enabled; Source and Timestamp must stay untouched.
    mOptionalAttributes.SetpointChangeAmount = true;

    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling);
    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_FALSE(HasAttribute(cluster, SetpointChangeSource::Id));
    EXPECT_TRUE(HasAttribute(cluster, SetpointChangeAmount::Id));
    EXPECT_FALSE(HasAttribute(cluster, SetpointChangeSourceTimestamp::Id));

    EXPECT_EQ(tester.WriteAttribute(OccupiedHeatingSetpoint::Id, static_cast<temperature>(2050)), Status::Success);

    EXPECT_TRUE(tester.IsAttributeDirty(SetpointChangeAmount::Id));
    EXPECT_FALSE(tester.IsAttributeDirty(SetpointChangeSource::Id));
    EXPECT_FALSE(tester.IsAttributeDirty(SetpointChangeSourceTimestamp::Id));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestDirectWriteCascadeReportsInitiatingSetpointDelta)
{
    EnableAllSetpointChangeAttributes(mOptionalAttributes);

    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling, Feature::kAutoMode);
    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate,
                              mAutoDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Default OccupiedHeatingSetpoint (2000) and OccupiedCoolingSetpoint (2600) start further apart than the 2.0C
    // deadband. Writing OccupiedCoolingSetpoint down to 2050 leaves only 0.5C between them, so Setpoints::Fix()
    // also pulls OccupiedHeatingSetpoint down to 1850 to restore the deadband. Both attributes end up in
    // changedAttributes, but only OccupiedCoolingSetpoint was directly written; its delta - not the cascaded
    // heating delta - must be the one reported.
    EXPECT_EQ(tester.WriteAttribute(OccupiedCoolingSetpoint::Id, static_cast<temperature>(2050)), Status::Success);
    EXPECT_EQ(mHeatingDelegate.mOccupiedHeatingSetpoint, 1850);

    DataModel::Nullable<int16_t> amount;
    EXPECT_EQ(tester.ReadAttribute(SetpointChangeAmount::Id, amount), Status::Success);
    ASSERT_FALSE(amount.IsNull());
    EXPECT_EQ(amount.Value(), -550);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestSetpointRaiseLowerCommandReportsFirstChangedSetpointDelta)
{
    EnableAllSetpointChangeAttributes(mOptionalAttributes);

    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling, Feature::kAutoMode);
    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate,
                              mAutoDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    Commands::SetpointRaiseLower::Type request;
    request.mode   = SetpointRaiseLowerModeEnum::kBoth;
    request.amount = 5; // +0.5 degC, applied to both heating and cooling setpoints.

    auto result = tester.Invoke(request);
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(mHeatingDelegate.mOccupiedHeatingSetpoint, 2050);
    EXPECT_EQ(mCoolingDelegate.mOccupiedCoolingSetpoint, 2650);

    DataModel::Nullable<int16_t> amount;
    EXPECT_EQ(tester.ReadAttribute(SetpointChangeAmount::Id, amount), Status::Success);
    ASSERT_FALSE(amount.IsNull());
    // Both the heating and cooling setpoints moved by +50 in the same command. Summing signed deltas across
    // setpoints could cancel out or double-count, so only the first changed setpoint's delta (heating) is
    // reported, not the sum.
    EXPECT_EQ(amount.Value(), 50);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestNoOpWriteDoesNotUpdateTrackingAttributes)
{
    EnableAllSetpointChangeAttributes(mOptionalAttributes);

    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling);
    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Writing the setpoint's current value (2000) is a no-op: the delegate reports changed=false, the write
    // still succeeds, but nothing actually moved and tracking must not update.
    EXPECT_EQ(tester.WriteAttribute(OccupiedHeatingSetpoint::Id, static_cast<temperature>(2000)), Status::Success);

    EXPECT_FALSE(tester.IsAttributeDirty(SetpointChangeSource::Id));
    EXPECT_FALSE(tester.IsAttributeDirty(SetpointChangeAmount::Id));
    EXPECT_FALSE(tester.IsAttributeDirty(SetpointChangeSourceTimestamp::Id));

    DataModel::Nullable<int16_t> amount(static_cast<int16_t>(7));
    EXPECT_EQ(tester.ReadAttribute(SetpointChangeAmount::Id, amount), Status::Success);
    EXPECT_TRUE(amount.IsNull());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestLimitWriteCascadeDoesNotUpdateTrackingAttributes)
{
    EnableAllSetpointChangeAttributes(mOptionalAttributes);
    mOptionalAttributes.MinHeatSetpointLimit = true;

    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling);
    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Raising MinHeatSetpointLimit above the current OccupiedHeatingSetpoint (2000) forces Setpoints::Fix() to
    // clamp it up to the new minimum as a side effect. The setpoint value genuinely changes, but this was not a
    // direct write to (or SetpointRaiseLower targeting) an operational setpoint, so it must not be reported.
    EXPECT_EQ(tester.WriteAttribute(MinHeatSetpointLimit::Id, static_cast<temperature>(2500)), Status::Success);
    EXPECT_EQ(mHeatingDelegate.mOccupiedHeatingSetpoint, 2500);

    EXPECT_FALSE(tester.IsAttributeDirty(SetpointChangeSource::Id));
    EXPECT_FALSE(tester.IsAttributeDirty(SetpointChangeAmount::Id));
    EXPECT_FALSE(tester.IsAttributeDirty(SetpointChangeSourceTimestamp::Id));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestSetpointChangeSourceTimestampMatchesCurrentTime)
{
    EnableAllSetpointChangeAttributes(mOptionalAttributes);

    System::Clock::Internal::RAIIMockClock mockClock;
    ASSERT_EQ(mockClock.SetClock_RealTime(Microseconds64(kMockUnixTimeMicroseconds)), CHIP_NO_ERROR);

    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling);
    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_EQ(tester.WriteAttribute(OccupiedHeatingSetpoint::Id, static_cast<temperature>(2100)), Status::Success);

    uint32_t timestamp = 0;
    EXPECT_EQ(tester.ReadAttribute(SetpointChangeSourceTimestamp::Id, timestamp), Status::Success);
    EXPECT_EQ(timestamp,
              static_cast<uint32_t>((kMockUnixTimeMicroseconds - chip::kChipEpochUsSinceUnixEpoch) / chip::kMicrosecondsPerSecond));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestUnsyncedClockLeavesTimestampStaleButStillUpdatesSourceAndAmount)
{
    EnableAllSetpointChangeAttributes(mOptionalAttributes);

    // A mock clock that is never given a real time simulates a device that hasn't completed time sync yet
    // (GetClock_MatterEpochS() fails in that state).
    System::Clock::Internal::RAIIMockClock mockClock;

    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling);
    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_EQ(tester.WriteAttribute(OccupiedHeatingSetpoint::Id, static_cast<temperature>(2100)), Status::Success);

    // Source and Amount are independent of the clock and still update...
    EXPECT_TRUE(tester.IsAttributeDirty(SetpointChangeSource::Id));
    EXPECT_TRUE(tester.IsAttributeDirty(SetpointChangeAmount::Id));
    // ...but the timestamp is left at its default/stale value and is not reported as changed.
    EXPECT_FALSE(tester.IsAttributeDirty(SetpointChangeSourceTimestamp::Id));

    uint32_t timestamp = 1;
    EXPECT_EQ(tester.ReadAttribute(SetpointChangeSourceTimestamp::Id, timestamp), Status::Success);
    EXPECT_EQ(timestamp, 0u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

} // namespace
