/*
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
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
};

inline AppTask & GetAppTask(void)
{
    return AppTask::sAppTask;
}
