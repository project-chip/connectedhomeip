/*
<<<<<<< HEAD:examples/lighting-app/cc13x2x7_26x2x7/src/AppTask.h
=======
 *
>>>>>>> refs/tags/v1.0.0.2:examples/lock-app/genio/include/AppTask.h
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
#include "LockManager.h"
#include "filogic_button.h"

<<<<<<< HEAD:examples/lighting-app/cc13x2x7_26x2x7/src/AppTask.h
#include <ti/drivers/apps/Button.h>
=======
#include "FreeRTOS.h"
#include "timers.h" // provides FreeRTOS timer support
#include <platform/CHIPDeviceLayer.h>
>>>>>>> refs/tags/v1.0.0.2:examples/lock-app/genio/include/AppTask.h

// Application-defined error codes in the CHIP_ERROR space.
#define APP_ERROR_EVENT_QUEUE_FAILED CHIP_APPLICATION_ERROR(0x01)
#define APP_ERROR_CREATE_TASK_FAILED CHIP_APPLICATION_ERROR(0x02)
#define APP_ERROR_UNHANDLED_EVENT CHIP_APPLICATION_ERROR(0x03)
#define APP_ERROR_CREATE_TIMER_FAILED CHIP_APPLICATION_ERROR(0x04)
#define APP_ERROR_START_TIMER_FAILED CHIP_APPLICATION_ERROR(0x05)
#define APP_ERROR_STOP_TIMER_FAILED CHIP_APPLICATION_ERROR(0x06)
#define APP_ERROR_ALLOCATION_FAILED CHIP_APPLICATION_ERROR(0x07)
<<<<<<< HEAD:examples/lighting-app/cc13x2x7_26x2x7/src/AppTask.h

struct Identify;
=======
>>>>>>> refs/tags/v1.0.0.2:examples/lock-app/genio/include/AppTask.h

class AppTask
{

public:
    int StartAppTask();
    static void AppTaskMain(void * pvParameter);

<<<<<<< HEAD:examples/lighting-app/cc13x2x7_26x2x7/src/AppTask.h
    static AppTask & GetAppTask() { return sAppTask; }

    void PostLightActionRequest(int32_t aActor, LightingManager::Action_t aAction);
    void PostEvent(const AppEvent * event);

    static void IdentifyStartHandler(::Identify *);
    static void IdentifyStopHandler(::Identify *);
    static void TriggerIdentifyEffectHandler(::Identify * identify);
=======
    void ActionRequest(int32_t aActor, LockManager::Action_t aAction);
    void PostEvent(const AppEvent * event);

    void ButtonHandler(const filogic_button_t & button);
>>>>>>> refs/tags/v1.0.0.2:examples/lock-app/genio/include/AppTask.h

private:
    friend AppTask & GetAppTask(void);

    int Init();

<<<<<<< HEAD:examples/lighting-app/cc13x2x7_26x2x7/src/AppTask.h
    static void ActionInitiated(LightingManager::Action_t aAction, int32_t aActor);
    static void ActionCompleted(LightingManager::Action_t aAction);
=======
    static void ActionInitiated(LockManager::Action_t aAction, int32_t aActor);
    static void ActionCompleted(LockManager::Action_t aAction);
>>>>>>> refs/tags/v1.0.0.2:examples/lock-app/genio/include/AppTask.h

    void DispatchEvent(AppEvent * event);
    static void UpdateClusterState(intptr_t context);
    static void SingleButtonEventHandler(AppEvent * aEvent);
    static void ButtonTimerEventHandler(AppEvent * aEvent);

<<<<<<< HEAD:examples/lighting-app/cc13x2x7_26x2x7/src/AppTask.h
    static void ButtonLeftEventHandler(Button_Handle handle, Button_EventMask events);
    static void ButtonRightEventHandler(Button_Handle handle, Button_EventMask events);
    static void TimerEventHandler(void * p_context);
=======
    static void LockActionEventHandler(AppEvent * aEvent);

    static void TimerEventHandler(TimerHandle_t xTimer);
    void StartTimer(uint32_t aTimeoutMs);
    void CancelTimer(void);
>>>>>>> refs/tags/v1.0.0.2:examples/lock-app/genio/include/AppTask.h

    enum Function_t
    {
        kFunction_NoneSelected   = 0,
<<<<<<< HEAD:examples/lighting-app/cc13x2x7_26x2x7/src/AppTask.h
        kFunction_SoftwareUpdate = 0,
        kFunction_FactoryReset,
=======
        kFunction_StartBleAdv    = 1,
        kFunction_LightSwitch    = 2,
        kFunction_FactoryReset   = 3,
        kFunction_SoftwareUpdate = 0,
>>>>>>> refs/tags/v1.0.0.2:examples/lock-app/genio/include/AppTask.h

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
