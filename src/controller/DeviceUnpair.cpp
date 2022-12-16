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

#include <controller-clusters/zap-generated/CHIPClusters.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/InteractionModelEngine.h>

using namespace chip::app::Clusters;
namespace chip {
namespace Controller {

DeviceUnpair::DeviceUnpair() : mOnDeviceConnectedCallback(OnDeviceConnectedFn, this),
                            mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureFn, this)
{}

void DeviceUnpair::RegisterCallback(Callback *callback)
{
    mCallback = callback;
}
void DeviceUnpair::UnpairDevice(DeviceProxy * proxy, NodeId remoteDeviceId)
{
    mRemoteDeviceId = remoteDeviceId;
    PerformDeviceUnpairStep(proxy, UnpairDeviceStage::kReadCurrentFabricIndex, CHIP_NO_ERROR);
}

void DeviceUnpair::PerformDeviceUnpairStep(DeviceProxy * proxy, UnpairDeviceStage stage, CHIP_ERROR err)
{
    mProxy = proxy;
    mStage = stage;

    switch (stage)
    {
        case kReadCurrentFabricIndex:
        {
            ReadCurrentFabricIndex(proxy);
            break;
        }
        case kSendRemoveFabric:
        {
            SendRemoveFabricIndex(proxy, mFabricIndex);
            break;
        }
        case kUnpairError:
        case kUnpairCleanUp:
        default:
        {
            FinishUnpairDevice(err);
            break;
        }
    }
}

void DeviceUnpair::ReadCurrentFabricIndex(DeviceProxy *proxy)
{
    app::InteractionModelEngine * engine = app::InteractionModelEngine::GetInstance();
    app::ReadPrepareParams readParams(proxy->GetSecureSession().Value());

    app::AttributePathParams readPaths[1];
    readPaths[0] = app::AttributePathParams(OperationalCredentials::Id,
                                            OperationalCredentials::Attributes::CurrentFabricIndex::Id);

    readParams.mpAttributePathParamsList    = readPaths;
    readParams.mAttributePathParamsListSize = 1;
    readParams.mIsFabricFiltered = false;

    auto attributeCache = Platform::MakeUnique<app::ClusterStateCache>(*this);
    auto readClient     = Platform::MakeUnique<app::ReadClient>(
        engine, proxy->GetExchangeManager(), attributeCache->GetBufferedCallback(), app::ReadClient::InteractionType::Read);
    CHIP_ERROR err = readClient->SendRequest(readParams);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Controller, "Failed to send read request for networking clusters");
        PerformDeviceUnpairStep(proxy, UnpairDeviceStage::kUnpairError, err);
        return;
    }
    
    mAttributeCache = std::move(attributeCache);
    mReadClient     = std::move(readClient);
}

void DeviceUnpair::SendRemoveFabricIndex(DeviceProxy *proxy, FabricIndex fabricIndex)
{
    if (fabricIndex == kUndefinedFabricIndex) {
        PerformDeviceUnpairStep(nullptr, UnpairDeviceStage::kUnpairError, CHIP_ERROR_INVALID_FABRIC_INDEX);
        return;
    }
    ChipLogProgress(Controller, "SendRemoveFabricIndex : %u", fabricIndex);

    OperationalCredentials::Commands::RemoveFabric::Type request;
    request.fabricIndex = fabricIndex;

    OperationalCredentialsCluster cluster(*proxy->GetExchangeManager(), proxy->GetSecureSession().Value(), 0);

    cluster.InvokeCommand(request, this, OnRemoveFabric, OnCommandFailure);
}

void DeviceUnpair::FinishUnpairDevice(CHIP_ERROR err)
{
    mAttributeCache = nullptr;
    mReadClient = nullptr;
    mProxy = nullptr;
    mOperationalDeviceProxy = nullptr;

    if (mCallback != nullptr) {
        mCallback->OnDeviceUnpair(mRemoteDeviceId, err);
    }
}

void DeviceUnpair::OnDeviceConnectedFn(void * context, Messaging::ExchangeManager & exchangeMgr,
                                             SessionHandle & sessionHandle)
{
    DeviceUnpair * deviceUnpair = static_cast<DeviceUnpair *>(context);
    VerifyOrReturn(deviceUnpair != nullptr, ChipLogProgress(Controller, "Device connected callback with null context. Ignoring"));

    auto proxy = Platform::MakeShared<OperationalDeviceProxy>(&exchangeMgr, sessionHandle);
    deviceUnpair->mOperationalDeviceProxy = std::move(proxy);
    deviceUnpair->PerformDeviceUnpairStep(deviceUnpair->mOperationalDeviceProxy.get(), UnpairDeviceStage::kReadCurrentFabricIndex, CHIP_NO_ERROR);
}

void DeviceUnpair::OnDeviceConnectionFailureFn(void * context, const ScopedNodeId & peerId, CHIP_ERROR err)
{
    ChipLogProgress(Controller, "OnDeviceConnectionFailureFn %s", err.AsString());

    DeviceUnpair * deviceUnpair = static_cast<DeviceUnpair *>(context);
    VerifyOrReturn(deviceUnpair != nullptr, ChipLogProgress(Controller, "Device connected callback with null context. Ignoring"));

    deviceUnpair->PerformDeviceUnpairStep(nullptr, UnpairDeviceStage::kUnpairError, err);
}

void DeviceUnpair::OnDone(app::ReadClient * apReadClient) {
    FabricIndex fabricIndex;
    CHIP_ERROR err = CHIP_NO_ERROR;
    err = mAttributeCache->ForEachAttribute(OperationalCredentials::Id, [this, &fabricIndex](const app::ConcreteAttributePath & path) {
        if (path.mAttributeId != OperationalCredentials::Attributes::CurrentFabricIndex::Id)
        {
            // Continue on
            return CHIP_NO_ERROR;
        }

        switch (path.mAttributeId)
        {
        case OperationalCredentials::Attributes::CurrentFabricIndex::Id:
            return this->mAttributeCache->Get<OperationalCredentials::Attributes::CurrentFabricIndex::TypeInfo>(path, fabricIndex);
        default:
            return CHIP_NO_ERROR;
        }
    });
    mFabricIndex = fabricIndex;
    PerformDeviceUnpairStep(mProxy, UnpairDeviceStage::kSendRemoveFabric, CHIP_NO_ERROR);
}

void DeviceUnpair::OnRemoveFabric(void * context,
                                       const OperationalCredentials::Commands::NOCResponse::DecodableType & data) {
    DeviceUnpair * deviceUnpair = static_cast<DeviceUnpair *>(context);
    VerifyOrReturn(deviceUnpair != nullptr, ChipLogProgress(Controller, "Device connected callback with null context. Ignoring"));

    deviceUnpair->PerformDeviceUnpairStep(nullptr, UnpairDeviceStage::kUnpairCleanUp, CHIP_NO_ERROR);
}

void DeviceUnpair::OnCommandFailure(void * context, CHIP_ERROR err) {
    ChipLogProgress(Controller, "OnCommandFailure %s", err.AsString());

    DeviceUnpair * deviceUnpair = static_cast<DeviceUnpair *>(context);
    VerifyOrReturn(deviceUnpair != nullptr, ChipLogProgress(Controller, "Device connected callback with null context. Ignoring"));

    deviceUnpair->PerformDeviceUnpairStep(nullptr, UnpairDeviceStage::kUnpairError, err);
}
}
}