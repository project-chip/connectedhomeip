/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "binding-handler.h"

#include <app-common/zap-generated/ids/Clusters.h>
#include <app-common/zap-generated/ids/Commands.h>
#include <app/CommandSender.h>
#include <app/clusters/bindings/BindingManager.h>
#include <app/server/Server.h>
#include <clusters/LevelControl/Commands.h>
#include <clusters/OnOff/Commands.h>
#include <controller/InvokeInteraction.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip::app::Clusters;

namespace chip::app {

namespace {

void BoundDeviceChangedHandler(const Binding::TableEntry & binding, OperationalDeviceProxy * peer_device, void * context)
{
    uintptr_t args    = reinterpret_cast<uintptr_t>(context);
    uint8_t commandId = static_cast<uint8_t>(args & kBindingCommandMask);
    uint8_t param1    = static_cast<uint8_t>((args >> 8) & 0xFF);
    uint8_t param2    = static_cast<uint8_t>((args >> 16) & 0xFF);
    uint8_t param3    = static_cast<uint8_t>((args >> 24) & 0xFF);

    auto onSuccess = [](const ConcreteCommandPath & commandPath, const StatusIB & status, const auto & dataResponse) {
        ChipLogProgress(AppServer, "Binding command succeeded");
    };
    auto onFailure = [](CHIP_ERROR error) {
        ChipLogError(AppServer, "Binding command failed: %" CHIP_ERROR_FORMAT, error.Format());
    };

    auto SendCommand = [&](auto & commandObj) {
        if (binding.type == Binding::MATTER_UNICAST_BINDING)
        {
            if (peer_device == nullptr || !peer_device->ConnectionReady() || !peer_device->GetSecureSession().HasValue())
            {
                ChipLogError(AppServer, "Peer device connection or session is not ready");
                return;
            }
            TEMPORARY_RETURN_IGNORED Controller::InvokeCommandRequest(peer_device->GetExchangeManager(),
                                                                      peer_device->GetSecureSession().Value(), binding.remote,
                                                                      commandObj, onSuccess, onFailure);
        }
        else if (binding.type == Binding::MATTER_MULTICAST_BINDING)
        {
            TEMPORARY_RETURN_IGNORED Controller::InvokeGroupCommandRequest(&Server::GetInstance().GetExchangeManager(),
                                                                           binding.fabricIndex, binding.groupId, commandObj);
        }
    };

    switch (commandId)
    {
    case kBindingOnOffOnCommandId: {
        Clusters::OnOff::Commands::On::Type cmd;
        SendCommand(cmd);
        break;
    }
    case kBindingOnOffOffCommandId: {
        Clusters::OnOff::Commands::Off::Type cmd;
        SendCommand(cmd);
        break;
    }
    case kBindingOnOffToggleCommandId: {
        Clusters::OnOff::Commands::Toggle::Type cmd;
        SendCommand(cmd);
        break;
    }
    case kBindingLevelMoveToLevelCommandId: {
        Clusters::LevelControl::Commands::MoveToLevel::Type cmd;
        cmd.level = param1;
        cmd.transitionTime.SetNonNull(static_cast<uint16_t>(param2) * 10); // Convert sec to 10ths of sec
        cmd.optionsMask.SetRaw(param3);
        cmd.optionsOverride.SetRaw(param3);
        SendCommand(cmd);
        break;
    }
    case kBindingLevelMoveCommandId: {
        Clusters::LevelControl::Commands::Move::Type cmd;
        cmd.moveMode = static_cast<Clusters::LevelControl::MoveModeEnum>(param1);
        cmd.rate.SetNonNull(param2);
        cmd.optionsMask.SetRaw(param3);
        cmd.optionsOverride.SetRaw(param3);
        SendCommand(cmd);
        break;
    }
    case kBindingLevelStepCommandId: {
        Clusters::LevelControl::Commands::Step::Type cmd;
        cmd.stepMode = static_cast<Clusters::LevelControl::StepModeEnum>(param1);
        cmd.stepSize = param2;
        cmd.transitionTime.SetNonNull(static_cast<uint16_t>(0));
        cmd.optionsMask.SetRaw(param3);
        cmd.optionsOverride.SetRaw(param3);
        SendCommand(cmd);
        break;
    }
    case kBindingLevelStopCommandId: {
        Clusters::LevelControl::Commands::Stop::Type cmd;
        SendCommand(cmd);
        break;
    }
    default:
        ChipLogError(AppServer, "Unknown binding command ID: %u", commandId);
        break;
    }
}

void BoundDeviceContextReleaseHandler(void * context)
{
    (void) context;
}

void InitBindingHandlerInternal(intptr_t arg)
{
    auto & server = Server::GetInstance();
    LogErrorOnFailure(Binding::Manager::GetInstance().Init(
        { &server.GetFabricTable(), server.GetCASESessionManager(), &server.GetPersistentStorage() }));
    Binding::Manager::GetInstance().RegisterBoundDeviceChangedHandler(BoundDeviceChangedHandler);
    Binding::Manager::GetInstance().RegisterBoundDeviceContextReleaseHandler(BoundDeviceContextReleaseHandler);
}

} // namespace

CHIP_ERROR InitBindingHandler()
{
    static bool sBindingHandlerInitialized = false;
    if (sBindingHandlerInitialized)
    {
        return CHIP_NO_ERROR;
    }
    sBindingHandlerInitialized = true;

    // The server instance needs to be initialized before the binding handler. This is why
    // we schedule the init of the binding handler to be run later, once the neccessary values
    // can be fetched from the server.
    TEMPORARY_RETURN_IGNORED DeviceLayer::PlatformMgr().ScheduleWork(InitBindingHandlerInternal);
    return CHIP_NO_ERROR;
}

void SimulateBindingOnOff(EndpointId endpointId, bool on)
{
    uintptr_t cmdId = on ? kBindingOnOffOnCommandId : kBindingOnOffOffCommandId;
    TEMPORARY_RETURN_IGNORED Binding::Manager::GetInstance().NotifyBoundClusterChanged(endpointId, Clusters::OnOff::Id,
                                                                                       reinterpret_cast<void *>(cmdId));
}

void SimulateBindingToggle(EndpointId endpointId)
{
    uintptr_t cmdId = kBindingOnOffToggleCommandId;
    TEMPORARY_RETURN_IGNORED Binding::Manager::GetInstance().NotifyBoundClusterChanged(endpointId, Clusters::OnOff::Id,
                                                                                       reinterpret_cast<void *>(cmdId));
}

void SimulateBindingMoveToLevel(EndpointId endpointId, uint8_t newLevel, uint8_t transitionTimeSec, uint8_t optionsMask)
{
    uintptr_t packedArgs = kBindingLevelMoveToLevelCommandId | (static_cast<uintptr_t>(newLevel) << 8) |
        (static_cast<uintptr_t>(transitionTimeSec) << 16) | (static_cast<uintptr_t>(optionsMask) << 24);

    TEMPORARY_RETURN_IGNORED Binding::Manager::GetInstance().NotifyBoundClusterChanged(endpointId, Clusters::LevelControl::Id,
                                                                                       reinterpret_cast<void *>(packedArgs));
}

void SimulateBindingMove(EndpointId endpointId, uint8_t moveMode, uint8_t rate, uint8_t optionsMask)
{
    uintptr_t packedArgs = kBindingLevelMoveCommandId | (static_cast<uintptr_t>(moveMode) << 8) |
        (static_cast<uintptr_t>(rate) << 16) | (static_cast<uintptr_t>(optionsMask) << 24);

    TEMPORARY_RETURN_IGNORED Binding::Manager::GetInstance().NotifyBoundClusterChanged(endpointId, Clusters::LevelControl::Id,
                                                                                       reinterpret_cast<void *>(packedArgs));
}

void SimulateBindingStep(EndpointId endpointId, uint8_t stepMode, uint8_t stepSize, uint8_t optionsMask)
{
    uintptr_t packedArgs = kBindingLevelStepCommandId | (static_cast<uintptr_t>(stepMode) << 8) |
        (static_cast<uintptr_t>(stepSize) << 16) | (static_cast<uintptr_t>(optionsMask) << 24);

    TEMPORARY_RETURN_IGNORED Binding::Manager::GetInstance().NotifyBoundClusterChanged(endpointId, Clusters::LevelControl::Id,
                                                                                       reinterpret_cast<void *>(packedArgs));
}

void SimulateBindingStop(EndpointId endpointId)
{
    uintptr_t cmdId = kBindingLevelStopCommandId;
    TEMPORARY_RETURN_IGNORED Binding::Manager::GetInstance().NotifyBoundClusterChanged(endpointId, Clusters::LevelControl::Id,
                                                                                       reinterpret_cast<void *>(cmdId));
}

} // namespace chip::app
