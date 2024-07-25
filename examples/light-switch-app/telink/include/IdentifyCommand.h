/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#if defined(CONFIG_CHIP_LIB_SHELL)
#include "lib/shell/Engine.h"
#include "lib/shell/commands/Help.h"
#endif // CONFIG_CHIP_LIB_SHELL

using namespace chip;
using namespace chip::app;

#if defined(CONFIG_CHIP_LIB_SHELL)
using Shell::Engine;
using Shell::shell_command_t;
using Shell::streamer_get;
using Shell::streamer_printf;

Engine sShellSwitchIdentifySubCommands;
Engine sShellSwitchIdentifyReadSubCommands;
Engine sShellSwitchGroupsIdentifySubCommands;
#endif // CONFIG_CHIP_LIB_SHELL

void ProcessIdentifyUnicastBindingRead(BindingCommandData * data, const EmberBindingTableEntry & binding,
                                       OperationalDeviceProxy * peer_device)
{
    auto onSuccess = [](const ConcreteDataAttributePath & attributePath, const auto & dataResponse) {
        ChipLogProgress(NotSpecified, "Read Identify attribute succeeds");
    };

    auto onFailure = [](const ConcreteDataAttributePath * attributePath, CHIP_ERROR error) {
        ChipLogError(NotSpecified, "Read Identify attribute failed: %" CHIP_ERROR_FORMAT, error.Format());
    };

    VerifyOrDie(peer_device != nullptr && peer_device->ConnectionReady());

    switch (data->attributeId)
    {
    case Clusters::Identify::Attributes::AttributeList::Id:
        Controller::ReadAttribute<Clusters::Identify::Attributes::AttributeList::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case Clusters::Identify::Attributes::IdentifyTime::Id:
        Controller::ReadAttribute<Clusters::Identify::Attributes::IdentifyTime::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case Clusters::Identify::Attributes::IdentifyType::Id:
        Controller::ReadAttribute<Clusters::Identify::Attributes::IdentifyType::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;
    }
}

void ProcessIdentifyUnicastBindingCommand(BindingCommandData * data, const EmberBindingTableEntry & binding,
                                          OperationalDeviceProxy * peer_device)
{
    auto onSuccess = [](const ConcreteCommandPath & commandPath, const StatusIB & status, const auto & dataResponse) {
        ChipLogProgress(NotSpecified, "Identify command succeeds");
    };

    auto onFailure = [](CHIP_ERROR error) {
        ChipLogError(NotSpecified, "Identify command failed: %" CHIP_ERROR_FORMAT, error.Format());
    };

    VerifyOrDie(peer_device != nullptr && peer_device->ConnectionReady());

    Clusters::Identify::Commands::Identify::Type identifyCommand;
    Clusters::Identify::Commands::TriggerEffect::Type triggerEffectCommand;

    switch (data->commandId)
    {
    case Clusters::Identify::Commands::Identify::Id:
        identifyCommand.identifyTime = static_cast<uint16_t>(data->args[0]);
        Controller::InvokeCommandRequest(peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote,
                                         identifyCommand, onSuccess, onFailure);
        break;

    case Clusters::Identify::Commands::TriggerEffect::Id:
        triggerEffectCommand.effectIdentifier = static_cast<Clusters::Identify::EffectIdentifierEnum>(data->args[0]);
        triggerEffectCommand.effectVariant    = static_cast<Clusters::Identify::EffectVariantEnum>(data->args[1]);
        Controller::InvokeCommandRequest(peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote,
                                         triggerEffectCommand, onSuccess, onFailure);
        break;
    }
}

void ProcessIdentifyGroupBindingCommand(BindingCommandData * data, const EmberBindingTableEntry & binding)
{
    Messaging::ExchangeManager & exchangeMgr = Server::GetInstance().GetExchangeManager();

    Clusters::Identify::Commands::Identify::Type identifyCommand;
    Clusters::Identify::Commands::TriggerEffect::Type triggerEffectCommand;

    switch (data->commandId)
    {
    case Clusters::Identify::Commands::Identify::Id:
        identifyCommand.identifyTime = static_cast<uint16_t>(data->args[0]);
        Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId, identifyCommand);
        break;

    case Clusters::Identify::Commands::TriggerEffect::Id:
        triggerEffectCommand.effectIdentifier = static_cast<Clusters::Identify::EffectIdentifierEnum>(data->args[0]);
        triggerEffectCommand.effectVariant    = static_cast<Clusters::Identify::EffectVariantEnum>(data->args[1]);
        Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId, triggerEffectCommand);
        break;
    }
}

#if defined(CONFIG_CHIP_LIB_SHELL)
/********************************************************
 * Identify switch shell functions
 *********************************************************/

CHIP_ERROR IdentifyHelpHandler(int argc, char ** argv)
{
    sShellSwitchIdentifySubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR IdentifySwitchCommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return IdentifyHelpHandler(argc, argv);
    }

    return sShellSwitchIdentifySubCommands.ExecCommand(argc, argv);
}

CHIP_ERROR IdentifyCommandHandler(int argc, char ** argv)
{
    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->commandId           = Clusters::Identify::Commands::Identify::Id;
    data->clusterId           = Clusters::Identify::Id;
    data->args[0]             = atoi(argv[0]);

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR TriggerEffectSwitchCommandHandler(int argc, char ** argv)
{
    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->commandId           = Clusters::Identify::Commands::TriggerEffect::Id;
    data->clusterId           = Clusters::Identify::Id;
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

/********************************************************
 * Identify Read switch shell functions
 *********************************************************/

CHIP_ERROR IdentifyReadHelpHandler(int argc, char ** argv)
{
    sShellSwitchIdentifyReadSubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR IdentifyRead(int argc, char ** argv)
{
    if (argc == 0)
    {
        return IdentifyReadHelpHandler(argc, argv);
    }

    return sShellSwitchIdentifyReadSubCommands.ExecCommand(argc, argv);
}

CHIP_ERROR IdentifyReadAttributeList(int argc, char ** argv)
{
    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->attributeId         = Clusters::Identify::Attributes::AttributeList::Id;
    data->clusterId           = Clusters::Identify::Id;
    data->isReadAttribute     = true;

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR IdentifyReadIdentifyTime(int argc, char ** argv)
{
    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->attributeId         = Clusters::Identify::Attributes::IdentifyTime::Id;
    data->clusterId           = Clusters::Identify::Id;
    data->isReadAttribute     = true;

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR IdentifyReadIdentifyType(int argc, char ** argv)
{
    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->attributeId         = Clusters::Identify::Attributes::IdentifyType::Id;
    data->clusterId           = Clusters::Identify::Id;
    data->isReadAttribute     = true;

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

/********************************************************
 * Groups Identify switch shell functions
 *********************************************************/

CHIP_ERROR GroupsIdentifyHelpHandler(int argc, char ** argv)
{
    sShellSwitchGroupsIdentifySubCommands.ForEachCommand(Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupsIdentifySwitchCommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return GroupsIdentifyHelpHandler(argc, argv);
    }

    return sShellSwitchGroupsIdentifySubCommands.ExecCommand(argc, argv);
}

CHIP_ERROR GroupIdentifyCommandHandler(int argc, char ** argv)
{
    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->commandId           = Clusters::Identify::Commands::Identify::Id;
    data->clusterId           = Clusters::Identify::Id;
    data->args[0]             = atoi(argv[0]);
    data->isGroup             = true;

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupTriggerEffectSwitchCommandHandler(int argc, char ** argv)
{
    BindingCommandData * data = Platform::New<BindingCommandData>();
    data->commandId           = Clusters::Identify::Commands::TriggerEffect::Id;
    data->clusterId           = Clusters::Identify::Id;
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);
    data->isGroup             = true;

    DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}
#endif // CONFIG_CHIP_LIB_SHELL
