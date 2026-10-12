/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#pragma once

#include <app/CommandSender.h>
#include <app/OperationalSessionSetup.h>
#include <app/clusters/bindings/BindingManager.h>
#include <app/server/Server.h>
#include <controller/InvokeInteraction.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/PlatformManager.h>

#include <memory>
#include <utility>

namespace chip {
class OperationalDeviceProxy;

namespace app {

/**
 * Abstract base class for binding command contexts.
 *
 * Ensures type-safe dispatch and safe deallocation of context data passed to
 * Clusters::Binding::Manager::NotifyBoundClusterChanged.
 */
class BindingContext
{
public:
    virtual ~BindingContext() = default;

    /**
     * Target cluster ID for this binding command.
     */
    virtual ClusterId GetClusterId() const = 0;

    /**
     * Dispatches the command to a bound peer (unicast or multicast).
     *
     * @param binding Table entry for the binding.
     * @param peer_device Proxy to connected peer (nullptr for multicast group bindings).
     */
    virtual void DispatchCommand(const Clusters::Binding::TableEntry & binding,
                                 OperationalDeviceProxy * peer_device) = 0;
};

/**
 * Reusable command context for standard ZAP-generated cluster commands.
 *
 * Automatically handles unicast InvokeCommandRequest and multicast
 * InvokeGroupCommandRequest with standard logging.
 *
 * @tparam CommandType Standard cluster command struct, e.g.:
 *         - Clusters::Chime::Commands::PlayChimeSound::Type
 *         - Clusters::OnOff::Commands::Toggle::Type
 *         - Clusters::LevelControl::Commands::MoveToLevel::Type
 */
template <typename CommandType>
class SimpleBindingCommand : public BindingContext
{
public:
    explicit SimpleBindingCommand(const CommandType & command) : mCommand(command) {}
    explicit SimpleBindingCommand(CommandType && command) : mCommand(std::move(command)) {}

    ClusterId GetClusterId() const override { return CommandType::GetClusterId(); }

    void DispatchCommand(const Clusters::Binding::TableEntry & binding,
                         OperationalDeviceProxy * peer_device) override
    {
        ClusterId targetClusterId = binding.clusterId.value_or(CommandType::GetClusterId());
        if (targetClusterId != CommandType::GetClusterId())
        {
            ChipLogDetail(AppServer, "Binding cluster mismatch (binding: 0x%" PRIx32 ", command: 0x%" PRIx32 ")",
                          targetClusterId, CommandType::GetClusterId());
            return;
        }

        if (binding.type == Clusters::Binding::MATTER_UNICAST_BINDING)
        {
            VerifyOrReturn(peer_device != nullptr && peer_device->ConnectionReady(),
                           ChipLogError(AppServer, "SimpleBindingCommand: Peer device connection not ready"));

            auto onSuccess = [](const ConcreteCommandPath & commandPath, const StatusIB & status, const auto & dataResponse) {
                ChipLogProgress(AppServer, "Binding command (0x%" PRIx32 ", 0x%" PRIx32 ") succeeded on endpoint %u",
                                CommandType::GetClusterId(), CommandType::GetCommandId(), commandPath.mEndpointId);
            };

            auto onFailure = [](CHIP_ERROR error) {
                ChipLogError(AppServer, "Binding command (0x%" PRIx32 ", 0x%" PRIx32 ") failed: %" CHIP_ERROR_FORMAT,
                             CommandType::GetClusterId(), CommandType::GetCommandId(), error.Format());
            };

            CHIP_ERROR err = Controller::InvokeCommandRequest(
                peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(),
                binding.remote, mCommand, onSuccess, onFailure);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(AppServer, "Failed to invoke binding command: %" CHIP_ERROR_FORMAT, err.Format());
            }
        }
        else if (binding.type == Clusters::Binding::MATTER_MULTICAST_BINDING)
        {
            Messaging::ExchangeManager & exchangeMgr = Server::GetInstance().GetExchangeManager();
            CHIP_ERROR err = Controller::InvokeGroupCommandRequest(&exchangeMgr, binding.fabricIndex, binding.groupId, mCommand);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(AppServer, "Failed to invoke binding group command: %" CHIP_ERROR_FORMAT, err.Format());
            }
            else
            {
                ChipLogProgress(AppServer, "Binding group command sent to group %u", binding.groupId);
            }
        }
        else
        {
            ChipLogError(AppServer, "SimpleBindingCommand: Unsupported binding type %u", static_cast<unsigned>(binding.type));
        }
    }

private:
    CommandType mCommand;
};

/**
 * Initializes the Binding Manager and registers the bound device changed
 * and context release handlers.
 *
 * This function schedules the actual initialization onto the PlatformManager
 * event loop so that Server dependencies are fully available.
 */
CHIP_ERROR InitBindingHandler();

/**
 * Dispatches a binding command synchronously.
 * Ownership of command is transferred to BindingManager on success.
 */
CHIP_ERROR DispatchBindingCommand(Clusters::Binding::Manager & bindingManager,
                                  EndpointId localEndpointId,
                                  BindingContext * command);

/**
 * Schedules a binding command onto the PlatformManager event loop.
 * Ownership of command is transferred on success.
 */
CHIP_ERROR ScheduleBindingCommand(DeviceLayer::PlatformManager & platformManager,
                                  Clusters::Binding::Manager & bindingManager,
                                  EndpointId localEndpointId,
                                  BindingContext * command);

/**
 * Dispatches a binding command synchronously using a Platform::UniquePtr.
 * Ownership is released on success, or deleted on failure.
 */
template <typename ContextType>
inline CHIP_ERROR DispatchBindingCommand(Clusters::Binding::Manager & bindingManager,
                                         EndpointId localEndpointId,
                                         Platform::UniquePtr<ContextType> command)
{
    static_assert(std::is_base_of_v<BindingContext, ContextType>, "ContextType must derive from BindingContext");
    VerifyOrReturnError(command != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    CHIP_ERROR err = DispatchBindingCommand(bindingManager, localEndpointId, command.get());
    if (err == CHIP_NO_ERROR)
    {
        command.release();
    }
    return err;
}

/**
 * Schedules a binding command onto the PlatformManager event loop using a Platform::UniquePtr.
 * Ownership is transferred on success, or deleted on failure.
 */
template <typename ContextType>
inline CHIP_ERROR ScheduleBindingCommand(DeviceLayer::PlatformManager & platformManager,
                                         Clusters::Binding::Manager & bindingManager,
                                         EndpointId localEndpointId,
                                         Platform::UniquePtr<ContextType> command)
{
    static_assert(std::is_base_of_v<BindingContext, ContextType>, "ContextType must derive from BindingContext");
    VerifyOrReturnError(command != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    CHIP_ERROR err = ScheduleBindingCommand(platformManager, bindingManager, localEndpointId, command.get());
    if (err == CHIP_NO_ERROR)
    {
        command.release();
    }
    return err;
}

/**
 * Handler invoked by the Binding::Manager when a binding needs to be notified.
 * Dispatches commands to bound unicast or multicast endpoints.
 */
void AllDevicesBoundDeviceChangedHandler(const Clusters::Binding::TableEntry & binding,
                                         OperationalDeviceProxy * peer_device, void * context);

/**
 * Handler invoked by the Binding::Manager when the command context is no longer needed.
 * Frees any context resources allocated during notification dispatch.
 */
void AllDevicesBoundDeviceContextReleaseHandler(void * context);

} // namespace app
} // namespace chip
