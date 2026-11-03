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

// Binding::Manager callbacks carry no handler state, so the context carries the group exchange manager
// along with the command.
struct BindingHandler::BindingContext
{
    BindingContext(Messaging::ExchangeManager & exchangeManager, Platform::UniquePtr<BindingCommand> && bindingCommand) :
        groupExchangeManager(exchangeManager), command(std::move(bindingCommand))
    {}

    Messaging::ExchangeManager & groupExchangeManager;
    Platform::UniquePtr<BindingCommand> command;
};

CHIP_ERROR BindingHandler::Init(Clusters::Binding::Manager & bindingManager, const Clusters::Binding::ManagerInitParams & params,
                                Messaging::ExchangeManager & exchangeManager)
{
    VerifyOrReturnError(!IsInitialized() && !mIsShutDown, CHIP_ERROR_INCORRECT_STATE,
                        ChipLogError(AppServer, "BindingHandler: Already initialized or shut down"));

    ReturnErrorOnFailure(bindingManager.Init(params));
    bindingManager.RegisterBoundDeviceChangedHandler(HandleBoundDeviceChanged);
    bindingManager.RegisterBoundDeviceContextReleaseHandler(ReleaseContext);

    mBindingManager  = &bindingManager;
    mExchangeManager = &exchangeManager;
    return CHIP_NO_ERROR;
}

void BindingHandler::Shutdown()
{
    mBindingManager  = nullptr;
    mExchangeManager = nullptr;
    mIsShutDown      = true;
}

CHIP_ERROR BindingHandler::Dispatch(EndpointId localEndpointId, Platform::UniquePtr<BindingCommand> command)
{
    VerifyOrReturnError(command != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INCORRECT_STATE,
                        ChipLogError(AppServer, "BindingHandler: Dispatch called before Init"));

    ClusterId clusterId = command->GetClusterId();

    // On allocation failure the constructor does not run, so `command` still owns the command and frees it.
    auto * context = Platform::New<BindingContext>(*mExchangeManager, std::move(command));
    VerifyOrReturnError(context != nullptr, CHIP_ERROR_NO_MEMORY);

    // From here on, the Binding::Manager owns the context and always calls ReleaseContext
    // once it no longer needs it.
    CHIP_ERROR err = mBindingManager->NotifyBoundClusterChanged(localEndpointId, clusterId, context);
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
    auto & bindingContext = *static_cast<BindingContext *>(context);

    switch (binding.type)
    {
    case Clusters::Binding::MATTER_UNICAST_BINDING: {
        VerifyOrReturn(peer_device != nullptr && peer_device->ConnectionReady() && peer_device->GetExchangeManager() != nullptr,
                       ChipLogError(AppServer, "BindingHandler: Peer device connection not ready"));
        auto session = peer_device->GetSecureSession();
        VerifyOrReturn(session.HasValue(), ChipLogError(AppServer, "BindingHandler: Peer device has no secure session"));

        bindingContext.command->Dispatch(UnicastBindingTarget{
            .exchangeManager = *peer_device->GetExchangeManager(),
            .session         = session.Value(),
            .endpoint        = binding.remote,
        });
        break;
    }
    case Clusters::Binding::MATTER_MULTICAST_BINDING:
        bindingContext.command->Dispatch(GroupBindingTarget{
            .exchangeManager = bindingContext.groupExchangeManager,
            .fabricIndex     = binding.fabricIndex,
            .groupId         = binding.groupId,
        });
        break;
    default:
        ChipLogError(AppServer, "BindingHandler: Unsupported binding type %u", static_cast<unsigned>(binding.type));
        break;
    }
}

void BindingHandler::ReleaseContext(void * context)
{
    VerifyOrReturn(context != nullptr);
    Platform::Delete(static_cast<BindingContext *>(context));
}

} // namespace app
} // namespace chip
