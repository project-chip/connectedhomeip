/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <lib/core/CHIPCore.h>
#include <lib/shell/Commands.h>
#if CONFIG_DEVICE_LAYER
#include <platform/CHIPDeviceLayer.h>
#endif
#include <lib/shell/Engine.h>
#include <lib/shell/commands/Help.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>

using chip::DeviceLayer::ConnectivityMgr;

namespace chip {
namespace Shell {

static chip::Shell::Engine sShellDeviceSubcommands;

CHIP_ERROR BLEHelpHandler(int argc, char ** argv)
{
    sShellDeviceSubcommands.ForEachCommand(PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEAdvertiseHandler(int argc, char ** argv)
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

        streamer_printf(sout, "BLE advertising already stopped\r\n");
    }
    else if (strcmp(argv[0], "state") == 0)
    {
        if (adv_enabled)
        {
            streamer_printf(sout, "BLE advertising is enabled\r\n");
        }
        else
        {
            streamer_printf(sout, "BLE advertising is disabled\r\n");
        }
    }
    else
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return error;
}

CHIP_ERROR BLEDispatch(int argc, char ** argv)
{
    if (argc == 0)
    {
        BLEHelpHandler(argc, argv);
        return CHIP_NO_ERROR;
    }
    return sShellDeviceSubcommands.ExecCommand(argc, argv);
}

void RegisterBLECommands()
{
    static const shell_command_t sBLESubCommands[] = {
        { &BLEHelpHandler, "help", "Usage: ble <subcommand>" },
        { &BLEAdvertiseHandler, "adv", "Enable or disable advertisement. Usage: ble adv <start|stop|state>" },
    };

    static const shell_command_t sBLECommand = { &BLEDispatch, "ble", "BLE transport commands" };

    // Register `device` subcommands with the local shell dispatcher.
    sShellDeviceSubcommands.RegisterCommands(sBLESubCommands, ArraySize(sBLESubCommands));

    // Register the root `btp` command with the top-level shell.
    Engine::Root().RegisterCommands(&sBLECommand, 1);
}

} // namespace Shell
} // namespace chip
