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

const void * sLastReleasedContext = nullptr;

void InitBindingHandlerInternal(intptr_t /* arg */)
{
    auto & server  = Server::GetInstance();
    CHIP_ERROR err = Clusters::Binding::Manager::GetInstance().Init(
        { &server.GetFabricTable(), server.GetCASESessionManager(), &server.GetPersistentStorage() });
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to initialize Binding Manager: %" CHIP_ERROR_FORMAT, err.Format());
        return;
    }

    Clusters::Binding::Manager::GetInstance().RegisterBoundDeviceChangedHandler(AllDevicesBoundDeviceChangedHandler);
    Clusters::Binding::Manager::GetInstance().RegisterBoundDeviceContextReleaseHandler(AllDevicesBoundDeviceContextReleaseHandler);
    ChipLogProgress(AppServer, "Binding Manager initialized successfully");
}

struct AsyncBindingWorkData
{
    EndpointId localEndpointId;
    Clusters::Binding::Manager * bindingManager;
    Platform::UniquePtr<BindingContext> command;
};

void AsyncBindingWorkerFunction(intptr_t context)
{
    VerifyOrReturn(context != 0, ChipLogError(AppServer, "AsyncBindingWorkerFunction - Invalid context"));
    auto * workData = reinterpret_cast<AsyncBindingWorkData *>(context);
    Platform::UniquePtr<AsyncBindingWorkData> cleanup(workData);

    Clusters::Binding::Manager * manager =
        workData->bindingManager != nullptr ? workData->bindingManager : &Clusters::Binding::Manager::GetInstance();

    CHIP_ERROR err = DispatchBindingCommand(*manager, workData->localEndpointId, std::move(workData->command));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "AsyncBindingWorkerFunction: DispatchBindingCommand failed: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

} // namespace

Messaging::ExchangeManager & GetDefaultExchangeManager()
{
    return Server::GetInstance().GetExchangeManager();
}

CHIP_ERROR DispatchBindingCommand(Clusters::Binding::Manager & bindingManager,
                                  EndpointId localEndpointId,
                                  Platform::UniquePtr<BindingContext> command)
{
    VerifyOrReturnError(command != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    ClusterId clusterId         = command->GetClusterId();
    BindingContext * rawCommand = command.release();
    sLastReleasedContext        = nullptr;

    CHIP_ERROR err = bindingManager.NotifyBoundClusterChanged(localEndpointId, clusterId, static_cast<void *>(rawCommand));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "NotifyBoundClusterChanged failed: %" CHIP_ERROR_FORMAT, err.Format());
        if (sLastReleasedContext != rawCommand)
        {
            Platform::Delete(rawCommand);
        }
    }
    return err;
}

CHIP_ERROR ScheduleBindingCommand(DeviceLayer::PlatformManager & platformManager,
                                  Clusters::Binding::Manager & bindingManager,
                                  EndpointId localEndpointId,
                                  Platform::UniquePtr<BindingContext> command)
{
    VerifyOrReturnError(command != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    auto workData = Platform::MakeUnique<AsyncBindingWorkData>();
    VerifyOrReturnError(workData != nullptr, CHIP_ERROR_NO_MEMORY);

    workData->localEndpointId = localEndpointId;
    workData->bindingManager  = &bindingManager;
    workData->command         = std::move(command);

    CHIP_ERROR err = platformManager.ScheduleWork(AsyncBindingWorkerFunction, reinterpret_cast<intptr_t>(workData.get()));
    if (err == CHIP_NO_ERROR)
    {
        workData.release();
    }
    return err;
}

void AllDevicesBoundDeviceChangedHandler(const Clusters::Binding::TableEntry & binding,
                                         OperationalDeviceProxy * peer_device, void * context)
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
    sLastReleasedContext = context;
    Platform::Delete(static_cast<BindingContext *>(context));
}

CHIP_ERROR InitBindingHandler()
{
    ReturnErrorOnFailure(DeviceLayer::PlatformMgr().ScheduleWork(InitBindingHandlerInternal));
    return CHIP_NO_ERROR;
}

} // namespace app
} // namespace chip
