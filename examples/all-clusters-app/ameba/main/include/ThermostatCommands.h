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
#include <app/clusters/binding-server/binding-cluster.h>

#if CONFIG_ENABLE_CHIP_SHELL
#include "lib/shell/Engine.h"
#include "lib/shell/commands/Help.h"
#endif // ENABLE_CHIP_SHELL

using chip::OperationalDeviceProxy;
using chip::app::ConcreteCommandPath;
using chip::app::ConcreteDataAttributePath;
using chip::app::StatusIB;
using chip::app::Clusters::BindingTableEntry;

#if CONFIG_ENABLE_CHIP_SHELL
using chip::Shell::Engine;
using chip::Shell::shell_command_t;
using chip::Shell::streamer_get;
using chip::Shell::streamer_printf;

Engine sShellSwitchThermostatSubCommands;
Engine sShellSwitchThermostatReadSubCommands;
Engine sShellSwitchGroupsThermostatSubCommands;
#endif // defined(ENABLE_CHIP_SHELL)

void ProcessThermostatUnicastBindingRead(BindingCommandData * data, const BindingTableEntry & binding,
                                         OperationalDeviceProxy * peer_device)
{
    auto onSuccess = [](const ConcreteDataAttributePath & attributePath, const auto & dataResponse) {
        ChipLogProgress(NotSpecified, "Read Thermostat attribute succeeds");
    };

    auto onFailure = [](const ConcreteDataAttributePath * attributePath, CHIP_ERROR error) {
        ChipLogError(NotSpecified, "Read Thermostat attribute failed: %" CHIP_ERROR_FORMAT, error.Format());
    };

    VerifyOrDie(peer_device != nullptr && peer_device->ConnectionReady());

    switch (data->attributeId)
    {
    case chip::app::Clusters::Thermostat::Attributes::AttributeList::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::Thermostat::Attributes::AttributeList::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::Thermostat::Attributes::LocalTemperature::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::Thermostat::Attributes::LocalTemperature::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::Thermostat::Attributes::AbsMinHeatSetpointLimit::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::Thermostat::Attributes::AbsMinHeatSetpointLimit::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::Thermostat::Attributes::AbsMaxHeatSetpointLimit::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::Thermostat::Attributes::AbsMaxHeatSetpointLimit::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::Thermostat::Attributes::AbsMinCoolSetpointLimit::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::Thermostat::Attributes::AbsMinCoolSetpointLimit::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::Thermostat::Attributes::AbsMaxCoolSetpointLimit::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::Thermostat::Attributes::AbsMaxCoolSetpointLimit::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::Thermostat::Attributes::PICoolingDemand::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::Thermostat::Attributes::PICoolingDemand::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::Thermostat::Attributes::PIHeatingDemand::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::Thermostat::Attributes::PIHeatingDemand::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::Thermostat::Attributes::OccupiedCoolingSetpoint::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::Thermostat::Attributes::OccupiedCoolingSetpoint::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::Thermostat::Attributes::OccupiedHeatingSetpoint::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::Thermostat::Attributes::OccupiedHeatingSetpoint::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::Thermostat::Attributes::MinHeatSetpointLimit::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::Thermostat::Attributes::MinHeatSetpointLimit::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::Thermostat::Attributes::MaxHeatSetpointLimit::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::Thermostat::Attributes::MaxHeatSetpointLimit::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::Thermostat::Attributes::MinCoolSetpointLimit::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::Thermostat::Attributes::MinCoolSetpointLimit::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::Thermostat::Attributes::MaxCoolSetpointLimit::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::Thermostat::Attributes::MaxCoolSetpointLimit::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::Thermostat::Attributes::ControlSequenceOfOperation::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::Thermostat::Attributes::ControlSequenceOfOperation::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;

    case chip::app::Clusters::Thermostat::Attributes::SystemMode::Id:
        chip::Controller::ReadAttribute<chip::app::Clusters::Thermostat::Attributes::SystemMode::TypeInfo>(
            peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(), binding.remote, onSuccess, onFailure);
        break;
    }
}

void ProcessThermostatUnicastBindingCommand(BindingCommandData * data, const BindingTableEntry & binding,
                                            OperationalDeviceProxy * peer_device)
{
    auto onSuccess = [](const ConcreteCommandPath & commandPath, const StatusIB & status, const auto & dataResponse) {
        ChipLogProgress(NotSpecified, "Thermostat command succeeds");
    };

    auto onFailure = [](CHIP_ERROR error) {
        ChipLogError(NotSpecified, "Thermostat command failed: %" CHIP_ERROR_FORMAT, error.Format());
    };

    VerifyOrDie(peer_device != nullptr && peer_device->ConnectionReady());

    chip::app::Clusters::Thermostat::Commands::SetpointRaiseLower::Type setpointRaiseLowerCommand;

    switch (data->commandId)
    {
    case chip::app::Clusters::Thermostat::Commands::SetpointRaiseLower::Id:
        setpointRaiseLowerCommand.mode   = static_cast<chip::app::Clusters::Thermostat::SetpointRaiseLowerModeEnum>(data->args[0]);
        setpointRaiseLowerCommand.amount = static_cast<int8_t>(data->args[1]);
        chip::Controller::InvokeCommandRequest(peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(),
                                               binding.remote, setpointRaiseLowerCommand, onSuccess, onFailure);
        break;
    }
}

void ProcessThermostatGroupBindingCommand(BindingCommandData * data, const BindingTableEntry & binding)
{
    chip::Messaging::ExchangeManager & exchangeMgr = chip::Server::GetInstance().GetExchangeManager();

    chip::app::Clusters::Thermostat::Commands::SetpointRaiseLower::Type setpointRaiseLowerCommand;

    switch (data->commandId)
    {
    case chip::app::Clusters::Thermostat::Commands::SetpointRaiseLower::Id:
        setpointRaiseLowerCommand.mode   = static_cast<chip::app::Clusters::Thermostat::SetpointRaiseLowerModeEnum>(data->args[0]);
        setpointRaiseLowerCommand.amount = static_cast<int8_t>(data->args[1]);
        chip::Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId, setpointRaiseLowerCommand);
        break;
    }
}

#if CONFIG_ENABLE_CHIP_SHELL
/********************************************************
 * Thermostat switch shell functions
 *********************************************************/

CHIP_ERROR ThermostatHelpHandler(int argc, char ** argv)
{
    sShellSwitchThermostatSubCommands.ForEachCommand(chip::Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ThermostatSwitchCommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return ThermostatHelpHandler(argc, argv);
    }

    return sShellSwitchThermostatSubCommands.ExecCommand(argc, argv);
}

CHIP_ERROR SetpointRaiseLowerSwitchCommandHandler(int argc, char ** argv)
{
    if (argc != 2)
    {
        return ThermostatHelpHandler(argc, argv);
    }

    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->commandId           = chip::app::Clusters::Thermostat::Commands::SetpointRaiseLower::Id;
    data->clusterId           = chip::app::Clusters::Thermostat::Id;
    data->args[0]             = atoi(argv[0]);
    data->args[1]             = atoi(argv[1]);

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

/********************************************************
 * Thermostat Read switch shell functions
 *********************************************************/

CHIP_ERROR ThermostatReadHelpHandler(int argc, char ** argv)
{
    sShellSwitchThermostatReadSubCommands.ForEachCommand(chip::Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ThermostatRead(int argc, char ** argv)
{
    if (argc == 0)
    {
        return ThermostatReadHelpHandler(argc, argv);
    }

    return sShellSwitchThermostatReadSubCommands.ExecCommand(argc, argv);
}

CHIP_ERROR ThermostatReadAttributeList(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::Thermostat::Attributes::AttributeList::Id;
    data->clusterId           = chip::app::Clusters::Thermostat::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ThermostatReadLocalTemperature(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::Thermostat::Attributes::LocalTemperature::Id;
    data->clusterId           = chip::app::Clusters::Thermostat::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ThermostatReadAbsMinHeatSetpointLimit(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::Thermostat::Attributes::AbsMinHeatSetpointLimit::Id;
    data->clusterId           = chip::app::Clusters::Thermostat::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ThermostatReadAbsMaxHeatSetpointLimit(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::Thermostat::Attributes::AbsMaxHeatSetpointLimit::Id;
    data->clusterId           = chip::app::Clusters::Thermostat::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ThermostatReadAbsMinCoolSetpointLimit(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::Thermostat::Attributes::AbsMinCoolSetpointLimit::Id;
    data->clusterId           = chip::app::Clusters::Thermostat::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ThermostatReadAbsMaxCoolSetpointLimit(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::Thermostat::Attributes::AbsMaxCoolSetpointLimit::Id;
    data->clusterId           = chip::app::Clusters::Thermostat::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ThermostatReadPiCoolingDemand(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::Thermostat::Attributes::PICoolingDemand::Id;
    data->clusterId           = chip::app::Clusters::Thermostat::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ThermostatReadPiHeatingDemand(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::Thermostat::Attributes::PIHeatingDemand::Id;
    data->clusterId           = chip::app::Clusters::Thermostat::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ThermostatReadOccupiedCoolingSetpoint(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::Thermostat::Attributes::OccupiedCoolingSetpoint::Id;
    data->clusterId           = chip::app::Clusters::Thermostat::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ThermostatReadOccupiedHeatingSetpoint(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::Thermostat::Attributes::OccupiedHeatingSetpoint::Id;
    data->clusterId           = chip::app::Clusters::Thermostat::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ThermostatReadMinHeatSetpointLimit(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::Thermostat::Attributes::MinHeatSetpointLimit::Id;
    data->clusterId           = chip::app::Clusters::Thermostat::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ThermostatReadMaxHeatSetpointLimit(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::Thermostat::Attributes::MaxHeatSetpointLimit::Id;
    data->clusterId           = chip::app::Clusters::Thermostat::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ThermostatReadMinCoolSetpointLimit(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::Thermostat::Attributes::MinCoolSetpointLimit::Id;
    data->clusterId           = chip::app::Clusters::Thermostat::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ThermostatReadMaxCoolSetpointLimit(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::Thermostat::Attributes::MaxCoolSetpointLimit::Id;
    data->clusterId           = chip::app::Clusters::Thermostat::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ThermostatReadControlSequenceOfOperation(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::Thermostat::Attributes::ControlSequenceOfOperation::Id;
    data->clusterId           = chip::app::Clusters::Thermostat::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ThermostatReadSystemMode(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->attributeId         = chip::app::Clusters::Thermostat::Attributes::SystemMode::Id;
    data->clusterId           = chip::app::Clusters::Thermostat::Id;
    data->isReadAttribute     = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}

/********************************************************
 * Groups Thermostat switch shell functions
 *********************************************************/

CHIP_ERROR GroupsThermostatHelpHandler(int argc, char ** argv)
{
    sShellSwitchGroupsThermostatSubCommands.ForEachCommand(chip::Shell::PrintCommandHelp, nullptr);
    return CHIP_NO_ERROR;
}

CHIP_ERROR GroupsThermostatSwitchCommandHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        return GroupsThermostatHelpHandler(argc, argv);
    }

    return sShellSwitchGroupsThermostatSubCommands.ExecCommand(argc, argv);
}

CHIP_ERROR GroupsSetpointRaiseLowerSwitchCommandHandler(int argc, char ** argv)
{
    BindingCommandData * data = chip::Platform::New<BindingCommandData>();
    data->commandId           = chip::app::Clusters::Thermostat::Commands::SetpointRaiseLower::Id;
    data->clusterId           = chip::app::Clusters::Thermostat::Id;
    data->isGroup             = true;

    chip::DeviceLayer::PlatformMgr().ScheduleWork(SwitchWorkerFunction, reinterpret_cast<intptr_t>(data));
    return CHIP_NO_ERROR;
}
#endif // CONFIG_ENABLE_CHIP_SHELL
