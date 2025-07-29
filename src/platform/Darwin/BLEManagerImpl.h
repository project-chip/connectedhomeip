/*
 *
 *    Copyright (c) 2020-2025 Project CHIP Authors
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
 *          Provides an implementation of the BLEManager singleton object
 *          for the Darwin platforms.
 */

#pragma once

#include <ble/Ble.h>
#include <lib/core/Global.h>
#include <lib/support/CodeUtils.h>
#include <platform/Darwin/BleApplicationDelegateImpl.h>
#include <platform/Darwin/BleConnectionDelegateImpl.h>
#include <platform/Darwin/BlePlatformDelegateImpl.h>
#include <platform/Darwin/BleScannerDelegate.h>

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

namespace chip {
namespace DeviceLayer {
namespace Internal {

using namespace chip::Ble;

/**
 * Concrete implementation of the BLEManagerImpl singleton object for the Darwin platforms.
 */
class BLEManagerImpl final : public BLEManager,
                             private BleLayer,
                             private BleApplicationDelegateImpl,
                             private BleConnectionDelegateImpl,
                             private BlePlatformDelegateImpl
{
    // Allow the BLEManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend BLEManager;

public:
    CHIP_ERROR ConfigureBle(uint32_t bleDeviceId, bool aIsCentral) { return CHIP_NO_ERROR; }
    CHIP_ERROR StartScan(BleScannerDelegate * delegate, BleScanMode mode = BleScanMode::kDefault);
    CHIP_ERROR StopScan();

private:
    // ===== Members that implement the BLEManager internal interface.

    CHIP_ERROR _Init(void);
    void _Shutdown();
    bool _IsAdvertisingEnabled(void);
    CHIP_ERROR _SetAdvertisingEnabled(bool val);
    bool _IsAdvertising(void);
    CHIP_ERROR _SetAdvertisingMode(BLEAdvertisingMode mode);
    CHIP_ERROR _GetDeviceName(char * buf, size_t bufSize);
    CHIP_ERROR _SetDeviceName(const char * deviceName);
    uint16_t _NumConnections(void);
    void _OnPlatformEvent(const ChipDeviceEvent * event);
    BleLayer * _GetBleLayer(void);

    // ===== Members for internal use by the following friends.

    friend BLEManager & BLEMgr(void);
    friend BLEManagerImpl & BLEMgrImpl(void);

    static Global<BLEManagerImpl> sInstance;
};

/**
 * Returns a reference to the public interface of the BLEManager singleton object.
 *
 * Internal components should use this to access features of the BLEManager object
 * that are common to all platforms.
 */
inline BLEManager & BLEMgr(void)
{
    return BLEManagerImpl::sInstance.get();
}

/**
 * Returns the platform-specific implementation of the BLEManager singleton object.
 *
 * Internal components can use this to gain access to features of the BLEManager
 * that are specific to the Darwin platforms.
 */
inline BLEManagerImpl & BLEMgrImpl(void)
{
    return BLEManagerImpl::sInstance.get();
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
