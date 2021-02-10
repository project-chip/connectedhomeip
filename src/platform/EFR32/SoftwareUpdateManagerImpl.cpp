/* See Project CHIP LICENSE file for licensing information. */

/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#if CHIP_DEVICE_CONFIG_ENABLE_SOFTWARE_UPDATE_MANAGER

#include <platform/internal/GenericSoftwareUpdateManagerImpl.cpp>
#include <platform/internal/GenericSoftwareUpdateManagerImpl_BDX.cpp>

#include <platform/Profiles/CHIPProfiles.h>
#include <platform/Profiles/common/CommonProfile.h>

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
