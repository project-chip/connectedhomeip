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
#include "rpc/RpcClient.h"

#include <app/InteractionModelEngine.h>
#include <app/server/Server.h>

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>

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

    switch (path.mAttributeId)
    {
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
    default:
        break;
    }
}

void DeviceSynchronizer::OnReportEnd()
{
    // Report end is at the end of all attributes (success)
#if defined(PW_RPC_ENABLED)
    AddSynchronizedDevice(mCurrentDeviceData);
#else
    ChipLogError(NotSpecified, "Cannot synchronize device with fabric bridge: RPC not enabled");
#endif
}

void DeviceSynchronizer::OnDone(chip::app::ReadClient * apReadClient)
{
    // Nothing to do: error reported on OnError or report ended called.
    mDeviceSyncInProcess = false;
}

void DeviceSynchronizer::OnError(CHIP_ERROR error)
{
    ChipLogProgress(NotSpecified, "Error fetching device data: %" CHIP_ERROR_FORMAT, error.Format());
}

void DeviceSynchronizer::OnDeviceConnected(chip::Messaging::ExchangeManager & exchangeMgr,
                                           const chip::SessionHandle & sessionHandle)
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
        mDeviceSyncInProcess = false;
    }
}

void DeviceSynchronizer::OnDeviceConnectionFailure(const chip::ScopedNodeId & peerId, CHIP_ERROR error)
{
    ChipLogError(NotSpecified, "Device Sync failed to connect to " ChipLogFormatX64, ChipLogValueX64(peerId.GetNodeId()));
    mDeviceSyncInProcess = false;
}

void DeviceSynchronizer::StartDeviceSynchronization(chip::Controller::DeviceController & controller, chip::NodeId nodeId,
                                                    bool deviceIsIcd)
{
    if (mDeviceSyncInProcess)
    {
        ChipLogError(NotSpecified, "Device Sync NOT POSSIBLE: another sync is in progress");
        return;
    }

    mCurrentDeviceData            = chip_rpc_SynchronizedDevice_init_default;
    mCurrentDeviceData.node_id    = nodeId;
    mCurrentDeviceData.has_is_icd = true;
    mCurrentDeviceData.is_icd     = deviceIsIcd;

    mDeviceSyncInProcess = true;

    controller.GetConnectedDevice(nodeId, &mOnDeviceConnectedCallback, &mOnDeviceConnectionFailureCallback);
}
