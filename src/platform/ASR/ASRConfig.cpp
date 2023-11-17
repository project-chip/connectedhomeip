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

/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/KeyValueStoreManager.h>

#include <platform/ASR/ASRConfig.h>

#include <lib/core/CHIPEncoding.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/ASR/ASRUtils.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

// *** CAUTION ***: Changing the names or namespaces of these values will *break* existing devices.

// Namespaces used to store device configuration information.
const char ASRConfig::kConfigNamespace_ChipFactory[]  = "chip-factory";
const char ASRConfig::kConfigNamespace_ChipConfig[]   = "chip-config";
const char ASRConfig::kConfigNamespace_ChipCounters[] = "chip-counters";

// Keys stored in the chip-factory namespace
const ASRConfig::Key ASRConfig::kConfigKey_SerialNum           = { kConfigNamespace_ChipFactory, "serial-num" };
const ASRConfig::Key ASRConfig::kConfigKey_MfrDeviceId         = { kConfigNamespace_ChipFactory, "device-id" };
const ASRConfig::Key ASRConfig::kConfigKey_MfrDeviceCert       = { kConfigNamespace_ChipFactory, "device-cert" };
const ASRConfig::Key ASRConfig::kConfigKey_MfrDeviceICACerts   = { kConfigNamespace_ChipFactory, "device-ca-certs" };
const ASRConfig::Key ASRConfig::kConfigKey_MfrDevicePrivateKey = { kConfigNamespace_ChipFactory, "device-key" };
const ASRConfig::Key ASRConfig::kConfigKey_SoftwareVersion     = { kConfigNamespace_ChipFactory, "software-ver" };
const ASRConfig::Key ASRConfig::kConfigKey_HardwareVersion     = { kConfigNamespace_ChipFactory, "hardware-ver" };
const ASRConfig::Key ASRConfig::kConfigKey_ManufacturingDate   = { kConfigNamespace_ChipFactory, "mfg-date" };
#if !CONFIG_ENABLE_ASR_FACTORY_DATA_PROVIDER
const ASRConfig::Key ASRConfig::kConfigKey_SetupPinCode          = { kConfigNamespace_ChipFactory, "pin-code" };
const ASRConfig::Key ASRConfig::kConfigKey_SetupDiscriminator    = { kConfigNamespace_ChipFactory, "discriminator" };
const ASRConfig::Key ASRConfig::kConfigKey_Spake2pIterationCount = { kConfigNamespace_ChipFactory, "iteration-count" };
const ASRConfig::Key ASRConfig::kConfigKey_Spake2pSalt           = { kConfigNamespace_ChipFactory, "salt" };
const ASRConfig::Key ASRConfig::kConfigKey_Spake2pVerifier       = { kConfigNamespace_ChipFactory, "verifier" };
const ASRConfig::Key ASRConfig::kConfigKey_DACCert               = { kConfigNamespace_ChipFactory, "dac-cert" };
const ASRConfig::Key ASRConfig::kConfigKey_DACPrivateKey         = { kConfigNamespace_ChipFactory, "dac-key" };
const ASRConfig::Key ASRConfig::kConfigKey_DACPublicKey          = { kConfigNamespace_ChipFactory, "dac-pub-key" };
const ASRConfig::Key ASRConfig::kConfigKey_PAICert               = { kConfigNamespace_ChipFactory, "pai-cert" };
const ASRConfig::Key ASRConfig::kConfigKey_CertDeclaration       = { kConfigNamespace_ChipFactory, "cert-dclrn" };
#endif

// Keys stored in the chip-config namespace
const ASRConfig::Key ASRConfig::kConfigKey_ServiceConfig      = { kConfigNamespace_ChipConfig, "service-config" };
const ASRConfig::Key ASRConfig::kConfigKey_PairedAccountId    = { kConfigNamespace_ChipConfig, "account-id" };
const ASRConfig::Key ASRConfig::kConfigKey_ServiceId          = { kConfigNamespace_ChipConfig, "service-id" };
const ASRConfig::Key ASRConfig::kConfigKey_LastUsedEpochKeyId = { kConfigNamespace_ChipConfig, "last-ek-id" };
const ASRConfig::Key ASRConfig::kConfigKey_FailSafeArmed      = { kConfigNamespace_ChipConfig, "fail-safe-armed" };
const ASRConfig::Key ASRConfig::kConfigKey_WiFiStationSecType = { kConfigNamespace_ChipConfig, "sta-sec-type" };
const ASRConfig::Key ASRConfig::kConfigKey_RegulatoryLocation = { kConfigNamespace_ChipConfig, "regulatory-location" };
const ASRConfig::Key ASRConfig::kConfigKey_CountryCode        = { kConfigNamespace_ChipConfig, "country-code" };
const ASRConfig::Key ASRConfig::kConfigKey_WiFiSSID           = { kConfigNamespace_ChipConfig, "wifi-ssid" };
const ASRConfig::Key ASRConfig::kConfigKey_WiFiPassword       = { kConfigNamespace_ChipConfig, "wifi-password" };
const ASRConfig::Key ASRConfig::kConfigKey_WiFiSecurity       = { kConfigNamespace_ChipConfig, "wifi-security" };
const ASRConfig::Key ASRConfig::kConfigKey_WiFiMode           = { kConfigNamespace_ChipConfig, "wifimode" };
const ASRConfig::Key ASRConfig::kConfigKey_UniqueId           = { kConfigNamespace_ChipConfig, "unique-id" };

// Keys stored in the Chip-counters namespace
const ASRConfig::Key ASRConfig::kCounterKey_RebootCount           = { kConfigNamespace_ChipCounters, "reboot-count" };
const ASRConfig::Key ASRConfig::kCounterKey_UpTime                = { kConfigNamespace_ChipCounters, "up-time" };
const ASRConfig::Key ASRConfig::kCounterKey_TotalOperationalHours = { kConfigNamespace_ChipCounters, "total-hours" };

#define _KVSTORE_MAX_KEY_SIZE 64

CHIP_ERROR ASRConfig::ReadConfigValue(Key key, bool & val)
{
    bool in;
    char key_str[_KVSTORE_MAX_KEY_SIZE] = { 0 };
    key.to_str(key_str, _KVSTORE_MAX_KEY_SIZE);
    CHIP_ERROR err = PersistedStorage::KeyValueStoreMgr().Get(key_str, static_cast<void *>(&in), sizeof(bool));
    val            = in;
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    return err;
}

CHIP_ERROR ASRConfig::ReadConfigValue(Key key, uint32_t & val)
{
    uint32_t in;
    char key_str[_KVSTORE_MAX_KEY_SIZE] = { 0 };
    key.to_str(key_str, _KVSTORE_MAX_KEY_SIZE);
    CHIP_ERROR err = PersistedStorage::KeyValueStoreMgr().Get(key_str, static_cast<void *>(&in), 4);
    val            = in;
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    return err;
}

CHIP_ERROR ASRConfig::ReadConfigValue(Key key, uint64_t & val)
{
    uint64_t in;
    char key_str[_KVSTORE_MAX_KEY_SIZE] = { 0 };
    key.to_str(key_str, _KVSTORE_MAX_KEY_SIZE);
    CHIP_ERROR err = PersistedStorage::KeyValueStoreMgr().Get(key_str, static_cast<void *>(&in), 8);
    val            = in;
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    return err;
}

CHIP_ERROR ASRConfig::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    char key_str[_KVSTORE_MAX_KEY_SIZE] = { 0 };
    key.to_str(key_str, _KVSTORE_MAX_KEY_SIZE);
    CHIP_ERROR err = PersistedStorage::KeyValueStoreMgr().Get(key_str, buf, bufSize, &outLen);
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    return err;
}

CHIP_ERROR ASRConfig::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    char key_str[_KVSTORE_MAX_KEY_SIZE] = { 0 };
    key.to_str(key_str, _KVSTORE_MAX_KEY_SIZE);
    CHIP_ERROR err = PersistedStorage::KeyValueStoreMgr().Get(key_str, buf, bufSize, &outLen);
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    return err;
}

CHIP_ERROR ASRConfig::WriteConfigValue(Key key, bool val)
{
    char key_str[_KVSTORE_MAX_KEY_SIZE] = { 0 };
    key.to_str(key_str, _KVSTORE_MAX_KEY_SIZE);
    return PersistedStorage::KeyValueStoreMgr().Put(key_str, static_cast<void *>(&val), sizeof(bool));
}

CHIP_ERROR ASRConfig::WriteConfigValue(Key key, uint32_t val)
{
    char key_str[_KVSTORE_MAX_KEY_SIZE] = { 0 };
    key.to_str(key_str, _KVSTORE_MAX_KEY_SIZE);
    return PersistedStorage::KeyValueStoreMgr().Put(key_str, static_cast<void *>(&val), 4);
}

CHIP_ERROR ASRConfig::WriteConfigValue(Key key, uint64_t val)
{
    char key_str[_KVSTORE_MAX_KEY_SIZE] = { 0 };
    key.to_str(key_str, _KVSTORE_MAX_KEY_SIZE);
    return PersistedStorage::KeyValueStoreMgr().Put(key_str, static_cast<void *>(&val), 8);
}

CHIP_ERROR ASRConfig::WriteConfigValueStr(Key key, const char * str)
{
    size_t size                         = strlen(str) + 1;
    char key_str[_KVSTORE_MAX_KEY_SIZE] = { 0 };
    key.to_str(key_str, _KVSTORE_MAX_KEY_SIZE);
    return PersistedStorage::KeyValueStoreMgr().Put(key_str, str, size);
}

CHIP_ERROR ASRConfig::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    char key_str[_KVSTORE_MAX_KEY_SIZE] = { 0 };
    key.to_str(key_str, _KVSTORE_MAX_KEY_SIZE);
    return PersistedStorage::KeyValueStoreMgr().Put(key_str, str, strLen);
}

CHIP_ERROR ASRConfig::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    char key_str[_KVSTORE_MAX_KEY_SIZE] = { 0 };
    key.to_str(key_str, _KVSTORE_MAX_KEY_SIZE);
    return PersistedStorage::KeyValueStoreMgr().Put(key_str, static_cast<void *>(&data), dataLen);
}

CHIP_ERROR ASRConfig::ClearConfigValue(Key key)
{
    char key_str[_KVSTORE_MAX_KEY_SIZE] = { 0 };
    key.to_str(key_str, _KVSTORE_MAX_KEY_SIZE);
    return PersistedStorage::KeyValueStoreMgr().Delete(key_str);
}

bool ASRConfig::ConfigValueExists(Key key)
{
    char key_str[_KVSTORE_MAX_KEY_SIZE] = { 0 };
    char buf[4];
    size_t outLen;
    key.to_str(key_str, _KVSTORE_MAX_KEY_SIZE);
    if (PersistedStorage::KeyValueStoreMgr().Get(key_str, buf, 4, &outLen) == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        return false;
    }

    return true;
}

#if CONFIG_ENABLE_ASR_FACTORY_DATA_PROVIDER
CHIP_ERROR ASRConfig::ReadFactoryConfigValue(asr_matter_partition_t matter_partition, uint8_t * buf, size_t bufSize,
                                             size_t & outLen)
{
    factory_error_t ret = asr_factory_config_read(matter_partition, buf, (uint32_t) bufSize, (uint32_t *) &outLen);

    if (ret != FACTORY_NO_ERROR)
    {
        ChipLogProgress(DeviceLayer, "asr_factory_config_read: %d failed, err = %d\n", matter_partition, ret);
        outLen = 0;
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ASRConfig::ReadFactoryConfigValue(asr_matter_partition_t matter_partition, uint32_t & val)
{
    uint8_t buf[4];
    size_t outlen       = 0;
    factory_error_t ret = asr_factory_config_read(matter_partition, buf, sizeof(uint32_t), (uint32_t *) &outlen);

    if (outlen > sizeof(uint32_t))
        return CHIP_ERROR_BUFFER_TOO_SMALL;

    if (ret != FACTORY_NO_ERROR)
    {
        ChipLogProgress(DeviceLayer, "asr_factory_config_read: %d failed, err = %d\n", matter_partition, ret);
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    else
    {
        val = *((uint32_t *) buf);
        return CHIP_NO_ERROR;
    }
}
#endif
// Clear out keys in config namespace
CHIP_ERROR ASRConfig::FactoryResetConfig(void)
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

void ASRConfig::RunConfigUnitTest() {}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
