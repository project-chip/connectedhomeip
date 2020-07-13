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

#include <core/CHIPCore.h>

#if CONFIG_DEVICE_LAYER

#include <platform/CHIPDeviceLayer.h>
#include <shell/shell.h>
#include <support/Base64.h>
#include <support/CHIPArgParser.hpp>
#include <support/CodeUtils.h>
#include <support/RandUtils.h>

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace chip;
using namespace chip::Shell;
using namespace chip::DeviceLayer;
using namespace chip::Logging;
using namespace chip::ArgParser;

#define THE_SHELL_SUBMODULE theShellDevice
static chip::Shell::Shell THE_SHELL_SUBMODULE;

int cmd_device_help_iterator(shell_command_t * command, void * arg)
{
    streamer_printf(streamer_get(), "  %-15s %s\n\r", command->cmd_name, command->cmd_help);
    return 0;
}

int cmd_device_help(int argc, char ** argv)
{
    THE_SHELL_SUBMODULE.ForEachCommand(cmd_device_help_iterator, NULL);
    return 0;
}

int cmd_device_start(int argc, char ** argv)
{
    CHIP_ERROR error  = CHIP_NO_ERROR;
    streamer_t * sout = streamer_get();

    VerifyOrExit(argc == 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    streamer_printf(sout, "Init CHIP Stack\r\n");
    error = PlatformMgr().InitChipStack();
    SuccessOrExit(error);

    streamer_printf(sout, "Starting Platform Manager Event Loop\r\n");
    error = PlatformMgr().StartEventLoopTask();
    SuccessOrExit(error);

exit:
    return error;
}

int cmd_device_config(int argc, char ** argv)
{
    CHIP_ERROR error  = CHIP_NO_ERROR;
    streamer_t * sout = streamer_get();
    uint16_t value16;
    uint64_t value64;
    char buf[256];
    size_t bufSize;

    VerifyOrExit(argc == 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    SuccessOrExit(error = ConfigurationMgr().GetVendorId(value16));
    streamer_printf(sout, "VendorId:        %04x\n\r", value16);

    SuccessOrExit(error = ConfigurationMgr().GetProductId(value16));
    streamer_printf(sout, "ProductId:       %04x\n\r", value16);

    SuccessOrExit(error = ConfigurationMgr().GetProductRevision(value16));
    streamer_printf(sout, "ProductRevision: %04x\n\r", value16);

    error = ConfigurationMgr().GetServiceId(value64);
    streamer_printf(sout, "ServiceId:       ");
    if (error == CHIP_NO_ERROR)
    {
        streamer_print_hex(sout, reinterpret_cast<const uint8_t *>(&value64), sizeof(value64));
    }
    else
    {
        streamer_printf(sout, "<None>");
        error = CHIP_NO_ERROR;
    }
    streamer_printf(sout, "\r\n");

    error = ConfigurationMgr().GetFabricId(value64);
    streamer_printf(sout, "FabricId:        ");
    if (error == CHIP_NO_ERROR)
    {
        streamer_print_hex(sout, reinterpret_cast<const uint8_t *>(&value64), sizeof(value64));
    }
    else
    {
        streamer_printf(sout, "<None>");
        error = CHIP_NO_ERROR;
    }
    streamer_printf(sout, "\r\n");

    error = ConfigurationMgr().GetPairingCode(buf, sizeof(buf), bufSize);
    streamer_printf(sout, "PairingCode:     ");
    if (error == CHIP_NO_ERROR)
    {
        streamer_print_hex(sout, reinterpret_cast<const uint8_t *>(buf), bufSize);
        streamer_printf(sout, "\n\r");
    }
    else
    {
        streamer_printf(sout, "<None>");
        error = CHIP_NO_ERROR;
    }
    streamer_printf(sout, "\r\n");

exit:
    return error;
}

int cmd_device_get(int argc, char ** argv)
{
    CHIP_ERROR error  = CHIP_NO_ERROR;
    streamer_t * sout = streamer_get();

    if (argc == 0)
    {
        return cmd_device_config(argc, argv);
    }

    if (strcmp(argv[0], "vendorid") == 0)
    {
        uint16_t value;
        SuccessOrExit(error = ConfigurationMgr().GetVendorId(value));
        streamer_printf(sout, "%04x\n\r", value);
    }
    else if (strcmp(argv[0], "productid") == 0)
    {
        uint16_t value;
        SuccessOrExit(error = ConfigurationMgr().GetProductId(value));
        streamer_printf(sout, "%04x\n\r", value);
    }
    else if (strcmp(argv[0], "productrev") == 0)
    {
        uint16_t value;
        SuccessOrExit(error = ConfigurationMgr().GetProductRevision(value));
        streamer_printf(sout, "%04x\n\r", value);
    }
    else if (strcmp(argv[0], "serial") == 0)
    {
        size_t bufSize = 20;
        char buf[bufSize];
        size_t serialNumLength;
        SuccessOrExit(error = ConfigurationMgr().GetSerialNumber(buf, bufSize, serialNumLength));
        streamer_print_hex(sout, reinterpret_cast<const uint8_t *>(buf), serialNumLength);
        streamer_printf(sout, "\n\r");
    }
    else
    {
        ExitNow(error = CHIP_ERROR_INVALID_ARGUMENT);
    }

exit:
    return error;
}

int cmd_device_dispatch(int argc, char ** argv)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    VerifyOrExit(argc > 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    error = THE_SHELL_SUBMODULE.ExecCommand(argc, argv);

exit:
    return error;
}

static const shell_command_t cmds_base64_root = { &cmd_device_dispatch, "device", "Device Layer commands" };

/// Subcommands for root command: `base64 <subcommand>`
static const shell_command_t cmds_device[] = {
    { &cmd_device_help, "help", "Usage: device <subcommand>" },
    { &cmd_device_start, "start", "Start the device layer. Usage: device start" },
    { &cmd_device_get, "get", "Get configuration value. Usage: device get <param_name>" },
    { &cmd_device_config, "config", "Dump entire configuration of device. Usage: device dump" },
};

#endif // CONFIG_DEVICE_LAYER

void cmd_device_init(void)
{
#if CONFIG_DEVICE_LAYER
    // Register `device` subcommands with the local shell dispatcher.
    THE_SHELL_SUBMODULE.RegisterCommands(cmds_device, ARRAY_SIZE(cmds_device));

    // Register the root `base64` command with the top-level shell.
    shell_register(&cmds_base64_root, 1);
#endif // CONFIG_DEVICE_LAYER
}
