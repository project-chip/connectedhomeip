/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *          for the ESP32.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <core/CHIPKeyIds.h>
#include <platform/ConfigurationManager.h>
#include <platform/ESP32/ESP32Config.h>
#include <platform/internal/GenericConfigurationManagerImpl.cpp>
#include <support/CodeUtils.h>

#include "esp_wifi.h"
#include "nvs.h"
#include "nvs_flash.h"
namespace chip {
namespace DeviceLayer {

using namespace ::chip::DeviceLayer::Internal;

namespace {

enum
{
    kChipProduct_Connect = 0x0016
};

} // unnamed namespace

// TODO: Define a Singleton instance of CHIP Group Key Store here (#1266)

/** Singleton instance of the ConfigurationManager implementation object for the ESP32.
 */
ConfigurationManagerImpl ConfigurationManagerImpl::sInstance;

CHIP_ERROR ConfigurationManagerImpl::_Init()
{
    CHIP_ERROR err;
    bool failSafeArmed;

    // Force initialization of NVS namespaces if they doesn't already exist.
    err = EnsureNamespace(kConfigNamespace_ChipFactory);
    SuccessOrExit(err);
    err = EnsureNamespace(kConfigNamespace_ChipConfig);
    SuccessOrExit(err);
    err = EnsureNamespace(kConfigNamespace_ChipCounters);
    SuccessOrExit(err);

    // Initialize the generic implementation base class.
    err = Internal::GenericConfigurationManagerImpl<ConfigurationManagerImpl>::_Init();
    SuccessOrExit(err);

    // TODO: Initialize the global GroupKeyStore object here (#1266)

#if CHIP_DEVICE_CONFIG_ENABLE_FACTORY_PROVISIONING

    {
        FactoryProvisioning factoryProv;
        uint8_t * const kInternalSRAM12Start = (uint8_t *) 0x3FFAE000;
        uint8_t * const kInternalSRAM12End   = kInternalSRAM12Start + (328 * 1024) - 1;

        // Scan ESP32 Internal SRAM regions 1 and 2 for injected provisioning data and save
        // to persistent storage if found.
        err = factoryProv.ProvisionDeviceFromRAM(kInternalSRAM12Start, kInternalSRAM12End);
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

CHIP_ERROR ConfigurationManagerImpl::_GetPrimaryWiFiMACAddress(uint8_t * buf)
{
    return esp_wifi_get_mac(WIFI_IF_STA, buf);
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

CHIP_ERROR ConfigurationManagerImpl::_ReadPersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t & value)
{
    ESP32Config::Key configKey{ kConfigNamespace_ChipCounters, key };

    CHIP_ERROR err = ReadConfigValue(configKey, value);
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }
    return err;
}

CHIP_ERROR ConfigurationManagerImpl::_WritePersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t value)
{
    ESP32Config::Key configKey{ kConfigNamespace_ChipCounters, key };
    return WriteConfigValue(configKey, value);
}

void ConfigurationManagerImpl::DoFactoryReset(intptr_t arg)
{
    CHIP_ERROR err;

    ChipLogProgress(DeviceLayer, "Performing factory reset");

    // Erase all values in the chip-config NVS namespace.
    err = ClearNamespace(kConfigNamespace_ChipConfig);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "ClearNamespace(ChipConfig) failed: %s", chip::ErrorStr(err));
    }

    // Restore WiFi persistent settings to default values.
    err = esp_wifi_restore();
    if (err != ESP_OK)
    {
        ChipLogError(DeviceLayer, "esp_wifi_restore() failed: %s", chip::ErrorStr(err));
    }

    // Restart the system.
    ChipLogProgress(DeviceLayer, "System restarting");
    esp_restart();
}

} // namespace DeviceLayer
} // namespace chip
