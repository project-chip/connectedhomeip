/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <DeviceManager.h>
#include <air-quality-sensor-manager.h>
#include <app/clusters/fan-control-server/fan-control-server.h>
#include <filter-delegates.h>
#include <relative-humidity-sensor-manager.h>
#include <temperature-sensor-manager.h>
#include <thermostat-manager.h>

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

/**
 * @brief
 *  This class provides a singleton AIr Purifier Manager which also implements the FanControl::Delegate and the Device Callbacks.
 *  The class is responsible for all Air Purifier Elements on the Air Purifier Endpoint and it has members that are other Managers
 *  for the devices in its composition tree.
 */
class AirPurifierManager : public FanControl::Delegate, public DeviceManager::DeviceManagerCallbacks
{
public:
    // Delete copy constructor and assignment operator.
    AirPurifierManager(const AirPurifierManager &)             = delete;
    AirPurifierManager(const AirPurifierManager &&)            = delete;
    AirPurifierManager & operator=(const AirPurifierManager &) = delete;

    static void InitInstance(EndpointId aEndpointId = 1, EndpointId aAirQualitySensorEndpointId = 2,
                             EndpointId aTemperatureSensorEndpointId = 3, EndpointId aHumiditySensorEndpointId = 4,
                             EndpointId aThermostatEndpointId = 5)
    {
        if (mInstance == nullptr)
        {
            mInstance = new AirPurifierManager(aEndpointId, aAirQualitySensorEndpointId, aTemperatureSensorEndpointId,
                                               aHumiditySensorEndpointId, aThermostatEndpointId);
            mInstance->Init();
        }
    };

    /**
     * @brief Get an Air Purifier Manager object - this class acts as a singleton device manager for the air purifier
     * @param[in] aEndpointId    Endpoint that the air purifier is on
     * @param[in] aAirQualitySensorEndpointId    Endpoint that the air quality sensor is on
     * @param[in] aTemperatureSensorEndpointId    Endpoint that the temperature sensor is on
     * @param[in] aHumiditySensorEndpointId    Endpoint that the humidity sensor is on
     * @return mInstance    The AirPurifierManager instance, note this this could be nullptr if InitInstance has not been called
     */
    static AirPurifierManager * GetInstance() { return mInstance; };

    /**
     * @brief Initialize the Air Purifier Manager and call init on underlying composed members.
     */
    void Init();

    /**
     * @brief Top level handler for all attribute changes in the device. This function will call the appropriate attribute change
     * handler based on the cluster id.
     */
    void PostAttributeChangeCallback(EndpointId endpoint, ClusterId clusterId, AttributeId attributeId, uint8_t type, uint16_t size,
                                     uint8_t * value) override;

    /**
     * @brief Handle the step command from the Fan Control Cluster
     */
    Protocols::InteractionModel::Status HandleStep(FanControl::StepDirectionEnum aDirection, bool aWrap, bool aLowestOff) override;

    /**
     * @brief Callback that thermostat manager calls when the heating state changes
     */
    void HeatingCallback();

private:
    inline static AirPurifierManager * mInstance;

    EndpointId mEndpointId;
    EndpointId mAirQualitySensorEndpointId;
    EndpointId mTemperatureSensorEndpointId;
    EndpointId mHumiditySensorEndpointId;
    EndpointId mThermostatEndpointId;

    uint8_t percentCurrent;
    uint8_t speedCurrent;

    bool fanWasStartedByUser = false;

    // Set up for Activated Carbon Filter Monitoring
    ActivatedCarbonFilterMonitoringDelegate activatedCarbonFilterDelegate;
    ResourceMonitoring::Instance activatedCarbonFilterInstance;

    // Set up for Hepa Filter Monitoring
    HepaFilterMonitoringDelegate hepaFilterDelegate;
    ResourceMonitoring::Instance hepaFilterInstance;

    // Set up other members of composed device
    AirQualitySensorManager mAirQualitySensorManager;
    TemperatureSensorManager mTemperatureSensorManager;
    RelativeHumiditySensorManager mHumiditySensorManager;
    ThermostatManager mThermostatManager;

    // Fan Mode Limits
    static constexpr int FAN_MODE_LOW_LOWER_BOUND    = 1;
    static constexpr int FAN_MODE_LOW_UPPER_BOUND    = 3;
    static constexpr int FAN_MODE_MEDIUM_LOWER_BOUND = 4;
    static constexpr int FAN_MODE_MEDIUM_UPPER_BOUND = 7;
    static constexpr int FAN_MODE_HIGH_LOWER_BOUND   = 8;
    static constexpr int FAN_MODE_HIGH_UPPER_BOUND   = 10;

    /**
     * @brief Construct a new Air Purifier Manager object - this class acts as a singleton device manager for the air purifier
     * @param[in] aEndpointId    Endpoint that the air purifier is on
     * @param[in] aAirQualitySensorEndpointId    Endpoint that the air quality sensor is on
     * @param[in] aTemperatureSensorEndpointId    Endpoint that the temperature sensor is on
     * @param[in] aHumiditySensorEndpointId    Endpoint that the humidity sensor is on
     */
    AirPurifierManager(EndpointId aEndpointId, EndpointId aAirQualitySensorEndpointId, EndpointId aTemperatureSensorEndpointId,
                       EndpointId aHumiditySensorEndpointId, EndpointId aThermostatEndpointId) :
        FanControl::Delegate(aEndpointId),
        mEndpointId(aEndpointId),
        activatedCarbonFilterInstance(&activatedCarbonFilterDelegate, mEndpointId, ActivatedCarbonFilterMonitoring::Id,
                                      static_cast<uint32_t>(gActivatedCarbonFeatureMap.to_ulong()),
                                      ResourceMonitoring::DegradationDirectionEnum::kDown, true),
        hepaFilterInstance(&hepaFilterDelegate, mEndpointId, HepaFilterMonitoring::Id,
                           static_cast<uint32_t>(gHepaFilterFeatureMap.to_ulong()),
                           ResourceMonitoring::DegradationDirectionEnum::kDown, true),
        mAirQualitySensorManager(aAirQualitySensorEndpointId), mTemperatureSensorManager(aTemperatureSensorEndpointId),
        mHumiditySensorManager(aHumiditySensorEndpointId),
        mThermostatManager(aThermostatEndpointId, [this]() { HeatingCallback(); })
    {}

    /**
     * @brief Handle attribute changes for the Fan Control Cluster
     * @param[in] attributeId    Cluster attribute id that changed
     * @param[in] type           Cluster attribute type
     * @param[in] size           Size of the attribute
     * @param[in] value          Pointer to the new value
     */
    void HandleFanControlAttributeChange(AttributeId attributeId, uint8_t type, uint16_t size, uint8_t * value);

    void PercentSettingWriteCallback(uint8_t aNewPercentSetting);
    void SpeedSettingWriteCallback(uint8_t aNewSpeedSetting);
    void FanModeWriteCallback(FanControl::FanModeEnum aNewFanMode);

    void SetSpeedSetting(DataModel::Nullable<uint8_t> aNewSpeedSetting);
    DataModel::Nullable<uint8_t> GetSpeedSetting();
    DataModel::Nullable<Percent> GetPercentSetting();

    void HandleThermostatAttributeChange(AttributeId attributeId, uint8_t type, uint16_t size, uint8_t * value);
    void ThermostatHeatingSetpointWriteCallback(int16_t aNewHeatingSetpoint);
    void ThermostatSystemModeWriteCallback(uint8_t aNewSystemMode);
};

} // namespace Clusters
} // namespace app
} // namespace chip
