/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2018 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
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

CHIP_ERROR BLEManagerImpl::StartScan(BleScannerDelegate * delegate)
{
    if (mConnectionDelegate)
    {
        static_cast<BleConnectionDelegateImpl *>(mConnectionDelegate)->StartScan(delegate);
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_INCORRECT_STATE;
}

CHIP_ERROR BLEManagerImpl::StopScan()
{
    if (mConnectionDelegate)
    {
        static_cast<BleConnectionDelegateImpl *>(mConnectionDelegate)->StopScan();
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_INCORRECT_STATE;
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
