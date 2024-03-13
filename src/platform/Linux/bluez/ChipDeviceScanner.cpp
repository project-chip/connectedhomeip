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

#include <errno.h>
#include <pthread.h>

#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/GLibTypeDeleter.h>

#include "BluezObjectList.h"
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

    // Make this function idempotent by shutting down previously initialized state if any.
    Shutdown();

    mAdapter.reset(reinterpret_cast<BluezAdapter1 *>(g_object_ref(adapter)));
    mDelegate = delegate;

    // Create the D-Bus object manager client object on the glib thread, so that all D-Bus signals
    // will be delivered to the glib thread.
    ReturnErrorOnFailure(PlatformMgrImpl().GLibMatterContextInvokeSync(
        +[](ChipDeviceScanner * self) {
            // When creating D-Bus proxy object, the thread default context must be initialized.
            VerifyOrDie(g_main_context_get_thread_default() != nullptr);

            GAutoPtr<GError> err;
            self->mManager = g_dbus_object_manager_client_new_for_bus_sync(
                G_BUS_TYPE_SYSTEM, G_DBUS_OBJECT_MANAGER_CLIENT_FLAGS_NONE, BLUEZ_INTERFACE, "/",
                bluez_object_manager_client_get_proxy_type, nullptr /* unused user data in the Proxy Type Func */,
                nullptr /* destroy notify */, nullptr /* cancellable */, &err.GetReceiver());
            VerifyOrReturnError(self->mManager != nullptr, CHIP_ERROR_INTERNAL,
                                ChipLogError(Ble, "Failed to get D-Bus object manager for device scanning: %s", err->message));
            return CHIP_NO_ERROR;
        },
        this));

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
            if (self->mManager != nullptr)
                g_object_unref(self->mManager);
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
    if (PlatformMgrImpl().GLibMatterContextInvokeSync(MainLoopStartScan, this) != CHIP_NO_ERROR)
    {
        ChipLogError(Ble, "Failed to schedule BLE scan start.");

        ChipDeviceScannerDelegate * delegate = this->mDelegate;
        // callback is explicitly allowed to delete the scanner (hence no more
        // references to 'self' here)
        delegate->OnScanComplete();

        return CHIP_ERROR_INTERNAL;
    }

    // Here need to set the Bluetooth scanning status immediately.
    // So that if the timer fails to start in the next step,
    // calling StopScan will be effective.
    mScannerState = ChipDeviceScannerState::SCANNER_SCANNING;

    CHIP_ERROR err = chip::DeviceLayer::SystemLayer().StartTimer(timeout, TimerExpiredCallback, static_cast<void *>(this));

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Ble, "Failed to schedule scan timeout.");
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

    if (PlatformMgrImpl().GLibMatterContextInvokeSync(MainLoopStopScan, this) != CHIP_NO_ERROR)
    {
        ChipLogError(Ble, "Failed to schedule BLE scan stop.");
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

    ChipDeviceScannerDelegate * delegate = this->mDelegate;
    // callback is explicitly allowed to delete the scanner (hence no more
    // references to 'self' here)
    delegate->OnScanComplete();

    return CHIP_NO_ERROR;
}

CHIP_ERROR ChipDeviceScanner::MainLoopStopScan(ChipDeviceScanner * self)
{
    GAutoPtr<GError> error;

    // In case we are currently running a scan
    g_cancellable_cancel(self->mCancellable.get());
    self->mCancellable.reset();

    if (self->mObjectAddedSignal)
    {
        g_signal_handler_disconnect(self->mManager, self->mObjectAddedSignal);
        self->mObjectAddedSignal = 0;
    }

    if (self->mInterfaceChangedSignal)
    {
        g_signal_handler_disconnect(self->mManager, self->mInterfaceChangedSignal);
        self->mInterfaceChangedSignal = 0;
    }

    if (!bluez_adapter1_call_stop_discovery_sync(self->mAdapter.get(), nullptr /* not cancellable */, &error.GetReceiver()))
    {
        ChipLogError(Ble, "Failed to stop discovery %s", error->message);
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

void ChipDeviceScanner::SignalObjectAdded(GDBusObjectManager * manager, GDBusObject * object, ChipDeviceScanner * self)
{
    GAutoPtr<BluezDevice1> device(bluez_object_get_device1(reinterpret_cast<BluezObject *>(object)));
    VerifyOrReturn(device.get() != nullptr);

    self->ReportDevice(*device.get());
}

void ChipDeviceScanner::SignalInterfaceChanged(GDBusObjectManagerClient * manager, GDBusObjectProxy * object,
                                               GDBusProxy * aInterface, GVariant * aChangedProperties,
                                               const gchar * const * aInvalidatedProps, ChipDeviceScanner * self)
{
    GAutoPtr<BluezDevice1> device(bluez_object_get_device1(reinterpret_cast<BluezObject *>(object)));
    VerifyOrReturn(device.get() != nullptr);

    self->ReportDevice(*device.get());
}

void ChipDeviceScanner::ReportDevice(BluezDevice1 & device)
{
    if (strcmp(bluez_device1_get_adapter(&device), g_dbus_proxy_get_object_path(G_DBUS_PROXY(mAdapter.get()))) != 0)
    {
        return;
    }

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
    if (strcmp(bluez_device1_get_adapter(&device), g_dbus_proxy_get_object_path(G_DBUS_PROXY(mAdapter.get()))) != 0)
    {
        return;
    }

    chip::Ble::ChipBLEDeviceIdentificationInfo deviceInfo;

    if (!BluezGetChipDeviceInfo(device, deviceInfo))
    {
        return;
    }

    const auto devicePath = g_dbus_proxy_get_object_path(G_DBUS_PROXY(&device));
    GAutoPtr<GError> error;

    if (!bluez_adapter1_call_remove_device_sync(mAdapter.get(), devicePath, nullptr, &error.GetReceiver()))
    {
        ChipLogDetail(Ble, "Failed to remove device %s: %s", StringOrNullMarker(devicePath), error->message);
    }
}

CHIP_ERROR ChipDeviceScanner::MainLoopStartScan(ChipDeviceScanner * self)
{
    GAutoPtr<GError> error;

    self->mObjectAddedSignal = g_signal_connect(self->mManager, "object-added", G_CALLBACK(SignalObjectAdded), self);
    self->mInterfaceChangedSignal =
        g_signal_connect(self->mManager, "interface-proxy-properties-changed", G_CALLBACK(SignalInterfaceChanged), self);

    ChipLogProgress(Ble, "BLE removing known devices.");
    for (BluezObject & object : BluezObjectList(self->mManager))
    {
        GAutoPtr<BluezDevice1> device(bluez_object_get_device1(&object));
        if (device.get() != nullptr)
        {
            self->RemoveDevice(*device.get());
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

    if (!bluez_adapter1_call_set_discovery_filter_sync(self->mAdapter.get(), filter, self->mCancellable.get(),
                                                       &error.GetReceiver()))
    {
        // Not critical: ignore if fails
        ChipLogError(Ble, "Failed to set discovery filters: %s", error->message);
        error.reset();
    }

    ChipLogProgress(Ble, "BLE initiating scan.");
    if (!bluez_adapter1_call_start_discovery_sync(self->mAdapter.get(), self->mCancellable.get(), &error.GetReceiver()))
    {
        ChipLogError(Ble, "Failed to start discovery: %s", error->message);
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
