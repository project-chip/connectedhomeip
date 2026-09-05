/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
 *          for nRF Connect SDK platforms.
 */

#include <platform/nrfconnect/ConfigurationManagerImpl.h>

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <app/server/Server.h> // nogncheck
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/Zephyr/ZephyrConfig.h>
#include <platform/internal/GenericConfigurationManagerImpl.h>

#ifdef CONFIG_CHIP_FACTORY_RESET_ERASE_SETTINGS
#include <zephyr/settings/settings.h>
#if defined(CONFIG_SETTINGS_NVS)
#include <zephyr/fs/nvs.h>
#elif defined(CONFIG_SETTINGS_ZMS) || defined(CONFIG_SETTINGS_ZMS_LEGACY)
#include <zephyr/fs/zms.h>
#endif // CONFIG_SETTINGS_NVS || CONFIG_SETTINGS_ZMS || CONFIG_SETTINGS_ZMS_LEGACY
#endif // CONFIG_CHIP_FACTORY_RESET_ERASE_SETTINGS

#if defined(CONFIG_OPENTHREAD) || defined(CONFIG_NET_L2_OPENTHREAD)
#include <platform/ThreadStackManager.h>
#endif

#ifdef CONFIG_SOC_FLASH_NRF_RADIO_SYNC_MPSL
#include <mpsl/mpsl_lib.h>
#endif // CONFIG_SOC_FLASH_NRF_RADIO_SYNC_MPSL

namespace chip {
namespace DeviceLayer {

using namespace ::chip::DeviceLayer::Internal;

ConfigurationManagerImplNrf & ConfigurationManagerImplNrf::GetDefaultInstance()
{
    static ConfigurationManagerImplNrf sInstance;
    return sInstance;
}

CHIP_ERROR ConfigurationManagerImplNrf::Init()
{
    ReturnErrorOnFailure(GenericConfigurationManagerImpl<ZephyrConfig>::Init());

    if (ZephyrConfig::ConfigValueExists(ZephyrConfig::kCounterKey_RebootCount))
    {
        uint32_t rebootCount;
        ReturnErrorOnFailure(GetRebootCount(rebootCount));

        // Do not increment reboot count if the value is going to overflow UINT16.
        ReturnErrorOnFailure(StoreRebootCount(rebootCount < UINT16_MAX ? rebootCount + 1 : rebootCount));
    }
    else
    {
        // The first boot after factory reset of the Node.
        ReturnErrorOnFailure(StoreRebootCount(1));
    }

    return PlatformMgr().AddEventHandler(FactoryResetEventHandler, 0);
}

void ConfigurationManagerImplNrf::FactoryResetEventHandler(const ChipDeviceEvent * event, intptr_t /* arg */)
{
    VerifyOrReturn(event->Type == DeviceEventType::kFactoryReset);

    ConfigurationManagerImplNrf & instance = GetDefaultInstance();
    instance.mFactoryResetScheduled        = true;

#ifdef CONFIG_CHIP_FACTORY_RESET_TIME_MEASUREMENT
    instance.mFactoryResetStartTime = System::SystemClock().GetMonotonicTimestamp();
#endif // CONFIG_CHIP_FACTORY_RESET_TIME_MEASUREMENT
}

void ConfigurationManagerImplNrf::InitiateFactoryReset()
{
    // Removing the persistent storage entries alone leaves the node in an inconsistent state, so
    // let the Matter server delete all the fabrics and emit the Leave event first. The server
    // announces that with the kFactoryReset event and then calls this method back.
    if (!mFactoryResetScheduled)
    {
        Server::GetInstance().ScheduleFactoryReset();
        return;
    }

    TEMPORARY_RETURN_IGNORED PlatformMgr().ScheduleWork(DoFactoryReset);
}

void ConfigurationManagerImplNrf::DoFactoryReset(intptr_t /* arg */)
{
    ChipLogProgress(DeviceLayer, "Performing factory reset");

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
    TEMPORARY_RETURN_IGNORED ThreadStackMgr().ClearAllSrpHostAndServices();
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT

// Lock the Thread stack to avoid unwanted interaction with settings NVS during factory reset.
#if defined(CONFIG_OPENTHREAD) || defined(CONFIG_NET_L2_OPENTHREAD)
    ThreadStackMgr().LockThreadStack();
#endif

#ifdef CONFIG_CHIP_FACTORY_RESET_ERASE_SETTINGS
    void * storage = nullptr;
    int status     = settings_storage_get(&storage);

    if (status == 0)
    {
#if defined(CONFIG_SETTINGS_NVS)
        status = nvs_clear(static_cast<nvs_fs *>(storage));
#elif defined(CONFIG_SETTINGS_ZMS) || defined(CONFIG_SETTINGS_ZMS_LEGACY)
        status = zms_clear(static_cast<zms_fs *>(storage));
#endif // CONFIG_SETTINGS_NVS || CONFIG_SETTINGS_ZMS || CONFIG_SETTINGS_ZMS_LEGACY
    }
    if (status)
    {
        ChipLogError(DeviceLayer, "Factory reset failed: %d", status);
    }
#else

    ConnectivityMgr().ErasePersistentInfo();

    // Suspending MPSL before the bulk of the flash operations speeds the factory reset up, because
    // the flash driver no longer waits for the MPSL synchronization on every write. This is only
    // needed when the flash driver and MPSL are actually synchronized.
#ifdef CONFIG_SOC_FLASH_NRF_RADIO_SYNC_MPSL
    mpsl_lib_uninit();
#endif // CONFIG_SOC_FLASH_NRF_RADIO_SYNC_MPSL

    const CHIP_ERROR err = PersistedStorage::KeyValueStoreMgrImpl().DoFactoryReset();

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Factory reset failed: %" CHIP_ERROR_FORMAT, err.Format());
    }
#endif // CONFIG_CHIP_FACTORY_RESET_ERASE_SETTINGS

#ifdef CONFIG_CHIP_FACTORY_RESET_TIME_MEASUREMENT
    ChipLogProgress(DeviceLayer, "Factory reset completed, it took: %" PRIu64 " ms",
                    GetDefaultInstance().GetFactoryResetDuration().count());
#endif // CONFIG_CHIP_FACTORY_RESET_TIME_MEASUREMENT

    PlatformMgr().Shutdown();
}

#ifdef CONFIG_CHIP_FACTORY_RESET_TIME_MEASUREMENT
System::Clock::Milliseconds64 ConfigurationManagerImplNrf::GetFactoryResetDuration() const
{
    const System::Clock::Timestamp currentTime = System::SystemClock().GetMonotonicTimestamp();

    VerifyOrReturnValue(currentTime >= mFactoryResetStartTime, System::Clock::Milliseconds64(0));

    return std::chrono::duration_cast<System::Clock::Milliseconds64>(currentTime - mFactoryResetStartTime);
}
#endif // CONFIG_CHIP_FACTORY_RESET_TIME_MEASUREMENT

} // namespace DeviceLayer
} // namespace chip
