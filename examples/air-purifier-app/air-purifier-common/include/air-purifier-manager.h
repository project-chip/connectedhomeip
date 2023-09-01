#include <air-quality-sensor-manager.h>
#include <app/clusters/fan-control-server/fan-control-server.h>
#include <filter-delegates.h>
#include <relative-humidity-sensor-manager.h>
#include <temperature-sensor-manager.h>

#pragma once

namespace chip {
namespace app {
namespace Clusters {

constexpr std::bitset<4> gHepaFilterFeatureMap{ static_cast<uint32_t>(ResourceMonitoring::Feature::kCondition) |
                                                static_cast<uint32_t>(ResourceMonitoring::Feature::kWarning) |
                                                static_cast<uint32_t>(ResourceMonitoring::Feature::kReplacementProductList) };

constexpr std::bitset<4> gActivatedCarbonFeatureMap{ static_cast<uint32_t>(ResourceMonitoring::Feature::kCondition) |
                                                     static_cast<uint32_t>(ResourceMonitoring::Feature::kWarning) |
                                                     static_cast<uint32_t>(ResourceMonitoring::Feature::kReplacementProductList) };

class AirPurifierManager : public FanControl::Delegate
{
public:
    AirPurifierManager(EndpointId aEndpointId, EndpointId aAirQualitySensorEndpointId, EndpointId aTemperatureSensorEndpointId,
                       EndpointId aHumiditySensorEndpointId) :
        FanControl::Delegate(aEndpointId),
        mEndpointId(aEndpointId),
        activatedCarbonFilterInstance(&activatedCarbonFilterDelegate, mEndpointId, ActivatedCarbonFilterMonitoring::Id,
                                      static_cast<uint32_t>(gActivatedCarbonFeatureMap.to_ulong()),
                                      ResourceMonitoring::DegradationDirectionEnum::kDown, true),
        hepaFilterInstance(&hepaFilterDelegate, mEndpointId, HepaFilterMonitoring::Id,
                           static_cast<uint32_t>(gHepaFilterFeatureMap.to_ulong()),
                           ResourceMonitoring::DegradationDirectionEnum::kDown, true),
        mAirQualitySensorManager(aAirQualitySensorEndpointId), mTemperatureSensorManager(aTemperatureSensorEndpointId),
        mHumiditySensorManager(aHumiditySensorEndpointId)
    {
        FanControl::SetDefaultDelegate(mEndpointId, this);
        Init();
    };

    Protocols::InteractionModel::Status HandleStep(FanControl::StepDirectionEnum aDirection, bool aWrap, bool aLowestOff) override;
    void PercentSettingChangedCallback(uint8_t aNewPercentSetting);
    void SpeedSettingChangedCallback(uint8_t aNewSpeedSetting);

private:
    EndpointId mEndpointId;
    EndpointId mAirQualitySensorEndpointId;
    EndpointId mTemperatureSensorEndpointId;
    EndpointId mHumiditySensorEndpointId;

    uint8_t percentCurrent;
    uint8_t speedCurrent;

    // Set up for Activated Carbon Filter Monitoring
    ActivatedCarbonFilterMonitoringDelegate activatedCarbonFilterDelegate;
    ResourceMonitoring::Instance activatedCarbonFilterInstance;

    // Set up for Hepa Filter Monitoring
    HepaFilterMonitoringDelegate hepaFilterDelegate;
    ResourceMonitoring::Instance hepaFilterInstance;

    AirQualitySensorManager mAirQualitySensorManager;
    TemperatureSensorManager mTemperatureSensorManager;
    RelativeHumiditySensorManager mHumiditySensorManager;

    void Init();
};

} // namespace Clusters
} // namespace app
} // namespace chip