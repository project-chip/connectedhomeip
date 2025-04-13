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
#include "ColorControlCommands.h"
#include "IdentifyCommand.h"
#include "LevelControlCommands.h"
#include "OnOffCommands.h"
#include "ThermostatCommands.h"
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
        if (binding.type == MATTER_UNICAST_BINDING && !data->isGroup)
        {
            switch (data->clusterId)
            {
            case Clusters::Identify::Id:
                ProcessIdentifyUnicastBindingRead(data, binding, peer_device);
                break;
            case Clusters::OnOff::Id:
                ProcessOnOffUnicastBindingRead(data, binding, peer_device);
                break;
            case Clusters::LevelControl::Id:
                ProcessLevelControlUnicastBindingRead(data, binding, peer_device);
                break;
            case Clusters::ColorControl::Id:
                ProcessColorControlUnicastBindingRead(data, binding, peer_device);
                break;
            case Clusters::Thermostat::Id:
                ProcessThermostatUnicastBindingRead(data, binding, peer_device);
                break;
            }
        }
    }
    else
    {
        if (binding.type == MATTER_MULTICAST_BINDING && data->isGroup)
        {
            switch (data->clusterId)
            {
            case Clusters::Identify::Id:
                ProcessIdentifyGroupBindingCommand(data, binding);
                break;
            case Clusters::OnOff::Id:
                ProcessOnOffGroupBindingCommand(data, binding);
                break;
            case Clusters::LevelControl::Id:
                ProcessColorControlGroupBindingCommand(data, binding);
                break;
            case Clusters::ColorControl::Id:
                ProcessColorControlGroupBindingCommand(data, binding);
                break;
            case Clusters::Thermostat::Id:
                ProcessThermostatGroupBindingCommand(data, binding);
                break;
            }
        }
        else if (binding.type == MATTER_UNICAST_BINDING && !data->isGroup)
        {
            switch (data->clusterId)
            {
            case Clusters::Identify::Id:
                ProcessIdentifyUnicastBindingCommand(data, binding, peer_device);
                break;
            case Clusters::OnOff::Id:
                ProcessOnOffUnicastBindingCommand(data, binding, peer_device);
                break;
            case Clusters::LevelControl::Id:
                ProcessLevelControlUnicastBindingCommand(data, binding, peer_device);
                break;
            case Clusters::ColorControl::Id:
                ProcessColorControlUnicastBindingCommand(data, binding, peer_device);
                break;
            case Clusters::Thermostat::Id:
                ProcessThermostatUnicastBindingCommand(data, binding, peer_device);
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

#if CONFIG_ENABLE_CHIP_SHELL

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

/**
 * @brief configures switch matter shell
 *
 */
static void RegisterSwitchCommands()
{

    static const shell_command_t sSwitchSubCommands[] = {
        { &SwitchHelpHandler, "help", "Usage: switch <subcommand>" },
        { &IdentifySwitchCommandHandler, "identify", " Usage: switch identify <subcommand>" },
        { &OnOffSwitchCommandHandler, "onoff", " Usage: switch onoff <subcommand>" },
        { &LevelControlSwitchCommandHandler, "levelcontrol", " Usage: switch levlecontrol <subcommand>" },
        { &ColorControlSwitchCommandHandler, "colorcontrol", " Usage: switch colorcontrol <subcommand>" },
        { &ThermostatSwitchCommandHandler, "thermostat", " Usage: switch thermostat <subcommand>" },
        { &GroupsSwitchCommandHandler, "groups", "Usage: switch groups <subcommand>" },
        { &BindingSwitchCommandHandler, "binding", "Usage: switch binding <subcommand>" }
    };

    static const shell_command_t sSwitchIdentifySubCommands[] = {
        { &IdentifyHelpHandler, "help", "Usage: switch identify <subcommand>" },
        { &IdentifyCommandHandler, "identify", "identify Usage: switch identify identify" },
        { &TriggerEffectSwitchCommandHandler, "triggereffect", "triggereffect Usage: switch identify triggereffect" },
        { &IdentifyRead, "read", "Usage : switch identify read <attribute>" }
    };

    static const shell_command_t sSwitchIdentifyReadSubCommands[] = {
        { &IdentifyReadHelpHandler, "help", "Usage : switch identify read <attribute>" },
        { &IdentifyReadAttributeList, "attlist", "Read attribute list" },
        { &IdentifyReadIdentifyTime, "identifytime", "Read identifytime attribute" },
        { &IdentifyReadIdentifyType, "identifytype", "Read identifytype attribute" },
    };

    static const shell_command_t sSwitchOnOffSubCommands[] = {
        { &OnOffHelpHandler, "help", "Usage: switch ononff <subcommand>" },
        { &OnSwitchCommandHandler, "on", "on Usage: switch onoff on" },
        { &OffSwitchCommandHandler, "off", "off Usage: switch onoff off" },
        { &ToggleSwitchCommandHandler, "toggle", "toggle Usage: switch onoff toggle" },
        { &OffWithEffectSwitchCommandHandler, "offWE", "off-with-effect Usage: switch onoff offWE <EffectId> <EffectVariant>" },
        { &OnWithRecallGlobalSceneSwitchCommandHandler, "onWRGS", "on-with-recall-global-scene Usage: switch onoff onWRGS" },
        { &OnWithTimedOffSwitchCommandHandler, "onWTO",
          "on-with-timed-off Usage: switch onoff onWTO <OnOffControl> <OnTime> <OffWaitTime>" },
        { &OnOffRead, "read", "Usage : switch onoff read <attribute>" }
    };

    static const shell_command_t sSwitchOnOffReadSubCommands[] = {
        { &OnOffReadHelpHandler, "help", "Usage : switch ononff read <attribute>" },
        { &OnOffReadAttributeList, "attlist", "Read attribute list" },
        { &OnOffReadOnOff, "onoff", "Read onoff attribute" },
        { &OnOffReadGlobalSceneControl, "GSC", "Read GlobalSceneControl attribute" },
        { &OnOffReadOnTime, "ontime", "Read ontime attribute" },
        { &OnOffReadOffWaitTime, "offwaittime", "Read offwaittime attribute" },
        { &OnOffReadStartUpOnOff, "SOO", "Read startuponoff attribute" },
    };

    static const shell_command_t sSwitchLevelControlSubCommands[] = {
        { &LevelControlHelpHandler, "help", "Usage: switch levelcontrol <subcommand>" },
        { &MoveToLevelSwitchCommandHandler, "movetolevel",
          "movetolevel Usage: switch levelcontrol movetolevel <level> <transitiontime> <optionsmask> <optionsoverride>" },
        { &MoveSwitchCommandHandler, "move",
          "move Usage: switch levelcontrol move <movemode> <rate> <optionsmask> <optionsoverride>" },
        { &StepSwitchCommandHandler, "step",
          "step Usage: switch levelcontrol step <stepmode> <stepsize> <transitiontime> <optionsmask> <optionsoverride>" },
        { &StopSwitchCommandHandler, "stop", "step Usage: switch levelcontrol stop <optionsmask> <optionsoverride>" },
        { &MoveToLevelWithOnOffSwitchCommandHandler, "MTLWOO",
          "movetolevelwithonoff Usage: switch levelcontrol MTLWOO <level> <transitiontime> <optionsmask> <optionsoverride>" },
        { &MoveWithOnOffSwitchCommandHandler, "MWOO",
          "movewithonoff Usage: switch levelcontrol MWOO <movemode> <rate> <optionsmask> <optionsoverride>" },
        { &StepWithOnOffSwitchCommandHandler, "stepWOO",
          "stepwithonoff Usage: switch levelcontrol stepWOO <stepmode> <stepsize> <transitiontime> <optionsmask> "
          "<optionsoverride>" },
        { &StopWithOnOffSwitchCommandHandler, "stopWOO",
          "stopwithonoff Usage: switch levelcontrol stopWOO <optionsmask> <optionsoverride>" },
        { &LevelControlRead, "read", "Usage : switch levelcontrol read <attribute>" }
    };

    static const shell_command_t sSwitchLevelControlReadSubCommands[] = {
        { &LevelControlReadHelpHandler, "help", "Usage : switch levelcontrol read <attribute>" },
        { &LevelControlReadAttributeList, "attlist", "Read attribute list" },
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
        { &ColorControlHelpHandler, "help", "Usage: switch colorcontrol <subcommand>" },
        { &MoveToHueCommandHandler, "movetohue",
          "movetohue Usage: switch colorcontrol movetohue <hue> <direction> <transitiontime> <optionsmask> <optionsoverride>" },
        { &MoveHueCommandHandler, "movehue",
          "movehue Usage: switch colorcontrol movehue <movemode> <rate> <optionsmask> <optionsoverride>" },
        { &StepHueCommandHandler, "stephue",
          "stephue Usage: switch colorcontrol stephue <stepmode> <stepsize> <transitiontime> <optionsmask> <optionsoverride>" },
        { &MoveToSaturationCommandHandler, "movetosat",
          "movetosaturation Usage: switch colorcontrol movetosat <saturation> <transitiontime> <optionsmask> <optionsoverride>" },
        { &MoveSaturationCommandHandler, "movesat",
          "movesaturation Usage: switch colorcontrol movesat <movemode> <rate> <optionsmask> <optionsoverride>" },
        { &StepSaturationCommandHandler, "stepsat",
          "stepsaturation Usage: switch colorcontrol stepsat <stepmode> <stepsize> <transitiontime> <optionsmask> "
          "<optionsoverride>" },
        { &MoveToHueAndSaturationCommandHandler, "movetoHS",
          "movetohueandsaturation Usage: switch colorcontrol movetoHS <hue> <saturation> <transitiontime> <optionsmask> "
          "<optionsoverride>" },
        { &MoveToColorCommandHandler, "movetocolor",
          "movetocolor Usage: switch colorcontrol movetocolor <colorx> <colory> <transitiontime> <optionsmask> <optionsoverride>" },
        { &MoveColorCommandHandler, "movecolor",
          "movecolor Usage: switch colorcontrol movecolor <ratex> <ratey> <optionsmask> <optionsoverride>" },
        { &StepColorCommandHandler, "stepcolor",
          "stepcolor Usage: switch colorcontrol stepcolor <stepx> <stepy> <transitiontime> <optionsmask> <optionsoverride>" },
        { &MoveToColorTemperatureCommandHandler, "movetoCT",
          "movetocolortemperature Usage: switch colorcontrol movetoCT <colortempmireds> <transitiontime> <optionsmask> "
          "<optionsoverride>" },
        { &EnhancedMoveToHueCommandHandler, "Emovetohue",
          "enhancedmovetohue Usage: switch colorcontrol Emovetohue <enhancedhue> <direction> <transitiontime> <optionsmask> "
          "<optionsoverride>" },
        { &EnhancedMoveHueCommandHandler, "Emovehue",
          "enhancedmovehue Usage: switch colorcontrol Emovehue <movemode> <rate> <optionsmask> <optionsoverride>" },
        { &EnhancedStepHueCommandHandler, "Estephue",
          "enhancedstephue Usage: switch colorcontrol Estephue <stepmode> <stepsize> <transitiontime> <optionsmask> "
          "<optionsoverride>" },
        { &EnhancedMoveToHueAndSaturationCommandHandler, "EmovetoHS",
          "enhancedmovetohueandsaturation Usage: switch colorcontrol EmovetoHS <enhancedhue> <saturation> <transitiontime> "
          "<optionsmask> <optionsoverride>" },
        { &ColorLoopSetCommandHandler, "colorloopset",
          "colorloopset Usage: switch colorcontrol colorloopset <updateflags> <action> <direction> <time> <starhue> <optionsmask> "
          "<optionsoverride>" },
        { &StopMoveStepCommandHandler, "stopmovestep",
          "stopmovestep Usage: switch colorcontrol stopmovestep <optionsmask> <optionsoverride>" },
        { &MoveColorTemperatureCommandHandler, "moveCT",
          "movecolortemperature Usage: switch colorcontrol moveCT <movemode> <rate> <colortempminmireds> <colortempmaxmireds> "
          "<optionsmask> <optionsoverride>" },
        { &StepColorTemperatureCommandHandler, "stepCT",
          "stepcolortemperature Usage: switch colorcontrol stepCT <stepmode> <stepsize> <transitiontime> <colortempminmireds> "
          "<colortempmaxmireds> <optionsmask> <optionsoverride>" },
        { &ColorControlRead, "read", "Usage : switch colorcontrol read <attribute>" }
    };

    static const shell_command_t sSwitchColorControlReadSubCommands[] = {
        // fill in read color control attributes commands
        { &ColorControlReadHelpHandler, "help", "Usage : switch colorcontrol read <attribute>" },
        { &ColorControlReadAttributeList, "attlist", "Read attribute list" },
        { &ColorControlReadCurrentHue, "currenthue", "Read currenthue attribute" },
        { &ColorControlReadCurrentSaturation, "currentsat", "Read currentsaturaion attribute" },
        { &ColorControlReadRemainingTime, "remaintime", "Read remainingtime attribute" },
        { &ColorControlReadCurrentX, "current-x", "Read current-x attribute" },
        { &ColorControlReadCurrentY, "current-y", "Read current-y attribute" },
        { &ColorControlReadDriftCompensation, "driftcomp", "Read driftcompensation attribute" },
        { &ColorControlReadCompensationText, "comptext", "Read compensationtext attribute" },
        { &ColorControlReadColorTemperature, "colortemp", "Read colortemperature attribute" },
        { &ColorControlReadColorMode, "colormode", "Read colormode attribute" },
        { &ColorControlReadOptions, "options", "Read options attribute" },
        { &ColorControlReadNumberOfPrimaries, "noofprimaries", "Read numberofprimaries attribute" },
        { &ColorControlReadPrimary1X, "primary1x", "Read primary1x attribute" },
        { &ColorControlReadPrimary1Y, "primary1y", "Read primary1y attribute" },
        { &ColorControlReadPrimary1Intensity, "primary1intensity", "Read primary1intensity attribute" },
        { &ColorControlReadPrimary2X, "primary2x", "Read primary2x attribute" },
        { &ColorControlReadPrimary2Y, "primary2y", "Read primary2y attribute" },
        { &ColorControlReadPrimary2Intensity, "primary2intensity", "Read primary2intensity attribute" },
        { &ColorControlReadPrimary3X, "primary3x", "Read primary3x attribute" },
        { &ColorControlReadPrimary3Y, "primary3y", "Read primary3y attribute" },
        { &ColorControlReadPrimary3Intensity, "primary3intensity", "Read primary3intensity attribute" },
        { &ColorControlReadPrimary4X, "primary4x", "Read primary4x attribute" },
        { &ColorControlReadPrimary4Y, "primary4y", "Read primary4y attribute" },
        { &ColorControlReadPrimary4Intensity, "primary4intensity", "Read primary4intensity attribute" },
        { &ColorControlReadPrimary5X, "primary5x", "Read primary5x attribute" },
        { &ColorControlReadPrimary5Y, "primary5y", "Read primary5y attribute" },
        { &ColorControlReadPrimary5Intensity, "primary5intensity", "Read primary5intensity attribute" },
        { &ColorControlReadPrimary6X, "primary6x", "Read primary6x attribute" },
        { &ColorControlReadPrimary6Y, "primary6y", "Read primary6y attribute" },
        { &ColorControlReadPrimary6Intensity, "primary6intensity", "Read primary6intensity attribute" },
        { &ColorControlReadWhitePointX, "whitepointx", "Read whitepointx attribute" },
        { &ColorControlReadWhitePointY, "whitepointy", "Read whitepointy attribute" },
        { &ColorControlReadColorPointRX, "colorpointrx", "Read colorpointrx attribute" },
        { &ColorControlReadColorPointRY, "colorpointry", "Read colorpointry attribute" },
        { &ColorControlReadColorPointRIntensity, "colorpointrintensity", "Read colorpointrintensity attribute" },
        { &ColorControlReadColorPointGX, "colorpointgx", "Read colorpointgx attribute" },
        { &ColorControlReadColorPointGY, "colorpointgy", "Read colorpointgy attribute" },
        { &ColorControlReadColorPointGIntensity, "colorpointgintensity", "Read colorpointgintensity attribute" },
        { &ColorControlReadColorPointBX, "colorpointbx", "Read colorpointbx attribute" },
        { &ColorControlReadColorPointBY, "colorpointby", "Read colorpointby attribute" },
        { &ColorControlReadColorPointBIntensity, "colorpointbintensity", "Read colorpointbintensity attribute" },
        { &ColorControlReadEnhancedCurrentHue, "Ecurrenthue", "Read enhancedcurrenthue attribute" },
        { &ColorControlReadEnhancedColorMode, "Ecolormode", "Read enhancedcolormode attribute" },
        { &ColorControlReadColorLoopActive, "colorloopactive", "Read colorloopactive attribute" },
        { &ColorControlReadColorLoopDirection, "colorloopdirection", "Read colorloopdirection attribute" },
        { &ColorControlReadColorLoopTime, "colorlooptime", "Read colorlooptime attribute" },
        { &ColorControlReadColorLoopStartEnhancedHue, "colorloopstartenhancedhue", "Read colorloopstartenhancedHue attribute" },
        { &ColorControlReadColorLoopStoredEnhancedHue, "colorloopstoredenhancedhue", "Read colorloopstoredenhancedHue attribute" },
        { &ColorControlReadColorCapabilities, "colorcapabilities", "Read colorcapabilities attribute" },
        { &ColorControlReadColorTempPhysicalMinMireds, "colortempphyminmireds", "Read colortempphysicalminmireds attribute" },
        { &ColorControlReadColorTempPhysicalMaxMireds, "colortempphymaxmireds", "Read colortempphysicalmaxmireds attribute" },
        { &ColorControlReadCoupleColorTempToLevelMinMireds, "CCTTMM", "Read couplecolortemptolevelminmireds attribute" },
        { &ColorControlReadStartUpColorTemperatureMireds, "SUCTM", "Read startupcolortempmireds attribute" },
    };

    static const shell_command_t sSwitchThermostatSubCommands[] = {
        { &ThermostatHelpHandler, "help", "Usage: switch thermostat <subcommand>" },
        { &SetpointRaiseLowerSwitchCommandHandler, "SPRL", "setpointraiselower Usage: switch thermostat SPRL <mode> <amount>" },
        { &ThermostatRead, "read", "Usage : switch thermostat read <attribute>" }
    };

    static const shell_command_t sSwitchThermostatReadSubCommands[] = {
        { &ThermostatReadHelpHandler, "help", "Usage : switch thermostat read <attribute>" },
        { &ThermostatReadAttributeList, "attlist", "Read attribute list" },
        { &ThermostatReadLocalTemperature, "localtemp", "Read localtemperature attribute" },
        { &ThermostatReadAbsMinHeatSetpointLimit, "absminHSL", "Read absminheatsetpointlimit attribute" },
        { &ThermostatReadAbsMaxHeatSetpointLimit, "absmaxHSL", "Read absmaxheatsetpointlimit attribute" },
        { &ThermostatReadAbsMinCoolSetpointLimit, "absminCSL", "Read absmincoolsetpointlimit attribute" },
        { &ThermostatReadAbsMaxCoolSetpointLimit, "absmaxCSL", "Read absmaxcoolsetpointlimit attribute" },
        { &ThermostatReadPiCoolingDemand, "picoolingdemand", "Read picoolingdemand attribute" },
        { &ThermostatReadPiHeatingDemand, "piheatingdemand", "Read piheatingdemand attribute" },
        { &ThermostatReadOccupiedCoolingSetpoint, "OCS", "Read occupiedcoolingsetpoint attribute" },
        { &ThermostatReadOccupiedHeatingSetpoint, "OHS", "Read occupiedheatingsetpoint attribute" },
        { &ThermostatReadMinHeatSetpointLimit, "minHSL", "Read minheatsetpointlimit attribute" },
        { &ThermostatReadMaxHeatSetpointLimit, "maxHSL", "Read maxheatsetpointlimit attribute" },
        { &ThermostatReadMinCoolSetpointLimit, "minCSL", "Read mincoolsetpointlimit attribute" },
        { &ThermostatReadMaxCoolSetpointLimit, "maxCSL", "Read maxcoolsetpointlimit attribute" },
        { &ThermostatReadControlSequenceOfOperation, "CSOP", "Read controlsequenceofoperation attribute" },
        { &ThermostatReadSystemMode, "systemmode", "Read systemmode attribute" },
    };

    static const shell_command_t sSwitchGroupsSubCommands[] = {
        { &GroupsHelpHandler, "help", "Usage: switch groups <subcommand>" },
        { &GroupsOnOffSwitchCommandHandler, "onoff", "Usage: switch groups onoff <subcommand>" },
        { &GroupsLevelControlSwitchCommandHandler, "levelcontrol", "Usage: switch groups levelcontrol <subcommand>" },
        { &GroupsColorControlSwitchCommandHandler, "colorcontrol", "Usage: switch groups colorcontrol <subcommand>" },
        { &GroupsThermostatSwitchCommandHandler, "thermostat", "Usage: switch groups thermostat <subcommand>" },
    };

    static const shell_command_t sSwitchGroupsIdentifySubCommands[] = {
        { &GroupsIdentifyHelpHandler, "help", "Usage: switch groups onoff <subcommand>" },
        { &GroupIdentifyCommandHandler, "identify", "Sends identify command to bound group" },
        { &GroupTriggerEffectSwitchCommandHandler, "triggereffect", "Sends triggereffect command to group" },
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

    static const shell_command_t sSwitchGroupsLevelControlSubCommands[] = {
        { &GroupsLevelControlHelpHandler, "help", "Usage: switch groupos levelcontrol <subcommand>" },
        { &GroupsMoveToLevelSwitchCommandHandler, "movetolevel",
          "movetolevel Usage: switch groups levelcontrol movetolevel <level> <transitiontime> <optionsmask> <optionsoverride>" },
        { &GroupsMoveSwitchCommandHandler, "move",
          "move Usage: switch groups levelcontrol move <movemode> <rate> <optionsmask> <optionsoverride>" },
        { &GroupsStepSwitchCommandHandler, "step",
          "step Usage: switch groups levelcontrol step <stepmode> <stepsize> <transitiontime> <optionsmask> <optionsoverride>" },
        { &GroupsStopSwitchCommandHandler, "stop", "step Usage: switch groups levelcontrol stop <optionsmask> <optionsoverride>" },
        { &GroupsMoveToLevelWithOnOffSwitchCommandHandler, "MTLWOO",
          "movetolevelwithonoff Usage: switch groups levelcontrol MTLWOO <level> <transitiontime> <optionsmask> "
          "<optionsoverride>" },
        { &GroupsMoveWithOnOffSwitchCommandHandler, "MWOO",
          "movewithonoff Usage: switch groups levelcontrol MWOO <movemode> <rate> <optionsmask> <optionsoverride>" },
        { &GroupsStepWithOnOffSwitchCommandHandler, "stepWOO",
          "stepwithonoff Usage: switch groups levelcontrol stepWOO <stepmode> <stepsize> <transitiontime> <optionsmask> "
          "<optionsoverride>" },
        { &GroupsStopWithOnOffSwitchCommandHandler, "stopWOO",
          "stopwithonoff Usage: switch groups levelcontrol stopWOO <optionsmask> <optionsoverride>" },
    };

    static const shell_command_t sSwitchGroupsColorControlSubCommands[] = {
        { &GroupsColorControlHelpHandler, "help", "Usage: switch groups colorcontrol <subcommand>" },
        { &GroupsMoveToHueCommandHandler, "movetohue",
          "movetohue Usage: switch groups colorcontrol movetohue <hue> <direction> <transitiontime> <optionsmask> "
          "<optionsoverride>" },
        { &GroupsMoveHueCommandHandler, "movehue",
          "movehue Usage: switch groups colorcontrol movehue <movemode> <rate> <optionsmask> <optionsoverride>" },
        { &GroupsStepHueCommandHandler, "stephue",
          "stephue Usage: switch groups colorcontrol stephue <stepmode> <stepsize> <transitiontime> <optionsmask> "
          "<optionsoverride>" },
        { &GroupsMoveToSaturationCommandHandler, "movetosat",
          "movetosaturation Usage: switch groups colorcontrol movetosat <saturation> <transitiontime> <optionsmask> "
          "<optionsoverride>" },
        { &GroupsMoveSaturationCommandHandler, "movesat",
          "movesaturation Usage: switch groups colorcontrol movesat <movemode> <rate> <optionsmask> <optionsoverride>" },
        { &GroupsStepSaturationCommandHandler, "stepsat",
          "stepsaturation Usage: switch groups colorcontrol stepsat <stepmode> <stepsize> <transitiontime> <optionsmask> "
          "<optionsoverride>" },
        { &GroupsMoveToHueAndSaturationCommandHandler, "movetoHS",
          "movetohueandsaturation Usage: switch groups colorcontrol movetoHS <hue> <saturation> <transitiontime> <optionsmask> "
          "<optionsoverride>" },
        { &GroupsMoveToColorCommandHandler, "movetocolor",
          "movetocolor Usage: switch groups colorcontrol movetocolor <colorx> <colory> <transitiontime> <optionsmask> "
          "<optionsoverride>" },
        { &GroupsMoveColorCommandHandler, "movecolor",
          "movecolor Usage: switch groups colorcontrol movecolor <ratex> <ratey> <optionsmask> <optionsoverride>" },
        { &GroupsStepColorCommandHandler, "stepcolor",
          "stepcolor Usage: switch groups colorcontrol stepcolor <stepx> <stepy> <transitiontime> <optionsmask> "
          "<optionsoverride>" },
        { &GroupsMoveToColorTemperatureCommandHandler, "movetoCT",
          "movetocolortemperature Usage: switch groups colorcontrol movetoCT <colortempmireds> <transitiontime> <optionsmask> "
          "<optionsoverride>" },
        { &GroupsEnhancedMoveToHueCommandHandler, "Emovetohue",
          "enhancedmovetohue Usage: switch groups colorcontrol Emovetohue <enhancedhue> <direction> <transitiontime> <optionsmask> "
          "<optionsoverride>" },
        { &GroupsEnhancedMoveHueCommandHandler, "Emovehue",
          "enhancedmovehue Usage: switch groups colorcontrol Emovehue <movemode> <rate> <optionsmask> <optionsoverride>" },
        { &GroupsEnhancedStepHueCommandHandler, "Estephue",
          "enhancedstephue Usage: switch gropus colorcontrol Estephue <stepmode> <stepsize> <transitiontime> <optionsmask> "
          "<optionsoverride>" },
        { &GroupsEnhancedMoveToHueAndSaturationCommandHandler, "EmovetoHS",
          "enhancedmovetohueandsaturation Usage: switch groups colorcontrol EmovetoHS <enhancedhue> <saturation> <transitiontime> "
          "<optionsmask> <optionsoverride>" },
        { &GroupsColorLoopSetCommandHandler, "colorloopset",
          "colorloopset Usage: switch groups colorcontrol colorloopset <updateflags> <action> <direction> <time> <starhue> "
          "<optionsmask> <optionsoverride>" },
        { &GroupsStopMoveStepCommandHandler, "stopmovestep",
          "stopmovestep Usage: switch groups colorcontrol stopmovestep <optionsmask> <optionsoverride>" },
        { &GroupsMoveColorTemperatureCommandHandler, "moveCT",
          "movecolortemperature Usage: switch groups colorcontrol moveCT <movemode> <rate> <colortempminmireds> "
          "<colortempmaxmireds> <optionsmask> <optionsoverride>" },
        { &GroupsStepColorTemperatureCommandHandler, "stepCT",
          "stepcolortemperature Usage: switch groups colorcontrol stepCT <stepmode> <stepsize> <transitiontime> "
          "<colortempminmireds> <colortempmaxmireds> <optionsmask> <optionsoverride>" },
    };

    static const shell_command_t sSwitchGroupsThermostatSubCommands[] = {
        { &GroupsThermostatHelpHandler, "help", "Usage: switch groups thermostat <subcommand>" },
        { &GroupsSetpointRaiseLowerSwitchCommandHandler, "SPRL",
          "setpointraiselower Usage: switch groups thermostat SPRL <mode> <amount>" },
    };

    static const shell_command_t sSwitchBindingSubCommands[] = {
        { &BindingHelpHandler, "help", "Usage: switch binding <subcommand>" },
        { &BindingGroupBindCommandHandler, "group", "Usage: switch binding group <fabric index> <group id>" },
        { &BindingUnicastBindCommandHandler, "unicast", "Usage: switch binding unicast <fabric index> <node id> <endpoint>" }
    };

    static const shell_command_t sSwitchCommand = { &SwitchCommandHandler, "switch",
                                                    "Light-switch commands. Usage: switch <subcommand>" };

    // Register groups command
    sShellSwitchGroupsIdentifySubCommands.RegisterCommands(sSwitchGroupsIdentifySubCommands,
                                                           MATTER_ARRAY_SIZE(sSwitchGroupsIdentifySubCommands));
    sShellSwitchGroupsOnOffSubCommands.RegisterCommands(sSwitchGroupsOnOffSubCommands,
                                                        MATTER_ARRAY_SIZE(sSwitchGroupsOnOffSubCommands));
    sShellSwitchGroupsLevelControlSubCommands.RegisterCommands(sSwitchGroupsLevelControlSubCommands,
                                                               MATTER_ARRAY_SIZE(sSwitchGroupsLevelControlSubCommands));
    sShellSwitchGroupsColorControlSubCommands.RegisterCommands(sSwitchGroupsColorControlSubCommands,
                                                               MATTER_ARRAY_SIZE(sSwitchGroupsColorControlSubCommands));
    sShellSwitchGroupsThermostatSubCommands.RegisterCommands(sSwitchGroupsThermostatSubCommands,
                                                             MATTER_ARRAY_SIZE(sSwitchGroupsThermostatSubCommands));

    // Register commands
    sShellSwitchIdentifySubCommands.RegisterCommands(sSwitchIdentifySubCommands, MATTER_ARRAY_SIZE(sSwitchIdentifySubCommands));
    sShellSwitchIdentifyReadSubCommands.RegisterCommands(sSwitchIdentifyReadSubCommands,
                                                         MATTER_ARRAY_SIZE(sSwitchIdentifyReadSubCommands));
    sShellSwitchOnOffSubCommands.RegisterCommands(sSwitchOnOffSubCommands, MATTER_ARRAY_SIZE(sSwitchOnOffSubCommands));
    sShellSwitchOnOffReadSubCommands.RegisterCommands(sSwitchOnOffReadSubCommands, MATTER_ARRAY_SIZE(sSwitchOnOffReadSubCommands));
    sShellSwitchLevelControlSubCommands.RegisterCommands(sSwitchLevelControlSubCommands,
                                                         MATTER_ARRAY_SIZE(sSwitchLevelControlSubCommands));
    sShellSwitchLevelControlReadSubCommands.RegisterCommands(sSwitchLevelControlReadSubCommands,
                                                             MATTER_ARRAY_SIZE(sSwitchLevelControlReadSubCommands));
    sShellSwitchColorControlSubCommands.RegisterCommands(sSwitchColorControlSubCommands,
                                                         MATTER_ARRAY_SIZE(sSwitchColorControlSubCommands));
    sShellSwitchColorControlReadSubCommands.RegisterCommands(sSwitchColorControlReadSubCommands,
                                                             MATTER_ARRAY_SIZE(sSwitchColorControlReadSubCommands));
    sShellSwitchThermostatSubCommands.RegisterCommands(sSwitchThermostatSubCommands,
                                                       MATTER_ARRAY_SIZE(sSwitchThermostatSubCommands));
    sShellSwitchThermostatReadSubCommands.RegisterCommands(sSwitchThermostatReadSubCommands,
                                                           MATTER_ARRAY_SIZE(sSwitchThermostatReadSubCommands));
    sShellSwitchGroupsSubCommands.RegisterCommands(sSwitchGroupsSubCommands, MATTER_ARRAY_SIZE(sSwitchGroupsSubCommands));
    sShellSwitchBindingSubCommands.RegisterCommands(sSwitchBindingSubCommands, MATTER_ARRAY_SIZE(sSwitchBindingSubCommands));
    sShellSwitchSubCommands.RegisterCommands(sSwitchSubCommands, MATTER_ARRAY_SIZE(sSwitchSubCommands));

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
