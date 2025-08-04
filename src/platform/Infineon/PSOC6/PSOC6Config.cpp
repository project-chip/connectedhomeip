/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
 *    Copyright (c) 2019-2020 Google LLC.
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
 *          Utilities for interacting with the the PSOC6 key-value store.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/KeyValueStoreManager.h>

#include <platform/Infineon/PSOC6/PSOC6Config.h>

#include <lib/core/CHIPEncoding.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/Infineon/PSOC6/PSOC6Utils.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

// *** CAUTION ***: Changing the names or namespaces of these values will *break* existing devices.

// Namespaces used to store device configuration information.
const char PSOC6Config::kConfigNamespace_ChipFactory[]  = "chip-factory";
const char PSOC6Config::kConfigNamespace_ChipConfig[]   = "chip-config";
const char PSOC6Config::kConfigNamespace_ChipCounters[] = "chip-counters";

// Keys stored in the chip-factory namespace
const PSOC6Config::Key PSOC6Config::kConfigKey_SerialNum             = { kConfigNamespace_ChipFactory, "serial-num" };
const PSOC6Config::Key PSOC6Config::kConfigKey_MfrDeviceId           = { kConfigNamespace_ChipFactory, "device-id" };
const PSOC6Config::Key PSOC6Config::kConfigKey_MfrDeviceCert         = { kConfigNamespace_ChipFactory, "device-cert" };
const PSOC6Config::Key PSOC6Config::kConfigKey_MfrDeviceICACerts     = { kConfigNamespace_ChipFactory, "device-ca-certs" };
const PSOC6Config::Key PSOC6Config::kConfigKey_MfrDevicePrivateKey   = { kConfigNamespace_ChipFactory, "device-key" };
const PSOC6Config::Key PSOC6Config::kConfigKey_SoftwareVersion       = { kConfigNamespace_ChipFactory, "software-ver" };
const PSOC6Config::Key PSOC6Config::kConfigKey_HardwareVersion       = { kConfigNamespace_ChipFactory, "hardware-ver" };
const PSOC6Config::Key PSOC6Config::kConfigKey_ManufacturingDate     = { kConfigNamespace_ChipFactory, "mfg-date" };
const PSOC6Config::Key PSOC6Config::kConfigKey_SetupPinCode          = { kConfigNamespace_ChipFactory, "pin-code" };
const PSOC6Config::Key PSOC6Config::kConfigKey_SetupDiscriminator    = { kConfigNamespace_ChipFactory, "discriminator" };
const PSOC6Config::Key PSOC6Config::kConfigKey_Spake2pIterationCount = { kConfigNamespace_ChipFactory, "iteration-count" };
const PSOC6Config::Key PSOC6Config::kConfigKey_Spake2pSalt           = { kConfigNamespace_ChipFactory, "salt" };
const PSOC6Config::Key PSOC6Config::kConfigKey_Spake2pVerifier       = { kConfigNamespace_ChipFactory, "verifier" };

// Keys stored in the chip-config namespace
const PSOC6Config::Key PSOC6Config::kConfigKey_ServiceConfig      = { kConfigNamespace_ChipConfig, "service-config" };
const PSOC6Config::Key PSOC6Config::kConfigKey_PairedAccountId    = { kConfigNamespace_ChipConfig, "account-id" };
const PSOC6Config::Key PSOC6Config::kConfigKey_ServiceId          = { kConfigNamespace_ChipConfig, "service-id" };
const PSOC6Config::Key PSOC6Config::kConfigKey_LastUsedEpochKeyId = { kConfigNamespace_ChipConfig, "last-ek-id" };
const PSOC6Config::Key PSOC6Config::kConfigKey_FailSafeArmed      = { kConfigNamespace_ChipConfig, "fail-safe-armed" };
const PSOC6Config::Key PSOC6Config::kConfigKey_WiFiStationSecType = { kConfigNamespace_ChipConfig, "sta-sec-type" };
const PSOC6Config::Key PSOC6Config::kConfigKey_RegulatoryLocation = { kConfigNamespace_ChipConfig, "regulatory-location" };
const PSOC6Config::Key PSOC6Config::kConfigKey_CountryCode        = { kConfigNamespace_ChipConfig, "country-code" };
const PSOC6Config::Key PSOC6Config::kConfigKey_WiFiSSID           = { kConfigNamespace_ChipConfig, "wifi-ssid" };
const PSOC6Config::Key PSOC6Config::kConfigKey_WiFiPassword       = { kConfigNamespace_ChipConfig, "wifi-password" };
const PSOC6Config::Key PSOC6Config::kConfigKey_WiFiSecurity       = { kConfigNamespace_ChipConfig, "wifi-security" };
const PSOC6Config::Key PSOC6Config::kConfigKey_WiFiMode           = { kConfigNamespace_ChipConfig, "wifimode" };
const PSOC6Config::Key PSOC6Config::kConfigKey_UniqueId           = { kConfigNamespace_ChipConfig, "unique-id" };
const PSOC6Config::Key PSOC6Config::kConfigKey_LockUser           = { kConfigNamespace_ChipConfig, "lock-user" };
const PSOC6Config::Key PSOC6Config::kConfigKey_Credential         = { kConfigNamespace_ChipConfig, "credential" };
const PSOC6Config::Key PSOC6Config::kConfigKey_LockUserName       = { kConfigNamespace_ChipConfig, "lock-user-name" };
const PSOC6Config::Key PSOC6Config::kConfigKey_CredentialData     = { kConfigNamespace_ChipConfig, "credential-data" };
const PSOC6Config::Key PSOC6Config::kConfigKey_UserCredentials    = { kConfigNamespace_ChipConfig, "user-credentials" };
const PSOC6Config::Key PSOC6Config::kConfigKey_WeekDaySchedules   = { kConfigNamespace_ChipConfig, "weekday-schedules" };
;
const PSOC6Config::Key PSOC6Config::kConfigKey_YearDaySchedules = { kConfigNamespace_ChipConfig, "yearday-schedules" };
;
const PSOC6Config::Key PSOC6Config::kConfigKey_HolidaySchedules = { kConfigNamespace_ChipConfig, "holiday-schedules" };
;

// Keys stored in the Chip-counters namespace
const PSOC6Config::Key PSOC6Config::kCounterKey_RebootCount           = { kConfigNamespace_ChipCounters, "reboot-count" };
const PSOC6Config::Key PSOC6Config::kCounterKey_UpTime                = { kConfigNamespace_ChipCounters, "up-time" };
const PSOC6Config::Key PSOC6Config::kCounterKey_TotalOperationalHours = { kConfigNamespace_ChipCounters, "total-hours" };

CHIP_ERROR PSOC6Config::ReadConfigValue(Key key, bool & val)
{
    bool in;
    char key_str[MTB_KVSTORE_MAX_KEY_SIZE] = { 0 };
    key.to_str(key_str, MTB_KVSTORE_MAX_KEY_SIZE);
    CHIP_ERROR err = PersistedStorage::KeyValueStoreMgr().Get(key_str, static_cast<void *>(&in), sizeof(bool));
    val            = in;
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    return err;
}

CHIP_ERROR PSOC6Config::ReadConfigValue(Key key, uint32_t & val)
{
    uint32_t in;
    char key_str[MTB_KVSTORE_MAX_KEY_SIZE] = { 0 };
    key.to_str(key_str, MTB_KVSTORE_MAX_KEY_SIZE);
    CHIP_ERROR err = PersistedStorage::KeyValueStoreMgr().Get(key_str, static_cast<void *>(&in), 4);
    val            = in;
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    return err;
}

CHIP_ERROR PSOC6Config::ReadConfigValue(Key key, uint64_t & val)
{
    uint64_t in;
    char key_str[MTB_KVSTORE_MAX_KEY_SIZE] = { 0 };
    key.to_str(key_str, MTB_KVSTORE_MAX_KEY_SIZE);
    CHIP_ERROR err = PersistedStorage::KeyValueStoreMgr().Get(key_str, static_cast<void *>(&in), 8);
    val            = in;
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    return err;
}

CHIP_ERROR PSOC6Config::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    char key_str[MTB_KVSTORE_MAX_KEY_SIZE] = { 0 };
    key.to_str(key_str, MTB_KVSTORE_MAX_KEY_SIZE);
    CHIP_ERROR err = PersistedStorage::KeyValueStoreMgr().Get(key_str, buf, bufSize, &outLen);
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    return err;
}

CHIP_ERROR PSOC6Config::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    char key_str[MTB_KVSTORE_MAX_KEY_SIZE] = { 0 };
    key.to_str(key_str, MTB_KVSTORE_MAX_KEY_SIZE);
    CHIP_ERROR err = PersistedStorage::KeyValueStoreMgr().Get(key_str, buf, bufSize, &outLen);
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    return err;
}

CHIP_ERROR PSOC6Config::WriteConfigValue(Key key, bool val)
{
    char key_str[MTB_KVSTORE_MAX_KEY_SIZE] = { 0 };
    key.to_str(key_str, MTB_KVSTORE_MAX_KEY_SIZE);
    return PersistedStorage::KeyValueStoreMgr().Put(key_str, static_cast<void *>(&val), sizeof(bool));
}

CHIP_ERROR PSOC6Config::WriteConfigValue(Key key, uint32_t val)
{
    char key_str[MTB_KVSTORE_MAX_KEY_SIZE] = { 0 };
    key.to_str(key_str, MTB_KVSTORE_MAX_KEY_SIZE);
    return PersistedStorage::KeyValueStoreMgr().Put(key_str, static_cast<void *>(&val), 4);
}

CHIP_ERROR PSOC6Config::WriteConfigValue(Key key, uint64_t val)
{
    char key_str[MTB_KVSTORE_MAX_KEY_SIZE] = { 0 };
    key.to_str(key_str, MTB_KVSTORE_MAX_KEY_SIZE);
    return PersistedStorage::KeyValueStoreMgr().Put(key_str, static_cast<void *>(&val), 8);
}

CHIP_ERROR PSOC6Config::WriteConfigValueStr(Key key, const char * str)
{
    size_t size                            = strlen(str) + 1;
    char key_str[MTB_KVSTORE_MAX_KEY_SIZE] = { 0 };
    key.to_str(key_str, MTB_KVSTORE_MAX_KEY_SIZE);
    return PersistedStorage::KeyValueStoreMgr().Put(key_str, str, size);
}

CHIP_ERROR PSOC6Config::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    char key_str[MTB_KVSTORE_MAX_KEY_SIZE] = { 0 };
    key.to_str(key_str, MTB_KVSTORE_MAX_KEY_SIZE);
    return PersistedStorage::KeyValueStoreMgr().Put(key_str, str, strLen);
}
CHIP_ERROR PSOC6Config::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    char key_str[MTB_KVSTORE_MAX_KEY_SIZE] = { 0 };
    key.to_str(key_str, MTB_KVSTORE_MAX_KEY_SIZE);
    return PersistedStorage::KeyValueStoreMgr().Put(key_str, data, dataLen);
}

CHIP_ERROR PSOC6Config::ClearConfigValue(Key key)
{
    char key_str[MTB_KVSTORE_MAX_KEY_SIZE] = { 0 };
    key.to_str(key_str, MTB_KVSTORE_MAX_KEY_SIZE);
    return PersistedStorage::KeyValueStoreMgr().Delete(key_str);
}

bool PSOC6Config::ConfigValueExists(Key key)
{
    char key_str[MTB_KVSTORE_MAX_KEY_SIZE] = { 0 };
    key.to_str(key_str, MTB_KVSTORE_MAX_KEY_SIZE);
    if (PersistedStorage::KeyValueStoreMgr().Get(key_str, NULL, 0) == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        return false;
    }

    return true;
}

// Clear out keys in config namespace
CHIP_ERROR PSOC6Config::FactoryResetConfig(void)
{
    CHIP_ERROR err            = CHIP_NO_ERROR;
    const Key * config_keys[] = { &kConfigKey_ServiceConfig,      &kConfigKey_PairedAccountId, &kConfigKey_ServiceId,
                                  &kConfigKey_LastUsedEpochKeyId, &kConfigKey_FailSafeArmed,   &kConfigKey_WiFiStationSecType,
                                  &kConfigKey_WiFiSSID,           &kConfigKey_WiFiPassword,    &kConfigKey_WiFiSecurity,
                                  &kConfigKey_WiFiMode,           &kConfigKey_SoftwareVersion };

    for (uint32_t i = 0; i < (sizeof(config_keys) / sizeof(config_keys[0])); i++)
    {
        err = ClearConfigValue(*config_keys[i]);
        // Something unexpected happened
        if (err != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND && err != CHIP_NO_ERROR)
        {
            return err;
        }
    }

    // Erase all key-values including fabric info.
    err = PersistedStorage::KeyValueStoreMgrImpl().Erase();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Clear Key-Value Storage failed");
    }

    return CHIP_NO_ERROR;
}

void PSOC6Config::RunConfigUnitTest() {}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
