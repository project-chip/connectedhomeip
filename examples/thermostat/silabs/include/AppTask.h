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

#pragma once

#include "BaseApplication.h"

#include <app/ConcreteAttributePath.h>
#include <cstdint>
#include <lib/core/CHIPError.h>

struct AppEvent;

class AppTask : public BaseApplication
{
public:
    AppTask() = default;

    static AppTask & GetAppTask();

    /**
     * @brief AppTask task main loop function.
     *
     * @param pvParameter FreeRTOS task parameter
     */
    static void AppTaskMain(void * pvParameter);

    CHIP_ERROR StartAppTask();

    static void UpdateThermoStatUI();

    /**
     * @brief Event handler when a button is pressed.
     *
     * @param button    APP_FUNCTION_BUTTON
     * @param btnAction SL_SIMPLE_BUTTON_PRESSED, SL_SIMPLE_BUTTON_RELEASED or SL_SIMPLE_BUTTON_DISABLED
     */
    static void ButtonEventHandler(uint8_t button, uint8_t btnAction);

    /**
     * @brief Periodic CMSIS timer callback. Posts a temperature-update event into the AppTask queue.
     */
    static void SensorTimerEventHandler(void * arg);

    /**
     * @brief AppTask-thread handler that reads the sensor (Si70xx or simulated) and pushes
     *        Thermostat::LocalTemperature.
     */
    static void TemperatureUpdateEventHandler(AppEvent * aEvent);

    /**
     * @brief Thermostat-cluster post-attribute-change callback. Logs per-attribute info and
     *        triggers a UI refresh. Also fans out to the AWS hook when SL_MATTER_ENABLE_AWS is set.
     */
    void DMPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value);

    /**
     * @brief Initialize the temperature sensor backing this thermostat.
     *
     * Default behavior: when `SL_MATTER_USE_SI70XX_SENSOR` is set, initializes the
     * Si70xx driver; otherwise this is a no-op (simulated reads do not need init).
     */
    CHIP_ERROR InitSensor();

    /**
     * @brief Read the current temperature into @p temperature, in units of 0.01 deg C
     *        (matches the `Thermostat::LocalTemperature` cluster schema).
     *
     * Default behavior: averages 100 Si70xx samples when `SL_MATTER_USE_SI70XX_SENSOR`
     * is set, otherwise steps through a canned simulated table. On error @p temperature
     * is left untouched and the caller (`TemperatureUpdateEventHandler`) skips the
     * `LocalTemperature::Set` for that tick.
     */
    CHIP_ERROR GetTemperature(int16_t & temperature);

protected:
    /** Override of `BaseApplication::AppInit()`. */
    CHIP_ERROR AppInit() override;

    /** Bring up the thermostat app: sensor timer, sensor driver, first UI paint. */
    CHIP_ERROR InitThermostat();
};
