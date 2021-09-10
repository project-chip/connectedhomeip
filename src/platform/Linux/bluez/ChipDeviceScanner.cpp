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

#include "ChipDeviceScanner.h"

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#include "BluezObjectList.h"
#include "MainLoop.h"
#include "Types.h"

#include <errno.h>
#include <lib/support/logging/CHIPLogging.h>
#include <pthread.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {
namespace {

struct GObjectUnref
{
    template <typename T>
    void operator()(T * value)
    {
        g_object_unref(value);
    }
};

using GCancellableUniquePtr       = std::unique_ptr<GCancellable, GObjectUnref>;
using GDBusObjectManagerUniquePtr = std::unique_ptr<GDBusObjectManager, GObjectUnref>;

/// Retrieve CHIP device identification info from the device advertising data
bool BluezGetChipDeviceInfo(BluezDevice1 & aDevice, chip::Ble::ChipBLEDeviceIdentificationInfo & aDeviceInfo)
{
    GVariant * serviceData = bluez_device1_get_service_data(&aDevice);
    VerifyOrReturnError(serviceData != nullptr, false);

    GVariant * dataValue = g_variant_lookup_value(serviceData, CHIP_BLE_UUID_SERVICE_STRING, nullptr);
    VerifyOrReturnError(dataValue != nullptr, false);

    size_t dataLen         = 0;
    const void * dataBytes = g_variant_get_fixed_array(dataValue, &dataLen, sizeof(uint8_t));
    VerifyOrReturnError(dataBytes != nullptr && dataLen >= sizeof(aDeviceInfo), false);

    memcpy(&aDeviceInfo, dataBytes, sizeof(aDeviceInfo));
    return true;
}

} // namespace

ChipDeviceScanner::ChipDeviceScanner(GDBusObjectManager * manager, BluezAdapter1 * adapter, GCancellable * cancellable,
                                     ChipDeviceScannerDelegate * delegate) :
    mManager(manager),
    mAdapter(adapter), mCancellable(cancellable), mDelegate(delegate)
{
    g_object_ref(mAdapter);
    g_object_ref(mCancellable);
    g_object_ref(mManager);
}

ChipDeviceScanner::~ChipDeviceScanner()
{
    StopScan();

    // In case the timeout timer is still active
    chip::DeviceLayer::SystemLayer.CancelTimer(TimerExpiredCallback, this);

    g_object_unref(mManager);
    g_object_unref(mCancellable);
    g_object_unref(mAdapter);

    mManager     = nullptr;
    mAdapter     = nullptr;
    mCancellable = nullptr;
    mDelegate    = nullptr;
}

std::unique_ptr<ChipDeviceScanner> ChipDeviceScanner::Create(BluezAdapter1 * adapter, ChipDeviceScannerDelegate * delegate)
{
    GError * error = nullptr;

    GCancellableUniquePtr cancellable(g_cancellable_new(), GObjectUnref());

    if (!cancellable)
    {
        return std::unique_ptr<ChipDeviceScanner>();
    }

    GDBusObjectManagerUniquePtr manager(
        g_dbus_object_manager_client_new_for_bus_sync(G_BUS_TYPE_SYSTEM, G_DBUS_OBJECT_MANAGER_CLIENT_FLAGS_NONE, BLUEZ_INTERFACE,
                                                      "/", bluez_object_manager_client_get_proxy_type,
                                                      nullptr /* unused user data in the Proxy Type Func */,
                                                      nullptr /*destroy notify */, cancellable.get(), &error),
        GObjectUnref());
    if (!manager)
    {
        ChipLogError(Ble, "Failed to get DBUS object manager for device scanning: %s", error->message);
        g_error_free(error);
        return std::unique_ptr<ChipDeviceScanner>();
    }

    return std::make_unique<ChipDeviceScanner>(manager.get(), adapter, cancellable.get(), delegate);
}

CHIP_ERROR ChipDeviceScanner::StartScan(unsigned timeoutMs)
{
    ReturnErrorCodeIf(mIsScanning, CHIP_ERROR_INCORRECT_STATE);

    ReturnErrorOnFailure(MainLoop::Instance().EnsureStarted());

    mIsScanning = true; // optimistic, to allow all callbacks to check this
    if (!MainLoop::Instance().Schedule(MainLoopStartScan, this))
    {
        ChipLogError(Ble, "Failed to schedule BLE scan start.");
        mIsScanning = false;
        return CHIP_ERROR_INTERNAL;
    }

    CHIP_ERROR err = chip::DeviceLayer::SystemLayer.StartTimer(timeoutMs, TimerExpiredCallback, static_cast<void *>(this));

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Ble, "Failed to schedule scan timeout.");
        StopScan();
        return err;
    }

    return CHIP_NO_ERROR;
}

void ChipDeviceScanner::TimerExpiredCallback(chip::System::Layer * layer, void * appState)
{
    static_cast<ChipDeviceScanner *>(appState)->StopScan();
}

CHIP_ERROR ChipDeviceScanner::StopScan()
{
    ReturnErrorCodeIf(!mIsScanning, CHIP_NO_ERROR);
    ReturnErrorCodeIf(mIsStopping, CHIP_NO_ERROR);
    mIsStopping = true;
    g_cancellable_cancel(mCancellable); // in case we are currently running a scan

    if (mObjectAddedSignal)
    {
        g_signal_handler_disconnect(mManager, mObjectAddedSignal);
        mObjectAddedSignal = 0;
    }

    if (mInterfaceChangedSignal)
    {
        g_signal_handler_disconnect(mManager, mInterfaceChangedSignal);
        mInterfaceChangedSignal = 0;
    }

    if (!MainLoop::Instance().ScheduleAndWait(MainLoopStopScan, this))
    {
        ChipLogError(Ble, "Failed to schedule BLE scan stop.");
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

int ChipDeviceScanner::MainLoopStopScan(ChipDeviceScanner * self)
{
    GError * error = nullptr;

    if (!bluez_adapter1_call_stop_discovery_sync(self->mAdapter, nullptr /* not cancellable */, &error))
    {
        ChipLogError(Ble, "Failed to stop discovery %s", error->message);
        g_error_free(error);
    }
    ChipDeviceScannerDelegate * delegate = self->mDelegate;
    self->mIsScanning                    = false;

    // callback is explicitly allowed to delete the scanner (hence no more
    // references to 'self' here)
    delegate->OnScanComplete();

    return 0;
}

void ChipDeviceScanner::SignalObjectAdded(GDBusObjectManager * manager, GDBusObject * object, ChipDeviceScanner * self)
{
    self->ReportDevice(bluez_object_get_device1(BLUEZ_OBJECT(object)));
}

void ChipDeviceScanner::SignalInterfaceChanged(GDBusObjectManagerClient * manager, GDBusObjectProxy * object,
                                               GDBusProxy * aInterface, GVariant * aChangedProperties,
                                               const gchar * const * aInvalidatedProps, ChipDeviceScanner * self)
{
    self->ReportDevice(bluez_object_get_device1(BLUEZ_OBJECT(object)));
}

void ChipDeviceScanner::ReportDevice(BluezDevice1 * device)
{
    if (device == nullptr)
    {
        return;
    }

    if (strcmp(bluez_device1_get_adapter(device), g_dbus_proxy_get_object_path(G_DBUS_PROXY(mAdapter))) != 0)
    {
        return;
    }

    chip::Ble::ChipBLEDeviceIdentificationInfo deviceInfo;

    if (!BluezGetChipDeviceInfo(*device, deviceInfo))
    {
        ChipLogDetail(Ble, "Device %s does not look like a CHIP device.", bluez_device1_get_address(device));
        return;
    }

    mDelegate->OnDeviceScanned(device, deviceInfo);
}

void ChipDeviceScanner::RemoveDevice(BluezDevice1 * device)
{
    if (device == nullptr)
    {
        return;
    }

    if (strcmp(bluez_device1_get_adapter(device), g_dbus_proxy_get_object_path(G_DBUS_PROXY(mAdapter))) != 0)
    {
        return;
    }

    chip::Ble::ChipBLEDeviceIdentificationInfo deviceInfo;

    if (!BluezGetChipDeviceInfo(*device, deviceInfo))
    {
        return;
    }

    const auto devicePath = g_dbus_proxy_get_object_path(G_DBUS_PROXY(device));
    GError * error        = nullptr;

    if (!bluez_adapter1_call_remove_device_sync(mAdapter, devicePath, nullptr, &error))
    {
        ChipLogDetail(Ble, "Failed to remove device %s: %s", devicePath, error->message);
        g_error_free(error);
    }
}

int ChipDeviceScanner::MainLoopStartScan(ChipDeviceScanner * self)
{
    GError * error = nullptr;

    self->mObjectAddedSignal = g_signal_connect(self->mManager, "object-added", G_CALLBACK(SignalObjectAdded), self);
    self->mInterfaceChangedSignal =
        g_signal_connect(self->mManager, "interface-proxy-properties-changed", G_CALLBACK(SignalInterfaceChanged), self);

    ChipLogProgress(Ble, "BLE removing known devices.");
    for (BluezObject & object : BluezObjectList(self->mManager))
    {
        self->RemoveDevice(bluez_object_get_device1(&object));
    }

    ChipLogProgress(Ble, "BLE initiating scan.");
    if (!bluez_adapter1_call_start_discovery_sync(self->mAdapter, self->mCancellable, &error))
    {
        ChipLogError(Ble, "Failed to start discovery: %s", error->message);
        g_error_free(error);

        self->mIsScanning = false;
        self->mDelegate->OnScanComplete();
    }

    return 0;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
