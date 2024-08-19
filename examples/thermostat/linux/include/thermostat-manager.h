/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#pragma once

#include <app-common/zap-generated/attributes/Accessors.h>

class ThermostatManager
{
public:
    CHIP_ERROR Init();

    /// @brief Callback called when any attribute changed on the device
    void AttributeChangeHandler(chip::EndpointId endpointId, chip::ClusterId clusterId, chip::AttributeId attributeId,
                                uint8_t * value, uint16_t size);

    chip::app::Clusters::Thermostat::SystemModeEnum GetSystemMode();
    chip::app::Clusters::Thermostat::ThermostatRunningModeEnum GetRunningMode();
    int16_t GetCurrentTemperature();
    int16_t GetCurrentHeatingSetPoint();
    int16_t GetCurrentCoolingSetPoint();
    uint8_t GetNumberOfPresets();
    CHIP_ERROR SetSystemMode(chip::app::Clusters::Thermostat::SystemModeEnum systemMode);
    CHIP_ERROR SetRunningMode(chip::app::Clusters::Thermostat::ThermostatRunningModeEnum runningMode);
    CHIP_ERROR SetCurrentTemperature(int16_t temperature);
    CHIP_ERROR SetCurrentHeatingSetPoint(int16_t heatingSetpoint);
    CHIP_ERROR SetCurrentCoolingSetPoint(int16_t coolingSetpoint);

private:
    friend ThermostatManager & ThermostatMgr();

    chip::app::Clusters::Thermostat::SystemModeEnum mSystemMode;
    chip::app::Clusters::Thermostat::ThermostatRunningModeEnum mRunningMode;
    int16_t mLocalTemperature;
    int16_t mOccupiedCoolingSetpoint;
    int16_t mOccupiedHeatingSetpoint;
    uint8_t mOccupiedSetback;

    static ThermostatManager sThermostatMgr;

    /// @brief attribute handler for the thermostat endpoint
    void ThermostatEndpointAttributeChangeHandler(chip::ClusterId clusterId, chip::AttributeId attributeId, uint8_t * value,
                                                  uint16_t size);
    void ThermostatClusterAttributeChangeHandler(chip::AttributeId attributeId, uint8_t * value, uint16_t size);
    void LocalTemperatureMeasurementEndpointAttributeChangeHandler(chip::ClusterId clusterId, chip::AttributeId attributeId,
                                                                   uint8_t * value, uint16_t size);
    void LocalTemperatureMeasurementClusterAttributeChangeHandler(chip::AttributeId attributeId, uint8_t * value, uint16_t size);

    /// @brief  Main method that evaluates the current thermostat state and updates attributes
    void EvalThermostatState();
    void UpdateRunningModeForHeating();
    void UpdateRunningModeForCooling();
};

inline ThermostatManager & ThermostatMgr()
{
    return ThermostatManager::sThermostatMgr;
}
