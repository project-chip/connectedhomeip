/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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
 *          Utilities for interacting with multiple file partitions and maps
 *          key-value config calls to the correct partition.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/internal/testing/ConfigUnitTest.h>

#include <lib/core/CHIPEncoding.h>
#include <lib/support/CodeUtils.h>
#include <platform/Darwin/PosixConfig.h>

#include <platform/KeyValueStoreManager.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

// *** CAUTION ***: Changing the names or namespaces of these values will *break* existing devices.

// NVS namespaces used to store device configuration information.
const char PosixConfig::kConfigNamespace_ChipFactory[]  = "chip-factory";
const char PosixConfig::kConfigNamespace_ChipConfig[]   = "chip-config";
const char PosixConfig::kConfigNamespace_ChipCounters[] = "chip-counters";

// Keys stored in the Chip-factory namespace
const PosixConfig::Key PosixConfig::kConfigKey_SerialNum             = { kConfigNamespace_ChipFactory, "serial-num" };
const PosixConfig::Key PosixConfig::kConfigKey_HardwareVersion       = { kConfigNamespace_ChipFactory, "hardware-ver" };
const PosixConfig::Key PosixConfig::kConfigKey_ManufacturingDate     = { kConfigNamespace_ChipFactory, "mfg-date" };
const PosixConfig::Key PosixConfig::kConfigKey_SetupPinCode          = { kConfigNamespace_ChipFactory, "pin-code" };
const PosixConfig::Key PosixConfig::kConfigKey_SetupDiscriminator    = { kConfigNamespace_ChipFactory, "discriminator" };
const PosixConfig::Key PosixConfig::kConfigKey_Spake2pIterationCount = { kConfigNamespace_ChipFactory, "iteration-count" };
const PosixConfig::Key PosixConfig::kConfigKey_Spake2pSalt           = { kConfigNamespace_ChipFactory, "salt" };
const PosixConfig::Key PosixConfig::kConfigKey_Spake2pVerifier       = { kConfigNamespace_ChipFactory, "verifier" };
const PosixConfig::Key PosixConfig::kConfigKey_VendorId              = { kConfigNamespace_ChipFactory, "vendor-id" };
const PosixConfig::Key PosixConfig::kConfigKey_ProductId             = { kConfigNamespace_ChipFactory, "product-id" };

// Keys stored in the Chip-config namespace
const PosixConfig::Key PosixConfig::kConfigKey_FailSafeArmed      = { kConfigNamespace_ChipConfig, "fail-safe-armed" };
const PosixConfig::Key PosixConfig::kConfigKey_RegulatoryLocation = { kConfigNamespace_ChipConfig, "regulatory-location" };
const PosixConfig::Key PosixConfig::kConfigKey_CountryCode        = { kConfigNamespace_ChipConfig, "country-code" };
const PosixConfig::Key PosixConfig::kConfigKey_LocationCapability = { kConfigNamespace_ChipConfig, "location-capability" };

// Keys stored in the Chip-counters namespace
const PosixConfig::Key PosixConfig::kCounterKey_RebootCount           = { kConfigNamespace_ChipCounters, "reboot-count" };
const PosixConfig::Key PosixConfig::kCounterKey_BootReason            = { kConfigNamespace_ChipCounters, "boot-reason" };
const PosixConfig::Key PosixConfig::kCounterKey_TotalOperationalHours = { kConfigNamespace_ChipCounters,
                                                                          "total-operational-hours" };
const PosixConfig::Key PosixConfig::kConfigKey_UniqueId               = { kConfigNamespace_ChipConfig, "unique-id" };

#if !CHIP_DISABLE_PLATFORM_KVS
CHIP_ERROR PosixConfig::Init()
{
    return PersistedStorage::KeyValueStoreMgrImpl().Init(CHIP_CONFIG_KVS_PATH);
}

CHIP_ERROR PosixConfig::ReadConfigValue(Key key, bool & val)
{
    size_t outLen = 0;
    return ReadConfigValueBin(key, reinterpret_cast<uint8_t *>(&val), sizeof(val), outLen);
}

CHIP_ERROR PosixConfig::ReadConfigValue(Key key, uint16_t & val)
{
    size_t outLen = 0;
    return ReadConfigValueBin(key, reinterpret_cast<uint8_t *>(&val), sizeof(val), outLen);
}

CHIP_ERROR PosixConfig::ReadConfigValue(Key key, uint32_t & val)
{
    size_t outLen = 0;
    return ReadConfigValueBin(key, reinterpret_cast<uint8_t *>(&val), sizeof(val), outLen);
}

CHIP_ERROR PosixConfig::ReadConfigValue(Key key, uint64_t & val)
{
    size_t outLen = 0;
    return ReadConfigValueBin(key, reinterpret_cast<uint8_t *>(&val), sizeof(val), outLen);
}

CHIP_ERROR PosixConfig::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    return ReadConfigValueBin(key, reinterpret_cast<uint8_t *>(buf), bufSize, outLen);
}

CHIP_ERROR PosixConfig::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    CHIP_ERROR err = PersistedStorage::KeyValueStoreMgr().Get(key.Name, buf, bufSize, &outLen);
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    return err;
}

CHIP_ERROR PosixConfig::WriteConfigValue(Key key, bool val)
{
    return WriteConfigValueBin(key, reinterpret_cast<uint8_t *>(&val), sizeof(val));
}

CHIP_ERROR PosixConfig::WriteConfigValue(Key key, uint16_t val)
{
    return WriteConfigValueBin(key, reinterpret_cast<uint8_t *>(&val), sizeof(val));
}

CHIP_ERROR PosixConfig::WriteConfigValue(Key key, uint32_t val)
{
    return WriteConfigValueBin(key, reinterpret_cast<uint8_t *>(&val), sizeof(val));
}

CHIP_ERROR PosixConfig::WriteConfigValue(Key key, uint64_t val)
{
    return WriteConfigValueBin(key, reinterpret_cast<uint8_t *>(&val), sizeof(val));
}

CHIP_ERROR PosixConfig::WriteConfigValueStr(Key key, const char * str)
{
    return WriteConfigValueBin(key, reinterpret_cast<const uint8_t *>(str), strlen(str));
}

CHIP_ERROR PosixConfig::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    return WriteConfigValueBin(key, reinterpret_cast<const uint8_t *>(str), strLen);
}

CHIP_ERROR PosixConfig::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    return PersistedStorage::KeyValueStoreMgr().Put(key.Name, data, dataLen);
}

CHIP_ERROR PosixConfig::ClearConfigValue(Key key)
{
    return PersistedStorage::KeyValueStoreMgr().Delete(key.Name);
}

bool PosixConfig::ConfigValueExists(Key key)
{
    size_t outLen;
    CHIP_ERROR err = ReadConfigValueBin(key, nullptr, 0, outLen);
    if (err == CHIP_NO_ERROR || err == CHIP_ERROR_BUFFER_TOO_SMALL)
    {
        return true;
    }

    return false;
}

CHIP_ERROR PosixConfig::EnsureNamespace(const char * ns)
{
    CHIP_ERROR err = CHIP_ERROR_PERSISTED_STORAGE_FAILED;
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR PosixConfig::ClearNamespace(const char * ns)
{
    CHIP_ERROR err = CHIP_ERROR_PERSISTED_STORAGE_FAILED;
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR PosixConfig::FactoryResetConfig(void)
{
    CHIP_ERROR err = CHIP_ERROR_PERSISTED_STORAGE_FAILED;
    SuccessOrExit(err);

exit:
    return err;
}

void PosixConfig::RunConfigUnitTest() {}

#endif // CHIP_DISABLE_PLATFORM_KVS

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
