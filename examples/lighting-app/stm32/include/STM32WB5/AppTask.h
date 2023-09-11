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
#include "app_entry.h"

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
    static void DelayNvmHandler(TimerHandle_t xTimer);
    static void MatterEventHandler(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg);
    static void UpdateLCD(void);
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
