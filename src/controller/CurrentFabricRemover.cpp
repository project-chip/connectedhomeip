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
#include <controller-clusters/zap-generated/CHIPClusters.h>

using namespace chip::app::Clusters;

namespace chip {
namespace Controller {

CHIP_ERROR CurrentFabricRemover::CurrentFabricRemove(NodeId remoteDeviceId, Callback::Callback<OnCurrentFabricRemove> * callback)
{
    mRemoteDeviceId              = remoteDeviceId;
    mCurrentFabricRemoveCallback = callback;
    mNextStep                    = Step::kReadCurrentFabricIndex;

    return mCommissioner->GetConnectedDevice(remoteDeviceId, &mOnDeviceConnectedCallback, &mOnDeviceConnectionFailureCallback);
}

CHIP_ERROR CurrentFabricRemover::ReadCurrentFabricIndex(Messaging::ExchangeManager & exchangeMgr, SessionHandle & sessionHandle)
{
    using TypeInfo = chip::app::Clusters::OperationalCredentials::Attributes::CurrentFabricIndex::TypeInfo;
    OperationalCredentialsCluster cluster(exchangeMgr, sessionHandle, kRootEndpointId);

    return cluster.ReadAttribute<TypeInfo>(this, OnSuccessReadCurrentFabricIndex, OnReadAttributeFailure);
}

CHIP_ERROR CurrentFabricRemover::SendRemoveFabricIndex(Messaging::ExchangeManager & exchangeMgr, SessionHandle & sessionHandle)
{
    if (mFabricIndex == kUndefinedFabricIndex)
    {
        return CHIP_ERROR_INVALID_FABRIC_INDEX;
    }
    ChipLogProgress(Controller, "SendRemoveFabricIndex : %u", mFabricIndex);

    OperationalCredentials::Commands::RemoveFabric::Type request;
    request.fabricIndex = mFabricIndex;

    OperationalCredentialsCluster cluster(exchangeMgr, sessionHandle, 0);

    return cluster.InvokeCommand(request, this, OnSuccessRemoveFabric, OnCommandFailure);
}

void CurrentFabricRemover::OnDeviceConnectedFn(void * context, Messaging::ExchangeManager & exchangeMgr,
                                               SessionHandle & sessionHandle)
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
        ChipLogError(Controller, "%" CHIP_ERROR_FORMAT, err.Format());
        FinishRemoveCurrentFabric(context, err);
    }

    return;
}

void CurrentFabricRemover::OnDeviceConnectionFailureFn(void * context, const ScopedNodeId & peerId, CHIP_ERROR err)
{
    ChipLogProgress(Controller, "OnDeviceConnectionFailureFn: %s", err.AsString());

    auto * self = static_cast<CurrentFabricRemover *>(context);
    VerifyOrReturn(self != nullptr, ChipLogProgress(Controller, "Device connected callback with null context. Ignoring"));

    FinishRemoveCurrentFabric(context, err);
    return;
}

void CurrentFabricRemover::OnSuccessReadCurrentFabricIndex(void * context, uint8_t fabricIndex)
{
    auto * self = static_cast<CurrentFabricRemover *>(context);
    VerifyOrReturn(self != nullptr, ChipLogProgress(Controller, "Device connected callback with null context. Ignoring"));
    self->mFabricIndex = fabricIndex;
    self->mNextStep    = Step::kSendRemoveFabric;
    CHIP_ERROR err     = self->mCommissioner->GetConnectedDevice(self->mRemoteDeviceId, &self->mOnDeviceConnectedCallback,
                                                             &self->mOnDeviceConnectionFailureCallback);
    if (err != CHIP_NO_ERROR)
    {
        FinishRemoveCurrentFabric(context, err);
    }
    return;
}

void CurrentFabricRemover::OnReadAttributeFailure(void * context, CHIP_ERROR err)
{
    ChipLogProgress(Controller, "OnReadAttributeFailure %s", err.AsString());

    auto * self = static_cast<CurrentFabricRemover *>(context);
    VerifyOrReturn(self != nullptr, ChipLogProgress(Controller, "Device connected callback with null context. Ignoring"));

    FinishRemoveCurrentFabric(context, err);
    return;
}

void CurrentFabricRemover::OnSuccessRemoveFabric(void * context,
                                                 const OperationalCredentials::Commands::NOCResponse::DecodableType & data)
{
    auto * self = static_cast<CurrentFabricRemover *>(context);
    VerifyOrReturn(self != nullptr, ChipLogProgress(Controller, "Device connected callback with null context. Ignoring"));

    FinishRemoveCurrentFabric(context, CHIP_NO_ERROR);
    return;
}

void CurrentFabricRemover::OnCommandFailure(void * context, CHIP_ERROR err)
{
    ChipLogProgress(Controller, "OnCommandFailure %s", err.AsString());

    auto * self = static_cast<CurrentFabricRemover *>(context);
    VerifyOrReturn(self != nullptr, ChipLogProgress(Controller, "Device connected callback with null context. Ignoring"));

    FinishRemoveCurrentFabric(context, err);
    return;
}

void CurrentFabricRemover::FinishRemoveCurrentFabric(void * context, CHIP_ERROR err)
{
    ChipLogError(Controller, "Remove Current Fabric Result : %" CHIP_ERROR_FORMAT, err.Format());
    auto * self     = static_cast<CurrentFabricRemover *>(context);
    self->mNextStep = Step::kAcceptRemoveFabricStart;
    if (self->mCurrentFabricRemoveCallback != nullptr)
    {
        self->mCurrentFabricRemoveCallback->mCall(self->mCurrentFabricRemoveCallback->mContext, self->mRemoteDeviceId, err);
    }
}

AutoCurrentFabricRemover::AutoCurrentFabricRemover(DeviceCommissioner * commissioner) :
    CurrentFabricRemover(commissioner), mOnRemoveCurrentFabricCallback(OnRemoveCurrentFabric, this)
{}

CHIP_ERROR AutoCurrentFabricRemover::RemoveCurrentFabric(DeviceCommissioner * commissoner, NodeId remoteDeviceId)
{
    // Not using Platform::New because we want to keep our constructor private.
    auto * remover = new (std::nothrow) AutoCurrentFabricRemover(commissoner);
    if (remover == nullptr)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    CHIP_ERROR err = remover->CurrentFabricRemover::CurrentFabricRemove(remoteDeviceId, &remover->mOnRemoveCurrentFabricCallback);
    if (err != CHIP_NO_ERROR)
    {
        delete remover;
    }
    // Else will clean up when the callback is called.
    return err;
}

void AutoCurrentFabricRemover::OnRemoveCurrentFabric(void * context, NodeId remoteDeviceId, CHIP_ERROR status)
{
    auto * self = static_cast<AutoCurrentFabricRemover *>(context);
    delete self;
}
} // namespace Controller
} // namespace chip
