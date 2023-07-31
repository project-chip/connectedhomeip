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
#include "timers.h" // provides FreeRTOS timer support
#include <app/clusters/identify-server/identify-server.h>
#include <app/util/config.h>
#include <ble/BLEEndPoint.h>
#include <lib/core/CHIPError.h>
#include <platform/CHIPDeviceEvent.h>
#include <platform/CHIPDeviceLayer.h>

#ifdef EMBER_AF_PLUGIN_IDENTIFY_SERVER
#include <app/clusters/identify-server/identify-server.h>
#endif

#ifdef DISPLAY_ENABLED
#include "demo-ui.h"
#include "lcd.h"
#ifdef QR_CODE_ENABLED
#include "qrcodegen.h"
#endif // QR_CODE_ENABLED
#endif // DISPLAY_ENABLED

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
 * BaseApplication Declaration
 *********************************************************/

class BaseApplication
{

public:
    BaseApplication() = default;
    virtual ~BaseApplication(){};

    /**
     * @brief Create AppTask task and Event Queue
     * If an error occurs during creation, application will hang after printing out error code
     *
     * @return CHIP_ERROR CHIP_NO_ERROR if no errors
     */
    CHIP_ERROR StartAppTask(TaskFunction_t taskFunction);

    /**
     * @brief PostEvent function that add event to AppTask queue for processing
     *
     * @param event AppEvent to post
     */
    static void PostEvent(const AppEvent * event);

#ifdef DISPLAY_ENABLED
    /**
     * @brief Return LCD object
     */
    static SilabsLCD & GetLCD(void);
#endif

    /**
     * @brief Function called to start the LED light timer
     */
    static void StartStatusLEDTimer(void);

    /**
     * @brief Function to stop LED light timer
     *        Turns off Status LED before stopping timer
     */
    static void StopStatusLEDTimer(void);

#ifdef EMBER_AF_PLUGIN_IDENTIFY_SERVER
    // Idenfiy server command callbacks.
    static void OnIdentifyStart(Identify * identify);
    static void OnIdentifyStop(Identify * identify);
    static void OnTriggerIdentifyEffectCompleted(chip::System::Layer * systemLayer, void * appState);
    static void OnTriggerIdentifyEffect(Identify * identify);
#endif

    enum Function_t
    {
        kFunction_NoneSelected   = 0,
        kFunction_SoftwareUpdate = 0,
        kFunction_StartBleAdv    = 1,
        kFunction_FactoryReset   = 2,

        kFunction_Invalid
    } Function;

protected:
    CHIP_ERROR Init();

    /**
     * @brief Function called to start the function timer
     *
     * @param aTimeoutMs timer duration in ms
     */
    static void StartFunctionTimer(uint32_t aTimeoutMs);

    /**
     * @brief Function to stop function timer
     */
    static void CancelFunctionTimer(void);

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

    /**
     * @brief Activate a set of Led patterns of the Status led
     *        Identify patterns and Trigger effects have priority
     *        If no identification patterns are in progress, we provide
     *        commissioning status feedback.
     *
     * @return True if a Led pattern was set, otherwise, returns false.
     */
    static bool ActivateStatusLedPatterns();

    /**
     * @brief Start the factory Reset process
     *  Almost identical to Server::ScheduleFactoryReset()
     *  but doesn't call GetFabricTable().DeleteAllFabrics(); which deletes Key per key.
     *  With our KVS platform implementation this is a lot slower than deleting the whole kvs section
     *  our silabs nvm3 driver which end up being doing in ConfigurationManagerImpl::DoFactoryReset(intptr_t arg).
     */
    static void ScheduleFactoryReset();

    static void OnPlatformEvent(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t);
    /**********************************************************
     * Protected Attributes declaration
     *********************************************************/
    bool mSyncClusterToButtonAction;
};
