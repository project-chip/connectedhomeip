/*
 *
 *    Copyright (c) 2020-2024 Project CHIP Authors
 *    Copyright (c) 2019-2024 Google LLC.
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

/**********************************************************
 * Includes
 *********************************************************/

#include <cstdint>
#include <stdbool.h>
#include <stdint.h>

#ifdef DISPLAY_ENABLED
#include "AirQualitySensorUI.h"
#endif

#include "AppEvent.h"
#include "BaseApplication.h"
#include <app/ConcreteAttributePath.h>
#include <ble/BLEEndPoint.h>
#include <lib/core/CHIPError.h>
#include <platform/CHIPDeviceLayer.h>

/**********************************************************
 * Defines
 *********************************************************/

// Application-defined error codes in the CHIP_ERROR space.
#define APP_ERROR_EVENT_QUEUE_FAILED CHIP_APPLICATION_ERROR(0x01)
#define APP_ERROR_CREATE_TASK_FAILED CHIP_APPLICATION_ERROR(0x02)
#define APP_ERROR_UNHANDLED_EVENT CHIP_APPLICATION_ERROR(0x03)
#define APP_ERROR_CREATE_TIMER_FAILED CHIP_APPLICATION_ERROR(0x04)
#define APP_ERROR_START_TIMER_FAILED CHIP_APPLICATION_ERROR(0x05)
#define APP_ERROR_STOP_TIMER_FAILED CHIP_APPLICATION_ERROR(0x06)

/**********************************************************
 * AppTask Declaration
 *********************************************************/
class AppTask : public BaseApplication
{

public:
    AppTask() = default;

    static AppTask & GetAppTask();

    /**
     * @brief AppTask task main loop function
     *
     * @param pvParameter FreeRTOS task parameter
     */
    static void AppTaskMain(void * pvParameter);

    CHIP_ERROR StartAppTask();

    /**
     * @brief Request an update of the Air Quality Sensor LCD UI
     */
    void UpdateAirQualitySensorUI();

    /**
     * @brief Update the Air Quality cluster attribute
     */
    static void WriteAirQualityToAttribute(intptr_t context);

    /**
     * @brief Event handler when a button is pressed
     * Function posts an event for button processing
     *
     * @param buttonHandle APP_CONTROL_BUTTON or APP_FUNCTION_BUTTON
     * @param btnAction button action - SL_SIMPLE_BUTTON_PRESSED,
     *                  SL_SIMPLE_BUTTON_RELEASED or SL_SIMPLE_BUTTON_DISABLED
     */
    static void ButtonEventHandler(uint8_t button, uint8_t btnAction);

    void DMPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value);

    /**
     * @brief Read the current raw air quality sensor value into @p air_quality.
     *
     * Calls `AirQualitySensor::GetAirQuality` when `USE_AIR_QUALITY_SENSOR` is set,
     * otherwise steps through a simulated table. On error @p air_quality is left untouched and
     * the caller skips scheduling a cluster update for that tick.
     */
    CHIP_ERROR GetAirQualityValue(int32_t & air_quality);

    // Reads new generated sensor value, stores it, and updates local Air Quality attribute
    static void SensorTimerEventHandler(void * arg);

protected:
    /** Override of `BaseApplication::AppInit()`. */
    CHIP_ERROR AppInit() override;

    /** Bring up the air quality sensor app: Matter manager, sensor timer, sensor driver, first reading. */
    CHIP_ERROR InitAirQualitySensor();
};
