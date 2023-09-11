/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <platform/CHIPDeviceLayer.h>

#define APP_NAME "Shell-app"

class AppTask
{

public:
    CHIP_ERROR Init();
    CHIP_ERROR StartAppTask();

private:
    friend AppTask & GetAppTask(void);

    static AppTask sAppTask;
};

inline AppTask & GetAppTask(void)
{
    return AppTask::sAppTask;
}
