/*
 *
 *    Copyright (c) 2022-2023 Project CHIP Authors
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

#include "DoorWindowManager.h"
#include "FreeRTOS.h"
#include "timers.h" // provides FreeRTOS timer support
#include <platform/CHIPDeviceLayer.h>

#include <platform/qpg/FactoryDataProvider.h>

#define APP_NAME "DoorWindow-Sensor-app"

// Application-defined error codes in the CHIP_ERROR space.
#define APP_ERROR_UNHANDLED_EVENT CHIP_APPLICATION_ERROR(0x03)

class AppTask
{

public:
    CHIP_ERROR Init();
    CHIP_ERROR StartAppTask();
    static void AppTaskMain(void * pvParameter);

    void PostDoorWindowActionRequest(DoorWindowManager::Action aAction);
    void PostEvent(const AppEvent * event);

    void UpdateClusterState();
    void UpdateDeviceState(void);
    bool IsSyncClusterToButtonAction(void);
    void SetSyncClusterToButtonAction(bool value);

private:
    friend AppTask & GetAppTask(void);

    static void InitServer(intptr_t arg);
    static void OpenCommissioning(intptr_t arg);
    static void DoorWindowStateInit(intptr_t arg);

    void DispatchEvent(AppEvent * event);

    static void OnStateChanged(DoorWindowManager::State aState);
    static void DoorWindowLedSet(bool state);

    static void UpdateClusterStateInternal(intptr_t arg);
    static void UpdateDeviceStateInternal(intptr_t arg);

    static void FunctionTimerEventHandler(AppEvent * aEvent);
    static void FunctionHandler(AppEvent * aEvent);

    static void ButtonEventHandler(uint8_t btnIdx, bool btnPressed);
    static void TimerEventHandler(chip::System::Layer * aLayer, void * aAppState);
    static void TotalHoursTimerHandler(chip::System::Layer * aLayer, void * aAppState);

    static void MatterEventHandler(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg);
    static void UpdateLEDs(void);

    void StartTimer(uint32_t aTimeoutMs);
    void CancelTimer(void);
    static void DoorWindowActionEventHandler(AppEvent * aEvent);

    enum Function_t
    {
        kFunction_NoneSelected   = 0,
        kFunction_SoftwareUpdate = 1,
        kFunction_FactoryReset   = 2,
#ifdef GP_UPGRADE_DIVERSITY_DUAL_BOOT
        kFunction_FactoryResetTrigger = 3,
        kFunction_StackSwitch         = 4,
#endif // GP_UPGRADE_DIVERSITY_DUAL_BOOT

        kFunction_Invalid
    } Function;

    Function_t mFunction;
    bool mFunctionTimerActive;
    bool mSyncClusterToButtonAction = false;

    chip::DeviceLayer::FactoryDataProvider mFactoryDataProvider;

    static AppTask sAppTask;
};

inline AppTask & GetAppTask(void)
{
    return AppTask::sAppTask;
}

inline bool AppTask::IsSyncClusterToButtonAction(void)
{
    return mSyncClusterToButtonAction;
}

inline void AppTask::SetSyncClusterToButtonAction(bool value)
{
    mSyncClusterToButtonAction = value;
}

#endif // APP_TASK_H
