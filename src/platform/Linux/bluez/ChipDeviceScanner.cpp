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

#include <ble/BleError.h>
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

    GAutoPtr<GVariant> dataValue(g_variant_lookup_value(serviceData, CHIP_BLE_UUID_SERVICE_STRING, nullptr));
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

    mScannerState = ChipDeviceScannerState::SCANNER_INITIALIZED;

    return CHIP_NO_ERROR;
}

void ChipDeviceScanner::Shutdown()
{
    VerifyOrReturn(mScannerState != ChipDeviceScannerState::SCANNER_UNINITIALIZED);

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

    mScannerState = ChipDeviceScannerState::SCANNER_UNINITIALIZED;
}

CHIP_ERROR ChipDeviceScanner::StartScan(System::Clock::Timeout timeout)
{
    assertChipStackLockedByCurrentThread();
    VerifyOrReturnError(mScannerState != ChipDeviceScannerState::SCANNER_SCANNING, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mTimerState == ScannerTimerState::TIMER_CANCELED, CHIP_ERROR_INCORRECT_STATE);

    mCancellable.reset(g_cancellable_new());
    CHIP_ERROR err = PlatformMgrImpl().GLibMatterContextInvokeSync(
        +[](ChipDeviceScanner * self) { return self->StartScanImpl(); }, this);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Ble, "Failed to initiate BLE scan start: %" CHIP_ERROR_FORMAT, err.Format());
        mDelegate->OnScanComplete();
        return err;
    }

    // Here need to set the Bluetooth scanning status immediately.
    // So that if the timer fails to start in the next step,
    // calling StopScan will be effective.
    mScannerState = ChipDeviceScannerState::SCANNER_SCANNING;

    err = chip::DeviceLayer::SystemLayer().StartTimer(timeout, TimerExpiredCallback, static_cast<void *>(this));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Ble, "Failed to schedule scan timeout: %" CHIP_ERROR_FORMAT, err.Format());
        StopScan();
        return err;
    }

    mTimerState = ScannerTimerState::TIMER_STARTED;

    ChipLogDetail(Ble, "ChipDeviceScanner has started scanning!");

    return CHIP_NO_ERROR;
}

void ChipDeviceScanner::TimerExpiredCallback(chip::System::Layer * layer, void * appState)
{
    ChipDeviceScanner * chipDeviceScanner = static_cast<ChipDeviceScanner *>(appState);
    chipDeviceScanner->mTimerState        = ScannerTimerState::TIMER_EXPIRED;
    chipDeviceScanner->mDelegate->OnScanError(CHIP_ERROR_TIMEOUT);
    chipDeviceScanner->StopScan();
}

CHIP_ERROR ChipDeviceScanner::StopScan()
{
    assertChipStackLockedByCurrentThread();
    VerifyOrReturnError(mScannerState == ChipDeviceScannerState::SCANNER_SCANNING, CHIP_NO_ERROR);

    CHIP_ERROR err = PlatformMgrImpl().GLibMatterContextInvokeSync(
        +[](ChipDeviceScanner * self) { return self->StopScanImpl(); }, this);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Ble, "Failed to initiate BLE scan stop: %" CHIP_ERROR_FORMAT, err.Format());
        return CHIP_ERROR_INTERNAL;
    }

    // Stop scanning and return to initialization state
    mScannerState = ChipDeviceScannerState::SCANNER_INITIALIZED;

    ChipLogDetail(Ble, "ChipDeviceScanner has stopped scanning!");

    if (mTimerState == ScannerTimerState::TIMER_STARTED)
    {
        chip::DeviceLayer::SystemLayer().CancelTimer(TimerExpiredCallback, this);
    }

    // Reset timer status
    mTimerState = ScannerTimerState::TIMER_CANCELED;

    mDelegate->OnScanComplete();

    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDeviceScanner::StopScanImpl()
{

    // In case we are currently running a scan
    g_cancellable_cancel(mCancellable.get());
    mCancellable.reset();

    if (mObjectAddedSignal)
    {
        g_signal_handler_disconnect(mObjectManager.GetObjectManager(), mObjectAddedSignal);
        mObjectAddedSignal = 0;
    }

    if (mPropertiesChangedSignal)
    {
        g_signal_handler_disconnect(mObjectManager.GetObjectManager(), mPropertiesChangedSignal);
        mPropertiesChangedSignal = 0;
    }

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

void ChipDeviceScanner::SignalObjectAdded(GDBusObjectManager * aManager, GDBusObject * aObject)
{
    GAutoPtr<BluezDevice1> device(bluez_object_get_device1(reinterpret_cast<BluezObject *>(aObject)));
    VerifyOrReturn(device);

    ReportDevice(*device.get());
}

void ChipDeviceScanner::SignalInterfacePropertiesChanged(GDBusObjectManagerClient * aManager, GDBusObjectProxy * aObject,
                                                         GDBusProxy * aInterface, GVariant * aChangedProperties,
                                                         const char * const * aInvalidatedProps)
{
    GAutoPtr<BluezDevice1> device(bluez_object_get_device1(reinterpret_cast<BluezObject *>(aObject)));
    VerifyOrReturn(device);

    ReportDevice(*device.get());
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

    mObjectAddedSignal = g_signal_connect(mObjectManager.GetObjectManager(), "object-added",
                                          G_CALLBACK(+[](GDBusObjectManager * aMgr, GDBusObject * aObj, ChipDeviceScanner * self) {
                                              return self->SignalObjectAdded(aMgr, aObj);
                                          }),
                                          this);

    mPropertiesChangedSignal = g_signal_connect(
        mObjectManager.GetObjectManager(), "interface-proxy-properties-changed",
        G_CALLBACK(+[](GDBusObjectManagerClient * aMgr, GDBusObjectProxy * aObj, GDBusProxy * aIface, GVariant * aChangedProps,
                       const char * const * aInvalidatedProps, ChipDeviceScanner * self) {
            return self->SignalInterfacePropertiesChanged(aMgr, aObj, aIface, aChangedProps, aInvalidatedProps);
        }),
        this);

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
