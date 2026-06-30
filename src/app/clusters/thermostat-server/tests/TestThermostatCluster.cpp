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

// Unit tests for the code-driven surface delivered by PR 3a of the Thermostat migration:
// scalar attributes, ported constraint validation + Auto-mode deadband handling, SetpointRaiseLower,
// LocalTemperatureNotExposed behavior, and feature-gated metadata.
//
// Preset / schedule / suggestion / atomic-write coverage is intentionally out of scope here; that logic
// is still owned by the retained ThermostatAttrAccess helper and is added in PR 3b once it is re-housed
// into the cluster instance (and emits events through the cluster context so they can be observed).

#include <app/clusters/thermostat-server/ThermostatCluster.h>

#include <app-common/zap-generated/cluster-objects.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/Thermostat/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <pw_unit_test/framework.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Thermostat;
using namespace chip::app::Clusters::Thermostat::Attributes;
using namespace chip::Testing;
using Protocols::InteractionModel::Status;

using chip::Testing::IsAcceptedCommandsListEqualTo;
using chip::Testing::IsAttributesListEqualTo;

// The cluster sources call the free Thermostat::GetDelegate(endpoint) (normally defined in
// CodegenIntegration.cpp, which is excluded from this test target to avoid the app-specific
// StaticApplicationConfig dependency). The scalar / SetpointRaiseLower paths exercised here never use a
// delegate, so a null-returning stub is sufficient to satisfy linking.
namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {
Delegate * GetDelegate(EndpointId)
{
    return nullptr;
}
} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip

// --- Temporary link shims for the Ember-coupled retained-helper Read path ---
// The retained ThermostatAttrAccess::Read still references the Ember Attributes::FeatureMap/RemoteSensing
// GetDefault accessors (in branches the code-driven ReadAttribute never actually reaches), whose real
// implementations require a full Ember app image. They are not exercised by the tests below, so dummy
// implementations are sufficient to link. PR 3b-3 removes the retained helper, after which these shims can
// be deleted.
namespace chip {
namespace app {
namespace Clusters {
namespace Thermostat {
namespace Attributes {
namespace FeatureMap {
Protocols::InteractionModel::Status GetDefault(EndpointId, uint32_t *)
{
    return Protocols::InteractionModel::Status::Failure;
}
} // namespace FeatureMap
namespace RemoteSensing {
Protocols::InteractionModel::Status GetDefault(EndpointId, BitMask<RemoteSensingBitmap> *)
{
    return Protocols::InteractionModel::Status::Failure;
}
} // namespace RemoteSensing
} // namespace Attributes
} // namespace Thermostat
} // namespace Clusters
} // namespace app
} // namespace chip

namespace {

constexpr EndpointId kEndpointId = 1;

constexpr uint32_t kFeatHeat = static_cast<uint32_t>(Feature::kHeating);
constexpr uint32_t kFeatCool = static_cast<uint32_t>(Feature::kCooling);
constexpr uint32_t kFeatOcc  = static_cast<uint32_t>(Feature::kOccupancy);
constexpr uint32_t kFeatAuto = static_cast<uint32_t>(Feature::kAutoMode);
constexpr uint32_t kFeatLtne = static_cast<uint32_t>(Feature::kLocalTemperatureNotExposed);
constexpr uint32_t kFeatPres = static_cast<uint32_t>(Feature::kPresets);

constexpr uint32_t kHeatCool     = kFeatHeat | kFeatCool;
constexpr uint32_t kHeatCoolAuto = kFeatHeat | kFeatCool | kFeatAuto;

ThermostatCluster::StartupConfiguration DefaultConfig()
{
    return ThermostatCluster::StartupConfiguration{};
}

class TestThermostatCluster : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }
};

// Small holder so each test can spin up an independent cluster with a chosen feature map / config.
struct ClusterFixture
{
    // Declared before `cluster` so it is constructed first (the cluster's Context holds a reference to it)
    // and destroyed last. AddFabricDelegate/RemoveFabricDelegate are simple list operations that do not
    // require FabricTable::Init(), so a default-constructed table is sufficient for these tests.
    chip::FabricTable fabricTable;
    ThermostatCluster cluster;
    ClusterTester tester{ cluster };

    ClusterFixture(uint32_t featureMap, const ThermostatCluster::StartupConfiguration & config = DefaultConfig()) :
        cluster(kEndpointId, featureMap, config, ThermostatCluster::Context{ fabricTable })
    {
        EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);
    }

    ~ClusterFixture() { cluster.Shutdown(app::ClusterShutdownType::kClusterShutdown); }
};

TEST_F(TestThermostatCluster, ReadsClusterRevisionAndFeatureMap)
{
    ClusterFixture fixture(kHeatCoolAuto);

    uint16_t revision = 0;
    EXPECT_EQ(fixture.tester.ReadAttribute(ClusterRevision::Id, revision), CHIP_NO_ERROR);
    EXPECT_EQ(revision, Thermostat::kRevision);

    uint32_t featureMap = 0;
    EXPECT_EQ(fixture.tester.ReadAttribute(FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, kHeatCoolAuto);
}

TEST_F(TestThermostatCluster, ReadsScalarDefaults)
{
    ClusterFixture fixture(kHeatCool);

    int16_t value = 0;
    EXPECT_EQ(fixture.tester.ReadAttribute(OccupiedHeatingSetpoint::Id, value), CHIP_NO_ERROR);
    EXPECT_EQ(value, 2000);
    EXPECT_EQ(fixture.tester.ReadAttribute(OccupiedCoolingSetpoint::Id, value), CHIP_NO_ERROR);
    EXPECT_EQ(value, 2600);
    EXPECT_EQ(fixture.tester.ReadAttribute(AbsMinHeatSetpointLimit::Id, value), CHIP_NO_ERROR);
    EXPECT_EQ(value, 700);
    EXPECT_EQ(fixture.tester.ReadAttribute(AbsMaxCoolSetpointLimit::Id, value), CHIP_NO_ERROR);
    EXPECT_EQ(value, 3200);
}

TEST_F(TestThermostatCluster, AttributeFeatureGating)
{
    // Heat + Cool + Auto, but no Occupancy and no Presets.
    ClusterFixture fixture(kHeatCoolAuto);

    int16_t i16 = 0;
    // Present: heating/cooling setpoints, Auto deadband.
    EXPECT_EQ(fixture.tester.ReadAttribute(OccupiedHeatingSetpoint::Id, i16), CHIP_NO_ERROR);
    EXPECT_EQ(fixture.tester.ReadAttribute(OccupiedCoolingSetpoint::Id, i16), CHIP_NO_ERROR);
    int8_t deadBand = 0;
    EXPECT_EQ(fixture.tester.ReadAttribute(MinSetpointDeadBand::Id, deadBand), CHIP_NO_ERROR);

    // Absent: Occupancy (no OCC), Unoccupied setpoints (need OCC), Presets attributes (no PRES).
    BitMask<OccupancyBitmap> occ;
    EXPECT_EQ(fixture.tester.ReadAttribute(Occupancy::Id, occ), Status::UnsupportedAttribute);
    EXPECT_EQ(fixture.tester.ReadAttribute(UnoccupiedHeatingSetpoint::Id, i16), Status::UnsupportedAttribute);
    uint8_t numberOfPresets = 0;
    EXPECT_EQ(fixture.tester.ReadAttribute(NumberOfPresets::Id, numberOfPresets), Status::UnsupportedAttribute);
}

TEST_F(TestThermostatCluster, OccupancyAttributesGatedOnOccupancyFeature)
{
    ClusterFixture fixture(kHeatCool | kFeatOcc);

    BitMask<OccupancyBitmap> occ;
    EXPECT_EQ(fixture.tester.ReadAttribute(Occupancy::Id, occ), CHIP_NO_ERROR);
    int16_t i16 = 0;
    EXPECT_EQ(fixture.tester.ReadAttribute(UnoccupiedHeatingSetpoint::Id, i16), CHIP_NO_ERROR);
    EXPECT_EQ(fixture.tester.ReadAttribute(UnoccupiedCoolingSetpoint::Id, i16), CHIP_NO_ERROR);
}

TEST_F(TestThermostatCluster, AcceptedCommandsGating_HeatCoolOnly)
{
    ClusterFixture fixture(kHeatCool);

    // With neither Presets nor MatterScheduleConfiguration, only the mandatory SetpointRaiseLower is accepted.
    EXPECT_TRUE(IsAcceptedCommandsListEqualTo(fixture.cluster,
                                              {
                                                  Commands::SetpointRaiseLower::kMetadataEntry,
                                              }));
}

TEST_F(TestThermostatCluster, CommandFeatureGating_PresetCommandRequiresPresetsFeature)
{
    {
        ClusterFixture fixture(kHeatCool);
        auto result = fixture.tester.Invoke<Commands::SetActivePresetRequest::Type>(Commands::SetActivePresetRequest::Type());
        EXPECT_EQ(result.GetStatusCode(),
                  Protocols::InteractionModel::ClusterStatusCode(Status::UnsupportedCommand));
    }
    {
        // With Presets enabled the command is accepted (it then reaches the delegate path, which is null in
        // this test and yields a failure status — but crucially NOT UnsupportedCommand).
        ClusterFixture fixture(kHeatCool | kFeatPres);
        auto result = fixture.tester.Invoke<Commands::SetActivePresetRequest::Type>(Commands::SetActivePresetRequest::Type());
        EXPECT_NE(result.GetStatusCode(),
                  Protocols::InteractionModel::ClusterStatusCode(Status::UnsupportedCommand));
    }
}

TEST_F(TestThermostatCluster, WriteOccupiedHeatingSetpointWithinLimitsReportsDirty)
{
    ClusterFixture fixture(kHeatCool);

    EXPECT_EQ(fixture.tester.WriteAttribute(OccupiedHeatingSetpoint::Id, static_cast<int16_t>(2100)), CHIP_NO_ERROR);

    int16_t value = 0;
    EXPECT_EQ(fixture.tester.ReadAttribute(OccupiedHeatingSetpoint::Id, value), CHIP_NO_ERROR);
    EXPECT_EQ(value, 2100);
    EXPECT_TRUE(fixture.tester.IsAttributeDirty(OccupiedHeatingSetpoint::Id));
}

TEST_F(TestThermostatCluster, WriteOccupiedHeatingSetpointConstraintMatrix)
{
    ClusterFixture fixture(kHeatCool); // no Auto -> no deadband coupling

    // Below AbsMin / above AbsMax are rejected, value unchanged.
    EXPECT_EQ(fixture.tester.WriteAttribute(OccupiedHeatingSetpoint::Id, static_cast<int16_t>(600)), Status::InvalidValue);
    EXPECT_EQ(fixture.tester.WriteAttribute(OccupiedHeatingSetpoint::Id, static_cast<int16_t>(3100)), Status::InvalidValue);

    int16_t value = 0;
    EXPECT_EQ(fixture.tester.ReadAttribute(OccupiedHeatingSetpoint::Id, value), CHIP_NO_ERROR);
    EXPECT_EQ(value, 2000); // still the default

    // At the min and max boundaries are accepted.
    EXPECT_EQ(fixture.tester.WriteAttribute(OccupiedHeatingSetpoint::Id, static_cast<int16_t>(700)), CHIP_NO_ERROR);
    EXPECT_EQ(fixture.tester.ReadAttribute(OccupiedHeatingSetpoint::Id, value), CHIP_NO_ERROR);
    EXPECT_EQ(value, 700);

    EXPECT_EQ(fixture.tester.WriteAttribute(OccupiedHeatingSetpoint::Id, static_cast<int16_t>(3000)), CHIP_NO_ERROR);
    EXPECT_EQ(fixture.tester.ReadAttribute(OccupiedHeatingSetpoint::Id, value), CHIP_NO_ERROR);
    EXPECT_EQ(value, 3000);
}

TEST_F(TestThermostatCluster, NoOpWriteDoesNotReportDirty)
{
    ClusterFixture fixture(kHeatCool);

    EXPECT_EQ(fixture.tester.WriteAttribute(OccupiedHeatingSetpoint::Id, static_cast<int16_t>(2100)), CHIP_NO_ERROR);
    auto & dirtyList = fixture.tester.GetDirtyList();
    EXPECT_FALSE(dirtyList.empty());
    dirtyList.clear();

    // Writing the same value again is a no-op and must not mark the attribute dirty.
    EXPECT_EQ(fixture.tester.WriteAttribute(OccupiedHeatingSetpoint::Id, static_cast<int16_t>(2100)), CHIP_NO_ERROR);
    EXPECT_TRUE(dirtyList.empty());
}

TEST_F(TestThermostatCluster, AutoModeShiftsPairedSetpointToMaintainDeadband)
{
    // Default deadband is 20 (0.1C) -> 200 in 0.01C units. occHeat=2000, occCool=2600.
    ClusterFixture fixture(kHeatCoolAuto);

    // Raising heating to 2500 violates the 200 deadband against cooling (2600 - 2500 = 100), so cooling must
    // be shifted up to 2700 to preserve it.
    EXPECT_EQ(fixture.tester.WriteAttribute(OccupiedHeatingSetpoint::Id, static_cast<int16_t>(2500)), CHIP_NO_ERROR);

    int16_t value = 0;
    EXPECT_EQ(fixture.tester.ReadAttribute(OccupiedHeatingSetpoint::Id, value), CHIP_NO_ERROR);
    EXPECT_EQ(value, 2500);
    EXPECT_EQ(fixture.tester.ReadAttribute(OccupiedCoolingSetpoint::Id, value), CHIP_NO_ERROR);
    EXPECT_EQ(value, 2700);
    EXPECT_TRUE(fixture.tester.IsAttributeDirty(OccupiedCoolingSetpoint::Id));
}

TEST_F(TestThermostatCluster, AutoModeRejectsSetpointWhenDeadbandInfeasible)
{
    // Constrain the cooling ceiling so the deadband cannot be preserved.
    ThermostatCluster::StartupConfiguration config = DefaultConfig();
    config.absMaxCoolSetpointLimit                 = 2500;

    ClusterFixture fixture(kHeatCoolAuto, config);

    fixture.cluster.SetMaxCoolSetpointLimit(2500);
    fixture.cluster.SetOccupiedCoolingSetpoint(2400);

    // Heating 2400 would require cooling >= 2600, but cooling is capped at 2500 -> rejected, nothing changes.
    EXPECT_EQ(fixture.tester.WriteAttribute(OccupiedHeatingSetpoint::Id, static_cast<int16_t>(2400)), Status::InvalidValue);

    int16_t value = 0;
    EXPECT_EQ(fixture.tester.ReadAttribute(OccupiedHeatingSetpoint::Id, value), CHIP_NO_ERROR);
    EXPECT_EQ(value, 2000);
    EXPECT_EQ(fixture.tester.ReadAttribute(OccupiedCoolingSetpoint::Id, value), CHIP_NO_ERROR);
    EXPECT_EQ(value, 2400);
}

TEST_F(TestThermostatCluster, SetpointRaiseLowerHeat)
{
    ClusterFixture fixture(kHeatCool);

    Commands::SetpointRaiseLower::Type request;
    request.mode   = SetpointRaiseLowerModeEnum::kHeat;
    request.amount = 5; // +0.5C -> +50 in 0.01C units

    auto result = fixture.tester.Invoke<Commands::SetpointRaiseLower::Type>(request);
    EXPECT_TRUE(result.IsSuccess());

    int16_t value = 0;
    EXPECT_EQ(fixture.tester.ReadAttribute(OccupiedHeatingSetpoint::Id, value), CHIP_NO_ERROR);
    EXPECT_EQ(value, 2050);
}

TEST_F(TestThermostatCluster, SetpointRaiseLowerCool)
{
    ClusterFixture fixture(kHeatCool);

    Commands::SetpointRaiseLower::Type request;
    request.mode   = SetpointRaiseLowerModeEnum::kCool;
    request.amount = -5; // -0.5C -> -50 in 0.01C units

    auto result = fixture.tester.Invoke<Commands::SetpointRaiseLower::Type>(request);
    EXPECT_TRUE(result.IsSuccess());

    int16_t value = 0;
    EXPECT_EQ(fixture.tester.ReadAttribute(OccupiedCoolingSetpoint::Id, value), CHIP_NO_ERROR);
    EXPECT_EQ(value, 2550);
}

TEST_F(TestThermostatCluster, SetpointRaiseLowerBoth)
{
    ClusterFixture fixture(kHeatCool);

    Commands::SetpointRaiseLower::Type request;
    request.mode   = SetpointRaiseLowerModeEnum::kBoth;
    request.amount = 10; // +1.0C -> +100 in 0.01C units

    auto result = fixture.tester.Invoke<Commands::SetpointRaiseLower::Type>(request);
    EXPECT_TRUE(result.IsSuccess());

    int16_t value = 0;
    EXPECT_EQ(fixture.tester.ReadAttribute(OccupiedHeatingSetpoint::Id, value), CHIP_NO_ERROR);
    EXPECT_EQ(value, 2100);
    EXPECT_EQ(fixture.tester.ReadAttribute(OccupiedCoolingSetpoint::Id, value), CHIP_NO_ERROR);
    EXPECT_EQ(value, 2700);
}

TEST_F(TestThermostatCluster, SetpointRaiseLowerCoolUnsupportedWithoutCoolingFeature)
{
    ClusterFixture fixture(kFeatHeat); // heat only

    Commands::SetpointRaiseLower::Type request;
    request.mode   = SetpointRaiseLowerModeEnum::kCool;
    request.amount = 5;

    auto result = fixture.tester.Invoke<Commands::SetpointRaiseLower::Type>(request);
    EXPECT_EQ(result.GetStatusCode(), Protocols::InteractionModel::ClusterStatusCode(Status::InvalidCommand));
}

TEST_F(TestThermostatCluster, LocalTemperatureNotExposedReadsNull)
{
    ClusterFixture fixture(kHeatCool | kFeatLtne);

    DataModel::Nullable<int16_t> localTemperature;
    localTemperature.SetNonNull(1234);
    EXPECT_EQ(fixture.tester.ReadAttribute(LocalTemperature::Id, localTemperature), CHIP_NO_ERROR);
    EXPECT_TRUE(localTemperature.IsNull());
}

TEST_F(TestThermostatCluster, RemoteSensingClearsLocalTemperatureBitUnderLtne)
{
    ClusterFixture fixture(kHeatCool | kFeatLtne);

    // Writing the LocalTemperature RemoteSensing bit is rejected while LTNE is supported.
    BitMask<RemoteSensingBitmap> withLocalTemp;
    withLocalTemp.Set(RemoteSensingBitmap::kLocalTemperature);
    EXPECT_EQ(fixture.tester.WriteAttribute(RemoteSensing::Id, withLocalTemp), Status::ConstraintError);

    // A non-LocalTemperature bit is accepted, and reads back with the LocalTemperature bit masked off.
    BitMask<RemoteSensingBitmap> outdoor;
    outdoor.Set(RemoteSensingBitmap::kOutdoorTemperature);
    EXPECT_EQ(fixture.tester.WriteAttribute(RemoteSensing::Id, outdoor), CHIP_NO_ERROR);

    BitMask<RemoteSensingBitmap> readBack;
    EXPECT_EQ(fixture.tester.ReadAttribute(RemoteSensing::Id, readBack), CHIP_NO_ERROR);
    EXPECT_FALSE(readBack.Has(RemoteSensingBitmap::kLocalTemperature));
    EXPECT_TRUE(readBack.Has(RemoteSensingBitmap::kOutdoorTemperature));
}

TEST_F(TestThermostatCluster, WriteReadOnlyAttributeRejected)
{
    ClusterFixture fixture(kHeatCool);

    // AbsMinHeatSetpointLimit is present (Heat feature) but read-only.
    EXPECT_EQ(fixture.tester.WriteAttribute(AbsMinHeatSetpointLimit::Id, static_cast<int16_t>(800)),
              Status::UnsupportedWrite);
}

TEST_F(TestThermostatCluster, SystemModeRejectedAgainstControlSequence)
{
    ClusterFixture fixture(kHeatCool);

    // Restrict to cooling-only, then a Heat system mode must be rejected.
    EXPECT_EQ(fixture.tester.WriteAttribute(ControlSequenceOfOperation::Id, ControlSequenceOfOperationEnum::kCoolingOnly),
              CHIP_NO_ERROR);
    EXPECT_EQ(fixture.tester.WriteAttribute(SystemMode::Id, SystemModeEnum::kHeat), Status::InvalidValue);

    // A compatible mode is accepted.
    EXPECT_EQ(fixture.tester.WriteAttribute(SystemMode::Id, SystemModeEnum::kCool), CHIP_NO_ERROR);
    SystemModeEnum mode = SystemModeEnum::kOff;
    EXPECT_EQ(fixture.tester.ReadAttribute(SystemMode::Id, mode), CHIP_NO_ERROR);
    EXPECT_EQ(mode, SystemModeEnum::kCool);
}

} // namespace
