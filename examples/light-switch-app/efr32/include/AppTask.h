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
#include "FreeRTOS.h"
#include "sl_simple_button_instances.h"
#include "timers.h" // provides FreeRTOS timer support
#include <app/clusters/identify-server/identify-server.h>
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

class AppTask
{

public:
    /**********************************************************
     * Public Function Declaration
     *********************************************************/

    /**
     * @brief Create AppTask task and Event Queue
     * If an error occurs during creation, application will hang after printing out error code
     *
     * @return CHIP_ERROR CHIP_NO_ERROR if no errors
     */
    CHIP_ERROR StartAppTask();

    /**
     * @brief AppTask task main loop function
     *
     * @param pvParameter FreeRTOS task parameter
     */
    static void AppTaskMain(void * pvParameter);

    /**
     * @brief PostEvent function that add event to AppTask queue for processing
     *
     * @param event AppEvent to post
     */
    void PostEvent(const AppEvent * event);

    /**
     * @brief Event handler when a button is pressed
     * Function posts an event for button processing
     *
     * @param buttonHandle APP_LIGHT_SWITCH or APP_FUNCTION_BUTTON
     * @param btnAction button action - SL_SIMPLE_BUTTON_PRESSED,
     *                  SL_SIMPLE_BUTTON_RELEASED or SL_SIMPLE_BUTTON_DISABLED
     */
    void ButtonEventHandler(const sl_button_t * buttonHandle, uint8_t btnAction);

    /**
     * @brief Callback called by the identify-server when an identify command is received
     *
     * @param identify identify structure the command applies on
     */
    static void OnIdentifyStart(Identify * identify);

    /**
     * @brief Callback called by the identify-server when an identify command is stopped or finished
     *
     * @param identify identify structure the command applies on
     */
    static void OnIdentifyStop(Identify * identify);

    /**
     * @brief Function called to start the LED light timer
     */
    void StartLightTimer(void);

    /**
     * @brief Function to stop LED light timer
     *        Turns off Status LED before stopping timer
     */
    void CancelLightTimer(void);

private:
    /**********************************************************
     * Private Function Declaration
     *********************************************************/

    friend AppTask & GetAppTask(void);

    /**
     * @brief AppTask initialisation function
     *
     * @return CHIP_ERROR
     */
    CHIP_ERROR Init();

    /**
     * @brief Function called to start the function timer
     *
     * @param aTimeoutMs timer duration in ms
     */
    void StartFunctionTimer(uint32_t aTimeoutMs);

    /**
     * @brief Function to stop function timer
     */
    void CancelFunctionTimer(void);

    /**
     * @brief Function call event callback function for processing
     *
     * @param event triggered event to be processed
     */
    void DispatchEvent(AppEvent * event);

    /**
     * @brief Function Timer finished callback function
     *        Post an FunctionEventHandler event
     *
     * @param xTimer timer that finished
     */
    static void FunctionTimerEventHandler(TimerHandle_t xTimer);

    /**
     * @brief Timer Event processing function
     *        Trigger factory if Press and Hold duration is respected
     *
     * @param aEvent post event being processed
     */
    static void FunctionEventHandler(AppEvent * aEvent);

    /**
     * @brief PB0 Button event processing function
     *        Press and hold will trigger a factory reset timer start
     *        Press and release will restart BLEAdvertising if not commisionned
     *
     * @param aEvent button event being processed
     */
    static void ButtonHandler(AppEvent * aEvent);

    /**
     * @brief PB1 Button event processing function
     *        Function triggers a switch action sent to the CHIP task
     *
     * @param aEvent button event being processed
     */
    static void SwitchActionEventHandler(AppEvent * aEvent);

    /**
     * @brief Light Timer finished callback function
     *        Calls LED processing function
     *
     * @param xTimer timer that finished
     */
    static void LightTimerEventHandler(TimerHandle_t xTimer);

    /**
     * @brief Updates device LEDs
     */
    static void LightEventHandler();

    /**********************************************************
     * Private Attributes declaration
     *********************************************************/

    enum Function_t
    {
        kFunction_NoneSelected   = 0,
        kFunction_SoftwareUpdate = 0,
        kFunction_StartBleAdv    = 1,
        kFunction_FactoryReset   = 2,

        kFunction_Invalid
    } Function;

    Function_t mFunction;
    bool mFunctionTimerActive;
    bool mSyncClusterToButtonAction;

    static AppTask sAppTask;
};

inline AppTask & GetAppTask(void)
{
    return AppTask::sAppTask;
}
