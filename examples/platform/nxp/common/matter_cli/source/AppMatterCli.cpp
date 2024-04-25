/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *    Copyright 2023-2024 NXP
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

#include "AppMatterCli.h"
#include "AppTaskBase.h"
#include <app/server/Server.h>
#include <cstring>
#include <platform/CHIPDeviceLayer.h>

#ifdef CONFIG_CHIP_APP_DEVICE_TYPE_LAUNDRY_WASHER
#include <operational-state-delegate-impl.h>
#endif /* CONFIG_CHIP_APP_DEVICE_TYPE_LAUNDRY_WASHER */

#ifdef ENABLE_CHIP_SHELL
#include "task.h"
#include <ChipShellCollection.h>
#include <lib/shell/Engine.h>

#define MATTER_CLI_TASK_SIZE ((configSTACK_DEPTH_TYPE) 2048 / sizeof(portSTACK_TYPE))
#define MATTER_CLI_LOG(message) (streamer_printf(streamer_get(), message))

using namespace chip::Shell;
TaskHandle_t AppMatterCliTaskHandle;
static bool isShellInitialized = false;
#else
#define MATTER_CLI_LOG(...)
#endif /* ENABLE_CHIP_SHELL */

using namespace chip;
using namespace chip::app::Clusters;

void AppMatterCliTask(void * args)
{
#ifdef ENABLE_CHIP_SHELL
    Engine::Root().RunMainLoop();
#endif /* ENABLE_CHIP_SHELL */
}

/* Application Matter CLI commands */

CHIP_ERROR commissioningManager(int argc, char * argv[])
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    if (strncmp(argv[0], "on", 2) == 0)
    {
        chip::NXP::App::GetAppTask().StartCommissioningHandler();
    }
    else if (strncmp(argv[0], "off", 3) == 0)
    {
        chip::NXP::App::GetAppTask().StopCommissioningHandler();
    }
    else
    {
        MATTER_CLI_LOG("wrong args should be either \"mattercommissioning on\" or \"mattercommissioning off\"");
        error = CHIP_ERROR_INVALID_ARGUMENT;
    }
    return error;
}

CHIP_ERROR cliFactoryReset(int argc, char * argv[])
{
    chip::NXP::App::GetAppTask().FactoryResetHandler();
    return CHIP_NO_ERROR;
}

CHIP_ERROR cliReset(int argc, char * argv[])
{
    /*
       Shutdown device before reboot,
       this emits the ShutDown event, handles the server shutting down,
       and stores in flash the total-operational-hours value.
    */
    chip::DeviceLayer::PlatformMgr().Shutdown();
    chip::DeviceLayer::PlatformMgrImpl().ScheduleResetInIdle();
    return CHIP_NO_ERROR;
}

CHIP_ERROR chip::NXP::App::AppMatterCli_RegisterCommands(void)
{
#ifdef ENABLE_CHIP_SHELL
    if (!isShellInitialized)
    {
        int error = Engine::Root().Init();
        if (error != 0)
        {
            ChipLogError(Shell, "Streamer initialization failed: %d", error);
            return CHIP_ERROR_INTERNAL;
        }

        /* Register common shell commands */
        cmd_misc_init();
        cmd_otcli_init();
#if CHIP_SHELL_ENABLE_CMD_SERVER
        cmd_app_server_init();
#endif /* CHIP_SHELL_ENABLE_CMD_SERVER */

        /* Register application commands */
        static const shell_command_t kCommands[] = {
            {
                .cmd_func = commissioningManager,
                .cmd_name = "mattercommissioning",
                .cmd_help = "Open/close the commissioning window. Usage : mattercommissioning [on|off]",
            },
            {
                .cmd_func = cliFactoryReset,
                .cmd_name = "matterfactoryreset",
                .cmd_help = "Perform a factory reset on the device",
            },
            {
                .cmd_func = cliReset,
                .cmd_name = "matterreset",
                .cmd_help = "Reset the device",
            },
        };

        Engine::Root().RegisterCommands(kCommands, sizeof(kCommands) / sizeof(kCommands[0]));
        isShellInitialized = true;
    }
#endif /* ENABLE_CHIP_SHELL */

    return CHIP_NO_ERROR;
}

CHIP_ERROR chip::NXP::App::AppMatterCli_StartTask()
{
#ifdef ENABLE_CHIP_SHELL
    if (xTaskCreate(&AppMatterCliTask, "AppMatterCli_task", MATTER_CLI_TASK_SIZE, NULL, 1, &AppMatterCliTaskHandle) != pdPASS)
    {
        ChipLogError(Shell, "Failed to start Matter CLI task");
        return CHIP_ERROR_INTERNAL;
    }
#endif /* ENABLE_CHIP_SHELL */
    return CHIP_NO_ERROR;
}
