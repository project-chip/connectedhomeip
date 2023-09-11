/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "AppTaskCommon.h"
#include "BoltLockManager.h"

class AppTask : public AppTaskCommon
{
public:
    void UpdateClusterState(BoltLockManager::State state, BoltLockManager::OperationSource source);

private:
    friend AppTask & GetAppTask(void);
    friend class AppTaskCommon;

    CHIP_ERROR Init(void);

    static void LockActionEventHandler(AppEvent * event);
    static void LockStateChanged(BoltLockManager::State state, BoltLockManager::OperationSource source);

    static AppTask sAppTask;
};

inline AppTask & GetAppTask(void)
{
    return AppTask::sAppTask;
}
