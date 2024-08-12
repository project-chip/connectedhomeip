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

#include <platform/ConfigurationManager.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/internal/GenericConfigurationManagerImpl.ipp>
#include <platform/nxp/k32w0/K32W0Config.h>
#include <platform/nxp/k32w0/KeyValueStoreManagerImpl.h>

#include "fsl_power.h"
#include "fsl_reset.h"

namespace chip {
namespace DeviceLayer {

using namespace ::chip::DeviceLayer::Internal;

ConfigurationManagerImpl & ConfigurationManagerImpl::GetDefaultInstance()
{
    static ConfigurationManagerImpl sInstance;
    return sInstance;
}

CHIP_ERROR ConfigurationManagerImpl::Init()
{
    CHIP_ERROR err;
    uint8_t rebootCause = POWER_GetResetCause();

    if (K32WConfig::ConfigValueExists(K32WConfig::kCounterKey_RebootCount))
    {
        uint32_t rebootCount = 0;
        SuccessOrExit(err = GetRebootCount(rebootCount));
        SuccessOrExit(err = StoreRebootCount(rebootCount + 1));
    }
    else
    {
        // The first boot after factory reset of the Node.
        SuccessOrExit(err = StoreRebootCount(0));
    }

    if (!K32WConfig::ConfigValueExists(K32WConfig::kCounterKey_TotalOperationalHours))
    {
        SuccessOrExit(err = StoreTotalOperationalHours(0));
    }

    SuccessOrExit(err = DetermineBootReason(rebootCause));

    // Initialize the generic implementation base class.
    err = Internal::GenericConfigurationManagerImpl<K32WConfig>::Init();
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR ConfigurationManagerImpl::StoreSoftwareUpdateCompleted()
{
    return WriteConfigValue(K32WConfig::kConfigKey_SoftwareUpdateCompleted, true);
}

CHIP_ERROR ConfigurationManagerImpl::GetRebootCount(uint32_t & rebootCount)
{
    return ReadConfigValue(K32WConfig::kCounterKey_RebootCount, rebootCount);
}

CHIP_ERROR ConfigurationManagerImpl::StoreRebootCount(uint32_t rebootCount)
{
    return WriteConfigValue(K32WConfig::kCounterKey_RebootCount, rebootCount);
}

CHIP_ERROR ConfigurationManagerImpl::GetTotalOperationalHours(uint32_t & totalOperationalHours)
{
    return ReadConfigValue(K32WConfig::kCounterKey_TotalOperationalHours, totalOperationalHours);
}

CHIP_ERROR ConfigurationManagerImpl::StoreTotalOperationalHours(uint32_t totalOperationalHours)
{
    return WriteConfigValue(K32WConfig::kCounterKey_TotalOperationalHours, totalOperationalHours);
}

CHIP_ERROR ConfigurationManagerImpl::GetBootReason(uint32_t & bootReason)
{
    return ReadConfigValue(K32WConfig::kCounterKey_BootReason, bootReason);
}

CHIP_ERROR ConfigurationManagerImpl::StoreBootReason(uint32_t bootReason)
{
    return WriteConfigValue(K32WConfig::kCounterKey_BootReason, bootReason);
}

CHIP_ERROR ConfigurationManagerImpl::GetUniqueId(char * buf, size_t bufSize)
{
    CHIP_ERROR err;
    size_t uniqueIdLen = 0; // without counting null-terminator
    err                = ReadConfigValueStr(K32WConfig::kConfigKey_UniqueId, buf, bufSize, uniqueIdLen);

    ReturnErrorOnFailure(err);

    ReturnErrorCodeIf(uniqueIdLen >= bufSize, CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(buf[uniqueIdLen] != 0, CHIP_ERROR_INVALID_STRING_LENGTH);

    return err;
}

CHIP_ERROR ConfigurationManagerImpl::StoreUniqueId(const char * uniqueId, size_t uniqueIdLen)
{
    return WriteConfigValueStr(K32WConfig::kConfigKey_UniqueId, uniqueId, uniqueIdLen);
}

CHIP_ERROR ConfigurationManagerImpl::GenerateUniqueId(char * buf, size_t bufSize)
{
    uint64_t randomUniqueId = Crypto::GetRandU64();
    return Encoding::BytesToUppercaseHexString(reinterpret_cast<uint8_t *>(&randomUniqueId), sizeof(uint64_t), buf, bufSize);
}

bool ConfigurationManagerImpl::CanFactoryReset()
{
    // TODO: query the application to determine if factory reset is allowed.
    return true;
}

void ConfigurationManagerImpl::InitiateFactoryReset()
{
    PlatformMgr().ScheduleWork(DoFactoryReset);
}

CHIP_ERROR ConfigurationManagerImpl::ReadPersistedStorageValue(::chip::Platform::PersistedStorage::Key persistedStorageKey,
                                                               uint32_t & value)
{
    CHIP_ERROR err;

    err = K32WConfig::ReadConfigValueCounter(persistedStorageKey, value);
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR ConfigurationManagerImpl::WritePersistedStorageValue(::chip::Platform::PersistedStorage::Key persistedStorageKey,
                                                                uint32_t value)
{
    // This method reads Chip Persisted Counter type nvm3 objects.
    // (where persistedStorageKey represents an index to the counter).
    CHIP_ERROR err;

    err = K32WConfig::WriteConfigValueCounter(persistedStorageKey, value);
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, bool & val)
{
    return K32WConfig::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, uint32_t & val)
{
    return K32WConfig::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, uint64_t & val)
{
    return K32WConfig::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    return K32WConfig::ReadConfigValueStr(key, buf, bufSize, outLen);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    return K32WConfig::ReadConfigValueBin(key, buf, bufSize, outLen);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, bool val)
{
    return K32WConfig::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, uint32_t val)
{
    return K32WConfig::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, uint64_t val)
{
    return K32WConfig::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueStr(Key key, const char * str)
{
    return K32WConfig::WriteConfigValueStr(key, str);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    return K32WConfig::WriteConfigValueStr(key, str, strLen);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    return K32WConfig::WriteConfigValueBin(key, data, dataLen);
}

void ConfigurationManagerImpl::RunConfigUnitTest(void) {}

CHIP_ERROR ConfigurationManagerImpl::DetermineBootReason(uint8_t rebootCause)
{
    BootReasonType bootReason = BootReasonType::kUnspecified;

    if ((rebootCause & RESET_POR) || (rebootCause & RESET_EXT_PIN))
    {
        bootReason = BootReasonType::kPowerOnReboot;
    }
    else if (rebootCause & RESET_BOR)
    {
        bootReason = BootReasonType::kBrownOutReset;
    }
    else if (rebootCause & RESET_WDT)
    {
        /* Reboot can be due to hardware or software watchdog */
        bootReason = BootReasonType::kHardwareWatchdogReset;
    }
    else if (rebootCause & RESET_SW_REQ)
    {
        if (K32WConfig::ConfigValueExists(K32WConfig::kConfigKey_SoftwareUpdateCompleted))
        {
            bootReason = BootReasonType::kSoftwareUpdateCompleted;
        }
        else
        {
            bootReason = BootReasonType::kSoftwareReset;
        }
    }

    K32WConfig::ClearConfigValue(K32WConfig::kConfigKey_SoftwareUpdateCompleted);

    return StoreBootReason(to_underlying(bootReason));
}

void ConfigurationManagerImpl::DoFactoryReset(intptr_t arg)
{
    ChipLogProgress(DeviceLayer, "Performing factory reset");

    K32WConfig::FactoryResetConfig();
    ChipLogProgress(DeviceLayer, "Erased K32WConfig storage.");
    PersistedStorage::KeyValueStoreManagerImpl::FactoryResetStorage();
    ChipLogProgress(DeviceLayer, "Erased KVS storage.");

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD

    ThreadStackMgr().ErasePersistentInfo();

#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

    // Restart the system.
    ChipLogProgress(DeviceLayer, "System restarting");
    RESET_SystemReset();
}

ConfigurationManager & ConfigurationMgrImpl()
{
    return ConfigurationManagerImpl::GetDefaultInstance();
}

} // namespace DeviceLayer
} // namespace chip
