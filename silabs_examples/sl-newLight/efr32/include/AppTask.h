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
#include "BaseApplication.h"
#include "AppEvent.h"
#include "FreeRTOS.h"
#include "sl_simple_button_instances.h"
#include "LightingManager.h"
#include "timers.h" // provides FreeRTOS timer support
#include <ble/BLEEndPoint.h>
#include <lib/core/CHIPError.h>
#include <platform/CHIPDeviceLayer.h>

// Application-defined error codes in the CHIP_ERROR space.
#define APP_ERROR_EVENT_QUEUE_FAILED CHIP_APPLICATION_ERROR(0x01)
#define APP_ERROR_CREATE_TASK_FAILED CHIP_APPLICATION_ERROR(0x02)
#define APP_ERROR_UNHANDLED_EVENT CHIP_APPLICATION_ERROR(0x03)
#define APP_ERROR_CREATE_TIMER_FAILED CHIP_APPLICATION_ERROR(0x04)
#define APP_ERROR_START_TIMER_FAILED CHIP_APPLICATION_ERROR(0x05)
#define APP_ERROR_STOP_TIMER_FAILED CHIP_APPLICATION_ERROR(0x06)

class AppTask : public BaseApplication
{

public:

    AppTask() = default;
    static AppTask & GetAppTask() { return sAppTask; }
    static void AppTaskMain(void * pvParameter);
    CHIP_ERROR StartAppTask();


    static void ButtonEventHandler(uint8_t button, uint8_t btnAction);

    void PostLightActionRequest(int32_t aActor, LightingManager::Action_t aAction);
    void PostLightControlActionRequest(int32_t aActor, LightingManager::Action_t aAction, uint8_t value);

private:
    static AppTask sAppTask;

    static void ActionInitiated(LightingManager::Action_t aAction, int32_t aActor);
    static void ActionCompleted(LightingManager::Action_t aAction);
    static void LightActionEventHandler(AppEvent * aEvent);
    static void LightControlEventHandler(AppEvent * aEvent);
    static void UpdateClusterState(intptr_t context);
    static void ButtonHandler(AppEvent * aEvent);
    CHIP_ERROR Init();
};
