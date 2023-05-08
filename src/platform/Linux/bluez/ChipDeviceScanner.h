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

#pragma once

#include <platform/CHIPDeviceConfig.h>

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#include <glib.h>
#include <memory>

#include <ble/CHIPBleServiceData.h>
#include <lib/core/CHIPError.h>
#include <platform/Linux/dbus/bluez/DbusBluez.h>
#include <system/SystemLayer.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

/// Receives callbacks when chip devices are being scanned
class ChipDeviceScannerDelegate
{
public:
    virtual ~ChipDeviceScannerDelegate() {}

    // Called when a CHIP device was found
    virtual void OnDeviceScanned(BluezDevice1 * device, const chip::Ble::ChipBLEDeviceIdentificationInfo & info) = 0;

    // Called when a scan was completed (stopped or timed out)
    virtual void OnScanComplete() = 0;

    // Call on scan error
    virtual void OnScanError(CHIP_ERROR) = 0;
};

/// Allows scanning for CHIP devices
///
/// Will perform scan operations and call back whenever a device is discovered.
class ChipDeviceScanner
{
public:
    /// NOTE: prefer to use the  ::Create method instead direct constructor calling.
    ChipDeviceScanner(GDBusObjectManager * manager, BluezAdapter1 * adapter, GCancellable * cancellable,
                      ChipDeviceScannerDelegate * delegate);

    ChipDeviceScanner(ChipDeviceScanner &&)      = default;
    ChipDeviceScanner(const ChipDeviceScanner &) = delete;
    ChipDeviceScanner & operator=(const ChipDeviceScanner &) = delete;

    ~ChipDeviceScanner();

    /// Initiate a scan for devices, with the given timeout
    CHIP_ERROR StartScan(System::Clock::Timeout timeout);

    /// Stop any currently running scan
    CHIP_ERROR StopScan();

    /// Create a new device scanner
    ///
    /// Convenience method to allocate any required variables.
    /// On success, maintains a reference to the provided adapter.
    static std::unique_ptr<ChipDeviceScanner> Create(BluezAdapter1 * adapter, ChipDeviceScannerDelegate * delegate);

private:
    static void TimerExpiredCallback(chip::System::Layer * layer, void * appState);
    static CHIP_ERROR MainLoopStartScan(ChipDeviceScanner * self);
    static CHIP_ERROR MainLoopStopScan(ChipDeviceScanner * self);
    static void SignalObjectAdded(GDBusObjectManager * manager, GDBusObject * object, ChipDeviceScanner * self);
    static void SignalInterfaceChanged(GDBusObjectManagerClient * manager, GDBusObjectProxy * object, GDBusProxy * aInterface,
                                       GVariant * aChangedProperties, const gchar * const * aInvalidatedProps,
                                       ChipDeviceScanner * self);

    /// Check if a given device is a CHIP device and if yes, report it as discovered
    void ReportDevice(BluezDevice1 * device);

    /// Check if a given device is a CHIP device and if yes, remove it from the adapter
    /// so that it can be re-discovered if it's still advertising.
    void RemoveDevice(BluezDevice1 * device);

    GDBusObjectManager * mManager         = nullptr;
    BluezAdapter1 * mAdapter              = nullptr;
    GCancellable * mCancellable           = nullptr;
    ChipDeviceScannerDelegate * mDelegate = nullptr;
    gulong mObjectAddedSignal             = 0;
    gulong mInterfaceChangedSignal        = 0;
    bool mIsScanning                      = false;
    bool mIsStopping                      = false;
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
