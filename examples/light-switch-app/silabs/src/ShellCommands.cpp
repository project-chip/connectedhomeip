/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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
#if defined(ENABLE_CHIP_SHELL)

#include "ShellCommands.h"
#include "BindingHandler.h"

#include <app/clusters/bindings/bindings.h>
#include <lib/shell/Engine.h>
#include <lib/shell/commands/Help.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::app;

namespace LightSwitchCommands {

using Shell::Engine;
using Shell::shell_command_t;
using Shell::streamer_get;
using Shell::streamer_printf;

Engine sShellSwitchSubCommands;
Engine sShellSwitchOnOffSubCommands;

Engine sShellSwitchGroupsSubCommands;
Engine sShellSwitchGroupsOnOffSubCommands;

Engine sShellSwitchBindingSubCommands;

/********************************************************
 * Switch shell functions
 *********************************************************/

CHIP_ERROR SwitchHelpHandler(int argc, char ** argv)
{
    sShellSwitchSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR SwitchCommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return SwitchHelpHandler(argc, argv);
    }

    return sShellSwitchSubCommands.ExecCommand(argc, argv);
}

/********************************************************
 * OnOff switch shell functions
 *********************************************************/

CHIP_ERROR OnOffHelpHandler(int argc, char ** argv)
{
    sShellSwitchOnOffSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR OnOffSwitchCommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return OnOffHelpHandler(argc, argv);
    }

    return sShellSwitchOnOffSubCommands.ExecCommand(argc, argv);
}

CHIP_ERROR OnSwitchCommandHandler(int argc, char ** argv)
{
    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->commandId           = Clusters::OnOff::Commands::On::Id;
    data->clusterId           = Clusters::OnOff::Id;

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OffSwitchCommandHandler(int argc, char ** argv)
{
    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->commandId           = Clusters::OnOff::Commands::Off::Id;
    data->clusterId           = Clusters::OnOff::Id;

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ToggleSwitchCommandHandler(int argc, char ** argv)
{
    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->commandId           = Clusters::OnOff::Commands::Toggle::Id;
    data->clusterId           = Clusters::OnOff::Id;

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

/********************************************************
 * bind switch shell functions
 *********************************************************/

CHIP_ERROR BindingHelpHandler(int argc, char ** argv)
{
    sShellSwitchBindingSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR BindingSwitchCommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return BindingHelpHandler(argc, argv);
    }

    return sShellSwitchBindingSubCommands.ExecCommand(argc, argv);
}

CHIP_ERROR BindingGroupBindCommandHandler(int argc, char ** argv)
{
    VerifyOrReturnError(argc == 2, CHIP_ERROR_INVALID_ARGUMENT);

    EmberBindingTableEntry * entry = Platform::New<EmberBindingTableEntry>();
    entry->type                    = MATTER_MULTICAST_BINDING;
    entry->fabricIndex             = atoi(argv[0]);
    entry->groupId                 = atoi(argv[1]);
    entry->local                   = 1; // Hardcoded to endpoint 1 for now
    entry->clusterId.emplace(6);        // Hardcoded to OnOff cluster for now

    DeviceLayer::PlatformMgr().ScheduleWork(BindingWorkerFunction, reinterpret_cast<intptr_t>(entry));
    return CHIP_NO_ERROR;
}

CHIP_ERROR BindingUnicastBindCommandHandler(int argc, char ** argv)
{
    VerifyOrReturnError(argc == 3, CHIP_ERROR_INVALID_ARGUMENT);

    EmberBindingTableEntry * entry = Platform::New<EmberBindingTableEntry>();
    entry->type                    = MATTER_UNICAST_BINDING;
    entry->fabricIndex             = atoi(argv[0]);
    entry->nodeId                  = atoi(argv[1]);
    entry->local                   = 1; // Hardcoded to endpoint 1 for now
    entry->remote                  = atoi(argv[2]);
    entry->clusterId.emplace(6); // Hardcode to OnOff cluster for now

    DeviceLayer::PlatformMgr().ScheduleWork(BindingWorkerFunction, reinterpret_cast<intptr_t>(entry));
    return CHIP_NO_ERROR;
}

/********************************************************
 * Groups switch shell functions
 *********************************************************/

CHIP_ERROR GroupsHelpHandler(int argc, char ** argv)
{
    sShellSwitchGroupsSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupsSwitchCommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return GroupsHelpHandler(argc, argv);
    }

    return sShellSwitchGroupsSubCommands.ExecCommand(argc, argv);
}

/********************************************************
 * Groups OnOff switch shell functions
 *********************************************************/

CHIP_ERROR GroupsOnOffHelpHandler(int argc, char ** argv)
{
    sShellSwitchGroupsOnOffSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupsOnOffSwitchCommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return GroupsOnOffHelpHandler(argc, argv);
    }

    return sShellSwitchGroupsOnOffSubCommands.ExecCommand(argc, argv);
}

CHIP_ERROR GroupOnSwitchCommandHandler(int argc, char ** argv)
{
    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->commandId           = Clusters::OnOff::Commands::On::Id;
    data->clusterId           = Clusters::OnOff::Id;
    data->isGroup             = true;

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupOffSwitchCommandHandler(int argc, char ** argv)
{
    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->commandId           = Clusters::OnOff::Commands::Off::Id;
    data->clusterId           = Clusters::OnOff::Id;
    data->isGroup             = true;

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupToggleSwitchCommandHandler(int argc, char ** argv)
{
    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->commandId           = Clusters::OnOff::Commands::Toggle::Id;
    data->clusterId           = Clusters::OnOff::Id;
    data->isGroup             = true;

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

/**
 * @brief configures switch matter shell
 */
void RegisterSwitchCommands()
{
    static const shell_command_t sSwitchSubCommands[] = {
        { &SwitchHelpHandler, "help", "Usage: switch <subcommand>" },
        { &OnOffSwitchCommandHandler, "onoff", " Usage: switch onoff <subcommand>" },
        { &GroupsSwitchCommandHandler, "groups", "Usage: switch groups <subcommand>" },
        { &BindingSwitchCommandHandler, "binding", "Usage: switch binding <subcommand>" }
    };

    static const shell_command_t sSwitchOnOffSubCommands[] = {
        { &OnOffHelpHandler, "help", "Usage : switch ononff <subcommand>" },
        { &OnSwitchCommandHandler, "on", "Sends on command to bound lighting app" },
        { &OffSwitchCommandHandler, "off", "Sends off command to bound lighting app" },
        { &ToggleSwitchCommandHandler, "toggle", "Sends toggle command to bound lighting app" }
    };

    static const shell_command_t sSwitchGroupsSubCommands[] = { { &GroupsHelpHandler, "help", "Usage: switch groups <subcommand>" },
                                                                { &GroupsOnOffSwitchCommandHandler, "onoff",
                                                                  "Usage: switch groups onoff <subcommand>" } };

    static const shell_command_t sSwitchGroupsOnOffSubCommands[] = {
        { &GroupsOnOffHelpHandler, "help", "Usage: switch groups onoff <subcommand>" },
        { &GroupOnSwitchCommandHandler, "on", "Sends on command to bound group" },
        { &GroupOffSwitchCommandHandler, "off", "Sends off command to bound group" },
        { &GroupToggleSwitchCommandHandler, "toggle", "Sends toggle command to group" }
    };

    static const shell_command_t sSwitchBindingSubCommands[] = {
        { &BindingHelpHandler, "help", "Usage: switch binding <subcommand>" },
        { &BindingGroupBindCommandHandler, "group", "Usage: switch binding group <fabric index> <group id>" },
        { &BindingUnicastBindCommandHandler, "unicast", "Usage: switch binding group <fabric index> <node id> <endpoint>" }
    };

    static const shell_command_t sSwitchCommand = { &SwitchCommandHandler, "switch",
                                                    "Light-switch commands. Usage: switch <subcommand>" };

    sShellSwitchGroupsOnOffSubCommands.RegisterCommands(sSwitchGroupsOnOffSubCommands,
                                                        MATTER_ARRAY_SIZE(sSwitchGroupsOnOffSubCommands));
    sShellSwitchOnOffSubCommands.RegisterCommands(sSwitchOnOffSubCommands, MATTER_ARRAY_SIZE(sSwitchOnOffSubCommands));
    sShellSwitchGroupsSubCommands.RegisterCommands(sSwitchGroupsSubCommands, MATTER_ARRAY_SIZE(sSwitchGroupsSubCommands));
    sShellSwitchBindingSubCommands.RegisterCommands(sSwitchBindingSubCommands, MATTER_ARRAY_SIZE(sSwitchBindingSubCommands));
    sShellSwitchSubCommands.RegisterCommands(sSwitchSubCommands, MATTER_ARRAY_SIZE(sSwitchSubCommands));

    Engine::Root().RegisterCommands(&sSwitchCommand, 1);
}

} // namespace LightSwitchCommands

#endif // ENABLE_CHIP_SHELL
