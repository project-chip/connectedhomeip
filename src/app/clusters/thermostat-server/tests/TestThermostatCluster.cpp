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
#include <clusters/Thermostat/Metadata.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace chip::app::Clusters::Thermostat::Commands;
using namespace chip::Protocols::InteractionModel;
using namespace chip::Testing;

namespace {

TEST_F(ThermostatTestFixture, TestClusterRevisionAndFeatureMap)
{
    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling, Feature::kAutoMode);
    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate,
                              mAutoDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    uint16_t clusterRevision = 0;
    EXPECT_EQ(tester.ReadAttribute(ClusterRevision::Id, clusterRevision), Status::Success);
    EXPECT_EQ(clusterRevision, Thermostat::kRevision);

    BitFlags<Feature> readFeatures;
    EXPECT_EQ(tester.ReadAttribute(FeatureMap::Id, readFeatures), Status::Success);
    EXPECT_EQ(readFeatures, features);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestMandatoryAttributesInAttributeList)
{
    BitFlags<Feature> features(Feature::kHeating);
    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(HasAttribute(cluster, ClusterRevision::Id));
    EXPECT_TRUE(HasAttribute(cluster, FeatureMap::Id));
    EXPECT_TRUE(HasAttribute(cluster, LocalTemperature::Id));
    EXPECT_TRUE(HasAttribute(cluster, ControlSequenceOfOperation::Id));
    EXPECT_TRUE(HasAttribute(cluster, SystemMode::Id));
    EXPECT_TRUE(HasAttribute(cluster, OccupiedHeatingSetpoint::Id));

    // Not configured
    EXPECT_FALSE(HasAttribute(cluster, OccupiedCoolingSetpoint::Id));
    EXPECT_FALSE(HasAttribute(cluster, OutdoorTemperature::Id));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestOptionalAttributesInAttributeList)
{
    mOptionalAttributes.LocalTemperatureCalibration = true;
    mOptionalAttributes.OutdoorTemperature          = true;
    mOptionalAttributes.RemoteSensing               = true;
    mOptionalAttributes.ThermostatRunningMode       = true;
    mOptionalAttributes.ThermostatRunningState      = true;

    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling);
    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(HasAttribute(cluster, LocalTemperatureCalibration::Id));
    EXPECT_TRUE(HasAttribute(cluster, OutdoorTemperature::Id));
    EXPECT_TRUE(HasAttribute(cluster, RemoteSensing::Id));
    EXPECT_TRUE(HasAttribute(cluster, ThermostatRunningMode::Id));
    EXPECT_TRUE(HasAttribute(cluster, ThermostatRunningState::Id));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestLocalTemperatureNormalAndEvents)
{
    BitFlags<Feature> features(Feature::kHeating, Feature::kEvents);
    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    mThermostatDelegate.mLocalTemperature = 2150;

    DataModel::Nullable<temperature> localTemp;
    EXPECT_EQ(tester.ReadAttribute(LocalTemperature::Id, localTemp), Status::Success);
    ASSERT_FALSE(localTemp.IsNull());
    EXPECT_EQ(localTemp.Value(), 2150);

    // Test SetLocalTemperature via cluster
    EXPECT_EQ(cluster.SetLocalTemperature(DataModel::MakeNullable<temperature>(2200)), Status::Success);
    EXPECT_EQ(mThermostatDelegate.mLocalTemperature.Value(), 2200);
    EXPECT_TRUE(tester.IsAttributeDirty(LocalTemperature::Id));

    // Verify event was generated
    auto event = tester.GetNextGeneratedEvent();
    ASSERT_TRUE(event.has_value());
    EXPECT_EQ(event->eventOptions.mPath.mClusterId, Thermostat::Id);
    EXPECT_EQ(event->eventOptions.mPath.mEventId, Events::LocalTemperatureChange::Id);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestLocalTemperatureNotExposedFeature)
{
    BitFlags<Feature> features(Feature::kHeating, Feature::kLocalTemperatureNotExposed);
    mOptionalAttributes.RemoteSensing = true;
    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    mThermostatDelegate.mLocalTemperature = 2100;

    // Per Matter spec, if LTNE feature is supported, LocalTemperature attribute SHALL always report null.
    DataModel::Nullable<temperature> localTemp;
    EXPECT_EQ(tester.ReadAttribute(LocalTemperature::Id, localTemp), Status::Success);
    EXPECT_TRUE(localTemp.IsNull());

    // Reading RemoteSensing clears kLocalTemperature bit when LTNE is supported
    mThermostatDelegate.mRemoteSensing.Set(RemoteSensingBitmap::kLocalTemperature);
    BitMask<RemoteSensingBitmap> remoteSensing;
    EXPECT_EQ(tester.ReadAttribute(RemoteSensing::Id, remoteSensing), Status::Success);
    EXPECT_FALSE(remoteSensing.Has(RemoteSensingBitmap::kLocalTemperature));

    // Writing RemoteSensing with kLocalTemperature bit when LTNE is supported returns ConstraintError
    BitMask<RemoteSensingBitmap> writeSensing;
    writeSensing.Set(RemoteSensingBitmap::kLocalTemperature);
    EXPECT_EQ(tester.WriteAttribute(RemoteSensing::Id, writeSensing), Status::ConstraintError);

    // Writing RemoteSensing without kLocalTemperature succeeds
    writeSensing.Clear(RemoteSensingBitmap::kLocalTemperature);
    writeSensing.Set(RemoteSensingBitmap::kOutdoorTemperature);
    EXPECT_EQ(tester.WriteAttribute(RemoteSensing::Id, writeSensing), Status::Success);
    EXPECT_TRUE(mThermostatDelegate.mRemoteSensing.Has(RemoteSensingBitmap::kOutdoorTemperature));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestSystemModeValidationAndEvents)
{
    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling, Feature::kEvents);
    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Read default system mode
    SystemModeEnum mode;
    EXPECT_EQ(tester.ReadAttribute(SystemMode::Id, mode), Status::Success);
    EXPECT_EQ(mode, SystemModeEnum::kOff);

    // Write Heat - supported because kHeating feature is set
    EXPECT_EQ(tester.WriteAttribute(SystemMode::Id, SystemModeEnum::kHeat), Status::Success);
    EXPECT_EQ(mThermostatDelegate.mSystemMode, SystemModeEnum::kHeat);
    EXPECT_TRUE(tester.IsAttributeDirty(SystemMode::Id));

    // Verify SystemModeChange event
    auto event = tester.GetNextGeneratedEvent();
    ASSERT_TRUE(event.has_value());
    EXPECT_EQ(event->eventOptions.mPath.mEventId, Events::SystemModeChange::Id);

    // Write Cool - supported because kCooling feature is set
    EXPECT_EQ(tester.WriteAttribute(SystemMode::Id, SystemModeEnum::kCool), Status::Success);
    EXPECT_EQ(mThermostatDelegate.mSystemMode, SystemModeEnum::kCool);

    // Write Auto - not supported because kAutoMode feature is NOT set -> ConstraintError
    EXPECT_EQ(tester.WriteAttribute(SystemMode::Id, SystemModeEnum::kAuto), Status::ConstraintError);

    // Write EmergencyHeat - supported because kHeating is set
    EXPECT_EQ(tester.WriteAttribute(SystemMode::Id, SystemModeEnum::kEmergencyHeat), Status::Success);

    // Write Precooling - supported because kCooling is set
    EXPECT_EQ(tester.WriteAttribute(SystemMode::Id, SystemModeEnum::kPrecooling), Status::Success);

    // Write FanOnly, Dry, Sleep - always allowed
    EXPECT_EQ(tester.WriteAttribute(SystemMode::Id, SystemModeEnum::kFanOnly), Status::Success);
    EXPECT_EQ(tester.WriteAttribute(SystemMode::Id, SystemModeEnum::kDry), Status::Success);
    EXPECT_EQ(tester.WriteAttribute(SystemMode::Id, SystemModeEnum::kSleep), Status::Success);

    // Invalid enum value
    EXPECT_EQ(tester.WriteAttribute(SystemMode::Id, static_cast<SystemModeEnum>(0xFF)), Status::InvalidValue);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestSystemModeConstraintsWithoutFeatures)
{
    // Heating only cluster: kCooling is NOT supported
    BitFlags<Feature> features(Feature::kHeating);
    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_EQ(tester.WriteAttribute(SystemMode::Id, SystemModeEnum::kCool), Status::ConstraintError);
    EXPECT_EQ(tester.WriteAttribute(SystemMode::Id, SystemModeEnum::kPrecooling), Status::ConstraintError);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestOutdoorTemperatureAttribute)
{
    mOptionalAttributes.OutdoorTemperature = true;
    BitFlags<Feature> features(Feature::kHeating);
    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    mThermostatDelegate.mOutdoorTemperature = 1850;
    DataModel::Nullable<temperature> outdoorTemp;
    EXPECT_EQ(tester.ReadAttribute(OutdoorTemperature::Id, outdoorTemp), Status::Success);
    ASSERT_FALSE(outdoorTemp.IsNull());
    EXPECT_EQ(outdoorTemp.Value(), 1850);

    // When delegate returns error
    mThermostatDelegate.mGetOutdoorTemperatureStatus = Status::Failure;
    EXPECT_EQ(tester.ReadAttribute(OutdoorTemperature::Id, outdoorTemp), Status::Failure);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestLocalTemperatureCalibration)
{
    mOptionalAttributes.LocalTemperatureCalibration = true;
    BitFlags<Feature> features(Feature::kHeating);
    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Read initial calibration
    int8_t cal = 0;
    EXPECT_EQ(tester.ReadAttribute(LocalTemperatureCalibration::Id, cal), Status::Success);
    EXPECT_EQ(cal, 0);

    // Valid writes in range [-128, 127]
    EXPECT_EQ(tester.WriteAttribute(LocalTemperatureCalibration::Id, static_cast<int8_t>(25)), Status::Success);
    EXPECT_EQ(mThermostatDelegate.mCalibration, 25);
    EXPECT_TRUE(tester.IsAttributeDirty(LocalTemperatureCalibration::Id));

    EXPECT_EQ(tester.WriteAttribute(LocalTemperatureCalibration::Id, static_cast<int8_t>(-10)), Status::Success);
    EXPECT_EQ(mThermostatDelegate.mCalibration, -10);

    // Out of range write (int16_t out of int8_t range) -> ConstraintError
    EXPECT_EQ(tester.WriteAttribute(LocalTemperatureCalibration::Id, static_cast<int16_t>(200)), Status::ConstraintError);
    EXPECT_EQ(tester.WriteAttribute(LocalTemperatureCalibration::Id, static_cast<int16_t>(-200)), Status::ConstraintError);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestControlSequenceOfOperation)
{
    BitFlags<Feature> features(Feature::kHeating);
    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    ControlSequenceOfOperationEnum cso;
    EXPECT_EQ(tester.ReadAttribute(ControlSequenceOfOperation::Id, cso), Status::Success);
    EXPECT_EQ(cso, ControlSequenceOfOperationEnum::kCoolingAndHeating);

    // Per spec, writes to ControlSequenceOfOperation are ignored, but success is returned for backwards compatibility
    EXPECT_EQ(tester.WriteAttribute(ControlSequenceOfOperation::Id, ControlSequenceOfOperationEnum::kHeatingOnly), Status::Success);

    // Calling SetControlSequenceOfOperation directly on cluster notifies dirty
    EXPECT_EQ(cluster.SetControlSequenceOfOperation(ControlSequenceOfOperationEnum::kHeatingOnly), Status::Success);
    EXPECT_TRUE(tester.IsAttributeDirty(ControlSequenceOfOperation::Id));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestThermostatRunningModeAndRunningState)
{
    mOptionalAttributes.ThermostatRunningMode  = true;
    mOptionalAttributes.ThermostatRunningState = true;
    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling, Feature::kEvents);
    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Read RunningMode
    ThermostatRunningModeEnum rmode;
    EXPECT_EQ(tester.ReadAttribute(ThermostatRunningMode::Id, rmode), Status::Success);
    EXPECT_EQ(rmode, ThermostatRunningModeEnum::kOff);

    // Set RunningMode on cluster
    EXPECT_EQ(cluster.SetRunningMode(ThermostatRunningModeEnum::kHeat), Status::Success);
    EXPECT_EQ(mThermostatDelegate.mRunningMode, ThermostatRunningModeEnum::kHeat);
    EXPECT_TRUE(tester.IsAttributeDirty(ThermostatRunningMode::Id));

    auto event = tester.GetNextGeneratedEvent();
    ASSERT_TRUE(event.has_value());
    EXPECT_EQ(event->eventOptions.mPath.mEventId, Events::RunningModeChange::Id);

    EXPECT_EQ(cluster.SetRunningMode(ThermostatRunningModeEnum::kCool), Status::Success);
    EXPECT_EQ(mThermostatDelegate.mRunningMode, ThermostatRunningModeEnum::kCool);
    event = tester.GetNextGeneratedEvent();
    ASSERT_TRUE(event.has_value());
    EXPECT_EQ(event->eventOptions.mPath.mEventId, Events::RunningModeChange::Id);

    // Invalid enum
    EXPECT_EQ(cluster.SetRunningMode(static_cast<ThermostatRunningModeEnum>(0xFF)), Status::ConstraintError);

    // Read RunningState
    BitMask<RelayStateBitmap> rstate;
    EXPECT_EQ(tester.ReadAttribute(ThermostatRunningState::Id, rstate), Status::Success);
    EXPECT_EQ(rstate.Raw(), 0u);

    // Set RunningState on cluster
    BitMask<RelayStateBitmap> newState;
    newState.Set(RelayStateBitmap::kHeat);
    EXPECT_EQ(cluster.SetRunningState(newState), Status::Success);
    EXPECT_TRUE(tester.IsAttributeDirty(ThermostatRunningState::Id));

    auto stateEvent = tester.GetNextGeneratedEvent();
    ASSERT_TRUE(stateEvent.has_value());
    EXPECT_EQ(stateEvent->eventOptions.mPath.mEventId, Events::RunningStateChange::Id);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestRunningStateConstraintsWithoutFeatures)
{
    // Heating only cluster: cooling relay state should be rejected
    BitFlags<Feature> features(Feature::kHeating);
    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    BitMask<RelayStateBitmap> coolState;
    coolState.Set(RelayStateBitmap::kCool);
    EXPECT_EQ(cluster.SetRunningState(coolState), Status::ConstraintError);

    EXPECT_EQ(cluster.SetRunningMode(ThermostatRunningModeEnum::kCool), Status::ConstraintError);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestMinSetpointDeadBandBackwardsCompatibilityWrite)
{
    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling, Feature::kAutoMode);
    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate,
                              mAutoDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Read deadband (mDeadband is 200 in 0.01 C -> 20 in 0.1 C)
    int8_t deadband = 0;
    EXPECT_EQ(tester.ReadAttribute(MinSetpointDeadBand::Id, deadband), Status::Success);
    EXPECT_EQ(deadband, 20);

    // Write within valid range [0, 127] succeeds (ignored per spec for backwards compat)
    EXPECT_EQ(tester.WriteAttribute(MinSetpointDeadBand::Id, static_cast<int16_t>(15)), Status::Success);

    // Write outside range [0, 127] returns ConstraintError
    EXPECT_EQ(tester.WriteAttribute(MinSetpointDeadBand::Id, static_cast<int16_t>(-5)), Status::ConstraintError);
    EXPECT_EQ(tester.WriteAttribute(MinSetpointDeadBand::Id, static_cast<int16_t>(150)), Status::ConstraintError);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestDefaultDelegateImplementations)
{
    // Test base Thermostat::Delegate default method implementations
    class MinimalDelegate : public Thermostat::Delegate
    {
    public:
        FabricTable & GetFabricTable() const override { return *mFabricTable; }
        DataModel::Nullable<temperature> GetLocalTemperature() const override { return 2000; }
        Status SetLocalTemperature(DataModel::Nullable<temperature> t, bool & c) override { return Status::Success; }
        SystemModeEnum GetSystemMode() const override { return SystemModeEnum::kOff; }
        Status SetSystemMode(SystemModeEnum s, bool & c) override { return Status::Success; }
        ControlSequenceOfOperationEnum GetControlSequenceOfOperation() const override
        {
            return ControlSequenceOfOperationEnum::kCoolingAndHeating;
        }
        Status SetControlSequenceOfOperation(ControlSequenceOfOperationEnum s, bool & c) override { return Status::Success; }
        Status GetRunningMode(ThermostatRunningModeEnum & m) const override { return Status::Success; }
        Status SetRunningMode(ThermostatRunningModeEnum m, bool & c) override { return Status::Success; }
        Status GetRunningState(BitMask<RelayStateBitmap> & s) const override { return Status::Success; }
        Status SetRunningState(BitMask<RelayStateBitmap> s, bool & c) override { return Status::Success; }
        Status SetRemoteSensing(BitMask<RemoteSensingBitmap> r, bool & c) override { return Status::Success; }

        FabricTable * mFabricTable = nullptr;
    };

    MinimalDelegate delegate;
    delegate.mFabricTable = &mFabricHelper.GetFabricTable();

    EXPECT_EQ(delegate.Startup(mTestContext.Get()), CHIP_NO_ERROR);
    delegate.Shutdown(ClusterShutdownType::kClusterShutdown);

    DataModel::Nullable<temperature> outdoor;
    EXPECT_EQ(delegate.GetOutdoorTemperature(outdoor), Status::UnsupportedAttribute);

    EXPECT_EQ(delegate.GetLocalTemperatureCalibration(), 0);
    bool changed = false;
    EXPECT_EQ(delegate.SetLocalTemperatureCalibration(5, changed), Status::Success);
    EXPECT_FALSE(changed);

    BitMask<RemoteSensingBitmap> remote;
    EXPECT_EQ(delegate.GetRemoteSensing(remote), Status::UnsupportedAttribute);
}

} // namespace
