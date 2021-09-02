/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *          Utilities for interacting with the the P6 key-value store.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/KeyValueStoreManager.h>

#include <platform/P6/P6Config.h>

#include <lib/core/CHIPEncoding.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/P6/P6Utils.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

// *** CAUTION ***: Changing the names or namespaces of these values will *break* existing devices.

// Namespaces used to store device configuration information.
const char P6Config::kConfigNamespace_ChipFactory[]  = "chip-factory";
const char P6Config::kConfigNamespace_ChipConfig[]   = "chip-config";
const char P6Config::kConfigNamespace_ChipCounters[] = "chip-counters";

// Keys stored in the chip-factory namespace
const P6Config::Key P6Config::kConfigKey_SerialNum           = { kConfigNamespace_ChipFactory, "serial-num" };
const P6Config::Key P6Config::kConfigKey_MfrDeviceId         = { kConfigNamespace_ChipFactory, "device-id" };
const P6Config::Key P6Config::kConfigKey_MfrDeviceCert       = { kConfigNamespace_ChipFactory, "device-cert" };
const P6Config::Key P6Config::kConfigKey_MfrDeviceICACerts   = { kConfigNamespace_ChipFactory, "device-ca-certs" };
const P6Config::Key P6Config::kConfigKey_MfrDevicePrivateKey = { kConfigNamespace_ChipFactory, "device-key" };
const P6Config::Key P6Config::kConfigKey_ProductRevision     = { kConfigNamespace_ChipFactory, "product-rev" };
const P6Config::Key P6Config::kConfigKey_ManufacturingDate   = { kConfigNamespace_ChipFactory, "mfg-date" };
const P6Config::Key P6Config::kConfigKey_SetupPinCode        = { kConfigNamespace_ChipFactory, "pin-code" };
const P6Config::Key P6Config::kConfigKey_SetupDiscriminator  = { kConfigNamespace_ChipFactory, "discriminator" };
const P6Config::Key P6Config::kConfigKey_RegulatoryLocation  = { kConfigNamespace_ChipConfig, "regulatory-location" };
const P6Config::Key P6Config::kConfigKey_CountryCode         = { kConfigNamespace_ChipConfig, "country-code" };
const P6Config::Key P6Config::kConfigKey_Breadcrumb          = { kConfigNamespace_ChipConfig, "breadcrumb" };

// Keys stored in the chip-config namespace
const P6Config::Key P6Config::kConfigKey_FabricId                    = { kConfigNamespace_ChipConfig, "fabric-id" };
const P6Config::Key P6Config::kConfigKey_ServiceConfig               = { kConfigNamespace_ChipConfig, "service-config" };
const P6Config::Key P6Config::kConfigKey_PairedAccountId             = { kConfigNamespace_ChipConfig, "account-id" };
const P6Config::Key P6Config::kConfigKey_ServiceId                   = { kConfigNamespace_ChipConfig, "service-id" };
const P6Config::Key P6Config::kConfigKey_GroupKeyIndex               = { kConfigNamespace_ChipConfig, "group-key-index" };
const P6Config::Key P6Config::kConfigKey_LastUsedEpochKeyId          = { kConfigNamespace_ChipConfig, "last-ek-id" };
const P6Config::Key P6Config::kConfigKey_FailSafeArmed               = { kConfigNamespace_ChipConfig, "fail-safe-armed" };
const P6Config::Key P6Config::kConfigKey_WiFiStationSecType          = { kConfigNamespace_ChipConfig, "sta-sec-type" };
const P6Config::Key P6Config::kConfigKey_OperationalDeviceId         = { kConfigNamespace_ChipConfig, "op-device-id" };
const P6Config::Key P6Config::kConfigKey_OperationalDeviceCert       = { kConfigNamespace_ChipConfig, "op-device-cert" };
const P6Config::Key P6Config::kConfigKey_OperationalDeviceICACerts   = { kConfigNamespace_ChipConfig, "op-device-ca-certs" };
const P6Config::Key P6Config::kConfigKey_OperationalDevicePrivateKey = { kConfigNamespace_ChipConfig, "op-device-key" };

// Prefix used for keys that contain Chip group encryption keys.
const char P6Config::kGroupKeyNamePrefix[] = "gk-";

CHIP_ERROR P6Config::ReadConfigValue(Key key, bool & val)
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

CHIP_ERROR P6Config::ReadConfigValue(Key key, uint32_t & val)
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

CHIP_ERROR P6Config::ReadConfigValue(Key key, uint64_t & val)
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

CHIP_ERROR P6Config::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
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

CHIP_ERROR P6Config::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
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

CHIP_ERROR P6Config::WriteConfigValue(Key key, bool val)
{
    char key_str[MTB_KVSTORE_MAX_KEY_SIZE] = { 0 };
    key.to_str(key_str, MTB_KVSTORE_MAX_KEY_SIZE);
    return PersistedStorage::KeyValueStoreMgr().Put(key_str, static_cast<void *>(&val), sizeof(bool));
}

CHIP_ERROR P6Config::WriteConfigValue(Key key, uint32_t val)
{
    char key_str[MTB_KVSTORE_MAX_KEY_SIZE] = { 0 };
    key.to_str(key_str, MTB_KVSTORE_MAX_KEY_SIZE);
    return PersistedStorage::KeyValueStoreMgr().Put(key_str, static_cast<void *>(&val), 4);
}

CHIP_ERROR P6Config::WriteConfigValue(Key key, uint64_t val)
{
    char key_str[MTB_KVSTORE_MAX_KEY_SIZE] = { 0 };
    key.to_str(key_str, MTB_KVSTORE_MAX_KEY_SIZE);
    return PersistedStorage::KeyValueStoreMgr().Put(key_str, static_cast<void *>(&val), 8);
}

CHIP_ERROR P6Config::WriteConfigValueStr(Key key, const char * str)
{
    size_t size                            = strlen(str) + 1;
    char key_str[MTB_KVSTORE_MAX_KEY_SIZE] = { 0 };
    key.to_str(key_str, MTB_KVSTORE_MAX_KEY_SIZE);
    return PersistedStorage::KeyValueStoreMgr().Put(key_str, str, size);
}

CHIP_ERROR P6Config::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    char key_str[MTB_KVSTORE_MAX_KEY_SIZE] = { 0 };
    key.to_str(key_str, MTB_KVSTORE_MAX_KEY_SIZE);
    return PersistedStorage::KeyValueStoreMgr().Put(key_str, str, strLen);
}
CHIP_ERROR P6Config::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    char key_str[MTB_KVSTORE_MAX_KEY_SIZE] = { 0 };
    key.to_str(key_str, MTB_KVSTORE_MAX_KEY_SIZE);
    return PersistedStorage::KeyValueStoreMgr().Put(key_str, static_cast<void *>(&data), dataLen);
}

CHIP_ERROR P6Config::ClearConfigValue(Key key)
{
    char key_str[MTB_KVSTORE_MAX_KEY_SIZE] = { 0 };
    key.to_str(key_str, MTB_KVSTORE_MAX_KEY_SIZE);
    return PersistedStorage::KeyValueStoreMgr().Delete(key_str);
}

bool P6Config::ConfigValueExists(Key key)
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
CHIP_ERROR P6Config::FactoryResetConfig(void)
{
    const Key * config_keys[] = { &kConfigKey_FabricId,
                                  &kConfigKey_ServiceConfig,
                                  &kConfigKey_PairedAccountId,
                                  &kConfigKey_ServiceId,
                                  &kConfigKey_GroupKeyIndex,
                                  &kConfigKey_LastUsedEpochKeyId,
                                  &kConfigKey_FailSafeArmed,
                                  &kConfigKey_WiFiStationSecType,
                                  &kConfigKey_OperationalDeviceId,
                                  &kConfigKey_OperationalDeviceCert,
                                  &kConfigKey_OperationalDeviceICACerts,
                                  &kConfigKey_OperationalDevicePrivateKey };

    for (uint32_t i = 0; i < (sizeof(config_keys) / sizeof(config_keys[0])); i++)
    {
        CHIP_ERROR err = ClearConfigValue(*config_keys[i]);
        // Something unexpected happened
        if (err != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND && err != CHIP_NO_ERROR)
        {
            return err;
        }
    }

    return CHIP_NO_ERROR;
}

void P6Config::RunConfigUnitTest() {}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
