/* @@@LICENSE
 *
 * Copyright (c) 2017-2025 LG Electronics, Inc.
 *
 * Confidential computer software. Valid license from LG required for
 * possession, use or copying. Consistent with FAR 12.211 and 12.212,
 * Commercial Computer Software, Computer Software Documentation, and
 * Technical Data for Commercial Items are licensed to the U.S. Government
 * under vendor's standard commercial license.
 *
 * LICENSE@@@
 */

#pragma once
#include "LsRequester.h"
#include "WbsConnection.h"
#include "WebosLockTracker.h"
#include <ble/Ble.h>
#include <glib.h>
#include <lib/core/CHIPError.h>
#include <luna-service2++/handle.hpp>
#include <luna-service2/lunaservice.h>
#include <pbnjson.hpp>
#include <platform/CHIPDeviceConfig.h>
#include <system/SystemLayer.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

struct BLEChipDevice
{
    BLEChipDevice(pbnjson::JValue & device, chip::Ble::ChipBLEDeviceIdentificationInfo deviceInfo) :
        mBleDevice(device), mDeviceInfo(deviceInfo)
    {}
    pbnjson::JValue mBleDevice;
    chip::Ble::ChipBLEDeviceIdentificationInfo mDeviceInfo;
};

/// Receives callbacks when chip devices are being scanned
class WbsDeviceScannerDelegate
{
public:
    virtual ~WbsDeviceScannerDelegate() {}

    // Called when a CHIP device was found
    virtual void OnDeviceScanned(const pbnjson::JValue & device, const chip::Ble::ChipBLEDeviceIdentificationInfo & info) = 0;

    // Called when a scan was completed (stopped or timed out)
    virtual void OnScanComplete() = 0;

    // Call on scan error
    virtual void OnScanError(CHIP_ERROR) = 0;
};

/// Allows scanning for CHIP devices
///
/// Will perform scan operations and call back whenever a device is discovered.
class WbsDeviceScanner
{
public:
    WbsDeviceScanner()                                     = default;
    WbsDeviceScanner(WbsDeviceScanner &&)                  = default;
    WbsDeviceScanner(const WbsDeviceScanner &)             = delete;
    WbsDeviceScanner & operator=(const WbsDeviceScanner &) = delete;

    ~WbsDeviceScanner() { Shutdown(); }

    /// Initialize the scanner.
    CHIP_ERROR Init(WbsDeviceScannerDelegate * delegate);

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
    bool IsScanning() const { return mScannerState == WbsDeviceScannerState::SCANNING; }
    void setAddress(const std::string & aAddr) { mAddress = aAddr; }

private:
    enum class WbsDeviceScannerState
    {
        UNINITIALIZED,
        INITIALIZED,
        SCANNING
    };
    static bool OnLeDeviceScanned(LSHandle * sh, LSMessage * message, void * ctx);
    CHIP_ERROR StartScanImpl();
    CHIP_ERROR StopScanImpl();

    /// Check if a given device is a CHIP device and if yes, report it as discovered
    void ReportDevice(const pbnjson::JValue & device);

    /// Check if a given device is a CHIP device and if yes, remove it from the adapter
    /// so that it can be re-discovered if it's still advertising.
    void RemoveDevice(const pbnjson::JValue & device);

    WbsDeviceScannerDelegate * mDelegate = nullptr;
    WbsDeviceScannerState mScannerState  = WbsDeviceScannerState::UNINITIALIZED;
    BLEChipDevice * mBleChipDevice       = nullptr;
    uint32_t mLeInternalStartScanToken;
    std::string mAddress;
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
