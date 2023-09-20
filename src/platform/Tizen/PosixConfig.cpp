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
 *          Utilities for interacting with multiple file partitions and maps
 *          key-value config calls to the correct partition.
 */

#include "PosixConfig.h"

#include <lib/support/CodeUtils.h>
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
const PosixConfig::Key PosixConfig::kConfigKey_MfrDeviceId           = { kConfigNamespace_ChipFactory, "device-id" };
const PosixConfig::Key PosixConfig::kConfigKey_MfrDeviceCert         = { kConfigNamespace_ChipFactory, "device-cert" };
const PosixConfig::Key PosixConfig::kConfigKey_MfrDeviceICACerts     = { kConfigNamespace_ChipFactory, "device-ca-certs" };
const PosixConfig::Key PosixConfig::kConfigKey_MfrDevicePrivateKey   = { kConfigNamespace_ChipFactory, "device-key" };
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
const PosixConfig::Key PosixConfig::kConfigKey_ServiceConfig      = { kConfigNamespace_ChipConfig, "service-config" };
const PosixConfig::Key PosixConfig::kConfigKey_PairedAccountId    = { kConfigNamespace_ChipConfig, "account-id" };
const PosixConfig::Key PosixConfig::kConfigKey_ServiceId          = { kConfigNamespace_ChipConfig, "service-id" };
const PosixConfig::Key PosixConfig::kConfigKey_LastUsedEpochKeyId = { kConfigNamespace_ChipConfig, "last-ek-id" };
const PosixConfig::Key PosixConfig::kConfigKey_FailSafeArmed      = { kConfigNamespace_ChipConfig, "fail-safe-armed" };
const PosixConfig::Key PosixConfig::kConfigKey_RegulatoryLocation = { kConfigNamespace_ChipConfig, "regulatory-location" };
const PosixConfig::Key PosixConfig::kConfigKey_CountryCode        = { kConfigNamespace_ChipConfig, "country-code" };
const PosixConfig::Key PosixConfig::kConfigKey_UniqueId           = { kConfigNamespace_ChipConfig, "unique-id" };

// Keys stored in the Chip-counters namespace
const PosixConfig::Key PosixConfig::kCounterKey_TotalOperationalHours = { kConfigNamespace_ChipCounters,
                                                                          "total-operational-hours" };
const PosixConfig::Key PosixConfig::kCounterKey_BootReason            = { kConfigNamespace_ChipCounters, "boot-reason" };

CHIP_ERROR PosixConfig::Init()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR PosixConfig::ReadConfigValue(Key key, bool & val)
{
    return PersistedStorage::KeyValueStoreMgr().Get(key.Name, &val);
}

CHIP_ERROR PosixConfig::ReadConfigValue(Key key, uint16_t & val)

{
    return PersistedStorage::KeyValueStoreMgr().Get(key.Name, &val);
}

CHIP_ERROR PosixConfig::ReadConfigValue(Key key, uint32_t & val)
{
    return PersistedStorage::KeyValueStoreMgr().Get(key.Name, &val);
}

CHIP_ERROR PosixConfig::ReadConfigValue(Key key, uint64_t & val)
{
    return PersistedStorage::KeyValueStoreMgr().Get(key.Name, &val);
}

CHIP_ERROR PosixConfig::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    VerifyOrReturnError(buf != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    auto err = PersistedStorage::KeyValueStoreMgr().Get(key.Name, buf, bufSize, &outLen);
    VerifyOrReturnError(err == CHIP_NO_ERROR, err);

    // We are storing string values in the config store without
    // the null terminator, so we need to add it here.
    VerifyOrReturnError(bufSize >= outLen + 1, CHIP_ERROR_BUFFER_TOO_SMALL);
    buf[outLen] = '\0';

    return CHIP_NO_ERROR;
}

CHIP_ERROR PosixConfig::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    VerifyOrReturnError(buf != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return PersistedStorage::KeyValueStoreMgr().Get(key.Name, buf, bufSize, &outLen);
}

CHIP_ERROR PosixConfig::WriteConfigValue(Key key, bool val)
{
    return PersistedStorage::KeyValueStoreMgr().Put(key.Name, val);
}

CHIP_ERROR PosixConfig::WriteConfigValue(Key key, uint16_t val)
{
    return PersistedStorage::KeyValueStoreMgr().Put(key.Name, val);
}

CHIP_ERROR PosixConfig::WriteConfigValue(Key key, uint32_t val)
{
    return PersistedStorage::KeyValueStoreMgr().Put(key.Name, val);
}

CHIP_ERROR PosixConfig::WriteConfigValue(Key key, uint64_t val)
{
    return PersistedStorage::KeyValueStoreMgr().Put(key.Name, val);
}

CHIP_ERROR PosixConfig::WriteConfigValueStr(Key key, const char * str)
{
    VerifyOrReturnError(str != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return PersistedStorage::KeyValueStoreMgr().Put(key.Name, str, strlen(str));
}

CHIP_ERROR PosixConfig::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    VerifyOrReturnError(str != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return PersistedStorage::KeyValueStoreMgr().Put(key.Name, str, strLen);
}

CHIP_ERROR PosixConfig::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    VerifyOrReturnError(data != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    return PersistedStorage::KeyValueStoreMgr().Put(key.Name, data, dataLen);
}

CHIP_ERROR PosixConfig::ClearConfigValue(Key key)
{
    return PersistedStorage::KeyValueStoreMgr().Delete(key.Name);
}

bool PosixConfig::ConfigValueExists(Key key)
{
    CHIP_ERROR err;
    err = PersistedStorage::KeyValueStoreMgrImpl()._Check(key.Name);
    return (err == CHIP_NO_ERROR) ? true : false;
}

CHIP_ERROR PosixConfig::EnsureNamespace(const char * ns)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR PosixConfig::ClearNamespace(const char * ns)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR PosixConfig::FactoryResetConfig()
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

void PosixConfig::RunConfigUnitTest() {}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
