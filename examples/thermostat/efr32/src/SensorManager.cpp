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

#ifdef USE_TEMP_SENSOR
#include "TemperatureSensor.h"
#endif
/**********************************************************
 * Defines and Constants
 *********************************************************/

#define ENDPOINT_ID 1

#define SENSOR_TIMER_PERIOD_ms 30000                                  // 5s timer period
#define SIMULATED_READING_REPETITION (60000 / SENSOR_TIMER_PERIOD_ms) // Change Simulated number at each minutes

using namespace chip;
using namespace ::chip::DeviceLayer;

/**********************************************************
 * Variable declarations
 *********************************************************/

TimerHandle_t sSensorTimer;
StaticTimer_t sStaticSensorTimerStruct;

constexpr uint16_t kMinTemperatureDelta = 50; // 0.5 degree Celcius

SensorManager SensorManager::sSensorManager;

#ifndef USE_TEMP_SENSOR
static int16_t mSimulatedTemp[] = { SIMULATED_TEMP };
#endif

CHIP_ERROR SensorManager::Init()
{
    // Create FreeRTOS sw timer for temp sensor timer.
    sSensorTimer = xTimerCreateStatic("sensorTmr", pdMS_TO_TICKS(SENSOR_TIMER_PERIOD_ms), true, nullptr, SensorTimerEventHandler,
                                      &sStaticSensorTimerStruct);

    if (sSensorTimer == NULL)
    {
        EFR32_LOG("sSensorTimer timer create failed");
        return APP_ERROR_CREATE_TIMER_FAILED;
    }

#ifdef USE_TEMP_SENSOR
    if (SL_STATUS_OK != TemperatureSensor::Init())
    {
        EFR32_LOG("Failed to Init Sensor");
        return CHIP_ERROR_INTERNAL;
    }
#endif

    // Update Temp immediatly at bootup
    SensorTimerEventHandler(sSensorTimer);

    // Trigger periodic update
    xTimerStart(sSensorTimer, portMAX_DELAY);

    return CHIP_NO_ERROR;
}

void SensorManager::SensorTimerEventHandler(TimerHandle_t xTimer)
{
    int16_t temperature            = 0;
    static int16_t lastTemperature = 0;

#ifdef USE_TEMP_SENSOR
    int32_t tempSum   = 0;
    uint32_t humidity = 0;

    for (uint8_t i = 0; i < 100; i++)
    {
        if (SL_STATUS_OK != TemperatureSensor::GetTemp(&humidity, &temperature))
        {
            EFR32_LOG("Failed to read Temperature !!!");
        }
        tempSum += temperature;
    }
    temperature = static_cast<int16_t>(tempSum / 100);
#else
    static uint8_t nbOfRepetition = 0;
    static uint8_t simulatedIndex = 0;
    if (simulatedIndex >= sizeof(mSimulatedTemp))
    {
        simulatedIndex = 0;
    }
    temperature = mSimulatedTemp[simulatedIndex];

    nbOfRepetition++;
    if (nbOfRepetition >= SIMULATED_READING_REPETITION)
    {
        simulatedIndex++;
        nbOfRepetition = 0;
    }
#endif // USE_TEMP_SENSOR

    EFR32_LOG("Sensor Temp is : %d", temperature);

    if ((temperature >= (lastTemperature + kMinTemperatureDelta)) || temperature <= (lastTemperature - kMinTemperatureDelta))
    {
        lastTemperature = temperature;
        PlatformMgr().LockChipStack();
        // The SensorMagager shouldn't be aware of the Endpoint ID TODO Fix this.
        // TODO Per Spec we should also apply the Offset stored in the same cluster before saving the temp

        app::Clusters::Thermostat::Attributes::LocalTemperature::Set(ENDPOINT_ID, temperature);
        PlatformMgr().UnlockChipStack();
    }
}
