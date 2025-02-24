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
#include <platform/ESP32/ESP32Utils.h>
#include <platform/ESP32/ScopedNvsHandle.h>
#include <platform/internal/GenericConfigurationManagerImpl.ipp>

#if CHIP_DEVICE_CONFIG_ENABLE_ETHERNET
#include "esp_mac.h"
#endif
#include "esp_ota_ops.h"
#ifndef CONFIG_IDF_TARGET_ESP32P4
#include "esp_phy_init.h"
#endif
#include "esp_wifi.h"
#include "nvs.h"
#include "nvs_flash.h"
namespace chip {
namespace DeviceLayer {

using namespace ::chip::DeviceLayer::Internal;

ConfigurationManagerImpl & ConfigurationManagerImpl::GetDefaultInstance()
{
    static ConfigurationManagerImpl sInstance;
    return sInstance;
}

uint32_t ConfigurationManagerImpl::mTotalOperationalHours = 0;

void ConfigurationManagerImpl::TotalOperationalHoursTimerCallback(TimerHandle_t timer)
{
    // This function is called from the FreeRTOS timer task. Since the task stack is limited,
    // we avoid logging error messages here to prevent stack overflows.
    (void) ConfigurationMgrImpl().StoreTotalOperationalHours(++mTotalOperationalHours);
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
        err = MapConfigError(esp_err);
        SuccessOrExit(err);
    }

    esp_err = nvs_flash_read_security_cfg(key_part, &cfg);
    if (esp_err == ESP_ERR_NVS_KEYS_NOT_INITIALIZED)
    {
        ChipLogError(DeviceLayer, "NVS key partition empty");
        err = MapConfigError(esp_err);
        SuccessOrExit(err);
    }
    else if (esp_err != ESP_OK)
    {
        ChipLogError(DeviceLayer, "Failed to read NVS security cfg, err:0x%02x", esp_err);
        err = MapConfigError(esp_err);
        SuccessOrExit(err);
    }

    // Securely initialize the nvs partitions,
    // nvs_flash_secure_init_partition() will initialize the partition only if it is not already initialized.
    esp_err = nvs_flash_secure_init_partition(CHIP_DEVICE_CONFIG_CHIP_FACTORY_NAMESPACE_PARTITION, &cfg);
    if (esp_err == ESP_ERR_NVS_NO_FREE_PAGES || esp_err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ChipLogError(DeviceLayer, "Failed to initialize NVS partition %s err:0x%02x",
                     CHIP_DEVICE_CONFIG_CHIP_FACTORY_NAMESPACE_PARTITION, esp_err);
        err = MapConfigError(esp_err);
        SuccessOrExit(err);
    }

    esp_err = nvs_flash_secure_init_partition(CHIP_DEVICE_CONFIG_CHIP_CONFIG_NAMESPACE_PARTITION, &cfg);
    if (esp_err == ESP_ERR_NVS_NO_FREE_PAGES || esp_err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ChipLogError(DeviceLayer, "Failed to initialize NVS partition %s err:0x%02x",
                     CHIP_DEVICE_CONFIG_CHIP_CONFIG_NAMESPACE_PARTITION, esp_err);
        err = MapConfigError(esp_err);
        SuccessOrExit(err);
    }

    esp_err = nvs_flash_secure_init_partition(CHIP_DEVICE_CONFIG_CHIP_COUNTERS_NAMESPACE_PARTITION, &cfg);
    if (esp_err == ESP_ERR_NVS_NO_FREE_PAGES || esp_err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ChipLogError(DeviceLayer, "Failed to initialize NVS partition %s err:0x%02x",
                     CHIP_DEVICE_CONFIG_CHIP_COUNTERS_NAMESPACE_PARTITION, esp_err);
        err = MapConfigError(esp_err);
        SuccessOrExit(err);
    }

    esp_err = nvs_flash_secure_init_partition(CHIP_DEVICE_CONFIG_CHIP_KVS_NAMESPACE_PARTITION, &cfg);
    if (esp_err == ESP_ERR_NVS_NO_FREE_PAGES || esp_err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ChipLogError(DeviceLayer, "Failed to initialize NVS partition %s err:0x%02x",
                     CHIP_DEVICE_CONFIG_CHIP_KVS_NAMESPACE_PARTITION, esp_err);
        err = MapConfigError(esp_err);
        SuccessOrExit(err);
    }
#else
    // Initialize the nvs partitions,
    // nvs_flash_init_partition() will initialize the partition only if it is not already initialized.
    err = MapConfigError(nvs_flash_init_partition(CHIP_DEVICE_CONFIG_CHIP_FACTORY_NAMESPACE_PARTITION));
    SuccessOrExit(err);

    err = MapConfigError(nvs_flash_init_partition(CHIP_DEVICE_CONFIG_CHIP_CONFIG_NAMESPACE_PARTITION));
    SuccessOrExit(err);

    err = MapConfigError(nvs_flash_init_partition(CHIP_DEVICE_CONFIG_CHIP_COUNTERS_NAMESPACE_PARTITION));
    SuccessOrExit(err);

    err = MapConfigError(nvs_flash_init_partition(CHIP_DEVICE_CONFIG_CHIP_KVS_NAMESPACE_PARTITION));
    SuccessOrExit(err);
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

    if (CHIP_NO_ERROR != GetTotalOperationalHours(mTotalOperationalHours))
    {
        err = StoreTotalOperationalHours(mTotalOperationalHours);
        SuccessOrExit(err);
    }

    {
        // The total-operational-hours is critical information. It intentionally uses the FreeRTOS timer
        // to increment the value, this ensures it is not affected by PostEvent failures.

        // Start a timer which reloads every one hour and bumps the total operational hours
        TickType_t reloadPeriod   = (1000 * 60 * 60) / portTICK_PERIOD_MS;
        TimerHandle_t timerHandle = xTimerCreate("tOpHrs", reloadPeriod, pdPASS, nullptr, TotalOperationalHoursTimerCallback);
        if (timerHandle == nullptr)
        {
            err = CHIP_ERROR_NO_MEMORY;
            ExitNow(ChipLogError(DeviceLayer, "total operational hours Timer creation failed"));
        }

        BaseType_t timerStartStatus = xTimerStart(timerHandle, 0);
        if (timerStartStatus == pdFAIL)
        {
            err = CHIP_ERROR_INTERNAL;
            ExitNow(ChipLogError(DeviceLayer, "total operational hours Timer start failed"));
        }
    }

    // Initialize the generic implementation base class.
    err = Internal::GenericConfigurationManagerImpl<ESP32Config>::Init();
    SuccessOrExit(err);

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
    ScopedNvsHandle handle;
    ESP32Config::Key key = ESP32Config::kCounterKey_TotalOperationalHours;

    ReturnErrorOnFailure(handle.Open(key.Namespace, NVS_READWRITE, ESP32Config::GetPartitionLabelByNamespace(key.Namespace)));
    ReturnMappedErrorOnFailure(nvs_set_u32(handle, key.Name, totalOperationalHours));
    ReturnMappedErrorOnFailure(nvs_commit(handle));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConfigurationManagerImpl::GetSoftwareVersionString(char * buf, size_t bufSize)
{
    memset(buf, 0, bufSize);
    const esp_app_desc_t * appDescription = NULL;

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
    appDescription = esp_app_get_description();
#else
    appDescription = esp_ota_get_app_description();
#endif

    VerifyOrReturnError(bufSize >= sizeof(appDescription->version), CHIP_ERROR_BUFFER_TOO_SMALL);
    VerifyOrReturnError(sizeof(appDescription->version) <= ConfigurationManager::kMaxSoftwareVersionStringLength,
                        CHIP_ERROR_INTERNAL);
    strcpy(buf, appDescription->version);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConfigurationManagerImpl::GetSoftwareVersion(uint32_t & softwareVer)
{
    softwareVer = CHIP_CONFIG_SOFTWARE_VERSION_NUMBER;
    return CHIP_NO_ERROR;
}

CHIP_ERROR ConfigurationManagerImpl::GetLocationCapability(uint8_t & location)
{
#ifdef CONFIG_ENABLE_ESP32_LOCATIONCAPABILITY
    uint32_t value = 0;
    CHIP_ERROR err = ReadConfigValue(ESP32Config::kConfigKey_LocationCapability, value);

    if (err == CHIP_NO_ERROR)
    {
        VerifyOrReturnError(value <= UINT8_MAX, CHIP_ERROR_INVALID_INTEGER_VALUE);
        location = static_cast<uint8_t>(value);
    }

    return err;
#else
    location       = static_cast<uint8_t>(chip::app::Clusters::GeneralCommissioning::RegulatoryLocationTypeEnum::kIndoor);
    return CHIP_NO_ERROR;
#endif // CONFIG_ENABLE_ESP32_LOCATIONCAPABILITY
}

CHIP_ERROR ConfigurationManagerImpl::GetDeviceTypeId(uint32_t & deviceType)
{
    uint32_t value = 0;
    CHIP_ERROR err = ReadConfigValue(ESP32Config::kConfigKey_PrimaryDeviceType, value);

    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        deviceType = CHIP_DEVICE_CONFIG_DEVICE_TYPE;
    }
    else
    {
        deviceType = value;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConfigurationManagerImpl::StoreCountryCode(const char * code, size_t codeLen)
{
    // As per spec, codeLen has to be 2
    VerifyOrReturnError((code != nullptr) && (codeLen == 2), CHIP_ERROR_INVALID_ARGUMENT);

    // Setting country is only possible on WiFi supported SoCs
#ifdef CONFIG_ESP32_WIFI_ENABLED
    // Write CountryCode to esp_phy layer
    ReturnErrorOnFailure(MapConfigError(esp_phy_update_country_info(code)));
#endif

    // As we do not have API to read country code from esp_phy layer, we are writing to NVS and when client reads the
    // CountryCode then we read from NVS
    return GenericConfigurationManagerImpl<ESP32Config>::StoreCountryCode(code, codeLen);
}

#if CHIP_DEVICE_CONFIG_ENABLE_ETHERNET

CHIP_ERROR ConfigurationManagerImpl::GetPrimaryMACAddress(MutableByteSpan & buf)
{
    if (GetPrimaryEthernetMACAddress(buf) == CHIP_NO_ERROR)
    {
        ChipLogDetail(DeviceLayer, "Using Ethernet MAC for hostname.");
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR ConfigurationManagerImpl::GetPrimaryEthernetMACAddress(MutableByteSpan & buf)
{
    if (buf.size() < ConfigurationManager::kPrimaryMACAddressLength)
        return CHIP_ERROR_BUFFER_TOO_SMALL;

    memset(buf.data(), 0, buf.size());

    esp_err_t err = esp_read_mac(buf.data(), ESP_MAC_ETH);
    buf.reduce_size(ConfigurationManager::kPrimaryMACAddressLength);
    return MapConfigError(err);
}
#endif

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
        return CHIP_ERROR_UNINITIALIZED;
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

    // Unregistering the wifi and IP event handlers from the esp_default_event_loop()
    err = ESP32Utils::MapError(esp_event_handler_unregister(IP_EVENT, ESP_EVENT_ANY_ID, PlatformManagerImpl::HandleESPSystemEvent));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to unregister IP event handler");
    }

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    err =
        ESP32Utils::MapError(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, PlatformManagerImpl::HandleESPSystemEvent));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to unregister wifi event handler");
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

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
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
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
