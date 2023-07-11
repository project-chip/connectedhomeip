/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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
#include <ble/BLEEndPoint.h>
#include <lega_rtos_api.h>
#include <platform/CHIPDeviceLayer.h>
#include <stdbool.h>
#include <stdint.h>

// Application-defined error codes in the CHIP_ERROR space.
#define APP_ERROR_EVENT_QUEUE_FAILED CHIP_APPLICATION_ERROR(0x01)
#define APP_ERROR_CREATE_TASK_FAILED CHIP_APPLICATION_ERROR(0x02)
#define APP_ERROR_UNHANDLED_EVENT CHIP_APPLICATION_ERROR(0x03)
#define APP_ERROR_CREATE_TIMER_FAILED CHIP_APPLICATION_ERROR(0x04)
#define APP_ERROR_START_TIMER_FAILED CHIP_APPLICATION_ERROR(0x05)
#define APP_ERROR_STOP_TIMER_FAILED CHIP_APPLICATION_ERROR(0x06)

class AppTask
{

public:
    CHIP_ERROR StartAppTask();
    static void AppTaskMain(void * pvParameter);
    static void AppEventHandler(AppEvent * aEvent);
    void ButtonEventHandler(uint8_t btnIdx, uint8_t btnAction);
    void PostEvent(const AppEvent * event);
    /**
     * Use internally for registration of the ChipDeviceEvents
     */
    static void CommonDeviceEventHandler(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg);

private:
    friend AppTask & GetAppTask(void);

    CHIP_ERROR Init();

    static AppTask sAppTask;
    static void AppTimerCallback(void * params);
    void DispatchEvent(AppEvent * event);
    static void OnOffUpdateClusterState(void);
};

inline AppTask & GetAppTask(void)
{
    return AppTask::sAppTask;
}
