/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/**
 *    @file
 *          Provides an implementation of the Matter Device Scanner
 *          for the Tizen platforms
 */

#include "ChipDeviceScanner.h"

#include <cstdint>
#include <cstring>
#include <strings.h>
#include <utility>

#include <bluetooth.h>
#include <bluetooth_internal.h>

#include <ble/BleUUID.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/GLibTypeDeleter.h>
#include <platform/PlatformError.h>
#include <platform/PlatformManager.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

static void __PrintLEScanData(const bt_adapter_le_service_data_s & data)
{
    // Print Service UUID in the Service Data
    ChipLogDetail(DeviceLayer, "======Service UUID========");
    ChipLogDetail(DeviceLayer, "Service UUID::[%s]", data.service_uuid);

    // Print Service Data
    ChipLogDetail(DeviceLayer, "======Service Data========");
    ChipLogDetail(DeviceLayer, "Service Data Length::[%d]", data.service_data_len);
    ChipLogByteSpan(DeviceLayer, ByteSpan(reinterpret_cast<uint8_t *>(data.service_data), data.service_data_len));
}

static bool __IsChipThingDevice(const bt_adapter_le_device_scan_result_info_s & scanInfo,
                                chip::Ble::ChipBLEDeviceIdentificationInfo & info)
{
    int count                               = 0;
    bt_adapter_le_service_data_s * dataList = nullptr;
    bool isChipDevice                       = false;

    if (bt_adapter_le_get_scan_result_service_data_list(&scanInfo, BT_ADAPTER_LE_PACKET_ADVERTISING, &dataList, &count) ==
        BT_ERROR_NONE)
    {
        for (int i = 0; i < count; i++)
        {
            if (strcasecmp(dataList[i].service_uuid, chip::Ble::CHIP_BLE_SERVICE_LONG_UUID_STR) == 0 ||
                strcasecmp(dataList[i].service_uuid, chip::Ble::CHIP_BLE_SERVICE_SHORT_UUID_STR) == 0)
            {
                __PrintLEScanData(dataList[i]);
                memcpy(&info, dataList[i].service_data, dataList[i].service_data_len);
                isChipDevice = true;
                break;
            }
        }
    }

    bt_adapter_le_free_service_data_list(dataList, count);
    return isChipDevice;
}

void ChipDeviceScanner::LeScanResultCb(int result, bt_adapter_le_device_scan_result_info_s * scanInfo)
{
    VerifyOrReturn(result == BT_ERROR_NONE, mDelegate->OnScanError(MATTER_PLATFORM_ERROR(result)));
    VerifyOrReturn(scanInfo != nullptr, mDelegate->OnScanError(CHIP_ERROR_INTERNAL));

    ChipLogProgress(DeviceLayer, "LE device reported: %s", scanInfo->remote_address);

    chip::Ble::ChipBLEDeviceIdentificationInfo info;
    VerifyOrReturn(__IsChipThingDevice(*scanInfo, info),
                   ChipLogDetail(Ble, "Device %s does not look like a CHIP device", scanInfo->remote_address));

    // Report probable CHIP device to BLEMgrImp class
    mDelegate->OnDeviceScanned(*scanInfo, info);
}

CHIP_ERROR ChipDeviceScanner::StartScanImpl()
{
    int ret = bt_adapter_le_start_scan(
        +[](int result, bt_adapter_le_device_scan_result_info_s * scanInfo, void * self) {
            return reinterpret_cast<ChipDeviceScanner *>(self)->LeScanResultCb(result, scanInfo);
        },
        this);
    VerifyOrReturnValue(ret == BT_ERROR_NONE, MATTER_PLATFORM_ERROR(ret));
    mIsScanning = true;
    return CHIP_NO_ERROR;
}

static bool __IsScanFilterSupported()
{
    bool is_supported;
    int ret = bt_adapter_le_is_scan_filter_supported(&is_supported);
    VerifyOrReturnValue(ret == BT_ERROR_NONE, false,
                        ChipLogError(DeviceLayer, "bt_adapter_le_is_scan_filter_supported() failed: %s", get_error_message(ret)));
    return is_supported;
}

CHIP_ERROR ChipDeviceScanner::SetupScanFilter(ScanFilterType filterType, const ScanFilterData & filterData)
{
    VerifyOrReturnError(__IsScanFilterSupported(), MATTER_PLATFORM_ERROR(BT_ERROR_NOT_SUPPORTED));

    int ret = bt_adapter_le_scan_filter_create(&mScanFilter);
    VerifyOrReturnError(ret == BT_ERROR_NONE, MATTER_PLATFORM_ERROR(ret));

    ReturnErrorOnFailure(RegisterScanFilter(filterType, filterData), UnRegisterScanFilter());
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDeviceScanner::StartScan(ScanFilterType filterType, const ScanFilterData & filterData)
{
    VerifyOrReturnError(!mIsScanning, CHIP_ERROR_INCORRECT_STATE);

    // Setup scan filter if supported. Otherwise, do filterless scan.
    CHIP_ERROR err = SetupScanFilter(filterType, filterData);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to set up scan filter: %" CHIP_ERROR_FORMAT, err.Format());
        ChipLogProgress(DeviceLayer, "Proceeding with filterless scan");
    }

    err = PlatformMgrImpl().GLibMatterContextInvokeSync(
        +[](ChipDeviceScanner * self) { return self->StartScanImpl(); }, this);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err, TEMPORARY_RETURN_IGNORED StopScan());

    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDeviceScanner::StopScan()
{
    int ret = BT_ERROR_NONE;
    VerifyOrReturnError(mIsScanning, CHIP_ERROR_INCORRECT_STATE);

    ret = bt_adapter_le_stop_scan();
    if (ret != BT_ERROR_NONE)
    {
        ChipLogError(DeviceLayer, "bt_adapter_le_stop_scan() failed: %s", get_error_message(ret));
    }

    ChipLogProgress(DeviceLayer, "CHIP Scanner Async Thread Quit Done..Wait for Thread Windup...!");

    UnRegisterScanFilter();

    // Report to Impl class
    mDelegate->OnScanComplete();

    mIsScanning = false;

    return CHIP_NO_ERROR;
}

void ChipDeviceScanner::UnRegisterScanFilter()
{
    if (mScanFilter)
    {
        bt_adapter_le_scan_filter_unregister(mScanFilter);
        bt_adapter_le_scan_filter_destroy(mScanFilter);
        mScanFilter = nullptr;
    }
}

CHIP_ERROR ChipDeviceScanner::RegisterScanFilter(ScanFilterType filterType, const ScanFilterData & filterData)
{
    int ret;

    switch (filterType)
    {
    case ScanFilterType::kAddress:
        ChipLogProgress(DeviceLayer, "Register BLE scan filter: Address");
        ret = bt_adapter_le_scan_filter_set_device_address(mScanFilter, filterData.address);
        VerifyOrReturnError(
            ret == BT_ERROR_NONE, MATTER_PLATFORM_ERROR(ret),
            ChipLogError(DeviceLayer, "bt_adapter_le_scan_filter_set_device_address() failed: %s", get_error_message(ret)));
        break;
    case ScanFilterType::kServiceUUID:
        ChipLogProgress(DeviceLayer, "Register BLE scan filter: Service UUID");
        ret = bt_adapter_le_scan_filter_set_service_uuid(mScanFilter, filterData.service_uuid);
        VerifyOrReturnError(
            ret == BT_ERROR_NONE, MATTER_PLATFORM_ERROR(ret),
            ChipLogError(DeviceLayer, "bt_adapter_le_scan_filter_set_service_uuid() failed: %s", get_error_message(ret)));
        break;
    case ScanFilterType::kServiceData:
        ChipLogProgress(DeviceLayer, "Register BLE scan filter: Service Data");
        ret = bt_adapter_le_scan_filter_set_service_data(mScanFilter, filterData.service_uuid, filterData.service_data,
                                                         filterData.service_data_len);
        VerifyOrReturnError(
            ret == BT_ERROR_NONE, MATTER_PLATFORM_ERROR(ret),
            ChipLogError(DeviceLayer, "bt_adapter_le_scan_filter_set_service_data() failed: %s", get_error_message(ret)));
        break;
    case ScanFilterType::kNoFilter:
    default:
        return CHIP_NO_ERROR;
    }

    ret = bt_adapter_le_scan_filter_register(mScanFilter);
    VerifyOrReturnError(ret == BT_ERROR_NONE, MATTER_PLATFORM_ERROR(ret),
                        ChipLogError(DeviceLayer, "bt_adapter_le_scan_filter_register() failed: %s", get_error_message(ret)));

    return CHIP_NO_ERROR;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
