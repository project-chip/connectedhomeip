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

#include "UniqueIdGetter.h"

using namespace ::chip;
using namespace ::chip::app;
using chip::app::ReadClient;

namespace {

void OnDeviceConnectedWrapper(void * context, Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle)
{
    reinterpret_cast<UniqueIdGetter *>(context)->OnDeviceConnected(exchangeMgr, sessionHandle);
}

void OnDeviceConnectionFailureWrapper(void * context, const ScopedNodeId & peerId, CHIP_ERROR error)
{
    reinterpret_cast<UniqueIdGetter *>(context)->OnDeviceConnectionFailure(peerId, error);
}

bool SuccessOrLog(CHIP_ERROR err, const char * name)
{
    if (err == CHIP_NO_ERROR)
    {
        return true;
    }

    ChipLogError(NotSpecified, "Failed to read %s: %" CHIP_ERROR_FORMAT, name, err.Format());

    return false;
}

} // namespace

UniqueIdGetter::UniqueIdGetter() :
    mOnDeviceConnectedCallback(OnDeviceConnectedWrapper, this),
    mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureWrapper, this)
{}

CHIP_ERROR UniqueIdGetter::GetUniqueId(OnDoneCallback onDoneCallback, Controller::DeviceController & controller, NodeId nodeId,
                                       EndpointId endpointId)
{
    assertChipStackLockedByCurrentThread();
    VerifyOrDie(!mCurrentlyGettingUid);

    mEndpointId       = endpointId;
    mOnDoneCallback   = onDoneCallback;
    mUniqueIdHasValue = false;
    memset(mUniqueId, 0, sizeof(mUniqueId));
    mCurrentlyGettingUid = true;

    CHIP_ERROR err = controller.GetConnectedDevice(nodeId, &mOnDeviceConnectedCallback, &mOnDeviceConnectionFailureCallback);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to connect to remote fabric sync bridge %" CHIP_ERROR_FORMAT, err.Format());
        mCurrentlyGettingUid = false;
    }
    return err;
}

void UniqueIdGetter::OnAttributeData(const ConcreteDataAttributePath & path, TLV::TLVReader * data, const StatusIB & status)
{
    VerifyOrDie(path.mClusterId == Clusters::BridgedDeviceBasicInformation::Id);

    if (!status.IsSuccess())
    {
        ChipLogError(NotSpecified, "Response Failure: %" CHIP_ERROR_FORMAT, status.ToChipError().Format());
        return;
    }

    switch (path.mAttributeId)
    {
    case Clusters::BridgedDeviceBasicInformation::Attributes::UniqueID::Id: {
        mUniqueIdHasValue = SuccessOrLog(data->GetString(mUniqueId, sizeof(mUniqueId)), "UniqueId");
        break;
    }
    default:
        break;
    }
}

void UniqueIdGetter::OnReportEnd()
{
    // We will call mOnDoneCallback in OnDone.
}

void UniqueIdGetter::OnError(CHIP_ERROR error)
{
    ChipLogProgress(NotSpecified, "Error Getting UID: %" CHIP_ERROR_FORMAT, error.Format());
}

void UniqueIdGetter::OnDone(ReadClient * apReadClient)
{
    mCurrentlyGettingUid = false;
    mOnDoneCallback(mUniqueIdHasValue ? std::make_optional<CharSpan>(mUniqueId) : std::nullopt);
}

void UniqueIdGetter::OnDeviceConnected(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle)
{
    VerifyOrDie(mCurrentlyGettingUid);
    mClient = std::make_unique<ReadClient>(app::InteractionModelEngine::GetInstance(), &exchangeMgr, *this /* callback */,
                                           ReadClient::InteractionType::Read);
    VerifyOrDie(mClient);

    AttributePathParams readPaths[1];
    readPaths[0] = AttributePathParams(mEndpointId, Clusters::BridgedDeviceBasicInformation::Id,
                                       Clusters::BridgedDeviceBasicInformation::Attributes::UniqueID::Id);

    ReadPrepareParams readParams(sessionHandle);

    readParams.mpAttributePathParamsList    = readPaths;
    readParams.mAttributePathParamsListSize = 1;

    CHIP_ERROR err = mClient->SendRequest(readParams);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to issue subscription to AdministratorCommissioning data");
        OnDone(nullptr);
        return;
    }
}

void UniqueIdGetter::OnDeviceConnectionFailure(const ScopedNodeId & peerId, CHIP_ERROR error)
{
    VerifyOrDie(mCurrentlyGettingUid);
    ChipLogError(NotSpecified, "DeviceSubscription failed to connect to " ChipLogFormatX64, ChipLogValueX64(peerId.GetNodeId()));

    OnDone(nullptr);
}
