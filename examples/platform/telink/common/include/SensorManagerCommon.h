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

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "AppTaskCommon.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <lib/core/CHIPError.h>

#if defined(CONFIG_CHIP_USE_MARS_SENSOR) && defined(CONFIG_WS2812_STRIP_GPIO_TELINK) && !defined(CONFIG_PM)
#define USE_COLOR_TEMPERATURE_LIGHT
#endif

class SensorManager
{
public:
    CHIP_ERROR Init();
    CHIP_ERROR GetTempAndHumMeasurValue(int16_t * pTempMeasured, uint16_t * pHumMeasured);

    int16_t GetMinMeasuredTempValue();
    int16_t GetMaxMeasuredTempValue();

private:
    friend SensorManager & SensorMgr();

#ifdef CONFIG_CHIP_USE_MARS_SENSOR
    static void SensorBanForNextMeasurTimerTimeoutCallback(k_timer * timer);

#ifdef USE_COLOR_TEMPERATURE_LIGHT
    void SetColorTemperatureLight(int8_t temp);
#endif // USE_COLOR_TEMPERATURE_LIGHT

    // SHT30 operating range −40…125°C
    int16_t mMinMeasuredTempCelsius = -40;
    int16_t mMaxMeasuredTempCelsius = 125;
#else
    int16_t mMinMeasuredTempCelsius = -40;
    int16_t mMaxMeasuredTempCelsius = 120;
#endif // CONFIG_CHIP_USE_MARS_SENSOR

    static SensorManager sSensorManager;
};

inline SensorManager & SensorMgr()
{
    return SensorManager::sSensorManager;
}
