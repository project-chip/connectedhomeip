/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
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
