/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <OnOffCommands.h>
#include <lib/shell/Commands.h>
#include <lib/shell/Engine.h>
#include <lib/shell/commands/Help.h>
#include <lib/support/logging/CHIPLogging.h>

#include <lib/core/CHIPCore.h>
#if CONFIG_DEVICE_LAYER
#include <platform/CHIPDeviceLayer.h>
#endif
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <lib/shell/Engine.h>
#include <lib/shell/commands/Help.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>

namespace chip {
namespace Shell {
namespace {

Shell::Engine sSubShell;

CHIP_ERROR OnOffHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        sSubShell.ForEachCommand(PrintCommandHelp, nullptr);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR error = sSubShell.ExecCommand(argc, argv);

    if (error != CHIP_NO_ERROR)
    {
        streamer_printf(streamer_get(), "Error: %" CHIP_ERROR_FORMAT "\r\n", error.Format());
    }

    return error;
}

static CHIP_ERROR OnLightHandler(int argc, char ** argv)
{
    if (argc != 1)
        return CHIP_ERROR_INVALID_ARGUMENT;
    chip::app::Clusters::OnOff::Attributes::OnOff::Set(atoi(argv[0]), 1);
    return CHIP_NO_ERROR;
}

static CHIP_ERROR OffLightHandler(int argc, char ** argv)
{
    if (argc != 1)
        return CHIP_ERROR_INVALID_ARGUMENT;
    chip::app::Clusters::OnOff::Attributes::OnOff::Set(atoi(argv[0]), 0);
    return CHIP_NO_ERROR;
}

static CHIP_ERROR ToggleLightHandler(int argc, char ** argv)
{
    if (argc != 1)
        return CHIP_ERROR_INVALID_ARGUMENT;
    bool value;
    chip::app::Clusters::OnOff::Attributes::OnOff::Get(atoi(argv[0]), &value);
    chip::app::Clusters::OnOff::Attributes::OnOff::Set(atoi(argv[0]), !value);
    return CHIP_NO_ERROR;
}

} // namespace

void OnOffCommands::Register()
{
    static const shell_command_t subCommands[] = { { &OnLightHandler, "on", "Usage: OnOff on endpoint-id" },
                                                   { &OffLightHandler, "off", "Usage: OnOff off endpoint-id" },
                                                   { &ToggleLightHandler, "toggle", "Usage: OnOff toggle endpoint-id" } };
    sSubShell.RegisterCommands(subCommands, ArraySize(subCommands));

    // Register the root `OnOff` command in the top-level shell.
    static const shell_command_t onOffCommand = { &OnOffHandler, "OnOff", "OnOff commands" };

    Engine::Root().RegisterCommands(&onOffCommand, 1);
}

} // namespace Shell
} // namespace chip
