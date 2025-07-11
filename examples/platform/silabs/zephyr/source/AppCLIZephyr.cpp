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

#include "AppCLIZephyr.h"
#include "AppTaskBase.h"
#include <lib/shell/Engine.h>

void chip::Zephyr::App::AppCLIZephyr::ResetCmdHandle(void)
{
    chip::DeviceLayer::PlatformMgr().Shutdown();
}

// This returns an instance of this class.
chip::Zephyr::App::AppCLIZephyr & chip::Zephyr::App::AppCLIZephyr::GetDefaultInstance()
{
    static chip::Zephyr::App::AppCLIZephyr sAppCLI;
    return sAppCLI;
}

chip::Zephyr::App::AppCLIBase & chip::Zephyr::App::GetAppCLI()
{
    return chip::Zephyr::App::AppCLIZephyr::GetDefaultInstance();
}

CHIP_ERROR chip::Zephyr::App::AppCLIZephyr::Init(void)
{
    if (!isShellInitialized)
    {
        RegisterDefaultCommands();

        isShellInitialized = true;
    }
    return CHIP_NO_ERROR;
}
