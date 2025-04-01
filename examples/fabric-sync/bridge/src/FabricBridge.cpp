/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "FabricBridge.h"
#include "BridgedDevice.h"
#include "BridgedDeviceManager.h"

#include <app/clusters/ecosystem-information-server/ecosystem-information-server.h>
#include <lib/core/CHIPError.h>

#include <string>
#include <thread>

using namespace chip;

namespace bridge {

FabricBridge FabricBridge::sInstance;

FabricBridge & FabricBridge::Instance()
{
    return sInstance;
}

CHIP_ERROR FabricBridge::AddSynchronizedDevice(const SynchronizedDevice & data)
{
    ChipLogProgress(NotSpecified, "Received AddSynchronizedDevice: Id=[%d:" ChipLogFormatX64 "]", data.id.GetFabricIndex(),
                    ChipLogValueX64(data.id.GetNodeId()));

    // Create a new BridgedDevice and set it as reachable
    auto device = std::make_unique<BridgedDevice>(data.id);
    device->SetReachable(true);

    // Initialize BridgedDevice attributes from data
    BridgedDevice::BridgedAttributes attributes;

    if (data.uniqueId.has_value())
    {
        attributes.uniqueId = data.uniqueId.value();
    }

    if (data.vendorName.has_value())
    {
        attributes.vendorName = data.vendorName.value();
    }

    if (data.vendorId.has_value())
    {
        attributes.vendorId = data.vendorId.value();
    }

    if (data.productName.has_value())
    {
        attributes.productName = data.productName.value();
    }

    if (data.productId.has_value())
    {
        attributes.productId = data.productId.value();
    }

    if (data.nodeLabel.has_value())
    {
        attributes.nodeLabel = data.nodeLabel.value();
    }

    if (data.hardwareVersion.has_value())
    {
        attributes.hardwareVersion = data.hardwareVersion.value();
    }

    if (data.hardwareVersionString.has_value())
    {
        attributes.hardwareVersionString = data.hardwareVersionString.value();
    }

    if (data.softwareVersion.has_value())
    {
        attributes.softwareVersion = data.softwareVersion.value();
    }

    if (data.softwareVersionString.has_value())
    {
        attributes.softwareVersionString = data.softwareVersionString.value();
    }

    // Set bridged device attributes and ICD status
    device->SetBridgedAttributes(attributes);
    device->SetIcd(data.isIcd.value_or(false));

    // Add the device to the bridge manager with a parent endpoint
    auto result = BridgedDeviceManager::Instance().AddDeviceEndpoint(std::move(device), /* parentEndpointId= */ 1);
    if (!result.has_value())
    {
        ChipLogError(NotSpecified, "Failed to add device with Id=[%d:0x" ChipLogFormatX64 "]", data.id.GetFabricIndex(),
                     ChipLogValueX64(data.id.GetNodeId()));
        return CHIP_ERROR_ENDPOINT_POOL_FULL;
    }

    // Retrieve and verify the added device by ScopedNodeId
    BridgedDevice * addedDevice = BridgedDeviceManager::Instance().GetDeviceByScopedNodeId(data.id);
    VerifyOrDie(addedDevice);

    ChipLogProgress(NotSpecified, "Added device with Id=[%d:0x" ChipLogFormatX64 "]", data.id.GetFabricIndex(),
                    ChipLogValueX64(data.id.GetNodeId()));

    // Add the Ecosystem Information Cluster to the device's endpoint
    CHIP_ERROR err =
        app::Clusters::EcosystemInformation::EcosystemInformationServer::Instance().AddEcosystemInformationClusterToEndpoint(
            addedDevice->GetEndpointId());

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "Failed to add Ecosystem Information Cluster to endpoint %u: %" CHIP_ERROR_FORMAT,
                     addedDevice->GetEndpointId(), err.Format());
    }

    return err;
}

CHIP_ERROR FabricBridge::RemoveSynchronizedDevice(ScopedNodeId scopedNodeId)
{
    ChipLogProgress(NotSpecified, "Received RemoveSynchronizedDevice: Id=[%d:" ChipLogFormatX64 "]", scopedNodeId.GetFabricIndex(),
                    ChipLogValueX64(scopedNodeId.GetNodeId()));

    auto removedIdx = BridgedDeviceManager::Instance().RemoveDeviceByScopedNodeId(scopedNodeId);
    if (!removedIdx.has_value())
    {
        ChipLogError(NotSpecified, "Failed to remove device with Id=[%d:0x" ChipLogFormatX64 "]", scopedNodeId.GetFabricIndex(),
                     ChipLogValueX64(scopedNodeId.GetNodeId()));
        return CHIP_ERROR_NOT_FOUND;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR FabricBridge::ActiveChanged(ScopedNodeId scopedNodeId, uint32_t promisedActiveDurationMs)
{
    ChipLogProgress(NotSpecified, "Received ActiveChanged: Id=[%d:" ChipLogFormatX64 "]", scopedNodeId.GetFabricIndex(),
                    ChipLogValueX64(scopedNodeId.GetNodeId()));

    auto * device = BridgedDeviceManager::Instance().GetDeviceByScopedNodeId(scopedNodeId);
    if (device == nullptr)
    {
        ChipLogError(NotSpecified, "Could not find bridged device associated with Id=[%d:0x" ChipLogFormatX64 "]",
                     scopedNodeId.GetFabricIndex(), ChipLogValueX64(scopedNodeId.GetNodeId()));
        return CHIP_ERROR_NOT_FOUND;
    }

    device->LogActiveChangeEvent(promisedActiveDurationMs);
    return CHIP_NO_ERROR;
}

CHIP_ERROR FabricBridge::AdminCommissioningAttributeChanged(const AdministratorCommissioningChanged & data)
{
    ChipLogProgress(NotSpecified, "Received CADMIN attribute change: Id=[%d:" ChipLogFormatX64 "]", data.id.GetFabricIndex(),
                    ChipLogValueX64(data.id.GetNodeId()));

    auto * device = BridgedDeviceManager::Instance().GetDeviceByScopedNodeId(data.id);
    if (device == nullptr)
    {
        ChipLogError(NotSpecified, "Could not find bridged device associated with Id=[%d:0x" ChipLogFormatX64 "]",
                     data.id.GetFabricIndex(), ChipLogValueX64(data.id.GetNodeId()));
        return CHIP_ERROR_NOT_FOUND;
    }

    BridgedDevice::AdminCommissioningAttributes adminCommissioningAttributes;

    VerifyOrReturnError(data.windowStatus <
                            app::Clusters::AdministratorCommissioning::CommissioningWindowStatusEnum::kUnknownEnumValue,
                        CHIP_ERROR_INVALID_ARGUMENT);

    adminCommissioningAttributes.commissioningWindowStatus = data.windowStatus;
    if (data.openerFabricIndex.has_value())
    {
        VerifyOrReturnError(data.openerFabricIndex >= kMinValidFabricIndex, CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrReturnError(data.openerFabricIndex <= kMaxValidFabricIndex, CHIP_ERROR_INVALID_ARGUMENT);
        adminCommissioningAttributes.openerFabricIndex = data.openerFabricIndex;
    }

    if (data.openerVendorId.has_value())
    {
        VerifyOrReturnError(data.openerVendorId != VendorId::NotSpecified, CHIP_ERROR_INVALID_ARGUMENT);
        adminCommissioningAttributes.openerVendorId = data.openerVendorId;
    }

    device->SetAdminCommissioningAttributes(adminCommissioningAttributes);
    return CHIP_NO_ERROR;
}

CHIP_ERROR FabricBridge::DeviceReachableChanged(ScopedNodeId scopedNodeId, bool reachability)
{
    ChipLogProgress(NotSpecified, "Received device reachable changed: Id=[%d:" ChipLogFormatX64 "]", scopedNodeId.GetFabricIndex(),
                    ChipLogValueX64(scopedNodeId.GetNodeId()));

    auto * device = BridgedDeviceManager::Instance().GetDeviceByScopedNodeId(scopedNodeId);
    if (device == nullptr)
    {
        ChipLogError(NotSpecified, "Could not find bridged device associated with Id=[%d:0x" ChipLogFormatX64 "]",
                     scopedNodeId.GetFabricIndex(), ChipLogValueX64(scopedNodeId.GetNodeId()));
        return CHIP_ERROR_NOT_FOUND;
    }

    device->ReachableChanged(reachability);

    return CHIP_NO_ERROR;
}

} // namespace bridge
