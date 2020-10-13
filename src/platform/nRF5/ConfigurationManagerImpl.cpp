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
 *          Provides the implementation of the Device Layer ConfigurationManager object
 *          for nRF52 platforms using the Nordic nRF5 SDK.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/ConfigurationManager.h>
#include <platform/internal/GenericConfigurationManagerImpl.cpp>

#include <core/CHIPVendorIdentifiers.hpp>
#include <platform/nRF5/nRF5Config.h>

#if CHIP_DEVICE_CONFIG_ENABLE_FACTORY_PROVISIONING
#include <platform/internal/FactoryProvisioning.cpp>
#endif // CHIP_DEVICE_CONFIG_ENABLE_FACTORY_PROVISIONING

#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

namespace chip {
namespace DeviceLayer {

using namespace ::chip::DeviceLayer::Internal;

// TODO: Define a Singleton instance of CHIP Group Key Store here (#1204)

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

    // TODO: Initialize the global GroupKeyStore object here (#1204)

#if CHIP_DEVICE_CONFIG_ENABLE_FACTORY_PROVISIONING

    {
        FactoryProvisioning factoryProv;
        uint8_t * const kDeviceRAMStart = (uint8_t *) 0x20000000;
        uint8_t * const kDeviceRAMEnd   = kDeviceRAMStart + (NRF_FICR->INFO.RAM * 1024) - 1;

        // Scan device RAM for injected provisioning data and save to persistent storage if found.
        err = factoryProv.ProvisionDeviceFromRAM(kDeviceRAMStart, kDeviceRAMEnd);
        SuccessOrExit(err);
    }

#endif // CHIP_DEVICE_CONFIG_ENABLE_FACTORY_PROVISIONING

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
    uintmax_t recordKey = persistedStorageKey + kPersistedCounterRecordKeyBase;

    VerifyOrExit(recordKey <= kPersistedCounterRecordKeyMax, err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    static_assert(kPersistedCounterRecordKeyMax <= UINT16_MAX, "Key won't fit in uint16_t");

    err = ReadConfigValue(NRF5ConfigKey(NRF5Config::kFileId_ChipCounter, static_cast<uint16_t>(recordKey)), value);
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
    CHIP_ERROR err;
    uintmax_t recordKey = persistedStorageKey + kPersistedCounterRecordKeyBase;

    VerifyOrExit(recordKey <= kPersistedCounterRecordKeyMax, err = CHIP_ERROR_INVALID_ARGUMENT);
    static_assert(kPersistedCounterRecordKeyMax <= UINT16_MAX, "Key won't fit in uint16_t");

    err = WriteConfigValue(NRF5ConfigKey(NRF5Config::kFileId_ChipCounter, static_cast<uint16_t>(recordKey)), value);
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
        ChipLogError(DeviceLayer, "FactoryResetConfig() failed: %s", ErrorStr(err));
    }

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD

    ThreadStackMgr().ErasePersistentInfo();

#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

    // Restart the system.
    ChipLogProgress(DeviceLayer, "System restarting");
    NVIC_SystemReset();
}

} // namespace DeviceLayer
} // namespace chip
