/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "AppTaskCommon.h"
#include "PWMDevice.h"

class AppTask : public AppTaskCommon
{
public:
    void UpdateClusterState(void);
    static void InitServer(intptr_t context);
    PWMDevice & GetPWMDevice(void) { return mPwmRgbBlueLed; }

private:
    friend AppTask & GetAppTask(void);
    friend class AppTaskCommon;

    CHIP_ERROR Init(void);

    static void ActionInitiated(PWMDevice::Action_t aAction, int32_t aActor);
    static void ActionCompleted(PWMDevice::Action_t aAction, int32_t aActor);

    static void LightingActionEventHandler(AppEvent * aEvent);

    PWMDevice mPwmRgbBlueLed;

    static AppTask sAppTask;
};

inline AppTask & GetAppTask(void)
{
    return AppTask::sAppTask;
}
