/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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

/**********************************************************
 * Includes
 *********************************************************/

#include "SensorManager.h"
#include "AppConfig.h"
#include "AppEvent.h"
#include "AppTask.h"

#if defined(SL_MATTER_USE_SI70XX_SENSOR) && SL_MATTER_USE_SI70XX_SENSOR
#include "Si70xxSensor.h"
#endif // defined(SL_MATTER_USE_SI70XX_SENSOR) && SL_MATTER_USE_SI70XX_SENSOR
/**********************************************************
 * Defines and Constants
 *********************************************************/

using namespace chip;
using namespace chip::app;
using namespace ::chip::DeviceLayer;

constexpr EndpointId kThermostatEndpoint = 1;
constexpr uint16_t kSensorTImerPeriodMs  = 30000; // 30s timer period
constexpr uint16_t kMinTemperatureDelta  = 50;    // 0.5 degree Celcius

/**********************************************************
 * Variable declarations
 *********************************************************/
SensorManager SensorManager::sSensorManager;

#if !(defined(SL_MATTER_USE_SI70XX_SENSOR) && (SL_MATTER_USE_SI70XX_SENSOR))
constexpr uint16_t kSimulatedReadingFrequency = (60000 / kSensorTImerPeriodMs); // Change Simulated number at each minutes
static int16_t mSimulatedTemp[]               = { 2300, 2400, 2800, 2550, 2200, 2125, 2100, 2600, 1800, 2700 };
#endif // !(defined(SL_MATTER_USE_SI70XX_SENSOR) && (SL_MATTER_USE_SI70XX_SENSOR))

CHIP_ERROR SensorManager::Init()
{
    // Create cmsisos sw timer for temp sensor timer.
    mSensorTimer = osTimerNew(SensorTimerEventHandler, osTimerPeriodic, nullptr, nullptr);

    if (mSensorTimer == NULL)
    {
        SILABS_LOG("mSensorTimer timer create failed");
        return APP_ERROR_CREATE_TIMER_FAILED;
    }

#if defined(SL_MATTER_USE_SI70XX_SENSOR) && SL_MATTER_USE_SI70XX_SENSOR
    if (SL_STATUS_OK != Si70xxSensor::Init())
    {
        SILABS_LOG("Failed to Init Sensor");
        return CHIP_ERROR_INTERNAL;
    }
#endif // defined(SL_MATTER_USE_SI70XX_SENSOR) && SL_MATTER_USE_SI70XX_SENSOR

    // Update Temp immediatly at bootup
    SensorTimerEventHandler(nullptr);
    // Trigger periodic update
    osTimerStart(mSensorTimer, pdMS_TO_TICKS(kSensorTImerPeriodMs));
    return CHIP_NO_ERROR;
}

void SensorManager::SensorTimerEventHandler(void * arg)
{
    AppEvent event;
    event.Type    = AppEvent::kEventType_Timer;
    event.Handler = TemperatureUpdateEventHandler;

    AppTask::GetAppTask().PostEvent(&event);
}

void SensorManager::TemperatureUpdateEventHandler(AppEvent * aEvent)
{
    int16_t temperature            = 0;
    static int16_t lastTemperature = 0;

#if defined(SL_MATTER_USE_SI70XX_SENSOR) && SL_MATTER_USE_SI70XX_SENSOR
    int32_t tempSum   = 0;
    uint16_t humidity = 0;

    for (uint8_t i = 0; i < 100; i++)
    {
        if (SL_STATUS_OK != Si70xxSensor::GetSensorData(humidity, temperature))
        {
            SILABS_LOG("Failed to read Temperature !!!");
        }
        tempSum += temperature;
    }
    temperature = static_cast<int16_t>(tempSum / 100);

#else
    static uint8_t nbOfRepetition = 0;
    static uint8_t simulatedIndex = 0;
    if (simulatedIndex >= MATTER_ARRAY_SIZE(mSimulatedTemp))
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
#endif // defined(SL_MATTER_USE_SI70XX_SENSOR) && SL_MATTER_USE_SI70XX_SENSOR

    SILABS_LOG("Sensor Temp is : %d", temperature);

    MarkAttributeDirty reportState = MarkAttributeDirty::kNo;
    if ((temperature >= (lastTemperature + kMinTemperatureDelta)) || temperature <= (lastTemperature - kMinTemperatureDelta))
    {
        reportState = MarkAttributeDirty::kIfChanged;
    }

    lastTemperature = temperature;
    PlatformMgr().LockChipStack();
    // The SensorMagager shouldn't be aware of the Endpoint ID TODO Fix this.
    // TODO Per Spec we should also apply the Offset stored in the same cluster before saving the temp
    app::Clusters::Thermostat::Attributes::LocalTemperature::Set(kThermostatEndpoint, temperature, reportState);
    PlatformMgr().UnlockChipStack();
}
