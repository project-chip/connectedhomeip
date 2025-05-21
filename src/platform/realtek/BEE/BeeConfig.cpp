/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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

#include "chip_porting.h"
#include <core/CHIPEncoding.h>
#include <platform/realtek/BEE/BeeConfig.h>
#include <support/CHIPMem.h>
#include <support/CHIPMemString.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

enum
{
    kPrefsTypeBoolean = 1,
    kPrefsTypeInteger = 2,
    kPrefsTypeString  = 3,
    kPrefsTypeBuffer  = 4,
    kPrefsTypeBinary  = 5
};

namespace chip {
namespace DeviceLayer {
namespace Internal {

// *** CAUTION ***: Changing the names or namespaces of these values will *break* existing devices.

// NVS namespaces used to store device configuration information.
const char BeeConfig::kConfigNamespace_ChipFactory[]  = "chip-factory";
const char BeeConfig::kConfigNamespace_ChipConfig[]   = "chip-config";
const char BeeConfig::kConfigNamespace_ChipCounters[] = "chip-counters";

// Keys stored in the chip-factory namespace
const BeeConfig::Key BeeConfig::kConfigKey_SerialNum             = { kConfigNamespace_ChipFactory, "serial-num" };
const BeeConfig::Key BeeConfig::kConfigKey_MfrDeviceId           = { kConfigNamespace_ChipFactory, "device-id" };
const BeeConfig::Key BeeConfig::kConfigKey_MfrDeviceCert         = { kConfigNamespace_ChipFactory, "device-cert" };
const BeeConfig::Key BeeConfig::kConfigKey_MfrDeviceICACerts     = { kConfigNamespace_ChipFactory, "device-ca-certs" };
const BeeConfig::Key BeeConfig::kConfigKey_MfrDevicePrivateKey   = { kConfigNamespace_ChipFactory, "device-key" };
const BeeConfig::Key BeeConfig::kConfigKey_HardwareVersion       = { kConfigNamespace_ChipFactory, "hardware-ver" };
const BeeConfig::Key BeeConfig::kConfigKey_ManufacturingDate     = { kConfigNamespace_ChipFactory, "mfg-date" };
const BeeConfig::Key BeeConfig::kConfigKey_SetupPinCode          = { kConfigNamespace_ChipFactory, "pin-code" };
const BeeConfig::Key BeeConfig::kConfigKey_SetupDiscriminator    = { kConfigNamespace_ChipFactory, "discriminator" };
const BeeConfig::Key BeeConfig::kConfigKey_Spake2pIterationCount = { kConfigNamespace_ChipFactory, "iteration-count" };
const BeeConfig::Key BeeConfig::kConfigKey_Spake2pSalt           = { kConfigNamespace_ChipFactory, "salt" };
const BeeConfig::Key BeeConfig::kConfigKey_Spake2pVerifier       = { kConfigNamespace_ChipFactory, "verifier" };
const BeeConfig::Key BeeConfig::kConfigKey_UniqueId              = { kConfigNamespace_ChipFactory, "uniqueId" };

// Keys stored in the chip-config namespace
const BeeConfig::Key BeeConfig::kConfigKey_ServiceConfig               = { kConfigNamespace_ChipConfig, "service-config" };
const BeeConfig::Key BeeConfig::kConfigKey_PairedAccountId             = { kConfigNamespace_ChipConfig, "account-id" };
const BeeConfig::Key BeeConfig::kConfigKey_ServiceId                   = { kConfigNamespace_ChipConfig, "service-id" };
const BeeConfig::Key BeeConfig::kConfigKey_LastUsedEpochKeyId          = { kConfigNamespace_ChipConfig, "last-ek-id" };
const BeeConfig::Key BeeConfig::kConfigKey_FailSafeArmed               = { kConfigNamespace_ChipConfig, "fail-safe-armed" };
const BeeConfig::Key BeeConfig::kConfigKey_WiFiStationSecType          = { kConfigNamespace_ChipConfig, "sta-sec-type" };
const BeeConfig::Key BeeConfig::kConfigKey_OperationalDeviceId         = { kConfigNamespace_ChipConfig, "op-device-id" };
const BeeConfig::Key BeeConfig::kConfigKey_OperationalDeviceCert       = { kConfigNamespace_ChipConfig, "op-device-cert" };
const BeeConfig::Key BeeConfig::kConfigKey_OperationalDeviceICACerts   = { kConfigNamespace_ChipConfig, "op-device-ca-certs" };
const BeeConfig::Key BeeConfig::kConfigKey_OperationalDevicePrivateKey = { kConfigNamespace_ChipConfig, "op-device-key" };
const BeeConfig::Key BeeConfig::kConfigKey_RegulatoryLocation          = { kConfigNamespace_ChipConfig, "regulatory-location" };
const BeeConfig::Key BeeConfig::kConfigKey_CountryCode                 = { kConfigNamespace_ChipConfig, "country-code" };
const BeeConfig::Key BeeConfig::kConfigKey_LockUser                    = { kConfigNamespace_ChipConfig, "lock-user" };
const BeeConfig::Key BeeConfig::kConfigKey_LockUserData                = { kConfigNamespace_ChipConfig, "lock-user-data" };
const BeeConfig::Key BeeConfig::kConfigKey_Credential                  = { kConfigNamespace_ChipConfig, "lock-credential" };
const BeeConfig::Key BeeConfig::kConfigKey_CredentialData              = { kConfigNamespace_ChipConfig, "lock-credential-data" };

// Keys stored in the Chip-counters namespace
const BeeConfig::Key BeeConfig::kCounterKey_RebootCount           = { kConfigNamespace_ChipCounters, "reboot-count" };
const BeeConfig::Key BeeConfig::kCounterKey_UpTime                = { kConfigNamespace_ChipCounters, "up-time" };
const BeeConfig::Key BeeConfig::kCounterKey_TotalOperationalHours = { kConfigNamespace_ChipCounters, "total-hours" };
const BeeConfig::Key BeeConfig::kCounterKey_BootReason            = { kConfigNamespace_ChipCounters, "boot-reason" };

CHIP_ERROR BeeConfig::ReadConfigValue(Key key, bool & val)
{
    bool tmp;
    size_t readsize;
    int32_t ret = matter_kvs_get(key.Name, &tmp, sizeof(tmp), &readsize);

    if (MATTER_KVS_STATUS_NO_ERROR != ret)
    {
        ChipLogError(DeviceLayer, "%s %s failed", __func__, key.Name);
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    else
    {
        val = tmp;
        return CHIP_NO_ERROR;
    }
}

CHIP_ERROR BeeConfig::ReadConfigValue(Key key, uint32_t & val)
{
    uint32_t tmp;
    size_t readsize;
    int32_t ret = matter_kvs_get(key.Name, &tmp, sizeof(tmp), &readsize);

    if (MATTER_KVS_STATUS_NO_ERROR != ret)
    {
        ChipLogError(DeviceLayer, "%s %s failed", __func__, key.Name);
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    else
    {
        val = tmp;
        return CHIP_NO_ERROR;
    }
}

CHIP_ERROR BeeConfig::ReadConfigValue(Key key, uint64_t & val)
{
    uint64_t tmp;
    size_t readsize;
    int32_t ret = matter_kvs_get(key.Name, &tmp, sizeof(tmp), &readsize);

    if (MATTER_KVS_STATUS_NO_ERROR != ret)
    {
        ChipLogError(DeviceLayer, "%s %s failed", __func__, key.Name);
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    else
    {
        val = tmp;
        return CHIP_NO_ERROR;
    }
}

CHIP_ERROR BeeConfig::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    size_t readsize;
    int32_t ret = matter_kvs_get(key.Name, buf, bufSize, &readsize);

    if (MATTER_KVS_STATUS_NO_ERROR != ret)
    {
        ChipLogError(DeviceLayer, "%s %s failed", __func__, key.Name);
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    else
    {
        outLen = readsize;
        return CHIP_NO_ERROR;
    }
}

CHIP_ERROR BeeConfig::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    size_t readsize;
    int32_t ret = matter_kvs_get(key.Name, buf, bufSize, &readsize);

    if (MATTER_KVS_STATUS_NO_ERROR != ret)
    {
        ChipLogError(DeviceLayer, "%s %s failed", __func__, key.Name);
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    else
    {
        outLen = readsize;
        return CHIP_NO_ERROR;
    }
}

CHIP_ERROR BeeConfig::WriteConfigValue(Key key, bool val)
{
    int32_t ret = matter_kvs_put(key.Name, &val, sizeof(val));

    if (MATTER_KVS_STATUS_NO_ERROR != ret)
    {
        ChipLogError(DeviceLayer, "%s %s failed", __func__, key.Name);
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    else
    {
        return CHIP_NO_ERROR;
    }
}

CHIP_ERROR BeeConfig::WriteConfigValue(Key key, uint32_t val)
{
    int32_t ret = matter_kvs_put(key.Name, &val, sizeof(val));

    if (MATTER_KVS_STATUS_NO_ERROR != ret)
    {
        ChipLogError(DeviceLayer, "%s %s failed", __func__, key.Name);
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    else
    {
        return CHIP_NO_ERROR;
    }
}

CHIP_ERROR BeeConfig::WriteConfigValue(Key key, uint64_t val)
{
    int32_t ret = matter_kvs_put(key.Name, &val, sizeof(val));

    if (MATTER_KVS_STATUS_NO_ERROR != ret)
    {
        ChipLogError(DeviceLayer, "%s %s failed", __func__, key.Name);
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    else
    {
        return CHIP_NO_ERROR;
    }
}

CHIP_ERROR BeeConfig::WriteConfigValueStr(Key key, const char * str)
{
    int32_t ret = matter_kvs_put(key.Name, str, strlen(str));

    if (MATTER_KVS_STATUS_NO_ERROR != ret)
    {
        ChipLogError(DeviceLayer, "%s %s failed", __func__, key.Name);
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    else
    {
        return CHIP_NO_ERROR;
    }
}

CHIP_ERROR BeeConfig::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    int32_t ret = matter_kvs_put(key.Name, str, strLen);

    if (MATTER_KVS_STATUS_NO_ERROR != ret)
    {
        ChipLogError(DeviceLayer, "%s %s failed", __func__, key.Name);
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    else
    {
        return CHIP_NO_ERROR;
    }
}

CHIP_ERROR BeeConfig::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    int32_t ret = matter_kvs_put(key.Name, data, dataLen);

    if (MATTER_KVS_STATUS_NO_ERROR != ret)
    {
        ChipLogError(DeviceLayer, "%s %s failed", __func__, key.Name);
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    else
    {
        return CHIP_NO_ERROR;
    }
}

CHIP_ERROR BeeConfig::ClearConfigValue(Key key)
{
    int32_t ret = matter_kvs_key_delete(key.Name);

    if (MATTER_KVS_STATUS_NO_ERROR != ret)
    {
        ChipLogError(DeviceLayer, "%s %s failed", __func__, key.Name);
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    else
    {
        return CHIP_NO_ERROR;
    }
}

bool BeeConfig::ConfigValueExists(Key key)
{
    return matter_kvs_key_find(key.Name);
}

CHIP_ERROR BeeConfig::InitNamespace()
{
    int32_t success = -1;

    success = matter_kvs_init();

    if (success != 0)
    {
        ChipLogError(DeviceLayer, "KVS modules init failed");
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR BeeConfig::ClearNamespace()
{
    matter_kvs_clean();

    return CHIP_NO_ERROR;
}

void BeeConfig::RunConfigUnitTest() {}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
