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
