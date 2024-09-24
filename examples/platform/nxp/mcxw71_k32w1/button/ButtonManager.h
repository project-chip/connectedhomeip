/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "AppEvent.h"

#include "FreeRTOS.h"
#include "timers.h"

#include "fsl_component_button.h"

#include <lib/core/CHIPError.h>

// Application-defined error codes in the CHIP_ERROR space.
#define APP_ERROR_CREATE_TIMER_FAILED CHIP_APPLICATION_ERROR(0x01)

/**
 * @brief Timeout (ms) for factory data reset action.
 *
 * During this timeout, the factory reset action can be cancelled by pressing a button.
 */
#ifndef BUTTON_MANAGER_FACTORY_RESET_TIMEOUT_MS
#define BUTTON_MANAGER_FACTORY_RESET_TIMEOUT_MS 6000
#endif

/**
 * @brief This class describes a manager for button callbacks.
 *
 */
class ButtonManager
{
public:
    CHIP_ERROR Init();

    // These are the callbacks registered with the buttons. They will delegate actions
    // to other methods based on the button event: short press, long press etc.
    static button_status_t BleCallback(void * handle, button_callback_message_t * message, void * param);
    static button_status_t AppActionCallback(void * handle, button_callback_message_t * message, void * param);

private:
    /**
     * @brief This callback performs a soft reset.
     *
     * This can be used when the user wants to clean reset the device,
     * meaning that Matter is properly shutdown, unlike a RESET button
     * press, where the device resets without calling the shutdown procedure.
     */
    static void SoftResetHandler(const AppEvent & event);

    /**
     * @brief This callback toggles between BLE start/stop advertising.
     *
     * It is used during commissioning to ensure a user-intent commissioning flow.
     */
    static void BleHandler(const AppEvent & event);

    /**
     * @brief This callback updates the application state.
     *
     * An example of application state would be the lighting manager light LED state.
     * This handler will toggle the light LED state.
     */
    static void AppActionEventHandler(const AppEvent & event);

    /**
     * @brief This callback schedules a factory reset.
     *
     * The factory reset is scheduled based on the value of BUTTON_MANAGER_FACTORY_RESET_TIMEOUT_MS.
     * Until the timer expires, the user can cancel the factory reset operation by doing an action.
     * In this reference app, the action would be pressing again the factory reset button.
     */
    static void ResetActionEventHandler(const AppEvent & event);

    /**
     * @brief This callback schedules a DSLS LIT action (Dynamic SIT LIT Support).
     *
     * It is used when the app requests SIT mode (check spec, "Runtime Operating Mode Switching")
     */
    static void DSLSActionEventHandler(const AppEvent & event);

    /**
     * @brief This callback performs a factory reset.
     *
     * This is the callback registered with the timer scheduled in ResetActionEventHandler.
     * It will schedule a factory reset using the Matter server instance.
     */
    static void FunctionTimerEventHandler(const AppEvent & event);

    void CancelTimer();
    void StartTimer(uint32_t aTimeoutInMs);

    friend ButtonManager & ButtonMgr();
    static ButtonManager sInstance;
};

inline ButtonManager & ButtonMgr()
{
    return ButtonManager::sInstance;
}
