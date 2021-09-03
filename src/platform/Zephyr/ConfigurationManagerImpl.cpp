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
#include <platform/internal/GenericConfigurationManagerImpl.cpp>

#include <lib/core/CHIPVendorIdentifiers.hpp>
#include <platform/Zephyr/ZephyrConfig.h>

#if CHIP_DEVICE_CONFIG_ENABLE_FACTORY_PROVISIONING
#include <platform/internal/FactoryProvisioning.cpp>
#endif // CHIP_DEVICE_CONFIG_ENABLE_FACTORY_PROVISIONING

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include <power/reboot.h>

namespace chip {
namespace DeviceLayer {

using namespace ::chip::DeviceLayer::Internal;

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

    // TODO: Initialize the global GroupKeyStore object here
#if CHIP_DEVICE_CONFIG_ENABLE_FACTORY_PROVISIONING
    {
        FactoryProvisioning factoryProv;
        uint8_t * const kDeviceRAMStart = (uint8_t *) CONFIG_SRAM_BASE_ADDRESS;
        uint8_t * const kDeviceRAMEnd   = kDeviceRAMStart + CONFIG_SRAM_SIZE * 1024 - 1;

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

void ConfigurationManagerImpl::_InitiateFactoryReset()
{
    PlatformMgr().ScheduleWork(DoFactoryReset);
}

void ConfigurationManagerImpl::DoFactoryReset(intptr_t arg)
{
    ChipLogProgress(DeviceLayer, "Performing factory reset");
    const CHIP_ERROR err = FactoryResetConfig();

    if (err != CHIP_NO_ERROR)
        ChipLogError(DeviceLayer, "FactoryResetConfig() failed: %s", ErrorStr(err));

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    ThreadStackMgr().ErasePersistentInfo();
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

#if CONFIG_REBOOT
    sys_reboot(SYS_REBOOT_WARM);
#endif
}

} // namespace DeviceLayer
} // namespace chip
