/*
 *   Copyright (c) 2025 Project CHIP Authors
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
using namespace chip::Dnssd;

#if CONFIG_NETWORK_LAYER_BLE
using namespace chip::Ble;
constexpr char kBleKey[] = "BLE";
#endif // CONFIG_NETWORK_LAYER_BLE

CHIP_ERROR DeviceScanner::Start()
{
    mDiscoveredResults.clear();

#if CONFIG_NETWORK_LAYER_BLE
    ReturnErrorOnFailure(DeviceLayer::PlatformMgrImpl().StartBleScan(this));
#endif // CONFIG_NETWORK_LAYER_BLE

    ReturnErrorOnFailure(chip::Dnssd::Resolver::Instance().Init(DeviceLayer::UDPEndPointManager()));

    char serviceName[kMaxCommissionableServiceNameSize];
    auto filter = DiscoveryFilterType::kNone;
    ReturnErrorOnFailure(MakeServiceTypeName(serviceName, sizeof(serviceName), filter, DiscoveryType::kCommissionableNode));

    return ChipDnssdBrowse(serviceName, DnssdServiceProtocol::kDnssdProtocolUdp, Inet::IPAddressType::kAny,
                           Inet::InterfaceId::Null(), this);
}

CHIP_ERROR DeviceScanner::Stop()
{
#if CONFIG_NETWORK_LAYER_BLE
    ReturnErrorOnFailure(DeviceLayer::PlatformMgrImpl().StopBleScan());
#endif // CONFIG_NETWORK_LAYER_BLE

    return ChipDnssdStopBrowse(this);
}

void DeviceScanner::OnNodeDiscovered(const DiscoveredNodeData & nodeData)
{
    VerifyOrReturn(nodeData.Is<CommissionNodeData>());
    auto & commissionData = nodeData.Get<CommissionNodeData>();

    auto discriminator = commissionData.longDiscriminator;
    auto vendorId      = static_cast<VendorId>(commissionData.vendorId);
    auto productId     = commissionData.productId;

    ChipLogProgress(NotSpecified, "OnNodeDiscovered (MDNS): discriminator: %u, vendorId: %u, productId: %u", discriminator,
                    vendorId, productId);

    const CommonResolutionData & resolutionData = commissionData;

    auto & instanceData  = mDiscoveredResults[commissionData.instanceName];
    auto & interfaceData = instanceData[resolutionData.interfaceId.GetPlatformInterface()];

    for (size_t i = 0; i < resolutionData.numIPs; i++)
    {
        auto params                = Controller::SetUpCodePairerParameters(resolutionData, i);
        DeviceScannerResult result = { params, vendorId, productId, discriminator, chip::MakeOptional(resolutionData) };
        interfaceData.push_back(result);
    }

    commissionData.LogDetail();
}

void DeviceScanner::OnBrowseAdd(chip::Dnssd::DnssdService service)
{
    ChipLogProgress(NotSpecified, "OnBrowseAdd: %s", service.mName);
    LogErrorOnFailure(ChipDnssdResolve(&service, service.mInterface, this));

    auto & instanceData  = mDiscoveredResults[service.mName];
    auto & interfaceData = instanceData[service.mInterface.GetPlatformInterface()];
    (void) interfaceData;
}

void DeviceScanner::OnBrowseRemove(chip::Dnssd::DnssdService service)
{
    ChipLogProgress(NotSpecified, "OnBrowseRemove: %s", service.mName);
    auto & instanceData  = mDiscoveredResults[service.mName];
    auto & interfaceData = instanceData[service.mInterface.GetPlatformInterface()];

    // Check if the interface data has been resolved already, otherwise, just inform the
    // back end that we may not need it anymore.
    if (interfaceData.size() == 0)
    {
        ChipDnssdResolveNoLongerNeeded(service.mName);
    }

    // Delete the interface placeholder.
    instanceData.erase(service.mInterface.GetPlatformInterface());

    // If there is nothing else to resolve for the given instance name, just remove it
    // too.
    if (instanceData.size() == 0)
    {
        mDiscoveredResults.erase(service.mName);
    }
}

void DeviceScanner::OnBrowseStop(CHIP_ERROR error)
{
    ChipLogProgress(NotSpecified, "OnBrowseStop: %" CHIP_ERROR_FORMAT, error.Format());

    for (auto & instance : mDiscoveredResults)
    {
        for (auto & interface : instance.second)
        {
            if (interface.second.size() == 0)
            {
                ChipDnssdResolveNoLongerNeeded(instance.first.c_str());
            }
        }
    }
}

#if CONFIG_NETWORK_LAYER_BLE
void DeviceScanner::OnBleScanAdd(BLE_CONNECTION_OBJECT connObj, const ChipBLEDeviceIdentificationInfo & info)
{
    auto discriminator = info.GetDeviceDiscriminator();
    auto vendorId      = static_cast<VendorId>(info.GetVendorId());
    auto productId     = info.GetProductId();

    ChipLogProgress(NotSpecified, "OnBleScanAdd (BLE): %p, discriminator: %u, vendorId: %u, productId: %u", connObj, discriminator,
                    vendorId, productId);

    auto params                = Controller::SetUpCodePairerParameters(connObj, false /* connected */);
    DeviceScannerResult result = { params, vendorId, productId, discriminator };

    auto & instanceData  = mDiscoveredResults[kBleKey];
    auto & interfaceData = instanceData[chip::Inet::InterfaceId::Null().GetPlatformInterface()];
    interfaceData.push_back(result);
}

void DeviceScanner::OnBleScanRemove(BLE_CONNECTION_OBJECT connObj)
{
    ChipLogProgress(NotSpecified, "OnBleScanRemove: %p", connObj);

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
    VerifyOrReturn(resultsCount > 0, ChipLogProgress(NotSpecified, "No device discovered."));

    [[maybe_unused]] uint16_t index = 0;
    for (auto & instance : mDiscoveredResults)
    {
        ChipLogProgress(NotSpecified, "Instance Name: %s ", instance.first.c_str());
        for (auto & interface : instance.second)
        {
            for (auto & result : interface.second)
            {
                char addr[Transport::PeerAddress::kMaxToStringSize];
                result.mParams.GetPeerAddress().ToString(addr);

                ChipLogProgress(NotSpecified, "\t %u - Discriminator: %u - Vendor: %u - Product: %u - %s", index,
                                result.mDiscriminator, result.mVendorId, result.mProductId, addr);
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
