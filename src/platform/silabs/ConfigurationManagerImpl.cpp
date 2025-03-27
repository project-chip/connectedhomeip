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
 *          Provides the implementation of the Device Layer ConfigurationManager object
 *          for Silabs platforms using the Silicon Labs SDK.
 */
/* this file behaves like a config.h, comes first */
#include <cmsis_os2.h>
#include <platform/ConfigurationManager.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/internal/GenericConfigurationManagerImpl.ipp>
#include <platform/silabs/SilabsConfig.h>
#include <platform/silabs/platformAbstraction/SilabsPlatform.h>

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
#include <platform/silabs/wifi/WifiInterface.h>
#endif

namespace chip {
namespace DeviceLayer {

using namespace ::chip::DeviceLayer::Internal;
using namespace ::chip ::DeviceLayer ::Silabs;

ConfigurationManagerImpl & ConfigurationManagerImpl::GetDefaultInstance()
{
    static ConfigurationManagerImpl sInstance;
    return sInstance;
}

CHIP_ERROR ConfigurationManagerImpl::Init()
{
    CHIP_ERROR err;

    // Initialize the generic implementation base class.
    err = Internal::GenericConfigurationManagerImpl<SilabsConfig>::Init();
    SuccessOrExit(err);

    IncreaseBootCount();
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
    return SilabsConfig::ReadConfigValue(SilabsConfig::kConfigKey_BootCount, rebootCount);
}

CHIP_ERROR ConfigurationManagerImpl::IncreaseBootCount(void)
{
    uint32_t bootCount = 0;

    if (SilabsConfig::ConfigValueExists(SilabsConfig::kConfigKey_BootCount))
    {
        GetRebootCount(bootCount);
    }

    return SilabsConfig::WriteConfigValue(SilabsConfig::kConfigKey_BootCount, bootCount + 1);
}

CHIP_ERROR ConfigurationManagerImpl::GetBootReason(uint32_t & bootReason)
{
    // rebootCause is obtained at bootup.
    BootReasonType matterBootCause;
    uint32_t rebootCause = Silabs::GetPlatform().GetRebootCause();

#if defined(_RMU_RSTCAUSE_MASK)
    if (rebootCause & RMU_RSTCAUSE_PORST || rebootCause & RMU_RSTCAUSE_EXTRST) // PowerOn or External pin reset
    {
        matterBootCause = BootReasonType::kPowerOnReboot;
    }
    else if (rebootCause & RMU_RSTCAUSE_AVDDBOD || rebootCause & RMU_RSTCAUSE_DVDDBOD || rebootCause & RMU_RSTCAUSE_DECBOD)
    {
        matterBootCause = BootReasonType::kBrownOutReset;
    }
    else if (rebootCause & RMU_RSTCAUSE_SYSREQRST)
    {
        matterBootCause = BootReasonType::kSoftwareReset;
    }
    else if (rebootCause & RMU_RSTCAUSE_WDOGRST)
    {
        matterBootCause = BootReasonType::kSoftwareWatchdogReset;
    }
    else
    {
        matterBootCause = BootReasonType::kUnspecified;
    }
    // Not tracked HARDWARE_WATCHDOG_RESET && SOFTWARE_UPDATE_COMPLETED
#elif defined(_EMU_RSTCAUSE_MASK)
    if (rebootCause & EMU_RSTCAUSE_POR || rebootCause & EMU_RSTCAUSE_PIN) // PowerOn or External pin reset
    {
        matterBootCause = BootReasonType::kPowerOnReboot;
    }
    else if (rebootCause & EMU_RSTCAUSE_AVDDBOD || rebootCause & EMU_RSTCAUSE_DVDDBOD || rebootCause & EMU_RSTCAUSE_DECBOD ||
             rebootCause & EMU_RSTCAUSE_IOVDD0BOD || rebootCause & EMU_RSTCAUSE_DVDDLEBOD)
    {
        matterBootCause = BootReasonType::kBrownOutReset;
    }
    else if (rebootCause & EMU_RSTCAUSE_SYSREQ)
    {
        matterBootCause = BootReasonType::kSoftwareReset;
    }
    else if (rebootCause & EMU_RSTCAUSE_WDOG0 || rebootCause & EMU_RSTCAUSE_WDOG1)
    {
        matterBootCause = BootReasonType::kSoftwareWatchdogReset;
    }
    else
    {
        matterBootCause = BootReasonType::kUnspecified;
    }
    // Not tracked HARDWARE_WATCHDOG_RESET && SOFTWARE_UPDATE_COMPLETED
#else
    matterBootCause = BootReasonType::kUnspecified;
#endif

    bootReason = to_underlying(matterBootCause);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConfigurationManagerImpl::GetTotalOperationalHours(uint32_t & totalOperationalHours)
{
    if (!SilabsConfig::ConfigValueExists(SilabsConfig::kConfigKey_TotalOperationalHours))
    {
        totalOperationalHours = 0;
        return CHIP_NO_ERROR;
    }

    return SilabsConfig::ReadConfigValue(SilabsConfig::kConfigKey_TotalOperationalHours, totalOperationalHours);
}

CHIP_ERROR ConfigurationManagerImpl::StoreTotalOperationalHours(uint32_t totalOperationalHours)
{
    return SilabsConfig::WriteConfigValue(SilabsConfig::kConfigKey_TotalOperationalHours, totalOperationalHours);
}

CHIP_ERROR ConfigurationManagerImpl::ReadPersistedStorageValue(::chip::Platform::PersistedStorage::Key persistedStorageKey,
                                                               uint32_t & value)
{
    // This method reads CHIP Persisted Counter type nvm3 objects.
    // (where persistedStorageKey represents an index to the counter).
    CHIP_ERROR err;

    err = SilabsConfig::ReadConfigValueCounter(persistedStorageKey, value);
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
    // This method reads CHIP Persisted Counter type nvm3 objects.
    // (where persistedStorageKey represents an index to the counter).
    CHIP_ERROR err;

    err = SilabsConfig::WriteConfigValueCounter(persistedStorageKey, value);
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
    return SilabsConfig::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, uint32_t & val)
{
    return SilabsConfig::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, uint64_t & val)
{
    return SilabsConfig::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    return SilabsConfig::ReadConfigValueStr(key, buf, bufSize, outLen);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    return SilabsConfig::ReadConfigValueBin(key, buf, bufSize, outLen);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, bool val)
{
    return SilabsConfig::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, uint32_t val)
{
    return SilabsConfig::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, uint64_t val)
{
    return SilabsConfig::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueStr(Key key, const char * str)
{
    return SilabsConfig::WriteConfigValueStr(key, str);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    return SilabsConfig::WriteConfigValueStr(key, str, strLen);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    return SilabsConfig::WriteConfigValueBin(key, data, dataLen);
}

void ConfigurationManagerImpl::RunConfigUnitTest(void)
{
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    SilabsConfig::RunConfigUnitTest();
#endif // CONFIG_BUILD_FOR_HOST_UNIT_TEST
}

/// @brief Helper to erase Thread info from device
void ConfigurationManagerImpl::ClearThreadStack()
{
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
    ThreadStackMgr().ClearAllSrpHostAndServices();
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
    ChipLogProgress(DeviceLayer, "Clearing Thread provision");
    ThreadStackMgr().ErasePersistentInfo();
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD
}

void ConfigurationManagerImpl::DoFactoryReset(intptr_t arg)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    ChipLogProgress(DeviceLayer, "Performing factory reset");

    error = SilabsConfig::FactoryResetConfig();
    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "FactoryResetConfig() failed: %s", chip::ErrorStr(error));
    }

    GetDefaultInstance().ClearThreadStack();

    PersistedStorage::KeyValueStoreMgrImpl().ErasePartition();

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
    error = WifiInterface::GetInstance().TriggerDisconnection();
    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "TriggerDisconnection() failed: %s", chip::ErrorStr(error));
    }

    ChipLogProgress(DeviceLayer, "Clearing WiFi provision");
    WifiInterface::GetInstance().ClearWifiCredentials();
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION

    // Restart the system.
    ChipLogProgress(DeviceLayer, "System restarting");

    // When called from an RPC, the following reset occurs before the RPC can respond,
    // which breaks tests (because it looks like the RPC hasn't successfully completed).
    // Block the task for 500 ms before the reset occurs to allow RPC response to be sent
    osDelay(pdMS_TO_TICKS(500));

    Silabs::GetPlatform().SoftwareReset();
}

#ifdef SL_WIFI
CHIP_ERROR ConfigurationManagerImpl::GetPrimaryWiFiMACAddress(uint8_t * buf)
{
    VerifyOrReturnError(buf != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    MutableByteSpan byteSpan(buf, kPrimaryMACAddressLength);
    return WifiInterface::GetInstance().GetMacAddress(SL_WFX_STA_INTERFACE, byteSpan);
}
#endif

ConfigurationManager & ConfigurationMgrImpl()
{
    return ConfigurationManagerImpl::GetDefaultInstance();
}

} // namespace DeviceLayer
} // namespace chip
