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
#pragma once

#include <device/types/air-quality-sensor/AirQualitySensor.h>
#include <lib/support/TimerDelegate.h>

namespace chip {
namespace app {

/**
 * @brief Simulation subclass of AirQualitySensor for testing and development.
 *
 * Implements `TimerContext` to periodically (default 10s):
 *   1. Rotate through AirQualityEnum states (Good -> Fair -> Moderate).
 *   2. Oscillate TemperatureMeasurement readings around ~21.5°C (if present).
 *   3. Oscillate RelativeHumidityMeasurement readings around ~45.0% (if present).
 *   4. Oscillate all configured concentration measurement clusters (CO2 around ~450-850 ppm,
 *      others around ~20-65 ppm).
 *
 * @tparam OptionalClusters Cluster IDs of the optional clusters to instantiate and simulate.
 */
template <ClusterId... OptionalClusters>
class SimulatedAirQualitySensor : public AirQualitySensor<OptionalClusters...>, public TimerContext
{
public:
    /// Default period between simulated telemetry updates.
    static constexpr System::Clock::Seconds16 kDefaultUpdateInterval = System::Clock::Seconds16(10);

    using Base = AirQualitySensor<OptionalClusters...>;
    using Base::AirQualitySensor;

    ~SimulatedAirQualitySensor() override { this->mTimerDelegate.CancelTimer(this); }

    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition = {}) override
    {
        ReturnErrorOnFailure(Base::Register(endpoint, provider, composition));
        CHIP_ERROR err = this->mTimerDelegate.StartTimer(this, kDefaultUpdateInterval);
        if (err != CHIP_NO_ERROR)
        {
            Base::Unregister(provider);
        }
        return err;
    }

    void Unregister(CodeDrivenDataModelProvider & provider) override
    {
        this->mTimerDelegate.CancelTimer(this);
        Base::Unregister(provider);
    }

    // TimerContext: Periodic tick callback that updates sensor measurements
    void TimerFired() override
    {
        mTickCount++;
        LogErrorOnFailure(this->mTimerDelegate.StartTimer(this, kDefaultUpdateInterval));

        // 1. Advance Air Quality enum
        Clusters::AirQuality::AirQualityEnum aqValue;
        switch (mTickCount % 3)
        {
        case 1:
            aqValue = Clusters::AirQuality::AirQualityEnum::kGood;
            break;
        case 2:
            aqValue = Clusters::AirQuality::AirQualityEnum::kFair;
            break;
        default:
            aqValue = Clusters::AirQuality::AirQualityEnum::kModerate;
            break;
        }
        Protocols::InteractionModel::Status aqStatus = this->AirQualityCluster().SetAirQuality(aqValue);
        if (aqStatus != Protocols::InteractionModel::Status::Success)
        {
            ChipLogError(AppServer, "Failed to set air quality: %u", to_underlying(aqStatus));
        }

        // 2. Oscillate Temperature (~21.5°C ± 1.0°C) if configured
        if (auto * temp = this->template GetCluster<Clusters::TemperatureMeasurement::Id>())
        {
            int16_t tempVal = static_cast<int16_t>(2150 + (static_cast<int>(mTickCount % 5) - 2) * 50);
            LogErrorOnFailure(temp->SetMeasuredValue(DataModel::MakeNullable(tempVal)));
        }

        // 3. Oscillate Relative Humidity (~45% ± 3.0%) if configured
        if (auto * hum = this->template GetCluster<Clusters::RelativeHumidityMeasurement::Id>())
        {
            uint16_t humidityVal = static_cast<uint16_t>(4500 + (static_cast<int>(mTickCount % 5) - 2) * 150);
            LogErrorOnFailure(hum->SetMeasuredValue(DataModel::MakeNullable(humidityVal)));
        }

        // 4. Oscillate all configured concentration measurement clusters
        auto oscillateConcentration = [this](auto & clusterWrapper, auto clusterIdTag) {
            using TagType                 = decltype(clusterIdTag);
            constexpr ClusterId clusterId = TagType::value;
            if constexpr (AirQualitySensorInternal::IsConcentrationCluster<clusterId>)
            {
                if (clusterWrapper.IsConstructed())
                {
                    float val = 20.0f + static_cast<float>((mTickCount % 10) * 5);
                    if constexpr (clusterId == Clusters::CarbonDioxideConcentrationMeasurement::Id)
                    {
                        val = 450.0f + static_cast<float>((mTickCount % 9) * 50);
                    }
                    LogErrorOnFailure(clusterWrapper.Cluster().SetMeasuredValue(DataModel::MakeNullable(val)));
                }
            }
        };

        if constexpr (sizeof...(OptionalClusters) > 0)
        {
            (oscillateConcentration(
                 std::get<AirQualitySensorInternal::IndexOf<OptionalClusters, OptionalClusters...>()>(this->mOptionalClusters),
                 std::integral_constant<ClusterId, OptionalClusters>{}),
             ...);
        }
    }

private:
    uint32_t mTickCount = 0;
};

} // namespace app
} // namespace chip
