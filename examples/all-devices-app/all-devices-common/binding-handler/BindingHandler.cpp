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

#include "BindingHandler.h"

#include <app/server/Server.h>
#include <platform/CHIPDeviceLayer.h>

namespace chip {
namespace app {

namespace {

// Manager successfully initialized through InitBindingManager. Only this manager is guaranteed to have
// passed the NotifyBoundClusterChanged preconditions (initialized + handlers registered).
Clusters::Binding::Manager * sInitializedBindingManager = nullptr;

void InitBindingHandlerInternal(intptr_t /* arg */)
{
    auto & server = Server::GetInstance();
    CHIP_ERROR err =
        InitBindingManager(Clusters::Binding::Manager::GetInstance(),
                           { &server.GetFabricTable(), server.GetCASESessionManager(), &server.GetPersistentStorage() });
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to initialize Binding Manager: %" CHIP_ERROR_FORMAT, err.Format());
        return;
    }
    ChipLogProgress(AppServer, "Binding Manager initialized successfully");
}

} // namespace

Messaging::ExchangeManager & GetDefaultExchangeManager()
{
    return Server::GetInstance().GetExchangeManager();
}

CHIP_ERROR DispatchBindingCommand(Clusters::Binding::Manager & bindingManager, EndpointId localEndpointId,
                                  Platform::UniquePtr<BindingContext> command)
{
    VerifyOrReturnError(command != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    // Reject managers that did not go through InitBindingManager while we still own `command`
    // (it is freed by the UniquePtr). This covers the NotifyBoundClusterChanged early returns
    // (CHIP_ERROR_INCORRECT_STATE / CHIP_ERROR_HANDLER_NOT_SET) that happen before it adopts the context.
    VerifyOrReturnError(&bindingManager == sInitializedBindingManager, CHIP_ERROR_INCORRECT_STATE,
                        ChipLogError(AppServer, "DispatchBindingCommand: Binding manager not initialized"));

    ClusterId clusterId = command->GetClusterId();

    // From here on, ownership belongs to the Binding::Manager, which always calls
    // AllDevicesBoundDeviceContextReleaseHandler once it no longer needs the context.
    //
    // NotifyBoundClusterChanged can fail *after* queueing the context for a unicast binding (e.g. when
    // session establishment fails synchronously). The queued entry keeps a reference and the context is
    // released later, so it must NOT be deleted here even though an error is returned. The only failure
    // that does not adopt the context is the PendingNotificationContext allocation failure, which cannot
    // be told apart from adopted failures by its error code; that case leaks the context rather than
    // risking a use-after-free.
    CHIP_ERROR err = bindingManager.NotifyBoundClusterChanged(localEndpointId, clusterId, static_cast<void *>(command.release()));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "NotifyBoundClusterChanged failed: %" CHIP_ERROR_FORMAT, err.Format());
    }
    return err;
}

void AllDevicesBoundDeviceChangedHandler(const Clusters::Binding::TableEntry & binding, OperationalDeviceProxy * peer_device,
                                         void * context)
{
    if (context == nullptr)
    {
        ChipLogDetail(AppServer, "AllDevicesBoundDeviceChangedHandler: Null binding context, skipping dispatch");
        return;
    }

    auto * bindingContext = static_cast<BindingContext *>(context);
    bindingContext->DispatchCommand(binding, peer_device);
}

void AllDevicesBoundDeviceContextReleaseHandler(void * context)
{
    VerifyOrReturn(context != nullptr);
    Platform::Delete(static_cast<BindingContext *>(context));
}

CHIP_ERROR InitBindingManager(Clusters::Binding::Manager & bindingManager, const Clusters::Binding::ManagerInitParams & params)
{
    ReturnErrorOnFailure(bindingManager.Init(params));
    bindingManager.RegisterBoundDeviceChangedHandler(AllDevicesBoundDeviceChangedHandler);
    bindingManager.RegisterBoundDeviceContextReleaseHandler(AllDevicesBoundDeviceContextReleaseHandler);
    sInitializedBindingManager = &bindingManager;
    return CHIP_NO_ERROR;
}

void ShutdownBindingHandler()
{
    sInitializedBindingManager = nullptr;
}

CHIP_ERROR InitBindingHandler()
{
    ReturnErrorOnFailure(DeviceLayer::PlatformMgr().ScheduleWork(InitBindingHandlerInternal));
    return CHIP_NO_ERROR;
}

} // namespace app
} // namespace chip
