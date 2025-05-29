/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *          for Zephyr platforms.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/ConfigurationManager.h>
#include <platform/internal/GenericConfigurationManagerImpl.ipp>

#include <lib/core/CHIPVendorIdentifiers.hpp>

#include <platform/Zephyr/ZephyrConfig.h>

#include "InetUtils.h"

#ifdef CHIP_DEVICE_CONFIG_ENABLE_ETHERNET
#if (CHIP_DEVICE_CONFIG_ENABLE_WIFI) || (CHIP_DEVICE_CONFIG_ENABLE_ETHERNET)
#include <zephyr/net/net_if.h>
#endif //(CHIP_DEVICE_CONFIG_ENABLE_WIFI) || (CHIP_DEVICE_CONFIG_ENABLE_ETHERNET)
#endif // CHIP_DEVICE_CONFIG_ENABLE_ETHERNET

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#ifdef CONFIG_CHIP_FACTORY_RESET_ERASE_SETTINGS
#include <zephyr/settings/settings.h>
#if defined(CONFIG_SETTINGS_NVS)
#include <zephyr/fs/nvs.h>
#elif defined(CONFIG_SETTINGS_ZMS)
#include <zephyr/fs/zms.h>
#endif // CONFIG_SETTINGS_NVS || CONFIG_SETTINGS_ZMS
#endif // CONFIG_CHIP_FACTORY_RESET_ERASE_SETTINGS

#ifdef CONFIG_NET_L2_OPENTHREAD
#include <platform/ThreadStackManager.h>
#endif

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

    // Initialize the generic implementation base class.
    err = Internal::GenericConfigurationManagerImpl<ZephyrConfig>::Init();
    SuccessOrExit(err);

    if (ZephyrConfig::ConfigValueExists(ZephyrConfig::kCounterKey_RebootCount))
    {
        err = GetRebootCount(rebootCount);
        SuccessOrExit(err);

        // Do not increment reboot count if the value is going to overflow UINT16.
        err = StoreRebootCount(rebootCount < UINT16_MAX ? rebootCount + 1 : rebootCount);
        SuccessOrExit(err);
    }
    else
    {
        // The first boot after factory reset of the Node.
        err = StoreRebootCount(1);
        SuccessOrExit(err);
    }

    err = CHIP_NO_ERROR;

exit:
    return err;
}

CHIP_ERROR ConfigurationManagerImpl::GetRebootCount(uint32_t & rebootCount)
{
    return ReadConfigValue(ZephyrConfig::kCounterKey_RebootCount, rebootCount);
}

CHIP_ERROR ConfigurationManagerImpl::StoreRebootCount(uint32_t rebootCount)
{
    return WriteConfigValue(ZephyrConfig::kCounterKey_RebootCount, rebootCount);
}

CHIP_ERROR ConfigurationManagerImpl::GetTotalOperationalHours(uint32_t & totalOperationalHours)
{
    if (!ZephyrConfig::ConfigValueExists(ZephyrConfig::kCounterKey_TotalOperationalHours))
    {
        totalOperationalHours = 0;
        return CHIP_NO_ERROR;
    }

    return ZephyrConfig::ReadConfigValue(ZephyrConfig::kCounterKey_TotalOperationalHours, totalOperationalHours);
}

CHIP_ERROR ConfigurationManagerImpl::StoreTotalOperationalHours(uint32_t totalOperationalHours)
{
    return ZephyrConfig::WriteConfigValue(ZephyrConfig::kCounterKey_TotalOperationalHours, totalOperationalHours);
}

void ConfigurationManagerImpl::InitiateFactoryReset()
{
    PlatformMgr().ScheduleWork(DoFactoryReset);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, bool & val)
{
    return ZephyrConfig::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, uint32_t & val)
{
    return ZephyrConfig::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, uint64_t & val)
{
    return ZephyrConfig::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    return ZephyrConfig::ReadConfigValueStr(key, buf, bufSize, outLen);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    return ZephyrConfig::ReadConfigValueBin(key, buf, bufSize, outLen);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, bool val)
{
    return ZephyrConfig::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, uint32_t val)
{
    return ZephyrConfig::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, uint64_t val)
{
    return ZephyrConfig::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueStr(Key key, const char * str)
{
    return ZephyrConfig::WriteConfigValueStr(key, str);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    return ZephyrConfig::WriteConfigValueStr(key, str, strLen);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    return ZephyrConfig::WriteConfigValueBin(key, data, dataLen);
}

void ConfigurationManagerImpl::RunConfigUnitTest()
{
#if CHIP_CONFIG_TEST
    ZephyrConfig::RunConfigUnitTest();
#endif
}

void ConfigurationManagerImpl::DoFactoryReset(intptr_t arg)
{
    ChipLogProgress(DeviceLayer, "Performing factory reset");

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
    ThreadStackMgr().ClearAllSrpHostAndServices();
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT

// Lock the Thread stack to avoid unwanted interaction with settings NVS during factory reset.
#ifdef CONFIG_NET_L2_OPENTHREAD
    ThreadStackMgr().LockThreadStack();
#endif

#ifdef CONFIG_CHIP_FACTORY_RESET_ERASE_SETTINGS
    void * storage = nullptr;
    int status     = settings_storage_get(&storage);

    if (status == 0)
    {
#if defined(CONFIG_SETTINGS_NVS)
        status = nvs_clear(static_cast<nvs_fs *>(storage));
#elif defined(CONFIG_SETTINGS_ZMS)
        status = zms_clear(static_cast<zms_fs *>(storage));
#endif // CONFIG_SETTINGS_NVS || CONFIG_SETTINGS_ZMS
    }
    if (status)
    {
        ChipLogError(DeviceLayer, "Factory reset failed: %d", status);
    }
#else

    const CHIP_ERROR err = PersistedStorage::KeyValueStoreMgrImpl().DoFactoryReset();

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Factory reset failed: %" CHIP_ERROR_FORMAT, err.Format());
    }

    ConnectivityMgr().ErasePersistentInfo();
#endif // CONFIG_CHIP_FACTORY_RESET_ERASE_SETTINGS

    PlatformMgr().Shutdown();
}

CHIP_ERROR ConfigurationManagerImpl::GetPrimaryWiFiMACAddress(uint8_t * buf)
{
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI || CHIP_DEVICE_CONFIG_ENABLE_ETHERNET
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    const net_if * const iface = InetUtils::GetWiFiInterface();
#else
    const net_if * const iface = InetUtils::GetInterface();
#endif

    VerifyOrReturnError(iface != nullptr, CHIP_ERROR_INTERNAL);

    const auto linkAddrStruct = iface->if_dev->link_addr;
    memcpy(buf, linkAddrStruct.addr, linkAddrStruct.len);

    return CHIP_NO_ERROR;
#else
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif
}

ConfigurationManager & ConfigurationMgrImpl()
{
    return ConfigurationManagerImpl::GetDefaultInstance();
}

} // namespace DeviceLayer
} // namespace chip
