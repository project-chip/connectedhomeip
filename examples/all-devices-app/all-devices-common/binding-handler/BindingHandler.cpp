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

#include <binding-handler/BindingHandler.h>

#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {

namespace {

// Binding::Manager callbacks are plain function pointers without user data, so they reach the
// initialized handler through this pointer.
BindingHandler * sActiveHandler = nullptr;

} // namespace

CHIP_ERROR BindingHandler::Init(Clusters::Binding::Manager & bindingManager, const Clusters::Binding::ManagerInitParams & params,
                                Messaging::ExchangeManager & exchangeManager)
{
    VerifyOrReturnError(sActiveHandler == nullptr, CHIP_ERROR_INCORRECT_STATE,
                        ChipLogError(AppServer, "BindingHandler: Another handler is already initialized"));

    ReturnErrorOnFailure(bindingManager.Init(params));
    bindingManager.RegisterBoundDeviceChangedHandler(HandleBoundDeviceChanged);
    bindingManager.RegisterBoundDeviceContextReleaseHandler(ReleaseContext);

    mBindingManager  = &bindingManager;
    mExchangeManager = &exchangeManager;
    sActiveHandler   = this;
    return CHIP_NO_ERROR;
}

void BindingHandler::Shutdown()
{
    if (sActiveHandler == this)
    {
        sActiveHandler = nullptr;
    }
    mBindingManager  = nullptr;
    mExchangeManager = nullptr;
}

CHIP_ERROR BindingHandler::Dispatch(EndpointId localEndpointId, Platform::UniquePtr<BindingContext> command)
{
    VerifyOrReturnError(command != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE,
                        ChipLogError(AppServer, "BindingHandler: Dispatch called before Init"));

    ClusterId clusterId = command->GetClusterId();

    // From here on, the Binding::Manager owns the command and always calls ReleaseContext
    // once it no longer needs it.
    CHIP_ERROR err = mBindingManager->NotifyBoundClusterChanged(localEndpointId, clusterId, static_cast<void *>(command.release()));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "NotifyBoundClusterChanged failed: %" CHIP_ERROR_FORMAT, err.Format());
    }
    return err;
}

void BindingHandler::HandleBoundDeviceChanged(const Clusters::Binding::TableEntry & binding, OperationalDeviceProxy * peer_device,
                                              void * context)
{
    VerifyOrReturn(context != nullptr, ChipLogDetail(AppServer, "BindingHandler: Null binding context, skipping dispatch"));
    VerifyOrReturn(sActiveHandler != nullptr,
                   ChipLogError(AppServer, "BindingHandler: Not initialized, skipping dispatch of pending notification"));

    static_cast<BindingContext *>(context)->DispatchCommand(binding, peer_device, *sActiveHandler->mExchangeManager);
}

void BindingHandler::ReleaseContext(void * context)
{
    VerifyOrReturn(context != nullptr);
    Platform::Delete(static_cast<BindingContext *>(context));
}

} // namespace app
} // namespace chip
