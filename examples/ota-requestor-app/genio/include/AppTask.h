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

#include "FreeRTOS.h"
#include "timers.h" // provides FreeRTOS timer support
#include <platform/CHIPDeviceLayer.h>

// Application-defined error codes in the CHIP_ERROR space.
#define APP_ERROR_CREATE_TASK_FAILED CHIP_APPLICATION_ERROR(0x01)
#define APP_ERROR_CREATE_TIMER_FAILED CHIP_APPLICATION_ERROR(0x02)
#define APP_ERROR_START_TIMER_FAILED CHIP_APPLICATION_ERROR(0x03)
#define APP_ERROR_STOP_TIMER_FAILED CHIP_APPLICATION_ERROR(0x04)

class AppTask
{

public:
    CHIP_ERROR StartAppTask();
    static void AppTaskMain(void * pvParameter);

private:
    friend AppTask & GetAppTask(void);

    CHIP_ERROR Init();

    static AppTask sAppTask;
};

inline AppTask & GetAppTask(void)
{
    return AppTask::sAppTask;
}
