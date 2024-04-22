/*
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

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "AppEvent.h"
#include "LightingManager.h"

#include <ti/drivers/apps/Button.h>

#ifdef CC13XX_26XX_FACTORY_DATA
#include <platform/cc13xx_26xx/FactoryDataProvider.h>
#endif

// Application-defined error codes in the CHIP_ERROR space.
#define APP_ERROR_EVENT_QUEUE_FAILED CHIP_APPLICATION_ERROR(0x01)
#define APP_ERROR_CREATE_TASK_FAILED CHIP_APPLICATION_ERROR(0x02)
#define APP_ERROR_UNHANDLED_EVENT CHIP_APPLICATION_ERROR(0x03)
#define APP_ERROR_CREATE_TIMER_FAILED CHIP_APPLICATION_ERROR(0x04)
#define APP_ERROR_START_TIMER_FAILED CHIP_APPLICATION_ERROR(0x05)
#define APP_ERROR_STOP_TIMER_FAILED CHIP_APPLICATION_ERROR(0x06)
#define APP_ERROR_ALLOCATION_FAILED CHIP_APPLICATION_ERROR(0x07)
struct Identify;

class AppTask
{

public:
    int StartAppTask();
    static void AppTaskMain(void * pvParameter);

    static AppTask & GetAppTask() { return sAppTask; }

    void PostLightActionRequest(int32_t aActor, LightingManager::Action_t aAction);
    void PostEvent(const AppEvent * event);

    static void IdentifyStartHandler(::Identify *);
    static void IdentifyStopHandler(::Identify *);
    static void TriggerIdentifyEffectHandler(::Identify * identify);

private:
    friend AppTask & GetAppTask(void);

    int Init();

    static void ActionInitiated(LightingManager::Action_t aAction, int32_t aActor);
    static void ActionCompleted(LightingManager::Action_t aAction);

    void DispatchEvent(AppEvent * event);
    void uiInit(void);
    static void UpdateClusterState(intptr_t context);
    static void SingleButtonEventHandler(AppEvent * aEvent);
    static void ButtonTimerEventHandler(AppEvent * aEvent);

    static void ButtonLeftEventHandler(Button_Handle handle, Button_EventMask events);
    static void ButtonRightEventHandler(Button_Handle handle, Button_EventMask events);
    static void TimerEventHandler(void * p_context);

    enum Function_t
    {
        kFunction_NoneSelected   = 0,
        kFunction_SoftwareUpdate = 0,
        kFunction_FactoryReset,
        kFunction_Invalid
    } Function;

    Function_t mFunction;
    bool mFunctionTimerActive;

    static AppTask sAppTask;

#ifdef CC13XX_26XX_FACTORY_DATA
    chip::DeviceLayer::FactoryDataProvider mFactoryDataProvider;
#endif
};

inline AppTask & GetAppTask(void)
{
    return AppTask::sAppTask;
}
