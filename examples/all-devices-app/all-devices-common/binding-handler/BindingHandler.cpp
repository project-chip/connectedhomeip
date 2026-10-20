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

namespace chip {
namespace app {

namespace {

// Manager successfully initialized through InitBindingManager.
Clusters::Binding::Manager * sInitializedBindingManager = nullptr;

// Exchange manager registered through InitBindingManager, handed to BindingContext::DispatchCommand.
Messaging::ExchangeManager * sExchangeManager = nullptr;

} // namespace

CHIP_ERROR DispatchBindingCommand(Clusters::Binding::Manager & bindingManager, EndpointId localEndpointId,
                                  Platform::UniquePtr<BindingContext> command)
{
    VerifyOrReturnError(command != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    // Reject managers that did not go through InitBindingManager while we still own `command`
    // (it is freed by the UniquePtr).
    VerifyOrReturnError(&bindingManager == sInitializedBindingManager, CHIP_ERROR_INCORRECT_STATE,
                        ChipLogError(AppServer, "DispatchBindingCommand: Binding manager not initialized"));

    ClusterId clusterId = command->GetClusterId();

    // From here on, ownership belongs to the Binding::Manager, which always calls
    // AllDevicesBoundDeviceContextReleaseHandler once it no longer needs the context.
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

    // Pending notifications may still complete after ShutdownBindingHandler(); the context is
    // released normally through the release handler, it is just not dispatched.
    VerifyOrReturn(
        sExchangeManager != nullptr,
        ChipLogError(AppServer, "AllDevicesBoundDeviceChangedHandler: Binding handler not initialized, skipping dispatch"));

    auto * bindingContext = static_cast<BindingContext *>(context);
    bindingContext->DispatchCommand(binding, peer_device, *sExchangeManager);
}

void AllDevicesBoundDeviceContextReleaseHandler(void * context)
{
    VerifyOrReturn(context != nullptr);
    Platform::Delete(static_cast<BindingContext *>(context));
}

CHIP_ERROR InitBindingManager(Clusters::Binding::Manager & bindingManager, const Clusters::Binding::ManagerInitParams & params,
                              Messaging::ExchangeManager & exchangeManager)
{
    ReturnErrorOnFailure(bindingManager.Init(params));
    bindingManager.RegisterBoundDeviceChangedHandler(AllDevicesBoundDeviceChangedHandler);
    bindingManager.RegisterBoundDeviceContextReleaseHandler(AllDevicesBoundDeviceContextReleaseHandler);
    sExchangeManager           = &exchangeManager;
    sInitializedBindingManager = &bindingManager;
    return CHIP_NO_ERROR;
}

void ShutdownBindingHandler()
{
    sInitializedBindingManager = nullptr;
    sExchangeManager           = nullptr;
}

} // namespace app
} // namespace chip
