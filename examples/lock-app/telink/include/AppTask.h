/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "AppConfig.h"
#include "AppTaskCommon.h"
#include "LockManager.h"

#define APP_ERROR_EVENT_QUEUE_FAILED CHIP_APPLICATION_ERROR(0x01)
#define APP_ERROR_CREATE_TASK_FAILED CHIP_APPLICATION_ERROR(0x02)
#define APP_ERROR_UNHANDLED_EVENT CHIP_APPLICATION_ERROR(0x03)
#define APP_ERROR_CREATE_TIMER_FAILED CHIP_APPLICATION_ERROR(0x04)
#define APP_ERROR_START_TIMER_FAILED CHIP_APPLICATION_ERROR(0x05)
#define APP_ERROR_STOP_TIMER_FAILED CHIP_APPLICATION_ERROR(0x06)
#define APP_ERROR_ALLOCATION_FAILED CHIP_APPLICATION_ERROR(0x07)

class AppTask : public AppTaskCommon
{
private:
    friend AppTask & GetAppTask(void);
    friend class AppTaskCommon;

    CHIP_ERROR Init(void);

    static void LockActionEventHandler(AppEvent * event);
    static void LockStateChanged(LockManager::State_t state);
    static void LockJammedEventHandler(void);
    static void LockJammedActionHandler(AppEvent * aEvent);
    static void LockStateEventHandler(void);
    static void LockStateActionHandler(AppEvent * aEvent);

    static AppTask sAppTask;
};

inline AppTask & GetAppTask(void)
{
    return AppTask::sAppTask;
}
