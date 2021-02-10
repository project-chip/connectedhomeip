/* See Project CHIP LICENSE file for licensing information. */


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

    ChipLogDetail(DeviceLayer, "%s", __FUNCTION__);

    // Initialize the Chip BleLayer.
    BleApplicationDelegateImpl * appDelegate   = new BleApplicationDelegateImpl();
    BleConnectionDelegateImpl * connDelegate   = new BleConnectionDelegateImpl();
    BlePlatformDelegateImpl * platformDelegate = new BlePlatformDelegateImpl();
    err                                        = BleLayer::Init(platformDelegate, connDelegate, appDelegate, &SystemLayer);
    return err;
}

ConnectivityManager::CHIPoBLEServiceMode BLEManagerImpl::_GetCHIPoBLEServiceMode(void)
{
    ChipLogDetail(DeviceLayer, "%s", __FUNCTION__);
    return ConnectivityManager::kCHIPoBLEServiceMode_NotSupported;
}

CHIP_ERROR BLEManagerImpl::_SetCHIPoBLEServiceMode(ConnectivityManager::CHIPoBLEServiceMode val)
{
    ChipLogDetail(DeviceLayer, "%s", __FUNCTION__);
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
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

bool BLEManagerImpl::_IsFastAdvertisingEnabled(void)
{
    ChipLogDetail(DeviceLayer, "%s", __FUNCTION__);
    return false;
}

CHIP_ERROR BLEManagerImpl::_SetFastAdvertisingEnabled(bool val)
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
