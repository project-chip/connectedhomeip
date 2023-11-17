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

#define CHIPoBLE_SERVICE_UUID 0xFFF6

namespace chip {
namespace DeviceLayer {
namespace Internal {
namespace {

/// Retrieve CHIP device identification info from the device advertising data
bool BluedroidGetChipDeviceInfo(esp_ble_gap_cb_param_t & scan_result, chip::Ble::ChipBLEDeviceIdentificationInfo & deviceInfo)
{
    // Check for CHIP Service UUID
    if (scan_result.scan_rst.ble_adv != NULL)
    {
        if (scan_result.scan_rst.adv_data_len > 13 && scan_result.scan_rst.ble_adv[5] == 0xf6 &&
            scan_result.scan_rst.ble_adv[6] == 0xff)
        {
            deviceInfo.OpCode                              = scan_result.scan_rst.ble_adv[7];
            deviceInfo.DeviceDiscriminatorAndAdvVersion[0] = scan_result.scan_rst.ble_adv[8];
            deviceInfo.DeviceDiscriminatorAndAdvVersion[1] = scan_result.scan_rst.ble_adv[9];
            // vendor and product Id from adv
            deviceInfo.DeviceVendorId[0]  = scan_result.scan_rst.ble_adv[10];
            deviceInfo.DeviceVendorId[1]  = scan_result.scan_rst.ble_adv[11];
            deviceInfo.DeviceProductId[0] = scan_result.scan_rst.ble_adv[12];
            deviceInfo.DeviceProductId[1] = scan_result.scan_rst.ble_adv[13];
            deviceInfo.AdditionalDataFlag = scan_result.scan_rst.ble_adv[14];
            return true;
        }
    }
    return false;
}

} // namespace

void ChipDeviceScanner::ReportDevice(esp_ble_gap_cb_param_t & scan_result, esp_bd_addr_t & addr)
{
    chip::Ble::ChipBLEDeviceIdentificationInfo deviceInfo;
    if (BluedroidGetChipDeviceInfo(scan_result, deviceInfo) == false)
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
    ReturnErrorCodeIf(mIsScanning, CHIP_ERROR_INCORRECT_STATE);

    static esp_ble_scan_params_t ble_scan_params = { .scan_type          = BLE_SCAN_TYPE_PASSIVE,
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
    ReturnErrorCodeIf(!mIsScanning, CHIP_NO_ERROR);

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
