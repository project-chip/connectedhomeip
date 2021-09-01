/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

namespace chip {
namespace DeviceLayer {
namespace Internal {

// *** CAUTION ***: Changing the names or namespaces of these values will *break* existing devices.

// NVS namespaces used to store device configuration information.
const char PosixConfig::kConfigNamespace_ChipFactory[]  = "chip-factory";
const char PosixConfig::kConfigNamespace_ChipConfig[]   = "chip-config";
const char PosixConfig::kConfigNamespace_ChipCounters[] = "chip-counters";

// Keys stored in the Chip-factory namespace
const PosixConfig::Key PosixConfig::kConfigKey_SerialNum           = { kConfigNamespace_ChipFactory, "serial-num" };
const PosixConfig::Key PosixConfig::kConfigKey_MfrDeviceId         = { kConfigNamespace_ChipFactory, "device-id" };
const PosixConfig::Key PosixConfig::kConfigKey_MfrDeviceCert       = { kConfigNamespace_ChipFactory, "device-cert" };
const PosixConfig::Key PosixConfig::kConfigKey_MfrDeviceICACerts   = { kConfigNamespace_ChipFactory, "device-ca-certs" };
const PosixConfig::Key PosixConfig::kConfigKey_MfrDevicePrivateKey = { kConfigNamespace_ChipFactory, "device-key" };
const PosixConfig::Key PosixConfig::kConfigKey_ProductRevision     = { kConfigNamespace_ChipFactory, "product-rev" };
const PosixConfig::Key PosixConfig::kConfigKey_ManufacturingDate   = { kConfigNamespace_ChipFactory, "mfg-date" };
const PosixConfig::Key PosixConfig::kConfigKey_SetupPinCode        = { kConfigNamespace_ChipFactory, "pin-code" };
const PosixConfig::Key PosixConfig::kConfigKey_SetupDiscriminator  = { kConfigNamespace_ChipFactory, "discriminator" };

// Keys stored in the Chip-config namespace
const PosixConfig::Key PosixConfig::kConfigKey_FabricId                    = { kConfigNamespace_ChipConfig, "fabric-id" };
const PosixConfig::Key PosixConfig::kConfigKey_ServiceConfig               = { kConfigNamespace_ChipConfig, "service-config" };
const PosixConfig::Key PosixConfig::kConfigKey_PairedAccountId             = { kConfigNamespace_ChipConfig, "account-id" };
const PosixConfig::Key PosixConfig::kConfigKey_ServiceId                   = { kConfigNamespace_ChipConfig, "service-id" };
const PosixConfig::Key PosixConfig::kConfigKey_FabricSecret                = { kConfigNamespace_ChipConfig, "fabric-secret" };
const PosixConfig::Key PosixConfig::kConfigKey_GroupKeyIndex               = { kConfigNamespace_ChipConfig, "group-key-index" };
const PosixConfig::Key PosixConfig::kConfigKey_LastUsedEpochKeyId          = { kConfigNamespace_ChipConfig, "last-ek-id" };
const PosixConfig::Key PosixConfig::kConfigKey_FailSafeArmed               = { kConfigNamespace_ChipConfig, "fail-safe-armed" };
const PosixConfig::Key PosixConfig::kConfigKey_WiFiStationSecType          = { kConfigNamespace_ChipConfig, "sta-sec-type" };
const PosixConfig::Key PosixConfig::kConfigKey_OperationalDeviceId         = { kConfigNamespace_ChipConfig, "op-device-id" };
const PosixConfig::Key PosixConfig::kConfigKey_OperationalDeviceCert       = { kConfigNamespace_ChipConfig, "op-device-cert" };
const PosixConfig::Key PosixConfig::kConfigKey_OperationalDeviceICACerts   = { kConfigNamespace_ChipConfig, "op-device-ca-certs" };
const PosixConfig::Key PosixConfig::kConfigKey_OperationalDevicePrivateKey = { kConfigNamespace_ChipConfig, "op-device-key" };
const PosixConfig::Key PosixConfig::kConfigKey_RegulatoryLocation          = { kConfigNamespace_ChipConfig, "regulatory-location" };
const PosixConfig::Key PosixConfig::kConfigKey_CountryCode                 = { kConfigNamespace_ChipConfig, "country-code" };
const PosixConfig::Key PosixConfig::kConfigKey_Breadcrumb                  = { kConfigNamespace_ChipConfig, "breadcrumb" };

// Prefix used for NVS keys that contain Chip group encryption keys.
const char PosixConfig::kGroupKeyNamePrefix[] = "gk-";

CHIP_ERROR PosixConfig::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    return err;
}

CHIP_ERROR PosixConfig::ReadConfigValue(Key key, bool & val)
{
    CHIP_ERROR err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR PosixConfig::ReadConfigValue(Key key, uint32_t & val)
{
    CHIP_ERROR err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR PosixConfig::ReadConfigValue(Key key, uint64_t & val)
{
    CHIP_ERROR err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR PosixConfig::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    CHIP_ERROR err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR PosixConfig::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    CHIP_ERROR err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR PosixConfig::WriteConfigValue(Key key, bool val)
{
    CHIP_ERROR err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR PosixConfig::WriteConfigValue(Key key, uint32_t val)
{
    CHIP_ERROR err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR PosixConfig::WriteConfigValue(Key key, uint64_t val)
{
    CHIP_ERROR err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR PosixConfig::WriteConfigValueStr(Key key, const char * str)
{
    CHIP_ERROR err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR PosixConfig::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    CHIP_ERROR err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR PosixConfig::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    CHIP_ERROR err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR PosixConfig::ClearConfigValue(Key key)
{
    CHIP_ERROR err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    SuccessOrExit(err);

exit:
    return err;
}

bool PosixConfig::ConfigValueExists(Key key)
{
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

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
