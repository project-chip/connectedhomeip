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

#include "DeviceSubscription.h"
#include "rpc/RpcClient.h"

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
    reinterpret_cast<DeviceSubscription *>(context)->OnDeviceConnected(exchangeMgr, sessionHandle);
}

void OnDeviceConnectionFailureWrapper(void * context, const ScopedNodeId & peerId, CHIP_ERROR error)
{
    reinterpret_cast<DeviceSubscription *>(context)->OnDeviceConnectionFailure(peerId, error);
}

} // namespace

DeviceSubscription::DeviceSubscription() :
    mOnDeviceConnectedCallback(OnDeviceConnectedWrapper, this),
    mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureWrapper, this)
{}

void DeviceSubscription::OnAttributeData(const ConcreteDataAttributePath & path, TLV::TLVReader * data, const StatusIB & status)
{
    VerifyOrDie(path.mEndpointId == kRootEndpointId);
    VerifyOrDie(path.mClusterId == Clusters::AdministratorCommissioning::Id);

    switch (path.mAttributeId)
    {
    case Clusters::AdministratorCommissioning::Attributes::WindowStatus::Id: {
        Clusters::AdministratorCommissioning::CommissioningWindowStatusEnum windowStatus;
        CHIP_ERROR err = data->Get(windowStatus);
        VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(NotSpecified, "Failed to read WindowStatus"));
        VerifyOrReturn(windowStatus != Clusters::AdministratorCommissioning::CommissioningWindowStatusEnum::kUnknownEnumValue);
        mCurrentAdministratorCommissioningAttributes.window_status = static_cast<uint32_t>(windowStatus);
        mChangeDetected                                            = true;
        break;
    }
    case Clusters::AdministratorCommissioning::Attributes::AdminFabricIndex::Id: {
        FabricIndex fabricIndex;
        CHIP_ERROR err                                                       = data->Get(fabricIndex);
        mCurrentAdministratorCommissioningAttributes.has_opener_fabric_index = err == CHIP_NO_ERROR;
        if (mCurrentAdministratorCommissioningAttributes.has_opener_fabric_index)
        {
            mCurrentAdministratorCommissioningAttributes.opener_fabric_index = static_cast<uint32_t>(fabricIndex);
        }
        mChangeDetected = true;
        break;
    }
    case Clusters::AdministratorCommissioning::Attributes::AdminVendorId::Id: {
        chip::VendorId vendorId;
        CHIP_ERROR err                                                    = data->Get(vendorId);
        mCurrentAdministratorCommissioningAttributes.has_opener_vendor_id = err == CHIP_NO_ERROR;
        if (mCurrentAdministratorCommissioningAttributes.has_opener_vendor_id)
        {
            mCurrentAdministratorCommissioningAttributes.opener_vendor_id = static_cast<uint32_t>(vendorId);
        }
        mChangeDetected = true;
        break;
    }
    default:
        break;
    }
}

void DeviceSubscription::OnReportEnd()
{
    // Report end is at the end of all attributes (success)
    if (mChangeDetected)
    {
#if defined(PW_RPC_ENABLED)
        AdminCommissioningAttributeChanged(mCurrentAdministratorCommissioningAttributes);
#else
        ChipLogError(NotSpecified, "Cannot synchronize device with fabric bridge: RPC not enabled");
#endif
        mChangeDetected = false;
    }
}

void DeviceSubscription::OnDone(ReadClient * apReadClient)
{
    // TODO(#35077) In follow up PR we will indicate to a manager DeviceSubscription is terminal.
}

void DeviceSubscription::OnError(CHIP_ERROR error)
{
    ChipLogProgress(NotSpecified, "Error subscribing: %" CHIP_ERROR_FORMAT, error.Format());
}

void DeviceSubscription::OnDeviceConnected(Messaging::ExchangeManager & exchangeMgr, const SessionHandle & sessionHandle)
{
    mClient = std::make_unique<ReadClient>(app::InteractionModelEngine::GetInstance(), &exchangeMgr /* echangeMgr */,
                                           *this /* callback */, ReadClient::InteractionType::Subscribe);
    VerifyOrDie(mClient);

    AttributePathParams readPaths[1];
    readPaths[0] = AttributePathParams(kRootEndpointId, Clusters::AdministratorCommissioning::Id);

    ReadPrepareParams readParams(sessionHandle);

    readParams.mpAttributePathParamsList    = readPaths;
    readParams.mAttributePathParamsListSize = 1;
    readParams.mMaxIntervalCeilingSeconds   = 5 * 60;

    CHIP_ERROR err = mClient->SendRequest(readParams);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to issue subscription to AdministratorCommissioning data");
        // TODO(#35077) In follow up PR we will indicate to a manager DeviceSubscription is terminal.
    }
}

void DeviceSubscription::OnDeviceConnectionFailure(const ScopedNodeId & peerId, CHIP_ERROR error)
{
    ChipLogError(NotSpecified, "Device Sync failed to connect to " ChipLogFormatX64, ChipLogValueX64(peerId.GetNodeId()));
    // TODO(#35077) In follow up PR we will indicate to a manager DeviceSubscription is terminal.
}

void DeviceSubscription::StartSubscription(Controller::DeviceController & controller, NodeId nodeId)
{
    VerifyOrDie(!mSubscriptionStarted);

    mCurrentAdministratorCommissioningAttributes         = chip_rpc_AdministratorCommissioningChanged_init_default;
    mCurrentAdministratorCommissioningAttributes.node_id = nodeId;
    mCurrentAdministratorCommissioningAttributes.window_status =
        static_cast<uint32_t>(Clusters::AdministratorCommissioning::CommissioningWindowStatusEnum::kWindowNotOpen);
    mSubscriptionStarted = true;

    controller.GetConnectedDevice(nodeId, &mOnDeviceConnectedCallback, &mOnDeviceConnectionFailureCallback);
}
