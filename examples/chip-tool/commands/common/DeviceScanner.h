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

#pragma once

#include <controller/CHIPDeviceController.h>
#include <lib/dnssd/ResolverProxy.h>

#include <vector>

#if CONFIG_NETWORK_LAYER_BLE
#if CHIP_DEVICE_LAYER_TARGET_DARWIN
#include <platform/Darwin/BleScannerDelegate.h>
#define CHIP_TOOL_DEVICE_SCANNER_USE_BLE 1
#endif // CHIP_DEVICE_LAYER_TARGET_DARWIN
#endif // CONFIG_NETWORK_LAYER_BLE

#ifndef CHIP_TOOL_DEVICE_SCANNER_USE_BLE
#define CHIP_TOOL_DEVICE_SCANNER_USE_BLE 0
#endif // CHIP_TOOL_DEVICE_SCANNER_USE_BLE

struct DeviceScannerResult
{
    chip::Controller::SetUpCodePairerParameters mParams;
    chip::VendorId mVendorId;
    uint16_t mProductId;
    uint16_t mDiscriminator;
};

class DeviceScanner : public chip::Dnssd::CommissioningResolveDelegate
#if CHIP_TOOL_DEVICE_SCANNER_USE_BLE
    ,
                      public chip::DeviceLayer::BleScannerDelegate
#endif // CHIP_TOOL_DEVICE_SCANNER_USE_BLE
{
public:
    CHIP_ERROR Start();
    CHIP_ERROR Stop();
    CHIP_ERROR Get(uint16_t index, chip::RendezvousParameters & params);
    void Log() const;

    /////////// CommissioningResolveDelegate Interface /////////
    void OnNodeDiscovered(const chip::Dnssd::DiscoveredNodeData & nodeData) override;

#if CHIP_TOOL_DEVICE_SCANNER_USE_BLE
    /////////// BleScannerDelegate Interface /////////
    void OnBleScanAdd(BLE_CONNECTION_OBJECT connObj, const chip::Ble::ChipBLEDeviceIdentificationInfo & info) override;
    void OnBleScanRemove(BLE_CONNECTION_OBJECT connObj) override;
#endif // CHIP_TOOL_DEVICE_SCANNER_USE_BLE

private:
    std::vector<DeviceScannerResult> mDiscoveredResults;
    chip::Dnssd::ResolverProxy mDNSResolver;
};

DeviceScanner & GetDeviceScanner();
