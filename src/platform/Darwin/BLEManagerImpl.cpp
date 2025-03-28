/*
 *
 *    Copyright (c) 2020-2025 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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
 *          for Darwin platforms.
 */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <ble/Ble.h>
#include <lib/core/Global.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/Darwin/BleApplicationDelegateImpl.h>
#include <platform/Darwin/BleConnectionDelegateImpl.h>
#include <platform/Darwin/BlePlatformDelegateImpl.h>

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

using namespace ::chip;
using namespace ::chip::Ble;

namespace chip {
namespace DeviceLayer {
namespace Internal {

Global<BLEManagerImpl> BLEManagerImpl::sInstance;

CHIP_ERROR BLEManagerImpl::_Init()
{
    ChipLogDetail(DeviceLayer, "Initializing BLE Manager");

    // Initialize the CHIP BleLayer. The application, connection, and platform delegate
    // implementations are all stateless classes that we inherit from privately.
    return BleLayer::Init(this, this, this, &DeviceLayer::SystemLayer());
}

void BLEManagerImpl::_Shutdown()
{
    // Nothing to do
}

CHIP_ERROR BLEManagerImpl::StartScan(BleScannerDelegate * delegate, BleScanMode mode)
{
    VerifyOrReturnError(BleLayer::IsInitialized(), CHIP_ERROR_INCORRECT_STATE);
    BleConnectionDelegateImpl::StartScan(delegate, mode);
    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::StopScan()
{
    VerifyOrReturnError(BleLayer::IsInitialized(), CHIP_ERROR_INCORRECT_STATE);
    BleConnectionDelegateImpl::StopScan();
    return CHIP_NO_ERROR;
}

bool BLEManagerImpl::_IsAdvertisingEnabled()
{
    ChipLogDetail(DeviceLayer, "%s", __FUNCTION__);
    return false;
}

CHIP_ERROR BLEManagerImpl::_SetAdvertisingEnabled(bool val)
{
    ChipLogDetail(DeviceLayer, "%s", __FUNCTION__);
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR BLEManagerImpl::_SetAdvertisingMode(BLEAdvertisingMode mode)
{
    ChipLogDetail(DeviceLayer, "%s", __FUNCTION__);
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

bool BLEManagerImpl::_IsAdvertising()
{
    ChipLogDetail(DeviceLayer, "%s", __FUNCTION__);
    return false;
}

CHIP_ERROR BLEManagerImpl::_GetDeviceName(char * buf, size_t bufSize)
{
    ChipLogDetail(DeviceLayer, "%s", __FUNCTION__);
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR BLEManagerImpl::_SetDeviceName(const char * deviceName)
{
    ChipLogDetail(DeviceLayer, "%s", __FUNCTION__);
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

BleLayer * BLEManagerImpl::_GetBleLayer()
{
    return this;
}

uint16_t BLEManagerImpl::_NumConnections()
{
    ChipLogDetail(DeviceLayer, "%s", __FUNCTION__);
    return 0;
}

void BLEManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    ChipLogDetail(DeviceLayer, "%s", __FUNCTION__);
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
