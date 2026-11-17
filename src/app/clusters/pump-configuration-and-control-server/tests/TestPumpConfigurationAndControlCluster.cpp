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

#include <app/clusters/pump-configuration-and-control-server/PumpConfigurationAndControlCluster.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/PumpConfigurationAndControl/Events.h>
#include <clusters/PumpConfigurationAndControl/Metadata.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <pw_unit_test/framework.h>

#include <vector>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::PumpConfigurationAndControl;
using namespace chip::app::Clusters::PumpConfigurationAndControl::Attributes;
using namespace chip::Testing;
using chip::Protocols::InteractionModel::Status;

constexpr EndpointId kTestEndpointId = 1;

class TestDelegate : public PumpConfigurationAndControl::Delegate
{
public:
    RemoteSensorType GetRemoteSensorType(EndpointId endpointId) override { return mRemoteSensorType; }
    void OnOperationModeMinMax(EndpointId endpointId, OperationModeEnum operationMode) override
    {
        mMinMaxCalls.push_back(operationMode);
    }

    RemoteSensorType mRemoteSensorType = RemoteSensorType::kNoSensor;
    std::vector<OperationModeEnum> mMinMaxCalls;
};

struct TestPumpConfigurationAndControlCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_SUCCESS(chip::Platform::MemoryInit()); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

PumpConfigurationAndControlCluster::Config ConfigWithFeatures(BitFlags<Feature> features)
{
    PumpConfigurationAndControlCluster::Config config;
    config.features = features;
    return config;
}

const std::vector<DataModel::AttributeEntry> kMandatoryAttributes(kMandatoryMetadata.begin(), kMandatoryMetadata.end());

std::vector<DataModel::AttributeEntry> MandatoryPlus(std::initializer_list<DataModel::AttributeEntry> extra)
{
    std::vector<DataModel::AttributeEntry> result = kMandatoryAttributes;
    for (const auto & entry : extra)
    {
        result.push_back(entry);
    }
    return result;
}

} // namespace

TEST_F(TestPumpConfigurationAndControlCluster, AttributeListWithoutFeatures)
{
    PumpConfigurationAndControlCluster cluster(kTestEndpointId, {});
    EXPECT_TRUE(IsAttributesListEqualTo(cluster, kMandatoryAttributes));
}

TEST_F(TestPumpConfigurationAndControlCluster, AttributeListFeatureLimitsAreMandatory)
{
    PumpConfigurationAndControlCluster cluster(
        kTestEndpointId, ConfigWithFeatures(BitFlags<Feature>(Feature::kConstantPressure, Feature::kConstantSpeed)));
    EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                        MandatoryPlus({
                                            MinConstPressure::kMetadataEntry,
                                            MaxConstPressure::kMetadataEntry,
                                            MinConstSpeed::kMetadataEntry,
                                            MaxConstSpeed::kMetadataEntry,
                                        })));
}

TEST_F(TestPumpConfigurationAndControlCluster, AttributeListLimitsOptionalWithAutomatic)
{
    PumpConfigurationAndControlCluster::OptionalAttributes requested;
    requested.minConstFlow = true;
    requested.maxConstFlow = true;

    {
        // Without the Automatic feature (or the Flow feature), the flow limits are disallowed.
        PumpConfigurationAndControlCluster::Config config;
        config.optionalAttributes = requested;
        PumpConfigurationAndControlCluster cluster(kTestEndpointId, config);
        EXPECT_TRUE(IsAttributesListEqualTo(cluster, kMandatoryAttributes));
    }

    {
        PumpConfigurationAndControlCluster::Config config = ConfigWithFeatures(BitFlags<Feature>(Feature::kAutomatic));
        config.optionalAttributes                         = requested;
        PumpConfigurationAndControlCluster cluster(kTestEndpointId, config);
        EXPECT_TRUE(
            IsAttributesListEqualTo(cluster, MandatoryPlus({ MinConstFlow::kMetadataEntry, MaxConstFlow::kMetadataEntry })));
    }
}

TEST_F(TestPumpConfigurationAndControlCluster, AttributeListOptionalAttributes)
{
    PumpConfigurationAndControlCluster::Config config;
    config.optionalAttributes.pumpStatus             = true;
    config.optionalAttributes.speed                  = true;
    config.optionalAttributes.lifetimeRunningHours   = true;
    config.optionalAttributes.power                  = true;
    config.optionalAttributes.lifetimeEnergyConsumed = true;
    config.optionalAttributes.controlMode            = true;

    PumpConfigurationAndControlCluster cluster(kTestEndpointId, config);
    EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                        MandatoryPlus({
                                            PumpStatus::kMetadataEntry,
                                            Speed::kMetadataEntry,
                                            LifetimeRunningHours::kMetadataEntry,
                                            Power::kMetadataEntry,
                                            LifetimeEnergyConsumed::kMetadataEntry,
                                            ControlMode::kMetadataEntry,
                                        })));
}

TEST_F(TestPumpConfigurationAndControlCluster, ReadAttributes)
{
    PumpConfigurationAndControlCluster::Config config =
        ConfigWithFeatures(BitFlags<Feature>(Feature::kConstantPressure, Feature::kConstantSpeed));
    config.fixedAttributes.maxPressure.SetNonNull(static_cast<int16_t>(20000));
    config.fixedAttributes.maxSpeed.SetNonNull(static_cast<uint16_t>(1000));
    config.fixedAttributes.minConstPressure.SetNonNull(static_cast<int16_t>(-1000));
    config.fixedAttributes.maxConstPressure.SetNonNull(static_cast<int16_t>(1000));

    PumpConfigurationAndControlCluster cluster(kTestEndpointId, config);
    ClusterTester tester(cluster);
    ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));

    uint16_t revision{};
    ASSERT_SUCCESS(tester.ReadAttribute(ClusterRevision::Id, revision));
    EXPECT_EQ(revision, PumpConfigurationAndControl::kRevision);

    uint32_t featureMap{};
    ASSERT_SUCCESS(tester.ReadAttribute(FeatureMap::Id, featureMap));
    EXPECT_EQ(featureMap, config.features.Raw());

    DataModel::Nullable<int16_t> int16Value;
    ASSERT_SUCCESS(tester.ReadAttribute(MaxPressure::Id, int16Value));
    EXPECT_EQ(int16Value, DataModel::MakeNullable(static_cast<int16_t>(20000)));
    ASSERT_SUCCESS(tester.ReadAttribute(MinConstPressure::Id, int16Value));
    EXPECT_EQ(int16Value, DataModel::MakeNullable(static_cast<int16_t>(-1000)));
    ASSERT_SUCCESS(tester.ReadAttribute(Capacity::Id, int16Value));
    EXPECT_TRUE(int16Value.IsNull());

    DataModel::Nullable<uint16_t> uint16Value;
    ASSERT_SUCCESS(tester.ReadAttribute(MaxSpeed::Id, uint16Value));
    EXPECT_EQ(uint16Value, DataModel::MakeNullable(static_cast<uint16_t>(1000)));
    ASSERT_SUCCESS(tester.ReadAttribute(MaxFlow::Id, uint16Value));
    EXPECT_TRUE(uint16Value.IsNull());

    OperationModeEnum operationMode{};
    ASSERT_SUCCESS(tester.ReadAttribute(OperationMode::Id, operationMode));
    EXPECT_EQ(operationMode, OperationModeEnum::kNormal);
    ASSERT_SUCCESS(tester.ReadAttribute(EffectiveOperationMode::Id, operationMode));
    EXPECT_EQ(operationMode, OperationModeEnum::kNormal);

    ControlModeEnum controlMode{};
    ASSERT_SUCCESS(tester.ReadAttribute(EffectiveControlMode::Id, controlMode));
    EXPECT_EQ(controlMode, ControlModeEnum::kConstantSpeed);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestPumpConfigurationAndControlCluster, OperationModeRequiresFeature)
{
    {
        PumpConfigurationAndControlCluster cluster(kTestEndpointId, {});
        ClusterTester tester(cluster);
        ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));

        EXPECT_EQ(tester.WriteAttribute(OperationMode::Id, OperationModeEnum::kNormal), CHIP_NO_ERROR);
        // Minimum and Maximum require ConstantSpeed, Local requires LocalOperation.
        EXPECT_EQ(tester.WriteAttribute(OperationMode::Id, OperationModeEnum::kMinimum), Status::ConstraintError);
        EXPECT_EQ(tester.WriteAttribute(OperationMode::Id, OperationModeEnum::kMaximum), Status::ConstraintError);
        EXPECT_EQ(tester.WriteAttribute(OperationMode::Id, OperationModeEnum::kLocal), Status::ConstraintError);
        // Values outside of OperationModeEnum are rejected.
        EXPECT_EQ(tester.WriteAttribute(OperationMode::Id, static_cast<uint8_t>(4)), Status::ConstraintError);
        EXPECT_EQ(cluster.GetOperationMode(), OperationModeEnum::kNormal);

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        PumpConfigurationAndControlCluster cluster(
            kTestEndpointId, ConfigWithFeatures(BitFlags<Feature>(Feature::kConstantSpeed, Feature::kLocalOperation)));
        ClusterTester tester(cluster);
        ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));

        EXPECT_EQ(tester.WriteAttribute(OperationMode::Id, OperationModeEnum::kMinimum), CHIP_NO_ERROR);
        EXPECT_EQ(tester.WriteAttribute(OperationMode::Id, OperationModeEnum::kMaximum), CHIP_NO_ERROR);
        EXPECT_EQ(tester.WriteAttribute(OperationMode::Id, OperationModeEnum::kLocal), CHIP_NO_ERROR);
        EXPECT_EQ(cluster.GetOperationMode(), OperationModeEnum::kLocal);

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestPumpConfigurationAndControlCluster, ControlModeRequiresFeature)
{
    struct
    {
        ControlModeEnum mode;
        Feature feature;
    } const kModeFeatures[] = {
        { ControlModeEnum::kConstantSpeed, Feature::kConstantSpeed },
        { ControlModeEnum::kConstantPressure, Feature::kConstantPressure },
        { ControlModeEnum::kProportionalPressure, Feature::kCompensatedPressure },
        { ControlModeEnum::kConstantFlow, Feature::kConstantFlow },
        { ControlModeEnum::kConstantTemperature, Feature::kConstantTemperature },
        { ControlModeEnum::kAutomatic, Feature::kAutomatic },
    };

    for (const auto & entry : kModeFeatures)
    {
        {
            PumpConfigurationAndControlCluster::Config config;
            config.optionalAttributes.controlMode = true;
            config.controlMode                    = ControlModeEnum::kAutomatic;
            PumpConfigurationAndControlCluster cluster(kTestEndpointId, config);
            ClusterTester tester(cluster);
            ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));
            EXPECT_EQ(tester.WriteAttribute(ControlMode::Id, entry.mode), Status::ConstraintError);
            cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
        }
        {
            PumpConfigurationAndControlCluster::Config config = ConfigWithFeatures(BitFlags<Feature>(entry.feature));
            config.optionalAttributes.controlMode             = true;
            PumpConfigurationAndControlCluster cluster(kTestEndpointId, config);
            ClusterTester tester(cluster);
            ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));
            EXPECT_EQ(tester.WriteAttribute(ControlMode::Id, entry.mode), CHIP_NO_ERROR);
            EXPECT_EQ(cluster.GetControlMode(), entry.mode);
            cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
        }
    }
}

TEST_F(TestPumpConfigurationAndControlCluster, RejectsUndefinedControlModeWithinAttributeRange)
{
    // ControlModeEnum has gaps: 4 and 6 are within the attribute range (max 7) but are not defined.
    PumpConfigurationAndControlCluster::Config config = ConfigWithFeatures(
        BitFlags<Feature>(Feature::kConstantPressure, Feature::kCompensatedPressure, Feature::kConstantFlow,
                          Feature::kConstantSpeed, Feature::kConstantTemperature, Feature::kAutomatic, Feature::kLocalOperation));
    config.optionalAttributes.controlMode = true;
    PumpConfigurationAndControlCluster cluster(kTestEndpointId, config);
    ClusterTester tester(cluster);
    ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));

    EXPECT_EQ(tester.WriteAttribute(ControlMode::Id, static_cast<uint8_t>(4)), Status::ConstraintError);
    EXPECT_EQ(tester.WriteAttribute(ControlMode::Id, static_cast<uint8_t>(6)), Status::ConstraintError);
    EXPECT_EQ(cluster.GetControlMode(), ControlModeEnum::kConstantSpeed);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestPumpConfigurationAndControlCluster, EffectiveModesFollowOperationMode)
{
    TestDelegate delegate;
    PumpConfigurationAndControlCluster::Config config =
        ConfigWithFeatures(BitFlags<Feature>(Feature::kConstantSpeed, Feature::kConstantFlow, Feature::kLocalOperation));
    config.optionalAttributes.controlMode = true;
    config.controlMode                    = ControlModeEnum::kConstantFlow;
    config.delegate                       = &delegate;

    PumpConfigurationAndControlCluster cluster(kTestEndpointId, config);
    ClusterTester tester(cluster);
    ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));
    EXPECT_EQ(cluster.GetEffectiveOperationMode(), OperationModeEnum::kNormal);
    EXPECT_EQ(cluster.GetEffectiveControlMode(), ControlModeEnum::kConstantFlow);

    tester.GetDirtyList().clear();
    EXPECT_EQ(tester.WriteAttribute(OperationMode::Id, OperationModeEnum::kMaximum), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetEffectiveOperationMode(), OperationModeEnum::kMaximum);
    EXPECT_EQ(cluster.GetEffectiveControlMode(), ControlModeEnum::kConstantSpeed);
    EXPECT_TRUE(tester.IsAttributeDirty(OperationMode::Id));
    EXPECT_TRUE(tester.IsAttributeDirty(EffectiveOperationMode::Id));
    EXPECT_TRUE(tester.IsAttributeDirty(EffectiveControlMode::Id));
    ASSERT_EQ(delegate.mMinMaxCalls.size(), 1u);
    EXPECT_EQ(delegate.mMinMaxCalls[0], OperationModeEnum::kMaximum);

    EXPECT_EQ(tester.WriteAttribute(OperationMode::Id, OperationModeEnum::kMinimum), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetEffectiveOperationMode(), OperationModeEnum::kMinimum);
    EXPECT_EQ(cluster.GetEffectiveControlMode(), ControlModeEnum::kConstantSpeed);
    ASSERT_EQ(delegate.mMinMaxCalls.size(), 2u);
    EXPECT_EQ(delegate.mMinMaxCalls[1], OperationModeEnum::kMinimum);

    EXPECT_EQ(tester.WriteAttribute(OperationMode::Id, OperationModeEnum::kLocal), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetEffectiveOperationMode(), OperationModeEnum::kLocal);
    EXPECT_EQ(cluster.GetEffectiveControlMode(), ControlModeEnum::kConstantFlow);

    // The application owns EffectiveControlMode in Local mode.
    EXPECT_SUCCESS(cluster.SetEffectiveControlMode(ControlModeEnum::kConstantSpeed));
    EXPECT_EQ(cluster.GetEffectiveControlMode(), ControlModeEnum::kConstantSpeed);

    EXPECT_EQ(tester.WriteAttribute(OperationMode::Id, OperationModeEnum::kNormal), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetEffectiveOperationMode(), OperationModeEnum::kNormal);
    EXPECT_EQ(cluster.GetEffectiveControlMode(), ControlModeEnum::kConstantFlow);

    EXPECT_EQ(tester.WriteAttribute(ControlMode::Id, ControlModeEnum::kConstantSpeed), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetEffectiveControlMode(), ControlModeEnum::kConstantSpeed);
    EXPECT_EQ(delegate.mMinMaxCalls.size(), 2u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestPumpConfigurationAndControlCluster, RemoteSensorOverridesControlModeInNormalMode)
{
    TestDelegate delegate;
    delegate.mRemoteSensorType = RemoteSensorType::kPressureSensor;

    PumpConfigurationAndControlCluster::Config config =
        ConfigWithFeatures(BitFlags<Feature>(Feature::kConstantSpeed, Feature::kConstantPressure));
    config.optionalAttributes.controlMode = true;
    config.optionalAttributes.pumpStatus  = true;
    config.delegate                       = &delegate;

    PumpConfigurationAndControlCluster cluster(kTestEndpointId, config);
    ClusterTester tester(cluster);
    ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));
    EXPECT_SUCCESS(cluster.SetPumpStatus(PumpConfigurationAndControlCluster::PumpStatusBitMask(PumpStatusBitmap::kRunning)));

    EXPECT_EQ(tester.WriteAttribute(ControlMode::Id, ControlModeEnum::kConstantPressure), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetEffectiveControlMode(), ControlModeEnum::kConstantPressure);
    EXPECT_TRUE(cluster.GetPumpStatus().Has(PumpStatusBitmap::kRemotePressure));
    EXPECT_TRUE(cluster.GetPumpStatus().Has(PumpStatusBitmap::kRunning));

    delegate.mRemoteSensorType = RemoteSensorType::kFlowSensor;
    EXPECT_EQ(tester.WriteAttribute(ControlMode::Id, ControlModeEnum::kConstantSpeed), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetEffectiveControlMode(), ControlModeEnum::kConstantFlow);
    EXPECT_TRUE(cluster.GetPumpStatus().Has(PumpStatusBitmap::kRemoteFlow));
    EXPECT_FALSE(cluster.GetPumpStatus().Has(PumpStatusBitmap::kRemotePressure));

    // Remote sensors only apply while OperationMode is Normal.
    EXPECT_EQ(tester.WriteAttribute(OperationMode::Id, OperationModeEnum::kMaximum), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetEffectiveControlMode(), ControlModeEnum::kConstantSpeed);
    EXPECT_FALSE(cluster.GetPumpStatus().HasAny(PumpStatusBitmap::kRemoteFlow, PumpStatusBitmap::kRemotePressure,
                                                PumpStatusBitmap::kRemoteTemperature));
    EXPECT_TRUE(cluster.GetPumpStatus().Has(PumpStatusBitmap::kRunning));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestPumpConfigurationAndControlCluster, LifetimeAttributesWrite)
{
    PumpConfigurationAndControlCluster::Config config;
    config.optionalAttributes.lifetimeRunningHours   = true;
    config.optionalAttributes.lifetimeEnergyConsumed = true;
    PumpConfigurationAndControlCluster cluster(kTestEndpointId, config);
    ClusterTester tester(cluster);
    ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));

    DataModel::Nullable<uint32_t> value;
    ASSERT_SUCCESS(tester.ReadAttribute(LifetimeRunningHours::Id, value));
    EXPECT_EQ(value, DataModel::MakeNullable(static_cast<uint32_t>(0)));
    ASSERT_SUCCESS(tester.ReadAttribute(LifetimeEnergyConsumed::Id, value));
    EXPECT_EQ(value, DataModel::MakeNullable(static_cast<uint32_t>(0)));

    // LifetimeRunningHours is a nullable int24u: 0xFFFFFF is the null value.
    EXPECT_EQ(tester.WriteAttribute(LifetimeRunningHours::Id, DataModel::MakeNullable(static_cast<uint32_t>(0xFFFFFE))),
              CHIP_NO_ERROR);
    EXPECT_EQ(tester.WriteAttribute(LifetimeRunningHours::Id, DataModel::MakeNullable(static_cast<uint32_t>(0xFFFFFF))),
              Status::ConstraintError);
    EXPECT_EQ(cluster.GetLifetimeRunningHours(), DataModel::MakeNullable(static_cast<uint32_t>(0xFFFFFE)));
    EXPECT_EQ(tester.WriteAttribute(LifetimeRunningHours::Id, DataModel::Nullable<uint32_t>()), CHIP_NO_ERROR);
    EXPECT_TRUE(cluster.GetLifetimeRunningHours().IsNull());

    EXPECT_EQ(tester.WriteAttribute(LifetimeEnergyConsumed::Id, DataModel::MakeNullable(static_cast<uint32_t>(1234))),
              CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetLifetimeEnergyConsumed(), DataModel::MakeNullable(static_cast<uint32_t>(1234)));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestPumpConfigurationAndControlCluster, AttributesPersistAcrossRestart)
{
    PumpConfigurationAndControlCluster::Config config =
        ConfigWithFeatures(BitFlags<Feature>(Feature::kConstantSpeed, Feature::kConstantFlow, Feature::kLocalOperation));
    config.optionalAttributes.controlMode            = true;
    config.optionalAttributes.lifetimeRunningHours   = true;
    config.optionalAttributes.lifetimeEnergyConsumed = true;

    PumpConfigurationAndControlCluster firstCluster(kTestEndpointId, config);
    ClusterTester tester(firstCluster);
    ASSERT_SUCCESS(firstCluster.Startup(tester.GetServerClusterContext()));

    EXPECT_EQ(tester.WriteAttribute(ControlMode::Id, ControlModeEnum::kConstantFlow), CHIP_NO_ERROR);
    EXPECT_EQ(tester.WriteAttribute(OperationMode::Id, OperationModeEnum::kLocal), CHIP_NO_ERROR);
    EXPECT_SUCCESS(firstCluster.SetEffectiveControlMode(ControlModeEnum::kConstantSpeed));
    EXPECT_EQ(tester.WriteAttribute(LifetimeRunningHours::Id, DataModel::MakeNullable(static_cast<uint32_t>(100))), CHIP_NO_ERROR);
    EXPECT_EQ(tester.WriteAttribute(LifetimeEnergyConsumed::Id, DataModel::Nullable<uint32_t>()), CHIP_NO_ERROR);
    firstCluster.Shutdown(ClusterShutdownType::kClusterShutdown);

    PumpConfigurationAndControlCluster secondCluster(kTestEndpointId, config);
    ASSERT_SUCCESS(secondCluster.Startup(tester.GetServerClusterContext()));
    EXPECT_EQ(secondCluster.GetOperationMode(), OperationModeEnum::kLocal);
    EXPECT_EQ(secondCluster.GetControlMode(), ControlModeEnum::kConstantFlow);
    EXPECT_EQ(secondCluster.GetEffectiveOperationMode(), OperationModeEnum::kLocal);
    EXPECT_EQ(secondCluster.GetEffectiveControlMode(), ControlModeEnum::kConstantSpeed);
    EXPECT_EQ(secondCluster.GetLifetimeRunningHours(), DataModel::MakeNullable(static_cast<uint32_t>(100)));
    EXPECT_TRUE(secondCluster.GetLifetimeEnergyConsumed().IsNull());
    secondCluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestPumpConfigurationAndControlCluster, SettersValidateAndReportChanges)
{
    PumpConfigurationAndControlCluster::Config config;
    config.optionalAttributes.speed = true;
    config.optionalAttributes.power = true;
    PumpConfigurationAndControlCluster cluster(kTestEndpointId, config);
    ClusterTester tester(cluster);
    ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));

    tester.GetDirtyList().clear();
    EXPECT_SUCCESS(cluster.SetCapacity(DataModel::MakeNullable(static_cast<int16_t>(50))));
    EXPECT_TRUE(tester.IsAttributeDirty(Capacity::Id));
    tester.GetDirtyList().clear();
    EXPECT_SUCCESS(cluster.SetCapacity(DataModel::MakeNullable(static_cast<int16_t>(50))));
    EXPECT_TRUE(tester.GetDirtyList().empty());

    // 0x8000 is the null value of a nullable int16s and 0xFFFF the null value of a nullable int16u.
    EXPECT_EQ(cluster.SetCapacity(DataModel::MakeNullable(static_cast<int16_t>(INT16_MIN))),
              CHIP_IM_GLOBAL_STATUS(ConstraintError));
    EXPECT_EQ(cluster.SetSpeed(DataModel::MakeNullable(static_cast<uint16_t>(0xFFFF))), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    EXPECT_SUCCESS(cluster.SetSpeed(DataModel::MakeNullable(static_cast<uint16_t>(0xFFFE))));
    EXPECT_EQ(cluster.SetPower(DataModel::MakeNullable(static_cast<uint32_t>(0xFFFFFF))), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    EXPECT_SUCCESS(cluster.SetPower(DataModel::MakeNullable(static_cast<uint32_t>(0xFFFFFE))));

    DataModel::Nullable<uint32_t> power;
    ASSERT_SUCCESS(tester.ReadAttribute(Power::Id, power));
    EXPECT_EQ(power, DataModel::MakeNullable(static_cast<uint32_t>(0xFFFFFE)));

    // Setters are not allowed to bypass the feature checks.
    EXPECT_EQ(cluster.SetOperationMode(OperationModeEnum::kMaximum), CHIP_IM_GLOBAL_STATUS(ConstraintError));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestPumpConfigurationAndControlCluster, GenerateAlarmEvent)
{
    PumpConfigurationAndControlCluster cluster(kTestEndpointId, {});
    ClusterTester tester(cluster);

    EXPECT_FALSE(cluster.GenerateAlarmEvent(Events::GeneralFault::Type{}).has_value());

    ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));
    EXPECT_TRUE(cluster.GenerateAlarmEvent(Events::GeneralFault::Type{}).has_value());

    auto event = tester.GetNextGeneratedEvent();
    ASSERT_TRUE(event.has_value());
    EXPECT_EQ(event->eventOptions.mPath,
              ConcreteEventPath(kTestEndpointId, PumpConfigurationAndControl::Id, Events::GeneralFault::Id));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}
