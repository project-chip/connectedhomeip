/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "AppEvent.h"
#include "LightingManager.h"

#include "FreeRTOS.h"
#include "timers.h" // provides FreeRTOS timer support
#include <ble/BLEEndPoint.h>
#include <platform/CHIPDeviceLayer.h>

// Application-defined error codes in the CHIP_ERROR space.
#define APP_ERROR_EVENT_QUEUE_FAILED CHIP_APPLICATION_ERROR(0x01)
#define APP_ERROR_CREATE_TASK_FAILED CHIP_APPLICATION_ERROR(0x02)
#define APP_ERROR_UNHANDLED_EVENT CHIP_APPLICATION_ERROR(0x03)
#define APP_ERROR_CREATE_TIMER_FAILED CHIP_APPLICATION_ERROR(0x04)
#define APP_ERROR_START_TIMER_FAILED CHIP_APPLICATION_ERROR(0x05)
#define APP_ERROR_STOP_TIMER_FAILED CHIP_APPLICATION_ERROR(0x06)

class AppTask
{

public:
    CHIP_ERROR StartAppTask();
    static void AppTaskMain(void * pvParameter);

    void PostLightActionRequest(int32_t actor, LightingManager::Action_t action);
    void PostEvent(const AppEvent * event);

    void ButtonEventHandler(uint8_t btnIdx, uint8_t btnAction);
    void InitOTARequestor();

private:
    friend AppTask & GetAppTask(void);

    CHIP_ERROR Init();

    static void ActionInitiated(LightingManager::Action_t action, int32_t actor);
    static void ActionCompleted(LightingManager::Action_t action);

    void CancelTimer(void);

    void DispatchEvent(AppEvent * event);

    static void FunctionTimerEventHandler(AppEvent * event);
    static void FunctionHandler(AppEvent * event);
    static void LightActionEventHandler(AppEvent * event);
    static void TimerEventHandler(TimerHandle_t timer);

    static void UpdateClusterState(intptr_t context);

    void StartTimer(uint32_t aTimeoutMs);

    enum class Function
    {
        kNoneSelected = 0,
        kFactoryReset = 1,
        kInvalid
    };

    Function mFunction              = Function::kNoneSelected;
    bool mFunctionTimerActive       = false;
    bool mSyncClusterToButtonAction = false;

    static AppTask sAppTask;
};

inline AppTask & GetAppTask(void)
{
    return AppTask::sAppTask;
}
