/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <controller/CurrentFabricRemover.h>

#include <app-common/zap-generated/cluster-objects.h>

using namespace chip::app::Clusters;

namespace chip {
namespace Controller {

CHIP_ERROR CurrentFabricRemover::RemoveCurrentFabric(NodeId remoteNodeId, Callback::Callback<OnCurrentFabricRemove> * callback)
{
    mRemoteNodeId                = remoteNodeId;
    mCurrentFabricRemoveCallback = callback;
    mNextStep                    = Step::kReadCurrentFabricIndex;

    return mController->GetConnectedDevice(remoteNodeId, &mOnDeviceConnectedCallback, &mOnDeviceConnectionFailureCallback);
}

CHIP_ERROR CurrentFabricRemover::ReadCurrentFabricIndex(Messaging::ExchangeManager & exchangeMgr,
                                                        const SessionHandle & sessionHandle)
{
    using TypeInfo = OperationalCredentials::Attributes::CurrentFabricIndex::TypeInfo;
    ClusterBase cluster(exchangeMgr, sessionHandle, kRootEndpointId);

    return cluster.ReadAttribute<TypeInfo>(this, OnSuccessReadCurrentFabricIndex, OnReadAttributeFailure);
}

CHIP_ERROR CurrentFabricRemover::SendRemoveFabricIndex(Messaging::ExchangeManager & exchangeMgr,
                                                       const SessionHandle & sessionHandle)
{
    if (mFabricIndex == kUndefinedFabricIndex)
    {
        return CHIP_ERROR_INVALID_FABRIC_INDEX;
    }

    OperationalCredentials::Commands::RemoveFabric::Type request;
    request.fabricIndex = mFabricIndex;

    ClusterBase cluster(exchangeMgr, sessionHandle, 0);

    return cluster.InvokeCommand(request, this, OnSuccessRemoveFabric, OnCommandFailure);
}

void CurrentFabricRemover::OnDeviceConnectedFn(void * context, Messaging::ExchangeManager & exchangeMgr,
                                               const SessionHandle & sessionHandle)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    auto * self    = static_cast<CurrentFabricRemover *>(context);
    VerifyOrReturn(self != nullptr, ChipLogProgress(Controller, "Device connected callback with null context. Ignoring"));

    switch (self->mNextStep)
    {
    case Step::kReadCurrentFabricIndex: {
        err = self->ReadCurrentFabricIndex(exchangeMgr, sessionHandle);
        break;
    }
    case Step::kSendRemoveFabric: {
        err = self->SendRemoveFabricIndex(exchangeMgr, sessionHandle);
        break;
    }
    default:
        err = CHIP_ERROR_INCORRECT_STATE;
        break;
    }

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Current Fabric Remover failure : %" CHIP_ERROR_FORMAT, err.Format());
        FinishRemoveCurrentFabric(context, err);
    }
}

void CurrentFabricRemover::OnDeviceConnectionFailureFn(void * context, const ScopedNodeId & peerId, CHIP_ERROR err)
{
    ChipLogProgress(Controller, "OnDeviceConnectionFailureFn: %" CHIP_ERROR_FORMAT, err.Format());

    auto * self = static_cast<CurrentFabricRemover *>(context);
    VerifyOrReturn(self != nullptr, ChipLogProgress(Controller, "Device connected failure callback with null context. Ignoring"));

    FinishRemoveCurrentFabric(context, err);
}

void CurrentFabricRemover::OnSuccessReadCurrentFabricIndex(void * context, FabricIndex fabricIndex)
{
    auto * self = static_cast<CurrentFabricRemover *>(context);
    VerifyOrReturn(self != nullptr,
                   ChipLogProgress(Controller, "Success Read Current Fabric index callback with null context. Ignoring"));
    self->mFabricIndex = fabricIndex;
    self->mNextStep    = Step::kSendRemoveFabric;
    CHIP_ERROR err     = self->mController->GetConnectedDevice(self->mRemoteNodeId, &self->mOnDeviceConnectedCallback,
                                                               &self->mOnDeviceConnectionFailureCallback);
    if (err != CHIP_NO_ERROR)
    {
        FinishRemoveCurrentFabric(context, err);
    }
}

void CurrentFabricRemover::OnReadAttributeFailure(void * context, CHIP_ERROR err)
{
    ChipLogProgress(Controller, "OnReadAttributeFailure %" CHIP_ERROR_FORMAT, err.Format());

    auto * self = static_cast<CurrentFabricRemover *>(context);
    VerifyOrReturn(self != nullptr, ChipLogProgress(Controller, "Read Attribute failure callback with null context. Ignoring"));

    FinishRemoveCurrentFabric(context, err);
}

void CurrentFabricRemover::OnSuccessRemoveFabric(void * context,
                                                 const OperationalCredentials::Commands::NOCResponse::DecodableType & data)
{
    auto * self = static_cast<CurrentFabricRemover *>(context);
    VerifyOrReturn(self != nullptr,
                   ChipLogProgress(Controller, "Success Remove Fabric command callback with null context. Ignoring"));

    FinishRemoveCurrentFabric(context, CHIP_NO_ERROR);
}

void CurrentFabricRemover::OnCommandFailure(void * context, CHIP_ERROR err)
{
    ChipLogProgress(Controller, "OnCommandFailure %" CHIP_ERROR_FORMAT, err.Format());

    auto * self = static_cast<CurrentFabricRemover *>(context);
    VerifyOrReturn(self != nullptr, ChipLogProgress(Controller, "Send command failure callback with null context. Ignoring"));

    FinishRemoveCurrentFabric(context, err);
}

void CurrentFabricRemover::FinishRemoveCurrentFabric(void * context, CHIP_ERROR err)
{
    if (err == CHIP_NO_ERROR)
    {
        ChipLogProgress(Controller, "Remove Current Fabric succeeded.");
    }
    else
    {
        ChipLogError(Controller, "Remove Current Fabric Failed : %" CHIP_ERROR_FORMAT, err.Format());
    }
    auto * self     = static_cast<CurrentFabricRemover *>(context);
    self->mNextStep = Step::kAcceptRemoveFabricStart;
    if (self->mCurrentFabricRemoveCallback != nullptr)
    {
        self->mCurrentFabricRemoveCallback->mCall(self->mCurrentFabricRemoveCallback->mContext, self->mRemoteNodeId, err);
    }
}

AutoCurrentFabricRemover::AutoCurrentFabricRemover(DeviceController * controller) :
    CurrentFabricRemover(controller), mOnRemoveCurrentFabricCallback(OnRemoveCurrentFabric, this)
{}

CHIP_ERROR AutoCurrentFabricRemover::RemoveCurrentFabric(DeviceController * controller, NodeId remoteNodeId)
{
    // Not using Platform::New because we want to keep our constructor private.
    auto * remover = new (std::nothrow) AutoCurrentFabricRemover(controller);
    if (remover == nullptr)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    CHIP_ERROR err = remover->CurrentFabricRemover::RemoveCurrentFabric(remoteNodeId, &remover->mOnRemoveCurrentFabricCallback);
    if (err != CHIP_NO_ERROR)
    {
        delete remover;
    }
    // Else will clean up when the callback is called.
    return err;
}

void AutoCurrentFabricRemover::OnRemoveCurrentFabric(void * context, NodeId remoteNodeId, CHIP_ERROR status)
{
    auto * self = static_cast<AutoCurrentFabricRemover *>(context);
    delete self;
}
} // namespace Controller
} // namespace chip
