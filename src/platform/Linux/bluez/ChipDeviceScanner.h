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

#include <glib.h>

#include <ble/CHIPBleServiceData.h>
#include <lib/core/CHIPError.h>
#include <platform/GLibTypeDeleter.h>
#include <platform/Linux/dbus/bluez/DbusBluez.h>
#include <system/SystemLayer.h>

#include "BluezObjectManager.h"
#include "Types.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

/// Receives callbacks when chip devices are being scanned
class ChipDeviceScannerDelegate
{
public:
    virtual ~ChipDeviceScannerDelegate() {}

    // Called when a CHIP device was found
    virtual void OnDeviceScanned(BluezDevice1 & device, const chip::Ble::ChipBLEDeviceIdentificationInfo & info) = 0;

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
    ChipDeviceScanner(BluezObjectManager & aObjectManager) : mObjectManager(aObjectManager) {}
    ChipDeviceScanner(ChipDeviceScanner &&)                  = default;
    ChipDeviceScanner(const ChipDeviceScanner &)             = delete;
    ChipDeviceScanner & operator=(const ChipDeviceScanner &) = delete;

    ~ChipDeviceScanner() { Shutdown(); }

    /// Initialize the scanner.
    CHIP_ERROR Init(BluezAdapter1 * adapter, ChipDeviceScannerDelegate * delegate);

    /// Release any resources associated with the scanner.
    void Shutdown();

    /// Initiate a scan for devices, with the given timeout
    ///
    /// This method must be called while in the Matter context (from the Matter event
    /// loop, or while holding the Matter stack lock).
    CHIP_ERROR StartScan(System::Clock::Timeout timeout);

    /// Stop any currently running scan
    CHIP_ERROR StopScan();

private:
    enum ChipDeviceScannerState
    {
        SCANNER_UNINITIALIZED,
        SCANNER_INITIALIZED,
        SCANNER_SCANNING
    };

    enum ScannerTimerState
    {
        TIMER_CANCELED,
        TIMER_STARTED,
        TIMER_EXPIRED
    };

    CHIP_ERROR StartScanImpl();
    CHIP_ERROR StopScanImpl();
    static void TimerExpiredCallback(chip::System::Layer * layer, void * appState);

    void SignalObjectAdded(GDBusObjectManager * aManager, GDBusObject * aObject);
    void SignalInterfacePropertiesChanged(GDBusObjectManagerClient * aManager, GDBusObjectProxy * aObject, GDBusProxy * aInterface,
                                          GVariant * aChangedProperties, const char * const * aInvalidatedProps);

    /// Check if a given device is a CHIP device and if yes, report it as discovered
    void ReportDevice(BluezDevice1 & device);

    /// Check if a given device is a CHIP device and if yes, remove it from the adapter
    /// so that it can be re-discovered if it's still advertising.
    void RemoveDevice(BluezDevice1 & device);

    BluezObjectManager & mObjectManager;
    GAutoPtr<BluezAdapter1> mAdapter;

    ChipDeviceScannerDelegate * mDelegate  = nullptr;
    unsigned long mObjectAddedSignal       = 0;
    unsigned long mPropertiesChangedSignal = 0;
    ChipDeviceScannerState mScannerState   = ChipDeviceScannerState::SCANNER_UNINITIALIZED;
    /// Used to track if timer has already expired and doesn't need to be canceled.
    ScannerTimerState mTimerState = ScannerTimerState::TIMER_CANCELED;
    GAutoPtr<GCancellable> mCancellable;
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
