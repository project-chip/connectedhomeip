/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <core/CHIPCore.h>
#include <lib/shell/Commands.h>
#if CONFIG_DEVICE_LAYER
#include <platform/CHIPDeviceLayer.h>
#endif
#include <lib/shell/commands/CommandsHelp.h>
#include <lib/shell/shell_core.h>
#include <support/CHIPArgParser.hpp>
#include <support/CHIPMem.h>
#include <support/CodeUtils.h>

using chip::DeviceLayer::ConnectivityMgr;

namespace chip {
namespace Shell {

static chip::Shell::Shell sShellDeviceSubcommands;

int CommandBLEHelp(int argc, char ** argv)
{
    sShellDeviceSubcommands.ForEachCommand(PrintCommandHelp, nullptr);
    return 0;
}

int CommandBLEAdvertise(int argc, char ** argv)
{
    CHIP_ERROR error  = CHIP_NO_ERROR;
    streamer_t * sout = streamer_get();
    bool adv_enabled;

    VerifyOrReturnError(argc == 1, error = CHIP_ERROR_INVALID_ARGUMENT);

    adv_enabled = ConnectivityMgr().IsBLEAdvertisingEnabled();
    if (strcmp(argv[0], "start") == 0)
    {
        if (adv_enabled)
        {
            streamer_printf(sout, "BLE advertising already enabled\r\n");
        }
        else
        {
            streamer_printf(sout, "Starting BLE advertising\r\n");
            return ConnectivityMgr().SetBLEAdvertisingEnabled(true);
        }
    }
    else if (strcmp(argv[0], "stop") == 0)
    {
        if (adv_enabled)
        {
            streamer_printf(sout, "Stopping BLE advertising\r\n");
            return ConnectivityMgr().SetBLEAdvertisingEnabled(false);
        }
        else
        {
            streamer_printf(sout, "BLE advertising already stopped\r\n");
        }
    }
    else
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return CHIP_NO_ERROR;
}

int CommandBLEDispatch(int argc, char ** argv)
{
    if (argc == 0)
    {
        CommandBLEHelp(argc, argv);
        return CHIP_NO_ERROR;
    }
    return sShellDeviceSubcommands.ExecCommand(argc, argv);
}

void CommandBLEInit()
{
    static const shell_command_t sBLESubCommands[] = {
        { &CommandBLEHelp, "help", "Usage: ble <subcommand>" },
        { &CommandBLEAdvertise, "adv", "Enable or disable advertisement. Usage: ble adv <start|stop>" },
    };

    static const shell_command_t sBLECommand = { &CommandBLEDispatch, "ble", "BLE transport commands" };

    // Register `device` subcommands with the local shell dispatcher.
    sShellDeviceSubcommands.RegisterCommands(sBLESubCommands, ArraySize(sBLESubCommands));

    // Register the root `btp` command with the top-level shell.
    shell_register(&sBLECommand, 1);
}

} // namespace Shell
} // namespace chip
