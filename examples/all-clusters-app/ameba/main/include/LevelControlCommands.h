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
#include <app/clusters/bindings/bindings.h>

#include <type_traits>

#if CONFIG_ENABLE_CHIP_SHELL
#include "lib/shell/Engine.h"
#include "lib/shell/commands/Help.h"
#endif // ENABLE_CHIP_SHELL

using namespace chip;
using namespace chip::app;
using chip::app::Clusters::LevelControl::MoveModeEnum;
using chip::app::Clusters::LevelControl::OptionsBitmap;
using chip::app::Clusters::LevelControl::StepModeEnum;

#if CONFIG_ENABLE_CHIP_SHELL
using Shell::Engine;
using Shell::shell_command_t;
using Shell::streamer_get;
using Shell::streamer_printf;

Engine sShellSwitchLevelControlSubCommands;
Engine sShellSwitchLevelControlReadSubCommands;
Engine sShellSwitchGroupsLevelControlSubCommands;
#endif // defined(ENABLE_CHIP_SHELL)

namespace {

template <class T>
T from_underlying(std::underlying_type_t<T> value)
{
    return static_cast<T>(value);
}

} // namespace

void ProcessLevelControlUnicastBindingRead(BindingCommandData * data, const EmberBindingTableEntry & binding,
                                           OperationalDeviceProxy * peer_device)
{
    auto onSuccess = [](const ConcreteDataAttributePath & attributePath, const auto & dataResponse) {
        ChipLogProgress(NotSpecified, "Read LevelControl attribute succeeds");
    };

    auto onFailure = [](const ConcreteDataAttributePath * attributePath, CHIP_ERROR error) {
        ChipLogError(NotSpecified, "Read LevelControl attribute failed: %" CHIP_ERROR_FORMAT, error.Format());
    };

    VerifyOrDie(peer_device != nullptr && peer_device->ConnectionReady());

    switch (data->attributeId)
    {
    case Clusters::LevelControl::Attributes::AttributeList::Id:
        Controller::ReadAttribute<Clusters::LevelControl::Attributes::AttributeList::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case Clusters::LevelControl::Attributes::CurrentLevel::Id:
        Controller::ReadAttribute<Clusters::LevelControl::Attributes::CurrentLevel::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case Clusters::LevelControl::Attributes::RemainingTime::Id:
        Controller::ReadAttribute<Clusters::LevelControl::Attributes::RemainingTime::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case Clusters::LevelControl::Attributes::MinLevel::Id:
        Controller::ReadAttribute<Clusters::LevelControl::Attributes::MinLevel::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case Clusters::LevelControl::Attributes::MaxLevel::Id:
        Controller::ReadAttribute<Clusters::LevelControl::Attributes::MaxLevel::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case Clusters::LevelControl::Attributes::CurrentFrequency::Id:
        Controller::ReadAttribute<Clusters::LevelControl::Attributes::CurrentFrequency::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case Clusters::LevelControl::Attributes::MinFrequency::Id:
        Controller::ReadAttribute<Clusters::LevelControl::Attributes::MinFrequency::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case Clusters::LevelControl::Attributes::MaxFrequency::Id:
        Controller::ReadAttribute<Clusters::LevelControl::Attributes::MaxFrequency::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case Clusters::LevelControl::Attributes::Options::Id:
        Controller::ReadAttribute<Clusters::LevelControl::Attributes::Options::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case Clusters::LevelControl::Attributes::OnOffTransitionTime::Id:
        Controller::ReadAttribute<Clusters::LevelControl::Attributes::OnOffTransitionTime::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case Clusters::LevelControl::Attributes::OnLevel::Id:
        Controller::ReadAttribute<Clusters::LevelControl::Attributes::OnLevel::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case Clusters::LevelControl::Attributes::OnTransitionTime::Id:
        Controller::ReadAttribute<Clusters::LevelControl::Attributes::OnTransitionTime::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case Clusters::LevelControl::Attributes::OffTransitionTime::Id:
        Controller::ReadAttribute<Clusters::LevelControl::Attributes::OffTransitionTime::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case Clusters::LevelControl::Attributes::DefaultMoveRate::Id:
        Controller::ReadAttribute<Clusters::LevelControl::Attributes::DefaultMoveRate::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case Clusters::LevelControl::Attributes::StartUpCurrentLevel::Id:
        Controller::ReadAttribute<Clusters::LevelControl::Attributes::StartUpCurrentLevel::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;
    }
}

void ProcessLevelControlUnicastBindingCommand(BindingCommandData * data, const EmberBindingTableEntry & binding,
                                              OperationalDeviceProxy * peer_device)
{
    auto onSuccess = [](const ConcreteCommandPath & commandPath, const StatusIB & status, const auto & dataResponse) {
        ChipLogProgress(NotSpecified, "LevelControl command succeeds");
    };

    auto onFailure = [](CHIP_ERROR error) {
        ChipLogError(NotSpecified, "LevelControl command failed: %" CHIP_ERROR_FORMAT, error.Format());
    };

    VerifyOrDie(peer_device != nullptr && peer_device->ConnectionReady());

    Clusters::LevelControl::Commands::MoveToLevel::Type moveToLevelCommand;
    Clusters::LevelControl::Commands::Move::Type moveCommand;
    Clusters::LevelControl::Commands::Step::Type stepCommand;
    Clusters::LevelControl::Commands::Stop::Type stopCommand;
    Clusters::LevelControl::Commands::MoveToLevelWithOnOff::Type moveToLevelWithOnOffCommand;
    Clusters::LevelControl::Commands::MoveWithOnOff::Type moveWithOnOffCommand;
    Clusters::LevelControl::Commands::StepWithOnOff::Type stepWithOnOffCommand;
    Clusters::LevelControl::Commands::StopWithOnOff::Type stopWithOnOffCommand;

    switch (data->commandId)
    {
    case Clusters::LevelControl::Commands::MoveToLevel::Id:
        moveToLevelCommand.level           = static_cast<uint8_t>(data->args[0]);
        moveToLevelCommand.transitionTime  = static_cast<DataModel::Nullable<uint16_t>>(data->args[1]);
        moveToLevelCommand.optionsMask     = static_cast<chip::BitMask<OptionsBitmap>>(data->args[2]);
        moveToLevelCommand.optionsOverride = static_cast<chip::BitMask<OptionsBitmap>>(data->args[3]);
        Controller::InvokeCommandRequest(peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote,
                                         moveToLevelCommand, onSuccess, onFailure);
        break;

    case Clusters::LevelControl::Commands::Move::Id:
        moveCommand.moveMode        = from_underlying<MoveModeEnum>(data->args[0]);
        moveCommand.rate            = static_cast<DataModel::Nullable<uint8_t>>(data->args[1]);
        moveCommand.optionsMask     = static_cast<chip::BitMask<OptionsBitmap>>(data->args[2]);
        moveCommand.optionsOverride = static_cast<chip::BitMask<OptionsBitmap>>(data->args[3]);
        Controller::InvokeCommandRequest(peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote,
                                         moveCommand, onSuccess, onFailure);
        break;

    case Clusters::LevelControl::Commands::Step::Id:
        stepCommand.stepMode        = from_underlying<StepModeEnum>(data->args[0]);
        stepCommand.stepSize        = static_cast<uint8_t>(data->args[1]);
        stepCommand.transitionTime  = static_cast<DataModel::Nullable<uint16_t>>(data->args[2]);
        stepCommand.optionsMask     = static_cast<chip::BitMask<OptionsBitmap>>(data->args[3]);
        stepCommand.optionsOverride = static_cast<chip::BitMask<OptionsBitmap>>(data->args[4]);
        Controller::InvokeCommandRequest(peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote,
                                         stepCommand, onSuccess, onFailure);
        break;

    case Clusters::LevelControl::Commands::Stop::Id:
        stopCommand.optionsMask     = static_cast<chip::BitMask<OptionsBitmap>>(data->args[0]);
        stopCommand.optionsOverride = static_cast<chip::BitMask<OptionsBitmap>>(data->args[1]);
        Controller::InvokeCommandRequest(peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote,
                                         stopCommand, onSuccess, onFailure);
        break;

    case Clusters::LevelControl::Commands::MoveToLevelWithOnOff::Id:
        moveToLevelWithOnOffCommand.level           = static_cast<uint8_t>(data->args[0]);
        moveToLevelWithOnOffCommand.transitionTime  = static_cast<DataModel::Nullable<uint16_t>>(data->args[1]);
        moveToLevelWithOnOffCommand.optionsMask     = static_cast<chip::BitMask<OptionsBitmap>>(data->args[2]);
        moveToLevelWithOnOffCommand.optionsOverride = static_cast<chip::BitMask<OptionsBitmap>>(data->args[3]);
        Controller::InvokeCommandRequest(peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote,
                                         moveToLevelWithOnOffCommand, onSuccess, onFailure);
        break;

    case Clusters::LevelControl::Commands::MoveWithOnOff::Id:
        moveWithOnOffCommand.moveMode        = from_underlying<MoveModeEnum>(data->args[0]);
        moveWithOnOffCommand.rate            = static_cast<DataModel::Nullable<uint8_t>>(data->args[1]);
        moveWithOnOffCommand.optionsMask     = static_cast<chip::BitMask<OptionsBitmap>>(data->args[2]);
        moveWithOnOffCommand.optionsOverride = static_cast<chip::BitMask<OptionsBitmap>>(data->args[3]);
        Controller::InvokeCommandRequest(peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote,
                                         moveWithOnOffCommand, onSuccess, onFailure);
        break;

    case Clusters::LevelControl::Commands::StepWithOnOff::Id:
        stepWithOnOffCommand.stepMode        = from_underlying<StepModeEnum>(data->args[0]);
        stepWithOnOffCommand.stepSize        = static_cast<uint8_t>(data->args[1]);
        stepWithOnOffCommand.transitionTime  = static_cast<DataModel::Nullable<uint16_t>>(data->args[2]);
        stepWithOnOffCommand.optionsMask     = static_cast<chip::BitMask<OptionsBitmap>>(data->args[3]);
        stepWithOnOffCommand.optionsOverride = static_cast<chip::BitMask<OptionsBitmap>>(data->args[4]);
        Controller::InvokeCommandRequest(peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote,
                                         stepWithOnOffCommand, onSuccess, onFailure);
        break;

    case Clusters::LevelControl::Commands::StopWithOnOff::Id:
        stopWithOnOffCommand.optionsMask     = static_cast<chip::BitMask<OptionsBitmap>>(data->args[0]);
        stopWithOnOffCommand.optionsOverride = static_cast<chip::BitMask<OptionsBitmap>>(data->args[1]);
        Controller::InvokeCommandRequest(peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote,
                                         stopWithOnOffCommand, onSuccess, onFailure);
        break;
    }
}

void ProcessLevelControlGroupBindingCommand(BindingCommandData * data, const EmberBindingTableEntry & binding)
{
    Messaging::ExchangeManager & exchangeMgr = Server::GetInstance().GetExchangeManager();

    Clusters::LevelControl::Commands::MoveToLevel::Type moveToLevelCommand;
    Clusters::LevelControl::Commands::Move::Type moveCommand;
    Clusters::LevelControl::Commands::Step::Type stepCommand;
    Clusters::LevelControl::Commands::Stop::Type stopCommand;
    Clusters::LevelControl::Commands::MoveToLevelWithOnOff::Type moveToLevelWithOnOffCommand;
    Clusters::LevelControl::Commands::MoveWithOnOff::Type moveWithOnOffCommand;
    Clusters::LevelControl::Commands::StepWithOnOff::Type stepWithOnOffCommand;
    Clusters::LevelControl::Commands::StopWithOnOff::Type stopWithOnOffCommand;

    switch (data->commandId)
    {
    case Clusters::LevelControl::Commands::MoveToLevel::Id:
        moveToLevelCommand.level           = static_cast<uint8_t>(data->args[0]);
        moveToLevelCommand.transitionTime  = static_cast<DataModel::Nullable<uint16_t>>(data->args[1]);
        moveToLevelCommand.optionsMask     = static_cast<chip::BitMask<OptionsBitmap>>(data->args[2]);
        moveToLevelCommand.optionsOverride = static_cast<chip::BitMask<OptionsBitmap>>(data->args[3]);
        Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId, moveToLevelCommand);
        break;

    case Clusters::LevelControl::Commands::Move::Id:
        moveCommand.moveMode        = from_underlying<MoveModeEnum>(data->args[0]);
        moveCommand.rate            = static_cast<DataModel::Nullable<uint8_t>>(data->args[1]);
        moveCommand.optionsMask     = static_cast<chip::BitMask<OptionsBitmap>>(data->args[2]);
        moveCommand.optionsOverride = static_cast<chip::BitMask<OptionsBitmap>>(data->args[3]);
        Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId, moveCommand);
        break;

    case Clusters::LevelControl::Commands::Step::Id:
        stepCommand.stepMode        = from_underlying<StepModeEnum>(data->args[0]);
        stepCommand.stepSize        = static_cast<uint8_t>(data->args[1]);
        stepCommand.transitionTime  = static_cast<DataModel::Nullable<uint16_t>>(data->args[2]);
        stepCommand.optionsMask     = static_cast<chip::BitMask<OptionsBitmap>>(data->args[3]);
        stepCommand.optionsOverride = static_cast<chip::BitMask<OptionsBitmap>>(data->args[4]);
        Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId, stepCommand);
        break;

    case Clusters::LevelControl::Commands::Stop::Id:
        stopCommand.optionsMask     = static_cast<chip::BitMask<OptionsBitmap>>(data->args[0]);
        stopCommand.optionsOverride = static_cast<chip::BitMask<OptionsBitmap>>(data->args[1]);
        Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId, stopCommand);
        break;

    case Clusters::LevelControl::Commands::MoveToLevelWithOnOff::Id:
        moveToLevelWithOnOffCommand.level           = static_cast<uint8_t>(data->args[0]);
        moveToLevelWithOnOffCommand.transitionTime  = static_cast<DataModel::Nullable<uint16_t>>(data->args[1]);
        moveToLevelWithOnOffCommand.optionsMask     = static_cast<chip::BitMask<OptionsBitmap>>(data->args[2]);
        moveToLevelWithOnOffCommand.optionsOverride = static_cast<chip::BitMask<OptionsBitmap>>(data->args[3]);
        Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId, moveToLevelWithOnOffCommand);
        break;

    case Clusters::LevelControl::Commands::MoveWithOnOff::Id:
        moveWithOnOffCommand.moveMode        = from_underlying<MoveModeEnum>(data->args[0]);
        moveWithOnOffCommand.rate            = static_cast<DataModel::Nullable<uint8_t>>(data->args[1]);
        moveWithOnOffCommand.optionsMask     = static_cast<chip::BitMask<OptionsBitmap>>(data->args[2]);
        moveWithOnOffCommand.optionsOverride = static_cast<chip::BitMask<OptionsBitmap>>(data->args[3]);
        Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId, moveWithOnOffCommand);
        break;

    case Clusters::LevelControl::Commands::StepWithOnOff::Id:
        stepWithOnOffCommand.stepMode        = from_underlying<StepModeEnum>(data->args[0]);
        stepWithOnOffCommand.stepSize        = static_cast<uint8_t>(data->args[1]);
        stepWithOnOffCommand.transitionTime  = static_cast<DataModel::Nullable<uint16_t>>(data->args[2]);
        stepWithOnOffCommand.optionsMask     = static_cast<chip::BitMask<OptionsBitmap>>(data->args[3]);
        stepWithOnOffCommand.optionsOverride = static_cast<chip::BitMask<OptionsBitmap>>(data->args[4]);
        Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId, stepWithOnOffCommand);
        break;

    case Clusters::LevelControl::Commands::StopWithOnOff::Id:
        stopWithOnOffCommand.optionsMask     = static_cast<chip::BitMask<OptionsBitmap>>(data->args[0]);
        stopWithOnOffCommand.optionsOverride = static_cast<chip::BitMask<OptionsBitmap>>(data->args[1]);
        Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId, stopWithOnOffCommand);
        break;
    }
}

#if CONFIG_ENABLE_CHIP_SHELL
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
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->args[3]             = atoi(argv[3]);

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
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->args[3]             = atoi(argv[3]);

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
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->args[3]             = atoi(argv[3]);
    data->args[4]             = atoi(argv[4]);

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
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);

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
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->args[3]             = atoi(argv[3]);

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
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->args[3]             = atoi(argv[3]);

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
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->args[3]             = atoi(argv[3]);
    data->args[4]             = atoi(argv[4]);

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
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

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
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->args[3]             = atoi(argv[3]);
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
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->args[3]             = atoi(argv[3]);
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
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->args[3]             = atoi(argv[3]);
    data->args[4]             = atoi(argv[4]);
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
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
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
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->args[3]             = atoi(argv[3]);
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
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->args[3]             = atoi(argv[3]);
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
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->args[3]             = atoi(argv[3]);
    data->args[4]             = atoi(argv[4]);
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
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->isGroup             = true;

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}
#endif // CONFIG_ENABLE_CHIP_SHELL
