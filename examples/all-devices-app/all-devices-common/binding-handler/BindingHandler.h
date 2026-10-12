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
#include <platform/PlatformManager.h>

#include <memory>
#include <type_traits>
#include <utility>

namespace chip {
namespace app {

/**
 * Returns the default ExchangeManager used for group binding commands when no
 * custom ExchangeManager is injected.
 */
Messaging::ExchangeManager & GetDefaultExchangeManager();

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
    explicit SimpleBindingCommand(const CommandType & command, Messaging::ExchangeManager * exchangeMgr = nullptr) :
        mCommand(command), mExchangeManager(exchangeMgr)
    {}
    explicit SimpleBindingCommand(CommandType && command, Messaging::ExchangeManager * exchangeMgr = nullptr) :
        mCommand(std::move(command)), mExchangeManager(exchangeMgr)
    {}

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
                (void) status;
                (void) dataResponse;
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
            Messaging::ExchangeManager * exchangeMgr =
                mExchangeManager != nullptr ? mExchangeManager : &GetDefaultExchangeManager();
            CHIP_ERROR err = Controller::InvokeGroupCommandRequest(exchangeMgr, binding.fabricIndex, binding.groupId, mCommand);
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
    Messaging::ExchangeManager * mExchangeManager = nullptr;
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
 * Dispatches a binding command synchronously using a Platform::UniquePtr.
 * Ownership is transferred to BindingManager on success, or deleted on failure.
 */
CHIP_ERROR DispatchBindingCommand(Clusters::Binding::Manager & bindingManager,
                                  EndpointId localEndpointId,
                                  Platform::UniquePtr<BindingContext> command);

/**
 * Schedules a binding command onto the PlatformManager event loop using a Platform::UniquePtr.
 * Ownership is transferred on success, or deleted on failure.
 */
CHIP_ERROR ScheduleBindingCommand(DeviceLayer::PlatformManager & platformManager,
                                  Clusters::Binding::Manager & bindingManager,
                                  EndpointId localEndpointId,
                                  Platform::UniquePtr<BindingContext> command);

/**
 * Dispatches a derived BindingContext synchronously using a Platform::UniquePtr.
 * Ownership is transferred to BindingManager on success, or deleted on failure.
 */
template <typename ContextType, typename = std::enable_if_t<!std::is_same_v<ContextType, BindingContext>>>
inline CHIP_ERROR DispatchBindingCommand(Clusters::Binding::Manager & bindingManager,
                                         EndpointId localEndpointId,
                                         Platform::UniquePtr<ContextType> command)
{
    static_assert(std::is_base_of_v<BindingContext, ContextType>, "ContextType must derive from BindingContext");
    return DispatchBindingCommand(bindingManager, localEndpointId,
                                  Platform::UniquePtr<BindingContext>(command.release()));
}

/**
 * Schedules a derived BindingContext onto the PlatformManager event loop using a Platform::UniquePtr.
 * Ownership is transferred on success, or deleted on failure.
 */
template <typename ContextType, typename = std::enable_if_t<!std::is_same_v<ContextType, BindingContext>>>
inline CHIP_ERROR ScheduleBindingCommand(DeviceLayer::PlatformManager & platformManager,
                                         Clusters::Binding::Manager & bindingManager,
                                         EndpointId localEndpointId,
                                         Platform::UniquePtr<ContextType> command)
{
    static_assert(std::is_base_of_v<BindingContext, ContextType>, "ContextType must derive from BindingContext");
    return ScheduleBindingCommand(platformManager, bindingManager, localEndpointId,
                                  Platform::UniquePtr<BindingContext>(command.release()));
}

/**
 * Allocates a SimpleBindingCommand for a standard cluster command struct and dispatches it synchronously.
 */
template <typename CommandType>
inline CHIP_ERROR DispatchSimpleBindingCommand(Clusters::Binding::Manager & bindingManager,
                                               EndpointId localEndpointId,
                                               CommandType && commandPayload,
                                               Messaging::ExchangeManager * exchangeMgr = nullptr)
{
    using DecayedCommand = std::decay_t<CommandType>;
    auto command = Platform::MakeUnique<SimpleBindingCommand<DecayedCommand>>(std::forward<CommandType>(commandPayload), exchangeMgr);
    VerifyOrReturnError(command != nullptr, CHIP_ERROR_NO_MEMORY);
    return DispatchBindingCommand(bindingManager, localEndpointId, std::move(command));
}

/**
 * Allocates a SimpleBindingCommand for a standard cluster command struct and schedules it on the PlatformManager event loop.
 */
template <typename CommandType>
inline CHIP_ERROR ScheduleSimpleBindingCommand(DeviceLayer::PlatformManager & platformManager,
                                               Clusters::Binding::Manager & bindingManager,
                                               EndpointId localEndpointId,
                                               CommandType && commandPayload,
                                               Messaging::ExchangeManager * exchangeMgr = nullptr)
{
    using DecayedCommand = std::decay_t<CommandType>;
    auto command = Platform::MakeUnique<SimpleBindingCommand<DecayedCommand>>(std::forward<CommandType>(commandPayload), exchangeMgr);
    VerifyOrReturnError(command != nullptr, CHIP_ERROR_NO_MEMORY);
    return ScheduleBindingCommand(platformManager, bindingManager, localEndpointId, std::move(command));
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
