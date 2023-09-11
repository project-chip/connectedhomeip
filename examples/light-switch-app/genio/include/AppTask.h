/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "AppEvent.h"
#include "filogic_button.h"

#include "FreeRTOS.h"
#include "timers.h" // provides FreeRTOS timer support
#include <platform/CHIPDeviceLayer.h>

// Application-defined error codes in the CHIP_ERROR space.
#define APP_ERROR_EVENT_QUEUE_FAILED CHIP_APPLICATION_ERROR(0x01)
#define APP_ERROR_CREATE_TASK_FAILED CHIP_APPLICATION_ERROR(0x02)
#define APP_ERROR_UNHANDLED_EVENT CHIP_APPLICATION_ERROR(0x03)
#define APP_ERROR_CREATE_TIMER_FAILED CHIP_APPLICATION_ERROR(0x04)
#define APP_ERROR_START_TIMER_FAILED CHIP_APPLICATION_ERROR(0x05)
#define APP_ERROR_STOP_TIMER_FAILED CHIP_APPLICATION_ERROR(0x06)
#define APP_ERROR_BHANDLER_INIT_FAILED CHIP_APPLICATION_ERROR(0x07)

class AppTask
{

public:
    CHIP_ERROR StartAppTask();
    static void AppTaskMain(void * pvParameter);

    void PostEvent(const AppEvent * event);

    void ButtonHandler(const filogic_button_t & button);

private:
    friend AppTask & GetAppTask(void);

    CHIP_ERROR Init();

    void DispatchEvent(AppEvent * event);

    static void SingleButtonEventHandler(AppEvent * aEvent);
    static void ButtonTimerEventHandler(AppEvent * aEvent);

    static void LightSwitchActionEventHandler(AppEvent * aEvent);

    static void TimerEventHandler(TimerHandle_t xTimer);
    void StartTimer(uint32_t aTimeoutMs);
    void CancelTimer(void);

    enum Function_t
    {
        kFunction_NoneSelected   = 0,
        kFunction_StartBleAdv    = 1,
        kFunction_LightSwitch    = 2,
        kFunction_FactoryReset   = 3,
        kFunction_SoftwareUpdate = 0,

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
