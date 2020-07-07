/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2020 Nest Labs, Inc.
 *    All rights reserved.
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
 *          Provides the implementation of the Device Layer ConfigurationManager object
 *          for K32W platforms using the NXP SDK.
 */

/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/internal/GenericConfigurationManagerImpl.ipp>

#include <platform/ConfigurationManager.h>
#include <platform/K32W/GroupKeyStoreImpl.h>
#include <platform/K32W/K32WConfig.h>
#include <platform/Profiles/security/CHIPApplicationKeys.h>

#include "fsl_reset.h"

namespace chip {
namespace DeviceLayer {

using namespace ::chip::Profiles::Security::AppKeys;
using namespace ::chip::Profiles::DeviceDescription;
using namespace ::chip::DeviceLayer::Internal;

namespace {

// Singleton instance of CHIP Group Key Store.
GroupKeyStoreImpl gGroupKeyStore;

} // unnamed namespace

/** Singleton instance of the ConfigurationManager implementation object.
 */
ConfigurationManagerImpl ConfigurationManagerImpl::sInstance;

CHIP_ERROR ConfigurationManagerImpl::_Init()
{
    CHIP_ERROR err;
    bool failSafeArmed;

    // Initialize the generic implementation base class.
    err = Internal::GenericConfigurationManagerImpl<ConfigurationManagerImpl>::_Init();
    SuccessOrExit(err);

    // Initialize the global GroupKeyStore object.
    err = gGroupKeyStore.Init();
    SuccessOrExit(err);

    // If the fail-safe was armed when the device last shutdown, initiate a factory reset.
    if (_GetFailSafeArmed(failSafeArmed) == CHIP_NO_ERROR && failSafeArmed)
    {
        ChipLogProgress(DeviceLayer, "Detected fail-safe armed on reboot; initiating factory reset");
        _InitiateFactoryReset();
    }
    err = CHIP_NO_ERROR;

exit:
    return err;
}

WEAVE_ERROR
ConfigurationManagerImpl::_GetDeviceDescriptor(::nl::Weave::Profiles::DeviceDescription::WeaveDeviceDescriptor & deviceDesc)
{
    WEAVE_ERROR err;

    // Call the generic version of _GetDeviceDescriptor() supplied by the base class.
    err = Internal::GenericConfigurationManagerImpl<ConfigurationManagerImpl>::_GetDeviceDescriptor(deviceDesc);
    SuccessOrExit(err);

exit:
    return err;
}

::chip::Profiles::Security::AppKeys::GroupKeyStoreBase * ConfigurationManagerImpl::_GetGroupKeyStore()
{
    return &gGroupKeyStore;
}

bool ConfigurationManagerImpl::_CanFactoryReset()
{
    // TODO: query the application to determine if factory reset is allowed.
    return true;
}

void ConfigurationManagerImpl::_InitiateFactoryReset()
{
    PlatformMgr().ScheduleWork(DoFactoryReset);
}

CHIP_ERROR ConfigurationManagerImpl::_ReadPersistedStorageValue(::chip::Platform::PersistedStorage::Key persistedStorageKey,
                                                                uint32_t & value)
{
    CHIP_ERROR err;

    err = ReadConfigValueCounter(persistedStorageKey, value);
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR ConfigurationManagerImpl::_WritePersistedStorageValue(::chip::Platform::PersistedStorage::Key persistedStorageKey,
                                                                 uint32_t value)
{
    // This method reads Chip Persisted Counter type nvm3 objects.
    // (where persistedStorageKey represents an index to the counter).
    CHIP_ERROR err;

    err = WriteConfigValueCounter(persistedStorageKey, value);
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }
    SuccessOrExit(err);

exit:
    return err;
}

void ConfigurationManagerImpl::DoFactoryReset(intptr_t arg)
{
    CHIP_ERROR err;

    ChipLogProgress(DeviceLayer, "Performing factory reset");

    err = FactoryResetConfig();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "FactoryResetConfig() failed: %s", nl::ErrorStr(err));
    }

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD

    ChipLogProgress(DeviceLayer, "Clearing Thread provision");
    ThreadStackMgr().ClearThreadProvision();

#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

    // Restart the system.
    ChipLogProgress(DeviceLayer, "System restarting");
    RESET_SystemReset();
}

} // namespace DeviceLayer
} // namespace chip
