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
#include <app/clusters/thermostat-server/Setpoint.h>
#include <app/clusters/thermostat-server/SetpointAttributes.h>
#include <app/clusters/thermostat-server/SetpointLimits.h>
#include <app/clusters/thermostat-server/SetpointRange.h>
#include <app/clusters/thermostat-server/Setpoints.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace chip::app::Clusters::Thermostat::Commands;
using namespace chip::Protocols::InteractionModel;
using namespace chip::Testing;

namespace {

TEST(TestSetpointPrimitives, TestAbsoluteSetpoint)
{
    AbsoluteSetpoint heat(OccupiedHeatingSetpoint::Id, 2000);
    EXPECT_TRUE(heat.HasTemperature());
    EXPECT_EQ(heat.Temperature(), 2000);
    EXPECT_EQ(heat.AttributeId(), OccupiedHeatingSetpoint::Id);
    EXPECT_EQ(heat.Mode(), SystemModeEnum::kHeat);

    // SetTemperature changes value
    EXPECT_TRUE(heat.SetTemperature(2100));
    EXPECT_EQ(heat.Temperature(), 2100);
    // Setting identical temperature returns false
    EXPECT_FALSE(heat.SetTemperature(2100));

    AbsoluteSetpoint heat2(OccupiedHeatingSetpoint::Id, 2100);
    EXPECT_TRUE(heat == heat2);
    EXPECT_FALSE(heat != heat2);

    AbsoluteSetpoint cool(OccupiedCoolingSetpoint::Id, 2500);
    EXPECT_EQ(cool.Mode(), SystemModeEnum::kCool);
    EXPECT_TRUE(heat != cool);
}

TEST(TestSetpointPrimitives, TestOptionalSetpoint)
{
    AbsoluteSetpoint baseAbs(MinHeatSetpointLimit::Id, 1000);
    OptionalSetpoint opt(MinHeatSetpointLimit::Id, baseAbs);

    // Initially has no temperature override
    EXPECT_FALSE(opt.HasTemperature());
    EXPECT_EQ(opt.Temperature(), 1000);

    // Set override
    EXPECT_TRUE(opt.SetTemperature(1200));
    EXPECT_TRUE(opt.HasTemperature());
    EXPECT_EQ(opt.Temperature(), 1200);

    // Setting same returns false
    EXPECT_FALSE(opt.SetTemperature(1200));

    // Clear override
    EXPECT_TRUE(opt.ClearTemperature());
    EXPECT_FALSE(opt.HasTemperature());
    EXPECT_EQ(opt.Temperature(), 1000);
    EXPECT_FALSE(opt.ClearTemperature());

    // Assignment from Optional<temperature>
    opt = MakeOptional<temperature>(static_cast<temperature>(1400));
    EXPECT_TRUE(opt.HasTemperature());
    EXPECT_EQ(opt.Temperature(), 1400);

    // Copy constructor & assignment
    OptionalSetpoint optCopy(opt);
    EXPECT_TRUE(optCopy.HasTemperature());
    EXPECT_EQ(optCopy.Temperature(), 1400);

    OptionalSetpoint optAssign(MinHeatSetpointLimit::Id, baseAbs);
    optAssign = opt;
    EXPECT_TRUE(optAssign.HasTemperature());
    EXPECT_EQ(optAssign.Temperature(), 1400);
}

TEST(TestSetpointPrimitives, TestSetpointLimitsAndRange)
{
    AbsoluteSetpoint minHeat(AbsMinHeatSetpointLimit::Id, 700);
    AbsoluteSetpoint maxHeat(AbsMaxHeatSetpointLimit::Id, 3000);
    AbsoluteSetpointLimits limits(minHeat, maxHeat);

    EXPECT_TRUE(limits.IsValid());
    EXPECT_EQ(limits.Minimum(), 700);
    EXPECT_EQ(limits.Maximum(), 3000);
    EXPECT_TRUE(limits.Valid(2000));
    EXPECT_FALSE(limits.Valid(500));
    EXPECT_FALSE(limits.Valid(3500));

    AbsoluteSetpoint testPt(OccupiedHeatingSetpoint::Id, 2000);
    EXPECT_TRUE(limits.Valid(testPt));

    EXPECT_EQ(limits.Clamp(500), 700);
    EXPECT_EQ(limits.Clamp(3500), 3000);
    EXPECT_EQ(limits.Clamp(2000), 2000);

    AbsoluteSetpoint cool(OccupiedCoolingSetpoint::Id, 2500);
    SetpointRange range(testPt, cool);
    EXPECT_EQ(range.heating.Temperature(), 2000);
    EXPECT_EQ(range.cooling.Temperature(), 2500);

    SetpointRange rangeCopy(range);
    EXPECT_EQ(rangeCopy.heating.Temperature(), 2000);
}

TEST(TestSetpointPrimitives, TestSetpointAttributes)
{
    SetpointAttributes attrs;
    EXPECT_TRUE(attrs.Empty());
    EXPECT_FALSE(attrs.Has(OccupiedHeatingSetpoint::Id));

    attrs.Set(OccupiedHeatingSetpoint::Id);
    EXPECT_FALSE(attrs.Empty());
    EXPECT_TRUE(attrs.Has(OccupiedHeatingSetpoint::Id));
    EXPECT_TRUE(attrs.HasAny(OccupiedHeatingSetpoint::Id, OccupiedCoolingSetpoint::Id));
    EXPECT_EQ(attrs.FirstDirtyAttribute(), OccupiedHeatingSetpoint::Id);

    attrs.Set(OccupiedCoolingSetpoint::Id);
    EXPECT_TRUE(attrs.Has(OccupiedCoolingSetpoint::Id));

    attrs.ClearFirstDirtyAttribute();
    EXPECT_EQ(attrs.FirstDirtyAttribute(), kInvalidAttributeId);

    attrs.Clear(OccupiedHeatingSetpoint::Id);
    EXPECT_FALSE(attrs.Has(OccupiedHeatingSetpoint::Id));
    EXPECT_TRUE(attrs.Has(OccupiedCoolingSetpoint::Id));

    attrs.ClearAll();
    EXPECT_TRUE(attrs.Empty());
}

TEST(TestSetpointPrimitives, TestSetpointsLogic)
{
    Setpoints setpoints;
    setpoints.heatSupported = true;
    setpoints.coolSupported = true;
    setpoints.autoSupported = true;
    setpoints.deadBand      = 200; // 2.0 C

    EXPECT_TRUE(setpoints.Valid());

    // Violating deadband in auto mode: heat 2450, cool 2550 (difference 100 < 200)
    setpoints.occupiedRange.heating.SetTemperature(2450);
    setpoints.occupiedRange.cooling.SetTemperature(2550);
    EXPECT_FALSE(setpoints.Valid());

    // Fix should adjust cooling to 2650
    SetpointAttributes changed;
    changed.Set(OccupiedHeatingSetpoint::Id);
    EXPECT_EQ(setpoints.Fix(changed), Status::Success);
    EXPECT_TRUE(setpoints.Valid());
    EXPECT_GE(setpoints.occupiedRange.cooling.Temperature() - setpoints.occupiedRange.heating.Temperature(), 200);

    // Test ChangeRangeHeating
    SetpointAttributes changedAttrs;
    EXPECT_EQ(setpoints.ChangeRangeHeating(setpoints.occupiedRange, 2100, Setpoints::ClampMode::kClamp, changedAttrs),
              Status::Success);
    EXPECT_EQ(setpoints.occupiedRange.heating.Temperature(), 2100);

    // Test ChangeLimitMinimum & ChangeLimitMaximum
    EXPECT_EQ(setpoints.ChangeLimitMinimum(setpoints.userHeatLimits, setpoints.absoluteHeatLimits, 1000, changedAttrs),
              Status::Success);
    EXPECT_EQ(setpoints.userHeatLimits.minimum.Temperature(), 1000);

    EXPECT_EQ(setpoints.ChangeLimitMaximum(setpoints.userHeatLimits, setpoints.absoluteHeatLimits, 2800, changedAttrs),
              Status::Success);
    EXPECT_EQ(setpoints.userHeatLimits.maximum.Temperature(), 2800);
}

TEST_F(ThermostatTestFixture, TestOccupiedHeatingAndCoolingSetpointsAttributes)
{
    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling, Feature::kAutoMode, Feature::kEvents);
    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate,
                              mAutoDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Read OccupiedHeatingSetpoint
    temperature heat = 0;
    EXPECT_EQ(tester.ReadAttribute(OccupiedHeatingSetpoint::Id, heat), Status::Success);
    EXPECT_EQ(heat, 2000);

    // Read OccupiedCoolingSetpoint
    temperature cool = 0;
    EXPECT_EQ(tester.ReadAttribute(OccupiedCoolingSetpoint::Id, cool), Status::Success);
    EXPECT_EQ(cool, 2600);

    // Write OccupiedHeatingSetpoint
    EXPECT_EQ(tester.WriteAttribute(OccupiedHeatingSetpoint::Id, static_cast<temperature>(2100)), Status::Success);
    EXPECT_EQ(mHeatingDelegate.mOccupiedHeatingSetpoint, 2100);
    EXPECT_TRUE(tester.IsAttributeDirty(OccupiedHeatingSetpoint::Id));

    // Verify SetpointChangeEvent
    auto event = tester.GetNextGeneratedEvent();
    ASSERT_TRUE(event.has_value());
    EXPECT_EQ(event->eventOptions.mPath.mEventId, Events::SetpointChange::Id);

    // Write OccupiedCoolingSetpoint
    EXPECT_EQ(tester.WriteAttribute(OccupiedCoolingSetpoint::Id, static_cast<temperature>(2500)), Status::Success);
    EXPECT_EQ(mCoolingDelegate.mOccupiedCoolingSetpoint, 2500);
    EXPECT_TRUE(tester.IsAttributeDirty(OccupiedCoolingSetpoint::Id));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestUnoccupiedSetpointsWithOccupancyFeature)
{
    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling, Feature::kOccupancy);
    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate,
                              mOccupancyDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(HasAttribute(cluster, UnoccupiedHeatingSetpoint::Id));
    EXPECT_TRUE(HasAttribute(cluster, UnoccupiedCoolingSetpoint::Id));

    // Read unoccupied heating & cooling setpoints
    temperature unoccHeat = 0;
    EXPECT_EQ(tester.ReadAttribute(UnoccupiedHeatingSetpoint::Id, unoccHeat), Status::Success);
    EXPECT_EQ(unoccHeat, 1600);

    temperature unoccCool = 0;
    EXPECT_EQ(tester.ReadAttribute(UnoccupiedCoolingSetpoint::Id, unoccCool), Status::Success);
    EXPECT_EQ(unoccCool, 2800);

    // Write unoccupied heating & cooling setpoints
    EXPECT_EQ(tester.WriteAttribute(UnoccupiedHeatingSetpoint::Id, static_cast<temperature>(1700)), Status::Success);
    EXPECT_EQ(mHeatingDelegate.mUnoccupiedHeatingSetpoint, 1700);

    EXPECT_EQ(tester.WriteAttribute(UnoccupiedCoolingSetpoint::Id, static_cast<temperature>(2700)), Status::Success);
    EXPECT_EQ(mCoolingDelegate.mUnoccupiedCoolingSetpoint, 2700);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestAbsAndMinMaxLimitsAttributes)
{
    mOptionalAttributes.AbsMinHeatSetpointLimit = true;
    mOptionalAttributes.AbsMaxHeatSetpointLimit = true;
    mOptionalAttributes.AbsMinCoolSetpointLimit = true;
    mOptionalAttributes.AbsMaxCoolSetpointLimit = true;
    mOptionalAttributes.MinHeatSetpointLimit    = true;
    mOptionalAttributes.MaxHeatSetpointLimit    = true;
    mOptionalAttributes.MinCoolSetpointLimit    = true;
    mOptionalAttributes.MaxCoolSetpointLimit    = true;

    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling);
    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Read Abs limits
    temperature absMinHeat = 0;
    EXPECT_EQ(tester.ReadAttribute(AbsMinHeatSetpointLimit::Id, absMinHeat), Status::Success);
    EXPECT_EQ(absMinHeat, 700);

    temperature absMaxHeat = 0;
    EXPECT_EQ(tester.ReadAttribute(AbsMaxHeatSetpointLimit::Id, absMaxHeat), Status::Success);
    EXPECT_EQ(absMaxHeat, 3000);

    temperature absMinCool = 0;
    EXPECT_EQ(tester.ReadAttribute(AbsMinCoolSetpointLimit::Id, absMinCool), Status::Success);
    EXPECT_EQ(absMinCool, 1600);

    temperature absMaxCool = 0;
    EXPECT_EQ(tester.ReadAttribute(AbsMaxCoolSetpointLimit::Id, absMaxCool), Status::Success);
    EXPECT_EQ(absMaxCool, 3200);

    // Writing Abs limits is unsupported
    EXPECT_EQ(tester.WriteAttribute(AbsMinHeatSetpointLimit::Id, static_cast<temperature>(800)), Status::UnsupportedWrite);
    EXPECT_EQ(tester.WriteAttribute(AbsMaxHeatSetpointLimit::Id, static_cast<temperature>(2900)), Status::UnsupportedWrite);

    // Read & Write User Limits
    temperature minHeat = 0;
    EXPECT_EQ(tester.ReadAttribute(MinHeatSetpointLimit::Id, minHeat), Status::Success);
    EXPECT_EQ(minHeat, 700);

    EXPECT_EQ(tester.WriteAttribute(MinHeatSetpointLimit::Id, static_cast<temperature>(900)), Status::Success);
    EXPECT_EQ(mHeatingDelegate.mMinHeat, 900);

    temperature maxCool = 0;
    EXPECT_EQ(tester.ReadAttribute(MaxCoolSetpointLimit::Id, maxCool), Status::Success);
    EXPECT_EQ(maxCool, 3200);

    EXPECT_EQ(tester.WriteAttribute(MaxCoolSetpointLimit::Id, static_cast<temperature>(3100)), Status::Success);
    EXPECT_EQ(mCoolingDelegate.mMaxCool, 3100);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestSetpointRaiseLowerCommand)
{
    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling, Feature::kAutoMode);
    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate,
                              mAutoDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Initial setpoints: heating = 2000, cooling = 2600
    // Raise Heat by +1.0 C (amount = 10, meaning 100 in 0.01 C)
    Commands::SetpointRaiseLower::Type req;
    req.mode   = SetpointRaiseLowerModeEnum::kHeat;
    req.amount = 10;

    auto result = tester.Invoke(req);
    EXPECT_TRUE(result.status.has_value());
    EXPECT_EQ(result.status->GetUnderlyingError(), CHIP_NO_ERROR);
    EXPECT_EQ(mHeatingDelegate.mOccupiedHeatingSetpoint, 2100);
    EXPECT_EQ(mCoolingDelegate.mOccupiedCoolingSetpoint, 2600);

    // Lower Heat by -2.0 C (amount = -20)
    req.mode   = SetpointRaiseLowerModeEnum::kHeat;
    req.amount = -20;
    result     = tester.Invoke(req);
    EXPECT_TRUE(result.status.has_value());
    EXPECT_EQ(result.status->GetUnderlyingError(), CHIP_NO_ERROR);
    EXPECT_EQ(mHeatingDelegate.mOccupiedHeatingSetpoint, 1900);

    // Raise Cool by +1.5 C (amount = 15)
    req.mode   = SetpointRaiseLowerModeEnum::kCool;
    req.amount = 15;
    result     = tester.Invoke(req);
    EXPECT_TRUE(result.status.has_value());
    EXPECT_EQ(result.status->GetUnderlyingError(), CHIP_NO_ERROR);
    EXPECT_EQ(mCoolingDelegate.mOccupiedCoolingSetpoint, 2750);

    // Raise Both by +1.0 C (amount = 10)
    req.mode   = SetpointRaiseLowerModeEnum::kBoth;
    req.amount = 10;
    result     = tester.Invoke(req);
    EXPECT_TRUE(result.status.has_value());
    EXPECT_EQ(result.status->GetUnderlyingError(), CHIP_NO_ERROR);
    EXPECT_EQ(mHeatingDelegate.mOccupiedHeatingSetpoint, 2000);
    EXPECT_EQ(mCoolingDelegate.mOccupiedCoolingSetpoint, 2850);

    // Invalid mode
    req.mode   = static_cast<SetpointRaiseLowerModeEnum>(0x99);
    req.amount = 10;
    result     = tester.Invoke(req);
    EXPECT_TRUE(result.status.has_value());
    ASSERT_TRUE(result.GetStatusCode().has_value());
    EXPECT_EQ(result.GetStatusCode()->GetStatus(), Status::InvalidCommand);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(ThermostatTestFixture, TestSetpointRaiseLowerUnoccupied)
{
    BitFlags<Feature> features(Feature::kHeating, Feature::kCooling, Feature::kOccupancy);
    ThermostatCluster cluster(kTestEndpointId, features, MakeConfig(), mThermostatDelegate, mHeatingDelegate, mCoolingDelegate,
                              mOccupancyDelegate);
    ClusterTester tester(cluster);
    SetupTesterSubject(tester);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Set occupancy to unoccupied
    mOccupancyDelegate.mOccupancy.ClearAll();
    EXPECT_FALSE(cluster.IsOccupied());

    // Initial unoccupied setpoints: heat = 1600, cool = 2800
    Commands::SetpointRaiseLower::Type req;
    req.mode   = SetpointRaiseLowerModeEnum::kHeat;
    req.amount = 20;

    auto result = tester.Invoke(req);
    EXPECT_TRUE(result.status.has_value());
    EXPECT_EQ(result.status->GetUnderlyingError(), CHIP_NO_ERROR);
    EXPECT_EQ(mHeatingDelegate.mUnoccupiedHeatingSetpoint, 1800);
    // Occupied setpoint unchanged
    EXPECT_EQ(mHeatingDelegate.mOccupiedHeatingSetpoint, 2000);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

} // namespace
