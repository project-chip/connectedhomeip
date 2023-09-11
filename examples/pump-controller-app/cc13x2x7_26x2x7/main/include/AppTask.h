/*
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef APP_TASK_H
#define APP_TASK_H

#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "AppEvent.h"
#include "PumpManager.h"

#include <ti/drivers/apps/Button.h>

struct Identify;

class AppTask
{
public:
    int StartAppTask();
    static void AppTaskMain(void * pvParameter);

    void PostStartActionRequest(int32_t aActor, PumpManager::Action_t aAction);
    void PostEvent(const AppEvent * event);
    void UpdateClusterState();

    static void IdentifyStartHandler(::Identify *);
    static void IdentifyStopHandler(::Identify *);
    static void TriggerIdentifyEffectHandler(::Identify * identify);

private:
    friend AppTask & GetAppTask(void);

    int Init();

    static void ActionInitiated(PumpManager::Action_t aAction, int32_t aActor);
    static void ActionCompleted(PumpManager::Action_t aAction, int32_t aActor);

    void DispatchEvent(AppEvent * event);

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

#endif // APP_TASK_H
