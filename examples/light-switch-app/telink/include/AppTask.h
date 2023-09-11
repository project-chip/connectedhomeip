/*
 *
 * SPDX-FileCopyrightText: 2022-2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "AppTaskCommon.h"

class AppTask : public AppTaskCommon
{
public:
    void UpdateClusterState();

private:
    friend AppTask & GetAppTask(void);
    friend class AppTaskCommon;

    CHIP_ERROR Init(void);

    static void SwitchActionEventHandler(AppEvent * aEvent);

    static AppTask sAppTask;
};

inline AppTask & GetAppTask(void)
{
    return AppTask::sAppTask;
}
