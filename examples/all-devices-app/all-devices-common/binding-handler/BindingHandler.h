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
#include <binding-handler/BindingCommand.h>
#include <binding-handler/SimpleBindingCommand.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <messaging/ExchangeMgr.h>

#include <utility>

namespace chip {
namespace app {

/**
 * Sends commands to the peers bound to a local endpoint, through a Clusters::Binding::Manager.
 *
 * Devices that act as a client of another cluster (e.g. a Doorbell sending Chime commands) receive a
 * reference to a BindingHandler and call Dispatch/DispatchSimple. The application owns the instance,
 * calls Init once the server is initialized, and calls Shutdown before shutting the server down.
 *
 * All methods must be called from the Matter event loop or with the Matter stack lock held.
 */
class BindingHandler
{
public:
    BindingHandler() = default;

    BindingHandler(const BindingHandler &)             = delete;
    BindingHandler & operator=(const BindingHandler &) = delete;

    /**
     * Initializes @p bindingManager and registers the binding callbacks on it.
     *
     * Must be called after the objects referenced by @p params (fabric table, CASE session manager,
     * storage) are initialized.
     *
     * @param bindingManager  Manager to initialize (usually Clusters::Binding::Manager::GetInstance()).
     *                        Must outlive this handler, or Shutdown must be called first.
     * @param params          Init parameters forwarded to Clusters::Binding::Manager::Init.
     * @param exchangeManager Exchange manager used for group targets (GroupBindingTarget::exchangeManager).
     *                        Must outlive every command dispatched through this handler, including commands
     *                        still pending in @p bindingManager after Shutdown.
     *
     * @retval CHIP_ERROR_INCORRECT_STATE if this handler is already initialized, or if Shutdown was called.
     *         Re-initialization is not supported because Clusters::Binding::Manager::Init is not idempotent.
     */
    CHIP_ERROR Init(Clusters::Binding::Manager & bindingManager, const Clusters::Binding::ManagerInitParams & params,
                    Messaging::ExchangeManager & exchangeManager);

    /**
     * Shutdown is final: subsequent Dispatch and Init calls fail with CHIP_ERROR_INCORRECT_STATE. Unicast
     * commands already queued in the manager are still sent if the session to the peer is established
     * later, and are released as usual. Safe to call when not initialized.
     */
    void Shutdown();

    bool IsInitialized() const { return mBindingManager != nullptr; }

    /**
     * Sends @p command to every binding on @p localEndpointId that matches command->GetClusterId().
     *
     * @p localEndpointId is the local endpoint that hosts the client cluster and the Binding cluster
     * (e.g. the Doorbell endpoint for Chime commands). A binding matches when its local endpoint is
     * @p localEndpointId and its cluster is either unset or equal to command->GetClusterId().
     *
     * A conflicting pending unicast target rejects the entire submission with CHIP_ERROR_BUSY before
     * any send. Errors transfer no ownership to the manager and invoke no completion callbacks.
     * Success transfers ownership; each target is dispatched or fails within caseTimeout (default 30s).
     * No matching bindings is a successful no-op with no target callbacks.
     * Removal of a pending binding reports CHIP_ERROR_CANCELLED. Shutdown does not cancel accepted work.
     * DispatchSimple reports each target's response or local group send via options.onComplete, possibly
     * before returning. Response timeouts cannot guarantee that the peer did not execute a command.
     * Custom commands own their response callbacks; their Failed method receives pre-send failures.
     * Ownership of command always transfers; command may be destroyed before returning or before responses.
     */
    CHIP_ERROR Dispatch(EndpointId localEndpointId, Platform::UniquePtr<BindingCommand> command,
                        System::Clock::Timeout caseTimeout = System::Clock::Seconds16(30));

    /**
     * Wraps a standard cluster command struct in a SimpleBindingCommand and calls Dispatch.
     * Same contract as Dispatch.
     */
    template <typename CommandType>
    CHIP_ERROR DispatchSimple(EndpointId localEndpointId, const CommandType & commandPayload, BindingCommandOptions options = {})
    {
        VerifyOrReturnError(options.caseTimeout.count() > 0 && options.responseTimeout.count() > 0, CHIP_ERROR_INVALID_ARGUMENT);
        Platform::UniquePtr<BindingCommand> command(Platform::New<SimpleBindingCommand<CommandType>>(commandPayload, options));
        VerifyOrReturnError(command != nullptr, CHIP_ERROR_NO_MEMORY);
        return Dispatch(localEndpointId, std::move(command), options.caseTimeout);
    }

private:
    // The `void * context` handed to Binding::Manager. Defined in BindingHandler.cpp.
    struct BindingContext;

    static void HandleBoundDeviceChanged(const Clusters::Binding::TableEntry & binding, OperationalDeviceProxy * peer_device,
                                         void * context);
    static void HandleCommandFailure(const Clusters::Binding::TableEntry & binding, void * context, CHIP_ERROR error);
    static void ReleaseContext(void * context);

    Clusters::Binding::Manager * mBindingManager  = nullptr;
    Messaging::ExchangeManager * mExchangeManager = nullptr;
    bool mIsShutDown                              = false;
};

} // namespace app
} // namespace chip
