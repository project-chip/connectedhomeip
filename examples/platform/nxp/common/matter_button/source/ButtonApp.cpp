/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "ButtonApp.h"
#include "AppTaskBase.h"

extern "C" {
#include "app.h"
#include "board_comp.h"
}

CHIP_ERROR chip::NXP::App::ButtonApp::Init()
{
    // Button is initialized in otSysInit, when APP_InitServices is called.
    // Overwrite the handle to reference the SDK handle.
    handle = &g_buttonHandle[1];

    return CHIP_NO_ERROR;
}

void chip::NXP::App::ButtonApp::HandleShortPress()
{
    chip::DeviceLayer::PlatformMgr().ScheduleWork(
        [](intptr_t arg) {
            auto status = chip::NXP::App::GetAppTask().ProcessSetStateClusterHandler();
            if (status != CHIP_NO_ERROR)
            {
                ChipLogProgress(DeviceLayer, "Error when updating cluster attribute");
            }
        },
        (intptr_t) nullptr);
}

void chip::NXP::App::ButtonApp::HandleLongPress()
{
    chip::DeviceLayer::PlatformMgrImpl().CleanReset();
}

void chip::NXP::App::ButtonApp::HandleDoubleClick()
{
    /* Currently not mapped to any action */
}
