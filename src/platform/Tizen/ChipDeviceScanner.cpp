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
#include <platform/PlatformManager.h>

#include "ErrorUtils.h"

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
    VerifyOrReturn(result == BT_ERROR_NONE, mDelegate->OnScanError(TizenToChipError(result)));
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
    VerifyOrReturnValue(ret == BT_ERROR_NONE, CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "bt_adapter_le_start_scan() failed: %s", get_error_message(ret)));
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

int ChipDeviceScanner::SetupScanFilter(ScanFilterType filterType, const ScanFilterData & filterData)
{
    VerifyOrReturnValue(__IsScanFilterSupported(), BT_ERROR_NONE, ChipLogError(DeviceLayer, "BLE scan filter not supported"));

    int ret = CreateLEScanFilter(filterType);
    VerifyOrExit(ret == BT_ERROR_NONE,
                 ChipLogError(DeviceLayer, "BLE scan filter creation failed: %s. Do Normal Scan", get_error_message(ret)));

    ret = RegisterScanFilter(filterType, filterData);
    VerifyOrExit(ret == BT_ERROR_NONE,
                 ChipLogError(DeviceLayer, "BLE scan filter registration failed: %s. Do Normal Scan", get_error_message(ret)));

    return ret;

exit:
    UnRegisterScanFilter();
    return ret;
}

CHIP_ERROR ChipDeviceScanner::StartScan(ScanFilterType filterType, const ScanFilterData & filterData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrReturnError(!mIsScanning, CHIP_ERROR_INCORRECT_STATE);

    // Scan Filter Setup if supported: silently bypass error & do filterless scan in case of error
    SetupScanFilter(filterType, filterData);

    // All set to trigger LE Scan
    err = PlatformMgrImpl().GLibMatterContextInvokeSync(
        +[](ChipDeviceScanner * self) { return self->StartScanImpl(); }, this);
    SuccessOrExit(err);

    return CHIP_NO_ERROR;

exit:
    ChipLogError(DeviceLayer, "Start CHIP Scan could not succeed fully! Stop Scan...");
    StopScan();
    UnRegisterScanFilter();
    return err;
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
        mScanFilter = nullptr;
    }
}

int ChipDeviceScanner::RegisterScanFilter(ScanFilterType filterType, const ScanFilterData & filterData)
{
    int ret = BT_ERROR_NONE;

    switch (filterType)
    {
    case ScanFilterType::kAddress: {
        ChipLogProgress(DeviceLayer, "Register BLE scan filter: Address");
        ret = bt_adapter_le_scan_filter_set_device_address(mScanFilter, filterData.address);
        VerifyOrExit(
            ret == BT_ERROR_NONE,
            ChipLogError(DeviceLayer, "bt_adapter_le_scan_filter_set_device_address() failed: %s", get_error_message(ret)));
        break;
    }
    case ScanFilterType::kServiceUUID: {
        ChipLogProgress(DeviceLayer, "Register BLE scan filter: Service UUID");
        ret = bt_adapter_le_scan_filter_set_service_uuid(mScanFilter, filterData.service_uuid);
        VerifyOrExit(ret == BT_ERROR_NONE,
                     ChipLogError(DeviceLayer, "bt_adapter_le_scan_filter_set_service_uuid() failed: %s", get_error_message(ret)));
        break;
    }
    case ScanFilterType::kServiceData: {
        ChipLogProgress(DeviceLayer, "Register BLE scan filter: Service Data");
        ret = bt_adapter_le_scan_filter_set_service_data(mScanFilter, filterData.service_uuid, filterData.service_data,
                                                         filterData.service_data_len);
        VerifyOrExit(ret == BT_ERROR_NONE,
                     ChipLogError(DeviceLayer, "bt_adapter_le_scan_filter_set_service_data() failed: %s", get_error_message(ret)));
        break;
    }
    case ScanFilterType::kNoFilter:
    default:
        goto exit;
    }

    ret = bt_adapter_le_scan_filter_register(mScanFilter);
    VerifyOrExit(ret == BT_ERROR_NONE,
                 ChipLogError(DeviceLayer, "bt_adapter_le_scan_filter_register() failed: %s", get_error_message(ret)));

exit:
    return ret;
}

int ChipDeviceScanner::CreateLEScanFilter(ScanFilterType filterType)
{
    int ret = bt_adapter_le_scan_filter_create(&mScanFilter);
    VerifyOrExit(ret == BT_ERROR_NONE,
                 ChipLogError(DeviceLayer, "bt_adapter_le_scan_filter_create() failed: %s", get_error_message(ret)));
    ChipLogProgress(DeviceLayer, "BLE scan filter created successfully");
exit:
    return ret;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
