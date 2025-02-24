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

#include <OTAProviderCommands.h>
#include <lib/shell/Commands.h>
#include <lib/shell/Engine.h>
#include <lib/shell/commands/Help.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters::OtaSoftwareUpdateProvider;

namespace chip {
namespace Shell {
namespace {

OTAProviderExample * exampleOTAProvider = nullptr;
Shell::Engine sSubShell;

CHIP_ERROR DelayedActionTimeHandler(int argc, char ** argv)
{
    VerifyOrReturnError(argc == 1, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(exampleOTAProvider != nullptr, CHIP_ERROR_INCORRECT_STATE);

    const uint32_t delay = strtoul(argv[0], nullptr, 10);
    exampleOTAProvider->SetDelayedQueryActionTimeSec(delay);
    exampleOTAProvider->SetDelayedApplyActionTimeSec(delay);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ApplyUpdateActionHandler(int argc, char ** argv)
{
    VerifyOrReturnError(argc == 1, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(exampleOTAProvider != nullptr, CHIP_ERROR_INCORRECT_STATE);

    if (strcmp(argv[0], "proceed") == 0)
    {
        exampleOTAProvider->SetApplyUpdateAction(OTAApplyUpdateAction::kProceed);
    }
    else if (strcmp(argv[0], "awaitNextAction") == 0)
    {
        exampleOTAProvider->SetApplyUpdateAction(OTAApplyUpdateAction::kAwaitNextAction);
    }
    else if (strcmp(argv[0], "discontinue") == 0)
    {
        exampleOTAProvider->SetApplyUpdateAction(OTAApplyUpdateAction::kDiscontinue);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAProviderHandler(int argc, char ** argv)
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
} // namespace

void OTAProviderCommands::Register()
{
    // These commands can be moved to src/lib/shell/commands/Ota.cpp along with the other OTA commands.
    // But as of now only Linux and ESP32 platforms supports OTA provider

    // Register subcommands of the `OTAProvider` commands.
    static const shell_command_t subCommands[] = {
        { &DelayedActionTimeHandler, "delay",
          "Set delayed action time for QueryImageResponse and ApplyUpdateResponse\n"
          "Usage: OTAProvider delay <delay in seconds>" },
        { &ApplyUpdateActionHandler, "applyUpdateAction",
          "Set apply update action in the apply ApplyUpdateResponse\n"
          "Usage: OTAProvider applyUpdateAction <proceed/awaitNextAction/discontinue>" },
    };

    sSubShell.RegisterCommands(subCommands, MATTER_ARRAY_SIZE(subCommands));

    // Register the root `OTA Provider` command in the top-level shell.
    static const shell_command_t otaProviderCommand = { &OTAProviderHandler, "OTAProvider", "OTA Provider commands" };

    Engine::Root().RegisterCommands(&otaProviderCommand, 1);
}

// Set Example OTA provider
void OTAProviderCommands::SetExampleOTAProvider(OTAProviderExample * otaProvider)
{
    exampleOTAProvider = otaProvider;
}

} // namespace Shell
} // namespace chip
