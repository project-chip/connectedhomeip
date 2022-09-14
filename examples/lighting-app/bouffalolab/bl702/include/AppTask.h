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

// #include "AppEvent.h"

#include "FreeRTOS.h"
#include "timers.h"
#include <platform/CHIPDeviceLayer.h>

using namespace ::chip;
using namespace ::chip::DeviceLayer;

// Application-defined error codes in the CHIP_ERROR space.
#define APP_ERROR_EVENT_QUEUE_FAILED CHIP_APPLICATION_ERROR(0x01)
#define APP_ERROR_CREATE_TASK_FAILED CHIP_APPLICATION_ERROR(0x02)
#define APP_ERROR_UNHANDLED_EVENT CHIP_APPLICATION_ERROR(0x03)
#define APP_ERROR_CREATE_TIMER_FAILED CHIP_APPLICATION_ERROR(0x04)
#define APP_ERROR_START_TIMER_FAILED CHIP_APPLICATION_ERROR(0x05)
#define APP_ERROR_STOP_TIMER_FAILED CHIP_APPLICATION_ERROR(0x06)

struct Identify;

class AppTask
{
public:
    enum app_event_t
    {
        APP_EVENT_NONE = 0x00000000,

        APP_EVENT_STARTED                  = 0x00000001,
        APP_EVENT_BTN_FACTORY_RESET_CANCEL = 0x00000002,
        APP_EVENT_BTN_FACTORY_RESET_IND    = 0x00000004,
        APP_EVENT_BTN_FACTORY_RESET_PW_PRC = 0x00000008,

        APP_EVENT_INIT_ALL_MASK = APP_EVENT_STARTED | APP_EVENT_BTN_FACTORY_RESET_CANCEL | APP_EVENT_BTN_FACTORY_RESET_IND,
        APP_EVENT_TIMER         = 0x00000010,

        APP_EVENT_SYS_BLE_ADV     = 0x00000100,
        APP_EVENT_SYS_BLE_CONN    = 0x00000200,
        APP_EVENT_SYS_PROVISIONED = 0x00000400,
        APP_EVENT_FACTORY_RESET   = 0x00001000,

        APP_EVENT_SYS_ALL_MASK =
            APP_EVENT_SYS_BLE_ADV | APP_EVENT_SYS_BLE_CONN | APP_EVENT_SYS_PROVISIONED | APP_EVENT_FACTORY_RESET,

        APP_EVENT_LIGHTING_ONOFF      = 0x00010000,
        APP_EVENT_LIGHTING_LEVEL      = 0x00020000,
        APP_EVENT_LIGHTING_COLOR      = 0x00040000,
        APP_EVENT_LIGHTING_CHECK      = 0x00080000,
        APP_EVENT_LIGHTING_GO_THROUGH = 0x00100000,
        APP_EVENT_LIGHTING_MASK =
            APP_EVENT_LIGHTING_ONOFF | APP_EVENT_LIGHTING_LEVEL | APP_EVENT_LIGHTING_COLOR | APP_EVENT_LIGHTING_CHECK,

        APP_EVENT_IDENTIFY_START    = 0x01000000,
        APP_EVENT_IDENTIFY_IDENTIFY = 0x02000000,
        APP_EVENT_IDENTIFY_STOP     = 0x04000000,
        APP_EVENT_IDENTIFY_MASK     = APP_EVENT_IDENTIFY_START | APP_EVENT_IDENTIFY_IDENTIFY | APP_EVENT_IDENTIFY_STOP,

        APP_EVENT_ALL_MASK =
            APP_EVENT_LIGHTING_MASK | APP_EVENT_INIT_ALL_MASK | APP_EVENT_SYS_ALL_MASK | APP_EVENT_TIMER | APP_EVENT_IDENTIFY_MASK,
    };

    void SetEndpointId(EndpointId endpointId)
    {
        if (mEndpointId != (EndpointId) -1)
            mEndpointId = endpointId;
    }

    EndpointId GetEndpointId(void) { return mEndpointId; }
    void PostEvent(app_event_t event);
    void ButtonEventHandler(uint8_t btnIdx, uint8_t btnAction);

    static void IdentifyStartHandler(Identify *);
    static void IdentifyStopHandler(Identify *);
    static void IdentifyHandleOp(app_event_t event);

private:
    friend AppTask & GetAppTask(void);
    friend void StartAppTask(void);
    friend PlatformManagerImpl;

    static void ChipEventHandler(const ChipDeviceEvent * event, intptr_t arg);
    static uint32_t AppRebootCheck(uint32_t time = 0);

    static void LightingSetOnoff(uint8_t bonoff);
    static void LightingSetStatus(app_event_t status);

    static void LightingSetBleAdv(void);
    static void LightingSetProvisioned(void);
    static void LightingSetFactoryReset(void);

    static void LightingUpdate(app_event_t event = APP_EVENT_NONE);

#if APP_BOARD_LED_STATUS || APP_BOARD_BTN
    static bool StartTimer(void);
    static void CancelTimer(void);
    static void TimerEventHandler(void);
    static void TimerDutyCycle(app_event_t event);
    static void TimerCallback(TimerHandle_t xTimer);
#endif

#if APP_BOARD_BTN
    static void ButtonInit(void);
    static bool ButtonPressed(void);
    static void ButtonEventHandler(void * arg);
#endif

    static void ScheduleInit(intptr_t arg);
    static void AppTaskMain(void * pvParameter);

    static CHIP_ERROR StartAppShellTask();
    static void AppShellTask(void * args);

    EndpointId mEndpointId = (EndpointId) 1;
    TaskHandle_t sAppTaskHandle;
    QueueHandle_t sAppEventQueue;
    TimerHandle_t sTimer;
    uint32_t mBlinkOnTimeMS;
    uint32_t mBlinkOffTimeMS;
    uint64_t buttonPressedTimeout;

    static StackType_t appStack[APP_TASK_STACK_SIZE / sizeof(StackType_t)];
    static StaticTask_t appTaskStruct;
    static AppTask sAppTask;
};

inline AppTask & GetAppTask(void)
{
    return AppTask::sAppTask;
}

void StartAppTask();
