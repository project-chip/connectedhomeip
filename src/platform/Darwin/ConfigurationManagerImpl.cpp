/* See Project CHIP LICENSE file for licensing information. */

/**
 *    @file
 *          Provides the implementation of the Device Layer ConfigurationManager object
 *          for Darwin platforms.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <core/CHIPVendorIdentifiers.hpp>
#include <platform/ConfigurationManager.h>
#include <platform/Darwin/PosixConfig.h>
#include <platform/internal/GenericConfigurationManagerImpl.cpp>

#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

namespace chip {
namespace DeviceLayer {

using namespace ::chip::DeviceLayer::Internal;

/** Singleton instance of the ConfigurationManager implementation object.
 */
ConfigurationManagerImpl ConfigurationManagerImpl::sInstance;

CHIP_ERROR ConfigurationManagerImpl::_Init()
{
    CHIP_ERROR err;

    // Initialize the generic implementation base class.
    err = Internal::GenericConfigurationManagerImpl<ConfigurationManagerImpl>::_Init();
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR ConfigurationManagerImpl::_GetPrimaryWiFiMACAddress(uint8_t * buf)
{
    // TODO(#739): add WiFi support
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

bool ConfigurationManagerImpl::_CanFactoryReset()
{
    // TODO(#742): query the application to determine if factory reset is allowed.
    return true;
}

void ConfigurationManagerImpl::_InitiateFactoryReset()
{
    ChipLogError(DeviceLayer, "InitiateFactoryReset not implemented");
}

CHIP_ERROR ConfigurationManagerImpl::_ReadPersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t & value)
{
    PosixConfig::Key configKey{ kConfigNamespace_ChipCounters, key };

    CHIP_ERROR err = ReadConfigValue(configKey, value);
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }
    return err;
}

CHIP_ERROR ConfigurationManagerImpl::_WritePersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t value)
{
    PosixConfig::Key configKey{ kConfigNamespace_ChipCounters, key };
    return WriteConfigValue(configKey, value);
}

} // namespace DeviceLayer
} // namespace chip
