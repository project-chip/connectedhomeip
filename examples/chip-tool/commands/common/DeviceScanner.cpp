/*
 *   Copyright (c) 2023 Project CHIP Authors
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

#include "DeviceScanner.h"

#include <lib/dnssd/ServiceNaming.h> // For MakeInstanceName

using namespace chip;
using namespace chip::Dnssd;

#if CHIP_DEVICE_LAYER_TARGET_DARWIN
#if CONFIG_NETWORK_LAYER_BLE
using namespace chip::Ble;
constexpr const char * kBleKey = "BLE";
#endif // CONFIG_NETWORK_LAYER_BLE
#endif // CHIP_DEVICE_LAYER_TARGET_DARWIN

CHIP_ERROR DeviceScanner::StartInternal()
{
    VerifyOrReturnError(mIsBrowsing == false, CHIP_ERROR_INCORRECT_STATE);

    mDiscoveredResults.clear();

#if CHIP_DEVICE_LAYER_TARGET_DARWIN
#if CONFIG_NETWORK_LAYER_BLE
    ReturnErrorOnFailure(DeviceLayer::PlatformMgrImpl().StartBleScan(this));
#endif // CONFIG_NETWORK_LAYER_BLE
#endif // CHIP_DEVICE_LAYER_TARGET_DARWIN

    ReturnLogErrorOnFailure(mDNSResolver.Init(chip::DeviceLayer::UDPEndPointManager()));
    mDNSResolver.SetBrowseDelegate(this);
    mDNSResolver.SetCommissioningDelegate(this);
    mDNSResolver.SetOperationalDelegate(this);

    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceScanner::StartCommissionableDiscovery()
{
    ReturnErrorOnFailure(StartInternal());
    ReturnErrorOnFailure(mDNSResolver.StartBrowse());
    mIsBrowsing = true;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceScanner::StartOperationalDiscovery(chip::Optional<uint64_t> compressedFabricIdFilter)
{
    ReturnErrorOnFailure(StartInternal());
    ReturnErrorOnFailure(mDNSResolver.StartBrowse(compressedFabricIdFilter));
    mIsBrowsing = true;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DeviceScanner::Stop()
{
#if CHIP_DEVICE_LAYER_TARGET_DARWIN
#if CONFIG_NETWORK_LAYER_BLE
    ReturnErrorOnFailure(DeviceLayer::PlatformMgrImpl().StopBleScan());
#endif // CONFIG_NETWORK_LAYER_BLE
#endif // CHIP_DEVICE_LAYER_TARGET_DARWIN

    ReturnErrorOnFailure(mDNSResolver.StopBrowse());
    mDNSResolver.Shutdown();

    mIsBrowsing = false;
    return CHIP_NO_ERROR;
}

void DeviceScanner::OnNodeDiscovered(const DiscoveredNodeData & nodeData)
{
    auto & commissionData = nodeData.commissionData;

    auto discriminator = commissionData.longDiscriminator;
    auto vendorId      = static_cast<VendorId>(commissionData.vendorId);
    auto productId     = commissionData.productId;

    ChipLogProgress(chipTool, "OnNodeDiscovered (MDNS): discriminator: %u, vendorId: %u, productId: %u", discriminator, vendorId,
                    productId);

    auto & resolutionData = nodeData.resolutionData;

    auto & instanceData  = mDiscoveredResults[commissionData.instanceName];
    auto & interfaceData = instanceData[resolutionData.interfaceId.GetPlatformInterface()];

    for (size_t i = 0; i < resolutionData.numIPs; i++)
    {
        auto params                = Controller::SetUpCodePairerParameters(resolutionData, i);
        DeviceScannerResult result = { params, vendorId, productId, discriminator, chip::MakeOptional(resolutionData) };
        interfaceData.push_back(result);
    }

    nodeData.LogDetail();
}

void DeviceScanner::OnOperationalNodeResolved(const ResolvedNodeData & nodeData)
{
    auto & operationalData = nodeData.operationalData;
    ChipLogProgress(chipTool, "OnOperationalNodeResolved peerId: " ChipLogFormatX64 ":" ChipLogFormatX64,
                    ChipLogValueX64(operationalData.peerId.GetCompressedFabricId()),
                    ChipLogValueX64(operationalData.peerId.GetNodeId()));
    mDNSResolver.NodeIdResolutionNoLongerNeeded(operationalData.peerId);

    auto & resolutionData = nodeData.resolutionData;

    char instanceName[Common::kInstanceNameMaxLength + 1];
    ReturnOnFailure(MakeInstanceName(instanceName, sizeof(instanceName), operationalData.peerId));
    auto & instanceData  = mDiscoveredResults[instanceName];
    auto & interfaceData = instanceData[resolutionData.interfaceId.GetPlatformInterface()];

    for (size_t i = 0; i < resolutionData.numIPs; i++)
    {
        DeviceScannerResult result;
        result.isOperational = true;
        interfaceData.push_back(result);
    }
}

void DeviceScanner::OnOperationalNodeResolutionFailed(const PeerId & peerId, CHIP_ERROR error)
{
    mDNSResolver.NodeIdResolutionNoLongerNeeded(peerId);

    char instanceName[Common::kInstanceNameMaxLength + 1];
    ReturnOnFailure(MakeInstanceName(instanceName, sizeof(instanceName), peerId));
    mDiscoveredResults.erase(instanceName);
}

void DeviceScanner::OnBrowseAdd(const NodeBrowseData & nodeData)
{
    ChipLogError(chipTool, "OnBrowseAdd: %s", nodeData.mName);
    auto error = mDNSResolver.ResolveNode(nodeData);
    LogErrorOnFailure(error);
    ReturnOnFailure(error);

    auto & instanceData  = mDiscoveredResults[nodeData.mName];
    auto & interfaceData = instanceData[nodeData.mInterfaceId.GetPlatformInterface()];
    (void) interfaceData;
}

void DeviceScanner::OnBrowseRemove(const NodeBrowseData & nodeData)
{
    ChipLogError(chipTool, "OnBrowseRemove: %s", nodeData.mName);

    auto & instanceData  = mDiscoveredResults[nodeData.mName];
    auto & interfaceData = instanceData[nodeData.mInterfaceId.GetPlatformInterface()];

    // Check if the interface data has been resolved already, otherwise, just inform the
    // back end that we may not need it anymore.
    if (interfaceData.size() == 0)
    {
        mDNSResolver.NodeNameResolutionNoLongerNeeded(nodeData.mName);
    }

    // Delete the interface placeholder.
    instanceData.erase(nodeData.mInterfaceId.GetPlatformInterface());

    // If there is nothing else to resolve for the given instance name, just remove it
    // too.
    if (instanceData.size() == 0)
    {
        mDiscoveredResults.erase(nodeData.mName);
    }
}

void DeviceScanner::OnBrowseStop(CHIP_ERROR error)
{
    ChipLogProgress(chipTool, "OnBrowseStop: %" CHIP_ERROR_FORMAT, error.Format());

    for (auto & instance : mDiscoveredResults)
    {
        for (auto & interface : instance.second)
        {
            if (interface.second.size() == 0)
            {
                mDNSResolver.NodeNameResolutionNoLongerNeeded(instance.first.c_str());
            }
        }
    }
}

#if CHIP_DEVICE_LAYER_TARGET_DARWIN
#if CONFIG_NETWORK_LAYER_BLE
void DeviceScanner::OnBleScanAdd(BLE_CONNECTION_OBJECT connObj, const ChipBLEDeviceIdentificationInfo & info)
{
    auto discriminator = info.GetDeviceDiscriminator();
    auto vendorId      = static_cast<VendorId>(info.GetVendorId());
    auto productId     = info.GetProductId();

    ChipLogProgress(chipTool, "OnBleScanAdd (BLE): %p, discriminator: %u, vendorId: %u, productId: %u", connObj, discriminator,
                    vendorId, productId);

    auto params                = Controller::SetUpCodePairerParameters(connObj, false /* connected */);
    DeviceScannerResult result = { params, vendorId, productId, discriminator };

    auto & instanceData  = mDiscoveredResults[kBleKey];
    auto & interfaceData = instanceData[chip::Inet::InterfaceId::Null().GetPlatformInterface()];
    interfaceData.push_back(result);
}

void DeviceScanner::OnBleScanRemove(BLE_CONNECTION_OBJECT connObj)
{
    ChipLogProgress(chipTool, "OnBleScanRemove: %p", connObj);

    auto & instanceData  = mDiscoveredResults[kBleKey];
    auto & interfaceData = instanceData[chip::Inet::InterfaceId::Null().GetPlatformInterface()];

    interfaceData.erase(std::remove_if(interfaceData.begin(), interfaceData.end(),
                                       [connObj](const DeviceScannerResult & result) {
                                           return result.mParams.HasDiscoveredObject() &&
                                               result.mParams.GetDiscoveredObject() == connObj;
                                       }),
                        interfaceData.end());

    if (interfaceData.size() == 0)
    {
        instanceData.clear();
        mDiscoveredResults.erase(kBleKey);
    }
}
#endif // CONFIG_NETWORK_LAYER_BLE
#endif // CHIP_DEVICE_LAYER_TARGET_DARWIN

CHIP_ERROR DeviceScanner::Get(uint16_t index, RendezvousParameters & params)
{
    uint16_t currentIndex = 0;
    for (auto & instance : mDiscoveredResults)
    {
        for (auto & interface : instance.second)
        {
            for (auto & result : interface.second)
            {
                if (currentIndex == index)
                {
                    params = result.mParams;
                    return CHIP_NO_ERROR;
                }
                currentIndex++;
            }
        }
    }

    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR DeviceScanner::Get(uint16_t index, Dnssd::CommonResolutionData & resolutionData)
{
    uint16_t currentIndex = 0;
    for (auto & instance : mDiscoveredResults)
    {
        for (auto & interface : instance.second)
        {
            for (auto & result : interface.second)
            {
                if (currentIndex == index && result.mResolutionData.HasValue())
                {
                    resolutionData = result.mResolutionData.Value();
                    return CHIP_NO_ERROR;
                }
                currentIndex++;
            }
        }
    }

    return CHIP_ERROR_NOT_FOUND;
}

void DeviceScanner::Log() const
{
    auto resultsCount = mDiscoveredResults.size();
    VerifyOrReturn(resultsCount > 0, ChipLogProgress(chipTool, "No device discovered."));

    uint16_t index = 0;
    for (auto & instance : mDiscoveredResults)
    {
        ChipLogProgress(chipTool, "Instance Name: %s ", instance.first.c_str());
        for (auto & interface : instance.second)
        {
            for (auto & result : interface.second)
            {
                // TODO This is not pretty. Instead of having a single list, it should likely be 2 lists. One for commissionable
                //      nodes and one for operational nodes.
                if (result.isOperational)
                {
                    continue;
                }
                char addr[Transport::PeerAddress::kMaxToStringSize];
                result.mParams.GetPeerAddress().ToString(addr);

                ChipLogProgress(chipTool, "\t %u - Discriminator: %u - Vendor: %u - Product: %u - %s", index, result.mDiscriminator,
                                result.mVendorId, result.mProductId, addr);
                index++;
            }
        }
    }
}

DeviceScanner & GetDeviceScanner()
{
    static DeviceScanner scanner;
    return scanner;
}
