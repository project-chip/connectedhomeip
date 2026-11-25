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
#include <app/persistence/AttributePersistenceProvider.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/PumpConfigurationAndControl/Events.h>
#include <clusters/PumpConfigurationAndControl/Metadata.h>
#include <lib/support/odd-sized-integers.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <pw_unit_test/framework.h>
#include <system/RAIIMockClock.h>

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

TEST_F(TestPumpConfigurationAndControlCluster, SanitizesInvalidFixedAttributes)
{
    const BitFlags<Feature> allLimitFeatures(Feature::kConstantPressure, Feature::kCompensatedPressure, Feature::kConstantSpeed,
                                             Feature::kConstantFlow, Feature::kConstantTemperature);

    {
        // Reserved null sentinels (INT16_MIN for int16s, 0xFFFF for int16u) and temperatures below
        // -27315 (-273.15 °C) are replaced with null.
        PumpConfigurationAndControlCluster::Config config = ConfigWithFeatures(allLimitFeatures);
        config.fixedAttributes.maxPressure.SetNonNull(INT16_MIN);
        config.fixedAttributes.maxSpeed.SetNonNull(static_cast<uint16_t>(0xFFFF));
        config.fixedAttributes.maxFlow.SetNonNull(static_cast<uint16_t>(0xFFFF));
        config.fixedAttributes.minConstPressure.SetNonNull(INT16_MIN);
        config.fixedAttributes.maxConstPressure.SetNonNull(INT16_MIN);
        config.fixedAttributes.minCompPressure.SetNonNull(INT16_MIN);
        config.fixedAttributes.maxCompPressure.SetNonNull(INT16_MIN);
        config.fixedAttributes.minConstSpeed.SetNonNull(static_cast<uint16_t>(0xFFFF));
        config.fixedAttributes.maxConstSpeed.SetNonNull(static_cast<uint16_t>(0xFFFF));
        config.fixedAttributes.minConstFlow.SetNonNull(static_cast<uint16_t>(0xFFFF));
        config.fixedAttributes.maxConstFlow.SetNonNull(static_cast<uint16_t>(0xFFFF));
        config.fixedAttributes.minConstTemp.SetNonNull(static_cast<int16_t>(-27316));
        config.fixedAttributes.maxConstTemp.SetNonNull(static_cast<int16_t>(-27316));

        PumpConfigurationAndControlCluster cluster(kTestEndpointId, config);
        ClusterTester tester(cluster);
        ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));

        for (AttributeId id : { MaxPressure::Id, MinConstPressure::Id, MaxConstPressure::Id, MinCompPressure::Id,
                                MaxCompPressure::Id, MinConstTemp::Id, MaxConstTemp::Id })
        {
            DataModel::Nullable<int16_t> value = DataModel::MakeNullable(static_cast<int16_t>(0));
            ASSERT_SUCCESS(tester.ReadAttribute(id, value));
            EXPECT_TRUE(value.IsNull()) << "attribute " << id;
        }
        for (AttributeId id :
             { MaxSpeed::Id, MaxFlow::Id, MinConstSpeed::Id, MaxConstSpeed::Id, MinConstFlow::Id, MaxConstFlow::Id })
        {
            DataModel::Nullable<uint16_t> value = DataModel::MakeNullable(static_cast<uint16_t>(0));
            ASSERT_SUCCESS(tester.ReadAttribute(id, value));
            EXPECT_TRUE(value.IsNull()) << "attribute " << id;
        }

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        // Valid boundary values (including -27315 and MaxConstTemp == MinConstTemp) are kept.
        PumpConfigurationAndControlCluster::Config config = ConfigWithFeatures(allLimitFeatures);
        config.fixedAttributes.maxPressure.SetNonNull(static_cast<int16_t>(-32767));
        config.fixedAttributes.maxSpeed.SetNonNull(static_cast<uint16_t>(0xFFFE));
        config.fixedAttributes.minConstTemp.SetNonNull(static_cast<int16_t>(-27315));
        config.fixedAttributes.maxConstTemp.SetNonNull(static_cast<int16_t>(-27315));

        PumpConfigurationAndControlCluster cluster(kTestEndpointId, config);
        EXPECT_EQ(cluster.GetFixedAttributes().maxPressure, DataModel::MakeNullable(static_cast<int16_t>(-32767)));
        EXPECT_EQ(cluster.GetFixedAttributes().maxSpeed, DataModel::MakeNullable(static_cast<uint16_t>(0xFFFE)));
        EXPECT_EQ(cluster.GetFixedAttributes().minConstTemp, DataModel::MakeNullable(static_cast<int16_t>(-27315)));
        EXPECT_EQ(cluster.GetFixedAttributes().maxConstTemp, DataModel::MakeNullable(static_cast<int16_t>(-27315)));
    }

    {
        // MaxConstTemp < MinConstTemp invalidates MaxConstTemp while keeping a valid MinConstTemp.
        PumpConfigurationAndControlCluster::Config config = ConfigWithFeatures(BitFlags<Feature>(Feature::kConstantTemperature));
        config.fixedAttributes.minConstTemp.SetNonNull(static_cast<int16_t>(2000));
        config.fixedAttributes.maxConstTemp.SetNonNull(static_cast<int16_t>(1999));

        PumpConfigurationAndControlCluster cluster(kTestEndpointId, config);
        EXPECT_EQ(cluster.GetFixedAttributes().minConstTemp, DataModel::MakeNullable(static_cast<int16_t>(2000)));
        EXPECT_TRUE(cluster.GetFixedAttributes().maxConstTemp.IsNull());
    }
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

TEST_F(TestPumpConfigurationAndControlCluster, MinMaxActionOnlyOnOperationModeChange)
{
    // In Minimum and Maximum, ControlMode does not affect how the pump runs. A ControlMode write
    // must not repeat the action, which would overwrite a level set in between.
    PumpConfigurationAndControlCluster::Config config =
        ConfigWithFeatures(BitFlags<Feature>(Feature::kConstantSpeed, Feature::kConstantFlow));
    config.optionalAttributes.Set(OptionalAttributes::kControlMode);

    PumpConfigurationAndControlCluster cluster(kTestEndpointId, config);
    ClusterTester tester(cluster);
    ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));

    EXPECT_EQ(tester.WriteAttribute(OperationMode::Id, OperationModeEnum::kMinimum), CHIP_NO_ERROR);
    ASSERT_EQ(mDelegate.mMinMaxCalls.size(), 1u);

    mDelegate.mChangedAttributes.clear();
    EXPECT_EQ(tester.WriteAttribute(ControlMode::Id, ControlModeEnum::kConstantFlow), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetEffectiveControlMode(), ControlModeEnum::kConstantSpeed);
    EXPECT_EQ(mDelegate.mMinMaxCalls.size(), 1u);
    EXPECT_TRUE(mDelegate.WasChanged(ControlMode::Id));
    EXPECT_FALSE(mDelegate.WasChanged(EffectiveControlMode::Id));

    EXPECT_EQ(tester.WriteAttribute(OperationMode::Id, OperationModeEnum::kMaximum), CHIP_NO_ERROR);
    ASSERT_EQ(mDelegate.mMinMaxCalls.size(), 2u);
    EXPECT_EQ(mDelegate.mMinMaxCalls[1], OperationModeEnum::kMaximum);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestPumpConfigurationAndControlCluster, RemoteSensorOverridesControlModeInNormalMode)
{
    TestDelegate & delegate    = mDelegate;
    delegate.mRemoteSensorType = RemoteSensorType::kPressureSensor;

    PumpConfigurationAndControlCluster::Config config = ConfigWithFeatures(BitFlags<Feature>(
        Feature::kConstantSpeed, Feature::kConstantPressure, Feature::kConstantFlow, Feature::kConstantTemperature));
    config.optionalAttributes.Set(OptionalAttributes::kControlMode);
    config.optionalAttributes.Set(OptionalAttributes::kPumpStatus);

    PumpConfigurationAndControlCluster cluster(kTestEndpointId, config);
    ClusterTester tester(cluster);
    ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));
    EXPECT_SUCCESS(cluster.SetPumpStatus(PumpConfigurationAndControlCluster::PumpStatusBitMask(PumpStatusBitmap::kRunning)));
    EXPECT_TRUE(cluster.GetPumpStatus().Has(PumpStatusBitmap::kRemotePressure));
    EXPECT_TRUE(cluster.GetPumpStatus().Has(PumpStatusBitmap::kRunning));

    EXPECT_EQ(tester.WriteAttribute(ControlMode::Id, ControlModeEnum::kConstantPressure), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetEffectiveControlMode(), ControlModeEnum::kConstantPressure);
    EXPECT_TRUE(cluster.GetPumpStatus().Has(PumpStatusBitmap::kRemotePressure));
    EXPECT_TRUE(cluster.GetPumpStatus().Has(PumpStatusBitmap::kRunning));

    delegate.mRemoteSensorType = RemoteSensorType::kFlowSensor;
    EXPECT_EQ(tester.WriteAttribute(ControlMode::Id, ControlModeEnum::kConstantSpeed), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetEffectiveControlMode(), ControlModeEnum::kConstantFlow);
    EXPECT_TRUE(cluster.GetPumpStatus().Has(PumpStatusBitmap::kRemoteFlow));
    EXPECT_FALSE(cluster.GetPumpStatus().Has(PumpStatusBitmap::kRemotePressure));

    delegate.mRemoteSensorType = RemoteSensorType::kTemperatureSensor;
    cluster.OnRemoteSensorChanged();
    EXPECT_EQ(cluster.GetEffectiveControlMode(), ControlModeEnum::kConstantTemperature);
    EXPECT_TRUE(cluster.GetPumpStatus().Has(PumpStatusBitmap::kRemoteTemperature));
    EXPECT_FALSE(cluster.GetPumpStatus().Has(PumpStatusBitmap::kRemoteFlow));

    // Remote sensors only apply while OperationMode is Normal.
    EXPECT_EQ(tester.WriteAttribute(OperationMode::Id, OperationModeEnum::kMaximum), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetEffectiveControlMode(), ControlModeEnum::kConstantSpeed);
    EXPECT_FALSE(cluster.GetPumpStatus().HasAny(PumpStatusBitmap::kRemoteFlow, PumpStatusBitmap::kRemotePressure,
                                                PumpStatusBitmap::kRemoteTemperature));
    EXPECT_TRUE(cluster.GetPumpStatus().Has(PumpStatusBitmap::kRunning));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestPumpConfigurationAndControlCluster, RemoteSensorRequiresFeature)
{
    PumpConfigurationAndControlCluster::Config config = ConfigWithFeatures(BitFlags<Feature>(Feature::kConstantSpeed));
    config.optionalAttributes.Set(OptionalAttributes::kPumpStatus);

    mDelegate.mRemoteSensorType = RemoteSensorType::kPressureSensor;
    PumpConfigurationAndControlCluster cluster(kTestEndpointId, config);
    ClusterTester tester(cluster);
    ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));
    EXPECT_EQ(cluster.GetEffectiveControlMode(), ControlModeEnum::kConstantSpeed);
    EXPECT_FALSE(cluster.GetPumpStatus().HasAny(PumpStatusBitmap::kRemoteFlow, PumpStatusBitmap::kRemotePressure,
                                                PumpStatusBitmap::kRemoteTemperature));

    for (RemoteSensorType sensor :
         { RemoteSensorType::kPressureSensor, RemoteSensorType::kFlowSensor, RemoteSensorType::kTemperatureSensor })
    {
        mDelegate.mRemoteSensorType = sensor;
        cluster.OnRemoteSensorChanged();
        EXPECT_EQ(cluster.GetEffectiveControlMode(), ControlModeEnum::kConstantSpeed);
        EXPECT_FALSE(cluster.GetPumpStatus().HasAny(PumpStatusBitmap::kRemoteFlow, PumpStatusBitmap::kRemotePressure,
                                                    PumpStatusBitmap::kRemoteTemperature));
    }

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
    // The EffectiveControlMode override is not restored: Local mode starts from ControlMode.
    EXPECT_EQ(secondCluster.GetEffectiveControlMode(), ControlModeEnum::kConstantFlow);
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

TEST_F(TestPumpConfigurationAndControlCluster, QuieterReportingForCapacitySpeedAndPower)
{
    System::Clock::Internal::RAIIMockClock mockClock;
    mockClock.SetMonotonic(System::Clock::Milliseconds64(1000));

    PumpConfigurationAndControlCluster::Config config = ConfigWithFeatures(BitFlags<Feature>(Feature::kConstantSpeed));
    config.optionalAttributes.Set(OptionalAttributes::kSpeed).Set(OptionalAttributes::kPower);
    PumpConfigurationAndControlCluster cluster(kTestEndpointId, config);
    ClusterTester tester(cluster);
    ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));

    auto verifyQuietAttribute = [&](AttributeId id, auto setValue, auto getValue, auto first, auto second, auto third) {
        using NullableType = DataModel::Nullable<decltype(first)>;

        // Null -> non-null is immediately reportable.
        tester.GetDirtyList().clear();
        tester.GetTestContext().ChangeListener().ChangedList().clear();
        mDelegate.mChangedAttributes.clear();
        EXPECT_SUCCESS(setValue(DataModel::MakeNullable(first)));
        EXPECT_EQ(getValue(), DataModel::MakeNullable(first));
        EXPECT_TRUE(tester.IsAttributeDirty(id)) << "attribute " << id;
        EXPECT_TRUE(mDelegate.WasChanged(id)) << "attribute " << id;

        // Within 1 second, non-null changes update the value and notify quietly without marking dirty.
        mockClock.AdvanceMonotonic(System::Clock::Milliseconds64(999));
        tester.GetDirtyList().clear();
        tester.GetTestContext().ChangeListener().ChangedList().clear();
        mDelegate.mChangedAttributes.clear();
        EXPECT_SUCCESS(setValue(DataModel::MakeNullable(second)));
        EXPECT_EQ(getValue(), DataModel::MakeNullable(second));
        EXPECT_FALSE(tester.IsAttributeDirty(id)) << "attribute " << id;
        EXPECT_TRUE(tester.GetTestContext().ChangeListener().IsChanged({ kTestEndpointId, PumpConfigurationAndControl::Id, id }))
            << "attribute " << id;
        EXPECT_TRUE(mDelegate.WasChanged(id)) << "attribute " << id;

        // Once 1 second has elapsed since the last reportable change, a new value is reportable.
        mockClock.AdvanceMonotonic(System::Clock::Milliseconds64(1));
        tester.GetDirtyList().clear();
        EXPECT_SUCCESS(setValue(DataModel::MakeNullable(third)));
        EXPECT_EQ(getValue(), DataModel::MakeNullable(third));
        EXPECT_TRUE(tester.IsAttributeDirty(id)) << "attribute " << id;

        // Non-null -> null and null -> non-null are reportable even within 1 second.
        mockClock.AdvanceMonotonic(System::Clock::Milliseconds64(100));
        tester.GetDirtyList().clear();
        EXPECT_SUCCESS(setValue(NullableType()));
        EXPECT_TRUE(getValue().IsNull()) << "attribute " << id;
        EXPECT_TRUE(tester.IsAttributeDirty(id)) << "attribute " << id;

        mockClock.AdvanceMonotonic(System::Clock::Milliseconds64(100));
        tester.GetDirtyList().clear();
        EXPECT_SUCCESS(setValue(DataModel::MakeNullable(first)));
        EXPECT_TRUE(tester.IsAttributeDirty(id)) << "attribute " << id;
    };

    verifyQuietAttribute(
        Capacity::Id, [&](auto v) { return cluster.SetCapacity(v); }, [&]() { return cluster.GetCapacity(); },
        static_cast<int16_t>(10), static_cast<int16_t>(20), static_cast<int16_t>(30));
    verifyQuietAttribute(
        Speed::Id, [&](auto v) { return cluster.SetSpeed(v); }, [&]() { return cluster.GetSpeed(); }, static_cast<uint16_t>(100),
        static_cast<uint16_t>(200), static_cast<uint16_t>(300));
    verifyQuietAttribute(
        Power::Id, [&](auto v) { return cluster.SetPower(v); }, [&]() { return cluster.GetPower(); }, static_cast<uint32_t>(1000),
        static_cast<uint32_t>(2000), static_cast<uint32_t>(3000));

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

    // PumpStatusBitmap defines bits 0 to 8; bits 6 to 8 are remote sensor bits owned by the cluster.
    EXPECT_EQ(cluster.SetPumpStatus(PumpConfigurationAndControlCluster::PumpStatusBitMask(static_cast<uint16_t>(0x0200))),
              CHIP_IM_GLOBAL_STATUS(ConstraintError));
    EXPECT_SUCCESS(cluster.SetPumpStatus(PumpConfigurationAndControlCluster::PumpStatusBitMask(static_cast<uint16_t>(0x01FF))));
    EXPECT_EQ(cluster.GetPumpStatus().Raw(), 0x003Fu);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestPumpConfigurationAndControlCluster, PumpStatusPreservesClusterOwnedRemoteSensorBits)
{
    mDelegate.mRemoteSensorType = RemoteSensorType::kPressureSensor;

    PumpConfigurationAndControlCluster::Config config =
        ConfigWithFeatures(BitFlags<Feature>(Feature::kConstantSpeed, Feature::kConstantPressure));
    config.optionalAttributes.Set(OptionalAttributes::kPumpStatus);
    PumpConfigurationAndControlCluster cluster(kTestEndpointId, config);
    ClusterTester tester(cluster);
    ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));
    ASSERT_EQ(cluster.GetPumpStatus().Raw(), to_underlying(PumpStatusBitmap::kRemotePressure));

    // Setting an application bit without reading first preserves RemotePressure.
    EXPECT_SUCCESS(cluster.SetPumpStatus(PumpConfigurationAndControlCluster::PumpStatusBitMask(PumpStatusBitmap::kRunning)));
    EXPECT_EQ(cluster.GetPumpStatus(),
              PumpConfigurationAndControlCluster::PumpStatusBitMask(PumpStatusBitmap::kRemotePressure, PumpStatusBitmap::kRunning));

    // Read-modify-write also preserves RemotePressure.
    PumpConfigurationAndControlCluster::PumpStatusBitMask status = cluster.GetPumpStatus();
    status.Set(PumpStatusBitmap::kDeviceFault);
    EXPECT_SUCCESS(cluster.SetPumpStatus(status));
    EXPECT_EQ(cluster.GetPumpStatus(),
              PumpConfigurationAndControlCluster::PumpStatusBitMask(PumpStatusBitmap::kRemotePressure, PumpStatusBitmap::kRunning,
                                                                    PumpStatusBitmap::kDeviceFault));

    // Remote sensor bits passed to SetPumpStatus are ignored.
    tester.GetDirtyList().clear();
    mDelegate.mChangedAttributes.clear();
    status.Clear(PumpStatusBitmap::kRemotePressure).Set(PumpStatusBitmap::kRemoteFlow);
    EXPECT_SUCCESS(cluster.SetPumpStatus(status));
    EXPECT_EQ(cluster.GetPumpStatus(),
              PumpConfigurationAndControlCluster::PumpStatusBitMask(PumpStatusBitmap::kRemotePressure, PumpStatusBitmap::kRunning,
                                                                    PumpStatusBitmap::kDeviceFault));
    EXPECT_TRUE(tester.GetDirtyList().empty());
    EXPECT_TRUE(mDelegate.mChangedAttributes.empty());

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
        ASSERT_SUCCESS(persistence.StoreNativeEndianValue(
            { kTestEndpointId, PumpConfigurationAndControl::Id, EffectiveOperationMode::Id }, modes.operationMode));
        ASSERT_SUCCESS(persistence.StoreNativeEndianValue(
            { kTestEndpointId, PumpConfigurationAndControl::Id, EffectiveControlMode::Id }, modes.controlMode));
        // LifetimeRunningHours is a 24-bit value.
        ASSERT_SUCCESS(persistence.StoreNativeEndianValue(
            { kTestEndpointId, PumpConfigurationAndControl::Id, LifetimeRunningHours::Id }, static_cast<uint32_t>(0x01000000)));

        ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));
        EXPECT_EQ(cluster.GetOperationMode(), OperationModeEnum::kNormal);
        EXPECT_EQ(cluster.GetControlMode(), ControlModeEnum::kConstantSpeed);
        EXPECT_EQ(cluster.GetEffectiveOperationMode(), OperationModeEnum::kNormal);
        EXPECT_EQ(cluster.GetEffectiveControlMode(), ControlModeEnum::kConstantSpeed);
        EXPECT_EQ(cluster.GetLifetimeRunningHours(), DataModel::MakeNullable(static_cast<uint32_t>(0)));
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestPumpConfigurationAndControlCluster, StartupQueriesRemoteSensorInNormalMode)
{
    // A delegate that reports a sensor before Startup (e.g. an ApplicationDelegate installed before
    // the cluster is created) is applied at startup, without any delegate notification.
    mDelegate.mRemoteSensorType = RemoteSensorType::kPressureSensor;

    PumpConfigurationAndControlCluster::Config config =
        ConfigWithFeatures(BitFlags<Feature>(Feature::kConstantSpeed, Feature::kConstantPressure));
    config.optionalAttributes.Set(OptionalAttributes::kPumpStatus);

    PumpConfigurationAndControlCluster cluster(kTestEndpointId, config);
    ClusterTester tester(cluster);
    ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));

    EXPECT_EQ(cluster.GetEffectiveControlMode(), ControlModeEnum::kConstantPressure);
    EXPECT_TRUE(cluster.GetPumpStatus().Has(PumpStatusBitmap::kRemotePressure));
    EXPECT_TRUE(mDelegate.mChangedAttributes.empty());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestPumpConfigurationAndControlCluster, StartupRebuildsEffectiveModesFromRestoredModes)
{
    PumpConfigurationAndControlCluster::Config config =
        ConfigWithFeatures(BitFlags<Feature>(Feature::kConstantSpeed, Feature::kConstantFlow, Feature::kLocalOperation));
    config.optionalAttributes.Set(OptionalAttributes::kControlMode);

    auto store = [](ClusterTester & tester, AttributeId attributeId, uint8_t value) {
        AttributePersistence persistence(tester.GetServerClusterContext().attributeStorage);
        return persistence.StoreNativeEndianValue({ kTestEndpointId, PumpConfigurationAndControl::Id, attributeId }, value);
    };

    {
        // Interrupted update: OperationMode was stored as Maximum, the effective modes were not.
        PumpConfigurationAndControlCluster cluster(kTestEndpointId, config);
        ClusterTester tester(cluster);
        ASSERT_SUCCESS(store(tester, OperationMode::Id, to_underlying(OperationModeEnum::kMaximum)));
        ASSERT_SUCCESS(store(tester, EffectiveOperationMode::Id, to_underlying(OperationModeEnum::kNormal)));
        ASSERT_SUCCESS(store(tester, EffectiveControlMode::Id, to_underlying(ControlModeEnum::kConstantFlow)));

        ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));
        EXPECT_EQ(cluster.GetEffectiveOperationMode(), OperationModeEnum::kMaximum);
        EXPECT_EQ(cluster.GetEffectiveControlMode(), ControlModeEnum::kConstantSpeed);
        // Startup does not repeat the Minimum/Maximum action.
        EXPECT_TRUE(mDelegate.mMinMaxCalls.empty());
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        // A stored EffectiveControlMode (e.g. written by the Ember implementation with NVM storage)
        // is ignored: Local mode starts from ControlMode.
        PumpConfigurationAndControlCluster cluster(kTestEndpointId, config);
        ClusterTester tester(cluster);
        ASSERT_SUCCESS(store(tester, OperationMode::Id, to_underlying(OperationModeEnum::kLocal)));
        ASSERT_SUCCESS(store(tester, EffectiveControlMode::Id, to_underlying(ControlModeEnum::kConstantFlow)));

        ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));
        EXPECT_EQ(cluster.GetEffectiveOperationMode(), OperationModeEnum::kLocal);
        EXPECT_EQ(cluster.GetEffectiveControlMode(), ControlModeEnum::kConstantSpeed);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestPumpConfigurationAndControlCluster, UnsupportedConfiguredOperationModeFallsBackToNormal)
{
    PumpConfigurationAndControlCluster::Config config = ConfigWithFeatures(BitFlags<Feature>(Feature::kConstantFlow));
    config.operationMode                              = OperationModeEnum::kMaximum;
    PumpConfigurationAndControlCluster cluster(kTestEndpointId, config);

    // Before Startup, the state already follows the enabled features.
    EXPECT_EQ(cluster.GetOperationMode(), OperationModeEnum::kNormal);
    EXPECT_EQ(cluster.GetEffectiveOperationMode(), OperationModeEnum::kNormal);
    EXPECT_EQ(cluster.GetEffectiveControlMode(), ControlModeEnum::kConstantFlow);
}

namespace {

/// Changes the remote sensor and calls OnRemoteSensorChanged once, from the notification for
/// `mTriggerAttribute`.
class ReentrantSensorDelegate : public TestDelegate
{
public:
    void OnAttributeChanged(EndpointId endpointId, AttributeId attributeId) override
    {
        TestDelegate::OnAttributeChanged(endpointId, attributeId);
        if (mCluster != nullptr && attributeId == mTriggerAttribute)
        {
            PumpConfigurationAndControlCluster * cluster = mCluster;
            mCluster                                     = nullptr;
            mRemoteSensorType                            = mNextSensorType;
            cluster->OnRemoteSensorChanged();
        }
    }

    PumpConfigurationAndControlCluster * mCluster = nullptr;
    AttributeId mTriggerAttribute                 = 0;
    RemoteSensorType mNextSensorType              = RemoteSensorType::kNoSensor;
};

} // namespace

TEST_F(TestPumpConfigurationAndControlCluster, ReentrantSensorChangeKeepsStateConsistent)
{
    // The delegate re-enters from each notification of a mode update. The final state must
    // reflect the latest sensor, with EffectiveControlMode and the Remote* bits in agreement.
    for (AttributeId trigger : { EffectiveOperationMode::Id, EffectiveControlMode::Id, PumpStatus::Id })
    {
        ReentrantSensorDelegate delegate;
        delegate.mRemoteSensorType = RemoteSensorType::kPressureSensor;

        PumpConfigurationAndControlCluster::Config config(delegate);
        config.features = BitFlags<Feature>(Feature::kConstantSpeed, Feature::kConstantPressure, Feature::kConstantFlow);
        config.optionalAttributes.Set(OptionalAttributes::kPumpStatus).Set(OptionalAttributes::kControlMode);
        config.operationMode = OperationModeEnum::kMaximum;

        PumpConfigurationAndControlCluster cluster(kTestEndpointId, config);
        ClusterTester tester(cluster);
        ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));

        // Maximum -> Normal changes EffectiveOperationMode, EffectiveControlMode
        // (ConstantSpeed -> ConstantPressure) and PumpStatus (RemotePressure).
        delegate.mCluster          = &cluster;
        delegate.mTriggerAttribute = trigger;
        delegate.mNextSensorType   = RemoteSensorType::kFlowSensor;
        EXPECT_EQ(tester.WriteAttribute(OperationMode::Id, OperationModeEnum::kNormal), CHIP_NO_ERROR);

        EXPECT_EQ(delegate.mCluster, nullptr) << "re-entry for attribute " << trigger;
        EXPECT_EQ(cluster.GetEffectiveOperationMode(), OperationModeEnum::kNormal);
        EXPECT_EQ(cluster.GetEffectiveControlMode(), ControlModeEnum::kConstantFlow) << "re-entry for attribute " << trigger;
        EXPECT_TRUE(cluster.GetPumpStatus().Has(PumpStatusBitmap::kRemoteFlow)) << "re-entry for attribute " << trigger;
        EXPECT_FALSE(cluster.GetPumpStatus().Has(PumpStatusBitmap::kRemotePressure)) << "re-entry for attribute " << trigger;

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestPumpConfigurationAndControlCluster, StartupLoadsInt24uLifetimeRunningHours)
{
    using Int24uTraits = NumericAttributeTraits<OddSizedInteger<3, false>>;
    const ConcreteAttributePath path(kTestEndpointId, PumpConfigurationAndControl::Id, LifetimeRunningHours::Id);

    PumpConfigurationAndControlCluster::Config config = ConfigWithFeatures(BitFlags<Feature>(Feature::kConstantSpeed));
    config.optionalAttributes.Set(OptionalAttributes::kLifetimeRunningHours);

    for (const DataModel::Nullable<uint32_t> & stored :
         { DataModel::MakeNullable(static_cast<uint32_t>(1234)), DataModel::Nullable<uint32_t>() })
    {
        PumpConfigurationAndControlCluster cluster(kTestEndpointId, config);
        ClusterTester tester(cluster);
        AttributePersistenceProvider & provider = tester.GetServerClusterContext().attributeStorage;

        // Earlier implementations stored LifetimeRunningHours in the 3-byte int24u format, with 0xFFFFFF as null.
        Int24uTraits::StorageType int24uValue;
        if (stored.IsNull())
        {
            Int24uTraits::SetNull(int24uValue);
        }
        else
        {
            Int24uTraits::WorkingToStorage(stored.Value(), int24uValue);
        }
        ASSERT_SUCCESS(provider.WriteValue(path, ByteSpan(int24uValue)));

        ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));
        EXPECT_EQ(cluster.GetLifetimeRunningHours(), stored);
        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);

        // The value was converted to the current format.
        DataModel::Nullable<uint32_t> converted;
        AttributePersistence persistence(provider);
        EXPECT_TRUE(persistence.LoadNativeEndianValue(path, converted, DataModel::MakeNullable(static_cast<uint32_t>(0))));
        EXPECT_EQ(converted, stored);
    }
}

TEST_F(TestPumpConfigurationAndControlCluster, EffectiveModesAreNotPersisted)
{
    PumpConfigurationAndControlCluster::Config config =
        ConfigWithFeatures(BitFlags<Feature>(Feature::kConstantSpeed, Feature::kConstantFlow, Feature::kLocalOperation));
    config.optionalAttributes.Set(OptionalAttributes::kControlMode);
    config.controlMode = ControlModeEnum::kConstantFlow;
    PumpConfigurationAndControlCluster cluster(kTestEndpointId, config);
    ClusterTester tester(cluster);
    ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));
    ASSERT_EQ(cluster.GetEffectiveControlMode(), ControlModeEnum::kConstantFlow);

    // Changing OperationMode to Maximum updates both EffectiveOperationMode and EffectiveControlMode,
    // without writing either derived value to persistent storage.
    EXPECT_EQ(tester.WriteAttribute(OperationMode::Id, OperationModeEnum::kMaximum), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetEffectiveOperationMode(), OperationModeEnum::kMaximum);
    EXPECT_EQ(cluster.GetEffectiveControlMode(), ControlModeEnum::kConstantSpeed);

    // Application overrides are not persisted either.
    EXPECT_EQ(tester.WriteAttribute(OperationMode::Id, OperationModeEnum::kLocal), CHIP_NO_ERROR);
    EXPECT_SUCCESS(cluster.SetEffectiveControlMode(ControlModeEnum::kConstantSpeed));
    EXPECT_SUCCESS(cluster.SetEffectiveOperationMode(OperationModeEnum::kMaximum));

    uint8_t buffer[4];
    MutableByteSpan value(buffer);
    AttributePersistenceProvider & provider = tester.GetServerClusterContext().attributeStorage;
    EXPECT_NE(provider.ReadValue({ kTestEndpointId, PumpConfigurationAndControl::Id, EffectiveOperationMode::Id }, value),
              CHIP_NO_ERROR);

    value = MutableByteSpan(buffer);
    EXPECT_NE(provider.ReadValue({ kTestEndpointId, PumpConfigurationAndControl::Id, EffectiveControlMode::Id }, value),
              CHIP_NO_ERROR);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestPumpConfigurationAndControlCluster, EffectiveControlModeOverrideIsNotRestored)
{
    // The application override reflects the runtime behavior of the pump. After a restart,
    // EffectiveControlMode is computed again until the application sets the override again.
    struct
    {
        BitFlags<Feature> features;
        bool controlModeEnabled;
        ControlModeEnum overrideMode;
        ControlModeEnum expectedAfterRestart;
    } const kCases[] = {
        // Normal mode with ControlMode Automatic.
        { BitFlags<Feature>(Feature::kConstantSpeed, Feature::kConstantPressure, Feature::kAutomatic), true,
          ControlModeEnum::kConstantPressure, ControlModeEnum::kAutomatic },
        // ControlMode disabled: the configured (here: first supported) control mode is used.
        { BitFlags<Feature>(Feature::kConstantSpeed, Feature::kConstantFlow), false, ControlModeEnum::kConstantFlow,
          ControlModeEnum::kConstantSpeed },
    };

    for (const auto & testCase : kCases)
    {
        PumpConfigurationAndControlCluster::Config config = ConfigWithFeatures(testCase.features);
        if (testCase.controlModeEnabled)
        {
            config.optionalAttributes.Set(OptionalAttributes::kControlMode);
            config.controlMode = ControlModeEnum::kAutomatic;
        }

        TestServerClusterContext sharedContext;
        {
            PumpConfigurationAndControlCluster firstCluster(kTestEndpointId, config);
            ASSERT_SUCCESS(firstCluster.Startup(sharedContext.Get()));
            EXPECT_SUCCESS(firstCluster.SetEffectiveControlMode(testCase.overrideMode));
            EXPECT_EQ(firstCluster.GetEffectiveControlMode(), testCase.overrideMode);
            firstCluster.Shutdown(ClusterShutdownType::kClusterShutdown);
        }
        {
            PumpConfigurationAndControlCluster secondCluster(kTestEndpointId, config);
            ASSERT_SUCCESS(secondCluster.Startup(sharedContext.Get()));
            EXPECT_EQ(secondCluster.GetEffectiveControlMode(), testCase.expectedAfterRestart);
            secondCluster.Shutdown(ClusterShutdownType::kClusterShutdown);
        }
    }
}

TEST_F(TestPumpConfigurationAndControlCluster, SetEffectiveOperationMode)
{
    {
        // Minimum and Maximum require ConstantSpeed, Local requires LocalOperation.
        PumpConfigurationAndControlCluster cluster(kTestEndpointId, ConfigWithFeatures(BitFlags<Feature>(Feature::kConstantFlow)));
        ClusterTester tester(cluster);
        ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));

        mDelegate.mChangedAttributes.clear();
        EXPECT_EQ(cluster.SetEffectiveOperationMode(OperationModeEnum::kMinimum), CHIP_IM_GLOBAL_STATUS(ConstraintError));
        EXPECT_EQ(cluster.SetEffectiveOperationMode(OperationModeEnum::kMaximum), CHIP_IM_GLOBAL_STATUS(ConstraintError));
        EXPECT_EQ(cluster.SetEffectiveOperationMode(OperationModeEnum::kLocal), CHIP_IM_GLOBAL_STATUS(ConstraintError));
        EXPECT_EQ(cluster.SetEffectiveOperationMode(static_cast<OperationModeEnum>(9)), CHIP_IM_GLOBAL_STATUS(ConstraintError));
        EXPECT_EQ(cluster.GetEffectiveOperationMode(), OperationModeEnum::kNormal);
        EXPECT_TRUE(mDelegate.mChangedAttributes.empty());

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        PumpConfigurationAndControlCluster::Config config =
            ConfigWithFeatures(BitFlags<Feature>(Feature::kConstantSpeed, Feature::kLocalOperation));
        PumpConfigurationAndControlCluster cluster(kTestEndpointId, config);
        ClusterTester tester(cluster);
        ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));

        mDelegate.mChangedAttributes.clear();
        EXPECT_SUCCESS(cluster.SetEffectiveOperationMode(OperationModeEnum::kLocal));
        EXPECT_EQ(cluster.GetEffectiveOperationMode(), OperationModeEnum::kLocal);
        EXPECT_EQ(cluster.GetOperationMode(), OperationModeEnum::kNormal);
        EXPECT_TRUE(mDelegate.WasChanged(EffectiveOperationMode::Id));

        EXPECT_EQ(cluster.SetEffectiveOperationMode(static_cast<OperationModeEnum>(9)), CHIP_IM_GLOBAL_STATUS(ConstraintError));

        // The next OperationMode change sets it back to OperationMode.
        EXPECT_EQ(tester.WriteAttribute(OperationMode::Id, OperationModeEnum::kMaximum), CHIP_NO_ERROR);
        EXPECT_EQ(cluster.GetEffectiveOperationMode(), OperationModeEnum::kMaximum);

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

TEST_F(TestPumpConfigurationAndControlCluster, SetEffectiveControlModeRequiresFeature)
{
    PumpConfigurationAndControlCluster::Config config =
        ConfigWithFeatures(BitFlags<Feature>(Feature::kConstantSpeed, Feature::kConstantFlow, Feature::kLocalOperation));
    PumpConfigurationAndControlCluster cluster(kTestEndpointId, config);
    ClusterTester tester(cluster);
    ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));
    EXPECT_EQ(tester.WriteAttribute(OperationMode::Id, OperationModeEnum::kLocal), CHIP_NO_ERROR);

    mDelegate.mChangedAttributes.clear();
    // ConstantPressure requires the ConstantPressure feature; 6 is not a ControlModeEnum value.
    EXPECT_EQ(cluster.SetEffectiveControlMode(ControlModeEnum::kConstantPressure), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    EXPECT_EQ(cluster.SetEffectiveControlMode(static_cast<ControlModeEnum>(6)), CHIP_IM_GLOBAL_STATUS(ConstraintError));
    EXPECT_EQ(cluster.GetEffectiveControlMode(), ControlModeEnum::kConstantSpeed);
    EXPECT_TRUE(mDelegate.mChangedAttributes.empty());

    EXPECT_SUCCESS(cluster.SetEffectiveControlMode(ControlModeEnum::kConstantFlow));
    EXPECT_EQ(cluster.GetEffectiveControlMode(), ControlModeEnum::kConstantFlow);
    EXPECT_TRUE(mDelegate.WasChanged(EffectiveControlMode::Id));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestPumpConfigurationAndControlCluster, SetEffectiveControlModeRejectedWhenComputedByCluster)
{
    PumpConfigurationAndControlCluster::Config config =
        ConfigWithFeatures(BitFlags<Feature>(Feature::kConstantSpeed, Feature::kConstantPressure, Feature::kAutomatic));
    config.optionalAttributes.Set(OptionalAttributes::kControlMode).Set(OptionalAttributes::kPumpStatus);
    config.controlMode = ControlModeEnum::kConstantSpeed;

    PumpConfigurationAndControlCluster cluster(kTestEndpointId, config);
    ClusterTester tester(cluster);
    ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));

    const CHIP_ERROR kInvalidInState = CHIP_IM_GLOBAL_STATUS(InvalidInState);

    // Normal mode with a ControlMode other than Automatic: EffectiveControlMode follows ControlMode.
    EXPECT_EQ(cluster.SetEffectiveControlMode(ControlModeEnum::kConstantPressure), kInvalidInState);
    EXPECT_EQ(cluster.GetEffectiveControlMode(), ControlModeEnum::kConstantSpeed);

    EXPECT_EQ(tester.WriteAttribute(ControlMode::Id, ControlModeEnum::kAutomatic), CHIP_NO_ERROR);
    EXPECT_SUCCESS(cluster.SetEffectiveControlMode(ControlModeEnum::kConstantSpeed));

    // An active remote sensor selects EffectiveControlMode, matching the Remote* bit of PumpStatus.
    mDelegate.mRemoteSensorType = RemoteSensorType::kPressureSensor;
    cluster.OnRemoteSensorChanged();
    ASSERT_EQ(cluster.GetEffectiveControlMode(), ControlModeEnum::kConstantPressure);
    EXPECT_EQ(cluster.SetEffectiveControlMode(ControlModeEnum::kConstantSpeed), kInvalidInState);
    EXPECT_EQ(cluster.GetEffectiveControlMode(), ControlModeEnum::kConstantPressure);
    EXPECT_TRUE(cluster.GetPumpStatus().Has(PumpStatusBitmap::kRemotePressure));

    // Minimum and Maximum run the pump at a constant speed.
    mDelegate.mRemoteSensorType = RemoteSensorType::kNoSensor;
    EXPECT_EQ(tester.WriteAttribute(OperationMode::Id, OperationModeEnum::kMaximum), CHIP_NO_ERROR);
    ASSERT_EQ(cluster.GetEffectiveControlMode(), ControlModeEnum::kConstantSpeed);
    EXPECT_EQ(cluster.SetEffectiveControlMode(ControlModeEnum::kConstantPressure), kInvalidInState);
    EXPECT_EQ(cluster.GetEffectiveControlMode(), ControlModeEnum::kConstantSpeed);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestPumpConfigurationAndControlCluster, UnchangedRemoteSensorKeepsEffectiveControlModeOverride)
{
    // SetApplicationDelegate calls OnRemoteSensorChanged on a started cluster. When the remote
    // sensor did not change, the application override is kept.
    PumpConfigurationAndControlCluster::Config config =
        ConfigWithFeatures(BitFlags<Feature>(Feature::kConstantSpeed, Feature::kConstantPressure, Feature::kAutomatic));
    config.optionalAttributes.Set(OptionalAttributes::kControlMode);
    config.controlMode = ControlModeEnum::kAutomatic;

    PumpConfigurationAndControlCluster cluster(kTestEndpointId, config);
    ClusterTester tester(cluster);
    ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));
    EXPECT_SUCCESS(cluster.SetEffectiveControlMode(ControlModeEnum::kConstantPressure));

    mDelegate.mChangedAttributes.clear();
    cluster.OnRemoteSensorChanged();
    EXPECT_EQ(cluster.GetEffectiveControlMode(), ControlModeEnum::kConstantPressure);
    EXPECT_TRUE(mDelegate.mChangedAttributes.empty());

    // A new remote sensor takes precedence over the override, and removing it recomputes the value.
    mDelegate.mRemoteSensorType = RemoteSensorType::kPressureSensor;
    cluster.OnRemoteSensorChanged();
    mDelegate.mRemoteSensorType = RemoteSensorType::kNoSensor;
    cluster.OnRemoteSensorChanged();
    EXPECT_EQ(cluster.GetEffectiveControlMode(), ControlModeEnum::kAutomatic);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestPumpConfigurationAndControlCluster, EffectiveOperationModeOverrideKeptUntilOperationModeChanges)
{
    PumpConfigurationAndControlCluster::Config config =
        ConfigWithFeatures(BitFlags<Feature>(Feature::kConstantSpeed, Feature::kConstantPressure, Feature::kLocalOperation));
    config.optionalAttributes.Set(OptionalAttributes::kControlMode);
    config.controlMode = ControlModeEnum::kConstantSpeed;

    PumpConfigurationAndControlCluster cluster(kTestEndpointId, config);
    ClusterTester tester(cluster);
    ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));
    EXPECT_SUCCESS(cluster.SetEffectiveOperationMode(OperationModeEnum::kLocal));

    // EffectiveOperationMode only depends on OperationMode: ControlMode and remote sensor changes keep the override.
    mDelegate.mChangedAttributes.clear();
    EXPECT_EQ(tester.WriteAttribute(ControlMode::Id, ControlModeEnum::kConstantPressure), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetEffectiveOperationMode(), OperationModeEnum::kLocal);

    mDelegate.mRemoteSensorType = RemoteSensorType::kPressureSensor;
    cluster.OnRemoteSensorChanged();
    EXPECT_EQ(cluster.GetEffectiveOperationMode(), OperationModeEnum::kLocal);
    EXPECT_FALSE(mDelegate.WasChanged(EffectiveOperationMode::Id));

    EXPECT_EQ(tester.WriteAttribute(OperationMode::Id, OperationModeEnum::kMaximum), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetEffectiveOperationMode(), OperationModeEnum::kMaximum);
    EXPECT_TRUE(mDelegate.WasChanged(EffectiveOperationMode::Id));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestPumpConfigurationAndControlCluster, WriteRejectsReadOnlyAndDisabledAttributes)
{
    {
        // Enabled read-only attributes reject client writes with UnsupportedWrite.
        PumpConfigurationAndControlCluster::Config config = ConfigWithFeatures(BitFlags<Feature>(
            Feature::kConstantPressure, Feature::kCompensatedPressure, Feature::kConstantFlow, Feature::kConstantSpeed,
            Feature::kConstantTemperature, Feature::kAutomatic, Feature::kLocalOperation));
        config.optionalAttributes.Set(OptionalAttributes::kPumpStatus)
            .Set(OptionalAttributes::kSpeed)
            .Set(OptionalAttributes::kPower);

        PumpConfigurationAndControlCluster cluster(kTestEndpointId, config);
        ClusterTester tester(cluster);
        ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));

        for (AttributeId id : { MaxPressure::Id, MinConstPressure::Id, MaxConstPressure::Id, MinCompPressure::Id,
                                MaxCompPressure::Id, MinConstTemp::Id, MaxConstTemp::Id, Capacity::Id })
        {
            EXPECT_EQ(tester.WriteAttribute(id, DataModel::MakeNullable(static_cast<int16_t>(1))), Status::UnsupportedWrite)
                << "attribute " << id;
        }
        for (AttributeId id :
             { MaxSpeed::Id, MaxFlow::Id, MinConstSpeed::Id, MaxConstSpeed::Id, MinConstFlow::Id, MaxConstFlow::Id, Speed::Id })
        {
            EXPECT_EQ(tester.WriteAttribute(id, DataModel::MakeNullable(static_cast<uint16_t>(1))), Status::UnsupportedWrite)
                << "attribute " << id;
        }
        EXPECT_EQ(tester.WriteAttribute(PumpStatus::Id,
                                        PumpConfigurationAndControlCluster::PumpStatusBitMask(PumpStatusBitmap::kRunning)),
                  Status::UnsupportedWrite);
        EXPECT_EQ(tester.WriteAttribute(EffectiveOperationMode::Id, OperationModeEnum::kNormal), Status::UnsupportedWrite);
        EXPECT_EQ(tester.WriteAttribute(EffectiveControlMode::Id, ControlModeEnum::kConstantSpeed), Status::UnsupportedWrite);
        EXPECT_EQ(tester.WriteAttribute(Power::Id, DataModel::MakeNullable(static_cast<uint32_t>(1))), Status::UnsupportedWrite);

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        // Disabled optional writable attributes reject client writes with UnsupportedAttribute.
        PumpConfigurationAndControlCluster cluster(kTestEndpointId, ConfigWithFeatures(BitFlags<Feature>(Feature::kConstantSpeed)));
        ClusterTester tester(cluster);
        ASSERT_SUCCESS(cluster.Startup(tester.GetServerClusterContext()));

        EXPECT_EQ(tester.WriteAttribute(ControlMode::Id, ControlModeEnum::kConstantSpeed), Status::UnsupportedAttribute);
        EXPECT_EQ(tester.WriteAttribute(LifetimeRunningHours::Id, DataModel::MakeNullable(static_cast<uint32_t>(1))),
                  Status::UnsupportedAttribute);
        EXPECT_EQ(tester.WriteAttribute(LifetimeEnergyConsumed::Id, DataModel::MakeNullable(static_cast<uint32_t>(1))),
                  Status::UnsupportedAttribute);

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}
