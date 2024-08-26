/*
 *
 *    Copyright (c) 2022-2024 Project CHIP Authors
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

#include "AppCLIFreeRTOS.h"
#include "AppTaskBase.h"
#include <ChipShellCollection.h>
#include <lib/shell/Engine.h>
#include <platform/CHIPDeviceLayer.h>

#define MATTER_CLI_TASK_SIZE ((configSTACK_DEPTH_TYPE) 2048 / sizeof(portSTACK_TYPE))

TaskHandle_t AppMatterCliTaskHandle;

void AppMatterCliTask(void * args)
{
    chip::Shell::Engine::Root().RunMainLoop();
}

// This returns an instance of this class.
chip::NXP::App::AppCLIFreeRTOS & chip::NXP::App::AppCLIFreeRTOS::GetDefaultInstance()
{
    static chip::NXP::App::AppCLIFreeRTOS sAppCLI;
    return sAppCLI;
}

chip::NXP::App::AppCLIBase & chip::NXP::App::GetAppCLI()
{
    return chip::NXP::App::AppCLIFreeRTOS::GetDefaultInstance();
}

void chip::NXP::App::AppCLIFreeRTOS::ResetCmdHandle(void)
{
    /*
       Shutdown device before reboot,
       this emits the ShutDown event, handles the server shutting down,
       and stores in flash the total-operational-hours value.
    */
    chip::DeviceLayer::PlatformMgr().Shutdown();
    chip::DeviceLayer::PlatformMgrImpl().ScheduleResetInIdle();
}

CHIP_ERROR chip::NXP::App::AppCLIFreeRTOS::Init(void)
{
    if (!isShellInitialized)
    {
        int error = chip::Shell::Engine::Root().Init();

        if (error != 0)
        {
            ChipLogError(Shell, "Streamer initialization failed: %d", error);
            return CHIP_ERROR_INTERNAL;
        }

        RegisterDefaultCommands();

        if (xTaskCreate(&AppMatterCliTask, "AppMatterCli_task", MATTER_CLI_TASK_SIZE, NULL, 1, &AppMatterCliTaskHandle) != pdPASS)
        {
            ChipLogError(Shell, "Failed to start Matter CLI task");
            return CHIP_ERROR_INTERNAL;
        }

        isShellInitialized = true;
    }
    return CHIP_NO_ERROR;
}
