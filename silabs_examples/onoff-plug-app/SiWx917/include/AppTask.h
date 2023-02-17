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

#include <stdbool.h>
#include <stdint.h>

#include "AppEvent.h"
#include "BaseApplication.h"
#include "FreeRTOS.h"
#include "OnOffPlugManager.h"
//#include "sl_simple_button_instances.h"
#include "timers.h" // provides FreeRTOS timer support
#include <ble/BLEEndPoint.h>
#include <platform/CHIPDeviceLayer.h>

/**********************************************************
 * Defines
 *********************************************************/
#define SL_SIMPLE_BUTTON_MODE_POLL               0U   ///< BUTTON input capture using polling
#define SL_SIMPLE_BUTTON_MODE_POLL_AND_DEBOUNCE  1U   ///< BUTTON input capture using polling and debouncing
#define SL_SIMPLE_BUTTON_MODE_INTERRUPT          2U   ///< BUTTON input capture using interrupt

#define SL_SIMPLE_BUTTON_DISABLED                2U   ///< BUTTON state is disabled
#define SL_SIMPLE_BUTTON_PRESSED                 1U   ///< BUTTON state is pressed
#define SL_SIMPLE_BUTTON_RELEASED                0U   ///< BUTTON state is released

typedef uint8_t sl_button_mode_t;       ///< BUTTON mode
typedef uint8_t sl_button_state_t;      ///< BUTTON state
typedef struct sl_button sl_button_t;

/// A BUTTON instance
typedef struct sl_button {
  void                  *context;                       ///< The context for this BUTTON instance
  void           (*init)(const sl_button_t *handle);   ///< Member function to initialize BUTTON instance
  void                  (*poll)(const sl_button_t *handle);         ///< Member function to poll BUTTON
  void                  (*enable)(const sl_button_t *handle);       ///< Member function to enable BUTTON
  void                  (*disable)(const sl_button_t *handle);      ///< Member function to disable BUTTON
  sl_button_state_t     (*get_state)(const sl_button_t *handle);    ///< Member function to retrieve BUTTON state
}sl_button;

const sl_button_t sl_button_btn0 = {
  .context = NULL,
  .init = NULL,
  .poll = NULL,
  .enable = NULL,
  .disable = NULL,
  .get_state = NULL,
};
#define APP_FUNCTION_BUTTON &sl_button_btn0

const sl_button_t sl_button_btn1 = {
  .context = NULL,
  .init = NULL,
  .poll = NULL,
  .enable = NULL,
  .disable = NULL,
  .get_state = NULL,
};
#define APP_LIGHT_SWITCH &sl_button_btn1

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

    static AppTask & GetAppTask() { return sAppTask; }

    /**
     * @brief AppTask task main loop function
     *
     * @param pvParameter FreeRTOS task parameter
     */
    static void AppTaskMain(void * pvParameter);

    CHIP_ERROR StartAppTask();

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

private:
    static AppTask sAppTask;

    static void ActionInitiated(OnOffPlugManager::Action_t aAction, int32_t aActor);
    static void ActionCompleted(OnOffPlugManager::Action_t aAction);
    static void OnOffActionEventHandler(AppEvent * aEvent);

    static void UpdateClusterState(intptr_t context);

    /**
     * @brief AppTask initialisation function
     *
     * @return CHIP_ERROR
     */
    CHIP_ERROR Init();

    static void TimerEventHandler(TimerHandle_t xTimer);
};
