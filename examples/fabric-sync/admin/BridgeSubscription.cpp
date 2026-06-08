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

#include "BridgeSubscription.h"
#include "DeviceManager.h"

using namespace ::chip;
using namespace ::chip::app;
using chip::app::ReadClient;

namespace admin {

namespace {

constexpr uint16_t kSubscribeMinInterval = 0;
constexpr uint16_t kSubscribeMaxInterval = 30;

void OnDeviceConnectedWrapper(void * context, Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle)
{
    reinterpret_cast<BridgeSubscription *>(context)->OnDeviceConnected(exchangeMgr, sessionHandle);
}

void OnDeviceConnectionFailureWrapper(void * context, const ScopedNodeId & peerId, CHIP_ERROR error)
{
    reinterpret_cast<BridgeSubscription *>(context)->OnDeviceConnectionFailure(peerId, error);
}

} // namespace

BridgeSubscription::BridgeSubscription() :
    mOnDeviceConnectedCallback(OnDeviceConnectedWrapper, this),
    mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureWrapper, this)
{}

CHIP_ERROR BridgeSubscription::StartSubscription(Controller::DeviceController & controller, NodeId nodeId, EndpointId endpointId)
{
    assertChipStackLockedByCurrentThread();

    VerifyOrDie(!subscriptionStarted); // Ensure it's not called multiple times.

    // Mark as started
    subscriptionStarted = true;

    mEndpointId = endpointId;

    CHIP_ERROR err = controller.GetConnectedDevice(nodeId, &mOnDeviceConnectedCallback, &mOnDeviceConnectionFailureCallback);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to connect to remote fabric sync bridge %" CHIP_ERROR_FORMAT, err.Format());
    }
    return err;
}

void BridgeSubscription::OnAttributeData(const ConcreteDataAttributePath & path, TLV::TLVReader * data, const StatusIB & status)
{
    if (!status.IsSuccess())
    {
        ChipLogError(NotSpecified, "Response Failure: %" CHIP_ERROR_FORMAT, status.ToChipError().Format());
        return;
    }

    if (data == nullptr)
    {
        ChipLogError(NotSpecified, "Response Failure: No Data");
        return;
    }

    DeviceManager::Instance().HandleAttributeData(path, *data);
}

void BridgeSubscription::OnEventData(const app::EventHeader & eventHeader, TLV::TLVReader * data, const app::StatusIB * status)
{
    if (status != nullptr)
    {
        CHIP_ERROR error = status->ToChipError();
        if (CHIP_NO_ERROR != error)
        {
            ChipLogError(NotSpecified, "Response Failure: %" CHIP_ERROR_FORMAT, error.Format());
            return;
        }
    }

    if (data == nullptr)
    {
        ChipLogError(NotSpecified, "Response Failure: No Data");
        return;
    }

    DeviceManager::Instance().HandleEventData(eventHeader, *data);
}

void BridgeSubscription::OnError(CHIP_ERROR error)
{
    ChipLogProgress(NotSpecified, "Error on remote fabric sync bridge subscription: %" CHIP_ERROR_FORMAT, error.Format());
}

void BridgeSubscription::OnDone(ReadClient * apReadClient)
{
    mClient.reset();
    ChipLogProgress(NotSpecified, "The remote fabric sync bridge subscription is terminated");

    // Reset the subscription state to allow retry
    subscriptionStarted = false;

    // TODO:(#36092) Fabric-Admin should attempt to re-subscribe when the subscription to the remote bridge is terminated.
}

void BridgeSubscription::OnDeviceConnected(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle)
{
    mClient = std::make_unique<ReadClient>(app::InteractionModelEngine::GetInstance(), &exchangeMgr /* echangeMgr */,
                                           *this /* callback */, ReadClient::InteractionType::Subscribe);
    VerifyOrDie(mClient);

    AttributePathParams readPaths[1];
    readPaths[0] = AttributePathParams(mEndpointId, Clusters::Descriptor::Id, Clusters::Descriptor::Attributes::PartsList::Id);

    EventPathParams eventPaths[1];
    eventPaths[0]                = EventPathParams(mEndpointId, Clusters::CommissionerControl::Id,
                                                   Clusters::CommissionerControl::Events::CommissioningRequestResult::Id);
    eventPaths[0].mIsUrgentEvent = true;

    ReadPrepareParams readParams(sessionHandle);

    readParams.mpAttributePathParamsList    = readPaths;
    readParams.mAttributePathParamsListSize = 1;
    readParams.mpEventPathParamsList        = eventPaths;
    readParams.mEventPathParamsListSize     = 1;
    readParams.mMinIntervalFloorSeconds     = kSubscribeMinInterval;
    readParams.mMaxIntervalCeilingSeconds   = kSubscribeMaxInterval;
    readParams.mKeepSubscriptions           = true;

    CHIP_ERROR err = mClient->SendRequest(readParams);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to issue subscription to the Descriptor Cluster of the remote bridged device.");
        OnDone(nullptr);
        return;
    }
}

void BridgeSubscription::OnDeviceConnectionFailure(const ScopedNodeId & peerId, CHIP_ERROR error)
{
    ChipLogError(NotSpecified, "BridgeSubscription failed to connect to " ChipLogFormatX64, ChipLogValueX64(peerId.GetNodeId()));
    OnDone(nullptr);
}

} // namespace admin
