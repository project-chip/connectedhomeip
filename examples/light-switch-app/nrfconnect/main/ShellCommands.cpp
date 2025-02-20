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

#include "ShellCommands.h"
#include "LightSwitch.h"
#include <platform/CHIPDeviceLayer.h>

#include "BindingHandler.h"

#ifdef CONFIG_CHIP_LIB_SHELL

using namespace chip;
using namespace chip::app;

namespace SwitchCommands {
using Shell::Engine;
using Shell::shell_command_t;
using Shell::streamer_get;
using Shell::streamer_printf;

Engine sShellSwitchSubCommands;
Engine sShellSwitchOnOffSubCommands;
Engine sShellSwitchGroupsSubCommands;
Engine sShellSwitchGroupsOnOffSubCommands;

static CHIP_ERROR SwitchHelpHandler(int argc, char ** argv)
{
    sShellSwitchSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

static CHIP_ERROR SwitchCommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return SwitchHelpHandler(argc, argv);
    }
    return sShellSwitchSubCommands.ExecCommand(argc, argv);
}

static CHIP_ERROR TableCommandHelper(int argc, char ** argv)
{
    BindingHandler::GetInstance().PrintBindingTable();
    return CHIP_NO_ERROR;
}

namespace Unicast {

static CHIP_ERROR OnOffHelpHandler(int argc, char ** argv)
{
    sShellSwitchOnOffSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

static CHIP_ERROR OnOffCommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return OnOffHelpHandler(argc, argv);
    }
    return sShellSwitchOnOffSubCommands.ExecCommand(argc, argv);
}

static CHIP_ERROR OnCommandHandler(int argc, char ** argv)
{
    BindingHandler::BindingData * data = Platform::New<BindingHandler::BindingData>();
    data->EndpointId                   = LightSwitch::GetInstance().GetLightSwitchEndpointId();
    data->CommandId                    = Clusters::OnOff::Commands::On::Id;
    data->ClusterId                    = Clusters::OnOff::Id;

    DeviceLayer::PlatformMgr().ScheduleWork(BindingHandler::SwitchWorkerHandler, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

static CHIP_ERROR OffCommandHandler(int argc, char ** argv)
{
    BindingHandler::BindingData * data = Platform::New<BindingHandler::BindingData>();
    data->EndpointId                   = LightSwitch::GetInstance().GetLightSwitchEndpointId();
    data->CommandId                    = Clusters::OnOff::Commands::Off::Id;
    data->ClusterId                    = Clusters::OnOff::Id;

    DeviceLayer::PlatformMgr().ScheduleWork(BindingHandler::SwitchWorkerHandler, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

static CHIP_ERROR ToggleCommandHandler(int argc, char ** argv)
{
    BindingHandler::BindingData * data = Platform::New<BindingHandler::BindingData>();
    data->EndpointId                   = LightSwitch::GetInstance().GetLightSwitchEndpointId();
    data->CommandId                    = Clusters::OnOff::Commands::Toggle::Id;
    data->ClusterId                    = Clusters::OnOff::Id;

    DeviceLayer::PlatformMgr().ScheduleWork(BindingHandler::SwitchWorkerHandler, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}
} // namespace Unicast

namespace Group {

CHIP_ERROR SwitchHelpHandler(int argc, char ** argv)
{
    sShellSwitchGroupsSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR SwitchCommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return SwitchHelpHandler(argc, argv);
    }

    return sShellSwitchGroupsSubCommands.ExecCommand(argc, argv);
}

static CHIP_ERROR OnOffHelpHandler(int argc, char ** argv)
{
    sShellSwitchGroupsOnOffSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

static CHIP_ERROR OnOffCommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return OnOffHelpHandler(argc, argv);
    }

    return sShellSwitchGroupsOnOffSubCommands.ExecCommand(argc, argv);
}

CHIP_ERROR OnCommandHandler(int argc, char ** argv)
{
    BindingHandler::BindingData * data = Platform::New<BindingHandler::BindingData>();
    data->EndpointId                   = LightSwitch::GetInstance().GetLightSwitchEndpointId();
    data->CommandId                    = Clusters::OnOff::Commands::On::Id;
    data->ClusterId                    = Clusters::OnOff::Id;
    data->IsGroup                      = true;

    DeviceLayer::PlatformMgr().ScheduleWork(BindingHandler::SwitchWorkerHandler, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OffCommandHandler(int argc, char ** argv)
{
    BindingHandler::BindingData * data = Platform::New<BindingHandler::BindingData>();
    data->EndpointId                   = LightSwitch::GetInstance().GetLightSwitchEndpointId();
    data->CommandId                    = Clusters::OnOff::Commands::Off::Id;
    data->ClusterId                    = Clusters::OnOff::Id;
    data->IsGroup                      = true;

    DeviceLayer::PlatformMgr().ScheduleWork(BindingHandler::SwitchWorkerHandler, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ToggleCommandHandler(int argc, char ** argv)
{
    BindingHandler::BindingData * data = Platform::New<BindingHandler::BindingData>();
    data->EndpointId                   = LightSwitch::GetInstance().GetLightSwitchEndpointId();
    data->CommandId                    = Clusters::OnOff::Commands::Toggle::Id;
    data->ClusterId                    = Clusters::OnOff::Id;
    data->IsGroup                      = true;

    DeviceLayer::PlatformMgr().ScheduleWork(BindingHandler::SwitchWorkerHandler, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

} // namespace Group

void RegisterSwitchCommands()
{
    static const shell_command_t sSwitchSubCommands[] = {
        { &SwitchHelpHandler, "help", "Switch commands" },
        { &Unicast::OnOffCommandHandler, "onoff", "Usage: switch onoff [on|off|toggle]" },
        { &Group::SwitchCommandHandler, "groups", "Usage: switch groups onoff [on|off|toggle]" },
        { &TableCommandHelper, "table", "Print a binding table" }
    };

    static const shell_command_t sSwitchOnOffSubCommands[] = {
        { &Unicast::OnOffHelpHandler, "help", "Usage : switch ononff [on|off|toggle]" },
        { &Unicast::OnCommandHandler, "on", "Sends on command to bound lighting app" },
        { &Unicast::OffCommandHandler, "off", "Sends off command to bound lighting app" },
        { &Unicast::ToggleCommandHandler, "toggle", "Sends toggle command to bound lighting app" }
    };

    static const shell_command_t sSwitchGroupsSubCommands[] = {
        { &Group::SwitchHelpHandler, "help", "switch a group of bounded lightning apps" },
        { &Group::OnOffCommandHandler, "onoff", "Usage: switch groups onoff [on|off|toggle]" }
    };

    static const shell_command_t sSwichGroupsOnOffSubCommands[] = {
        { &Group::OnOffHelpHandler, "help", "Usage: switch groups onoff [on|off|toggle]" },
        { &Group::OnCommandHandler, "on", "Sends on command to bound Group" },
        { &Group::OffCommandHandler, "off", "Sends off command to bound Group" },
        { &Group::ToggleCommandHandler, "toggle", "Sends toggle command to bound Group" }
    };

    static const shell_command_t sSwitchCommand = { &SwitchCommandHandler, "switch",
                                                    "Light-switch commands. Usage: switch [onoff|groups]" };

    sShellSwitchGroupsOnOffSubCommands.RegisterCommands(sSwichGroupsOnOffSubCommands,
                                                        MATTER_ARRAY_SIZE(sSwichGroupsOnOffSubCommands));
    sShellSwitchOnOffSubCommands.RegisterCommands(sSwitchOnOffSubCommands, MATTER_ARRAY_SIZE(sSwitchOnOffSubCommands));
    sShellSwitchGroupsSubCommands.RegisterCommands(sSwitchGroupsSubCommands, MATTER_ARRAY_SIZE(sSwitchGroupsSubCommands));
    sShellSwitchSubCommands.RegisterCommands(sSwitchSubCommands, MATTER_ARRAY_SIZE(sSwitchSubCommands));

    Engine::Root().RegisterCommands(&sSwitchCommand, 1);
}

} // namespace SwitchCommands
#endif
