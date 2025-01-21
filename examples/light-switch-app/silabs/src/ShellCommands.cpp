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
Engine sShellSwitchLevelControlSubCommands;

Engine sShellSwitchGroupsSubCommands;
Engine sShellSwitchGroupsOnOffSubCommands;
Engine sShellSwitchGroupsLevelControlSubCommands;

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

/********************************************************
 * LevelControl switch shell functions
 *********************************************************/

CHIP_ERROR LevelControlHelpHandler(int argc, char ** argv)
{
    sShellSwitchLevelControlSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR LevelControlSwitchCommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return LevelControlHelpHandler(argc, argv);
    }

    return sShellSwitchLevelControlSubCommands.ExecCommand(argc, argv);
}

CHIP_ERROR MoveToLevelSwitchCommandHandler(int argc, char ** argv)
{
    if (argc != 4)
    {
        return LevelControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->commandId           = Clusters::LevelControl::Commands::MoveToLevel::Id;
    data->clusterId           = Clusters::LevelControl::Id;
    data->commandData = BindingCommandData::MoveToLevel{};
    char * endPtr;
    if (auto *moveToLevel = std::get_if<BindingCommandData::MoveToLevel>(&data->commandData))
    {
        moveToLevel->level = static_cast<uint8_t>(strtol(argv[0], &endPtr, 10));
        moveToLevel->transitionTime = DataModel::Nullable<uint16_t>(strtol(argv[1], &endPtr, 10));
        moveToLevel->optionsMask = chip::BitMask<OptionsBitmap>(strtol(argv[2], &endPtr, 10));
        moveToLevel->optionsOverride = chip::BitMask<OptionsBitmap>(strtol(argv[3], &endPtr, 10));
    }
    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR MoveSwitchCommandHandler(int argc, char ** argv)
{
    if (argc != 4)
    {
        return LevelControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->commandId           = Clusters::LevelControl::Commands::Move::Id;
    data->clusterId           = Clusters::LevelControl::Id;
    data->commandData = BindingCommandData::Move{};
    char * endPtr;
    if (auto *move = std::get_if<BindingCommandData::Move>(&data->commandData))
    {
        move->moveMode = static_cast<Clusters::LevelControl::MoveModeEnum>(strtol(argv[0], &endPtr, 10));
        move->rate = static_cast<DataModel::Nullable<uint8_t>>(strtol(argv[1], &endPtr, 10));
        move->optionsMask = chip::BitMask<OptionsBitmap>(strtol(argv[2], &endPtr, 10));
        move->optionsOverride = chip::BitMask<OptionsBitmap>(strtol(argv[3], &endPtr, 10));
    }

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR StepSwitchCommandHandler(int argc, char ** argv)
{
    if (argc != 5)
    {
        return LevelControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->commandId           = Clusters::LevelControl::Commands::Step::Id;
    data->clusterId           = Clusters::LevelControl::Id;
    char * endPtr;
    data->commandData = BindingCommandData::Step{};
    if (auto *step = std::get_if<BindingCommandData::Step>(&data->commandData))
    {
        step->stepMode = static_cast<Clusters::LevelControl::StepModeEnum>(strtol(argv[0], &endPtr, 10));
        step->stepSize = static_cast<uint8_t>(strtol(argv[1], &endPtr, 10));
        step->transitionTime = DataModel::Nullable<uint16_t>(strtol(argv[2], &endPtr, 10));
        step->optionsMask = chip::BitMask<OptionsBitmap>(strtol(argv[3], &endPtr, 10));
        step->optionsOverride = chip::BitMask<OptionsBitmap>(strtol(argv[4], &endPtr, 10));
    }
    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR StopSwitchCommandHandler(int argc, char ** argv)
{
    if (argc != 2)
    {
        return LevelControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->commandId           = Clusters::LevelControl::Commands::Stop::Id;
    data->clusterId           = Clusters::LevelControl::Id;
    char * endPtr;
    data->commandData = BindingCommandData::Stop{};
    if (auto *stop = std::get_if<BindingCommandData::Stop>(&data->commandData))
    {
        stop->optionsMask = chip::BitMask<OptionsBitmap>(strtol(argv[0], &endPtr, 10));
        stop->optionsOverride = chip::BitMask<OptionsBitmap>(strtol(argv[1], &endPtr, 10));
    }

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR MoveToLevelWithOnOffSwitchCommandHandler(int argc, char ** argv)
{
    if (argc != 4)
    {
        return LevelControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->commandId           = Clusters::LevelControl::Commands::MoveToLevelWithOnOff::Id;
    data->clusterId           = Clusters::LevelControl::Id;
    data->commandData = BindingCommandData::MoveToLevel{};
    char * endPtr;
    if (auto *moveToLevel = std::get_if<BindingCommandData::MoveToLevel>(&data->commandData))
    {
        moveToLevel->level = static_cast<uint8_t>(strtol(argv[0], &endPtr, 10));
        moveToLevel->transitionTime = DataModel::Nullable<uint16_t>(strtol(argv[1], &endPtr, 10));
        moveToLevel->optionsMask = chip::BitMask<OptionsBitmap>(strtol(argv[2], &endPtr, 10));
        moveToLevel->optionsOverride = chip::BitMask<OptionsBitmap>(strtol(argv[3], &endPtr, 10));
    }

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR MoveWithOnOffSwitchCommandHandler(int argc, char ** argv)
{
    if (argc != 4)
    {
        return LevelControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->commandId           = Clusters::LevelControl::Commands::MoveWithOnOff::Id;
    data->clusterId           = Clusters::LevelControl::Id;
    data->commandData = BindingCommandData::Move{};
    char * endPtr;
    if (auto *move = std::get_if<BindingCommandData::Move>(&data->commandData))
    {
        move->moveMode = static_cast<Clusters::LevelControl::MoveModeEnum>(strtol(argv[0], &endPtr, 10));
        move->rate = static_cast<DataModel::Nullable<uint8_t>>(strtol(argv[1], &endPtr, 10));
        move->optionsMask = chip::BitMask<OptionsBitmap>(strtol(argv[2], &endPtr, 10));
        move->optionsOverride = chip::BitMask<OptionsBitmap>(strtol(argv[3], &endPtr, 10));
    }

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR StepWithOnOffSwitchCommandHandler(int argc, char ** argv)
{
    if (argc != 5)
    {
        return LevelControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->commandId           = Clusters::LevelControl::Commands::StepWithOnOff::Id;
    data->clusterId           = Clusters::LevelControl::Id;
    char * endPtr;
    data->commandData = BindingCommandData::Step{};
    if (auto *step = std::get_if<BindingCommandData::Step>(&data->commandData))
    {
        step->stepMode = static_cast<Clusters::LevelControl::StepModeEnum>(strtol(argv[0], &endPtr, 10));
        step->stepSize = static_cast<uint8_t>(strtol(argv[1], &endPtr, 10));
        step->transitionTime = DataModel::Nullable<uint16_t>(strtol(argv[2], &endPtr, 10));
        step->optionsMask = chip::BitMask<OptionsBitmap>(strtol(argv[3], &endPtr, 10));
        step->optionsOverride = chip::BitMask<OptionsBitmap>(strtol(argv[4], &endPtr, 10));
    }

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR StopWithOnOffSwitchCommandHandler(int argc, char ** argv)
{
    if (argc != 2)
    {
        return LevelControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->commandId           = Clusters::LevelControl::Commands::StopWithOnOff::Id;
    data->clusterId           = Clusters::LevelControl::Id;
    char * endPtr;
    data->commandData = BindingCommandData::Stop{};
    if (auto *stop = std::get_if<BindingCommandData::Stop>(&data->commandData))
    {
        stop->optionsMask = chip::BitMask<OptionsBitmap>(strtol(argv[0], &endPtr, 10));
        stop->optionsOverride = chip::BitMask<OptionsBitmap>(strtol(argv[1], &endPtr, 10));
    }

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

#if 0
/********************************************************
 * LevelControl Read switch shell functions
 *********************************************************/

CHIP_ERROR LevelControlReadHelpHandler(int argc, char ** argv)
{
    sShellSwitchLevelControlReadSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR LevelControlRead(int argc, char ** argv)
{
    if (argc == 0)
    {
        return LevelControlReadHelpHandler(argc, argv);
    }

    return sShellSwitchLevelControlReadSubCommands.ExecCommand(argc, argv);
}

CHIP_ERROR LevelControlReadAttributeList(int argc, char ** argv)
{
    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->attributeId         = Clusters::LevelControl::Attributes::AttributeList::Id;
    data->clusterId           = Clusters::LevelControl::Id;
    data->isReadAttribute     = true;

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR LevelControlReadCurrentLevel(int argc, char ** argv)
{
    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->attributeId         = Clusters::LevelControl::Attributes::CurrentLevel::Id;
    data->clusterId           = Clusters::LevelControl::Id;
    data->isReadAttribute     = true;
    ChipLogProgress(NotSpecified, "Read cluster=0x%x, attribute=0x%08x", data->clusterId, data->attributeId);
    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR LevelControlReadRemainingTime(int argc, char ** argv)
{
    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->attributeId         = Clusters::LevelControl::Attributes::RemainingTime::Id;
    data->clusterId           = Clusters::LevelControl::Id;
    data->isReadAttribute     = true;
    ChipLogProgress(NotSpecified, "Read cluster=0x%x, attribute=0x%08x", data->clusterId, data->attributeId);
    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR LevelControlReadMinLevel(int argc, char ** argv)
{
    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->attributeId         = Clusters::LevelControl::Attributes::MinLevel::Id;
    data->clusterId           = Clusters::LevelControl::Id;
    data->isReadAttribute     = true;
    ChipLogProgress(NotSpecified, "Read cluster=0x%x, attribute=0x%08x", data->clusterId, data->attributeId);
    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR LevelControlReadMaxLevel(int argc, char ** argv)
{
    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->attributeId         = Clusters::LevelControl::Attributes::MaxLevel::Id;
    data->clusterId           = Clusters::LevelControl::Id;
    data->isReadAttribute     = true;
    ChipLogProgress(NotSpecified, "Read cluster=0x%x, attribute=0x%08x", data->clusterId, data->attributeId);
    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR LevelControlReadCurrentFrequency(int argc, char ** argv)
{
    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->attributeId         = Clusters::LevelControl::Attributes::CurrentFrequency::Id;
    data->clusterId           = Clusters::LevelControl::Id;
    data->isReadAttribute     = true;
    ChipLogProgress(NotSpecified, "Read cluster=0x%x, attribute=0x%08x", data->clusterId, data->attributeId);
    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR LevelControlReadMinFrequency(int argc, char ** argv)
{
    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->attributeId         = Clusters::LevelControl::Attributes::MinFrequency::Id;
    data->clusterId           = Clusters::LevelControl::Id;
    data->isReadAttribute     = true;
    ChipLogProgress(NotSpecified, "Read cluster=0x%x, attribute=0x%08x", data->clusterId, data->attributeId);
    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR LevelControlReadMaxFrequency(int argc, char ** argv)
{
    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->attributeId         = Clusters::LevelControl::Attributes::MaxFrequency::Id;
    data->clusterId           = Clusters::LevelControl::Id;
    data->isReadAttribute     = true;
    ChipLogProgress(NotSpecified, "Read cluster=0x%x, attribute=0x%08x", data->clusterId, data->attributeId);
    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR LevelControlReadOptions(int argc, char ** argv)
{
    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->attributeId         = Clusters::LevelControl::Attributes::Options::Id;
    data->clusterId           = Clusters::LevelControl::Id;
    data->isReadAttribute     = true;
    ChipLogProgress(NotSpecified, "Read cluster=0x%x, attribute=0x%08x", data->clusterId, data->attributeId);
    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR LevelControlReadOnOffTransitionTime(int argc, char ** argv)
{
    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->attributeId         = Clusters::LevelControl::Attributes::OnOffTransitionTime::Id;
    data->clusterId           = Clusters::LevelControl::Id;
    data->isReadAttribute     = true;
    ChipLogProgress(NotSpecified, "Read cluster=0x%x, attribute=0x%08x", data->clusterId, data->attributeId);
    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR LevelControlReadOnLevel(int argc, char ** argv)
{
    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->attributeId         = Clusters::LevelControl::Attributes::OnLevel::Id;
    data->clusterId           = Clusters::LevelControl::Id;
    data->isReadAttribute     = true;
    ChipLogProgress(NotSpecified, "Read cluster=0x%x, attribute=0x%08x", data->clusterId, data->attributeId);
    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR LevelControlReadOnTransitionTime(int argc, char ** argv)
{
    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->attributeId         = Clusters::LevelControl::Attributes::OnTransitionTime::Id;
    data->clusterId           = Clusters::LevelControl::Id;
    data->isReadAttribute     = true;
    ChipLogProgress(NotSpecified, "Read cluster=0x%x, attribute=0x%08x", data->clusterId, data->attributeId);
    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR LevelControlReadOffTransitionTime(int argc, char ** argv)
{
    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->attributeId         = Clusters::LevelControl::Attributes::OffTransitionTime::Id;
    data->clusterId           = Clusters::LevelControl::Id;
    data->isReadAttribute     = true;
    ChipLogProgress(NotSpecified, "Read cluster=0x%x, attribute=0x%08x", data->clusterId, data->attributeId);
    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR LevelControlReadDefaultMoveRate(int argc, char ** argv)
{
    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->attributeId         = Clusters::LevelControl::Attributes::DefaultMoveRate::Id;
    data->clusterId           = Clusters::LevelControl::Id;
    data->isReadAttribute     = true;
    ChipLogProgress(NotSpecified, "Read cluster=0x%x, attribute=0x%08x", data->clusterId, data->attributeId);
    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR LevelControlReadStartUpCurrentLevel(int argc, char ** argv)
{
    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->attributeId         = Clusters::LevelControl::Attributes::StartUpCurrentLevel::Id;
    data->clusterId           = Clusters::LevelControl::Id;
    data->isReadAttribute     = true;
    ChipLogProgress(NotSpecified, "Read cluster=0x%x, attribute=0x%08x", data->clusterId, data->attributeId);
    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

#endif // commenting the read functions

/********************************************************
 * Groups LevelControl switch shell functions
 *********************************************************/

CHIP_ERROR GroupsLevelControlHelpHandler(int argc, char ** argv)
{
    sShellSwitchGroupsLevelControlSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupsLevelControlSwitchCommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return GroupsLevelControlHelpHandler(argc, argv);
    }

    return sShellSwitchGroupsLevelControlSubCommands.ExecCommand(argc, argv);
}

CHIP_ERROR GroupsMoveToLevelSwitchCommandHandler(int argc, char ** argv)
{
    if (argc != 4)
    {
        return GroupsLevelControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->commandId           = Clusters::LevelControl::Commands::MoveToLevel::Id;
    data->clusterId           = Clusters::LevelControl::Id;
    data->commandData = BindingCommandData::MoveToLevel{};
    char * endPtr;
    if (auto *moveToLevel = std::get_if<BindingCommandData::MoveToLevel>(&data->commandData))
    {
        moveToLevel->level = static_cast<uint8_t>(strtol(argv[0], &endPtr, 10));
        moveToLevel->transitionTime = DataModel::Nullable<uint16_t>(strtol(argv[1], &endPtr, 10));
        moveToLevel->optionsMask = chip::BitMask<OptionsBitmap>(strtol(argv[2], &endPtr, 10));
        moveToLevel->optionsOverride = chip::BitMask<OptionsBitmap>(strtol(argv[3], &endPtr, 10));
    }
    data->isGroup             = true;

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupsMoveSwitchCommandHandler(int argc, char ** argv)
{
    if (argc != 4)
    {
        return GroupsLevelControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->commandId           = Clusters::LevelControl::Commands::Move::Id;
    data->clusterId           = Clusters::LevelControl::Id;
    data->commandData = BindingCommandData::Move{};
    char * endPtr;
    if (auto *move = std::get_if<BindingCommandData::Move>(&data->commandData))
    {
        move->moveMode = static_cast<Clusters::LevelControl::MoveModeEnum>(strtol(argv[0], &endPtr, 10));
        move->rate = static_cast<DataModel::Nullable<uint8_t>>(strtol(argv[1], &endPtr, 10));
        move->optionsMask = chip::BitMask<OptionsBitmap>(strtol(argv[2], &endPtr, 10));
        move->optionsOverride = chip::BitMask<OptionsBitmap>(strtol(argv[3], &endPtr, 10));
    }
    data->isGroup             = true;

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupsStepSwitchCommandHandler(int argc, char ** argv)
{
    if (argc != 5)
    {
        return GroupsLevelControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->commandId           = Clusters::LevelControl::Commands::Step::Id;
    data->clusterId           = Clusters::LevelControl::Id;
    char * endPtr;
    data->commandData = BindingCommandData::Step{};
    if (auto *step = std::get_if<BindingCommandData::Step>(&data->commandData))
    {
        step->stepMode = static_cast<Clusters::LevelControl::StepModeEnum>(strtol(argv[0], &endPtr, 10));
        step->stepSize = static_cast<uint8_t>(strtol(argv[1], &endPtr, 10));
        step->transitionTime = DataModel::Nullable<uint16_t>(strtol(argv[2], &endPtr, 10));
        step->optionsMask = chip::BitMask<OptionsBitmap>(strtol(argv[3], &endPtr, 10));
        step->optionsOverride = chip::BitMask<OptionsBitmap>(strtol(argv[4], &endPtr, 10));
    }
    data->isGroup             = true;

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupsStopSwitchCommandHandler(int argc, char ** argv)
{
    if (argc != 2)
    {
        return GroupsLevelControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->commandId           = Clusters::LevelControl::Commands::Stop::Id;
    data->clusterId           = Clusters::LevelControl::Id;
    char * endPtr;
    data->commandData = BindingCommandData::Stop{};
    if (auto *stop = std::get_if<BindingCommandData::Stop>(&data->commandData))
    {
        stop->optionsMask = chip::BitMask<OptionsBitmap>(strtol(argv[0], &endPtr, 10));
        stop->optionsOverride = chip::BitMask<OptionsBitmap>(strtol(argv[1], &endPtr, 10));
    }
    data->isGroup             = true;

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupsMoveToLevelWithOnOffSwitchCommandHandler(int argc, char ** argv)
{
    if (argc != 4)
    {
        return GroupsLevelControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->commandId           = Clusters::LevelControl::Commands::MoveToLevelWithOnOff::Id;
    data->clusterId           = Clusters::LevelControl::Id;
    data->commandData = BindingCommandData::MoveToLevel{};
    char * endPtr;
    if (auto *moveToLevel = std::get_if<BindingCommandData::MoveToLevel>(&data->commandData))
    {
        moveToLevel->level = static_cast<uint8_t>(strtol(argv[0], &endPtr, 10));
        moveToLevel->transitionTime = DataModel::Nullable<uint16_t>(strtol(argv[1], &endPtr, 10));
        moveToLevel->optionsMask = chip::BitMask<OptionsBitmap>(strtol(argv[2], &endPtr, 10));
        moveToLevel->optionsOverride = chip::BitMask<OptionsBitmap>(strtol(argv[3], &endPtr, 10));
    }
    data->isGroup             = true;

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupsMoveWithOnOffSwitchCommandHandler(int argc, char ** argv)
{
    if (argc != 4)
    {
        return GroupsLevelControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->commandId           = Clusters::LevelControl::Commands::MoveWithOnOff::Id;
    data->clusterId           = Clusters::LevelControl::Id;
    data->commandData = BindingCommandData::Move{};
    char * endPtr;
    if (auto *move = std::get_if<BindingCommandData::Move>(&data->commandData))
    {
        move->moveMode = static_cast<Clusters::LevelControl::MoveModeEnum>(strtol(argv[0], &endPtr, 10));
        move->rate = static_cast<DataModel::Nullable<uint8_t>>(strtol(argv[1], &endPtr, 10));
        move->optionsMask = chip::BitMask<OptionsBitmap>(strtol(argv[2], &endPtr, 10));
        move->optionsOverride = chip::BitMask<OptionsBitmap>(strtol(argv[3], &endPtr, 10));
    }
    data->isGroup             = true;

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupsStepWithOnOffSwitchCommandHandler(int argc, char ** argv)
{
    if (argc != 5)
    {
        return GroupsLevelControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->commandId           = Clusters::LevelControl::Commands::StepWithOnOff::Id;
    data->clusterId           = Clusters::LevelControl::Id;
    char * endPtr;
    data->commandData = BindingCommandData::Step{};
    if (auto *step = std::get_if<BindingCommandData::Step>(&data->commandData))
    {
        step->stepMode = static_cast<Clusters::LevelControl::StepModeEnum>(strtol(argv[0], &endPtr, 10));
        step->stepSize = static_cast<uint8_t>(strtol(argv[1], &endPtr, 10));
        step->transitionTime = DataModel::Nullable<uint16_t>(strtol(argv[2], &endPtr, 10));
        step->optionsMask = chip::BitMask<OptionsBitmap>(strtol(argv[3], &endPtr, 10));
        step->optionsOverride = chip::BitMask<OptionsBitmap>(strtol(argv[4], &endPtr, 10));
    }
    data->isGroup             = true;

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupsStopWithOnOffSwitchCommandHandler(int argc, char ** argv)
{
    if (argc != 2)
    {
        return GroupsLevelControlHelpHandler(argc, argv);
    }

    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->commandId           = Clusters::LevelControl::Commands::StopWithOnOff::Id;
    data->clusterId           = Clusters::LevelControl::Id;
    char * endPtr;
    data->commandData = BindingCommandData::Stop{};
    if (auto *stop = std::get_if<BindingCommandData::Stop>(&data->commandData))
    {
        stop->optionsMask = chip::BitMask<OptionsBitmap>(strtol(argv[0], &endPtr, 10));
        stop->optionsOverride = chip::BitMask<OptionsBitmap>(strtol(argv[1], &endPtr, 10));
    }
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
        { &LevelControlSwitchCommandHandler, "levelcontrol", " Usage: switch levelcontrol <subcommand>" },
        { &GroupsSwitchCommandHandler, "groups", "Usage: switch groups <subcommand>" },
        { &BindingSwitchCommandHandler, "binding", "Usage: switch binding <subcommand>" }
    };

    static const shell_command_t sSwitchOnOffSubCommands[] = {
        { &OnOffHelpHandler, "help", "Usage : switch ononff <subcommand>" },
        { &OnSwitchCommandHandler, "on", "Sends on command to bound lighting app" },
        { &OffSwitchCommandHandler, "off", "Sends off command to bound lighting app" },
        { &ToggleSwitchCommandHandler, "toggle", "Sends toggle command to bound lighting app" }
    };

    static const shell_command_t sSwitchLevelControlSubCommands[] = {
        { &LevelControlHelpHandler, "help", "Usage: switch levelcontrol <subcommand>" },
        { &MoveToLevelSwitchCommandHandler, "move-to-level",
          "Usage: switch levelcontrol move-to-level <level> <transitiontime> <optionsmask> <optionsoverride>" },
        { &MoveSwitchCommandHandler, "move",
          "Usage: switch levelcontrol move <movemode> <rate> <optionsmask> <optionsoverride>" },
        { &StepSwitchCommandHandler, "step",
          "Usage: switch levelcontrol step <stepmode> <stepsize> <transitiontime> <optionsmask> <optionsoverride>" },
        { &StopSwitchCommandHandler, "stop", "step Usage: switch levelcontrol stop <optionsmask> <optionsoverride>" },
        { &MoveToLevelWithOnOffSwitchCommandHandler, "move-to-level-with-on-off",
          "Usage: switch levelcontrol move-with-to-level-with-on-off <level> <transitiontime> <optionsmask> <optionsoverride>" },
        { &MoveWithOnOffSwitchCommandHandler, "move-with-on-off",
          "Usage: switch levelcontrol move-with-on-off <movemode> <rate> <optionsmask> <optionsoverride>" },
        { &StepWithOnOffSwitchCommandHandler, "step-with-on-off",
          "Usage: switch levelcontrol step-with-on-off <stepmode> <stepsize> <transitiontime> <optionsmask> "
          "<optionsoverride>" },
        { &StopWithOnOffSwitchCommandHandler, "stop-with-on-off",
          "Usage: switch levelcontrol stop-with-on-off <optionsmask> <optionsoverride>" },
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

    static const shell_command_t sSwitchGroupsLevelControlSubCommands[] = {
        { &GroupsLevelControlHelpHandler, "help", "Usage: switch groups levelcontrol <subcommand>" },
        { &GroupsMoveToLevelSwitchCommandHandler, "move-to-level",
          "Usage: switch groups levelcontrol move-to-level <level> <transitiontime> <optionsmask> <optionsoverride>" },
        { &GroupsMoveSwitchCommandHandler, "move",
          "Usage: switch groups levelcontrol move <movemode> <rate> <optionsmask> <optionsoverride>" },
        { &GroupsStepSwitchCommandHandler, "step",
          "Usage: switch groups levelcontrol step <stepmode> <stepsize> <transitiontime> <optionsmask> <optionsoverride>" },
        { &GroupsStopSwitchCommandHandler, "stop", "step Usage: switch groups levelcontrol stop <optionsmask> <optionsoverride>" },
        { &GroupsMoveToLevelWithOnOffSwitchCommandHandler, "move-to-level-with-on-off",
          "Usage: switch groups levelcontrol move-with-to-level-with-on-off <level> <transitiontime> <optionsmask> <optionsoverride>" },
        { &GroupsMoveWithOnOffSwitchCommandHandler, "move-with-on-off",
          "Usage: switch groups levelcontrol move-with-on-off <movemode> <rate> <optionsmask> <optionsoverride>" },
        { &GroupsStepWithOnOffSwitchCommandHandler, "step-with-on-off",
          "Usage: switch groups levelcontrol step-with-on-off <stepmode> <stepsize> <transitiontime> <optionsmask> "
          "<optionsoverride>" },
        { &GroupsStopWithOnOffSwitchCommandHandler, "stop-with-on-off",
          "Usage: switch groups levelcontrol stop-with-on-off <optionsmask> <optionsoverride>" },
    };

    static const shell_command_t sSwitchBindingSubCommands[] = {
        { &BindingHelpHandler, "help", "Usage: switch binding <subcommand>" },
        { &BindingGroupBindCommandHandler, "group", "Usage: switch binding group <fabric index> <group id>" },
        { &BindingUnicastBindCommandHandler, "unicast", "Usage: switch binding group <fabric index> <node id> <endpoint>" }
    };

    static const shell_command_t sSwitchCommand = { &SwitchCommandHandler, "switch",
                                                    "Light-switch commands. Usage: switch <subcommand>" };

    sShellSwitchGroupsOnOffSubCommands.RegisterCommands(sSwitchGroupsOnOffSubCommands, ArraySize(sSwitchGroupsOnOffSubCommands));
    sShellSwitchGroupsLevelControlSubCommands.RegisterCommands(sSwitchGroupsLevelControlSubCommands, ArraySize(sSwitchGroupsLevelControlSubCommands));
    sShellSwitchOnOffSubCommands.RegisterCommands(sSwitchOnOffSubCommands, ArraySize(sSwitchOnOffSubCommands));
    sShellSwitchLevelControlSubCommands.RegisterCommands(sSwitchLevelControlSubCommands, ArraySize(sSwitchLevelControlSubCommands));
    sShellSwitchGroupsSubCommands.RegisterCommands(sSwitchGroupsSubCommands, ArraySize(sSwitchGroupsSubCommands));
    sShellSwitchBindingSubCommands.RegisterCommands(sSwitchBindingSubCommands, ArraySize(sSwitchBindingSubCommands));
    sShellSwitchSubCommands.RegisterCommands(sSwitchSubCommands, ArraySize(sSwitchSubCommands));

    Engine::Root().RegisterCommands(&sSwitchCommand, 1);
}

} // namespace LightSwitchCommands

#endif // ENABLE_CHIP_SHELL
