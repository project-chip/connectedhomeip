/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <ble/CHIPBleServiceData.h>
#include <platform/Darwin/BleApplicationDelegate.h>
#include <platform/Darwin/BleConnectionDelegate.h>
#include <platform/Darwin/BlePlatformDelegate.h>
#include <support/logging/CHIPLogging.h>

#include <new>

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

using namespace ::chip;
using namespace ::chip::Ble;

namespace chip {
namespace DeviceLayer {
namespace Internal {

BLEManagerImpl BLEManagerImpl::sInstance;

CHIP_ERROR BLEManagerImpl::_Init()
{
    CHIP_ERROR err;

    ChipLogProgress(DeviceLayer, "%s", __FUNCTION__);

    // Initialize the Chip BleLayer.
    BleApplicationDelegateImpl * appDelegate   = new BleApplicationDelegateImpl();
    BleConnectionDelegateImpl * connDelegate   = new BleConnectionDelegateImpl();
    BlePlatformDelegateImpl * platformDelegate = new BlePlatformDelegateImpl();
    err                                        = BleLayer::Init(platformDelegate, connDelegate, appDelegate, &SystemLayer);
    return err;
}

ConnectivityManager::CHIPoBLEServiceMode BLEManagerImpl::_GetCHIPoBLEServiceMode(void)
{
    ChipLogProgress(DeviceLayer, "%s", __FUNCTION__);
    return ConnectivityManager::kCHIPoBLEServiceMode_NotSupported;
}

CHIP_ERROR BLEManagerImpl::_SetCHIPoBLEServiceMode(ConnectivityManager::CHIPoBLEServiceMode val)
{
    ChipLogProgress(DeviceLayer, "%s", __FUNCTION__);
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

bool BLEManagerImpl::_IsAdvertisingEnabled(void)
{
    ChipLogProgress(DeviceLayer, "%s", __FUNCTION__);
    return false;
}

CHIP_ERROR BLEManagerImpl::_SetAdvertisingEnabled(bool val)
{
    ChipLogProgress(DeviceLayer, "%s", __FUNCTION__);
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

bool BLEManagerImpl::_IsFastAdvertisingEnabled(void)
{
    ChipLogProgress(DeviceLayer, "%s", __FUNCTION__);
    return false;
}

CHIP_ERROR BLEManagerImpl::_SetFastAdvertisingEnabled(bool val)
{
    ChipLogProgress(DeviceLayer, "%s", __FUNCTION__);
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

bool BLEManagerImpl::_IsAdvertising(void)
{
    ChipLogProgress(DeviceLayer, "%s", __FUNCTION__);
    return false;
}

CHIP_ERROR BLEManagerImpl::_GetDeviceName(char * buf, size_t bufSize)
{
    ChipLogProgress(DeviceLayer, "%s", __FUNCTION__);
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR BLEManagerImpl::_SetDeviceName(const char * deviceName)
{
    ChipLogProgress(DeviceLayer, "%s", __FUNCTION__);
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

BleLayer * BLEManagerImpl::_GetBleLayer()
{
    return this;
}

uint16_t BLEManagerImpl::_NumConnections(void)
{
    ChipLogProgress(DeviceLayer, "%s", __FUNCTION__);
    return 0;
}

void BLEManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    ChipLogProgress(DeviceLayer, "%s", __FUNCTION__);
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
