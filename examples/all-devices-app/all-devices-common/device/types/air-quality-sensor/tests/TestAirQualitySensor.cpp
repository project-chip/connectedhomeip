/*
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

#include <device/types/air-quality-sensor/AirQualitySensor.h>
#include <device/types/air-quality-sensor/impl/SimulatedAirQualitySensor.h>
#include <pw_unit_test/framework.h>

#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <lib/support/TimerDelegateMock.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::AirQuality;
using chip::Testing::ClusterTester;

namespace {

class MockIdentifyDelegate : public IdentifyDelegate
{
public:
    void OnIdentifyStart(IdentifyCluster & cluster) override {}
    void OnIdentifyStop(IdentifyCluster & cluster) override {}
    void OnTriggerEffect(IdentifyCluster & cluster) override {}
    bool IsTriggerEffectEnabled() const override { return false; }
};

class TestAirQualitySensor : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

protected:
    Testing::TestServerClusterContext mContext;
    CodeDrivenDataModelProvider mProvider{ mContext.StorageDelegate(), mContext.AttributePersistenceProvider() };
    TimerDelegateMock mTimerDelegate;
    MockIdentifyDelegate mIdentifyDelegate;
};

TEST_F(TestAirQualitySensor, TestMinimalConfiguration)
{
    AirQualitySensor<> sensor(mTimerDelegate, mIdentifyDelegate);

    // Before registration, GetCluster returns nullptr since clusters are not yet constructed
    EXPECT_EQ(sensor.GetCluster<Clusters::Identify::Id>(), nullptr);
    EXPECT_EQ(sensor.GetCluster<Clusters::AirQuality::Id>(), nullptr);

    EXPECT_EQ(sensor.Register(1, mProvider), CHIP_NO_ERROR);
    EXPECT_EQ(sensor.GetEndpointId(), 1);

    EXPECT_NE(sensor.GetCluster<Clusters::Identify::Id>(), nullptr);
    EXPECT_NE(sensor.GetCluster<Clusters::AirQuality::Id>(), nullptr);

    // Verify const overload of GetCluster
    const auto & constSensor = sensor;
    EXPECT_NE(constSensor.GetCluster<Clusters::AirQuality::Id>(), nullptr);
    EXPECT_EQ(constSensor.GetCluster<Clusters::TemperatureMeasurement::Id>(), nullptr);

    EXPECT_EQ(sensor.AirQualityCluster().GetAirQuality(), AirQualityEnum::kUnknown);
    EXPECT_EQ(sensor.GetCluster<TemperatureMeasurement::Id>(), nullptr);
    EXPECT_EQ(sensor.GetCluster<RelativeHumidityMeasurement::Id>(), nullptr);
    EXPECT_EQ(sensor.GetCluster<CarbonDioxideConcentrationMeasurement::Id>(), nullptr);

    sensor.Unregister(mProvider);

    // After unregistration, GetCluster returns nullptr again
    EXPECT_EQ(sensor.GetCluster<Clusters::Identify::Id>(), nullptr);
    EXPECT_EQ(sensor.GetCluster<Clusters::AirQuality::Id>(), nullptr);
}

TEST_F(TestAirQualitySensor, TestTemplatedClusters)
{
    using ConfiguredSensor =
        AirQualitySensor<TemperatureMeasurement::Id, RelativeHumidityMeasurement::Id, CarbonDioxideConcentrationMeasurement::Id>;

    ConfiguredSensor::Config config;
    config.Get<TemperatureMeasurement::Id>().minMeasuredValue           = DataModel::MakeNullable(static_cast<int16_t>(-2000));
    config.Get<TemperatureMeasurement::Id>().maxMeasuredValue           = DataModel::MakeNullable(static_cast<int16_t>(6000));
    config.Get<RelativeHumidityMeasurement::Id>().minMeasuredValue      = DataModel::MakeNullable(static_cast<uint16_t>(1000));
    config.Get<RelativeHumidityMeasurement::Id>().maxMeasuredValue      = DataModel::MakeNullable(static_cast<uint16_t>(9000));
    config.Get<CarbonDioxideConcentrationMeasurement::Id>().minMeasured = DataModel::MakeNullable(400.0f);
    config.Get<CarbonDioxideConcentrationMeasurement::Id>().maxMeasured = DataModel::MakeNullable(2000.0f);

    ConfiguredSensor sensor(mTimerDelegate, mIdentifyDelegate, config);

    EXPECT_EQ(sensor.Register(1, mProvider), CHIP_NO_ERROR);
    EXPECT_NE(sensor.GetCluster<TemperatureMeasurement::Id>(), nullptr);
    EXPECT_NE(sensor.GetCluster<RelativeHumidityMeasurement::Id>(), nullptr);
    EXPECT_NE(sensor.GetCluster<CarbonDioxideConcentrationMeasurement::Id>(), nullptr);
    EXPECT_EQ(sensor.GetCluster<Pm25ConcentrationMeasurement::Id>(), nullptr);

    {
        ClusterTester co2Tester(*sensor.GetCluster<CarbonDioxideConcentrationMeasurement::Id>());
        DataModel::Nullable<float> co2Val;
        EXPECT_EQ(co2Tester.ReadAttribute(ConcentrationMeasurement::Attributes::MeasuredValue::Id, co2Val), CHIP_NO_ERROR);
        EXPECT_TRUE(co2Val.IsNull());

        DataModel::Nullable<float> minVal;
        EXPECT_EQ(co2Tester.ReadAttribute(ConcentrationMeasurement::Attributes::MinMeasuredValue::Id, minVal), CHIP_NO_ERROR);
        EXPECT_FALSE(minVal.IsNull());
        EXPECT_FLOAT_EQ(minVal.Value(), 400.0f);

        DataModel::Nullable<float> maxVal;
        EXPECT_EQ(co2Tester.ReadAttribute(ConcentrationMeasurement::Attributes::MaxMeasuredValue::Id, maxVal), CHIP_NO_ERROR);
        EXPECT_FALSE(maxVal.IsNull());
        EXPECT_FLOAT_EQ(maxVal.Value(), 2000.0f);
    }
    EXPECT_EQ(sensor.GetCluster<TemperatureMeasurement::Id>()->GetMinMeasuredValue().Value(), -2000);
    EXPECT_EQ(sensor.GetCluster<TemperatureMeasurement::Id>()->GetMaxMeasuredValue().Value(), 6000);
    EXPECT_EQ(sensor.GetCluster<RelativeHumidityMeasurement::Id>()->GetMinMeasuredValue().Value(), 1000);
    EXPECT_EQ(sensor.GetCluster<RelativeHumidityMeasurement::Id>()->GetMaxMeasuredValue().Value(), 9000);

    sensor.Unregister(mProvider);
}

TEST_F(TestAirQualitySensor, TestTelemetryUpdate)
{
    AirQualitySensor<TemperatureMeasurement::Id, RelativeHumidityMeasurement::Id, CarbonDioxideConcentrationMeasurement::Id> sensor(
        mTimerDelegate, mIdentifyDelegate);

    EXPECT_EQ(sensor.Register(1, mProvider), CHIP_NO_ERROR);

    EXPECT_EQ(sensor.AirQualityCluster().SetAirQuality(AirQualityEnum::kFair), Protocols::InteractionModel::Status::Success);
    EXPECT_EQ(sensor.AirQualityCluster().GetAirQuality(), AirQualityEnum::kFair);

    auto * temp = sensor.GetCluster<TemperatureMeasurement::Id>();
    ASSERT_NE(temp, nullptr);
    EXPECT_EQ(temp->SetMeasuredValue(DataModel::MakeNullable<int16_t>(2150)), CHIP_NO_ERROR);
    EXPECT_EQ(temp->GetMeasuredValue().Value(), 2150);

    auto * hum = sensor.GetCluster<RelativeHumidityMeasurement::Id>();
    ASSERT_NE(hum, nullptr);
    EXPECT_EQ(hum->SetMeasuredValue(DataModel::MakeNullable<uint16_t>(4500)), CHIP_NO_ERROR);
    EXPECT_EQ(hum->GetMeasuredValue().Value(), 4500);

    auto * co2Cluster = sensor.GetCluster<CarbonDioxideConcentrationMeasurement::Id>();
    ASSERT_NE(co2Cluster, nullptr);
    EXPECT_EQ(co2Cluster->SetMeasuredValue(DataModel::MakeNullable<float>(550.0f)), CHIP_NO_ERROR);
    {
        ClusterTester co2Tester(*co2Cluster);
        DataModel::Nullable<float> co2Val;
        EXPECT_EQ(co2Tester.ReadAttribute(ConcentrationMeasurement::Attributes::MeasuredValue::Id, co2Val), CHIP_NO_ERROR);
        EXPECT_FALSE(co2Val.IsNull());
        EXPECT_FLOAT_EQ(co2Val.Value(), 550.0f);
    }

    sensor.Unregister(mProvider);
}

TEST_F(TestAirQualitySensor, TestConcentrationConfigWithoutExplicitClusterId)
{
    // Verify that designated initialization without repeating `.clusterId` works correctly
    using Sensor = AirQualitySensor<CarbonDioxideConcentrationMeasurement::Id>;
    Sensor::Config config;
    config.Get<CarbonDioxideConcentrationMeasurement::Id>() = {
        .features    = BitFlags<ConcentrationMeasurement::Feature>(ConcentrationMeasurement::Feature::kNumericMeasurement),
        .medium      = ConcentrationMeasurement::MeasurementMediumEnum::kAir,
        .unit        = ConcentrationMeasurement::MeasurementUnitEnum::kPpm,
        .minMeasured = DataModel::MakeNullable(100.0f),
        .maxMeasured = DataModel::MakeNullable(5000.0f),
    };

    Sensor sensor(mTimerDelegate, mIdentifyDelegate, config);
    EXPECT_EQ(sensor.Register(1, mProvider), CHIP_NO_ERROR);

    auto * co2 = sensor.GetCluster<CarbonDioxideConcentrationMeasurement::Id>();
    ASSERT_NE(co2, nullptr);
    ASSERT_EQ(co2->GetPaths().size(), 1u);
    EXPECT_EQ(co2->GetPaths()[0].mClusterId, CarbonDioxideConcentrationMeasurement::Id);

    sensor.Unregister(mProvider);
}

TEST_F(TestAirQualitySensor, TestSimulationTick)
{
    using Sensor = SimulatedAirQualitySensor<TemperatureMeasurement::Id, RelativeHumidityMeasurement::Id,
                                             CarbonDioxideConcentrationMeasurement::Id>;
    Sensor sensor(mTimerDelegate, mIdentifyDelegate);
    EXPECT_EQ(sensor.Register(1, mProvider), CHIP_NO_ERROR);

    EXPECT_EQ(sensor.AirQualityCluster().GetAirQuality(), AirQualityEnum::kUnknown);
    EXPECT_TRUE(sensor.GetCluster<TemperatureMeasurement::Id>()->GetMeasuredValue().IsNull());
    EXPECT_TRUE(sensor.GetCluster<RelativeHumidityMeasurement::Id>()->GetMeasuredValue().IsNull());
    {
        ClusterTester co2Tester(*sensor.GetCluster<CarbonDioxideConcentrationMeasurement::Id>());
        DataModel::Nullable<float> co2Val;
        EXPECT_EQ(co2Tester.ReadAttribute(ConcentrationMeasurement::Attributes::MeasuredValue::Id, co2Val), CHIP_NO_ERROR);
        EXPECT_TRUE(co2Val.IsNull());
    }

    // First simulation tick
    sensor.TimerFired();

    EXPECT_EQ(sensor.AirQualityCluster().GetAirQuality(), AirQualityEnum::kGood);
    EXPECT_FALSE(sensor.GetCluster<TemperatureMeasurement::Id>()->GetMeasuredValue().IsNull());
    EXPECT_FALSE(sensor.GetCluster<RelativeHumidityMeasurement::Id>()->GetMeasuredValue().IsNull());
    {
        ClusterTester co2Tester(*sensor.GetCluster<CarbonDioxideConcentrationMeasurement::Id>());
        DataModel::Nullable<float> co2Val;
        EXPECT_EQ(co2Tester.ReadAttribute(ConcentrationMeasurement::Attributes::MeasuredValue::Id, co2Val), CHIP_NO_ERROR);
        EXPECT_FALSE(co2Val.IsNull());
    }

    // Second simulation tick
    sensor.TimerFired();
    EXPECT_EQ(sensor.AirQualityCluster().GetAirQuality(), AirQualityEnum::kFair);

    sensor.Unregister(mProvider);
}

TEST_F(TestAirQualitySensor, TestAllConcentrationClusters)
{
    using FullSensor =
        AirQualitySensor<TemperatureMeasurement::Id, RelativeHumidityMeasurement::Id, CarbonDioxideConcentrationMeasurement::Id,
                         Pm25ConcentrationMeasurement::Id, TotalVolatileOrganicCompoundsConcentrationMeasurement::Id,
                         CarbonMonoxideConcentrationMeasurement::Id, NitrogenDioxideConcentrationMeasurement::Id,
                         OzoneConcentrationMeasurement::Id, FormaldehydeConcentrationMeasurement::Id,
                         Pm1ConcentrationMeasurement::Id, Pm10ConcentrationMeasurement::Id, RadonConcentrationMeasurement::Id>;

    FullSensor sensor(mTimerDelegate, mIdentifyDelegate);
    EXPECT_EQ(sensor.Register(1, mProvider), CHIP_NO_ERROR);

    EXPECT_NE(sensor.GetCluster<TemperatureMeasurement::Id>(), nullptr);
    EXPECT_NE(sensor.GetCluster<RelativeHumidityMeasurement::Id>(), nullptr);
    EXPECT_NE(sensor.GetCluster<CarbonDioxideConcentrationMeasurement::Id>(), nullptr);
    EXPECT_NE(sensor.GetCluster<Pm25ConcentrationMeasurement::Id>(), nullptr);
    EXPECT_NE(sensor.GetCluster<TotalVolatileOrganicCompoundsConcentrationMeasurement::Id>(), nullptr);
    EXPECT_NE(sensor.GetCluster<CarbonMonoxideConcentrationMeasurement::Id>(), nullptr);
    EXPECT_NE(sensor.GetCluster<NitrogenDioxideConcentrationMeasurement::Id>(), nullptr);
    EXPECT_NE(sensor.GetCluster<OzoneConcentrationMeasurement::Id>(), nullptr);
    EXPECT_NE(sensor.GetCluster<FormaldehydeConcentrationMeasurement::Id>(), nullptr);
    EXPECT_NE(sensor.GetCluster<Pm1ConcentrationMeasurement::Id>(), nullptr);
    EXPECT_NE(sensor.GetCluster<Pm10ConcentrationMeasurement::Id>(), nullptr);
    EXPECT_NE(sensor.GetCluster<RadonConcentrationMeasurement::Id>(), nullptr);

    sensor.Unregister(mProvider);
}

TEST_F(TestAirQualitySensor, TestCleanTeardown)
{
    using TestSensor =
        AirQualitySensor<TemperatureMeasurement::Id, RelativeHumidityMeasurement::Id, CarbonDioxideConcentrationMeasurement::Id>;

    TestSensor sensor(mTimerDelegate, mIdentifyDelegate);

    EXPECT_EQ(sensor.Register(1, mProvider), CHIP_NO_ERROR);
    sensor.Unregister(mProvider);

    EXPECT_EQ(sensor.Register(2, mProvider), CHIP_NO_ERROR);
    sensor.Unregister(mProvider);
}

TEST_F(TestAirQualitySensor, TestCleanTeardownStartedProvider)
{
    using TestSensor =
        AirQualitySensor<TemperatureMeasurement::Id, RelativeHumidityMeasurement::Id, CarbonDioxideConcentrationMeasurement::Id>;

    TestSensor sensor(mTimerDelegate, mIdentifyDelegate);

    EXPECT_EQ(sensor.Register(1, mProvider), CHIP_NO_ERROR);
    EXPECT_EQ(mProvider.Startup(mContext.ImContext()), CHIP_NO_ERROR);

    sensor.Unregister(mProvider);
    EXPECT_EQ(mProvider.Shutdown(), CHIP_NO_ERROR);
}

} // namespace
