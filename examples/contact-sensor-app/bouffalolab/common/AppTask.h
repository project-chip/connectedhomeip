/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "FreeRTOS.h"
#include "timers.h"

#include <app/icd/server/ICDStateObserver.h>
#include <platform/CHIPDeviceLayer.h>

extern "C" {
#include <bl_gpio.h>
#include <hal_gpio.h>
#include <hosal_gpio.h>
}

using namespace ::chip;
using namespace ::chip::DeviceLayer;

#define APP_BUTTON_PRESS_JITTER 100
#define APP_BUTTON_PRESS_SHORT 1000
#define APP_BUTTON_PRESS_LONG 3000

#define APP_LIGHT_ENDPOINT_ID 1

// Application-defined error codes in the CHIP_ERROR space.
#define APP_ERROR_EVENT_QUEUE_FAILED CHIP_APPLICATION_ERROR(0x01)
#define APP_ERROR_CREATE_TASK_FAILED CHIP_APPLICATION_ERROR(0x02)
#define APP_ERROR_UNHANDLED_EVENT CHIP_APPLICATION_ERROR(0x03)
#define APP_ERROR_CREATE_TIMER_FAILED CHIP_APPLICATION_ERROR(0x04)
#define APP_ERROR_START_TIMER_FAILED CHIP_APPLICATION_ERROR(0x05)
#define APP_ERROR_STOP_TIMER_FAILED CHIP_APPLICATION_ERROR(0x06)

struct Identify;

#if CHIP_DETAIL_LOGGING
class AppTask : public chip::app::ICDStateObserver
#else
class AppTask
#endif
{
public:
    friend AppTask & GetAppTask(void);

    enum app_event_t
    {
        APP_EVENT_NONE = 0x00000000,

        APP_EVENT_BUTTON_PRESSED      = 0x00000010,
        APP_EVENT_FACTORY_RESET       = 0x00000020,
        APP_EVENT_COMMISSION_COMPLETE = 0x00000080,
        APP_EVENT_BUTTON_MASK         = APP_EVENT_BUTTON_PRESSED | APP_EVENT_FACTORY_RESET | APP_EVENT_COMMISSION_COMPLETE,

        APP_EVENT_CONTACT_SENSOR_TRUE  = 0x00000100,
        APP_EVENT_CONTACT_SENSOR_FALSE = 0x00000200,
        APP_EVENT_CONTACT_SENSOR_MASK  = APP_EVENT_CONTACT_SENSOR_TRUE | APP_EVENT_CONTACT_SENSOR_FALSE,

        APP_EVENT_IDENTIFY_START    = 0x01000000,
        APP_EVENT_IDENTIFY_IDENTIFY = 0x02000000,
        APP_EVENT_IDENTIFY_STOP     = 0x04000000,
        APP_EVENT_IDENTIFY_MASK     = APP_EVENT_IDENTIFY_START | APP_EVENT_IDENTIFY_IDENTIFY | APP_EVENT_IDENTIFY_STOP,

        APP_EVENT_ALL_MASK = APP_EVENT_BUTTON_MASK | APP_EVENT_CONTACT_SENSOR_MASK | APP_EVENT_IDENTIFY_MASK,
    };

    void SetEndpointId(EndpointId endpointId)
    {
        if (mEndpointId != (EndpointId) -1)
            mEndpointId = endpointId;
    }

    EndpointId GetEndpointId(void) { return mEndpointId; }
    void PostEvent(app_event_t event);
    static void ButtonEventHandler(void * arg);

#if CHIP_DETAIL_LOGGING
    void OnEnterActiveMode();
    void OnEnterIdleMode();
    void OnTransitionToIdle();
    void OnICDModeChange();
#endif

private:
    friend void StartAppTask(void);
    friend PlatformManagerImpl;

    static void ScheduleInit(intptr_t arg);
    static void AppTaskMain(void * pvParameter);

#if CONFIG_ENABLE_CHIP_SHELL
    static CHIP_ERROR StartAppShellTask();
    static void AppShellTask(void * args);
#endif

    uint64_t mButtonPressedTime;

    EndpointId mEndpointId = (EndpointId) 1;
    TaskHandle_t sAppTaskHandle;
    QueueHandle_t sAppEventQueue;

    static StackType_t appStack[APP_TASK_STACK_SIZE / sizeof(StackType_t)];
    static StaticTask_t appTaskStruct;
    static AppTask sAppTask;
};

inline AppTask & GetAppTask(void)
{
    return AppTask::sAppTask;
}

void StartAppTask();
