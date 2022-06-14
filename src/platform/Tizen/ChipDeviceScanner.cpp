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
#include <platform/CHIPDeviceLayer.h>
#include <sys/param.h>

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#include "MainLoop.h"
#include <lib/support/logging/CHIPLogging.h>
#include <system/SystemTimer.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

// CHIPoBLE UUID strings
const char * chip_service_uuid       = "0000FFF6-0000-1000-8000-00805F9B34FB";
const char * chip_service_uuid_short = "FFF6";

// Default CHIP Scan Timeout in Millisecond
static unsigned int kScanTimeout = 10000;

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

gboolean ChipDeviceScanner::TriggerScan(GMainLoop * mainLoop, gpointer userData)
{
    auto self = reinterpret_cast<ChipDeviceScanner *>(userData);
    int ret   = BT_ERROR_NONE;
    GSource * idleSource;

    self->mAsyncLoop = mainLoop;

    // All set, trigger LE Scan
    ret = bt_adapter_le_start_scan(LeScanResultCb, userData);
    VerifyOrExit(ret == BT_ERROR_NONE, ChipLogError(DeviceLayer, "bt_adapter_le_start_scan() ret: %d", ret));
    ChipLogProgress(DeviceLayer, "Scan started");

    // Start Timer
    idleSource = g_timeout_source_new(kScanTimeout);
    g_source_set_callback(idleSource, TimerExpiredCb, userData, nullptr);
    g_source_set_priority(idleSource, G_PRIORITY_HIGH_IDLE);
    g_source_attach(idleSource, g_main_loop_get_context(self->mAsyncLoop));
    g_source_unref(idleSource);
    return true;

exit:
    return false;
}

static bool __IsScanFilterSupported(void)
{
    // Tizen API: bt_adapter_le_is_scan_filter_supported() is currently internal
    // Defaulting to true
    return true;
}

void ChipDeviceScanner::CheckScanFilter(ScanFilterType filterType, ScanFilterData & filterData)
{
    int ret = BT_ERROR_NONE;

    // Scan Filter check
    if (!__IsScanFilterSupported())
        return;

    ret = CreateLEScanFilter(filterType, filterData);
    VerifyOrExit(ret == BT_ERROR_NONE, ChipLogError(DeviceLayer, "Scan Filter Creation Failed! ret: %d Do Normal Scan", ret));

    ret = RegisterScanFilter(filterType, filterData);
    VerifyOrExit(ret == BT_ERROR_NONE, ChipLogError(DeviceLayer, "Scan Filter Registration Failed! ret: %d Do Normal Scan", ret));
    return;
exit:
    UnRegisterScanFilter();
}

CHIP_ERROR ChipDeviceScanner::StartChipScan(unsigned timeoutMs, ScanFilterType filterType, ScanFilterData & filterData)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ReturnErrorCodeIf(mIsScanning, CHIP_ERROR_INCORRECT_STATE);

    // Scan Filter Setup if supported: silently bypass error & do filterless scan in case of error
    CheckScanFilter(filterType, filterData);

    kScanTimeout = timeoutMs;

    // All set to trigger LE Scan
    ChipLogProgress(DeviceLayer, "Start CHIP Scan...");
    if (MainLoop::Instance().AsyncRequest(TriggerScan, this) == false)
    {
        ChipLogError(DeviceLayer, "Failed to trigger Scan...");
        err = CHIP_ERROR_INTERNAL;
        goto exit;
    }

    mIsScanning = true; // optimistic, to allow all callbacks to check this
    return CHIP_NO_ERROR;

exit:
    ChipLogError(DeviceLayer, "Start CHIP Scan could not succeed fully! Stop Scan...");
    StopChipScan();
    UnRegisterScanFilter();
    return err;
}

CHIP_ERROR ChipDeviceScanner::StopChipScan(void)
{
    int ret = BT_ERROR_NONE;
    ReturnErrorCodeIf(!mIsScanning, CHIP_ERROR_INCORRECT_STATE);

    ret = bt_adapter_le_stop_scan();
    if (ret != BT_ERROR_NONE)
    {
        ChipLogError(DeviceLayer, "bt_adapter_le_stop_scan() failed. ret: %d", ret);
    }

    g_main_loop_quit(mAsyncLoop);
    ChipLogProgress(DeviceLayer, "CHIP Scanner Async Thread Quit Done..Wait for Thread Windup...!");

    UnRegisterScanFilter();

    // Report to Impl class
    mDelegate->OnChipScanComplete();

    mIsScanning = false;

    return CHIP_NO_ERROR;
}

void ChipDeviceScanner::UnRegisterScanFilter(void)
{
    if (mScanFilter)
    {
        bt_adapter_le_scan_filter_unregister(mScanFilter);
        mScanFilter = nullptr;
    }
}

int ChipDeviceScanner::RegisterScanFilter(ScanFilterType filterType, ScanFilterData & filterData)
{
    int ret = BT_ERROR_NONE;

    switch (filterType)
    {
    case ScanFilterType::kAddress: {
        ChipLogProgress(DeviceLayer, "Register Scan filter: Address");
        ret = bt_adapter_le_scan_filter_set_device_address(mScanFilter, filterData.address);
        VerifyOrExit(ret == BT_ERROR_NONE,
                     ChipLogError(DeviceLayer, "bt_adapter_le_scan_filter_set_device_address() failed. ret: %d", ret));
        break;
    }
    case ScanFilterType::kServiceUUID: {
        ChipLogProgress(DeviceLayer, "Register Scan filter: Service UUID");
        ret = bt_adapter_le_scan_filter_set_service_uuid(mScanFilter, filterData.service_uuid);
        VerifyOrExit(ret == BT_ERROR_NONE,
                     ChipLogError(DeviceLayer, "bt_adapter_le_scan_filter_set_service_uuid() failed. ret: %d", ret));
        break;
    }
    case ScanFilterType::kServiceData: {
        ChipLogProgress(DeviceLayer, "Register Scan filter: Service Data");
        ret = bt_adapter_le_scan_filter_set_service_data(mScanFilter, filterData.service_uuid, filterData.service_data,
                                                         filterData.service_data_len);
        VerifyOrExit(ret == BT_ERROR_NONE,
                     ChipLogError(DeviceLayer, "bt_adapter_le_scan_filter_set_service_data() failed. ret: %d", ret));
        break;
    }
    case ScanFilterType::kNoFilter:
    default:
        return ret; // Without Scan Filter
    }

    ret = bt_adapter_le_scan_filter_register(mScanFilter);
    VerifyOrExit(ret == BT_ERROR_NONE, ChipLogError(DeviceLayer, "bt_adapter_le_scan_filter_register failed(). ret: %d", ret));
    return ret;
exit:
    return ret;
}

int ChipDeviceScanner::CreateLEScanFilter(ScanFilterType filterType, ScanFilterData & filterData)
{
    int ret = BT_ERROR_NONE;

    ret = bt_adapter_le_scan_filter_create(&mScanFilter);
    VerifyOrExit(ret == BT_ERROR_NONE, ChipLogError(DeviceLayer, "bt_adapter_le_scan_filter_create() Failed. ret: %d", ret));
    ChipLogError(DeviceLayer, "Scan Filter Created Successfully");
exit:
    return ret;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
