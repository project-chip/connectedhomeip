/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#include <system/SystemLayer.h>

#include <luna-service2/lunaservice.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

/// Receives callbacks when chip devices are being scanned
class ChipDeviceScannerDelegate
{
public:
    virtual ~ChipDeviceScannerDelegate() {}

    virtual void OnChipDeviceScanned(char * address) = 0;

    // Called when a scan was completed (stopped or timed out)
    virtual void OnScanComplete() = 0;

    // Called on scan error
    virtual void OnScanError(CHIP_ERROR err) = 0;
};

/// Allows scanning for CHIP devices
///
/// Will perform scan operations and call back whenever a device is discovered.
class ChipDeviceScanner
{
public:
    ChipDeviceScanner(LSHandle * handle, ChipDeviceScannerDelegate * delegate);
    ChipDeviceScanner(ChipDeviceScannerDelegate * delegate);

    ChipDeviceScanner(ChipDeviceScanner &&)      = default;
    ChipDeviceScanner(const ChipDeviceScanner &) = delete;
    ChipDeviceScanner & operator=(const ChipDeviceScanner &) = delete;

    ~ChipDeviceScanner();

    /// Initiate a scan for devices, with the given timeout
    CHIP_ERROR StartScan(System::Clock::Timeout timeout);

    CHIP_ERROR StartChipScan(unsigned timeoutMs);
    CHIP_ERROR StopChipScan(void);

    /// Stop any currently running scan
    CHIP_ERROR StopScan();

    static std::unique_ptr<ChipDeviceScanner> Create(LSHandle * handle, ChipDeviceScannerDelegate * delegate);
    static std::unique_ptr<ChipDeviceScanner> Create(ChipDeviceScannerDelegate * delegate);

private:
    static void printFoundChipDevice(const jvalue_ref & scanRecord, const std::string & address);
    static bool deviceGetstatusCb(LSHandle * sh, LSMessage * message, void * ctx);
    static bool startDiscoveryCb(LSHandle * sh, LSMessage * message, void * ctx);
    static bool cancelDiscoveryCb(LSHandle * sh, LSMessage * message, void * ctx);

    static gboolean TimerExpiredCb(gpointer user_data);
    static gboolean TriggerScan(GMainLoop * mainLoop, gpointer userData);

    static void TimerExpiredCallback(chip::System::Layer * layer, void * appState);
    static int MainLoopStartScan(ChipDeviceScanner * self);
    static int MainLoopStopScan(ChipDeviceScanner * self);

    LSHandle * mLSHandle                  = nullptr;
    ChipDeviceScannerDelegate * mDelegate = nullptr;
    gulong mObjectAddedSignal             = 0;
    gulong mInterfaceChangedSignal        = 0;
    bool mIsScanning                      = false;
    bool mIsStopping                      = false;
    GMainLoop * mAsyncLoop                = nullptr;
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
