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

#include "DeviceSynchronization.h"
#include "DeviceManager.h"
#include "DeviceSubscriptionManager.h"

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/InteractionModelEngine.h>
#include <app/server/Server.h>

using namespace ::chip;
using namespace ::chip::app;
using chip::app::ReadClient;

namespace admin {

namespace {

constexpr uint16_t kBasicInformationAttributeBufSize = 128;

void OnDeviceConnectedWrapper(void * context, Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle)
{
    reinterpret_cast<DeviceSynchronizer *>(context)->OnDeviceConnected(exchangeMgr, sessionHandle);
}

void OnDeviceConnectionFailureWrapper(void * context, const ScopedNodeId & peerId, CHIP_ERROR error)
{
    reinterpret_cast<DeviceSynchronizer *>(context)->OnDeviceConnectionFailure(peerId, error);
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

DeviceSynchronizer & DeviceSynchronizer::Instance()
{
    static DeviceSynchronizer instance;
    return instance;
}

DeviceSynchronizer::DeviceSynchronizer() :
    mOnDeviceConnectedCallback(OnDeviceConnectedWrapper, this),
    mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureWrapper, this)
{}

void DeviceSynchronizer::OnAttributeData(const ConcreteDataAttributePath & path, TLV::TLVReader * data, const StatusIB & status)
{
    VerifyOrDie(path.mEndpointId == kRootEndpointId);
    VerifyOrDie(path.mClusterId == Clusters::BasicInformation::Id);

    if (!status.IsSuccess())
    {
        ChipLogError(NotSpecified, "Response Failure: %" CHIP_ERROR_FORMAT, status.ToChipError().Format());
        return;
    }

    switch (path.mAttributeId)
    {
    case Clusters::BasicInformation::Attributes::UniqueID::Id: {
        char uniqueIdBuffer[kBasicInformationAttributeBufSize];
        if (SuccessOrLog(data->GetString(uniqueIdBuffer, sizeof(uniqueIdBuffer)), "UniqueId"))
        {
            mCurrentDeviceData.uniqueId = std::string(uniqueIdBuffer);
        }
    }
    break;
    case Clusters::BasicInformation::Attributes::VendorName::Id: {
        char vendorNameBuffer[kBasicInformationAttributeBufSize];
        if (SuccessOrLog(data->GetString(vendorNameBuffer, sizeof(vendorNameBuffer)), "VendorName"))
        {
            mCurrentDeviceData.vendorName = std::string(vendorNameBuffer);
        }
    }
    break;
    case Clusters::BasicInformation::Attributes::ProductName::Id: {
        char productNameBuffer[kBasicInformationAttributeBufSize];
        if (SuccessOrLog(data->GetString(productNameBuffer, sizeof(productNameBuffer)), "ProductName"))
        {
            mCurrentDeviceData.productName = std::string(productNameBuffer);
        }
    }
    break;
    case Clusters::BasicInformation::Attributes::NodeLabel::Id: {
        char nodeLabelBuffer[kBasicInformationAttributeBufSize];
        if (SuccessOrLog(data->GetString(nodeLabelBuffer, sizeof(nodeLabelBuffer)), "NodeLabel"))
        {
            mCurrentDeviceData.nodeLabel = std::string(nodeLabelBuffer);
        }
    }
    break;
    case Clusters::BasicInformation::Attributes::HardwareVersionString::Id: {
        char hardwareVersionStringBuffer[kBasicInformationAttributeBufSize];
        if (SuccessOrLog(data->GetString(hardwareVersionStringBuffer, sizeof(hardwareVersionStringBuffer)),
                         "HardwareVersionString"))
        {
            mCurrentDeviceData.hardwareVersionString = std::string(hardwareVersionStringBuffer);
        }
    }
    break;
    case Clusters::BasicInformation::Attributes::SoftwareVersionString::Id: {
        char softwareVersionStringBuffer[kBasicInformationAttributeBufSize];
        if (SuccessOrLog(data->GetString(softwareVersionStringBuffer, sizeof(softwareVersionStringBuffer)),
                         "SoftwareVersionString"))
        {
            mCurrentDeviceData.softwareVersionString = std::string(softwareVersionStringBuffer);
        }
    }
    break;
    default:
        break;
    }
}

void DeviceSynchronizer::OnReportEnd()
{
    // Report end is at the end of all attributes (success)
    MoveToState(State::ReceivedResponse);
}

void DeviceSynchronizer::OnDone(app::ReadClient * apReadClient)
{
    ChipLogProgress(NotSpecified, "Synchronization complete for NodeId:" ChipLogFormatX64, ChipLogValueX64(mNodeId));

    if (mState == State::ReceivedResponse && !DeviceManager::Instance().IsCurrentBridgeDevice(mNodeId))
    {
        GetUniqueId();
        if (mState == State::GettingUid)
        {
            ChipLogProgress(NotSpecified,
                            "GetUniqueId was successful and we rely on callback to call SynchronizationCompleteAddDevice.");
            return;
        }
        SynchronizationCompleteAddDevice();
    }

    MoveToState(State::Idle);
}

void DeviceSynchronizer::OnError(CHIP_ERROR error)
{
    MoveToState(State::ReceivedError);
    ChipLogProgress(NotSpecified, "Error fetching device data: %" CHIP_ERROR_FORMAT, error.Format());
}

void DeviceSynchronizer::OnDeviceConnected(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle)
{
    mClient = std::make_unique<ReadClient>(app::InteractionModelEngine::GetInstance(), &exchangeMgr /* echangeMgr */,
                                           *this /* callback */, ReadClient::InteractionType::Read);
    VerifyOrDie(mClient);

    AttributePathParams readPaths[1];
    readPaths[0] = AttributePathParams(kRootEndpointId, Clusters::BasicInformation::Id);

    ReadPrepareParams readParams(sessionHandle);

    readParams.mpAttributePathParamsList    = readPaths;
    readParams.mAttributePathParamsListSize = 1;

    CHIP_ERROR err = mClient->SendRequest(readParams);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to issue read for BasicInformation data");
        MoveToState(State::Idle);
    }
    MoveToState(State::AwaitingResponse);
}

void DeviceSynchronizer::OnDeviceConnectionFailure(const ScopedNodeId & peerId, CHIP_ERROR error)
{
    ChipLogError(NotSpecified, "Device Sync failed to connect to " ChipLogFormatX64, ChipLogValueX64(peerId.GetNodeId()));
    MoveToState(State::Idle);
}

void DeviceSynchronizer::StartDeviceSynchronization(Controller::DeviceController * controller, NodeId nodeId, bool deviceIsIcd)
{
    VerifyOrDie(controller);
    if (mState != State::Idle)
    {
        ChipLogError(NotSpecified, "Device Sync NOT POSSIBLE: another sync is in progress");
        return;
    }

    mNodeId = nodeId;

    ChipLogProgress(NotSpecified, "Start device synchronization for NodeId:" ChipLogFormatX64, ChipLogValueX64(mNodeId));

    mCurrentDeviceData       = SynchronizedDevice_init_default;
    mCurrentDeviceData.id    = chip::ScopedNodeId(nodeId, controller->GetFabricIndex());
    mCurrentDeviceData.isIcd = deviceIsIcd;

    ReturnOnFailure(controller->GetConnectedDevice(nodeId, &mOnDeviceConnectedCallback, &mOnDeviceConnectionFailureCallback));
    mController = controller;
    MoveToState(State::Connecting);
}

void DeviceSynchronizer::GetUniqueId()
{
    VerifyOrDie(mState == State::ReceivedResponse);
    VerifyOrDie(mController);

    // If we have a UniqueId we can return leaving state in ReceivedResponse.
    VerifyOrReturn(!mCurrentDeviceData.uniqueId.has_value(), ChipLogDetail(NotSpecified, "We already have UniqueId"));

    auto * device = DeviceManager::Instance().FindDeviceByNode(mNodeId);
    // If there is no associated remote Fabric Sync Aggregator there is no other place for us to try
    // getting the UniqueId from and can return leaving the state in ReceivedResponse.
    VerifyOrReturn(device, ChipLogDetail(NotSpecified, "No remote Fabric Sync Aggregator to get UniqueId from"));

    // Because device is not-null we expect IsFabricSyncReady to be true. IsFabricSyncReady indicates we have a
    // connection to the remote Fabric Sync Aggregator.
    VerifyOrDie(DeviceManager::Instance().IsFabricSyncReady());
    auto remoteBridgeNodeId               = DeviceManager::Instance().GetRemoteBridgeNodeId();
    EndpointId remoteEndpointIdOfInterest = device->GetEndpointId();

    ChipLogDetail(NotSpecified, "Attempting to get UniqueId from remote Fabric Sync Aggregator");
    CHIP_ERROR err = mUniqueIdGetter.GetUniqueId(
        [this](std::optional<CharSpan> aUniqueId) {
            if (aUniqueId.has_value())
            {
                // Convert CharSpan to std::string and set it as uniqueId
                this->mCurrentDeviceData.uniqueId = std::string(aUniqueId.value().data(), aUniqueId.value().size());
            }
            else
            {
                ChipLogError(NotSpecified, "We expected to get UniqueId from remote Fabric Sync Aggregator, but failed");
            }
            this->SynchronizationCompleteAddDevice();
        },
        *mController, remoteBridgeNodeId, remoteEndpointIdOfInterest);

    if (err == CHIP_NO_ERROR)
    {
        MoveToState(State::GettingUid);
    }
    else
    {
        ChipLogDetail(NotSpecified, "Failed to get UniqueId from remote Fabric Sync Aggregator");
    }
}

void DeviceSynchronizer::SynchronizationCompleteAddDevice()
{
    VerifyOrDie(mState == State::ReceivedResponse || mState == State::GettingUid);
    ChipLogProgress(NotSpecified, "Synchronization complete and add device");

    bridge::FabricBridge::Instance().AddSynchronizedDevice(mCurrentDeviceData);

    // TODO(#35077) Figure out how we should reflect CADMIN values of ICD.
    if (!mCurrentDeviceData.isIcd)
    {
        VerifyOrDie(mController);
        ScopedNodeId scopedNodeId(mNodeId, mController->GetFabricIndex());
        CHIP_ERROR err = DeviceSubscriptionManager::Instance().StartSubscription(*mController, scopedNodeId);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(NotSpecified, "Failed start subscription to NodeId:" ChipLogFormatX64, ChipLogValueX64(mNodeId));
            bridge::FabricBridge::Instance().DeviceReachableChanged(mCurrentDeviceData.id, false);
        }
    }

    MoveToState(State::Idle);
}

void DeviceSynchronizer::MoveToState(const State targetState)
{
    mState = targetState;
    ChipLogDetail(NotSpecified, "DeviceSynchronizer moving to [%10.10s]", GetStateStr());
}

const char * DeviceSynchronizer::GetStateStr() const
{
    switch (mState)
    {
    case State::Idle:
        return "Idle";

    case State::Connecting:
        return "Connecting";

    case State::AwaitingResponse:
        return "AwaitingResponse";

    case State::ReceivedResponse:
        return "ReceivedResponse";

    case State::ReceivedError:
        return "ReceivedError";

    case State::GettingUid:
        return "GettingUid";
    }
    return "N/A";
}

} // namespace admin
