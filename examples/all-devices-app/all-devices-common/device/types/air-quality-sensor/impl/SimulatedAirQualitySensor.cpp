/*
 *
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

#include "SimulatedAirQualitySensor.h"
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

namespace {

AirQualitySensor::Config DefaultSimulatedConfig()
{
    AirQualitySensor::Config config;
    config.WithTemperature().WithRelativeHumidity().WithCarbonDioxide();
    return config;
}

} // namespace

SimulatedAirQualitySensor::SimulatedAirQualitySensor(TimerDelegate & timerDelegate, const Config & config) :
    AirQualitySensor(timerDelegate, config)
{}

SimulatedAirQualitySensor::SimulatedAirQualitySensor(TimerDelegate & timerDelegate) :
    AirQualitySensor(timerDelegate, DefaultSimulatedConfig())
{}

SimulatedAirQualitySensor::~SimulatedAirQualitySensor()
{
    mTimerDelegate.CancelTimer(this);
}

CHIP_ERROR SimulatedAirQualitySensor::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                               EndpointComposition composition)
{
    ReturnErrorOnFailure(AirQualitySensor::Register(endpoint, provider, composition));
    return mTimerDelegate.StartTimer(this, kDefaultUpdateInterval);
}

void SimulatedAirQualitySensor::Unregister(CodeDrivenDataModelProvider & provider)
{
    mTimerDelegate.CancelTimer(this);
    AirQualitySensor::Unregister(provider);
}

void SimulatedAirQualitySensor::TimerFired()
{
    mTickCount++;

    // 1. Advance Air Quality enum
    AirQuality::AirQualityEnum aqValue;
    switch (mTickCount % 3)
    {
    case 1:
        aqValue = AirQuality::AirQualityEnum::kGood;
        break;
    case 2:
        aqValue = AirQuality::AirQualityEnum::kFair;
        break;
    default:
        aqValue = AirQuality::AirQualityEnum::kModerate;
        break;
    }
    AirQualityCluster().SetAirQuality(aqValue);

    // 2. Oscillate Temperature (~21.5°C ± 1.0°C)
    int16_t tempVal = static_cast<int16_t>(2150 + ((static_cast<int>(mTickCount) % 5) - 2) * 50);
    if (TemperatureCluster() != nullptr)
    {
        LogErrorOnFailure(TemperatureCluster()->SetMeasuredValue(DataModel::MakeNullable(tempVal)));
    }

    // 3. Oscillate Relative Humidity (~45% ± 3.0%)
    uint16_t humidityVal = static_cast<uint16_t>(4500 + ((static_cast<int>(mTickCount) % 5) - 2) * 150);
    if (HumidityCluster() != nullptr)
    {
        LogErrorOnFailure(HumidityCluster()->SetMeasuredValue(DataModel::MakeNullable(humidityVal)));
    }

    // 4. Oscillate CO2 (450 ppm to 850 ppm)
    float co2Val = 450.0f + static_cast<float>((mTickCount % 9) * 50);
    if (CO2Cluster() != nullptr)
    {
        LogErrorOnFailure(CO2Cluster()->SetMeasuredValue(DataModel::MakeNullable(co2Val)));
    }

    // 5. Update any other concentration clusters configured
    for (size_t i = 0; i < mNumConcentrationClusters; ++i)
    {
        if (mConcentrationClusters[i].IsConstructed())
        {
            ClusterId cid = mConfig.concentrationConfigs[i].clusterId;
            if (cid == CarbonDioxideConcentrationMeasurement::Id)
            {
                continue;
            }
            float val = 10.0f + static_cast<float>((mTickCount % 5) * 2);
            LogErrorOnFailure(mConcentrationClusters[i].Cluster().SetMeasuredValue(DataModel::MakeNullable(val)));
        }
    }

    ChipLogProgress(AppServer, "SimulatedAirQuality: AQ=%u, Temp=%.2fC, Humidity=%.1f%%, CO2=%.0fppm",
                    static_cast<unsigned>(aqValue), static_cast<double>(tempVal) / 100.0,
                    static_cast<double>(humidityVal) / 100.0, static_cast<double>(co2Val));

    LogErrorOnFailure(mTimerDelegate.StartTimer(this, kDefaultUpdateInterval));
}

} // namespace app
} // namespace chip
