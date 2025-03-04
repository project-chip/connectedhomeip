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

#include "TemperatureSensorManager.h"
#include "AppTask.h"



using namespace chip;
using namespace ::chip::DeviceLayer;

constexpr float kMinTemperatureDelta          = 0.5; // 0.5 degree Celsius
static float mSimulatedTemperature            = 25.5;


TemperatureSensorManager TemperatureSensorManager::sTemperatureSensorManager;


CHIP_ERROR TemperatureSensorManager::Init()
{
    // TODO: Initialize temperature sensor
    ChipLogProgress(DeviceLayer, "******* Temperature Sensor Initialized *******");
    return CHIP_NO_ERROR;
}


int16_t TemperatureSensorManager::GetMeasuredValue()
{
    mSimulatedTemperature += kMinTemperatureDelta;
    // Per spec Application Clusters 2.3.4.1. : MeasuredValue = 100 x temperature [°C]
	sTemperatureSensorManager.mMeasuredTempCelsius = (int16_t) 100 * mSimulatedTemperature;

    // TODO: provide REAL sensor implementation
    // sSensorManager.mMeasuredTempCelsius = (int16_t) 100 * GetRealSensorTemperature();

    return sTemperatureSensorManager.mMeasuredTempCelsius;
}

