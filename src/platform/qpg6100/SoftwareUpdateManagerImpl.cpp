/* See Project CHIP LICENSE file for licensing information. */


#include <platform/internal/CHIPDeviceLayerInternal.h>

#if CHIP_DEVICE_CONFIG_ENABLE_SOFTWARE_UPDATE_MANAGER

#include <platform/internal/GenericSoftwareUpdateManagerImpl.cpp>

namespace chip {
namespace DeviceLayer {

SoftwareUpdateManagerImpl SoftwareUpdateManagerImpl::sInstance;

CHIP_ERROR SoftwareUpdateManagerImpl::_Init(void)
{
    Internal::GenericSoftwareUpdateManagerImpl<SoftwareUpdateManagerImpl>::DoInit();

    return CHIP_NO_ERROR;
}

} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_SOFTWARE_UPDATE_MANAGER
