/*
 *
 * SPDX-FileCopyrightText: 2022 Texas Instruments Incorporated
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include <platform/CHIPDeviceLayer.h>

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
