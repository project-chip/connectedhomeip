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

class TestAirQualitySensor : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

protected:

    Testing::TestServerClusterContext mContext;
    CodeDrivenDataModelProvider mProvider{ mContext.StorageDelegate(), mContext.AttributePersistenceProvider() };
    TimerDelegateMock mTimerDelegate;
};

TEST_F(TestAirQualitySensor, TestMinimalConfiguration)
{
    AirQualitySensor::Config config;
    AirQualitySensor sensor(mTimerDelegate, config);

    EXPECT_EQ(sensor.Register(1, mProvider), CHIP_NO_ERROR);
    EXPECT_EQ(sensor.GetEndpointId(), 1);

    EXPECT_EQ(sensor.AirQualityCluster().GetAirQuality(), AirQualityEnum::kUnknown);
    EXPECT_EQ(sensor.TemperatureCluster(), nullptr);
    EXPECT_EQ(sensor.HumidityCluster(), nullptr);
    EXPECT_EQ(sensor.CO2Cluster(), nullptr);
    EXPECT_EQ(sensor.GetConcentrationCluster(Pm25ConcentrationMeasurement::Id), nullptr);

    sensor.Unregister(mProvider);
}

TEST_F(TestAirQualitySensor, TestFluentBuilder)
{
    AirQualitySensor::Config config;
    config.WithTemperature(-2000, 6000)
        .WithRelativeHumidity(1000, 9000)
        .WithCarbonDioxide(400.0f, 2000.0f);

    AirQualitySensor sensor(mTimerDelegate, config);

    EXPECT_EQ(sensor.Register(1, mProvider), CHIP_NO_ERROR);
    EXPECT_NE(sensor.TemperatureCluster(), nullptr);
    EXPECT_NE(sensor.HumidityCluster(), nullptr);
    EXPECT_NE(sensor.CO2Cluster(), nullptr);

    {
        ClusterTester co2Tester(*sensor.CO2Cluster());
        DataModel::Nullable<float> co2Val;
        EXPECT_EQ(co2Tester.ReadAttribute(ConcentrationMeasurement::Attributes::MeasuredValue::Id, co2Val), CHIP_NO_ERROR);
        EXPECT_TRUE(co2Val.IsNull());
    }
    EXPECT_EQ(sensor.TemperatureCluster()->GetMinMeasuredValue().Value(), -2000);
    EXPECT_EQ(sensor.TemperatureCluster()->GetMaxMeasuredValue().Value(), 6000);
    EXPECT_EQ(sensor.HumidityCluster()->GetMinMeasuredValue().Value(), 1000);
    EXPECT_EQ(sensor.HumidityCluster()->GetMaxMeasuredValue().Value(), 9000);

    sensor.Unregister(mProvider);
}

TEST_F(TestAirQualitySensor, TestTelemetryUpdate)
{
    AirQualitySensor::Config config;
    config.WithTemperature()
        .WithRelativeHumidity()
        .WithCarbonDioxide();

    AirQualitySensor sensor(mTimerDelegate, config);
    EXPECT_EQ(sensor.Register(1, mProvider), CHIP_NO_ERROR);

    EXPECT_EQ(sensor.AirQualityCluster().SetAirQuality(AirQualityEnum::kFair), Protocols::InteractionModel::Status::Success);
    EXPECT_EQ(sensor.AirQualityCluster().GetAirQuality(), AirQualityEnum::kFair);

    EXPECT_EQ(sensor.TemperatureCluster()->SetMeasuredValue(DataModel::MakeNullable<int16_t>(2150)), CHIP_NO_ERROR);
    EXPECT_EQ(sensor.TemperatureCluster()->GetMeasuredValue().Value(), 2150);

    EXPECT_EQ(sensor.HumidityCluster()->SetMeasuredValue(DataModel::MakeNullable<uint16_t>(4500)), CHIP_NO_ERROR);
    EXPECT_EQ(sensor.HumidityCluster()->GetMeasuredValue().Value(), 4500);

    EXPECT_EQ(sensor.CO2Cluster()->SetMeasuredValue(DataModel::MakeNullable<float>(550.0f)), CHIP_NO_ERROR);
    {
        ClusterTester co2Tester(*sensor.CO2Cluster());
        DataModel::Nullable<float> co2Val;
        EXPECT_EQ(co2Tester.ReadAttribute(ConcentrationMeasurement::Attributes::MeasuredValue::Id, co2Val), CHIP_NO_ERROR);
        EXPECT_FALSE(co2Val.IsNull());
        EXPECT_FLOAT_EQ(co2Val.Value(), 550.0f);
    }

    sensor.Unregister(mProvider);
}

TEST_F(TestAirQualitySensor, TestSimulationTick)
{
    SimulatedAirQualitySensor sensor(mTimerDelegate);
    EXPECT_EQ(sensor.Register(1, mProvider), CHIP_NO_ERROR);

    EXPECT_EQ(sensor.AirQualityCluster().GetAirQuality(), AirQualityEnum::kUnknown);
    EXPECT_TRUE(sensor.TemperatureCluster()->GetMeasuredValue().IsNull());
    EXPECT_TRUE(sensor.HumidityCluster()->GetMeasuredValue().IsNull());
    {
        ClusterTester co2Tester(*sensor.CO2Cluster());
        DataModel::Nullable<float> co2Val;
        EXPECT_EQ(co2Tester.ReadAttribute(ConcentrationMeasurement::Attributes::MeasuredValue::Id, co2Val), CHIP_NO_ERROR);
        EXPECT_TRUE(co2Val.IsNull());
    }

    // First simulation tick
    sensor.TimerFired();

    EXPECT_EQ(sensor.AirQualityCluster().GetAirQuality(), AirQualityEnum::kGood);
    EXPECT_FALSE(sensor.TemperatureCluster()->GetMeasuredValue().IsNull());
    EXPECT_FALSE(sensor.HumidityCluster()->GetMeasuredValue().IsNull());
    {
        ClusterTester co2Tester(*sensor.CO2Cluster());
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
    AirQualitySensor::Config config;
    config.WithTemperature()
        .WithRelativeHumidity()
        .WithAllConcentrationClusters();

    AirQualitySensor sensor(mTimerDelegate, config);
    EXPECT_EQ(sensor.Register(1, mProvider), CHIP_NO_ERROR);

    EXPECT_NE(sensor.TemperatureCluster(), nullptr);
    EXPECT_NE(sensor.HumidityCluster(), nullptr);
    EXPECT_NE(sensor.CO2Cluster(), nullptr);

    EXPECT_NE(sensor.GetConcentrationCluster(CarbonDioxideConcentrationMeasurement::Id), nullptr);
    EXPECT_NE(sensor.GetConcentrationCluster(Pm25ConcentrationMeasurement::Id), nullptr);
    EXPECT_NE(sensor.GetConcentrationCluster(TotalVolatileOrganicCompoundsConcentrationMeasurement::Id), nullptr);
    EXPECT_NE(sensor.GetConcentrationCluster(CarbonMonoxideConcentrationMeasurement::Id), nullptr);
    EXPECT_NE(sensor.GetConcentrationCluster(NitrogenDioxideConcentrationMeasurement::Id), nullptr);
    EXPECT_NE(sensor.GetConcentrationCluster(OzoneConcentrationMeasurement::Id), nullptr);
    EXPECT_NE(sensor.GetConcentrationCluster(FormaldehydeConcentrationMeasurement::Id), nullptr);
    EXPECT_NE(sensor.GetConcentrationCluster(Pm1ConcentrationMeasurement::Id), nullptr);
    EXPECT_NE(sensor.GetConcentrationCluster(Pm10ConcentrationMeasurement::Id), nullptr);
    EXPECT_NE(sensor.GetConcentrationCluster(RadonConcentrationMeasurement::Id), nullptr);

    sensor.Unregister(mProvider);
}

TEST_F(TestAirQualitySensor, TestCleanTeardown)
{
    AirQualitySensor::Config config;
    config.WithTemperature().WithRelativeHumidity().WithCarbonDioxide();
    AirQualitySensor sensor(mTimerDelegate, config);

    EXPECT_EQ(sensor.Register(1, mProvider), CHIP_NO_ERROR);
    sensor.Unregister(mProvider);

    EXPECT_EQ(sensor.Register(2, mProvider), CHIP_NO_ERROR);
    sensor.Unregister(mProvider);
}

} // namespace
