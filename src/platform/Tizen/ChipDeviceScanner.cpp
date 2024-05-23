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
#include <utility>

#include <bluetooth.h>
#include <bluetooth_internal.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/GLibTypeDeleter.h>
#include <platform/PlatformManager.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

// CHIPoBLE UUID strings
const char chip_service_uuid[]       = "0000FFF6-0000-1000-8000-00805F9B34FB";
const char chip_service_uuid_short[] = "FFF6";

ChipDeviceScanner::ChipDeviceScanner(ChipDeviceScannerDelegate * delegate) : mDelegate(delegate) {}

ChipDeviceScanner::~ChipDeviceScanner()
{
    // In case scan is ongoing
    StopChipScan();
    mDelegate = nullptr;
}

std::unique_ptr<ChipDeviceScanner> ChipDeviceScanner::Create(ChipDeviceScannerDelegate * delegate)
{
    return std::make_unique<ChipDeviceScanner>(delegate);
}

static void __CleanupServiceData(bt_adapter_le_service_data_s * dataList, size_t count)
{
    VerifyOrReturn(dataList != nullptr);
    VerifyOrReturn(count != 0);

    for (size_t i = 0; i < count; i++)
    {
        g_free(dataList[i].service_uuid);
        g_free(dataList[i].service_data);
    }
    g_free(dataList);
}

static void __PrintLEScanData(bt_adapter_le_service_data_s * dataList, size_t idx)
{
    VerifyOrReturn(dataList != nullptr);

    // Print Service UUID in the Service Data
    ChipLogDetail(DeviceLayer, "======Service UUID========");
    ChipLogDetail(DeviceLayer, "Service UUID::[%s]", dataList[idx].service_uuid);

    // Print Service Data
    ChipLogDetail(DeviceLayer, "======Service Data========");
    ChipLogDetail(DeviceLayer, "Service Data Length::[%d]", dataList[idx].service_data_len);
    ChipLogByteSpan(DeviceLayer, ByteSpan(reinterpret_cast<uint8_t *>(dataList[idx].service_data), dataList[idx].service_data_len));
}

static bool __IsChipThingDevice(bt_adapter_le_device_scan_result_info_s * info,
                                chip::Ble::ChipBLEDeviceIdentificationInfo & aDeviceInfo)
{
    VerifyOrReturnError(info != nullptr, false);

    int count                               = 0;
    bt_adapter_le_service_data_s * dataList = nullptr;
    bool isChipDevice                       = false;

    ChipLogProgress(DeviceLayer, "Is [%s] ChipThingDevice ?: Check now", info->remote_address);

    if (bt_adapter_le_get_scan_result_service_data_list(info, BT_ADAPTER_LE_PACKET_ADVERTISING, &dataList, &count) == BT_ERROR_NONE)
    {
        for (int i = 0; i < count; i++)
        {
            if (g_strcmp0(dataList[i].service_uuid, chip_service_uuid) == 0 ||
                g_strcmp0(dataList[i].service_uuid, chip_service_uuid_short) == 0)
            {
                ChipLogProgress(DeviceLayer, "CHIP Thing Device Found! [Service Data UUID] = %s", dataList[i].service_uuid);
                // Print full Service Data
                __PrintLEScanData(dataList, i);
                memcpy(&aDeviceInfo, dataList[i].service_data, dataList[i].service_data_len);
                isChipDevice = true;
                break;
            }
        }
    }
    __CleanupServiceData(dataList, count);
    return isChipDevice;
}

void ChipDeviceScanner::LeScanResultCb(int result, bt_adapter_le_device_scan_result_info_s * info, void * userData)
{
    VerifyOrReturn(info != nullptr);

    auto self = reinterpret_cast<ChipDeviceScanner *>(userData);
    chip::Ble::ChipBLEDeviceIdentificationInfo deviceInfo;

    ChipLogProgress(DeviceLayer, "LE Device Reported!! remote addr [%s]", info->remote_address);

    if (__IsChipThingDevice(info, deviceInfo))
    {
        // Looks like a CHIP Thing Device: Service UUID matched
        ChipLogProgress(DeviceLayer, "Looks Like Got a CHIP Thing Device: Process further");
        // Report probable CHIP Thing Device to BLEMgrImp class
        self->mDelegate->OnChipDeviceScanned(info, deviceInfo);
    }
    else
        ChipLogProgress(DeviceLayer, "Does not Look like a CHIP Device, Skip.....");
}

gboolean ChipDeviceScanner::TimerExpiredCb(gpointer userData)
{
    auto self = reinterpret_cast<ChipDeviceScanner *>(userData);
    ChipLogProgress(DeviceLayer, "Scan Timer expired!!");
    self->StopChipScan();
    return G_SOURCE_REMOVE;
}

CHIP_ERROR ChipDeviceScanner::TriggerScan(ChipDeviceScanner * self)
{
    GAutoPtr<GSource> idleSource;
    int ret;

    // Trigger LE Scan
    ret = bt_adapter_le_start_scan(LeScanResultCb, self);
    VerifyOrReturnValue(ret == BT_ERROR_NONE, CHIP_ERROR_INTERNAL,
                        ChipLogError(DeviceLayer, "bt_adapter_le_start_scan() failed: %s", get_error_message(ret)));
    self->mIsScanning = true;

    // Setup timer for scan timeout
    idleSource = GAutoPtr<GSource>(g_timeout_source_new(self->mScanTimeoutMs));
    g_source_set_callback(idleSource.get(), TimerExpiredCb, self, nullptr);
    g_source_set_priority(idleSource.get(), G_PRIORITY_HIGH_IDLE);
    g_source_attach(idleSource.get(), g_main_context_get_thread_default());

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

CHIP_ERROR ChipDeviceScanner::StartChipScan(System::Clock::Timeout timeout, ScanFilterType filterType,
                                            const ScanFilterData & filterData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ReturnErrorCodeIf(mIsScanning, CHIP_ERROR_INCORRECT_STATE);

    // Scan Filter Setup if supported: silently bypass error & do filterless scan in case of error
    SetupScanFilter(filterType, filterData);

    mScanTimeoutMs = System::Clock::Milliseconds32(timeout).count();

    // All set to trigger LE Scan
    ChipLogProgress(DeviceLayer, "Start CHIP BLE scan: timeout=%ums", mScanTimeoutMs);
    err = PlatformMgrImpl().GLibMatterContextInvokeSync(TriggerScan, this);
    SuccessOrExit(err);

    return CHIP_NO_ERROR;

exit:
    ChipLogError(DeviceLayer, "Start CHIP Scan could not succeed fully! Stop Scan...");
    StopChipScan();
    UnRegisterScanFilter();
    return err;
}

CHIP_ERROR ChipDeviceScanner::StopChipScan()
{
    int ret = BT_ERROR_NONE;
    ReturnErrorCodeIf(!mIsScanning, CHIP_ERROR_INCORRECT_STATE);

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
