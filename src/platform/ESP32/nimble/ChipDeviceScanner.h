/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#pragma once

// #if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
// #if CONFIG_BT_NIMBLE_ENABLED

#include <ble/CHIPBleServiceData.h>
#include <lib/core/CHIPError.h>
#include <system/SystemLayer.h>

#include <host/ble_hs.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

/// Receives callbacks when chip devices are being scanned
class ChipDeviceScannerDelegate
{
public:
    virtual ~ChipDeviceScannerDelegate() {}

    // Called when a CHIP device was found
    virtual void OnDeviceScanned(const struct ble_hs_adv_fields & fields, const ble_addr_t & addr, const chip::Ble::ChipBLEDeviceIdentificationInfo & info) = 0;

    // Called when a scan was completed (stopped or timed out)
    virtual void OnScanComplete() = 0;
};

/// Allows scanning for CHIP devices
/// Will perform scan operations and call back whenever a device is discovered.
class ChipDeviceScanner
{
public:
    ChipDeviceScanner(ChipDeviceScanner &&)      = delete;
    ChipDeviceScanner(const ChipDeviceScanner &) = delete;
    ChipDeviceScanner & operator=(const ChipDeviceScanner &) = delete;

    ~ChipDeviceScanner() = default;

    static ChipDeviceScanner & GetInstance()
    {
        static ChipDeviceScanner instance;
        return instance;
    }

    /// Initializes the scanner
    CHIP_ERROR Init(ChipDeviceScannerDelegate * delegate)
    {
        VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
        mDelegate = delegate;
        return CHIP_NO_ERROR;
    }

    /// Initiate a scan for devices, with the given timeout
    CHIP_ERROR StartScan(uint16_t timeout);

    /// Stop any currently running scan
    CHIP_ERROR StopScan();

private:
    ChipDeviceScanner() = default;

    /// Check if a given device is a CHIP device and if yes, report it as discovered
    void ReportDevice(const struct ble_hs_adv_fields & fields, const ble_addr_t & addr);
    static int OnBleCentralEvent(struct ble_gap_event *event, void *arg);
    void RemoveDevice();
    ChipDeviceScannerDelegate * mDelegate = nullptr;
    bool mIsScanning                      = false;
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

// #endif // CONFIG_BT_NIMBLE_ENABLED
// #endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
