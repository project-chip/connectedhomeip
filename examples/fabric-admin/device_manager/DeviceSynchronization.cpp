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

#include "DeviceSubscriptionManager.h"

#if defined(PW_RPC_ENABLED)
#include "rpc/RpcClient.h"
#endif

#include <app/InteractionModelEngine.h>
#include <app/server/Server.h>

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <device_manager/DeviceManager.h>

using namespace ::chip;
using namespace ::chip::app;
using chip::app::ReadClient;

namespace {

void OnDeviceConnectedWrapper(void * context, Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle)
{
    reinterpret_cast<DeviceSynchronizer *>(context)->OnDeviceConnected(exchangeMgr, sessionHandle);
}

void OnDeviceConnectionFailureWrapper(void * context, const ScopedNodeId & peerId, CHIP_ERROR error)
{
    reinterpret_cast<DeviceSynchronizer *>(context)->OnDeviceConnectionFailure(peerId, error);
}

#if defined(PW_RPC_ENABLED)
bool SuccessOrLog(CHIP_ERROR err, const char * name)
{
    if (err == CHIP_NO_ERROR)
    {
        return true;
    }

    ChipLogError(NotSpecified, "Failed to read %s: %" CHIP_ERROR_FORMAT, name, err.Format());

    return false;
}
#endif

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
#if defined(PW_RPC_ENABLED)
    case Clusters::BasicInformation::Attributes::UniqueID::Id:
        mCurrentDeviceData.has_unique_id =
            SuccessOrLog(data->GetString(mCurrentDeviceData.unique_id, sizeof(mCurrentDeviceData.unique_id)), "UniqueId");
        break;
    case Clusters::BasicInformation::Attributes::VendorName::Id:
        mCurrentDeviceData.has_vendor_name =
            SuccessOrLog(data->GetString(mCurrentDeviceData.vendor_name, sizeof(mCurrentDeviceData.vendor_name)), "VendorName");
        break;
    case Clusters::BasicInformation::Attributes::VendorID::Id:
        mCurrentDeviceData.has_vendor_id = SuccessOrLog(data->Get(mCurrentDeviceData.vendor_id), "VendorID");
        break;
    case Clusters::BasicInformation::Attributes::ProductName::Id:
        mCurrentDeviceData.has_product_name =
            SuccessOrLog(data->GetString(mCurrentDeviceData.product_name, sizeof(mCurrentDeviceData.product_name)), "ProductName");
        break;
    case Clusters::BasicInformation::Attributes::ProductID::Id:
        mCurrentDeviceData.has_product_id = SuccessOrLog(data->Get(mCurrentDeviceData.product_id), "ProductID");
        break;
    case Clusters::BasicInformation::Attributes::NodeLabel::Id:
        mCurrentDeviceData.has_node_label =
            SuccessOrLog(data->GetString(mCurrentDeviceData.node_label, sizeof(mCurrentDeviceData.node_label)), "NodeLabel");
        break;
    case Clusters::BasicInformation::Attributes::HardwareVersion::Id:
        mCurrentDeviceData.has_hardware_version = SuccessOrLog(data->Get(mCurrentDeviceData.hardware_version), "HardwareVersion");
        break;
    case Clusters::BasicInformation::Attributes::HardwareVersionString::Id:
        mCurrentDeviceData.has_hardware_version_string = SuccessOrLog(
            data->GetString(mCurrentDeviceData.hardware_version_string, sizeof(mCurrentDeviceData.hardware_version_string)),
            "HardwareVersionString");
        break;
    case Clusters::BasicInformation::Attributes::SoftwareVersion::Id:
        mCurrentDeviceData.has_software_version = SuccessOrLog(data->Get(mCurrentDeviceData.software_version), "HardwareVersion");
        break;
    case Clusters::BasicInformation::Attributes::SoftwareVersionString::Id:
        mCurrentDeviceData.has_software_version_string = SuccessOrLog(
            data->GetString(mCurrentDeviceData.software_version_string, sizeof(mCurrentDeviceData.software_version_string)),
            "SoftwareVersionString");
        break;
#endif // #if defined(PW_RPC_ENABLED)
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
#if defined(PW_RPC_ENABLED)
    if (mState == State::ReceivedResponse && !DeviceMgr().IsCurrentBridgeDevice(mNodeId))
    {
        GetUniqueId();
        if (mState == State::GettingUid)
        {
            // GetUniqueId was successful and we rely on callback to call SynchronizationCompleteAddDevice.
            return;
        }
        SynchronizationCompleteAddDevice();
    }
#else
    ChipLogError(NotSpecified, "Cannot synchronize device with fabric bridge: RPC not enabled");
#endif
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

#if defined(PW_RPC_ENABLED)
    mCurrentDeviceData                 = chip_rpc_SynchronizedDevice_init_default;
    mCurrentDeviceData.has_id          = true;
    mCurrentDeviceData.id.node_id      = nodeId;
    mCurrentDeviceData.id.fabric_index = controller->GetFabricIndex();
    mCurrentDeviceData.has_is_icd      = true;
    mCurrentDeviceData.is_icd          = deviceIsIcd;
#endif

    ReturnOnFailure(controller->GetConnectedDevice(nodeId, &mOnDeviceConnectedCallback, &mOnDeviceConnectionFailureCallback));
    mController = controller;
    MoveToState(State::Connecting);
}

void DeviceSynchronizer::GetUniqueId()
{
    VerifyOrDie(mState == State::ReceivedResponse);
    VerifyOrDie(mController);

#if defined(PW_RPC_ENABLED)
    // If we have a UniqueId we can return leaving state in ReceivedResponse.
    VerifyOrReturn(!mCurrentDeviceData.has_unique_id, ChipLogDetail(NotSpecified, "We already have UniqueId"));
#endif

    auto * device = DeviceMgr().FindDeviceByNode(mNodeId);
    // If there is no associated remote Fabric Sync Aggregator there is no other place for us to try
    // getting the UniqueId from and can return leaving the state in ReceivedResponse.
    VerifyOrReturn(device, ChipLogDetail(NotSpecified, "No remote Fabric Sync Aggregator to get UniqueId from"));

    // Because device is not-null we expect IsFabricSyncReady to be true. IsFabricSyncReady indicates we have a
    // connection to the remote Fabric Sync Aggregator.
    VerifyOrDie(DeviceMgr().IsFabricSyncReady());
    auto remoteBridgeNodeId               = DeviceMgr().GetRemoteBridgeNodeId();
    EndpointId remoteEndpointIdOfInterest = device->GetEndpointId();

    ChipLogDetail(NotSpecified, "Attempting to get UniqueId from remote Fabric Sync Aggregator") CHIP_ERROR err =
        mUniqueIdGetter.GetUniqueId(
            [this](std::optional<CharSpan> aUniqueId) {
                if (aUniqueId.has_value())
                {
#if defined(PW_RPC_ENABLED)
                    this->mCurrentDeviceData.has_unique_id = true;
                    memcpy(this->mCurrentDeviceData.unique_id, aUniqueId.value().data(), aUniqueId.value().size());
#endif
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
        ChipLogDetail(NotSpecified, "Failed to get UniqueId from remote Fabric Sync Aggregator")
    }
}

void DeviceSynchronizer::SynchronizationCompleteAddDevice()
{
    VerifyOrDie(mState == State::ReceivedResponse || mState == State::GettingUid);

#if defined(PW_RPC_ENABLED)
    AddSynchronizedDevice(mCurrentDeviceData);
    // TODO(#35077) Figure out how we should reflect CADMIN values of ICD.
    if (!mCurrentDeviceData.is_icd)
    {
        VerifyOrDie(mController);
        // TODO(#35333) Figure out how we should recover in this circumstance.
        CHIP_ERROR err = DeviceSubscriptionManager::Instance().StartSubscription(*mController, mNodeId);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(NotSpecified, "Failed start subscription to NodeId:" ChipLogFormatX64, ChipLogValueX64(mNodeId));
        }
    }
#endif

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
