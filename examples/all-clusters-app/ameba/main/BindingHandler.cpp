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

#include "BindingHandler.h"
#include "OnOffCommands.h"
#include "LevelControlCommands.h"
#include "app/clusters/bindings/BindingManager.h"
#include "app/server/Server.h"
#include "controller/InvokeInteraction.h"
#include "controller/ReadInteraction.h"
#include "platform/CHIPDeviceLayer.h"
#include <app/clusters/bindings/bindings.h>
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

Engine sShellSwitchSubCommands;
Engine sShellSwitchColorControlSubCommands;
Engine sShellSwitchColorControlReadSubCommands;

Engine sShellSwitchGroupsSubCommands;

Engine sShellSwitchBindingSubCommands;
#endif // defined(ENABLE_CHIP_SHELL)

namespace {
void LightSwitchChangedHandler(const EmberBindingTableEntry & binding, OperationalDeviceProxy * peer_device, void * context)
{
    VerifyOrReturn(context != nullptr, ChipLogError(NotSpecified, "OnDeviceConnectedFn: context is null"));
    BindingCommandData * data = static_cast<BindingCommandData *>(context);

    if (data->isReadAttribute)
    {
        // It should always enter here if isReadAttribute is true
        if (binding.type == EMBER_UNICAST_BINDING && !data->isGroup)
        {
            switch (data->clusterId)
            {
            case Clusters::OnOff::Id:
                ProcessOnOffUnicastBindingRead(data, binding, peer_device);
                break;
            case Clusters::LevelControl::Id:
                ProcessLevelControlUnicastBindingRead(data, binding, peer_device);
                break;
            }
        }
    }
    else
    {
        if (binding.type == EMBER_MULTICAST_BINDING && data->isGroup)
        {
            switch (data->clusterId)
            {
            case Clusters::OnOff::Id:
                ProcessOnOffGroupBindingCommand(data, binding);
                break;
            }
        }
        else if (binding.type == EMBER_UNICAST_BINDING && !data->isGroup)
        {
            switch (data->clusterId)
            {
            case Clusters::OnOff::Id:
                ProcessOnOffUnicastBindingCommand(data, binding, peer_device);
                break;
            case Clusters::LevelControl::Id:
                ProcessLevelControlUnicastBindingCommand(data, binding, peer_device);
                break;
            }
        }
    }
}

void LightSwitchContextReleaseHandler(void * context)
{
    VerifyOrReturn(context != nullptr, ChipLogError(NotSpecified, "Invalid context for Light switch context release handler"));

    Platform::Delete(static_cast<BindingCommandData *>(context));
}

void InitBindingHandlerInternal(intptr_t arg)
{
    auto & server = chip::Server::GetInstance();
    chip::BindingManager::GetInstance().Init(
        { &server.GetFabricTable(), server.GetCASESessionManager(), &server.GetPersistentStorage() });
    chip::BindingManager::GetInstance().RegisterBoundDeviceChangedHandler(LightSwitchChangedHandler);
    chip::BindingManager::GetInstance().RegisterBoundDeviceContextReleaseHandler(LightSwitchContextReleaseHandler);
}

#ifdef CONFIG_ENABLE_CHIP_SHELL

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
    entry->type                    = EMBER_MULTICAST_BINDING;
    entry->fabricIndex             = atoi(argv[0]);
    entry->groupId                 = atoi(argv[1]);
    entry->local                   = 1; // Hardcoded to endpoint 1 for now
    entry->clusterId.SetValue(6);       // Hardcoded to OnOff cluster for now

    DeviceLayer::PlatformMgr().ScheduleWork(BindingWorkerFunction, reinterpret_cast<intptr_t>(entry));
    return CHIP_NO_ERROR;
}

CHIP_ERROR BindingUnicastBindCommandHandler(int argc, char ** argv)
{
    VerifyOrReturnError(argc == 3, CHIP_ERROR_INVALID_ARGUMENT);

    EmberBindingTableEntry * entry = Platform::New<EmberBindingTableEntry>();
    entry->type                    = EMBER_UNICAST_BINDING;
    entry->fabricIndex             = atoi(argv[0]);
    entry->nodeId                  = atoi(argv[1]);
    entry->local                   = 1; // Hardcoded to endpoint 1 for now
    entry->remote                  = atoi(argv[2]);
    entry->clusterId.SetValue(6); // Hardcode to OnOff cluster for now

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

/**
 * @brief configures switch matter shell
 *
 */
static void RegisterSwitchCommands()
{

    static const shell_command_t sSwitchSubCommands[] = {
        { &SwitchHelpHandler, "help", "Usage: switch <subcommand>" },
        { &OnOffSwitchCommandHandler, "onoff", " Usage: switch onoff <subcommand>" },
        { &LevelControlSwitchCommandHandler, "levelcontrol", " Usage: switch onoff <subcommand>" },
        { &GroupsSwitchCommandHandler, "groups", "Usage: switch groups <subcommand>" },
        { &BindingSwitchCommandHandler, "binding", "Usage: switch binding <subcommand>" }
    };

    static const shell_command_t sSwitchOnOffSubCommands[] = {
        { &OnOffHelpHandler, "help", "Usage: switch ononff <subcommand>" },
        { &OnSwitchCommandHandler, "on", "on Usage: switch onoff on" },
        { &OffSwitchCommandHandler, "off", "off Usage: switch onoff off" },
        { &ToggleSwitchCommandHandler, "toggle", "toggle Usage: switch onoff toggle" },
        { &OffWithEffectSwitchCommandHandler, "offWE", "off-with-effect Usage: switch onoff offWE <EffectId> <EffectVariant>" },
        { &OnWithRecallGlobalSceneSwitchCommandHandler, "onWRGS", "on-with-recall-global-scene Usage: switch onoff onWRGS" },
        { &OnWithTimedOffSwitchCommandHandler, "onWTO", "on-with-timed-off Usage: switch onoff onWTO <OnOffControl> <OnTime> <OffWaitTime>" },
        { &OnOffRead, "read", "Usage : switch levelcontrol read <attributeId>" }
    };

    static const shell_command_t sSwitchOnOffReadSubCommands[] = {
        { &OnOffReadHelpHandler, "help", "Usage : switch ononff read <attribute>" },
        { &OnOffReadOnOff, "onoff", "Read onoff attribute" },
        { &OnOffReadGlobalSceneControl, "GSC", "Read GlobalSceneControl attribute" },
        { &OnOffReadOnTime, "ontime", "Read ontime attribute" },
        { &OnOffReadOffWaitTime, "offwaittime", "Read offwaittime attribute" },
        { &OnOffReadStartUpOnOff, "SOO", "Read startuponoff attribute" },
    };

    static const shell_command_t sSwitchLevelControlSubCommands[] = {
        // fill in level control commands
        { &LevelControlHelpHandler, "help", "Usage: switch levelcontrol <subcommand>" },
        { &MoveToLevelSwitchCommandHandler, "movetolevel", "movetolevel Usage: switch levelcontrol movetolevel <level> <transitiontime> <optionsmask> <optionsoverride>" },
        { &MoveSwitchCommandHandler, "move", "move Usage: switch levelcontrol move <movemode> <rate> <optionsmask> <optionsoverride>" },
        { &StepSwitchCommandHandler, "step", "step Usage: switch levelcontrol step <stepmode> <stepsize> <transitiontime> <optionsmask> <optionsoverride>" },
        { &StopSwitchCommandHandler, "stop", "step Usage: switch levelcontrol stop <optionsmask> <optionsoverride>" },
        { &MoveToLevelWithOnOffSwitchCommandHandler, "MTLWOO", "movetolevelwithonoff Usage: switch levelcontrol MTLWOO <level> <transitiontime> <optionsmask> <optionsoverride>" },
        { &MoveWithOnOffSwitchCommandHandler, "MWOO", "movewithonoff Usage: switch levelcontrol MWOO <movemode> <rate> <optionsmask> <optionsoverride>" },
        { &StepWithOnOffSwitchCommandHandler, "stepWOO", "stepwithonoff Usage: switch levelcontrol stepWOO <stepmode> <stepsize> <transitiontime> <optionsmask> <optionsoverride>" },
        { &StopWithOnOffSwitchCommandHandler, "stopWOO", "stopwithonoff Usage: switch levelcontrol stopWOO <optionsmask> <optionsoverride>" },
        { &LevelControlRead, "read", "Usage : switch levelcontrol read <attributeId>" }
    };

    static const shell_command_t sSwitchLevelControlReadSubCommands[] = {
        // fill in read level control attributes commands
        { &LevelControlReadHelpHandler, "help", "Usage : switch levelcontrol read <attribute>" },
        { &LevelControlReadCurrentLevel, "currentlevel", "Read currentlevel attribute" },
        { &LevelControlReadRemainingTime, "remainingtime", "Read remainingtime attribute" },
        { &LevelControlReadMinLevel, "minlevel", "Read minlevel attribute" },
        { &LevelControlReadMaxLevel, "maxlevel", "Read maxlevel attribute" },
        { &LevelControlReadCurrentFrequency, "currentfrequency", "Read currentfrequency attribute" },
        { &LevelControlReadMinFrequency, "minfrequency", "Read minfrequency attribute" },
        { &LevelControlReadMaxFrequency, "maxfrequency", "Read maxfrequency attribute" },
        { &LevelControlReadOptions, "options", "Read options attribute" },
        { &LevelControlReadOnOffTransitionTime, "OOTT", "Read onofftransitiontime attribute" },
        { &LevelControlReadOnLevel, "onlevel", "Read onlevel attribute" },
        { &LevelControlReadOnTransitionTime, "OnTT", "Read ontransitiontime attribute" },
        { &LevelControlReadOffTransitionTime, "OffTT", "Read offtransitiontime attribute" },
        { &LevelControlReadDefaultMoveRate, "DMR", "Read defaultmoverate attribute" },
        { &LevelControlReadStartUpCurrentLevel, "SUCL", "Read startupcurrentlevel attribute" },
    };

    static const shell_command_t sSwitchColorControlSubCommands[] = {
        // fill in color control commands
    };

    static const shell_command_t sSwitchReadColorControlSubCommands[] = {
        // fill in read color control attributes commands
    };

    static const shell_command_t sSwitchThermostatSubCommands[] = {
        // fill in thermostat commands
    };

    static const shell_command_t sSwitchReadThermostatSubCommands[] = {
        // fill in read thermostat attributes commands
    };

    static const shell_command_t sSwitchGroupsSubCommands[] = { 
        { &GroupsHelpHandler, "help", "Usage: switch groups <subcommand>" },
        { &GroupsOnOffSwitchCommandHandler, "onoff", "Usage: switch groups onoff <subcommand>" } 
    };

    static const shell_command_t sSwitchGroupsOnOffSubCommands[] = {
        { &GroupsOnOffHelpHandler, "help", "Usage: switch groups onoff <subcommand>" },
        { &GroupOnSwitchCommandHandler, "on", "Sends on command to bound group" },
        { &GroupOffSwitchCommandHandler, "off", "Sends off command to bound group" },
        { &GroupToggleSwitchCommandHandler, "toggle", "Sends toggle command to group" },
        { &GroupOffWithEffectSwitchCommandHandler, "offWE", "Sends off-with-effect command to group" },
        { &GroupOnWithRecallGlobalSceneSwitchCommandHandler, "onWRGS", "Sends on-with-recall-global-scene command to group" },
        { &GroupOnWithTimedOffSwitchCommandHandler, "onWTO", "Sends on-with-timed-off command to group" }
    };

    static const shell_command_t sSwitchBindingSubCommands[] = {
        { &BindingHelpHandler, "help", "Usage: switch binding <subcommand>" },
        { &BindingGroupBindCommandHandler, "group", "Usage: switch binding group <fabric index> <group id>" },
        { &BindingUnicastBindCommandHandler, "unicast", "Usage: switch binding unicast <fabric index> <node id> <endpoint>" }
    };

    static const shell_command_t sSwitchCommand = { &SwitchCommandHandler, "switch",
                                                    "Light-switch commands. Usage: switch <subcommand>" };

    sShellSwitchGroupsOnOffSubCommands.RegisterCommands(sSwitchGroupsOnOffSubCommands, ArraySize(sSwitchGroupsOnOffSubCommands));
    sShellSwitchOnOffSubCommands.RegisterCommands(sSwitchOnOffSubCommands, ArraySize(sSwitchOnOffSubCommands));
    sShellSwitchOnOffReadSubCommands.RegisterCommands(sSwitchOnOffReadSubCommands, ArraySize(sSwitchOnOffReadSubCommands));
    sShellSwitchLevelControlSubCommands.RegisterCommands(sSwitchLevelControlSubCommands, ArraySize(sSwitchLevelControlSubCommands));
    sShellSwitchLevelControlReadSubCommands.RegisterCommands(sSwitchLevelControlReadSubCommands, ArraySize(sSwitchLevelControlReadSubCommands));
    sShellSwitchGroupsSubCommands.RegisterCommands(sSwitchGroupsSubCommands, ArraySize(sSwitchGroupsSubCommands));
    sShellSwitchBindingSubCommands.RegisterCommands(sSwitchBindingSubCommands, ArraySize(sSwitchBindingSubCommands));
    sShellSwitchSubCommands.RegisterCommands(sSwitchSubCommands, ArraySize(sSwitchSubCommands));

    Engine::Root().RegisterCommands(&sSwitchCommand, 1);
}
#endif // ENABLE_CHIP_SHELL

} // namespace

/********************************************************
 * Switch functions
 *********************************************************/

void SwitchWorkerFunction(intptr_t context)
{
    VerifyOrReturn(context != 0, ChipLogError(NotSpecified, "SwitchWorkerFunction - Invalid work data"));

    BindingCommandData * data = reinterpret_cast<BindingCommandData *>(context);
    BindingManager::GetInstance().NotifyBoundClusterChanged(data->localEndpointId, data->clusterId, static_cast<void *>(data));
}

void BindingWorkerFunction(intptr_t context)
{
    VerifyOrReturn(context != 0, ChipLogError(NotSpecified, "BindingWorkerFunction - Invalid work data"));

    EmberBindingTableEntry * entry = reinterpret_cast<EmberBindingTableEntry *>(context);
    AddBindingEntry(*entry);

    Platform::Delete(entry);
}

CHIP_ERROR InitBindingHandler()
{
    // The initialization of binding manager will try establishing connection with unicast peers
    // so it requires the Server instance to be correctly initialized. Post the init function to
    // the event queue so that everything is ready when initialization is conducted.
    chip::DeviceLayer::PlatformMgr().ScheduleWork(InitBindingHandlerInternal);
#if CONFIG_ENABLE_CHIP_SHELL
    RegisterSwitchCommands();
#endif
    return CHIP_NO_ERROR;
}
