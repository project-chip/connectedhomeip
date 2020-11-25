/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "ChipShellCollection.h"

#include <core/CHIPCore.h>

#if CONFIG_NETWORK_LAYER_BLE

#if CONFIG_DEVICE_LAYER
#include <platform/CHIPDeviceLayer.h>
#endif

#include <lib/shell/shell.h>
#include <support/CHIPArgParser.hpp>
#include <support/CHIPMem.h>
#include <support/CodeUtils.h>

using namespace chip;
using namespace chip::Shell;
using namespace chip::Platform;
using namespace chip::DeviceLayer;
using namespace chip::Logging;
using namespace chip::ArgParser;

static chip::Shell::Shell sShellDeviceSubcommands;

int cmd_btp_help_iterator(shell_command_t * command, void * arg)
{
    streamer_printf(streamer_get(), "  %-15s %s\n\r", command->cmd_name, command->cmd_help);
    return 0;
}

int cmd_btp_help(int argc, char ** argv)
{
    sShellDeviceSubcommands.ForEachCommand(cmd_btp_help_iterator, nullptr);
    return 0;
}

int cmd_btp_adv(int argc, char ** argv)
{
    CHIP_ERROR error  = CHIP_NO_ERROR;
    streamer_t * sout = streamer_get();
    bool adv_enabled;

    if (argc == 0)
    {
        ExitNow(error = CHIP_ERROR_INVALID_ARGUMENT);
    }

    adv_enabled = ConnectivityMgr().IsBLEAdvertisingEnabled();
    if (strcmp(argv[0], "start") == 0)
    {
        if (adv_enabled)
        {
            streamer_printf(sout, "BLE advertising already enabled");
        }
        else
        {
            streamer_printf(sout, "Starting BLE advertising");
            ConnectivityMgr().SetBLEAdvertisingEnabled(true);
        }
    }
    else if (strcmp(argv[0], "stop") == 0)
    {
        if (adv_enabled)
        {
            streamer_printf(sout, "Stopping BLE advertising");
            ConnectivityMgr().SetBLEAdvertisingEnabled(false);
        }
        else
        {
            streamer_printf(sout, "BLE advertising already stopped");
        }
    }
    else
    {
        ExitNow(error = CHIP_ERROR_INVALID_ARGUMENT);
    }

exit:
    return error;
}

int cmd_btp_scan(int argc, char ** argv)
{
    CHIP_ERROR error  = CHIP_NO_ERROR;
    streamer_t * sout = streamer_get();

    if (argc == 0)
    {
        ExitNow(error = CHIP_ERROR_INVALID_ARGUMENT);
    }

    if (strcmp(argv[0], "start") == 0)
    {
        streamer_printf(sout, "Starting scanning over BLE");
        // TODO: start scanning
    }
    else if (strcmp(argv[0], "stop") == 0)
    {
        streamer_printf(sout, "Stopping scanning over BLE");
        // TODO: stop scanning
    }
    else
    {
        ExitNow(error = CHIP_ERROR_INVALID_ARGUMENT);
    }

exit:
    return error;
}

int cmd_btp_connect(int argc, char ** argv)
{
    CHIP_ERROR error  = CHIP_NO_ERROR;
    streamer_t * sout = streamer_get();

    if (argc == 0)
    {
        ExitNow(error = CHIP_ERROR_INVALID_ARGUMENT);
    }

    if (strcmp(argv[0], "start") == 0)
    {
        streamer_printf(sout, "Connecting to the device over BLE");
        // connecting
    }
    else if (strcmp(argv[0], "stop") == 0)
    {
        streamer_printf(sout, "Disconnecting from the device");
        // disconnecting
    }
    else
    {
        ExitNow(error = CHIP_ERROR_INVALID_ARGUMENT);
    }

exit:
    return error;
}

int cmd_btp_send(int argc, char ** argv)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

int cmd_btp_dispatch(int argc, char ** argv)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    VerifyOrExit(argc > 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    error = sShellDeviceSubcommands.ExecCommand(argc, argv);

exit:
    return error;
}

static const shell_command_t cmds_btp_root = { &cmd_btp_dispatch, "btp", "BLE transport commands" };

/// Subcommands for root command: `btp <subcommand>`
static const shell_command_t cmds_btp[] = {
    { &cmd_btp_help, "help", "Usage: btp <subcommand>" },
    { &cmd_btp_scan, "scan", "Enable or disable scan. Usage: btp scan <start timeout|stop>" },
    { &cmd_btp_connect, "connect", "Connect or disconnect to a device. Usage: btp connect <start address|stop>" },
    { &cmd_btp_adv, "adv", "Enable or disable advertisement. Usage: btp adv <start|stop>" },
    { &cmd_btp_send, "send", "Send binary data. Usage: device dump" },
};

#endif // CONFIG_NETWORK_LAYER_BLE

void cmd_btp_init()
{
#if CONFIG_NETWORK_LAYER_BLE
    // CHIP_ERROR error = CHIP_NO_ERROR;

    // Register `device` subcommands with the local shell dispatcher.
    sShellDeviceSubcommands.RegisterCommands(cmds_btp, ArraySize(cmds_btp));

    // Register the root `btp` command with the top-level shell.
    shell_register(&cmds_btp_root, 1);

#endif // CONFIG_NETWORK_LAYER_BLE
}
