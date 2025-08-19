/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <App.h>
#include <AppShellCommands.h>
#include <BindingHandler.h>
#include <LEDWidget.h>
#include <LightSwitch.h>
#include <cycfg.h>
#include <lib/shell/Engine.h>
#include <lib/shell/commands/Help.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::app;
using namespace chip::Shell;

using Shell::Engine;
using Shell::shell_command_t;
using Shell::streamer_get;
using Shell::streamer_printf;

static Engine AppCmdSubCommands;
static Engine AppCmdLocalSubCommands;
static Engine AppCmdOnOffSubCommands;
static Engine AppCmdGroupsSubCommands;
static Engine AppCmdGroupsOnOffSubCommands;
static Engine AppCmdDebugSubCommands;
static Engine AppCmdIdentifySubCommands;
static Engine AppCmdIdentifyReadSubCommands;

static CHIP_ERROR AppCmdHelpHandler(int argc, char ** argv)
{
    AppCmdSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

static CHIP_ERROR AppCmdCommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return AppCmdHelpHandler(argc, argv);
    }

    return AppCmdSubCommands.ExecCommand(argc, argv);
}

namespace Local {

static CHIP_ERROR OnOffHelpHandler(int argc, char ** argv)
{
    AppCmdLocalSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

static CHIP_ERROR AppCommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return OnOffHelpHandler(argc, argv);
    }

    return AppCmdLocalSubCommands.ExecCommand(argc, argv);
}

CHIP_ERROR OnCommandHandler(int argc, char ** argv)
{
    streamer_printf(streamer_get(), "Turning the light on ...\n");
    LEDWid().Set(true, PLATFORM_LED_RED);

    return CHIP_NO_ERROR;
}

CHIP_ERROR OffCommandHandler(int argc, char ** argv)
{
    streamer_printf(streamer_get(), "Turning the light off ...\n");
    LEDWid().Set(false, PLATFORM_LED_RED);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ToggleCommandHandler(int argc, char ** argv)
{
    streamer_printf(streamer_get(), "Toggling the light ...\n");
    if (LEDWid().IsLEDOn())
    {
        LEDWid().Set(false, PLATFORM_LED_RED);
    }
    else
    {
        LEDWid().Set(true, PLATFORM_LED_RED);
    }

    return CHIP_NO_ERROR;
}

} // namespace Local

CHIP_ERROR AssignUnicastData(LightSwitch::Status mStatus)
{
    BindingHandler::BindingData * data = Platform::New<BindingHandler::BindingData>();
    data->EndpointId                   = LightSwitch::GetInstance().GetLightSwitchEndpointId();
    switch (mStatus)
    {
    case LightSwitch::Status::Toggle:
        data->CommandId = Clusters::OnOff::Commands::Toggle::Id;
        break;
    case LightSwitch::Status::On:
        data->CommandId = Clusters::OnOff::Commands::On::Id;
        break;
    case LightSwitch::Status::Off:
        data->CommandId = Clusters::OnOff::Commands::Off::Id;
        break;
    default:
        data->CommandId = Clusters::OnOff::Commands::Off::Id;
        printf("[unicast] default -> Clusters::OnOff::Commands::Off\n");
        break;
    }
    data->ClusterId = Clusters::OnOff::Id;

    DeviceLayer::PlatformMgr().ScheduleWork(BindingHandler::SwitchWorkerHandler, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

namespace Unicast {

static CHIP_ERROR OnOffHelpHandler(int argc, char ** argv)
{
    AppCmdOnOffSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

static CHIP_ERROR AppCommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return OnOffHelpHandler(argc, argv);
    }

    return AppCmdOnOffSubCommands.ExecCommand(argc, argv);
}

CHIP_ERROR OnCommandHandler(int argc, char ** argv)
{
    return AssignUnicastData(LightSwitch::Status::On);
}

CHIP_ERROR OffCommandHandler(int argc, char ** argv)
{
    return AssignUnicastData(LightSwitch::Status::Off);
}

CHIP_ERROR ToggleCommandHandler(int argc, char ** argv)
{
    return AssignUnicastData(LightSwitch::Status::Toggle);
}

} // namespace Unicast

CHIP_ERROR AssignGroupData(LightSwitch::Status mStatus)
{
    BindingHandler::BindingData * data = Platform::New<BindingHandler::BindingData>();
    data->EndpointId                   = LightSwitch::GetInstance().GetLightSwitchEndpointId();
    switch (mStatus)
    {
    case LightSwitch::Status::Toggle:
        data->CommandId = Clusters::OnOff::Commands::Toggle::Id;
        break;
    case LightSwitch::Status::On:
        data->CommandId = Clusters::OnOff::Commands::On::Id;
        break;
    case LightSwitch::Status::Off:
        data->CommandId = Clusters::OnOff::Commands::Off::Id;
        break;
    default:
        data->CommandId = Clusters::OnOff::Commands::Off::Id;
        printf("[group] default -> Clusters::OnOff::Commands::Off\n");
        break;
    }
    data->ClusterId = Clusters::OnOff::Id;
    data->IsGroup   = true;

    DeviceLayer::PlatformMgr().ScheduleWork(BindingHandler::SwitchWorkerHandler, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

namespace Group {

static CHIP_ERROR AppSwitchHelpHandler(int argc, char ** argv)
{
    AppCmdGroupsSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

static CHIP_ERROR AppCommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return AppSwitchHelpHandler(argc, argv);
    }

    return AppCmdGroupsSubCommands.ExecCommand(argc, argv);
}

static CHIP_ERROR OnOffHelpHandler(int argc, char ** argv)
{
    AppCmdGroupsOnOffSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

static CHIP_ERROR OnOffCommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return OnOffHelpHandler(argc, argv);
    }

    return AppCmdGroupsOnOffSubCommands.ExecCommand(argc, argv);
}

CHIP_ERROR OnCommandHandler(int argc, char ** argv)
{
    return AssignGroupData(LightSwitch::Status::On);
}

CHIP_ERROR OffCommandHandler(int argc, char ** argv)
{
    return AssignGroupData(LightSwitch::Status::Off);
}

CHIP_ERROR ToggleCommandHandler(int argc, char ** argv)
{
    return AssignGroupData(LightSwitch::Status::Toggle);
}

} // namespace Group

namespace Identify {

CHIP_ERROR IdentifyHelpHandler(int argc, char ** argv)
{
    AppCmdIdentifySubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR AppCommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return IdentifyHelpHandler(argc, argv);
    }

    return AppCmdIdentifySubCommands.ExecCommand(argc, argv);
}

CHIP_ERROR ReadHelpHandler(int argc, char ** argv)
{
    AppCmdIdentifyReadSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR Read(int argc, char ** argv)
{
    if (argc == 0)
    {
        return ReadHelpHandler(argc, argv);
    }

    return AppCmdIdentifyReadSubCommands.ExecCommand(argc, argv);
}

CHIP_ERROR ReadAttributeList(int argc, char ** argv)
{
    BindingHandler::BindingData * data = Platform::New<BindingHandler::BindingData>();
    data->attributeId                  = Clusters::Identify::Attributes::AttributeList::Id;
    data->ClusterId                    = Clusters::Identify::Id;
    data->isReadAttribute              = true;

    DeviceLayer::PlatformMgr().ScheduleWork(BindingHandler::SwitchWorkerHandler, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ReadFeatureMap(int argc, char ** argv)
{
    BindingHandler::BindingData * data = Platform::New<BindingHandler::BindingData>();
    data->attributeId                  = Clusters::Identify::Attributes::FeatureMap::Id;
    data->ClusterId                    = Clusters::Identify::Id;
    data->isReadAttribute              = true;

    DeviceLayer::PlatformMgr().ScheduleWork(BindingHandler::SwitchWorkerHandler, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

} // namespace Identify

namespace Debug {

static CHIP_ERROR DebugHelpHandler(int argc, char ** argv)
{
    AppCmdDebugSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

static CHIP_ERROR AppCommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return DebugHelpHandler(argc, argv);
    }

    return AppCmdDebugSubCommands.ExecCommand(argc, argv);
}

CHIP_ERROR StartBLEAdvertisingDebugCommandHandler(int argc, char * argv[])
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if ((argc > 0) && (strcmp(argv[0], "start") == 0))
    {
        printf("Start BLE advertising ...\n");
        err = StartBLEAdvertisingHandler();
    }
    else if ((argc > 0) && (strcmp(argv[0], "stop") == 0))
    {
        printf("Stop BLE advertising ...\n");
        err = StopBLEAdvertisingHandler();
    }
    else
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return err;
}

static CHIP_ERROR TableDebugCommandHandler(int argc, char ** argv)
{
    BindingHandler::GetInstance().PrintBindingTable();
    return CHIP_NO_ERROR;
}

/*
 * Usage: switch debug bind_group [fabric index] [group id] [cluster id]
 */
CHIP_ERROR GroupBindCommandHandler(int argc, char ** argv)
{
    VerifyOrReturnError(argc == 3, CHIP_ERROR_INVALID_ARGUMENT);

    EmberBindingTableEntry * entry = Platform::New<EmberBindingTableEntry>();
    entry->type                    = MATTER_MULTICAST_BINDING;
    entry->local                   = 1; // Hardcoded to endpoint 1 for now
    entry->fabricIndex             = atoi(argv[0]);
    entry->groupId                 = atoi(argv[1]);
    entry->clusterId.emplace(atoi(argv[3]));

    DeviceLayer::PlatformMgr().ScheduleWork(BindingHandler::BindingWorkerHandler, reinterpret_cast<intptr_t>(entry));
    return CHIP_NO_ERROR;
}

/*
 * Usage: switch debug bind_unicast [fabric index] [node id] [endpoint] [cluster id]
 */
CHIP_ERROR UnicastBindCommandHandler(int argc, char ** argv)
{
    VerifyOrReturnError(argc == 4, CHIP_ERROR_INVALID_ARGUMENT);

    EmberBindingTableEntry * entry = Platform::New<EmberBindingTableEntry>();
    entry->type                    = MATTER_UNICAST_BINDING;
    entry->local                   = 1; // Hardcoded to endpoint 1 for now
    entry->fabricIndex             = atoi(argv[0]);
    entry->nodeId                  = atoi(argv[1]);
    entry->remote                  = atoi(argv[2]);
    entry->clusterId.emplace(atoi(argv[3]));

    DeviceLayer::PlatformMgr().ScheduleWork(BindingHandler::BindingWorkerHandler, reinterpret_cast<intptr_t>(entry));
    return CHIP_NO_ERROR;
}

/*
 * Usage: switch debug brightness [brightness value]
 */
CHIP_ERROR ChangeBrightnessCommandHandler(int argc, char ** argv)
{
    LightSwitch::GetInstance().DimmerChangeBrightness(atoi(argv[0]));
    return CHIP_NO_ERROR;
}

} // namespace Debug

void RegisterAppShellCommands()
{
    static const shell_command_t ifxAppCmdSubCommands[] = {
        { &AppCmdHelpHandler, "help", "Switch commands" },
        { &Local::AppCommandHandler, "local", "Light-switch on/off local device." },
        { &Unicast::AppCommandHandler, "onoff", "Lightbulb on/off remote device by unicast." },
        { &Group::AppCommandHandler, "groups", "Lightbulb on/off remote device by group." },
        { &Debug::AppCommandHandler, "debug", "Extend the debug command." },
        { &Identify::AppCommandHandler, "identify", "identify read attribute" },
    };

    static const shell_command_t ifxAppCmdLocalSubCommands[] = { { &Local::OnOffHelpHandler, "help",
                                                                   "Usage: switch local [on|off|toggle]" },
                                                                 { &Local::OnCommandHandler, "on", "Make light on" },
                                                                 { &Local::OffCommandHandler, "off", "Make light off" },
                                                                 { &Local::ToggleCommandHandler, "toggle", "Toggle the light" } };

    static const shell_command_t ifxAppCmdOnOffSubCommands[] = {
        { &Unicast::OnOffHelpHandler, "help", "Usage: switch onoff [on|off|toggle]" },
        { &Unicast::OnCommandHandler, "on", "Sends on command to bound Lightbulb" },
        { &Unicast::OffCommandHandler, "off", "Sends off command to bound Lightbulb" },
        { &Unicast::ToggleCommandHandler, "toggle", "Sends toggle command to bound Lightbulb" },
    };

    static const shell_command_t ifxAppCmdGroupsSubCommands[] = {
        { &Group::AppSwitchHelpHandler, "help", "Switch a group of bounded Lightbulbs" },
        { &Group::OnOffCommandHandler, "onoff", "Usage: switch groups onoff [on|off|toggle]" },
    };

    static const shell_command_t ifxAppCmdGroupsOnOffSubCommands[] = {
        { &Group::OnOffHelpHandler, "help", "Usage: switch groups onoff [on|off|toggle]" },
        { &Group::OnCommandHandler, "on", "Sends on command to bound Group" },
        { &Group::OffCommandHandler, "off", "Sends off command to bound Group" },
        { &Group::ToggleCommandHandler, "toggle", "Sends toggle command to bound Group" },
    };

    static const shell_command_t ifxAppCmdDebugSubCommands[] = {
        { &Debug::DebugHelpHandler, "help", "Debug use commands " },
        { &Debug::StartBLEAdvertisingDebugCommandHandler, "bleadv", "Usage: switch debug bleadv [start|stop]" },
        { &Debug::TableDebugCommandHandler, "table", "Print a binding table. Usage: switch debug table" },
        { &Debug::GroupBindCommandHandler, "bind_group", "Usage: switch debug bind_group [fabric index] [group id] [cluster id]" },
        { &Debug::UnicastBindCommandHandler, "bind_unicast",
          "Usage: switch debug bind_unicast [fabric index] [node id] [endpoint] [cluster id]" },
        { &Debug::ChangeBrightnessCommandHandler, "brightness",
          "Change the brightness and range is 0-254. Usage: switch debug brightness [brightness value]" },
    };

    static const shell_command_t ifxAppCmdIdentifySubCommands[] = {
        { &Identify::Read, "read", "Usage: switch identify read <attribute>" },
    };

    static const shell_command_t ifxAppCmdIdentifyReadSubCommands[] = {
        { &Identify::ReadHelpHandler, "help", "Usage: switch identify read <attribute>" },
        { &Identify::ReadAttributeList, "attlist", "attribute list attribute" },
        { &Identify::ReadFeatureMap, "featureMap", "featureMap attribute" },
    };

    static const shell_command_t AppLightSwitchCommand = {
        &AppCmdCommandHandler, "switch", "Light switch commands. Usage: switch [local|onoff|groups|debug][identify]"
    };

    AppCmdSubCommands.RegisterCommands(ifxAppCmdSubCommands, MATTER_ARRAY_SIZE(ifxAppCmdSubCommands));
    AppCmdLocalSubCommands.RegisterCommands(ifxAppCmdLocalSubCommands, MATTER_ARRAY_SIZE(ifxAppCmdLocalSubCommands));
    AppCmdOnOffSubCommands.RegisterCommands(ifxAppCmdOnOffSubCommands, MATTER_ARRAY_SIZE(ifxAppCmdOnOffSubCommands));
    AppCmdGroupsSubCommands.RegisterCommands(ifxAppCmdGroupsSubCommands, MATTER_ARRAY_SIZE(ifxAppCmdGroupsSubCommands));
    AppCmdGroupsOnOffSubCommands.RegisterCommands(ifxAppCmdGroupsOnOffSubCommands,
                                                  MATTER_ARRAY_SIZE(ifxAppCmdGroupsOnOffSubCommands));
    AppCmdDebugSubCommands.RegisterCommands(ifxAppCmdDebugSubCommands, MATTER_ARRAY_SIZE(ifxAppCmdDebugSubCommands));
    AppCmdIdentifySubCommands.RegisterCommands(ifxAppCmdIdentifySubCommands, MATTER_ARRAY_SIZE(ifxAppCmdIdentifySubCommands));
    AppCmdIdentifyReadSubCommands.RegisterCommands(ifxAppCmdIdentifyReadSubCommands,
                                                   MATTER_ARRAY_SIZE(ifxAppCmdIdentifyReadSubCommands));

    Engine::Root().RegisterCommands(&AppLightSwitchCommand, 1);
}
