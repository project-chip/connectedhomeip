/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Provides an implementation of the Matter Device Scanner
 *          for the Tizen platform.
 */

#pragma once

#include <memory>

#include <bluetooth.h>
#include <glib.h>

#include <ble/CHIPBleServiceData.h>
#include <lib/core/CHIPError.h>
#include <system/SystemClock.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

#define CHIP_TIZEN_BLE_ADDRESS_STRING_LEN 18
#define CHIP_TIZEN_BLE_SERVICE_DATA_MAX_LEN 7
#define CHIP_TIZEN_BLE_SERVICE_UUID_STRING_LEN 37

enum class ScanFilterType
{
    kAddress,     // Scan by Address
    kServiceUUID, // Scan by UUID
    kServiceData, // Scan by Service Data
    kNoFilter,    // For Filterless Scan
};

struct ScanFilterData
{
    char address[CHIP_TIZEN_BLE_ADDRESS_STRING_LEN];
    char service_uuid[CHIP_TIZEN_BLE_SERVICE_UUID_STRING_LEN];
    char service_data[CHIP_TIZEN_BLE_SERVICE_DATA_MAX_LEN];
    unsigned int service_data_len;
};

// Receives callbacks when chip devices are being scanned
class ChipDeviceScannerDelegate
{
public:
    virtual ~ChipDeviceScannerDelegate(void) {}

    // Called when a CHIP device was found
    virtual void OnChipDeviceScanned(void * device, const chip::Ble::ChipBLEDeviceIdentificationInfo & info) = 0;

    // Called when a scan was completed (stopped or timed out)
    virtual void OnScanComplete(void) = 0;

    // Called on scan error
    virtual void OnScanError(CHIP_ERROR err) = 0;
};

/// Allows scanning for CHIP devices
///
/// Will perform scan operations and call back whenever a device is discovered.
class ChipDeviceScanner
{
public:
    ChipDeviceScanner(void){};

    /// NOTE: prefer to use the  ::Create method instead direct constructor calling.
    ChipDeviceScanner(ChipDeviceScannerDelegate * delegate);

    ~ChipDeviceScanner(void);

    /// Initiate a scan for devices, with the given timeout & scan filter data
    CHIP_ERROR StartChipScan(System::Clock::Timeout timeout, ScanFilterType filterType, ScanFilterData & filterData);

    /// Stop any currently running scan
    CHIP_ERROR StopChipScan(void);

    /// Create a new device scanner
    /// Convenience method to allocate any required variables.
    static std::unique_ptr<ChipDeviceScanner> Create(ChipDeviceScannerDelegate * delegate);

private:
    static void LeScanResultCb(int result, bt_adapter_le_device_scan_result_info_s * info, void * userData);
    static gboolean TimerExpiredCb(gpointer user_data);
    static CHIP_ERROR TriggerScan(ChipDeviceScanner * userData);
    void CheckScanFilter(ScanFilterType filterType, ScanFilterData & filterData);
    int RegisterScanFilter(ScanFilterType filterType, ScanFilterData & filterData);
    void UnRegisterScanFilter(void);
    int CreateLEScanFilter(ScanFilterType filterType, ScanFilterData & filterData);

    ChipDeviceScannerDelegate * mDelegate = nullptr;
    bool mIsScanning                      = false;
    bool mIsStopping                      = false;
    unsigned int mScanTimeoutMs           = 10000;
    bt_scan_filter_h mScanFilter          = nullptr;
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
