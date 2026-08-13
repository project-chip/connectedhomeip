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

#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Compiler.h>
#include <platform/CHIPDeviceError.h>
#include <platform/KeyValueStoreManager.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

// *** CAUTION ***: Changing the names or namespaces of these values will *break* existing devices.

// NVS namespaces used to store device configuration information.
CHIP_CPP20(constinit) const char PosixConfig::kConfigNamespace_ChipFactory[]  = "chip-factory";
CHIP_CPP20(constinit) const char PosixConfig::kConfigNamespace_ChipConfig[]   = "chip-config";
CHIP_CPP20(constinit) const char PosixConfig::kConfigNamespace_ChipCounters[] = "chip-counters";

// Keys stored in the Chip-factory namespace
CHIP_CPP20(constinit)
const PosixConfig::Key PosixConfig::kConfigKey_SerialNum = MakeConfigKey(kConfigNamespace_ChipFactory, "serial-num");
CHIP_CPP20(constinit)
const PosixConfig::Key PosixConfig::kConfigKey_MfrDeviceId = MakeConfigKey(kConfigNamespace_ChipFactory, "device-id");
CHIP_CPP20(constinit)
const PosixConfig::Key PosixConfig::kConfigKey_MfrDeviceCert = MakeConfigKey(kConfigNamespace_ChipFactory, "device-cert");
CHIP_CPP20(constinit)
const PosixConfig::Key PosixConfig::kConfigKey_MfrDeviceICACerts = MakeConfigKey(kConfigNamespace_ChipFactory, "device-ca-certs");
CHIP_CPP20(constinit)
const PosixConfig::Key PosixConfig::kConfigKey_MfrDevicePrivateKey = MakeConfigKey(kConfigNamespace_ChipFactory, "device-key");
CHIP_CPP20(constinit)
const PosixConfig::Key PosixConfig::kConfigKey_HardwareVersion = MakeConfigKey(kConfigNamespace_ChipFactory, "hardware-ver");
CHIP_CPP20(constinit)
const PosixConfig::Key PosixConfig::kConfigKey_ManufacturingDate = MakeConfigKey(kConfigNamespace_ChipFactory, "mfg-date");
CHIP_CPP20(constinit)
const PosixConfig::Key PosixConfig::kConfigKey_SetupPinCode = MakeConfigKey(kConfigNamespace_ChipFactory, "pin-code");
CHIP_CPP20(constinit)
const PosixConfig::Key PosixConfig::kConfigKey_SetupDiscriminator = MakeConfigKey(kConfigNamespace_ChipFactory, "discriminator");
CHIP_CPP20(constinit)
const PosixConfig::Key PosixConfig::kConfigKey_Spake2pIterationCount =
    MakeConfigKey(kConfigNamespace_ChipFactory, "iteration-count");
CHIP_CPP20(constinit)
const PosixConfig::Key PosixConfig::kConfigKey_Spake2pSalt = MakeConfigKey(kConfigNamespace_ChipFactory, "salt");
CHIP_CPP20(constinit)
const PosixConfig::Key PosixConfig::kConfigKey_Spake2pVerifier = MakeConfigKey(kConfigNamespace_ChipFactory, "verifier");
CHIP_CPP20(constinit)
const PosixConfig::Key PosixConfig::kConfigKey_VendorId = MakeConfigKey(kConfigNamespace_ChipFactory, "vendor-id");
CHIP_CPP20(constinit)
const PosixConfig::Key PosixConfig::kConfigKey_ProductId = MakeConfigKey(kConfigNamespace_ChipFactory, "product-id");

// Keys stored in the Chip-config namespace
CHIP_CPP20(constinit)
const PosixConfig::Key PosixConfig::kConfigKey_ServiceConfig = MakeConfigKey(kConfigNamespace_ChipConfig, "service-config");
CHIP_CPP20(constinit)
const PosixConfig::Key PosixConfig::kConfigKey_PairedAccountId = MakeConfigKey(kConfigNamespace_ChipConfig, "account-id");
CHIP_CPP20(constinit)
const PosixConfig::Key PosixConfig::kConfigKey_ServiceId = MakeConfigKey(kConfigNamespace_ChipConfig, "service-id");
CHIP_CPP20(constinit)
const PosixConfig::Key PosixConfig::kConfigKey_LastUsedEpochKeyId = MakeConfigKey(kConfigNamespace_ChipConfig, "last-ek-id");
CHIP_CPP20(constinit)
const PosixConfig::Key PosixConfig::kConfigKey_FailSafeArmed = MakeConfigKey(kConfigNamespace_ChipConfig, "fail-safe-armed");
CHIP_CPP20(constinit)
const PosixConfig::Key PosixConfig::kConfigKey_RegulatoryLocation =
    MakeConfigKey(kConfigNamespace_ChipConfig, "regulatory-location");
CHIP_CPP20(constinit)
const PosixConfig::Key PosixConfig::kConfigKey_CountryCode = MakeConfigKey(kConfigNamespace_ChipConfig, "country-code");
CHIP_CPP20(constinit)
const PosixConfig::Key PosixConfig::kConfigKey_UniqueId = MakeConfigKey(kConfigNamespace_ChipConfig, "unique-id");

// Keys stored in the Chip-counters namespace
CHIP_CPP20(constinit)
const PosixConfig::Key PosixConfig::kCounterKey_TotalOperationalHours =
    MakeConfigKey(kConfigNamespace_ChipCounters, "total-operational-hours");
CHIP_CPP20(constinit)
const PosixConfig::Key PosixConfig::kCounterKey_BootReason = MakeConfigKey(kConfigNamespace_ChipCounters, "boot-reason");

namespace {
CHIP_ERROR MapToDeviceError(CHIP_ERROR err)
{
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    return err;
}
} // namespace

CHIP_ERROR PosixConfig::Init()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR PosixConfig::ReadConfigValue(Key key, bool & val)
{
    return MapToDeviceError(PersistedStorage::KeyValueStoreMgr().Get(key.Name, &val));
}

CHIP_ERROR PosixConfig::ReadConfigValue(Key key, uint16_t & val)

{
    return MapToDeviceError(PersistedStorage::KeyValueStoreMgr().Get(key.Name, &val));
}

CHIP_ERROR PosixConfig::ReadConfigValue(Key key, uint32_t & val)
{
    return MapToDeviceError(PersistedStorage::KeyValueStoreMgr().Get(key.Name, &val));
}

CHIP_ERROR PosixConfig::ReadConfigValue(Key key, uint64_t & val)
{
    return MapToDeviceError(PersistedStorage::KeyValueStoreMgr().Get(key.Name, &val));
}

CHIP_ERROR PosixConfig::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    VerifyOrReturnError(buf != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    auto err = MapToDeviceError(PersistedStorage::KeyValueStoreMgr().Get(key.Name, buf, bufSize, &outLen));
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
    return MapToDeviceError(PersistedStorage::KeyValueStoreMgr().Get(key.Name, buf, bufSize, &outLen));
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
