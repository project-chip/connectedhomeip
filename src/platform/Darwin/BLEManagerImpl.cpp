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
#include <lib/support/logging/CHIPLogging.h>
#include <platform/Darwin/BleApplicationDelegate.h>
#include <platform/Darwin/BleConnectionDelegate.h>
#include <platform/Darwin/BlePlatformDelegate.h>

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

    ChipLogDetail(DeviceLayer, "%s", __FUNCTION__);

    // Initialize the Chip BleLayer.
    BleApplicationDelegateImpl * appDelegate   = new BleApplicationDelegateImpl();
    BleConnectionDelegateImpl * connDelegate   = new BleConnectionDelegateImpl();
    BlePlatformDelegateImpl * platformDelegate = new BlePlatformDelegateImpl();

    mApplicationDelegate = appDelegate;
    mConnectionDelegate  = connDelegate;
    mPlatformDelegate    = platformDelegate;

    err = BleLayer::Init(platformDelegate, connDelegate, appDelegate, &DeviceLayer::SystemLayer());

    if (CHIP_NO_ERROR != err)
    {
        _Shutdown();
    }

    return err;
}

void BLEManagerImpl::_Shutdown()
{
    if (mApplicationDelegate)
    {
        delete mApplicationDelegate;
        mApplicationDelegate = nullptr;
    }

    if (mConnectionDelegate)
    {
        delete mConnectionDelegate;
        mConnectionDelegate = nullptr;
    }

    if (mPlatformDelegate)
    {
        delete mPlatformDelegate;
        mPlatformDelegate = nullptr;
    }
}

bool BLEManagerImpl::_IsAdvertisingEnabled(void)
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

bool BLEManagerImpl::_IsAdvertising(void)
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

uint16_t BLEManagerImpl::_NumConnections(void)
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
