/*
 *
 *    Copyright (c) 2020 Google LLC.
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

#include "AppEvent.h"
#include "BoltLockManager.h"
#include "K32W0FactoryDataProvider.h"

#include <platform/CHIPDeviceLayer.h>

#include "FreeRTOS.h"
#include "timers.h"

class AppTask
{
public:
    CHIP_ERROR StartAppTask();
    static void AppTaskMain(void * pvParameter);

    void PostLockActionRequest(int32_t aActor, BoltLockManager::Action_t aAction);
    void PostEvent(const AppEvent * event);

    void UpdateClusterState(void);

private:
    friend AppTask & GetAppTask(void);

    CHIP_ERROR Init();

    static void ActionInitiated(BoltLockManager::Action_t aAction, int32_t aActor);
    static void ActionCompleted(BoltLockManager::Action_t aAction);

    void CancelTimer(void);

    void DispatchEvent(AppEvent * event);

    static void FunctionTimerEventHandler(void * aGenericEvent);
    static void KBD_Callback(uint8_t events);
    static void HandleKeyboard(void);
    static void JoinHandler(void * aGenericEvent);
    static void BleHandler(void * aGenericEvent);
    static void BleStartAdvertising(intptr_t arg);
    static void LockActionEventHandler(void * aGenericEvent);
    static void ResetActionEventHandler(void * aGenericEvent);
    static void InstallEventHandler(void * aGenericEvent);

    static void ButtonEventHandler(uint8_t pin_no, uint8_t button_action);
    static void TimerEventHandler(TimerHandle_t xTimer);

    static void MatterEventHandler(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg);
    static void UpdateClusterStateInternal(intptr_t arg);
    static void ThreadStart();
    static void InitServer(intptr_t arg);
    void StartTimer(uint32_t aTimeoutInMs);

    enum Function_t
    {
        kFunction_NoneSelected   = 0,
        kFunction_SoftwareUpdate = 0,
        kFunction_FactoryReset,
        kFunctionLockUnlock,

        kFunction_Invalid
    } Function;

    Function_t mFunction            = kFunction_NoneSelected;
    bool mResetTimerActive          = false;
    bool mSyncClusterToButtonAction = false;

    static AppTask sAppTask;
};

inline AppTask & GetAppTask(void)
{
    return AppTask::sAppTask;
}
