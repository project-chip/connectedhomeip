/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "AppTaskCommon.h"
#include "PumpManager.h"

class AppTask : public AppTaskCommon
{
public:
    void UpdateClusterState();

    static void PostStartActionRequest(int32_t actor, PumpManager::Action_t action);

private:
    friend AppTask & GetAppTask(void);
    friend class AppTaskCommon;

    CHIP_ERROR Init(void);

    static void ActionInitiated(PumpManager::Action_t action, int32_t actor);
    static void ActionCompleted(PumpManager::Action_t action, int32_t actor);

    static void StartActionEventHandler(AppEvent * aEvent);

    static AppTask sAppTask;
};

inline AppTask & GetAppTask(void)
{
    return AppTask::sAppTask;
}
