/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "AppTaskCommon.h"
#include "WindowCovering.h"

#ifdef CONFIG_CHIP_PW_RPC
#include "Rpc.h"
#endif

class AppTask : public AppTaskCommon
{
private:
#ifdef CONFIG_CHIP_PW_RPC
    friend class chip::rpc::TelinkButton;
#endif
    friend AppTask & GetAppTask(void);
    friend class AppTaskCommon;

    CHIP_ERROR Init(void);

    static void OpenActionAndToggleMoveTypeButtonEventHandler(void);
    static void CloseActionButtonEventHandler(void);

    static void OpenTimerTimeoutCallback(k_timer * timer);
    static void OpenTimerEventHandler(AppEvent * aEvent);
    static void ToggleMoveTypeHandler(AppEvent * aEvent);

    static void OpenHandler(AppEvent * aEvent);
    static void CloseHandler(AppEvent * aEvent);
    static void ToggleMoveType();

    OperationalState mMoveType{ OperationalState::MovingUpOrOpen };

    static AppTask sAppTask;
};

inline AppTask & GetAppTask(void)
{
    return AppTask::sAppTask;
}
