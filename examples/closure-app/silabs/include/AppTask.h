/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
 *    Copyright (c) 2025 Google LLC.
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
     * @brief Event handler when a button is pressed.
     *
     * @param button    APP_CLOSURE_BUTTON or APP_FUNCTION_BUTTON
     * @param btnAction SL_SIMPLE_BUTTON_PRESSED, SL_SIMPLE_BUTTON_RELEASED or SL_SIMPLE_BUTTON_DISABLED
     */
    static void ButtonEventHandler(uint8_t button, uint8_t btnAction);

    /** @brief AppTask thread event handler for closure button actions */
    static void ClosureButtonActionEventHandler(AppEvent * aEvent);

    /** @brief Data model hook invoked when a cluster attribute changes */
    void DMPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value);

    /** @brief ClosureControl cluster attribute-change hook */
    void DMClosureControlClusterAttributeChangedCallback(const chip::app::ConcreteAttributePath & attributePath);

    /** @brief ClosureDimension cluster attribute-change hook */
    void DMClosureDimensionClusterAttributeChangedCallback(const chip::app::ConcreteAttributePath & attributePath);

protected:
    /** @brief Override of `BaseApplication::AppInit()` */
    CHIP_ERROR AppInit() override;
};
