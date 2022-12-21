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

#include <controller/DeviceUnpair.h>

#include <app-common/zap-generated/cluster-objects.h>
#include <controller-clusters/zap-generated/CHIPClusters.h>

using namespace chip::app::Clusters;

namespace chip {
namespace Controller {

DeviceUnpair::DeviceUnpair() :
    mOnDeviceConnectedCallback(OnDeviceConnectedFn, this), mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureFn, this)
{}

void DeviceUnpair::RegisterCallback(Callback * callback)
{
    mCallback = callback;
}
void DeviceUnpair::UnpairDevice(DeviceProxy * proxy, NodeId remoteDeviceId)
{
    mRemoteDeviceId = remoteDeviceId;
    RegisterDeviceProxy(proxy);
    PerformDeviceUnpairStep(UnpairDeviceStage::kReadCurrentFabricIndex);
}

void DeviceUnpair::PerformDeviceUnpairStep(UnpairDeviceStage stage)
{
    if (mProxy == nullptr)
    {
        mLastError = CHIP_ERROR_INCORRECT_STATE;
    }

    if (mLastError != CHIP_NO_ERROR)
    {
        stage = UnpairDeviceStage::kUnpairError;
    }

    mStage = stage;

    switch (stage)
    {
    case kReadCurrentFabricIndex: {
        ReadCurrentFabricIndex();
        break;
    }
    case kSendRemoveFabric: {
        SendRemoveFabricIndex(mFabricIndex);
        break;
    }
    case kUnpairError:
    case kUnpairCleanUp:
    default: {
        FinishUnpairDevice(mLastError);
        break;
    }
    }
}

void DeviceUnpair::ReadCurrentFabricIndex()
{
    using TypeInfo = chip::app::Clusters::OperationalCredentials::Attributes::CurrentFabricIndex::TypeInfo;
    OperationalCredentialsCluster cluster(*mProxy->GetExchangeManager(), mProxy->GetSecureSession().Value(), 0);

    CHIP_ERROR err = cluster.ReadAttribute<TypeInfo>(this, OnSuccessCurrentFabricIndex, OnReadAttributeFailure);

    if (err != CHIP_NO_ERROR)
    {
        SetLastError(err);
        PerformDeviceUnpairStep(UnpairDeviceStage::kUnpairError);
        return;
    }

    return;
}

void DeviceUnpair::SendRemoveFabricIndex(FabricIndex fabricIndex)
{
    if (fabricIndex == kUndefinedFabricIndex)
    {
        SetLastError(CHIP_ERROR_INVALID_FABRIC_INDEX);
        PerformDeviceUnpairStep(UnpairDeviceStage::kUnpairError);
        return;
    }
    ChipLogProgress(Controller, "SendRemoveFabricIndex : %u", fabricIndex);

    OperationalCredentials::Commands::RemoveFabric::Type request;
    request.fabricIndex = fabricIndex;

    OperationalCredentialsCluster cluster(*mProxy->GetExchangeManager(), mProxy->GetSecureSession().Value(), 0);

    CHIP_ERROR err = cluster.InvokeCommand(request, this, OnRemoveFabric, OnCommandFailure);
    if (err != CHIP_NO_ERROR)
    {
        SetLastError(err);
        PerformDeviceUnpairStep(UnpairDeviceStage::kUnpairError);
        return;
    }
}

void DeviceUnpair::FinishUnpairDevice(CHIP_ERROR err)
{
    mProxy                  = nullptr;
    mOperationalDeviceProxy = nullptr;

    if (mCallback != nullptr)
    {
        mCallback->OnDeviceUnpair(mRemoteDeviceId, err);
    }
}

void DeviceUnpair::GenerateOperationalDeviceProxy(Messaging::ExchangeManager & exchangeMgr, SessionHandle & sessionHandle)
{
    auto proxy              = Platform::MakeShared<OperationalDeviceProxy>(&exchangeMgr, sessionHandle);
    mOperationalDeviceProxy = std::move(proxy);
    RegisterDeviceProxy(mOperationalDeviceProxy.get());
}

void DeviceUnpair::OnDeviceConnectedFn(void * context, Messaging::ExchangeManager & exchangeMgr, SessionHandle & sessionHandle)
{
    DeviceUnpair * deviceUnpair = static_cast<DeviceUnpair *>(context);
    VerifyOrReturn(deviceUnpair != nullptr, ChipLogProgress(Controller, "Device connected callback with null context. Ignoring"));

    deviceUnpair->GenerateOperationalDeviceProxy(exchangeMgr, sessionHandle);
    deviceUnpair->PerformDeviceUnpairStep(UnpairDeviceStage::kReadCurrentFabricIndex);
}

void DeviceUnpair::OnDeviceConnectionFailureFn(void * context, const ScopedNodeId & peerId, CHIP_ERROR err)
{
    ChipLogProgress(Controller, "OnDeviceConnectionFailureFn %s", err.AsString());

    DeviceUnpair * deviceUnpair = static_cast<DeviceUnpair *>(context);
    VerifyOrReturn(deviceUnpair != nullptr, ChipLogProgress(Controller, "Device connected callback with null context. Ignoring"));

    deviceUnpair->SetLastError(err);
    deviceUnpair->PerformDeviceUnpairStep(UnpairDeviceStage::kUnpairError);
}

void DeviceUnpair::OnSuccessCurrentFabricIndex(void * context, uint8_t fabricIndex)
{
    DeviceUnpair * deviceUnpair = static_cast<DeviceUnpair *>(context);
    VerifyOrReturn(deviceUnpair != nullptr, ChipLogProgress(Controller, "Device connected callback with null context. Ignoring"));
    deviceUnpair->mFabricIndex = fabricIndex;

    deviceUnpair->PerformDeviceUnpairStep(UnpairDeviceStage::kSendRemoveFabric);
}

void DeviceUnpair::OnReadAttributeFailure(void * context, CHIP_ERROR err)
{
    ChipLogProgress(Controller, "OnCommandFailure %s", err.AsString());

    DeviceUnpair * deviceUnpair = static_cast<DeviceUnpair *>(context);
    VerifyOrReturn(deviceUnpair != nullptr, ChipLogProgress(Controller, "Device connected callback with null context. Ignoring"));

    deviceUnpair->SetLastError(err);
    deviceUnpair->PerformDeviceUnpairStep(UnpairDeviceStage::kUnpairError);
}

void DeviceUnpair::OnRemoveFabric(void * context, const OperationalCredentials::Commands::NOCResponse::DecodableType & data)
{
    DeviceUnpair * deviceUnpair = static_cast<DeviceUnpair *>(context);
    VerifyOrReturn(deviceUnpair != nullptr, ChipLogProgress(Controller, "Device connected callback with null context. Ignoring"));

    deviceUnpair->PerformDeviceUnpairStep(UnpairDeviceStage::kUnpairCleanUp);
}

void DeviceUnpair::OnCommandFailure(void * context, CHIP_ERROR err)
{
    ChipLogProgress(Controller, "OnCommandFailure %s", err.AsString());

    DeviceUnpair * deviceUnpair = static_cast<DeviceUnpair *>(context);
    VerifyOrReturn(deviceUnpair != nullptr, ChipLogProgress(Controller, "Device connected callback with null context. Ignoring"));

    deviceUnpair->SetLastError(err);
    deviceUnpair->PerformDeviceUnpairStep(UnpairDeviceStage::kUnpairError);
}
} // namespace Controller
} // namespace chip
