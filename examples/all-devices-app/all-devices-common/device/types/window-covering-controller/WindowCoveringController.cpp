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

#include "WindowCoveringController.h"

#include <clusters/Groups/Ids.h>
#include <clusters/Identify/Ids.h>
#include <clusters/WindowCovering/Ids.h>
#include <devices/Types.h>

#include <utility>

using namespace chip::app::Clusters;

namespace chip::app {

WindowCoveringController::WindowCoveringController(const Config & config) :
    SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kWindowCoveringController, 1)), mConfig(config)
{}

CHIP_ERROR WindowCoveringController::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                              EndpointComposition composition)
{
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    DeviceRegistrationTransaction transaction(*this, provider);

    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    mBindingCluster.Create(
        BindingCluster::Context{
            .bindingTable    = mConfig.bindingTable,
            .bindingManager  = mConfig.bindingManager,
            .platformManager = mConfig.platformManager,
        },
        endpoint);
    ReturnErrorOnFailure(provider.AddCluster(mBindingCluster.Registration()));

    if (mConfig.withIdentifyServer)
    {
        mIdentifyCluster.Create(IdentifyCluster::Config(endpoint, mConfig.timerDelegate));
        ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));
    }

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));
    transaction.Commit();
    return CHIP_NO_ERROR;
}

void WindowCoveringController::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);
    if (mIdentifyCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mIdentifyCluster.Cluster()));
        mIdentifyCluster.Destroy();
    }
    if (mBindingCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mBindingCluster.Cluster()));
        mBindingCluster.Destroy();
    }
}

CHIP_ERROR WindowCoveringController::ClientClusters(ReadOnlyBufferBuilder<ClusterId> & out) const
{
    ReturnErrorOnFailure(out.EnsureAppendCapacity(3));
    ReturnErrorOnFailure(out.Append(WindowCovering::Id));
    if (mConfig.withIdentifyClient)
    {
        ReturnErrorOnFailure(out.Append(Identify::Id));
    }
    if (mConfig.withGroupsClient)
    {
        ReturnErrorOnFailure(out.Append(Groups::Id));
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR WindowCoveringController::Dispatch(Platform::UniquePtr<BindingCommand> command)
{
    VerifyOrReturnError(mEndpointId != kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(command != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(IsEnabledClientCluster(command->GetClusterId()), CHIP_ERROR_INVALID_ARGUMENT);
    return mConfig.bindingHandler.Dispatch(mEndpointId, std::move(command));
}

bool WindowCoveringController::IsEnabledClientCluster(ClusterId clusterId) const
{
    switch (clusterId)
    {
    case WindowCovering::Id:
        return true;
    case Identify::Id:
        return mConfig.withIdentifyClient;
    case Groups::Id:
        return mConfig.withGroupsClient;
    default:
        return false;
    }
}

} // namespace chip::app
