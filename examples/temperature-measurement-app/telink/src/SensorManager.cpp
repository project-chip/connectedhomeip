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

#include "SensorManager.h"
#include "AppConfig.h"
#include "AppEvent.h"
#include "AppTask.h"

#define TEMPERATURE_SIMULATION_IS_USED

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

using namespace chip;
using namespace ::chip::DeviceLayer;

constexpr float kMinTemperatureDelta          = 0.5; // 0.5 degree Celsius
constexpr uint16_t kSimulatedReadingFrequency = 5;   // Change Simulated number
static float mSimulatedTemp[]                 = { 23.01, 24.02, 28.03, 25.50, 22.05, 21.25, 21.07, 26.08, 18.09, 27.11 };

k_timer sSensorTimer;

SensorManager SensorManager::sSensorManager;

CHIP_ERROR SensorManager::Init()
{
    // TODO: Initialize temp sensor
    return CHIP_NO_ERROR;
}

int16_t SensorManager::SensorEventHandler()
{
    float temperature            = 0.0;
    static float lastTemperature = 0.0;

#ifdef TEMPERATURE_SIMULATION_IS_USED
    static uint8_t nbOfRepetition = 0;
    static uint8_t simulatedIndex = 0;
    if (simulatedIndex >= sizeof(mSimulatedTemp) - 1)
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

    if ((temperature >= (lastTemperature + kMinTemperatureDelta)) || temperature <= (lastTemperature - kMinTemperatureDelta))
    {
        lastTemperature = temperature;
        // Per spec Application Clusters 2.3.4.1. : MeasuredValue = 100 x temperature [°C]
        sSensorManager.mMeasuredTempCelsius = (int16_t) 100 * temperature;
    }
#else
    // TODO: provide REAL sensor implementation
    sSensorManager.mMeasuredTempCelsius = (int16_t) 100 * GetRealSensorTemperature();
#endif // TEMPERATURE_SIMULATION_IS_USED
    return sSensorManager.mMeasuredTempCelsius;
}

int16_t SensorManager::GetMeasuredValue()
{
    return SensorEventHandler();
}

int16_t SensorManager::GetMinMeasuredValue()
{
    return mMinMeasuredTempCelsius;
}

int16_t SensorManager::GetMaxMeasuredValue()
{
    return mMaxMeasuredTempCelsius;
}
