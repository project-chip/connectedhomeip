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
#include <app/persistence/AttributePersistence.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/PumpConfigurationAndControl/Events.h>
#include <clusters/PumpConfigurationAndControl/Metadata.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <pw_unit_test/framework.h>

#include <algorithm>
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

using OptionalAttributes = PumpConfigurationAndControlCluster::OptionalAttributes;

class TestDelegate : public PumpConfigurationAndControl::Delegate
{
public:
    RemoteSensorType GetRemoteSensorType(EndpointId endpointId) override { return mRemoteSensorType; }
    void OnOperationModeMinMax(EndpointId endpointId, OperationModeEnum operationMode) override
    {
        mMinMaxCalls.push_back(operationMode);
    }
    void OnAttributeChanged(EndpointId endpointId, AttributeId attributeId) override { mChangedAttributes.push_back(attributeId); }

    bool WasChanged(AttributeId attributeId) const
    {
        return std::find(mChangedAttributes.begin(), mChangedAttributes.end(), attributeId) != mChangedAttributes.end();
    }

    RemoteSensorType mRemoteSensorType = RemoteSensorType::kNoSensor;
    std::vector<OperationModeEnum> mMinMaxCalls;
    std::vector<AttributeId> mChangedAttributes;
};

struct TestPumpConfigurationAndControlCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_SUCCESS(chip::Platform::MemoryInit()); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    PumpConfigurationAndControlCluster::Config ConfigWithFeatures(BitFlags<Feature> features = {})
    {
        PumpConfigurationAndControlCluster::Config config(mDelegate);
        config.features = features;
        return config;
    }

    TestDelegate mDelegate;
};

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
    PumpConfigurationAndControlCluster cluster(kTestEndpointId, ConfigWithFeatures());
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
    const BitMask<OptionalAttributes> requested(OptionalAttributes::kMinConstFlow, OptionalAttributes::kMaxConstFlow);

    {
        // Without the Automatic feature (or the Flow feature), the flow limits are disallowed.
        PumpConfigurationAndControlCluster::Config config = ConfigWithFeatures();
        config.optionalAttributes                         = requested;
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
    PumpConfigurationAndControlCluster::Config config = ConfigWithFeatures();
    config.optionalAttributes.Set(OptionalAttributes::kPumpStatus);
    config.optionalAttributes.Set(OptionalAttributes::kSpeed);
    config.optionalAttributes.Set(OptionalAttributes::kLifetimeRunningHours);
    config.optionalAttributes.Set(OptionalAttributes::kPower);
    config.optionalAttributes.Set(OptionalAttributes::kLifetimeEnergyConsumed);
    config.optionalAttributes.Set(OptionalAttributes::kControlMode);

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

TEST_F(TestPumpConfigurationAndControlCluster, SpecFallbackValues)
{
    // Every attribute enabled, nothing configured and nothing persisted: reads return the
    // Fallback column of the spec attribute table.
    PumpConfigurationAndControlCluster::Config config = ConfigWithFeatures(
        BitFlags<Feature>(Feature::kConstantPressure, Feature::kCompensatedPressure, Feature::kConstantFlow,
                          Feature::kConstantSpeed, Feature::kConstantTemperature, Feature::kAutomatic, Feature::kLocalOperation));
    config.optionalAttributes.Set(OptionalAttributes::kPumpStatus);
    config.optionalAttributes.Set(OptionalAttributes::kSpeed);
    config.optionalAttributes.Set(OptionalAttributes::kLifetimeRunningHours);
    config.optionalAttributes.Set(OptionalAttributes::kPower);
    config.optionalAttributes.Set(OptionalAttributes::kLifetimeEnergyConsumed);
    config.optionalAttributes.Set(OptionalAttributes::kControlMode);

    PumpConfigurationAndControlCluster cluster(kTestEndpointId, config);
    ClusterTester tester(cluster);
    ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));

    // Fallback null. The read targets start non-null so that a read which does not update them fails.
    for (AttributeId id : { MaxPressure::Id, MinConstPressure::Id, MaxConstPressure::Id, MinCompPressure::Id, MaxCompPressure::Id,
                            MinConstTemp::Id, MaxConstTemp::Id, Capacity::Id })
    {
        DataModel::Nullable<int16_t> value = DataModel::MakeNullable(static_cast<int16_t>(1));
        ASSERT_SUCCESS(tester.ReadAttribute(id, value));
        EXPECT_TRUE(value.IsNull()) << "attribute " << id;
    }
    for (AttributeId id :
         { MaxSpeed::Id, MaxFlow::Id, MinConstSpeed::Id, MaxConstSpeed::Id, MinConstFlow::Id, MaxConstFlow::Id, Speed::Id })
    {
        DataModel::Nullable<uint16_t> value = DataModel::MakeNullable(static_cast<uint16_t>(1));
        ASSERT_SUCCESS(tester.ReadAttribute(id, value));
        EXPECT_TRUE(value.IsNull()) << "attribute " << id;
    }
    {
        DataModel::Nullable<uint32_t> power = DataModel::MakeNullable(static_cast<uint32_t>(1));
        ASSERT_SUCCESS(tester.ReadAttribute(Power::Id, power));
        EXPECT_TRUE(power.IsNull());
    }

    // Fallback 0.
    PumpConfigurationAndControlCluster::PumpStatusBitMask pumpStatus(static_cast<uint16_t>(0xFFFF));
    ASSERT_SUCCESS(tester.ReadAttribute(PumpStatus::Id, pumpStatus));
    EXPECT_EQ(pumpStatus.Raw(), 0u);

    for (AttributeId id : { LifetimeRunningHours::Id, LifetimeEnergyConsumed::Id })
    {
        DataModel::Nullable<uint32_t> value;
        ASSERT_SUCCESS(tester.ReadAttribute(id, value));
        EXPECT_EQ(value, DataModel::MakeNullable(static_cast<uint32_t>(0))) << "attribute " << id;
    }

    OperationModeEnum operationMode = OperationModeEnum::kLocal;
    ASSERT_SUCCESS(tester.ReadAttribute(OperationMode::Id, operationMode));
    EXPECT_EQ(operationMode, OperationModeEnum::kNormal);

    ControlModeEnum controlMode = ControlModeEnum::kAutomatic;
    ASSERT_SUCCESS(tester.ReadAttribute(ControlMode::Id, controlMode));
    EXPECT_EQ(controlMode, ControlModeEnum::kConstantSpeed);

    // The Effective* fallbacks are "desc": they follow OperationMode Normal and ControlMode ConstantSpeed.
    operationMode = OperationModeEnum::kLocal;
    ASSERT_SUCCESS(tester.ReadAttribute(EffectiveOperationMode::Id, operationMode));
    EXPECT_EQ(operationMode, OperationModeEnum::kNormal);

    controlMode = ControlModeEnum::kAutomatic;
    ASSERT_SUCCESS(tester.ReadAttribute(EffectiveControlMode::Id, controlMode));
    EXPECT_EQ(controlMode, ControlModeEnum::kConstantSpeed);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestPumpConfigurationAndControlCluster, OperationModeRequiresFeature)
{
    {
        PumpConfigurationAndControlCluster cluster(kTestEndpointId, ConfigWithFeatures());
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
            PumpConfigurationAndControlCluster::Config config = ConfigWithFeatures();
            config.optionalAttributes.Set(OptionalAttributes::kControlMode);
            config.controlMode = ControlModeEnum::kAutomatic;
            PumpConfigurationAndControlCluster cluster(kTestEndpointId, config);
            ClusterTester tester(cluster);
            ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));
            EXPECT_EQ(tester.WriteAttribute(ControlMode::Id, entry.mode), Status::ConstraintError);
            cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
        }
        {
            PumpConfigurationAndControlCluster::Config config = ConfigWithFeatures(BitFlags<Feature>(entry.feature));
            config.optionalAttributes.Set(OptionalAttributes::kControlMode);
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
    config.optionalAttributes.Set(OptionalAttributes::kControlMode);
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
    TestDelegate & delegate = mDelegate;
    PumpConfigurationAndControlCluster::Config config =
        ConfigWithFeatures(BitFlags<Feature>(Feature::kConstantSpeed, Feature::kConstantFlow, Feature::kLocalOperation));
    config.optionalAttributes.Set(OptionalAttributes::kControlMode);
    config.controlMode = ControlModeEnum::kConstantFlow;

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
    TestDelegate & delegate    = mDelegate;
    delegate.mRemoteSensorType = RemoteSensorType::kPressureSensor;

    PumpConfigurationAndControlCluster::Config config =
        ConfigWithFeatures(BitFlags<Feature>(Feature::kConstantSpeed, Feature::kConstantPressure));
    config.optionalAttributes.Set(OptionalAttributes::kControlMode);
    config.optionalAttributes.Set(OptionalAttributes::kPumpStatus);

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
    PumpConfigurationAndControlCluster::Config config = ConfigWithFeatures();
    config.optionalAttributes.Set(OptionalAttributes::kLifetimeRunningHours);
    config.optionalAttributes.Set(OptionalAttributes::kLifetimeEnergyConsumed);
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
    config.optionalAttributes.Set(OptionalAttributes::kControlMode);
    config.optionalAttributes.Set(OptionalAttributes::kLifetimeRunningHours);
    config.optionalAttributes.Set(OptionalAttributes::kLifetimeEnergyConsumed);

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
    PumpConfigurationAndControlCluster::Config config = ConfigWithFeatures();
    config.optionalAttributes.Set(OptionalAttributes::kSpeed);
    config.optionalAttributes.Set(OptionalAttributes::kPower);
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
    PumpConfigurationAndControlCluster cluster(kTestEndpointId, ConfigWithFeatures());
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

TEST_F(TestPumpConfigurationAndControlCluster, OnRemoteSensorChangedOnlyAppliesInNormalMode)
{
    PumpConfigurationAndControlCluster::Config config =
        ConfigWithFeatures(BitFlags<Feature>(Feature::kConstantSpeed, Feature::kConstantPressure, Feature::kLocalOperation));
    config.optionalAttributes.Set(OptionalAttributes::kPumpStatus).Set(OptionalAttributes::kControlMode);

    PumpConfigurationAndControlCluster cluster(kTestEndpointId, config);
    ClusterTester tester(cluster);
    ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));

    mDelegate.mRemoteSensorType = RemoteSensorType::kPressureSensor;
    cluster.OnRemoteSensorChanged();
    EXPECT_EQ(cluster.GetEffectiveControlMode(), ControlModeEnum::kConstantPressure);
    EXPECT_TRUE(cluster.GetPumpStatus().Has(PumpStatusBitmap::kRemotePressure));

    mDelegate.mRemoteSensorType = RemoteSensorType::kNoSensor;
    cluster.OnRemoteSensorChanged();
    EXPECT_EQ(cluster.GetEffectiveControlMode(), ControlModeEnum::kConstantSpeed);
    EXPECT_FALSE(cluster.GetPumpStatus().Has(PumpStatusBitmap::kRemotePressure));

    // In Local mode, the EffectiveControlMode set by the application is kept.
    EXPECT_EQ(tester.WriteAttribute(OperationMode::Id, OperationModeEnum::kLocal), CHIP_NO_ERROR);
    EXPECT_SUCCESS(cluster.SetEffectiveControlMode(ControlModeEnum::kConstantPressure));
    mDelegate.mRemoteSensorType = RemoteSensorType::kPressureSensor;
    cluster.OnRemoteSensorChanged();
    EXPECT_EQ(cluster.GetEffectiveControlMode(), ControlModeEnum::kConstantPressure);
    EXPECT_FALSE(cluster.GetPumpStatus().Has(PumpStatusBitmap::kRemotePressure));

    // In Maximum mode, the Minimum/Maximum hook is not called again.
    EXPECT_EQ(tester.WriteAttribute(OperationMode::Id, OperationModeEnum::kMaximum), CHIP_NO_ERROR);
    ASSERT_EQ(mDelegate.mMinMaxCalls.size(), 1u);
    cluster.OnRemoteSensorChanged();
    EXPECT_EQ(mDelegate.mMinMaxCalls.size(), 1u);
    EXPECT_EQ(cluster.GetEffectiveControlMode(), ControlModeEnum::kConstantSpeed);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestPumpConfigurationAndControlCluster, DelegateNotifiedOfAttributeChanges)
{
    PumpConfigurationAndControlCluster::Config config =
        ConfigWithFeatures(BitFlags<Feature>(Feature::kConstantSpeed, Feature::kConstantFlow));
    config.optionalAttributes.Set(OptionalAttributes::kControlMode);
    config.controlMode = ControlModeEnum::kConstantFlow;

    PumpConfigurationAndControlCluster cluster(kTestEndpointId, config);
    ClusterTester tester(cluster);
    ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));

    mDelegate.mChangedAttributes.clear();
    EXPECT_EQ(tester.WriteAttribute(OperationMode::Id, OperationModeEnum::kMaximum), CHIP_NO_ERROR);
    EXPECT_TRUE(mDelegate.WasChanged(OperationMode::Id));
    EXPECT_TRUE(mDelegate.WasChanged(EffectiveOperationMode::Id));
    EXPECT_TRUE(mDelegate.WasChanged(EffectiveControlMode::Id));

    mDelegate.mChangedAttributes.clear();
    EXPECT_SUCCESS(cluster.SetCapacity(DataModel::MakeNullable(static_cast<int16_t>(5))));
    EXPECT_TRUE(mDelegate.WasChanged(Capacity::Id));

    // Setting the same value is not a change.
    mDelegate.mChangedAttributes.clear();
    EXPECT_SUCCESS(cluster.SetCapacity(DataModel::MakeNullable(static_cast<int16_t>(5))));
    EXPECT_EQ(tester.WriteAttribute(OperationMode::Id, OperationModeEnum::kMaximum), CHIP_NO_ERROR);
    EXPECT_TRUE(mDelegate.mChangedAttributes.empty());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestPumpConfigurationAndControlCluster, SettersRejectDisabledAttributes)
{
    PumpConfigurationAndControlCluster cluster(kTestEndpointId, ConfigWithFeatures(BitFlags<Feature>(Feature::kConstantSpeed)));
    ClusterTester tester(cluster);
    ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));

    const CHIP_ERROR kUnsupported = CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    EXPECT_EQ(cluster.SetPumpStatus(PumpConfigurationAndControlCluster::PumpStatusBitMask(PumpStatusBitmap::kRunning)),
              kUnsupported);
    EXPECT_EQ(cluster.SetSpeed(DataModel::MakeNullable(static_cast<uint16_t>(1))), kUnsupported);
    EXPECT_EQ(cluster.SetPower(DataModel::MakeNullable(static_cast<uint32_t>(1))), kUnsupported);
    EXPECT_EQ(cluster.SetLifetimeRunningHours(DataModel::MakeNullable(static_cast<uint32_t>(1))), kUnsupported);
    EXPECT_EQ(cluster.SetLifetimeEnergyConsumed(DataModel::MakeNullable(static_cast<uint32_t>(1))), kUnsupported);
    EXPECT_EQ(cluster.SetControlMode(ControlModeEnum::kConstantSpeed), kUnsupported);

    EXPECT_TRUE(cluster.IsAttributeEnabled(Capacity::Id));
    EXPECT_TRUE(cluster.IsAttributeEnabled(FeatureMap::Id));
    EXPECT_TRUE(cluster.IsAttributeEnabled(MinConstSpeed::Id));
    EXPECT_FALSE(cluster.IsAttributeEnabled(MinConstPressure::Id));
    EXPECT_FALSE(cluster.IsAttributeEnabled(Speed::Id));
    EXPECT_FALSE(cluster.IsAttributeEnabled(ControlMode::Id));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestPumpConfigurationAndControlCluster, PumpStatusRejectsUndefinedBits)
{
    PumpConfigurationAndControlCluster::Config config = ConfigWithFeatures(BitFlags<Feature>(Feature::kConstantSpeed));
    config.optionalAttributes.Set(OptionalAttributes::kPumpStatus);
    PumpConfigurationAndControlCluster cluster(kTestEndpointId, config);
    ClusterTester tester(cluster);
    ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));

    // PumpStatusBitmap defines bits 0 to 8.
    EXPECT_EQ(cluster.SetPumpStatus(PumpConfigurationAndControlCluster::PumpStatusBitMask(static_cast<uint16_t>(0x0200))),
              CHIP_IM_GLOBAL_STATUS(ConstraintError));
    EXPECT_SUCCESS(cluster.SetPumpStatus(PumpConfigurationAndControlCluster::PumpStatusBitMask(static_cast<uint16_t>(0x01FF))));
    EXPECT_EQ(cluster.GetPumpStatus().Raw(), 0x01FFu);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestPumpConfigurationAndControlCluster, ControlModeFallsBackToFirstSupportedMode)
{
    {
        // ControlMode disabled and no configured value: ConstantPressure is the only supported mode.
        PumpConfigurationAndControlCluster cluster(kTestEndpointId,
                                                   ConfigWithFeatures(BitFlags<Feature>(Feature::kConstantPressure)));
        ClusterTester tester(cluster);
        ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));
        EXPECT_EQ(cluster.GetControlMode(), ControlModeEnum::kConstantPressure);
        EXPECT_EQ(cluster.GetEffectiveControlMode(), ControlModeEnum::kConstantPressure);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
    {
        // A configured mode whose feature is not enabled is replaced.
        PumpConfigurationAndControlCluster::Config config =
            ConfigWithFeatures(BitFlags<Feature>(Feature::kConstantFlow, Feature::kAutomatic));
        config.controlMode = ControlModeEnum::kConstantSpeed;
        PumpConfigurationAndControlCluster cluster(kTestEndpointId, config);
        ClusterTester tester(cluster);
        ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));
        EXPECT_EQ(cluster.GetControlMode(), ControlModeEnum::kConstantFlow);
        EXPECT_EQ(cluster.GetEffectiveControlMode(), ControlModeEnum::kConstantFlow);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestPumpConfigurationAndControlCluster, LifetimeEnergyConsumedRejectsNullValue)
{
    PumpConfigurationAndControlCluster::Config config = ConfigWithFeatures(BitFlags<Feature>(Feature::kConstantSpeed));
    config.optionalAttributes.Set(OptionalAttributes::kLifetimeEnergyConsumed);
    PumpConfigurationAndControlCluster cluster(kTestEndpointId, config);
    ClusterTester tester(cluster);
    ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));

    // 0xFFFFFFFF is the null value of a nullable uint32.
    EXPECT_EQ(tester.WriteAttribute(LifetimeEnergyConsumed::Id, DataModel::MakeNullable(static_cast<uint32_t>(0xFFFFFFFF))),
              Status::ConstraintError);
    EXPECT_EQ(tester.WriteAttribute(LifetimeEnergyConsumed::Id, DataModel::MakeNullable(static_cast<uint32_t>(0xFFFFFFFE))),
              CHIP_NO_ERROR);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestPumpConfigurationAndControlCluster, StartupRecoversFromInvalidPersistedValues)
{
    PumpConfigurationAndControlCluster::Config config =
        ConfigWithFeatures(BitFlags<Feature>(Feature::kConstantSpeed, Feature::kConstantPressure));
    config.optionalAttributes.Set(OptionalAttributes::kControlMode).Set(OptionalAttributes::kLifetimeRunningHours);

    struct
    {
        uint8_t operationMode;
        uint8_t controlMode;
    } const kInvalidModes[] = {
        // Local and ConstantFlow are valid values, but their features are not enabled.
        { to_underlying(OperationModeEnum::kLocal), to_underlying(ControlModeEnum::kConstantFlow) },
        // Values outside of the enums.
        { 9, 6 },
    };

    for (const auto & modes : kInvalidModes)
    {
        PumpConfigurationAndControlCluster cluster(kTestEndpointId, config);
        ClusterTester tester(cluster);

        AttributePersistence persistence(tester.GetServerClusterContext().attributeStorage);
        ASSERT_SUCCESS(persistence.StoreNativeEndianValue({ kTestEndpointId, PumpConfigurationAndControl::Id, OperationMode::Id },
                                                          modes.operationMode));
        ASSERT_SUCCESS(persistence.StoreNativeEndianValue({ kTestEndpointId, PumpConfigurationAndControl::Id, ControlMode::Id },
                                                          modes.controlMode));
        // LifetimeRunningHours is a 24-bit value.
        ASSERT_SUCCESS(persistence.StoreNativeEndianValue(
            { kTestEndpointId, PumpConfigurationAndControl::Id, LifetimeRunningHours::Id }, static_cast<uint32_t>(0x01000000)));

        ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));
        EXPECT_EQ(cluster.GetOperationMode(), OperationModeEnum::kNormal);
        EXPECT_EQ(cluster.GetControlMode(), ControlModeEnum::kConstantSpeed);
        EXPECT_EQ(cluster.GetLifetimeRunningHours(), DataModel::MakeNullable(static_cast<uint32_t>(0)));
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}
