/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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

#include "ChipDeviceScanner.h"

#include <cstdint>
#include <cstring>
#include <memory>

#include <glib-object.h>

#include <ble/Ble.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/GLibTypeDeleter.h>

#include "Types.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

namespace {

/// Retrieve CHIP device identification info from the device advertising data
bool BluezGetChipDeviceInfo(BluezDevice1 & aDevice, chip::Ble::ChipBLEDeviceIdentificationInfo & aDeviceInfo)
{
    GVariant * serviceData = bluez_device1_get_service_data(&aDevice);
    VerifyOrReturnError(serviceData != nullptr, false);

    GAutoPtr<GVariant> dataValue(g_variant_lookup_value(serviceData, Ble::CHIP_BLE_SERVICE_LONG_UUID_STR, nullptr));
    VerifyOrReturnError(dataValue != nullptr, false);

    size_t dataLen         = 0;
    const void * dataBytes = g_variant_get_fixed_array(dataValue.get(), &dataLen, sizeof(uint8_t));
    VerifyOrReturnError(dataBytes != nullptr && dataLen >= sizeof(aDeviceInfo), false);

    memcpy(&aDeviceInfo, dataBytes, sizeof(aDeviceInfo));
    return true;
}

} // namespace

CHIP_ERROR ChipDeviceScanner::Init(BluezAdapter1 * adapter, ChipDeviceScannerDelegate * delegate)
{
    VerifyOrReturnError(adapter != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    // Make this function idempotent by shutting down previously initialized state if any.
    Shutdown();

    mAdapter.reset(reinterpret_cast<BluezAdapter1 *>(g_object_ref(adapter)));
    mDelegate = delegate;

    mScannerState = ChipDeviceScannerState::INITIALIZED;

    return CHIP_NO_ERROR;
}

void ChipDeviceScanner::Shutdown()
{
    VerifyOrReturn(mScannerState != ChipDeviceScannerState::UNINITIALIZED);

    StopScan();

    // Release resources on the glib thread. This is necessary because the D-Bus manager client
    // object handles D-Bus signals. Otherwise, we might face a race when the manager object is
    // released during a D-Bus signal being processed.
    PlatformMgrImpl().GLibMatterContextInvokeSync(
        +[](ChipDeviceScanner * self) {
            self->mAdapter.reset();
            return CHIP_NO_ERROR;
        },
        this);

    mScannerState = ChipDeviceScannerState::UNINITIALIZED;
}

CHIP_ERROR ChipDeviceScanner::StartScan()
{
    assertChipStackLockedByCurrentThread();
    VerifyOrReturnError(mScannerState != ChipDeviceScannerState::SCANNING, CHIP_ERROR_INCORRECT_STATE);

    mCancellable.reset(g_cancellable_new());
    CHIP_ERROR err = PlatformMgrImpl().GLibMatterContextInvokeSync(
        +[](ChipDeviceScanner * self) { return self->StartScanImpl(); }, this);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Ble, "Failed to initiate BLE scan start: %" CHIP_ERROR_FORMAT, err.Format());
        mDelegate->OnScanComplete();
        return err;
    }

    mScannerState = ChipDeviceScannerState::SCANNING;
    ChipLogDetail(Ble, "ChipDeviceScanner has started scanning!");

    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDeviceScanner::StopScan()
{
    assertChipStackLockedByCurrentThread();
    VerifyOrReturnError(mScannerState == ChipDeviceScannerState::SCANNING, CHIP_NO_ERROR);

    CHIP_ERROR err = PlatformMgrImpl().GLibMatterContextInvokeSync(
        +[](ChipDeviceScanner * self) { return self->StopScanImpl(); }, this);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Ble, "Failed to initiate BLE scan stop: %" CHIP_ERROR_FORMAT, err.Format());
        return CHIP_ERROR_INTERNAL;
    }

    // Stop scanning and return to initialization state
    mScannerState = ChipDeviceScannerState::INITIALIZED;

    ChipLogDetail(Ble, "ChipDeviceScanner has stopped scanning!");

    mDelegate->OnScanComplete();

    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDeviceScanner::StopScanImpl()
{

    // In case we are currently running a scan
    g_cancellable_cancel(mCancellable.get());
    mCancellable.reset();

    mObjectManager.UnsubscribeDeviceNotifications(mAdapter.get(), this);

    GAutoPtr<GError> error;
    if (!bluez_adapter1_call_stop_discovery_sync(mAdapter.get(), nullptr /* not cancellable */, &error.GetReceiver()))
    {
        // Do not report error if returned error indicates that the BLE adapter is not available.
        // In such case the scan is already stopped.
        if (BluezCallToChipError(error.get()) != BLE_ERROR_ADAPTER_UNAVAILABLE)
        {
            ChipLogError(Ble, "Failed to stop discovery: %s", error->message);
            return CHIP_ERROR_INTERNAL;
        }
    }

    return CHIP_NO_ERROR;
}

void ChipDeviceScanner::OnDeviceAdded(BluezDevice1 & device)
{
    ReportDevice(device);
}

void ChipDeviceScanner::OnDevicePropertyChanged(BluezDevice1 & device, GVariant * changedProps,
                                                const char * const * invalidatedProps)
{
    ReportDevice(device);
}

void ChipDeviceScanner::ReportDevice(BluezDevice1 & device)
{
    VerifyOrReturn(strcmp(bluez_device1_get_adapter(&device),
                          g_dbus_proxy_get_object_path(reinterpret_cast<GDBusProxy *>(mAdapter.get()))) == 0);

    chip::Ble::ChipBLEDeviceIdentificationInfo deviceInfo;

    if (!BluezGetChipDeviceInfo(device, deviceInfo))
    {
        ChipLogDetail(Ble, "Device %s does not look like a CHIP device.", bluez_device1_get_address(&device));
        return;
    }

    mDelegate->OnDeviceScanned(device, deviceInfo);
}

void ChipDeviceScanner::RemoveDevice(BluezDevice1 & device)
{
    VerifyOrReturn(strcmp(bluez_device1_get_adapter(&device),
                          g_dbus_proxy_get_object_path(reinterpret_cast<GDBusProxy *>(mAdapter.get()))) == 0);

    chip::Ble::ChipBLEDeviceIdentificationInfo deviceInfo;

    if (!BluezGetChipDeviceInfo(device, deviceInfo))
    {
        return;
    }

    GAutoPtr<GError> error;
    const auto devicePath = g_dbus_proxy_get_object_path(reinterpret_cast<GDBusProxy *>(&device));
    if (!bluez_adapter1_call_remove_device_sync(mAdapter.get(), devicePath, nullptr, &error.GetReceiver()))
    {
        ChipLogDetail(Ble, "Failed to remove device %s: %s", StringOrNullMarker(devicePath), error->message);
    }
}

CHIP_ERROR ChipDeviceScanner::StartScanImpl()
{
    CHIP_ERROR err = mObjectManager.SubscribeDeviceNotifications(mAdapter.get(), this);
    ReturnErrorOnFailure(err);

    ChipLogProgress(Ble, "BLE removing known devices");
    for (BluezObject & object : mObjectManager.GetObjects())
    {
        GAutoPtr<BluezDevice1> device(bluez_object_get_device1(&object));
        if (device)
        {
            RemoveDevice(*device.get());
        }
    }

    // Search for LE only.
    // Do NOT add filtering by UUID as it is done by the following kernel function:
    // https://github.com/torvalds/linux/blob/bdb575f872175ed0ecf2638369da1cb7a6e86a14/net/bluetooth/mgmt.c#L9258.
    // The function requires that devices advertise its services' UUIDs in UUID16/32/128 fields
    // while the Matter specification requires only FLAGS (0x01) and SERVICE_DATA_16 (0x16) fields
    // in the advertisement packets.
    GVariantBuilder filterBuilder;
    g_variant_builder_init(&filterBuilder, G_VARIANT_TYPE("a{sv}"));
    g_variant_builder_add(&filterBuilder, "{sv}", "Transport", g_variant_new_string("le"));
    GVariant * filter = g_variant_builder_end(&filterBuilder);

    GAutoPtr<GError> error;
    if (!bluez_adapter1_call_set_discovery_filter_sync(mAdapter.get(), filter, mCancellable.get(), &error.GetReceiver()))
    {
        // Not critical: ignore if fails
        ChipLogError(Ble, "Failed to set discovery filters: %s", error->message);
        error.reset();
    }

    ChipLogProgress(Ble, "BLE initiating scan");
    if (!bluez_adapter1_call_start_discovery_sync(mAdapter.get(), mCancellable.get(), &error.GetReceiver()))
    {
        ChipLogError(Ble, "Failed to start discovery: %s", error->message);
        return BluezCallToChipError(error.get());
    }

    return CHIP_NO_ERROR;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
