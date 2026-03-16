/*
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/ConfigurationManager.h>

#include <platform/internal/GenericConfigurationManagerImpl.ipp>

#if CHIP_DEVICE_LAYER_TARGET_BL616
extern "C" {
#include <bl_sys.h>
}
#else
extern "C" void hal_reboot(void);
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_ETHERNET
extern "C" struct netif * deviceInterface_getNetif(void);
#endif

namespace chip {
namespace DeviceLayer {

using namespace ::chip::DeviceLayer::Internal;
ConfigurationManagerImpl & ConfigurationManagerImpl::GetDefaultInstance()
{
    static ConfigurationManagerImpl sInstance;
    return sInstance;
}
bool ConfigurationManagerImpl::IsFullyProvisioned()
{
    return
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
        ConnectivityMgr().IsWiFiStationProvisioned() &&
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
        ConnectivityMgr().IsThreadProvisioned() &&
#endif
        true;
}
CHIP_ERROR ConfigurationManagerImpl::Init()
{
    CHIP_ERROR err;
    uint32_t rebootCount = 0;

    err = Internal::GenericConfigurationManagerImpl<BflbConfig>::Init();
    SuccessOrExit(err);

    if (BflbConfig::ConfigValueExists(BflbConfig::kCounterKey_RebootCount))
    {
        err = GetRebootCount(rebootCount);
        SuccessOrExit(err);
    }
    else
    {
        rebootCount = 0;
    }
    err = StoreRebootCount(rebootCount + 1);
    SuccessOrExit(err);

    if (!BflbConfig::ConfigValueExists(BflbConfig::kCounterKey_TotalOperationalHours))
    {
        err = StoreTotalOperationalHours(0);
        SuccessOrExit(err);
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
    TEMPORARY_RETURN_IGNORED PlatformMgr().ScheduleWork(DoFactoryReset);
}

CHIP_ERROR ConfigurationManagerImpl::GetRebootCount(uint32_t & rebootCount)
{
    return BflbConfig::ReadConfigValue(BflbConfig::kCounterKey_RebootCount, rebootCount);
}

CHIP_ERROR ConfigurationManagerImpl::StoreRebootCount(uint32_t rebootCount)
{
    return BflbConfig::WriteConfigValue(BflbConfig::kCounterKey_RebootCount, rebootCount);
}

CHIP_ERROR ConfigurationManagerImpl::GetTotalOperationalHours(uint32_t & totalOperationalHours)
{
    return ReadConfigValue(BflbConfig::kCounterKey_TotalOperationalHours, totalOperationalHours);
}

CHIP_ERROR ConfigurationManagerImpl::StoreTotalOperationalHours(uint32_t totalOperationalHours)
{
    return WriteConfigValue(BflbConfig::kCounterKey_TotalOperationalHours, totalOperationalHours);
}

CHIP_ERROR ConfigurationManagerImpl::ReadPersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t & value)
{
    BflbConfig::Key configKey{ key };

    CHIP_ERROR err = ReadConfigValue(configKey, value);
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }
    return err;
}

CHIP_ERROR ConfigurationManagerImpl::WritePersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t value)
{
    BflbConfig::Key configKey{ key };
    return WriteConfigValue(configKey, value);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, bool & val)
{
    return BflbConfig::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, uint32_t & val)
{
    return BflbConfig::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, uint64_t & val)
{
    return BflbConfig::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    return BflbConfig::ReadConfigValueStr(key, buf, bufSize, outLen);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    return BflbConfig::ReadConfigValueBin(key, buf, bufSize, outLen);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, bool val)
{
    return BflbConfig::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, uint32_t val)
{
    return BflbConfig::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, uint64_t val)
{
    return BflbConfig::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueStr(Key key, const char * str)
{
    return BflbConfig::WriteConfigValueStr(key, str);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    return BflbConfig::WriteConfigValueStr(key, str, strLen);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    return BflbConfig::WriteConfigValueBin(key, data, dataLen);
}

void ConfigurationManagerImpl::RunConfigUnitTest(void)
{
    BflbConfig::RunConfigUnitTest();
}

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
void ConfigurationManagerImpl::ClearThreadStack()
{
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
    TEMPORARY_RETURN_IGNORED ThreadStackMgr().ClearAllSrpHostAndServices();
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
    ChipLogProgress(DeviceLayer, "Clearing Thread provision");
    ThreadStackMgr().ErasePersistentInfo();
}
#endif

void ConfigurationManagerImpl::DoFactoryReset(intptr_t arg)
{
    CHIP_ERROR err;

    ChipLogProgress(DeviceLayer, "Performing factory reset");

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    ThreadStackMgr().ErasePersistentInfo();
#endif

    err = BflbConfig::FactoryResetConfig();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "FactoryResetConfig() failed: %" CHIP_ERROR_FORMAT, err.Format());
    }

    // Restart the system.
    ChipLogProgress(DeviceLayer, "System restarting");
#if CHIP_DEVICE_LAYER_TARGET_BL616
    bl_sys_reset_por();
#else
    hal_reboot();
#endif
}

#if CHIP_DEVICE_CONFIG_ENABLE_ETHERNET
CHIP_ERROR ConfigurationManagerImpl::GetPrimaryMACAddress(MutableByteSpan & buf)
{
    if (buf.size() != ConfigurationManager::kPrimaryMACAddressLength)
        return CHIP_ERROR_INVALID_ARGUMENT;

    memcpy(buf.data(), deviceInterface_getNetif()->hwaddr, ConfigurationManager::kPrimaryMACAddressLength);

    return CHIP_NO_ERROR;
}
#endif

ConfigurationManager & ConfigurationMgrImpl()
{
    return ConfigurationManagerImpl::GetDefaultInstance();
}
} // namespace DeviceLayer
} // namespace chip
