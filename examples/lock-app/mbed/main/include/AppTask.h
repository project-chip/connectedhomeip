/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "AppEvent.h"
#include "BoltLockManager.h"
#include <DFUManager.h>

class AppTask
{
public:
    int StartApp();

    void PostEvent(AppEvent * aEvent);
    void UpdateClusterState(void);

    void ButtonEventHandler(uint32_t id, bool pushed);

private:
    friend AppTask & GetAppTask(void);

    int Init();

    static void ActionInitiated(BoltLockManager::Action_t aAction, int32_t aActor);
    static void ActionCompleted(BoltLockManager::Action_t aAction, int32_t aActor);

    void CancelTimer(void);

    void DispatchEvent(const AppEvent * event);

    static void FunctionTimerEventHandler(AppEvent * aEvent);
    static void FunctionHandler(AppEvent * aEvent);
    static void LockActionEventHandler(AppEvent * aEvent);

    void LockButtonPressEventHandler(void);
    void FunctionButtonPressEventHandler(void);
    void FunctionButtonReleaseEventHandler(void);
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
