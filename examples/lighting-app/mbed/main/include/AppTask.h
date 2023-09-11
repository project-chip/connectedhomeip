/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "AppEvent.h"
#include "LightingManager.h"
#include <DFUManager.h>

class AppTask
{
public:
    int StartApp();

    void PostLightingActionRequest(LightingManager::Action_t aAction);
    void PostEvent(AppEvent * aEvent);
    void UpdateClusterState(void);

    void ButtonEventHandler(uint32_t id, bool pushed);

private:
    friend AppTask & GetAppTask(void);

    int Init();

    static void ActionInitiated(LightingManager::Action_t aAction, int32_t aActor);
    static void ActionCompleted(LightingManager::Action_t aAction, int32_t aActor);

    void CancelTimer(void);

    void DispatchEvent(const AppEvent * event);

    static void FunctionTimerEventHandler(AppEvent * aEvent);
    static void FunctionHandler(AppEvent * aEvent);
    static void LightingActionEventHandler(AppEvent * aEvent);

    void LightingButtonPressEventHandler(void);
    void FunctionButtonPressEventHandler(void);
    void FunctionButtonReleaseEventHandler(void);
    void SliderEventHandler(int slider_pos);
    void TimerEventHandler(void);

    void StartTimer(uint32_t aTimeoutInMs);

    enum Function_t
    {
        kFunction_NoneSelected   = 0,
        kFunction_SoftwareUpdate = 0,
        kFunction_StartBleAdv    = 1,
        kFunction_FactoryReset,

        kFunction_Invalid
    };

    Function_t mFunction;
    bool mFunctionTimerActive;
    static AppTask sAppTask;
};

inline AppTask & GetAppTask(void)
{
    return AppTask::sAppTask;
}
