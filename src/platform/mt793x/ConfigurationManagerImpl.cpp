/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Nest Labs, Inc.
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
 *          Provides an implementation of the Device LayerConfigurationManager
 *          object for MT793X platforms using the MT793X SDK.
 */

/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/internal/GenericConfigurationManagerImpl.ipp>

#include <platform/ConfigurationManager.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/mt793x/MT793XConfig.h>

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
#ifdef MT793X_PORTING
#include "wfx_host_events.h"
#endif /* MT793X_PORTING */
#endif
#include "stdio.h"

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
    bool failSafeArmed;

    // Initialize the generic implementation base class.
    err = Internal::GenericConfigurationManagerImpl<MT793XConfig>::Init();
    SuccessOrExit(err);
    // TODO: Initialize the global GroupKeyStore object here (#1626)
    IncreaseBootCount();

    // It is possible to configure the possible reset sources with RMU_ResetControl
    // In this case, we keep Reset control at default setting
    //    rebootCause = RMU_ResetCauseGet();
    //    RMU_ResetCauseClear();
    // If the fail-safe was armed when the device last shutdown, initiate a factory reset.
    if (GetFailSafeArmed(failSafeArmed) == CHIP_NO_ERROR && failSafeArmed)
    {
        ChipLogProgress(DeviceLayer, "Detected fail-safe armed on reboot; initiating factory reset");
        InitiateFactoryReset();
    }
    err = CHIP_NO_ERROR;

exit:
    return err;
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

CHIP_ERROR ConfigurationManagerImpl::GetRebootCount(uint32_t & rebootCount)
{
    return MT793XConfig::ReadConfigValue(MT793XConfig::kConfigKey_BootCount, rebootCount);
}

CHIP_ERROR ConfigurationManagerImpl::IncreaseBootCount(void)
{
    uint32_t bootCount = 0;
    if (MT793XConfig::ConfigValueExists(MT793XConfig::kConfigKey_BootCount))
    {
        GetRebootCount(bootCount);
    }
    return MT793XConfig::WriteConfigValue(MT793XConfig::kConfigKey_BootCount, bootCount + 1);
}

uint32_t ConfigurationManagerImpl::GetBootReason(void)
{
    // rebootCause is obtained at bootup.
    BootReasonType matterBootCause;
    matterBootCause = BootReasonType::kUnspecified;
    return to_underlying(matterBootCause);
}

CHIP_ERROR ConfigurationManagerImpl::GetTotalOperationalHours(uint32_t & totalOperationalHours)
{
    if (!MT793XConfig::ConfigValueExists(MT793XConfig::kConfigKey_TotalOperationalHours))
    {
        totalOperationalHours = 0;
        return CHIP_NO_ERROR;
    }

    return MT793XConfig::ReadConfigValue(MT793XConfig::kConfigKey_TotalOperationalHours, totalOperationalHours);
}

CHIP_ERROR ConfigurationManagerImpl::StoreTotalOperationalHours(uint32_t totalOperationalHours)
{
    return MT793XConfig::WriteConfigValue(MT793XConfig::kConfigKey_TotalOperationalHours, totalOperationalHours);
}

CHIP_ERROR ConfigurationManagerImpl::ReadPersistedStorageValue(::chip::Platform::PersistedStorage::Key persistedStorageKey,
                                                               uint32_t & value)
{
    CHIP_ERROR err;
    MT793XConfig::Key configKey{ MT793XConfig::kConfigNamespace_ChipCounters, (char *) &persistedStorageKey };

    err = ReadConfigValue(configKey, value);
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
    CHIP_ERROR err;
    MT793XConfig::Key configKey{ MT793XConfig::kConfigNamespace_ChipCounters, (char *) &persistedStorageKey };

    err = WriteConfigValue(configKey, value);
    {
        err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, bool & val)
{
    return MT793XConfig::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, uint32_t & val)
{
    return MT793XConfig::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, uint64_t & val)
{
    return MT793XConfig::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    return MT793XConfig::ReadConfigValueStr(key, buf, bufSize, outLen);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    return MT793XConfig::ReadConfigValueBin(key, buf, bufSize, outLen);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, bool val)
{
    return MT793XConfig::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, uint32_t val)
{
    return MT793XConfig::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, uint64_t val)
{
    return MT793XConfig::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueStr(Key key, const char * str)
{
    return MT793XConfig::WriteConfigValueStr(key, str);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    return MT793XConfig::WriteConfigValueStr(key, str, strLen);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    return MT793XConfig::WriteConfigValueBin(key, data, dataLen);
}

void ConfigurationManagerImpl::RunConfigUnitTest(void)
{
    MT793XConfig::RunConfigUnitTest();
}

void ConfigurationManagerImpl::DoFactoryReset(intptr_t arg)
{
    CHIP_ERROR err;

    ChipLogProgress(DeviceLayer, "Performing factory reset");

    err = MT793XConfig::FactoryResetConfig();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "FactoryResetConfig() failed: %s", chip::ErrorStr(err));
    }

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD

    ChipLogProgress(DeviceLayer, "Clearing Thread provision");
    ThreadStackMgr().ErasePersistentInfo();

#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

    PersistedStorage::KeyValueStoreMgrImpl().ErasePartition();

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
    ChipLogProgress(DeviceLayer, "Clearing WiFi provision");
#ifdef MT793X_PORTING
    wfx_clear_wifi_provision();
#endif /* MT793X_PORTING */
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION

    // Restart the system.
    ChipLogProgress(DeviceLayer, "System restarting");
    NVIC_SystemReset();
}

CHIP_ERROR ConfigurationManagerImpl::GetPrimaryWiFiMACAddress(uint8_t * buf)
{
    if (wifi_get_mac_addr_from_efuse(1, buf) < 0)
        return CHIP_ERROR_INTERNAL;
    return CHIP_NO_ERROR;
}

ConfigurationManager & ConfigurationMgrImpl()
{
    return ConfigurationManagerImpl::GetDefaultInstance();
}

} // namespace DeviceLayer
} // namespace chip
