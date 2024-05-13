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

#include "controller/InvokeInteraction.h"
#include "controller/ReadInteraction.h"
#include <rvc-modes.h>

#if CONFIG_ENABLE_CHIP_SHELL
#include "lib/shell/Engine.h"
#include "lib/shell/commands/Help.h"
#endif // ENABLE_CHIP_SHELL

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

#if CONFIG_ENABLE_CHIP_SHELL
using Shell::Engine;
using Shell::shell_command_t;
using Shell::streamer_get;
using Shell::streamer_printf;

Engine sShellManualRVCSubCommands;
Engine sShellManualRVCRunModeSubCommands;
Engine sShellManualRVCCleanModeSubCommands;
#endif // defined(ENABLE_CHIP_SHELL)

#if CONFIG_ENABLE_CHIP_SHELL
/********************************************************
 * RVC
 *********************************************************/

CHIP_ERROR ManualRVCCommandHelpHandler(int argc, char ** argv)
{
    sShellManualRVCSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ManualRVCCommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return ManualRVCCommandHelpHandler(argc, argv);
    }
    return sShellManualRVCSubCommands.ExecCommand(argc, argv);
}

/********************************************************
 * RVC Run Mode
 *********************************************************/

CHIP_ERROR ManualRVCRunModeCommandHelpHandler(int argc, char ** argv)
{
    sShellManualRVCRunModeSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ManualRVCRunModeCommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return ManualRVCRunModeCommandHelpHandler(argc, argv);
    }

    return sShellManualRVCRunModeSubCommands.ExecCommand(argc, argv);
}

CHIP_ERROR ManualRVCRunModeSetModeCommandHandler(int argc, char ** argv)
{
    if (argc != 1)
    {
        return ManualRVCRunModeCommandHelpHandler(argc, argv);
    }
    Protocols::InteractionModel::Status status;
    status = RvcRunMode::Instance()->UpdateCurrentMode((uint8_t) atoi(argv[0]));
    if (status != Protocols::InteractionModel::Status::Success)
    {
        ChipLogError(DeviceLayer, "ManualRVCRunModeSetModeCommandHandler Error!");
        return CHIP_ERROR_INTERNAL;
    }
    return CHIP_NO_ERROR;
}

/********************************************************
 * RVC Clean Mode
 *********************************************************/

CHIP_ERROR ManualRVCCleanModeCommandHelpHandler(int argc, char ** argv)
{
    sShellManualRVCCleanModeSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ManualRVCCleanModeCommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return ManualRVCCleanModeCommandHelpHandler(argc, argv);
    }

    return sShellManualRVCCleanModeSubCommands.ExecCommand(argc, argv);
}

CHIP_ERROR ManualRVCCleanModeSetModeCommandHandler(int argc, char ** argv)
{
    if (argc != 1)
    {
        return ManualRVCCleanModeCommandHelpHandler(argc, argv);
    }
    Protocols::InteractionModel::Status status;
    status = RvcCleanMode::Instance()->UpdateCurrentMode((uint8_t) atoi(argv[0]));
    if (status != Protocols::InteractionModel::Status::Success)
    {
        ChipLogError(DeviceLayer, "ManualRVCCleanModeSetModeCommandHandler Error!");
        return CHIP_ERROR_INTERNAL;
    }
    return CHIP_NO_ERROR;
}

#endif // CONFIG_ENABLE_CHIP_SHELL
