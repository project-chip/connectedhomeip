/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "SensorManager.h"
#include "AppConfig.h"
#include "AppEvent.h"
#include "AppTask.h"

LOG_MODULE_DECLARE(app);

using namespace chip;
using namespace ::chip::DeviceLayer;

constexpr EndpointId kThermostatEndpoint = 1;
constexpr uint16_t kSensorTimerPeriodMs  = 30000; // 30s timer period
constexpr uint16_t kMinTemperatureDelta  = 50;    // 0.5 degree Celsius

k_timer sSensorTimer;

SensorManager SensorManager::sSensorManager;

constexpr uint16_t kSimulatedReadingFrequency = (60000 / kSensorTimerPeriodMs); // Change Simulated number at each minutes
static int16_t mSimulatedTemp[]               = { 2300, 2400, 2800, 2550, 2200, 2125, 2100, 2600, 1800, 2700 };

CHIP_ERROR SensorManager::Init()
{
    // Initialize temp sensor timer
    k_timer_init(&sSensorTimer, &TimerEventHandler, nullptr);
    k_timer_user_data_set(&sSensorTimer, this);
    k_timer_start(&sSensorTimer, K_MSEC(kSensorTimerPeriodMs), K_NO_WAIT);

    return CHIP_NO_ERROR;
}

void SensorManager::TimerEventHandler(k_timer * timer)
{
    AppEvent event;
    event.Type               = AppEvent::kEventType_Thermostat;
    event.TimerEvent.Context = k_timer_user_data_get(timer);
    event.Handler            = SensorTimerEventHandler;
    GetAppTask().PostEvent(&event);
}

void SensorManager::SensorTimerEventHandler(AppEvent * aEvent)
{
    int16_t temperature            = 0;
    static int16_t lastTemperature = 0;

    static uint8_t nbOfRepetition = 0;
    static uint8_t simulatedIndex = 0;
    if (simulatedIndex >= sizeof(mSimulatedTemp))
    {
        simulatedIndex = 0;
    }
    temperature = mSimulatedTemp[simulatedIndex];

    nbOfRepetition++;
    if (nbOfRepetition >= kSimulatedReadingFrequency)
    {
        simulatedIndex++;
        nbOfRepetition = 0;
    }

    LOG_INF("Sensor Temp is : %d", temperature);

    if ((temperature >= (lastTemperature + kMinTemperatureDelta)) || temperature <= (lastTemperature - kMinTemperatureDelta))
    {
        lastTemperature = temperature;
        PlatformMgr().LockChipStack();
        app::Clusters::Thermostat::Attributes::LocalTemperature::Set(kThermostatEndpoint, temperature);
        PlatformMgr().UnlockChipStack();
    }

    // Start next timer to handle temp sensor.
    k_timer_start(&sSensorTimer, K_MSEC(kSensorTimerPeriodMs), K_NO_WAIT);
}
