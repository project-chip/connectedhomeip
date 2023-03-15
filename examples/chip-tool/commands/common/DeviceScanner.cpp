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

using namespace chip;
using namespace chip::Ble;
using namespace chip::Dnssd;

CHIP_ERROR DeviceScanner::Start()
{
    mDiscoveredResults.clear();

#if CHIP_TOOL_DEVICE_SCANNER_USE_BLE
    ReturnErrorOnFailure(DeviceLayer::PlatformMgrImpl().StartBleScan(this));
#endif // CHIP_TOOL_DEVICE_SCANNER_USE_BLE

    ReturnErrorOnFailure(mDNSResolver.Init(DeviceLayer::UDPEndPointManager()));
    mDNSResolver.SetCommissioningDelegate(this);

    DiscoveryFilter filter(DiscoveryFilterType::kNone, (uint64_t) 0);
    return mDNSResolver.DiscoverCommissionableNodes(filter);
}

CHIP_ERROR DeviceScanner::Stop()
{
#if CHIP_TOOL_DEVICE_SCANNER_USE_BLE
    ReturnErrorOnFailure(DeviceLayer::PlatformMgrImpl().StopBleScan());
#endif // CHIP_TOOL_DEVICE_SCANNER_USE_BLE

    mDNSResolver.SetCommissioningDelegate(nullptr);
    ReturnErrorOnFailure(mDNSResolver.StopDiscovery());
    mDNSResolver.Shutdown();
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
    for (size_t i = 0; i < resolutionData.numIPs; i++)
    {
        auto params                = Controller::SetUpCodePairerParameters(resolutionData, i);
        DeviceScannerResult result = { params, vendorId, productId, discriminator };
        mDiscoveredResults.push_back(result);
    }

    nodeData.LogDetail();
}

#if CHIP_TOOL_DEVICE_SCANNER_USE_BLE
void DeviceScanner::OnBleScanResult(BLE_CONNECTION_OBJECT connObj, const ChipBLEDeviceIdentificationInfo & info)
{
    auto discriminator = info.GetDeviceDiscriminator();
    auto vendorId      = static_cast<VendorId>(info.GetVendorId());
    auto productId     = info.GetProductId();

    ChipLogProgress(chipTool, "OnNodeDiscovered (BLE): discriminator: %u, vendorId: %u, productId: %u", discriminator, vendorId,
                    productId);

    auto params                = Controller::SetUpCodePairerParameters(connObj, false /* connected */);
    DeviceScannerResult result = { params, vendorId, productId, discriminator };
    mDiscoveredResults.push_back(result);
}
#endif // CHIP_TOOL_DEVICE_SCANNER_USE_BLE

CHIP_ERROR DeviceScanner::Get(uint16_t index, RendezvousParameters & params)
{
    VerifyOrReturnError(index < mDiscoveredResults.size(), CHIP_ERROR_NOT_FOUND);

    auto & result = mDiscoveredResults.at(index);
    params        = result.mParams;
    return CHIP_NO_ERROR;
}

void DeviceScanner::Log() const
{
    auto resultsCount = mDiscoveredResults.size();
    VerifyOrReturn(resultsCount > 0, ChipLogProgress(chipTool, "No device discovered."));

    uint16_t index = 0;
    for (auto & result : mDiscoveredResults)
    {
        char addr[Transport::PeerAddress::kMaxToStringSize];
        result.mParams.GetPeerAddress().ToString(addr);

        ChipLogProgress(chipTool, "\t %u - Discriminator: %u - Vendor: %u - Product: %u - %s", index, result.mDiscriminator,
                        result.mVendorId, result.mProductId, addr);
        index++;
    }
}

DeviceScanner & GetDeviceScanner()
{
    static DeviceScanner scanner;
    return scanner;
}
