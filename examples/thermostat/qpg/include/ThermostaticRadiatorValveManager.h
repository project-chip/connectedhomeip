/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <stdbool.h>
#include <stdint.h>

#include <functional>

#include "AppEvent.h"

#include "FreeRTOS.h"
#include "timers.h" // provides FreeRTOS timer support

#include <lib/core/CHIPError.h>

#include <app-common/zap-generated/cluster-objects.h>
#include <app/data-model/DecodableList.h>
#include <app/util/im-client-callbacks.h>

class ThermostaticRadiatorValveManager
{
public:
    enum Action_t
    {
        TRV_OFF_ACTION = 0,
        TRV_IDLE_ACTION,
        TRV_HEATING_ACTION,
        TRV_COOLING_ACTION,

        UNKNOWN_ACTION = 0xFF,
    } Action;

    typedef chip::app::Clusters::Thermostat::ControlSequenceOfOperationEnum Operation_t;
    typedef chip::app::Clusters::Thermostat::SystemModeEnum SystemMode_t;
    typedef chip::app::Clusters::ThermostatUserInterfaceConfiguration::TemperatureDisplayModeEnum TempDisplayMode_t;

    CHIP_ERROR Init(void);

    void SetAction(Action_t action);

    int16_t GetLocalTemperature(void);
    void UpdateLocalTemperature(int16_t aLocalTemperature);
    uint8_t GetPICoolingDemand(void);
    void SetPICoolingDemand(uint8_t aPI);
    uint8_t GetPIHeatingDemand(void);
    void SetPIHeatingDemand(uint8_t aPI);

    int16_t GetOccupiedCoolingSetpoint(void);
    void SetOccupiedCoolingSetpoint(int16_t aSetpoint);

    int16_t GetOccupiedHeatingSetpoint(void);
    void SetOccupiedHeatingSetpoint(int16_t aSetpoint);
    Operation_t GetControlSequenceOfOperation(void);
    void SetControlSequenceOfOperation(Operation_t aOperation);

    SystemMode_t GetSystemMode(void);
    void SetSystemMode(SystemMode_t aSystemMode);

    TempDisplayMode_t GetTemperatureDisplayMode(void);
    void SetTemperatureDisplayMode(TempDisplayMode_t aMode);

    using ThermostaticRadiatorValveCallback_fn = std::function<void(Action_t)>;

    void SetCallbacks(ThermostaticRadiatorValveCallback_fn aActionInitiated_CB,
                      ThermostaticRadiatorValveCallback_fn aActionCompleted_CB);

    void StartNormalOperation(void);
    void StopNormalOperation(void);
    void UpdateThermostaticRadiatorValveStatus(void);
    void DisplayTemperature(void);

private:
    friend ThermostaticRadiatorValveManager & ThermostaticRadiatorValveMgr(void);
    uint32_t mPeriodicDuration;
    Action_t mThermostaticRadiatorValve_action;

    ThermostaticRadiatorValveCallback_fn mActionInitiated_CB;
    ThermostaticRadiatorValveCallback_fn mActionCompleted_CB;

    void CancelTimer(void);
    void StartTimer(uint32_t aTimeoutMs);

    static ThermostaticRadiatorValveManager sThermostaticRadiatorValve;
    static void TimerEventHandler(TimerHandle_t xTimer);
    static void PeriodicTimerEventHandler(AppEvent * aEvent);
};

inline ThermostaticRadiatorValveManager & ThermostaticRadiatorValveMgr(void)
{
    return ThermostaticRadiatorValveManager::sThermostaticRadiatorValve;
}
