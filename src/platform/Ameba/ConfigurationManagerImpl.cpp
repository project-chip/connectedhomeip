/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *          for the Ameba.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/Ameba/AmebaConfig.h>
#include <platform/ConfigurationManager.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/internal/GenericConfigurationManagerImpl.ipp>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>
#include <wifi_conf.h>

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
    uint32_t rebootCount;

    // Force initialization of NVS namespaces if they doesn't already exist.
    err = AmebaConfig::EnsureNamespace(AmebaConfig::kConfigNamespace_ChipFactory);
    SuccessOrExit(err);
    err = AmebaConfig::EnsureNamespace(AmebaConfig::kConfigNamespace_ChipConfig);
    SuccessOrExit(err);
    err = AmebaConfig::EnsureNamespace(AmebaConfig::kConfigNamespace_ChipCounters);
    SuccessOrExit(err);
    err = AmebaConfig::EnsureNamespace2(AmebaConfig::kConfigNamespace_ChipFabric1);
    SuccessOrExit(err);
    err = AmebaConfig::EnsureNamespace2(AmebaConfig::kConfigNamespace_ChipFabric2);
    SuccessOrExit(err);
    err = AmebaConfig::EnsureNamespace2(AmebaConfig::kConfigNamespace_ChipFabric3);
    SuccessOrExit(err);
    err = AmebaConfig::EnsureNamespace2(AmebaConfig::kConfigNamespace_ChipFabric4);
    SuccessOrExit(err);
    err = AmebaConfig::EnsureNamespace2(AmebaConfig::kConfigNamespace_ChipFabric5);
    SuccessOrExit(err);
    err = AmebaConfig::EnsureNamespace(AmebaConfig::kConfigNamespace_ChipACL);
    SuccessOrExit(err);
    err = AmebaConfig::EnsureNamespace(AmebaConfig::kConfigNamespace_ChipGroupMessageCounters);
    SuccessOrExit(err);
    err = AmebaConfig::EnsureNamespace(AmebaConfig::kConfigNamespace_ChipAttributes);
    SuccessOrExit(err);
    err = AmebaConfig::EnsureNamespace(AmebaConfig::kConfigNamespace_ChipBindingTable);
    SuccessOrExit(err);
    err = AmebaConfig::EnsureNamespace(AmebaConfig::kConfigNamespace_ChipOTA);
    SuccessOrExit(err);
    err = AmebaConfig::EnsureNamespace(AmebaConfig::kConfigNamespace_ChipFailSafe);
    SuccessOrExit(err);
    err = AmebaConfig::EnsureNamespace(AmebaConfig::kConfigNamespace_ChipSessionResumption);
    SuccessOrExit(err);
    err = AmebaConfig::EnsureNamespace(AmebaConfig::kConfigNamespace_ChipDeviceInfoProvider);
    SuccessOrExit(err);
    err = AmebaConfig::EnsureNamespace(AmebaConfig::kConfigNamespace_ChipGroupDataProvider);
    SuccessOrExit(err);
    err = AmebaConfig::EnsureNamespace(AmebaConfig::kConfigNamespace_ChipOthers);
    SuccessOrExit(err);
    err = AmebaConfig::EnsureNamespace2(AmebaConfig::kConfigNamespace_ChipOthers2);
    SuccessOrExit(err);

    if (AmebaConfig::ConfigValueExists(AmebaConfig::kCounterKey_RebootCount))
    {
        err = GetRebootCount(rebootCount);
        SuccessOrExit(err);

        err = StoreRebootCount(rebootCount + 1);
        SuccessOrExit(err);
    }
    else
    {
        // The first boot after factory reset of the Node.
        err = StoreRebootCount(1);
        SuccessOrExit(err);
    }

    if (!AmebaConfig::ConfigValueExists(AmebaConfig::kCounterKey_TotalOperationalHours))
    {
        err = StoreTotalOperationalHours(0);
        SuccessOrExit(err);
    }

    if (!AmebaConfig::ConfigValueExists(AmebaConfig::kCounterKey_BootReason))
    {
        err = StoreBootReason(to_underlying(BootReasonType::kUnspecified));
        SuccessOrExit(err);
    }

    // Initialize the generic implementation base class.
    err = Internal::GenericConfigurationManagerImpl<AmebaConfig>::Init();
    SuccessOrExit(err);

    err = CHIP_NO_ERROR;

exit:
    return err;
}

CHIP_ERROR ConfigurationManagerImpl::GetRebootCount(uint32_t & rebootCount)
{
    return ReadConfigValue(AmebaConfig::kCounterKey_RebootCount, rebootCount);
}

CHIP_ERROR ConfigurationManagerImpl::StoreRebootCount(uint32_t rebootCount)
{
    return WriteConfigValue(AmebaConfig::kCounterKey_RebootCount, rebootCount);
}

CHIP_ERROR ConfigurationManagerImpl::GetTotalOperationalHours(uint32_t & totalOperationalHours)
{
    return ReadConfigValue(AmebaConfig::kCounterKey_TotalOperationalHours, totalOperationalHours);
}

CHIP_ERROR ConfigurationManagerImpl::StoreTotalOperationalHours(uint32_t totalOperationalHours)
{
    return WriteConfigValue(AmebaConfig::kCounterKey_TotalOperationalHours, totalOperationalHours);
}

CHIP_ERROR ConfigurationManagerImpl::GetBootReason(uint32_t & bootReason)
{
    return ReadConfigValue(AmebaConfig::kCounterKey_BootReason, bootReason);
}

CHIP_ERROR ConfigurationManagerImpl::StoreBootReason(uint32_t bootReason)
{
    return WriteConfigValue(AmebaConfig::kCounterKey_BootReason, bootReason);
}

CHIP_ERROR ConfigurationManagerImpl::GetPrimaryWiFiMACAddress(uint8_t * buf)
{
    char temp[32];
    uint32_t mac[ETH_ALEN];
    int i = 0;

    wifi_get_mac_address(temp);

    char * token = strtok(temp, ":");
    while (token != NULL)
    {
        mac[i] = (uint32_t) strtol(token, NULL, 16);
        token  = strtok(NULL, ":");
        i++;
    }

    for (i = 0; i < ETH_ALEN; i++)
        buf[i] = mac[i] & 0xFF;

    return CHIP_NO_ERROR;
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

CHIP_ERROR ConfigurationManagerImpl::ReadPersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t & value)
{
    AmebaConfig::Key configKey{ AmebaConfig::kConfigNamespace_ChipCounters, key };

    CHIP_ERROR err = ReadConfigValue(configKey, value);
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }
    return err;
}

CHIP_ERROR ConfigurationManagerImpl::WritePersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t value)
{
    AmebaConfig::Key configKey{ AmebaConfig::kConfigNamespace_ChipCounters, key };
    return WriteConfigValue(configKey, value);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, bool & val)
{
    return AmebaConfig::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, uint32_t & val)
{
    return AmebaConfig::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, uint64_t & val)
{
    return AmebaConfig::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    return AmebaConfig::ReadConfigValueStr(key, buf, bufSize, outLen);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    return AmebaConfig::ReadConfigValueBin(key, buf, bufSize, outLen);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, bool val)
{
    return AmebaConfig::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, uint32_t val)
{
    return AmebaConfig::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, uint64_t val)
{
    return AmebaConfig::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueStr(Key key, const char * str)
{
    return AmebaConfig::WriteConfigValueStr(key, str);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    return AmebaConfig::WriteConfigValueStr(key, str, strLen);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    return AmebaConfig::WriteConfigValueBin(key, data, dataLen);
}

void ConfigurationManagerImpl::RunConfigUnitTest(void)
{
    AmebaConfig::RunConfigUnitTest();
}

void ConfigurationManagerImpl::DoFactoryReset(intptr_t arg)
{
    CHIP_ERROR err;

    ChipLogProgress(DeviceLayer, "Performing factory reset");

    // Erase all values in the chip-config NVS namespace.
    err = AmebaConfig::ClearNamespace(AmebaConfig::kConfigNamespace_ChipConfig);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "ClearNamespace(ChipConfig) failed: %s", chip::ErrorStr(err));
    }

    // Restart the system.
    ChipLogProgress(DeviceLayer, "System restarting");
    // sys_reset();
}

ConfigurationManager & ConfigurationMgrImpl()
{
    return ConfigurationManagerImpl::GetDefaultInstance();
}

} // namespace DeviceLayer
} // namespace chip
