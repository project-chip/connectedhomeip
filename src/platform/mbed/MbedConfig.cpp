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
 *          Utilities for interacting with the the ESP32 "NVS" key-value store.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/mbed/MbedConfig.h>

#include <core/CHIPEncoding.h>
#include <support/CHIPMem.h>
#include <support/CHIPMemString.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

// *** CAUTION ***: Changing the names or namespaces of these values will *break* existing devices.

// NVS namespaces used to store device configuration information.
const char MbedConfig::kConfigNamespace_ChipFactory[]  = "chip-factory";
const char MbedConfig::kConfigNamespace_ChipConfig[]   = "chip-config";
const char MbedConfig::kConfigNamespace_ChipCounters[] = "chip-counters";

// Keys stored in the chip-factory namespace
const MbedConfig::Key MbedConfig::kConfigKey_SerialNum           = { kConfigNamespace_ChipFactory, "serial-num" };
const MbedConfig::Key MbedConfig::kConfigKey_MfrDeviceId         = { kConfigNamespace_ChipFactory, "device-id" };
const MbedConfig::Key MbedConfig::kConfigKey_MfrDeviceCert       = { kConfigNamespace_ChipFactory, "device-cert" };
const MbedConfig::Key MbedConfig::kConfigKey_MfrDeviceICACerts   = { kConfigNamespace_ChipFactory, "device-ca-certs" };
const MbedConfig::Key MbedConfig::kConfigKey_MfrDevicePrivateKey = { kConfigNamespace_ChipFactory, "device-key" };
const MbedConfig::Key MbedConfig::kConfigKey_ProductRevision     = { kConfigNamespace_ChipFactory, "product-rev" };
const MbedConfig::Key MbedConfig::kConfigKey_ManufacturingDate   = { kConfigNamespace_ChipFactory, "mfg-date" };
const MbedConfig::Key MbedConfig::kConfigKey_SetupPinCode        = { kConfigNamespace_ChipFactory, "pin-code" };
const MbedConfig::Key MbedConfig::kConfigKey_SetupDiscriminator  = { kConfigNamespace_ChipFactory, "discriminator" };

// Keys stored in the chip-config namespace
const MbedConfig::Key MbedConfig::kConfigKey_FabricId                    = { kConfigNamespace_ChipConfig, "fabric-id" };
const MbedConfig::Key MbedConfig::kConfigKey_ServiceConfig               = { kConfigNamespace_ChipConfig, "service-config" };
const MbedConfig::Key MbedConfig::kConfigKey_PairedAccountId             = { kConfigNamespace_ChipConfig, "account-id" };
const MbedConfig::Key MbedConfig::kConfigKey_ServiceId                   = { kConfigNamespace_ChipConfig, "service-id" };
const MbedConfig::Key MbedConfig::kConfigKey_GroupKeyIndex               = { kConfigNamespace_ChipConfig, "group-key-index" };
const MbedConfig::Key MbedConfig::kConfigKey_LastUsedEpochKeyId          = { kConfigNamespace_ChipConfig, "last-ek-id" };
const MbedConfig::Key MbedConfig::kConfigKey_FailSafeArmed               = { kConfigNamespace_ChipConfig, "fail-safe-armed" };
const MbedConfig::Key MbedConfig::kConfigKey_WiFiStationSecType          = { kConfigNamespace_ChipConfig, "sta-sec-type" };
const MbedConfig::Key MbedConfig::kConfigKey_OperationalDeviceId         = { kConfigNamespace_ChipConfig, "op-device-id" };
const MbedConfig::Key MbedConfig::kConfigKey_OperationalDeviceCert       = { kConfigNamespace_ChipConfig, "op-device-cert" };
const MbedConfig::Key MbedConfig::kConfigKey_OperationalDeviceICACerts   = { kConfigNamespace_ChipConfig, "op-device-ca-certs" };
const MbedConfig::Key MbedConfig::kConfigKey_OperationalDevicePrivateKey = { kConfigNamespace_ChipConfig, "op-device-key" };

// Prefix used for NVS keys that contain Chip group encryption keys.
const char MbedConfig::kGroupKeyNamePrefix[] = "gk-";

CHIP_ERROR MbedConfig::ReadConfigValue(Key key, bool & val)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR MbedConfig::ReadConfigValue(Key key, uint32_t & val)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR MbedConfig::ReadConfigValue(Key key, uint64_t & val)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR MbedConfig::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR MbedConfig::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR MbedConfig::WriteConfigValue(Key key, bool val)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR MbedConfig::WriteConfigValue(Key key, uint32_t val)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR MbedConfig::WriteConfigValue(Key key, uint64_t val)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR MbedConfig::WriteConfigValueStr(Key key, const char * str)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR MbedConfig::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR MbedConfig::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR MbedConfig::ClearConfigValue(Key key)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

bool MbedConfig::ConfigValueExists(Key key)
{
    return false;
}

CHIP_ERROR MbedConfig::EnsureNamespace(const char * ns)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR MbedConfig::ClearNamespace(const char * ns)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

void MbedConfig::RunConfigUnitTest() {}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
