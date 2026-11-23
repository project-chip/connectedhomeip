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

#include <device/types/humidity-sensor/impl/IncreasingHumiditySensor.h>
#include <pw_unit_test/framework.h>

#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <lib/support/TimerDelegateMock.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {

constexpr uint16_t kStepHundredthsPercent = 100;

Clusters::RelativeHumidityMeasurementCluster::Config HumidityRange(uint16_t min, uint16_t max)
{
    Clusters::RelativeHumidityMeasurementCluster::Config config;
    config.minMeasuredValue = DataModel::MakeNullable(min);
    config.maxMeasuredValue = DataModel::MakeNullable(max);
    return config;
}

class TestIncreasingHumiditySensor : public ::testing::Test
{
public:
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

protected:
    Testing::TestServerClusterContext mContext;
    CodeDrivenDataModelProvider mProvider{ mContext.StorageDelegate(), mContext.AttributePersistenceProvider() };
    TimerDelegateMock mTimerDelegate;
};

TEST_F(TestIncreasingHumiditySensor, StepsFromTheCurrentValue)
{
    IncreasingHumiditySensor sensor(mTimerDelegate, HumidityRange(0, 10000));
    ASSERT_SUCCESS(sensor.Register(1, mProvider));

    auto & cluster = sensor.RelativeHumidityMeasurementCluster();
    ASSERT_SUCCESS(cluster.SetMeasuredValue(DataModel::MakeNullable<uint16_t>(500)));

    sensor.TimerFired();

    EXPECT_EQ(cluster.GetMeasuredValue(), DataModel::MakeNullable<uint16_t>(500 + kStepHundredthsPercent));

    sensor.Unregister(mProvider);
}

TEST_F(TestIncreasingHumiditySensor, NullValueStartsFromTheMinimum)
{
    IncreasingHumiditySensor sensor(mTimerDelegate, HumidityRange(0, 10000));
    ASSERT_SUCCESS(sensor.Register(1, mProvider));

    auto & cluster = sensor.RelativeHumidityMeasurementCluster();
    ASSERT_SUCCESS(cluster.SetMeasuredValue(DataModel::NullNullable));

    sensor.TimerFired();
    EXPECT_EQ(cluster.GetMeasuredValue(), DataModel::MakeNullable<uint16_t>(0));

    sensor.TimerFired();
    EXPECT_EQ(cluster.GetMeasuredValue(), DataModel::MakeNullable<uint16_t>(kStepHundredthsPercent));

    sensor.Unregister(mProvider);
}

TEST_F(TestIncreasingHumiditySensor, WrapsToTheMinimumAtTheMaximum)
{
    IncreasingHumiditySensor sensor(mTimerDelegate, HumidityRange(0, 10000));
    ASSERT_SUCCESS(sensor.Register(1, mProvider));

    auto & cluster = sensor.RelativeHumidityMeasurementCluster();
    ASSERT_SUCCESS(cluster.SetMeasuredValue(DataModel::MakeNullable<uint16_t>(10000)));

    sensor.TimerFired();

    EXPECT_EQ(cluster.GetMeasuredValue(), DataModel::MakeNullable<uint16_t>(0));

    sensor.Unregister(mProvider);
}

} // namespace
