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

#include "AppCLIBase.h"
#include "AppTaskBase.h"
#include <ChipShellCollection.h>
#include <app/server/Server.h>
#include <cstring>
#include <lib/shell/Engine.h>
#include <platform/CHIPDeviceLayer.h>

#if (CHIP_DEVICE_CONFIG_ENABLE_WPA && CHIP_ENABLE_OPENTHREAD)

#include <platform/OpenThread/GenericThreadStackManagerImpl_OpenThread.h>

extern "C" {
#include "addons_cli.h"
}
#endif

#define MATTER_CLI_LOG(message) (chip::Shell::streamer_printf(chip::Shell::streamer_get(), message))

static CHIP_ERROR commissioningManager(int argc, char * argv[])
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

static CHIP_ERROR cliFactoryReset(int argc, char * argv[])
{
    chip::NXP::App::GetAppTask().FactoryResetHandler();
    return CHIP_NO_ERROR;
}

static CHIP_ERROR cliReset(int argc, char * argv[])
{
    chip::NXP::App::GetAppCLI().ResetCmdHandle();
    return CHIP_NO_ERROR;
}

void chip::NXP::App::AppCLIBase::RegisterDefaultCommands(void)
{
    static const chip::Shell::shell_command_t kCommands[] = {
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
        }
    };

    /* Register common shell commands */
    cmd_misc_init();
    cmd_otcli_init();
#if (CHIP_DEVICE_CONFIG_ENABLE_WPA && CHIP_ENABLE_OPENTHREAD)
    otAppCliAddonsInit(chip::DeviceLayer::ThreadStackMgrImpl().OTInstance());
#endif
#if CHIP_SHELL_ENABLE_CMD_SERVER
    cmd_app_server_init();
#endif /* CHIP_SHELL_ENABLE_CMD_SERVER */
    chip::Shell::Engine::Root().RegisterCommands(kCommands, sizeof(kCommands) / sizeof(kCommands[0]));
}
