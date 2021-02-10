/* See Project CHIP LICENSE file for licensing information. */

/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#if CHIP_DEVICE_CONFIG_ENABLE_SOFTWARE_UPDATE_MANAGER

#include <protocols/Protocols.h>

#include <DeviceLayer/internal/GenericSoftwareUpdateManagerImpl.cpp>
#include <DeviceLayer/internal/GenericSoftwareUpdateManagerImpl_BDX.cpp>

namespace chip {
namespace DeviceLayer {

SoftwareUpdateManagerImpl SoftwareUpdateManagerImpl::sInstance;

CHIP_ERROR SoftwareUpdateManagerImpl::_Init(void)
{
    Internal::GenericSoftwareUpdateManagerImpl_BDX<SoftwareUpdateManagerImpl>::DoInit();
    Internal::GenericSoftwareUpdateManagerImpl<SoftwareUpdateManagerImpl>::DoInit();

    return CHIP_NO_ERROR;
}

} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_SOFTWARE_UPDATE_MANAGER
