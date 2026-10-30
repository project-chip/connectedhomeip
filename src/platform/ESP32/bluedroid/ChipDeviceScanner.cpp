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

#include <platform/ESP32/ChipDeviceScanner.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include <cstring>

#define CHIPoBLE_SERVICE_UUID 0xFFF6

namespace chip {
namespace DeviceLayer {
namespace Internal {
namespace {

/// Retrieve CHIP device identification info from BLE advertising data.
bool BluedroidGetChipDeviceInfo(const uint8_t * data, size_t dataLength, chip::Ble::ChipBLEDeviceIdentificationInfo & deviceInfo)
{
    while (data != nullptr && dataLength > 1)
    {
        const size_t fieldLength = data[0];
        if (fieldLength == 0 || fieldLength + 1 > dataLength)
        {
            return false;
        }

        constexpr size_t kServiceDataHeaderSize = 3; // AD type and 16-bit UUID
        if (data[1] == ESP_BLE_AD_TYPE_SERVICE_DATA && fieldLength >= kServiceDataHeaderSize + sizeof(deviceInfo) &&
            data[2] == 0xf6 && data[3] == 0xff)
        {
            std::memcpy(&deviceInfo, data + 4, sizeof(deviceInfo));
            return true;
        }

        data += fieldLength + 1;
        dataLength -= fieldLength + 1;
    }
    return false;
}

} // namespace

void ChipDeviceScanner::ReportDevice(esp_ble_gap_cb_param_t & scan_result, esp_bd_addr_t & addr)
{
    chip::Ble::ChipBLEDeviceIdentificationInfo deviceInfo;
    const uint8_t * data              = scan_result.scan_rst.ble_adv;
    const bool foundInAdvertisingData = BluedroidGetChipDeviceInfo(data, scan_result.scan_rst.adv_data_len, deviceInfo);
    const bool foundInScanResponse    = !foundInAdvertisingData && data != nullptr &&
        BluedroidGetChipDeviceInfo(data + scan_result.scan_rst.adv_data_len, scan_result.scan_rst.scan_rsp_len, deviceInfo);
    if (!foundInAdvertisingData && !foundInScanResponse)
    {
        return;
    }
    mDelegate->OnDeviceScanned(scan_result.scan_rst.ble_addr_type, addr, deviceInfo);
}

void ChipDeviceScanner::RemoveDevice()
{
    // TODO
}

CHIP_ERROR ChipDeviceScanner::StartScan(uint16_t timeout)
{
    VerifyOrReturnError(!mIsScanning, CHIP_ERROR_INCORRECT_STATE);

#ifdef CONFIG_ESP32_BLE_COMMISSIONER_ACTIVE_SCAN
    constexpr esp_ble_scan_type_t kScanType = BLE_SCAN_TYPE_ACTIVE;
#else
    constexpr esp_ble_scan_type_t kScanType = BLE_SCAN_TYPE_PASSIVE;
#endif
    static esp_ble_scan_params_t ble_scan_params = { .scan_type          = kScanType,
                                                     .own_addr_type      = BLE_ADDR_TYPE_RANDOM,
                                                     .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
                                                     .scan_interval      = 0x00,
                                                     .scan_window        = 0x00,
                                                     .scan_duplicate     = BLE_SCAN_DUPLICATE_DISABLE };

    int rc = esp_ble_gap_set_scan_params(&ble_scan_params);
    if (rc != 0)
    {
        ChipLogError(DeviceLayer, "esp_ble_gap_set_scan_params failed: %d", rc);
        return CHIP_ERROR_INTERNAL;
    }
    /* Start the discovery process. */
    rc = esp_ble_gap_start_scanning(timeout);
    if (rc != 0)
    {
        ChipLogError(DeviceLayer, "esp_ble_gap_start_scanning failed: %d", rc);
        return CHIP_ERROR_INTERNAL;
    }
    mIsScanning = true;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDeviceScanner::StopScan()
{
    VerifyOrReturnError(mIsScanning, CHIP_NO_ERROR);

    int rc = esp_ble_gap_stop_scanning();
    if (rc != 0)
    {
        ChipLogError(DeviceLayer, "ble_gap_disc_cancel failed: %d", rc);
        return CHIP_ERROR_INTERNAL;
    }

    mIsScanning = false;
    mDelegate->OnScanComplete();
    return CHIP_NO_ERROR;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
