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

#ifndef APP_TASK_H
#define APP_TASK_H

#include <stdbool.h>
#include <stdint.h>

#include "AppEvent.h"
#include "LightingManager.h"
#include "app_entry.h"

#include <DeviceInfoProviderImpl.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/stm32/FactoryDataProvider.h>
#define APP_NAME "Lighting-app"

class AppTask
{

public:
    CHIP_ERROR StartAppTask();
    CHIP_ERROR Init();
    static void AppTaskMain(void * pvParameter);
    void PostLightActionRequest(int32_t aActor, LightingManager::Action_t aAction);
    void PostEvent(const AppEvent * event);
    void UpdateClusterState();
    CHIP_ERROR InitMatter(void);
    static void ButtonEventHandler(Push_Button_st * Button);

protected:
    TaskHandle_t mAppTask = NULL;

private:
    friend AppTask & GetAppTask(void);
    static void ActionInitiated(LightingManager::Action_t aAction);
    static void ActionCompleted(LightingManager::Action_t aAction);
    void CancelTimer(void);
    void DispatchEvent(AppEvent * event);
    static void FunctionHandler(AppEvent * aEvent);
    static void LightingActionEventHandler(AppEvent * aEvent);
    static void TimerEventHandler(TimerHandle_t xTimer);
    static void MatterEventHandler(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg);
#if (OTA_SUPPORT == 0)
    static void UpdateLCD(void);
#endif
    static void UpdateNvmEventHandler(AppEvent * aEvent);

    enum Function_t
    {
        kFunction_NoneSelected   = 0,
        kFunction_SoftwareUpdate = 0,
        kFunction_Joiner         = 1,
        kFunction_SaveNvm        = 2,
        kFunction_FactoryReset   = 3,

        kFunction_Invalid
    } Function;

    Function_t mFunction;
    bool mFunctionTimerActive;
    bool mSyncClusterToButtonAction;
    // chip::Ble::BLEEndPoint * mBLEEndPoint;

    static AppTask sAppTask;
};

inline AppTask & GetAppTask(void)
{
    return AppTask::sAppTask;
}

#endif // APP_TASK_H
