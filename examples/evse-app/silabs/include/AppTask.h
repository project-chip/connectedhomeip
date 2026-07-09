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

/**********************************************************
 * Includes
 *********************************************************/

#include <stdbool.h>
#include <stdint.h>

#include "AppEvent.h"
#include "BaseApplication.h"

#include "FreeRTOS.h"
#include "timers.h" // provides FreeRTOS timer support
#include <ble/BLEEndPoint.h>
#include <lib/core/CHIPError.h>
#include <platform/CHIPDeviceLayer.h>

/**********************************************************
 * AppTask Declaration
 *********************************************************/

class AppTask : public BaseApplication
{

public:
    AppTask() = default;

    /** @brief Returns the active app instance */
    static AppTask & GetAppTask();

    /**
     * @brief AppTask task main loop function
     *
     * @param pvParameter FreeRTOS task parameter
     */
    static void AppTaskMain(void * pvParameter);

    /** @brief Creates and starts the AppTask thread */
    CHIP_ERROR StartAppTask();

    /**
     * @brief Event handler when a button is pressed
     * Function posts an event for button processing
     *
     * @param buttonHandle APP_FUNCTION_BUTTON
     * @param btnAction button action - SL_SIMPLE_BUTTON_PRESSED,
     *                  SL_SIMPLE_BUTTON_RELEASED or SL_SIMPLE_BUTTON_DISABLED
     */
    static void ButtonEventHandler(uint8_t button, uint8_t btnAction);

        /**
     * @brief Processes energy management button actions posted from ButtonEventHandler.
     *
     * @param aEvent button event being processed
     */
     static void EnergyManagementActionEventHandler(AppEvent * aEvent);

protected:
    /**
     * @brief Override of BaseApplication::AppInit() virtual method, called by BaseApplication::Init()
     *
     * @return CHIP_ERROR
     */
    CHIP_ERROR AppInit() override;

    /** @brief Application bring up. Locks the stack and calls EvseApplicationInit(). */
    void ApplicationInit();
};
