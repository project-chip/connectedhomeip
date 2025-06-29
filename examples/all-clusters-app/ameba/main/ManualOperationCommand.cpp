/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "ManualOperationCommand.h"
#include "ManualDishWasherAlarmCommand.h"
#include "ManualOperationalStateCommand.h"
#include "ManualRVCModesCommand.h"
#include "ManualRefrigeratorAlarmCommand.h"

#include "app/server/Server.h"
#include "platform/CHIPDeviceLayer.h"
#include <lib/support/CodeUtils.h>

#if CONFIG_ENABLE_CHIP_SHELL
#include "lib/shell/Engine.h"
#include "lib/shell/commands/Help.h"
#endif // ENABLE_CHIP_SHELL

using namespace chip;
using namespace chip::app;

#if CONFIG_ENABLE_CHIP_SHELL
using Shell::Engine;
using Shell::shell_command_t;
using Shell::streamer_get;
using Shell::streamer_printf;

Engine sShellManualOperationSubCommands;
#endif // defined(ENABLE_CHIP_SHELL)

namespace {
#if CONFIG_ENABLE_CHIP_SHELL

/********************************************************
 * Manual Operation shell functions
 *********************************************************/

CHIP_ERROR ManualOperationHelpHandler(int argc, char ** argv)
{
    sShellManualOperationSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ManualOperationCommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return ManualOperationHelpHandler(argc, argv);
    }

    return sShellManualOperationSubCommands.ExecCommand(argc, argv);
}

/**
 * @brief configures switch matter shell
 *
 */
static void RegisterManualOperationCommands()
{

    static const shell_command_t sManualOperationSubCommands[] = {
        { &ManualOperationHelpHandler, "help", "Usage: manual <subcommand>" },
        { &ManualOperationalStateCommandHandler, "opstate", " Usage: manual opstate <subcommand>" },
        { &ManualRVCCommandHandler, "rvc", " Usage: manual rvc <subcommand>" },
        { &ManualRefrigeratorAlarmCommandHandler, "refalm", " Usage: manual refalm <subcommand>" },
        { &ManualDishWasherAlarmCommandHandler, "dishalm", " Usage: manual dishalm <subcommand>" },
        { &ManualOvenCavityOperationalStateCommandHandler, "oven-opstate", " Usage: manual dishalm <subcommand>" },
    };

    static const shell_command_t sManualOperationalStateSubCommands[] = {
        { &ManualOperationalStateCommandHelpHandler, "help", "Usage: manual opstate <subcommand>" },
        { &ManualOperationalStateSetStateCommandHandler, "set-state", "set-state Usage: manual opstate set-state <state>" },
        { &ManualOperationalStateSetErrorCommandHandler, "set-error", "set-error Usage: manual opstate set-error <error>" },
    };

    static const shell_command_t sManualRVCSubCommands[] = {
        { &ManualRVCCommandHelpHandler, "help", "Usage: manual rvc <subcommand>" },
        { &ManualRVCOperationalStateCommandHandler, "opstate", "Usage: manual rvc opstate <subcommand>" },
        { &ManualRVCRunModeCommandHandler, "runmode", "Usage: manual rvc runmode <subcommand>" },
        { &ManualRVCCleanModeCommandHandler, "cleanmode", "Usage: manual rvc cleanmode <subcommand>" },
    };

    static const shell_command_t sManualRVCOperationalStateSubCommands[] = {
        { &ManualRVCOperationalStateCommandHelpHandler, "help", "Usage: manual rvc opstate <subcommand>" },
        { &ManualRVCOperationalStateSetStateCommandHandler, "set-state", "set-state Usage: manual rvc opstate set-state <state>" },
        { &ManualRVCOperationalStateSetErrorCommandHandler, "set-error", "set-error Usage: manual rvc opstate set-error <error>" },
    };

    static const shell_command_t sManualRVCRunModeSubCommands[] = {
        { &ManualRVCRunModeCommandHelpHandler, "help", "Usage: manual rvc runmode <subcommand>" },
        { &ManualRVCRunModeSetModeCommandHandler, "set-mode", "set-mode Usage: manual rvc runmode set-mode <mode>" },
    };

    static const shell_command_t sManualRVCCleanModeSubCommands[] = {
        { &ManualRVCCleanModeCommandHelpHandler, "help", "Usage: manual rvc cleanmode <subcommand>" },
        { &ManualRVCCleanModeSetModeCommandHandler, "set-mode", "set-mode Usage: manual rvc cleanmode set-mode <mode>" },
    };

    static const shell_command_t sManualRefrigeratorAlarmStateSubCommands[] = {
        { &ManualRefrigeratorAlarmCommandHelpHandler, "help", "Usage: manual refalm <subcommand>" },
        { &ManualRefrigeratorAlarmDoorOpenCommandHandler, "door-open", "door-open Usage: manual refalm door-open" },
        { &ManualRefrigeratorAlarmDoorCloseCommandHandler, "door-close", "door-close Usage: manual refalm door-close" },
        { &ManualRefrigeratorAlarmSuppressCommandHandler, "suppress-alarm", "suppress-alarm Usage: manual refalm suppress-alarm" },
    };

    static const shell_command_t sManualDishWasherAlarmSubCommands[] = {
        { &ManualDishWasherAlarmCommandHelpHandler, "help", "Usage: manual dishalm <subcommand>" },
        { &ManualDishWasherAlarmSetRaiseCommandHandler, "raise", "raise Usage: manual dishalm raise" },
        { &ManualDishWasherAlarmSetLowerCommandHandler, "lower", "lower Usage: manual dishalm lower" },
    };

    static const shell_command_t sManualOvenCavityOperationalStateSubCommands[] = {
        { &ManualOvenCavityOperationalStateCommandHelpHandler, "help", "Usage: manual oven-opstate <subcommand>" },
        { &ManualOvenCavityOperationalStateSetStateCommandHandler, "set-state",
          "set-state Usage: manual oven-opstate set-state <state>" },
        { &ManualOvenCavityOperationalStateSetErrorCommandHandler, "set-error",
          "set-error Usage: manual oven-opstate set-error <error>" },
    };

    static const shell_command_t sManualOperationCommand = { &ManualOperationCommandHandler, "manual",
                                                             "Manual Operation commands. Usage: manual <subcommand>" };

    // Register commands
    sShellManualOperationSubCommands.RegisterCommands(sManualOperationSubCommands, MATTER_ARRAY_SIZE(sManualOperationSubCommands));
    sShellManualOperationalStateSubCommands.RegisterCommands(sManualOperationalStateSubCommands,
                                                             MATTER_ARRAY_SIZE(sManualOperationalStateSubCommands));
    sShellManualRVCSubCommands.RegisterCommands(sManualRVCSubCommands, MATTER_ARRAY_SIZE(sManualRVCSubCommands));
    sShellManualRVCOperationalStateSubCommands.RegisterCommands(sManualRVCOperationalStateSubCommands,
                                                                MATTER_ARRAY_SIZE(sManualRVCOperationalStateSubCommands));
    sShellManualRVCRunModeSubCommands.RegisterCommands(sManualRVCRunModeSubCommands,
                                                       MATTER_ARRAY_SIZE(sManualRVCRunModeSubCommands));
    sShellManualRVCCleanModeSubCommands.RegisterCommands(sManualRVCCleanModeSubCommands,
                                                         MATTER_ARRAY_SIZE(sManualRVCCleanModeSubCommands));
    sShellManualRefrigeratorAlarmStateSubCommands.RegisterCommands(sManualRefrigeratorAlarmStateSubCommands,
                                                                   MATTER_ARRAY_SIZE(sManualRefrigeratorAlarmStateSubCommands));
    sShellManualDishWasherAlarmStateSubCommands.RegisterCommands(sManualDishWasherAlarmSubCommands,
                                                                 MATTER_ARRAY_SIZE(sManualDishWasherAlarmSubCommands));
    sShellManualOvenCavityOperationalStateSubCommands.RegisterCommands(
        sManualOvenCavityOperationalStateSubCommands, MATTER_ARRAY_SIZE(sManualOvenCavityOperationalStateSubCommands));

    Engine::Root().RegisterCommands(&sManualOperationCommand, 1);
}
#endif // ENABLE_CHIP_SHELL

} // namespace

/********************************************************
 * Switch functions
 *********************************************************/

CHIP_ERROR InitManualOperation()
{
#if CONFIG_ENABLE_CHIP_SHELL
    RegisterManualOperationCommands();
#endif
    return CHIP_NO_ERROR;
}
