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

#include <core/CHIPEncoding.h>
#include <platform/Ameba/AmebaConfig.h>
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
const char AmebaConfig::kConfigNamespace_ChipFactory[]              = "chip-factory";
const char AmebaConfig::kConfigNamespace_ChipConfig[]               = "chip-config";
const char AmebaConfig::kConfigNamespace_ChipCounters[]             = "chip-counters";
const char AmebaConfig::kConfigNamespace_ChipFabric1[]              = "chip-fabric-1";
const char AmebaConfig::kConfigNamespace_ChipFabric2[]              = "chip-fabric-2";
const char AmebaConfig::kConfigNamespace_ChipFabric3[]              = "chip-fabric-3";
const char AmebaConfig::kConfigNamespace_ChipFabric4[]              = "chip-fabric-4";
const char AmebaConfig::kConfigNamespace_ChipFabric5[]              = "chip-fabric-5";
const char AmebaConfig::kConfigNamespace_ChipACL[]                  = "chip-acl";
const char AmebaConfig::kConfigNamespace_ChipGroupMessageCounters[] = "chip-groupmsgcounters";
const char AmebaConfig::kConfigNamespace_ChipAttributes[]           = "chip-attributes";
const char AmebaConfig::kConfigNamespace_ChipBindingTable[]         = "chip-bindingtable";
const char AmebaConfig::kConfigNamespace_ChipOTA[]                  = "chip-ota";
const char AmebaConfig::kConfigNamespace_ChipFailSafe[]             = "chip-failsafe";
const char AmebaConfig::kConfigNamespace_ChipSessionResumption[]    = "chip-sessionresumption";
const char AmebaConfig::kConfigNamespace_ChipDeviceInfoProvider[]   = "chip-deviceinfoprovider";
const char AmebaConfig::kConfigNamespace_ChipGroupDataProvider[]    = "chip-groupdataprovider";
const char AmebaConfig::kConfigNamespace_ChipOthers[]               = "chip-others";
const char AmebaConfig::kConfigNamespace_ChipOthers2[]              = "chip-others2";

// Keys stored in the chip-factory namespace
const AmebaConfig::Key AmebaConfig::kConfigKey_SerialNum             = { kConfigNamespace_ChipFactory, "serial-num" };
const AmebaConfig::Key AmebaConfig::kConfigKey_MfrDeviceId           = { kConfigNamespace_ChipFactory, "device-id" };
const AmebaConfig::Key AmebaConfig::kConfigKey_MfrDeviceCert         = { kConfigNamespace_ChipFactory, "device-cert" };
const AmebaConfig::Key AmebaConfig::kConfigKey_MfrDeviceICACerts     = { kConfigNamespace_ChipFactory, "device-ca-certs" };
const AmebaConfig::Key AmebaConfig::kConfigKey_MfrDevicePrivateKey   = { kConfigNamespace_ChipFactory, "device-key" };
const AmebaConfig::Key AmebaConfig::kConfigKey_HardwareVersion       = { kConfigNamespace_ChipFactory, "hardware-ver" };
const AmebaConfig::Key AmebaConfig::kConfigKey_ManufacturingDate     = { kConfigNamespace_ChipFactory, "mfg-date" };
const AmebaConfig::Key AmebaConfig::kConfigKey_SetupPinCode          = { kConfigNamespace_ChipFactory, "pin-code" };
const AmebaConfig::Key AmebaConfig::kConfigKey_SetupDiscriminator    = { kConfigNamespace_ChipFactory, "discriminator" };
const AmebaConfig::Key AmebaConfig::kConfigKey_Spake2pIterationCount = { kConfigNamespace_ChipFactory, "iteration-count" };
const AmebaConfig::Key AmebaConfig::kConfigKey_Spake2pSalt           = { kConfigNamespace_ChipFactory, "salt" };
const AmebaConfig::Key AmebaConfig::kConfigKey_Spake2pVerifier       = { kConfigNamespace_ChipFactory, "verifier" };
const AmebaConfig::Key AmebaConfig::kConfigKey_UniqueId              = { kConfigNamespace_ChipFactory, "uniqueId" };

// Keys stored in the chip-config namespace
const AmebaConfig::Key AmebaConfig::kConfigKey_ServiceConfig               = { kConfigNamespace_ChipConfig, "service-config" };
const AmebaConfig::Key AmebaConfig::kConfigKey_PairedAccountId             = { kConfigNamespace_ChipConfig, "account-id" };
const AmebaConfig::Key AmebaConfig::kConfigKey_ServiceId                   = { kConfigNamespace_ChipConfig, "service-id" };
const AmebaConfig::Key AmebaConfig::kConfigKey_LastUsedEpochKeyId          = { kConfigNamespace_ChipConfig, "last-ek-id" };
const AmebaConfig::Key AmebaConfig::kConfigKey_FailSafeArmed               = { kConfigNamespace_ChipConfig, "fail-safe-armed" };
const AmebaConfig::Key AmebaConfig::kConfigKey_WiFiStationSecType          = { kConfigNamespace_ChipConfig, "sta-sec-type" };
const AmebaConfig::Key AmebaConfig::kConfigKey_OperationalDeviceId         = { kConfigNamespace_ChipConfig, "op-device-id" };
const AmebaConfig::Key AmebaConfig::kConfigKey_OperationalDeviceCert       = { kConfigNamespace_ChipConfig, "op-device-cert" };
const AmebaConfig::Key AmebaConfig::kConfigKey_OperationalDeviceICACerts   = { kConfigNamespace_ChipConfig, "op-device-ca-certs" };
const AmebaConfig::Key AmebaConfig::kConfigKey_OperationalDevicePrivateKey = { kConfigNamespace_ChipConfig, "op-device-key" };
const AmebaConfig::Key AmebaConfig::kConfigKey_RegulatoryLocation          = { kConfigNamespace_ChipConfig, "regulatory-location" };
const AmebaConfig::Key AmebaConfig::kConfigKey_CountryCode                 = { kConfigNamespace_ChipConfig, "country-code" };

// Keys stored in the Chip-counters namespace
const AmebaConfig::Key AmebaConfig::kCounterKey_RebootCount           = { kConfigNamespace_ChipCounters, "reboot-count" };
const AmebaConfig::Key AmebaConfig::kCounterKey_UpTime                = { kConfigNamespace_ChipCounters, "up-time" };
const AmebaConfig::Key AmebaConfig::kCounterKey_TotalOperationalHours = { kConfigNamespace_ChipCounters, "total-hours" };
const AmebaConfig::Key AmebaConfig::kCounterKey_BootReason            = { kConfigNamespace_ChipCounters, "boot-reason" };

CHIP_ERROR AmebaConfig::ReadConfigValue(Key key, bool & val)
{
    uint8_t intVal;
    int32_t success = 0;

    success = getPref_bool_new(key.Namespace, key.Name, &intVal);
    if (success != 0)
        ChipLogProgress(DeviceLayer, "getPref_bool_new: %s/%s failed\n", key.Namespace, key.Name);

    val = (intVal != 0);

    if (success == 0)
        return CHIP_NO_ERROR;
    else
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
}

CHIP_ERROR AmebaConfig::ReadConfigValue(Key key, uint32_t & val)
{
    int32_t success = 0;

    success = getPref_u32_new(key.Namespace, key.Name, &val);
    if (success != 0)
        ChipLogProgress(DeviceLayer, "getPref_u32_new: %s/%s failed\n", key.Namespace, key.Name);

    if (success == 0)
        return CHIP_NO_ERROR;
    else
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
}

CHIP_ERROR AmebaConfig::ReadConfigValue(Key key, uint64_t & val)
{
    int32_t success = 0;

    success = getPref_u64_new(key.Namespace, key.Name, &val);
    if (success != 0)
        ChipLogProgress(DeviceLayer, "getPref_u32_new: %s/%s failed\n", key.Namespace, key.Name);

    if (success == 0)
        return CHIP_NO_ERROR;
    else
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
}

CHIP_ERROR AmebaConfig::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    int32_t success = 0;

    success = getPref_str_new(key.Namespace, key.Name, buf, bufSize, &outLen);
    if (success != 0)
        ChipLogProgress(DeviceLayer, "getPref_str_new: %s/%s failed\n", key.Namespace, key.Name);

    if (success == 0)
    {
        outLen -= 1; // Don't count trailing null
        return CHIP_NO_ERROR;
    }
    else
    {
        outLen = 0;
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
}

CHIP_ERROR AmebaConfig::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    int32_t success = 0;

    success = getPref_bin_new(key.Namespace, key.Name, buf, bufSize, &outLen);
    if (success != 0)
        ChipLogProgress(DeviceLayer, "getPref_bin_new: %s/%s failed\n", key.Namespace, key.Name);

    if (success == 0)
    {
        return CHIP_NO_ERROR;
    }
    else
    {
        outLen = 0;
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
}

CHIP_ERROR AmebaConfig::WriteConfigValue(Key key, bool val)
{
    int32_t success;
    uint8_t value;

    if (val == 1)
        value = 1;
    else
        value = 0;
    success = setPref_new(key.Namespace, key.Name, &value, 1);
    if (!success)
        ChipLogError(DeviceLayer, "setPref: %s/%s = %s failed\n", key.Namespace, key.Name, value ? "true" : "false");
    else
        ChipLogProgress(DeviceLayer, "NVS set: %s/%s = %s", key.Namespace, key.Name, val ? "true" : "false");

    return CHIP_NO_ERROR;
}

CHIP_ERROR AmebaConfig::WriteConfigValue(Key key, uint32_t val)
{
    int32_t success;

    success = setPref_new(key.Namespace, key.Name, (uint8_t *) &val, sizeof(uint32_t));
    if (!success)
        ChipLogError(DeviceLayer, "setPref: %s/%s = %d(0x%x) failed\n", key.Namespace, key.Name, val, val);
    else
        ChipLogProgress(DeviceLayer, "NVS set: %s/%s = %" PRIu32 " (0x%" PRIX32 ")", key.Namespace, key.Name, val, val);

    return CHIP_NO_ERROR;
}

CHIP_ERROR AmebaConfig::WriteConfigValue(Key key, uint64_t val)
{
    int32_t success;

    success = setPref_new(key.Namespace, key.Name, (uint8_t *) &val, sizeof(uint64_t));
    if (!success)
        ChipLogError(DeviceLayer, "setPref: %s/%s = %d(0x%x) failed\n", key.Namespace, key.Name, val, val);
    else
        ChipLogProgress(DeviceLayer, "NVS set: %s/%s = %" PRIu64 " (0x%" PRIX64 ")", key.Namespace, key.Name, val, val);

    return CHIP_NO_ERROR;
}

CHIP_ERROR AmebaConfig::WriteConfigValueStr(Key key, const char * str)
{
    int32_t success;

    success = setPref_new(key.Namespace, key.Name, (uint8_t *) str, strlen(str) + 1);
    if (!success)
        ChipLogError(DeviceLayer, "setPref: %s/%s = %s failed\n", key.Namespace, key.Name, str);
    else
        ChipLogProgress(DeviceLayer, "NVS set: %s/%s = \"%s\"", key.Namespace, key.Name, str);
    return CHIP_NO_ERROR;
}

CHIP_ERROR AmebaConfig::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    CHIP_ERROR err;
    chip::Platform::ScopedMemoryBuffer<char> strCopy;

    if (str != NULL)
    {
        strCopy.Calloc(strLen + 1);
        VerifyOrExit(strCopy, err = CHIP_ERROR_NO_MEMORY);
        strncpy(strCopy.Get(), str, strLen);
    }
    err = AmebaConfig::WriteConfigValueStr(key, strCopy.Get());
exit:
    return err;
}

CHIP_ERROR AmebaConfig::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    int32_t success;

    success = setPref_new(key.Namespace, key.Name, (uint8_t *) data, dataLen);
    if (!success)
        ChipLogError(DeviceLayer, "setPref: %s/%s failed\n", key.Namespace, key.Name);
    else
        ChipLogProgress(DeviceLayer, "NVS set: %s/%s = (blob length %" PRId32 ")", key.Namespace, key.Name, dataLen);

    return CHIP_NO_ERROR;
}

CHIP_ERROR AmebaConfig::ClearConfigValue(Key key)
{
    int32_t success;

    success = deleteKey(key.Namespace, key.Name);
    if (!success)
        ChipLogProgress(DeviceLayer, "%s : %s/%s failed\n", __FUNCTION__, key.Namespace, key.Name);
    else
        ChipLogProgress(DeviceLayer, "NVS erase: %s/%s", key.Namespace, key.Name);

    return CHIP_NO_ERROR;
}

bool AmebaConfig::ConfigValueExists(Key key)
{
    return checkExist(key.Namespace, key.Name);
}

CHIP_ERROR AmebaConfig::EnsureNamespace(const char * ns)
{
    int32_t success = -1;

    success = registerPref(ns);
    if (success != 0)
    {
        ChipLogError(DeviceLayer, "dct_register_module failed\n");
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR AmebaConfig::EnsureNamespace2(const char * ns)
{
    int32_t success = -1;

    success = registerPref2(ns);
    if (success != 0)
    {
        ChipLogError(DeviceLayer, "dct_register_module2 failed\n");
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR AmebaConfig::ClearNamespace(const char * ns)
{
    int32_t success = -1;

    success = clearPref(ns);
    if (success != 0)
    {
        ChipLogError(DeviceLayer, "ClearNamespace failed\n");
    }

    return CHIP_NO_ERROR;
}

void AmebaConfig::RunConfigUnitTest() {}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
