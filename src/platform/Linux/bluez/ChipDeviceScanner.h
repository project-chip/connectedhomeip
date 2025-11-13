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

#include <ble/Ble.h>
#include <lib/core/CHIPError.h>
#include <platform/GLibTypeDeleter.h>
#include <platform/Linux/dbus/bluez/DBusBluez.h>
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
class ChipDeviceScanner : public BluezObjectManagerAdapterNotificationsDelegate
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
    CHIP_ERROR StartScan();

    /// Stop any currently running scan
    CHIP_ERROR StopScan();

    /// Check if the scanner is active
    bool IsScanning() const { return mScannerState == ChipDeviceScannerState::SCANNING; }

    /// Members that implement virtual methods on BluezObjectManagerAdapterNotificationsDelegate
    void OnDeviceAdded(BluezDevice1 & device) override;
    void OnDevicePropertyChanged(BluezDevice1 & device, GVariant * changedProps, const char * const * invalidatedProps) override;
    void OnDeviceRemoved(BluezDevice1 & device) override {}

private:
    enum class ChipDeviceScannerState
    {
        UNINITIALIZED,
        INITIALIZED,
        SCANNING
    };

    CHIP_ERROR StartScanImpl();
    CHIP_ERROR StopScanImpl();

    /// Check if a given device is a CHIP device and if yes, report it as discovered
    void ReportDevice(BluezDevice1 & device);

    /// Check if a given device is a CHIP device and if yes, remove it from the adapter
    /// so that it can be re-discovered if it's still advertising.
    void RemoveDevice(BluezDevice1 & device);

    BluezObjectManager & mObjectManager;
    GAutoPtr<BluezAdapter1> mAdapter;

    ChipDeviceScannerDelegate * mDelegate = nullptr;
    ChipDeviceScannerState mScannerState  = ChipDeviceScannerState::UNINITIALIZED;
    GAutoPtr<GCancellable> mCancellable;
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
