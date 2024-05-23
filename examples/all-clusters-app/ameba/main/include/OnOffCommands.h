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

Engine sShellSwitchOnOffSubCommands;
Engine sShellSwitchOnOffReadSubCommands;
Engine sShellSwitchGroupsOnOffSubCommands;
#endif // defined(ENABLE_CHIP_SHELL)

void ProcessOnOffUnicastBindingRead(BindingCommandData * data, const EmberBindingTableEntry & binding,
                                    OperationalDeviceProxy * peer_device)
{
    auto onSuccess = [](const ConcreteDataAttributePath & attributePath, const auto & dataResponse) {
        ChipLogProgress(NotSpecified, "Read OnOff attribute succeeds");
    };

    auto onFailure = [](const ConcreteDataAttributePath * attributePath, CHIP_ERROR error) {
        ChipLogError(NotSpecified, "Read OnOff attribute failed: %" CHIP_ERROR_FORMAT, error.Format());
    };

    VerifyOrDie(peer_device != nullptr && peer_device->ConnectionReady());

    switch (data->attributeId)
    {
    case Clusters::OnOff::Attributes::AttributeList::Id:
        Controller::ReadAttribute<Clusters::OnOff::Attributes::AttributeList::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case Clusters::OnOff::Attributes::OnOff::Id:
        Controller::ReadAttribute<Clusters::OnOff::Attributes::OnOff::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case Clusters::OnOff::Attributes::GlobalSceneControl::Id:
        Controller::ReadAttribute<Clusters::OnOff::Attributes::GlobalSceneControl::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case Clusters::OnOff::Attributes::OnTime::Id:
        Controller::ReadAttribute<Clusters::OnOff::Attributes::OnTime::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case Clusters::OnOff::Attributes::OffWaitTime::Id:
        Controller::ReadAttribute<Clusters::OnOff::Attributes::OffWaitTime::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case Clusters::OnOff::Attributes::StartUpOnOff::Id:
        Controller::ReadAttribute<Clusters::OnOff::Attributes::StartUpOnOff::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;
    }
}

void ProcessOnOffUnicastBindingCommand(BindingCommandData * data, const EmberBindingTableEntry & binding,
                                       OperationalDeviceProxy * peer_device)
{
    auto onSuccess = [](const ConcreteCommandPath & commandPath, const StatusIB & status, const auto & dataResponse) {
        ChipLogProgress(NotSpecified, "OnOff command succeeds");
    };

    auto onFailure = [](CHIP_ERROR error) {
        ChipLogError(NotSpecified, "OnOff command failed: %" CHIP_ERROR_FORMAT, error.Format());
    };

    VerifyOrDie(peer_device != nullptr && peer_device->ConnectionReady());

    Clusters::OnOff::Commands::Toggle::Type toggleCommand;
    Clusters::OnOff::Commands::On::Type onCommand;
    Clusters::OnOff::Commands::Off::Type offCommand;
    Clusters::OnOff::Commands::OffWithEffect::Type offwitheffectCommand;
    Clusters::OnOff::Commands::OnWithRecallGlobalScene::Type onwithrecallglobalsceneCommand;
    Clusters::OnOff::Commands::OnWithTimedOff::Type onwithtimedoffCommand;

    switch (data->commandId)
    {
    case Clusters::OnOff::Commands::Toggle::Id:
        Controller::InvokeCommandRequest(peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote,
                                         toggleCommand, onSuccess, onFailure);
        break;

    case Clusters::OnOff::Commands::On::Id:
        Controller::InvokeCommandRequest(peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote,
                                         onCommand, onSuccess, onFailure);
        break;

    case Clusters::OnOff::Commands::Off::Id:
        Controller::InvokeCommandRequest(peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote,
                                         offCommand, onSuccess, onFailure);
        break;

    case Clusters::OnOff::Commands::OffWithEffect::Id:
        offwitheffectCommand.effectIdentifier = static_cast<Clusters::OnOff::EffectIdentifierEnum>(data->args[0]);
        offwitheffectCommand.effectVariant    = static_cast<uint8_t>(data->args[1]);
        Controller::InvokeCommandRequest(peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote,
                                         offwitheffectCommand, onSuccess, onFailure);
        break;

    case Clusters::OnOff::Commands::OnWithRecallGlobalScene::Id:
        Controller::InvokeCommandRequest(peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote,
                                         onwithrecallglobalsceneCommand, onSuccess, onFailure);
        break;

    case Clusters::OnOff::Commands::OnWithTimedOff::Id:
        onwithtimedoffCommand.onOffControl = static_cast<chip::BitMask<Clusters::OnOff::OnOffControlBitmap>>(data->args[0]);
        onwithtimedoffCommand.onTime       = static_cast<uint16_t>(data->args[1]);
        onwithtimedoffCommand.offWaitTime  = static_cast<uint16_t>(data->args[2]);
        Controller::InvokeCommandRequest(peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote,
                                         onwithtimedoffCommand, onSuccess, onFailure);
        break;
    }
}

void ProcessOnOffGroupBindingCommand(BindingCommandData * data, const EmberBindingTableEntry & binding)
{
    Messaging::ExchangeManager & exchangeMgr = Server::GetInstance().GetExchangeManager();

    Clusters::OnOff::Commands::Toggle::Type toggleCommand;
    Clusters::OnOff::Commands::On::Type onCommand;
    Clusters::OnOff::Commands::Off::Type offCommand;
    Clusters::OnOff::Commands::OffWithEffect::Type offwitheffectCommand;
    Clusters::OnOff::Commands::OnWithRecallGlobalScene::Type onwithrecallglobalsceneCommand;
    Clusters::OnOff::Commands::OnWithTimedOff::Type onwithtimedoffCommand;

    switch (data->commandId)
    {
    case Clusters::OnOff::Commands::Toggle::Id:
        Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId, toggleCommand);
        break;

    case Clusters::OnOff::Commands::On::Id:
        Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId, onCommand);
        break;

    case Clusters::OnOff::Commands::Off::Id:
        Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId, offCommand);
        break;

    case Clusters::OnOff::Commands::OffWithEffect::Id:
        offwitheffectCommand.effectIdentifier = static_cast<Clusters::OnOff::EffectIdentifierEnum>(data->args[0]);
        offwitheffectCommand.effectVariant    = static_cast<uint8_t>(data->args[1]);
        Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId, offwitheffectCommand);
        break;

    case Clusters::OnOff::Commands::OnWithRecallGlobalScene::Id:
        Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId, onwithrecallglobalsceneCommand);
        break;

    case Clusters::OnOff::Commands::OnWithTimedOff::Id:
        onwithtimedoffCommand.onOffControl = static_cast<chip::BitMask<Clusters::OnOff::OnOffControlBitmap>>(data->args[0]);
        onwithtimedoffCommand.onTime       = static_cast<uint16_t>(data->args[1]);
        onwithtimedoffCommand.offWaitTime  = static_cast<uint16_t>(data->args[2]);
        Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId, onwithtimedoffCommand);
        break;
    }
}

#if CONFIG_ENABLE_CHIP_SHELL
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

CHIP_ERROR OffWithEffectSwitchCommandHandler(int argc, char ** argv)
{
    if (argc != 2)
    {
        return OnOffHelpHandler(argc, argv);
    }

    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->commandId           = Clusters::OnOff::Commands::OffWithEffect::Id;
    data->clusterId           = Clusters::OnOff::Id;
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OnWithRecallGlobalSceneSwitchCommandHandler(int argc, char ** argv)
{
    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->commandId           = Clusters::OnOff::Commands::OnWithRecallGlobalScene::Id;
    data->clusterId           = Clusters::OnOff::Id;

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OnWithTimedOffSwitchCommandHandler(int argc, char ** argv)
{
    if (argc != 3)
    {
        return OnOffHelpHandler(argc, argv);
    }

    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->commandId           = Clusters::OnOff::Commands::OnWithTimedOff::Id;
    data->clusterId           = Clusters::OnOff::Id;
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

/********************************************************
 * OnOff Read switch shell functions
 *********************************************************/

CHIP_ERROR OnOffReadHelpHandler(int argc, char ** argv)
{
    sShellSwitchOnOffReadSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR OnOffRead(int argc, char ** argv)
{
    if (argc == 0)
    {
        return OnOffReadHelpHandler(argc, argv);
    }

    return sShellSwitchOnOffReadSubCommands.ExecCommand(argc, argv);
}

CHIP_ERROR OnOffReadAttributeList(int argc, char ** argv)
{
    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->attributeId         = Clusters::OnOff::Attributes::AttributeList::Id;
    data->clusterId           = Clusters::OnOff::Id;
    data->isReadAttribute     = true;

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OnOffReadOnOff(int argc, char ** argv)
{
    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->attributeId         = Clusters::OnOff::Attributes::OnOff::Id;
    data->clusterId           = Clusters::OnOff::Id;
    data->isReadAttribute     = true;

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OnOffReadGlobalSceneControl(int argc, char ** argv)
{
    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->attributeId         = Clusters::OnOff::Attributes::GlobalSceneControl::Id;
    data->clusterId           = Clusters::OnOff::Id;
    data->isReadAttribute     = true;

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OnOffReadOnTime(int argc, char ** argv)
{
    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->attributeId         = Clusters::OnOff::Attributes::OnTime::Id;
    data->clusterId           = Clusters::OnOff::Id;
    data->isReadAttribute     = true;

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OnOffReadOffWaitTime(int argc, char ** argv)
{
    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->attributeId         = Clusters::OnOff::Attributes::OffWaitTime::Id;
    data->clusterId           = Clusters::OnOff::Id;
    data->isReadAttribute     = true;

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OnOffReadStartUpOnOff(int argc, char ** argv)
{
    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->attributeId         = Clusters::OnOff::Attributes::StartUpOnOff::Id;
    data->clusterId           = Clusters::OnOff::Id;
    data->isReadAttribute     = true;

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
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

CHIP_ERROR GroupOffWithEffectSwitchCommandHandler(int argc, char ** argv)
{
    if (argc != 2)
    {
        return GroupsOnOffHelpHandler(argc, argv);
    }

    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->commandId           = Clusters::OnOff::Commands::OffWithEffect::Id;
    data->clusterId           = Clusters::OnOff::Id;
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->isGroup             = true;

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupOnWithRecallGlobalSceneSwitchCommandHandler(int argc, char ** argv)
{
    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->commandId           = Clusters::OnOff::Commands::OnWithRecallGlobalScene::Id;
    data->clusterId           = Clusters::OnOff::Id;
    data->isGroup             = true;

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupOnWithTimedOffSwitchCommandHandler(int argc, char ** argv)
{
    if (argc != 2)
    {
        return GroupsOnOffHelpHandler(argc, argv);
    }

    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->commandId           = Clusters::OnOff::Commands::OnWithTimedOff::Id;
    data->clusterId           = Clusters::OnOff::Id;
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->args[2]             = atoi(argv[2]);
    data->isGroup             = true;

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}
#endif // CONFIG_ENABLE_CHIP_SHELL
