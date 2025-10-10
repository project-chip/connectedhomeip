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

/**
 * @brief Flag to describe if the button handles are predefined in SDK.
 *
 * By default assuming button handles are predefined in the SDK
 * If not, flag CHIP_APP_BUTTON_HANDLE_SDK_NOT_PREDEFINED should be defined
 * or set to 1
 */
#ifndef CONFIG_CHIP_APP_BUTTON_HANDLE_SDK_NOT_PREDEFINED
#define CONFIG_CHIP_APP_BUTTON_HANDLE_SDK_NOT_PREDEFINED 0
#endif

#if CONFIG_CHIP_APP_BUTTON_HANDLE_SDK_NOT_PREDEFINED
BUTTON_HANDLE_ARRAY_DEFINE(g_buttonHandle, gAppButtonCnt_c);
#endif

CHIP_ERROR chip::NXP::App::ButtonApp::Init()
{
#if !CONFIG_CHIP_APP_BUTTON_HANDLE_SDK_NOT_PREDEFINED
    // Button is defined in the SDK and initialized in otSysInit, when APP_InitServices is called.
    handle = &g_buttonHandle[1];
#else
    // Button handle is defined in this file and it should be initialized here.
    handle = &g_buttonHandle[0];
    BOARD_InitButton0(handle);
#endif

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
    // Execute "clean" reset
    chip::DeviceLayer::PlatformMgr().ScheduleWork(
        [](intptr_t arg) {
            chip::DeviceLayer::PlatformMgr().StopEventLoopTask();
            chip::DeviceLayer::PlatformMgr().Shutdown();
            chip::DeviceLayer::PlatformMgrImpl().Reset();
        },
        (intptr_t) nullptr);
}

void chip::NXP::App::ButtonApp::HandleDoubleClick()
{
    /* Currently not mapped to any action */
}
