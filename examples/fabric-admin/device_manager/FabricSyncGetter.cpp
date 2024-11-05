/*
 *   Copyright (c) 2024 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "FabricSyncGetter.h"

using namespace ::chip;
using namespace ::chip::app;
using chip::app::ReadClient;

namespace admin {

namespace {

void OnDeviceConnectedWrapper(void * context, Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle)
{
    reinterpret_cast<FabricSyncGetter *>(context)->OnDeviceConnected(exchangeMgr, sessionHandle);
}

void OnDeviceConnectionFailureWrapper(void * context, const ScopedNodeId & peerId, CHIP_ERROR error)
{
    reinterpret_cast<FabricSyncGetter *>(context)->OnDeviceConnectionFailure(peerId, error);
}

} // namespace

FabricSyncGetter::FabricSyncGetter() :
    mOnDeviceConnectedCallback(OnDeviceConnectedWrapper, this),
    mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureWrapper, this)
{}

CHIP_ERROR FabricSyncGetter::GetFabricSynchronizationData(OnDoneCallback onDoneCallback, Controller::DeviceController & controller,
                                                          NodeId nodeId, EndpointId endpointId)
{
    assertChipStackLockedByCurrentThread();

    mEndpointId     = endpointId;
    mOnDoneCallback = onDoneCallback;

    CHIP_ERROR err = controller.GetConnectedDevice(nodeId, &mOnDeviceConnectedCallback, &mOnDeviceConnectionFailureCallback);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to connect to remote fabric bridge %" CHIP_ERROR_FORMAT, err.Format());
    }
    return err;
}

void FabricSyncGetter::OnAttributeData(const ConcreteDataAttributePath & path, TLV::TLVReader * data, const StatusIB & status)
{
    VerifyOrDie(path.mClusterId == Clusters::CommissionerControl::Id);

    if (!status.IsSuccess())
    {
        ChipLogError(NotSpecified, "Response Failure: %" CHIP_ERROR_FORMAT, status.ToChipError().Format());
        return;
    }

    switch (path.mAttributeId)
    {
    case Clusters::CommissionerControl::Attributes::SupportedDeviceCategories::Id: {
        mOnDoneCallback(*data);
        break;
    }
    default:
        break;
    }
}

void FabricSyncGetter::OnError(CHIP_ERROR error)
{
    ChipLogProgress(NotSpecified, "Error Getting SupportedDeviceCategories: %" CHIP_ERROR_FORMAT, error.Format());
}

void FabricSyncGetter::OnDone(ReadClient * apReadClient)
{
    ChipLogProgress(NotSpecified, "Reading SupportedDeviceCategories is done.");
}

void FabricSyncGetter::OnDeviceConnected(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle)
{
    mClient = std::make_unique<ReadClient>(app::InteractionModelEngine::GetInstance(), &exchangeMgr, *this /* callback */,
                                           ReadClient::InteractionType::Read);
    VerifyOrDie(mClient);

    AttributePathParams readPaths[1];
    readPaths[0] = AttributePathParams(mEndpointId, Clusters::CommissionerControl::Id,
                                       Clusters::CommissionerControl::Attributes::SupportedDeviceCategories::Id);

    ReadPrepareParams readParams(sessionHandle);

    readParams.mpAttributePathParamsList    = readPaths;
    readParams.mAttributePathParamsListSize = 1;

    CHIP_ERROR err = mClient->SendRequest(readParams);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to read SupportedDeviceCategories from the bridged device.");
        OnDone(nullptr);
        return;
    }
}

void FabricSyncGetter::OnDeviceConnectionFailure(const ScopedNodeId & peerId, CHIP_ERROR error)
{
    ChipLogError(NotSpecified, "FabricSyncGetter failed to connect to " ChipLogFormatX64, ChipLogValueX64(peerId.GetNodeId()));

    OnDone(nullptr);
}

} // namespace admin
