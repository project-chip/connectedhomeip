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

#include <app/clusters/bindings/BindingManager.h>
#include <controller/InvokeInteraction.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include <memory>
#include <utility>

namespace chip {
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
     * @param exchangeManager Exchange manager registered through InitBindingManager. Unicast commands
     *                        should use peer_device->GetExchangeManager(); this one is meant for
     *                        multicast (group) commands, which have no peer device.
     */
    virtual void DispatchCommand(const Clusters::Binding::TableEntry & binding, OperationalDeviceProxy * peer_device,
                                 Messaging::ExchangeManager & exchangeManager) = 0;
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

    void DispatchCommand(const Clusters::Binding::TableEntry & binding, OperationalDeviceProxy * peer_device,
                         Messaging::ExchangeManager & exchangeManager) override
    {
        ClusterId targetClusterId = binding.clusterId.value_or(CommandType::GetClusterId());
        if (targetClusterId != CommandType::GetClusterId())
        {
            ChipLogDetail(AppServer, "Binding cluster mismatch (binding: 0x%" PRIx32 ", command: 0x%" PRIx32 ")", targetClusterId,
                          CommandType::GetClusterId());
            return;
        }

        if (binding.type == Clusters::Binding::MATTER_UNICAST_BINDING)
        {
            VerifyOrReturn(peer_device != nullptr && peer_device->ConnectionReady(),
                           ChipLogError(AppServer, "SimpleBindingCommand: Peer device connection not ready"));

            auto onSuccess = [](const ConcreteCommandPath & commandPath, const StatusIB & status, const auto & dataResponse) {
                (void) status;
                (void) dataResponse;
                ChipLogProgress(AppServer, "Binding command (0x%" PRIx32 ", 0x%" PRIx32 ") succeeded on endpoint %u",
                                CommandType::GetClusterId(), CommandType::GetCommandId(), commandPath.mEndpointId);
            };

            auto onFailure = [](CHIP_ERROR error) {
                ChipLogError(AppServer, "Binding command (0x%" PRIx32 ", 0x%" PRIx32 ") failed: %" CHIP_ERROR_FORMAT,
                             CommandType::GetClusterId(), CommandType::GetCommandId(), error.Format());
            };

            CHIP_ERROR err =
                Controller::InvokeCommandRequest(peer_device->GetExchangeManager(), peer_device->GetSecureSession().Value(),
                                                 binding.remote, mCommand, onSuccess, onFailure);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(AppServer, "Failed to invoke binding command: %" CHIP_ERROR_FORMAT, err.Format());
            }
        }
        else if (binding.type == Clusters::Binding::MATTER_MULTICAST_BINDING)
        {
            CHIP_ERROR err =
                Controller::InvokeGroupCommandRequest(&exchangeManager, binding.fabricIndex, binding.groupId, mCommand);
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
 * Initializes @p bindingManager and registers the all-devices-app handlers on it.
 *
 * Synchronous: must be called from the Matter event loop or with the Matter stack lock held, after the
 * objects referenced by @p params (fabric table, CASE session manager, storage) are initialized.
 * Only the manager initialized through this function is accepted by DispatchBindingCommand.
 *
 * @param bindingManager  Manager to initialize (usually Clusters::Binding::Manager::GetInstance()).
 * @param params          Init parameters forwarded to Clusters::Binding::Manager::Init.
 * @param exchangeManager Exchange manager handed to BindingContext::DispatchCommand, used for group commands.
 */
CHIP_ERROR InitBindingManager(Clusters::Binding::Manager & bindingManager, const Clusters::Binding::ManagerInitParams & params,
                              Messaging::ExchangeManager & exchangeManager);

/**
 * Forgets the manager and exchange manager registered by InitBindingManager. Subsequent dispatches fail with
 * CHIP_ERROR_INCORRECT_STATE, and pending notifications are skipped, until InitBindingManager is called again.
 */
void ShutdownBindingHandler();

/**
 * Dispatches a binding command synchronously. Must be called from the Matter event loop.
 *
 * Custom contexts are passed as a base pointer, e.g.
 * `Platform::UniquePtr<BindingContext>(Platform::New<MyContext>(...))`.
 *
 * Ownership of @p command:
 *  - If @p bindingManager was not initialized through InitBindingManager, CHIP_ERROR_INCORRECT_STATE
 *    is returned and @p command is destroyed before returning.
 *  - Otherwise ownership is transferred to the Binding::Manager, even if an error is returned
 *    (the command may have been queued for a unicast peer before the failure). The command is
 *    destroyed through AllDevicesBoundDeviceContextReleaseHandler once it is no longer needed.
 */
CHIP_ERROR DispatchBindingCommand(Clusters::Binding::Manager & bindingManager, EndpointId localEndpointId,
                                  Platform::UniquePtr<BindingContext> command);

/**
 * Wraps a standard cluster command struct in a SimpleBindingCommand and dispatches it synchronously.
 * Must be called from the Matter event loop. Same ownership rules as DispatchBindingCommand.
 */
template <typename CommandType>
inline CHIP_ERROR DispatchSimpleBindingCommand(Clusters::Binding::Manager & bindingManager, EndpointId localEndpointId,
                                               const CommandType & commandPayload)
{
    Platform::UniquePtr<BindingContext> command(Platform::New<SimpleBindingCommand<CommandType>>(commandPayload));
    VerifyOrReturnError(command != nullptr, CHIP_ERROR_NO_MEMORY);
    return DispatchBindingCommand(bindingManager, localEndpointId, std::move(command));
}

/**
 * Handler invoked by the Binding::Manager when a binding needs to be notified.
 * Dispatches commands to bound unicast or multicast endpoints.
 */
void AllDevicesBoundDeviceChangedHandler(const Clusters::Binding::TableEntry & binding, OperationalDeviceProxy * peer_device,
                                         void * context);

/**
 * Handler invoked by the Binding::Manager when the command context is no longer needed.
 * Frees any context resources allocated during notification dispatch.
 */
void AllDevicesBoundDeviceContextReleaseHandler(void * context);

} // namespace app
} // namespace chip
