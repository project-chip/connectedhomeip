/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef APP_TASK_H
#define APP_TASK_H

#include <stdbool.h>
#include <stdint.h>

#include "AppEvent.h"
#include "LightingManager.h"

#include "FreeRTOS.h"
#include "timers.h" // provides FreeRTOS timer support
#include <platform/CHIPDeviceLayer.h>

#include <platform/qpg/FactoryDataProvider.h>

#define APP_NAME "Lighting-app"

class AppTask
{

public:
    CHIP_ERROR Init();
    CHIP_ERROR StartAppTask();
    static void AppTaskMain(void * pvParameter);

    void PostLightActionRequest(int32_t aActor, LightingManager::Action_t aAction);
    void PostEvent(const AppEvent * event);
    void UpdateClusterState();

    static void ButtonEventHandler(uint8_t btnIdx, bool btnPressed);

private:
    friend AppTask & GetAppTask(void);

    static void InitServer(intptr_t arg);
    static void OpenCommissioning(intptr_t arg);

    static void ActionInitiated(LightingManager::Action_t aAction);
    static void ActionCompleted(LightingManager::Action_t aAction);

    void DispatchEvent(AppEvent * event);

    static void FunctionTimerEventHandler(AppEvent * aEvent);
    static void FunctionHandler(AppEvent * aEvent);

    static void LightingActionEventHandler(AppEvent * aEvent);
    static void TimerEventHandler(chip::System::Layer * aLayer, void * aAppState);

    static void MatterEventHandler(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg);
    static void UpdateLEDs(void);

    void StartTimer(uint32_t aTimeoutMs);
    void CancelTimer(void);

    enum Function_t
    {
        kFunction_NoneSelected   = 0,
        kFunction_SoftwareUpdate = 1,
        kFunction_FactoryReset   = 2,
        kFunction_StartBleAdv    = 3,

        kFunction_Invalid
    } Function;

    Function_t mFunction;
    bool mFunctionTimerActive;
    bool mSyncClusterToButtonAction;

    chip::DeviceLayer::FactoryDataProvider mFactoryDataProvider;

    static AppTask sAppTask;
};

inline AppTask & GetAppTask(void)
{
    return AppTask::sAppTask;
}

#endif // APP_TASK_H
