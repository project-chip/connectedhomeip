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

#include <lib/core/CHIPKeyIds.h>
#include <lib/support/CodeUtils.h>
#include <platform/ConfigurationManager.h>
#include <platform/ESP32/ESP32Config.h>
#include <platform/internal/GenericConfigurationManagerImpl.ipp>

#include "esp_ota_ops.h"
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

ConfigurationManagerImpl & ConfigurationManagerImpl::GetDefaultInstance()
{
    static ConfigurationManagerImpl sInstance;
    return sInstance;
}

CHIP_ERROR ConfigurationManagerImpl::Init()
{
    CHIP_ERROR err;
    uint32_t rebootCount;

#ifdef CONFIG_NVS_ENCRYPTION
    nvs_sec_cfg_t cfg = {};
    esp_err_t esp_err = ESP_FAIL;

    const esp_partition_t * key_part = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_NVS_KEYS, NULL);
    if (key_part == NULL)
    {
        ChipLogError(DeviceLayer,
                     "CONFIG_NVS_ENCRYPTION is enabled, but no partition with subtype nvs_keys found in the partition table.");
        SuccessOrExit(MapConfigError(esp_err));
    }

    esp_err = nvs_flash_read_security_cfg(key_part, &cfg);
    if (esp_err == ESP_ERR_NVS_KEYS_NOT_INITIALIZED)
    {
        ChipLogError(DeviceLayer, "NVS key partition empty");
        SuccessOrExit(MapConfigError(esp_err));
    }
    else if (esp_err != ESP_OK)
    {
        ChipLogError(DeviceLayer, "Failed to read NVS security cfg, err:0x%02x", esp_err);
        SuccessOrExit(MapConfigError(esp_err));
    }

    // Securely initialize the nvs partitions,
    // nvs_flash_secure_init_partition() will initialize the partition only if it is not already initialized.
    esp_err = nvs_flash_secure_init_partition(CHIP_DEVICE_CONFIG_CHIP_FACTORY_NAMESPACE_PARTITION, &cfg);
    if (esp_err == ESP_ERR_NVS_NO_FREE_PAGES || esp_err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ChipLogError(DeviceLayer, "Failed to initialize NVS partition %s err:0x%02x",
                     CHIP_DEVICE_CONFIG_CHIP_FACTORY_NAMESPACE_PARTITION, esp_err);
        SuccessOrExit(MapConfigError(esp_err));
    }

    esp_err = nvs_flash_secure_init_partition(CHIP_DEVICE_CONFIG_CHIP_CONFIG_NAMESPACE_PARTITION, &cfg);
    if (esp_err == ESP_ERR_NVS_NO_FREE_PAGES || esp_err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ChipLogError(DeviceLayer, "Failed to initialize NVS partition %s err:0x%02x",
                     CHIP_DEVICE_CONFIG_CHIP_CONFIG_NAMESPACE_PARTITION, esp_err);
        SuccessOrExit(MapConfigError(esp_err));
    }

    esp_err = nvs_flash_secure_init_partition(CHIP_DEVICE_CONFIG_CHIP_COUNTERS_NAMESPACE_PARTITION, &cfg);
    if (esp_err == ESP_ERR_NVS_NO_FREE_PAGES || esp_err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ChipLogError(DeviceLayer, "Failed to initialize NVS partition %s err:0x%02x",
                     CHIP_DEVICE_CONFIG_CHIP_COUNTERS_NAMESPACE_PARTITION, esp_err);
        SuccessOrExit(MapConfigError(esp_err));
    }
#else
    // Initialize the nvs partitions,
    // nvs_flash_init_partition() will initialize the partition only if it is not already initialized.
    esp_err_t esp_err = nvs_flash_init_partition(CHIP_DEVICE_CONFIG_CHIP_FACTORY_NAMESPACE_PARTITION);
    SuccessOrExit(MapConfigError(esp_err));
    esp_err = nvs_flash_init_partition(CHIP_DEVICE_CONFIG_CHIP_CONFIG_NAMESPACE_PARTITION);
    SuccessOrExit(MapConfigError(esp_err));
    esp_err = nvs_flash_init_partition(CHIP_DEVICE_CONFIG_CHIP_COUNTERS_NAMESPACE_PARTITION);
    SuccessOrExit(MapConfigError(esp_err));
    esp_err = nvs_flash_init_partition(CHIP_DEVICE_CONFIG_CHIP_KVS_NAMESPACE_PARTITION);
    SuccessOrExit(MapConfigError(esp_err));
#endif

    // Force initialization of NVS namespaces if they doesn't already exist.
    err = ESP32Config::EnsureNamespace(ESP32Config::kConfigNamespace_ChipFactory);
    SuccessOrExit(err);
    err = ESP32Config::EnsureNamespace(ESP32Config::kConfigNamespace_ChipConfig);
    SuccessOrExit(err);
    err = ESP32Config::EnsureNamespace(ESP32Config::kConfigNamespace_ChipCounters);
    SuccessOrExit(err);

    if (ESP32Config::ConfigValueExists(ESP32Config::kCounterKey_RebootCount))
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

    if (!ESP32Config::ConfigValueExists(ESP32Config::kCounterKey_TotalOperationalHours))
    {
        err = StoreTotalOperationalHours(0);
        SuccessOrExit(err);
    }

    // Initialize the generic implementation base class.
    err = Internal::GenericConfigurationManagerImpl<ESP32Config>::Init();
    SuccessOrExit(err);

    // TODO: Initialize the global GroupKeyStore object here (#1266)

    err = CHIP_NO_ERROR;

exit:
    return err;
}

CHIP_ERROR ConfigurationManagerImpl::GetRebootCount(uint32_t & rebootCount)
{
    return ReadConfigValue(ESP32Config::kCounterKey_RebootCount, rebootCount);
}

CHIP_ERROR ConfigurationManagerImpl::StoreRebootCount(uint32_t rebootCount)
{
    return WriteConfigValue(ESP32Config::kCounterKey_RebootCount, rebootCount);
}

CHIP_ERROR ConfigurationManagerImpl::GetTotalOperationalHours(uint32_t & totalOperationalHours)
{
    return ReadConfigValue(ESP32Config::kCounterKey_TotalOperationalHours, totalOperationalHours);
}

CHIP_ERROR ConfigurationManagerImpl::StoreTotalOperationalHours(uint32_t totalOperationalHours)
{
    return WriteConfigValue(ESP32Config::kCounterKey_TotalOperationalHours, totalOperationalHours);
}

CHIP_ERROR ConfigurationManagerImpl::GetSoftwareVersionString(char * buf, size_t bufSize)
{
    memset(buf, 0, bufSize);
    const esp_app_desc_t * appDescription = esp_ota_get_app_description();
    ReturnErrorCodeIf(bufSize < sizeof(appDescription->version), CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorCodeIf(sizeof(appDescription->version) > ConfigurationManager::kMaxSoftwareVersionStringLength, CHIP_ERROR_INTERNAL);
    strcpy(buf, appDescription->version);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConfigurationManagerImpl::GetSoftwareVersion(uint32_t & softwareVer)
{
    softwareVer = CHIP_CONFIG_SOFTWARE_VERSION_NUMBER;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConfigurationManagerImpl::GetPrimaryWiFiMACAddress(uint8_t * buf)
{
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    wifi_mode_t mode;
    esp_wifi_get_mode(&mode);
    if ((mode == WIFI_MODE_AP) || (mode == WIFI_MODE_APSTA))
        return MapConfigError(esp_wifi_get_mac(WIFI_IF_AP, buf));
    else
        return MapConfigError(esp_wifi_get_mac(WIFI_IF_STA, buf));
#else
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif
}

CHIP_ERROR ConfigurationManagerImpl::MapConfigError(esp_err_t error)
{
    switch (error)
    {
    case ESP_OK:
        return CHIP_NO_ERROR;
    case ESP_ERR_WIFI_NOT_INIT:
        return CHIP_ERROR_WELL_UNINITIALIZED;
    case ESP_ERR_INVALID_ARG:
    case ESP_ERR_WIFI_IF:
        return CHIP_ERROR_INVALID_ARGUMENT;
    default:
        return CHIP_ERROR_INTERNAL;
    }
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
    ESP32Config::Key configKey{ ESP32Config::kConfigNamespace_ChipCounters, key };

    CHIP_ERROR err = ReadConfigValue(configKey, value);
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }
    return err;
}

CHIP_ERROR ConfigurationManagerImpl::WritePersistedStorageValue(::chip::Platform::PersistedStorage::Key key, uint32_t value)
{
    ESP32Config::Key configKey{ ESP32Config::kConfigNamespace_ChipCounters, key };
    return WriteConfigValue(configKey, value);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, bool & val)
{
    return ESP32Config::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, uint32_t & val)
{
    return ESP32Config::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(Key key, uint64_t & val)
{
    return ESP32Config::ReadConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    return ESP32Config::ReadConfigValueStr(key, buf, bufSize, outLen);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    return ESP32Config::ReadConfigValueBin(key, buf, bufSize, outLen);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, bool val)
{
    return ESP32Config::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, uint32_t val)
{
    return ESP32Config::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValue(Key key, uint64_t val)
{
    return ESP32Config::WriteConfigValue(key, val);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueStr(Key key, const char * str)
{
    return ESP32Config::WriteConfigValueStr(key, str);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    return ESP32Config::WriteConfigValueStr(key, str, strLen);
}

CHIP_ERROR ConfigurationManagerImpl::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    return ESP32Config::WriteConfigValueBin(key, data, dataLen);
}

void ConfigurationManagerImpl::RunConfigUnitTest(void)
{
    ESP32Config::RunConfigUnitTest();
}

void ConfigurationManagerImpl::DoFactoryReset(intptr_t arg)
{
    CHIP_ERROR err;

    ChipLogProgress(DeviceLayer, "Performing factory reset");

    // Erase all values in the chip-config NVS namespace.
    err = ESP32Config::ClearNamespace(ESP32Config::kConfigNamespace_ChipConfig);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "ClearNamespace(ChipConfig) failed: %s", chip::ErrorStr(err));
    }

    // Erase all values in the chip-counters NVS namespace.
    err = ESP32Config::ClearNamespace(ESP32Config::kConfigNamespace_ChipCounters);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "ClearNamespace(ChipCounters) failed: %s", chip::ErrorStr(err));
    }

    // Restore WiFi persistent settings to default values.
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    esp_err_t error = esp_wifi_restore();
    if (error != ESP_OK)
    {
        ChipLogError(DeviceLayer, "esp_wifi_restore() failed: %s", esp_err_to_name(error));
    }
#elif CHIP_DEVICE_CONFIG_ENABLE_THREAD
    ThreadStackMgr().ErasePersistentInfo();
#endif

    // Erase all key-values including fabric info.
    err = PersistedStorage::KeyValueStoreMgrImpl().EraseAll();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Clear Key-Value Storage failed");
    }

    // Restart the system.
    ChipLogProgress(DeviceLayer, "System restarting");
    esp_restart();
}

ConfigurationManager & ConfigurationMgrImpl()
{
    return ConfigurationManagerImpl::GetDefaultInstance();
}

} // namespace DeviceLayer
} // namespace chip
