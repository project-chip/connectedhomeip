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

#pragma once

#include <platform/CHIPDeviceConfig.h>

#if CHIP_DEVICE_LAYER_TARGET_DARWIN

#include <controller/CHIPDeviceController.h>
#include <lib/dnssd/platform/Dnssd.h>

#include <string>
#include <unordered_map>
#include <vector>

#if CONFIG_NETWORK_LAYER_BLE
#include <platform/Darwin/BleScannerDelegate.h>
#endif // CONFIG_NETWORK_LAYER_BLE

struct DeviceScannerResult
{
    chip::Controller::SetUpCodePairerParameters mParams;
    chip::VendorId mVendorId;
    uint16_t mProductId;
    uint16_t mDiscriminator;
    chip::Optional<chip::Dnssd::CommonResolutionData> mResolutionData;
};

class DeviceScanner : public chip::Dnssd::DiscoverNodeDelegate,
                      public chip::Dnssd::DnssdBrowseDelegate
#if CONFIG_NETWORK_LAYER_BLE
    ,
                      public chip::DeviceLayer::BleScannerDelegate
#endif // CONFIG_NETWORK_LAYER_BLE
{
public:
    CHIP_ERROR Start();
    CHIP_ERROR Stop();
    CHIP_ERROR Get(uint16_t index, chip::RendezvousParameters & params);
    CHIP_ERROR Get(uint16_t index, chip::Dnssd::CommonResolutionData & resolutionData);
    void Log() const;

    /////////// DiscoverNodeDelegate Interface /////////
    void OnNodeDiscovered(const chip::Dnssd::DiscoveredNodeData & nodeData) override;

    /////////// DnssdBrowseDelegate Interface /////////
    void OnBrowseAdd(chip::Dnssd::DnssdService service) override;
    void OnBrowseRemove(chip::Dnssd::DnssdService service) override;
    void OnBrowseStop(CHIP_ERROR error) override;

#if CONFIG_NETWORK_LAYER_BLE
    /////////// BleScannerDelegate Interface /////////
    void OnBleScanAdd(BLE_CONNECTION_OBJECT connObj, const chip::Ble::ChipBLEDeviceIdentificationInfo & info) override;
    void OnBleScanRemove(BLE_CONNECTION_OBJECT connObj) override;
#endif // CONFIG_NETWORK_LAYER_BLE

private:
    std::unordered_map<std::string, std::map<chip::Inet::InterfaceId::PlatformType, std::vector<DeviceScannerResult>>>
        mDiscoveredResults;
};

DeviceScanner & GetDeviceScanner();

#endif // CHIP_DEVICE_LAYER_TARGET_DARWIN
