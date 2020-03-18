 /*
 *
 *    Copyright (c) 2018 Nest Labs, Inc.
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
 *          for nRF52 platforms using the Nordic nRF5 SDK.
 */

#include <Weave/DeviceLayer/internal/WeaveDeviceLayerInternal.h>
#include <Weave/DeviceLayer/ConfigurationManager.h>
#include <Weave/Core/WeaveKeyIds.h>
#include <Weave/Core/WeaveVendorIdentifiers.hpp>
#include <Weave/Profiles/security/WeaveApplicationKeys.h>
#include <Weave/DeviceLayer/nRF5/GroupKeyStoreImpl.h>
#include <Weave/DeviceLayer/nRF5/nRF5Config.h>
#include <Weave/DeviceLayer/internal/GenericConfigurationManagerImpl.ipp>

#if WEAVE_DEVICE_CONFIG_ENABLE_FACTORY_PROVISIONING
#include <Weave/DeviceLayer/internal/FactoryProvisioning.ipp>
#endif // WEAVE_DEVICE_CONFIG_ENABLE_FACTORY_PROVISIONING

namespace nl {
namespace Weave {
namespace DeviceLayer {

using namespace ::nl::Weave::Profiles::Security::AppKeys;
using namespace ::nl::Weave::Profiles::DeviceDescription;
using namespace ::nl::Weave::DeviceLayer::Internal;

namespace {

// Singleton instance of Weave Group Key Store.
GroupKeyStoreImpl gGroupKeyStore;

} // unnamed namespace


/** Singleton instance of the ConfigurationManager implementation object.
 */
ConfigurationManagerImpl ConfigurationManagerImpl::sInstance;


WEAVE_ERROR ConfigurationManagerImpl::_Init()
{
    WEAVE_ERROR err;
    bool failSafeArmed;

    // Initialize the generic implementation base class.
    err = Internal::GenericConfigurationManagerImpl<ConfigurationManagerImpl>::_Init();
    SuccessOrExit(err);

    // Initialize the global GroupKeyStore object.
    err = gGroupKeyStore.Init();
    SuccessOrExit(err);

#if WEAVE_DEVICE_CONFIG_ENABLE_FACTORY_PROVISIONING

    {
        FactoryProvisioning factoryProv;
        uint8_t * const kDeviceRAMStart = (uint8_t *)0x20000000;
        uint8_t * const kDeviceRAMEnd = kDeviceRAMStart + (NRF_FICR->INFO.RAM * 1024) - 1;

        // Scan device RAM for injected provisioning data and save to persistent storage if found.
        err = factoryProv.ProvisionDeviceFromRAM(kDeviceRAMStart, kDeviceRAMEnd);
        SuccessOrExit(err);
    }

#endif // WEAVE_DEVICE_CONFIG_ENABLE_FACTORY_PROVISIONING

    // If the fail-safe was armed when the device last shutdown, initiate a factory reset.
    if (_GetFailSafeArmed(failSafeArmed) == WEAVE_NO_ERROR && failSafeArmed)
    {
        WeaveLogProgress(DeviceLayer, "Detected fail-safe armed on reboot; initiating factory reset");
        _InitiateFactoryReset();
    }
    err = WEAVE_NO_ERROR;

exit:
    return err;
}

WEAVE_ERROR ConfigurationManagerImpl::_GetDeviceDescriptor(::nl::Weave::Profiles::DeviceDescription::WeaveDeviceDescriptor & deviceDesc)
{
    WEAVE_ERROR err;

    // Call the generic version of _GetDeviceDescriptor() supplied by the base class.
    err = Internal::GenericConfigurationManagerImpl<ConfigurationManagerImpl>::_GetDeviceDescriptor(deviceDesc);
    SuccessOrExit(err);

exit:
    return err;
}

::nl::Weave::Profiles::Security::AppKeys::GroupKeyStoreBase * ConfigurationManagerImpl::_GetGroupKeyStore()
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

WEAVE_ERROR ConfigurationManagerImpl::_ReadPersistedStorageValue(::nl::Weave::Platform::PersistedStorage::Key persistedStorageKey, uint32_t & value)
{
    WEAVE_ERROR err;
    uint16_t recordKey = persistedStorageKey + kPersistedCounterRecordKeyBase;

    VerifyOrExit(recordKey <= kPersistedCounterRecordKeyMax, err = WEAVE_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

    err = ReadConfigValue(NRF5ConfigKey(NRF5Config::kFileId_WeaveCounter, recordKey), value);
    if (err == WEAVE_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        err = WEAVE_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }
    SuccessOrExit(err);

exit:
    return err;
}

WEAVE_ERROR ConfigurationManagerImpl::_WritePersistedStorageValue(::nl::Weave::Platform::PersistedStorage::Key persistedStorageKey, uint32_t value)
{
    WEAVE_ERROR err;
    uint16_t recordKey = persistedStorageKey + kPersistedCounterRecordKeyBase;

    VerifyOrExit(recordKey <= kPersistedCounterRecordKeyMax, err = WEAVE_ERROR_INVALID_ARGUMENT);

    err = WriteConfigValue(NRF5ConfigKey(NRF5Config::kFileId_WeaveCounter, recordKey), value);
    SuccessOrExit(err);

exit:
    return err;
}

void ConfigurationManagerImpl::DoFactoryReset(intptr_t arg)
{
    WEAVE_ERROR err;

    WeaveLogProgress(DeviceLayer, "Performing factory reset");

    err = FactoryResetConfig();
    if (err != WEAVE_NO_ERROR)
    {
        WeaveLogError(DeviceLayer, "FactoryResetConfig() failed: %s", nl::ErrorStr(err));
    }

#if WEAVE_DEVICE_CONFIG_ENABLE_THREAD

    WeaveLogProgress(DeviceLayer, "Clearing Thread provision");
    ThreadStackMgr().ClearThreadProvision();

#endif // WEAVE_DEVICE_CONFIG_ENABLE_THREAD

    // Restart the system.
    WeaveLogProgress(DeviceLayer, "System restarting");
    NVIC_SystemReset();
}

} // namespace DeviceLayer
} // namespace Weave
} // namespace nl
