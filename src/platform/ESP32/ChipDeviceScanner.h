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

#include <ble/Ble.h>
#include <lib/core/CHIPError.h>
#include <system/SystemLayer.h>

#ifdef CONFIG_BT_NIMBLE_ENABLED
#include "host/ble_hs.h"
#elif defined(CONFIG_BT_BLUEDROID_ENABLED)
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatt_common_api.h"
#include "esp_gatt_defs.h"
#include "esp_gattc_api.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include <lib/core/CHIPError.h>
#include <system/SystemLayer.h>
#endif

namespace chip {
namespace DeviceLayer {
namespace Internal {

/// Receives callbacks when chip devices are being scanned
class ChipDeviceScannerDelegate
{
public:
    virtual ~ChipDeviceScannerDelegate() {}

    // Called when a CHIP device was found
#ifdef CONFIG_BT_NIMBLE_ENABLED
    virtual void OnDeviceScanned(const struct ble_hs_adv_fields & fields, const ble_addr_t & addr,
                                 const chip::Ble::ChipBLEDeviceIdentificationInfo & info) = 0;
#else
    virtual void OnDeviceScanned(esp_ble_addr_type_t & addr_type, esp_bd_addr_t & addr,
                                 const chip::Ble::ChipBLEDeviceIdentificationInfo & info) = 0;
#endif
    // Called when a scan was completed (stopped or timed out)
    virtual void OnScanComplete() = 0;
};

/// Allows scanning for CHIP devices
/// Will perform scan operations and call back whenever a device is discovered.
class ChipDeviceScanner
{
public:
    ChipDeviceScanner(ChipDeviceScanner &&)                  = delete;
    ChipDeviceScanner(const ChipDeviceScanner &)             = delete;
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

    bool mIsScanning = false;
#ifdef CONFIG_BT_NIMBLE_ENABLED
    void ReportDevice(const struct ble_hs_adv_fields & fields, const ble_addr_t & addr);
#else
    void ReportDevice(esp_ble_gap_cb_param_t & fields, esp_bd_addr_t & addr);
#endif

private:
    ChipDeviceScanner() = default;

    /// Check if a given device is a CHIP device and if yes, report it as discovered
    static int OnBleCentralEvent(struct ble_gap_event * event, void * arg);
    void RemoveDevice();
    ChipDeviceScannerDelegate * mDelegate = nullptr;
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
