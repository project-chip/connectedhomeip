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

#include <platform/ConfigurationManager.h>
#include <platform/internal/GenericConfigurationManagerImpl.ipp>

#include <core/CHIPKeyIds.h>
#include <core/CHIPVendorIdentifiers.hpp>
// #include <Profiles/security/ChipApplicationKeys.h>
#include <platform/ESP32/ESP32Config.h>
#include <platform/ESP32/GroupKeyStoreImpl.h>

#if CHIP_DEVICE_CONFIG_ENABLE_FACTORY_PROVISIONING
#include <platform/internal/FactoryProvisioning.ipp>
#endif // CHIP_DEVICE_CONFIG_ENABLE_FACTORY_PROVISIONING

#include "esp_wifi.h"
#include "nvs.h"
#include "nvs_flash.h"
namespace chip {
namespace DeviceLayer {

using namespace ::chip::Profiles::Security::AppKeys;
using namespace ::chip::Profiles::DeviceDescription;
using namespace ::chip::DeviceLayer::Internal;

using ::chip::kChipVendor_Common;

namespace {

enum
{
    kChipProduct_Connect = 0x0016
};

// Singleton instance of Chip Group Key Store for the ESP32
//
// NOTE: This is declared as a private global variable, rather than a static
// member of ConfigurationManagerImpl, to reduce the number of headers that
// must be included by the application when using the ConfigurationManager API.
//
GroupKeyStoreImpl gGroupKeyStore;

} // unnamed namespace

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

    // Initialize the global GroupKeyStore object.
    err = gGroupKeyStore.Init();
    SuccessOrExit(err);

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
    return esp_wifi_get_mac(ESP_IF_WIFI_STA, buf);
}

CHIP_ERROR ConfigurationManagerImpl::_GetDeviceDescriptor(::chip::Profiles::DeviceDescription::ChipDeviceDescriptor & deviceDesc)
{
    CHIP_ERROR err;

    // Call the generic version of _GetDeviceDescriptor() supplied by the base class.
    err = Internal::GenericConfigurationManagerImpl<ConfigurationManagerImpl>::_GetDeviceDescriptor(deviceDesc);
    SuccessOrExit(err);

    // If we're pretending to be a CHIP Connect, fake the presence of a 805.15.4 radio by returning
    // the Chip device id as the Primary 802.15.4 MAC address field.  This is necessary to fool
    // the CHIP mobile app into believing we are indeed a Connect.
    if (CHIP_DEVICE_CONFIG_DEVICE_VENDOR_ID == kChipVendor_Common && CHIP_DEVICE_CONFIG_DEVICE_PRODUCT_ID == kChipProduct_Connect)
    {
        Encoding::BigEndian::Put64(deviceDesc.Primary802154MACAddress, deviceDesc.DeviceId);
        deviceDesc.DeviceId = kNodeIdNotSpecified;
    }

exit:
    return err;
}

::chip::Profiles::Security::AppKeys::GroupKeyStoreBase * ConfigurationManagerImpl::_GetGroupKeyStore()
{
    return &gGroupKeyStore;
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

CHIP_ERROR ConfigurationManagerImpl::GetWiFiStationSecurityType(Profiles::NetworkProvisioning::WiFiSecurityType & secType)
{
    CHIP_ERROR err;
    uint32_t secTypeInt;

    err = ReadConfigValue(kConfigKey_WiFiStationSecType, secTypeInt);
    if (err == CHIP_NO_ERROR)
    {
        secType = (Profiles::NetworkProvisioning::WiFiSecurityType) secTypeInt;
    }
    return err;
}

CHIP_ERROR ConfigurationManagerImpl::UpdateWiFiStationSecurityType(Profiles::NetworkProvisioning::WiFiSecurityType secType)
{
    CHIP_ERROR err;
    Profiles::NetworkProvisioning::WiFiSecurityType curSecType;

    if (secType != Profiles::NetworkProvisioning::kWiFiSecurityType_NotSpecified)
    {
        err = GetWiFiStationSecurityType(curSecType);
        if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND || (err == CHIP_NO_ERROR && secType != curSecType))
        {
            uint32_t secTypeInt = secType;
            err                 = WriteConfigValue(kConfigKey_WiFiStationSecType, secTypeInt);
        }
        SuccessOrExit(err);
    }
    else
    {
        err = ClearConfigValue(kConfigKey_WiFiStationSecType);
        SuccessOrExit(err);
    }

exit:
    return err;
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
