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

int DeviceHelpHandler(int argc, char ** argv)
{
    sShellDeviceSubcommands.ForEachCommand(PrintCommandHelp, nullptr);
    return 0;
}

static CHIP_ERROR FactoryResetHandler(int argc, char ** argv)
{
    streamer_printf(streamer_get(), "Performing factory reset ... \r\n");
    DeviceLayer::ConfigurationMgr().InitiateFactoryReset();
    return CHIP_NO_ERROR;
}

static CHIP_ERROR DeviceHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        DeviceHelpHandler(argc, argv);
        return CHIP_NO_ERROR;
    }
    return sShellDeviceSubcommands.ExecCommand(argc, argv);
}

void RegisterDeviceCommands()
{
    static const shell_command_t sDeviceSubCommands[] = {
        { &FactoryResetHandler, "factoryreset", "Performs device factory reset" },
    };

    static const shell_command_t sDeviceComand = { &DeviceHandler, "device", "Device management commands" };

    // Register `device` subcommands with the local shell dispatcher.
    sShellDeviceSubcommands.RegisterCommands(sDeviceSubCommands, ArraySize(sDeviceSubCommands));

    // Register the root `device` command with the top-level shell.
    Engine::Root().RegisterCommands(&sDeviceComand, 1);
}

} // namespace Shell
} // namespace chip
