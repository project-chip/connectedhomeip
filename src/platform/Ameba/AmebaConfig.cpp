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
#include <platform/Ameba/AmebaUtils.h>
#include <platform/Ameba/KeyValueStoreManagerImpl.h>
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

using namespace chip::DeviceLayer::PersistedStorage;

namespace chip {
namespace DeviceLayer {
namespace Internal {

// *** CAUTION ***: Changing the names or namespaces of these values will *break* existing devices.

// NVS namespaces used to store device configuration information.
const char AmebaConfig::kConfigNamespace_ChipFactory[]  = "chip-factory";
const char AmebaConfig::kConfigNamespace_ChipConfig[]   = "chip-config";
const char AmebaConfig::kConfigNamespace_ChipCounters[] = "chip-counters";

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
const AmebaConfig::Key AmebaConfig::kConfigKey_OperationalDeviceId         = { kConfigNamespace_ChipConfig, "op-device-id" };
const AmebaConfig::Key AmebaConfig::kConfigKey_OperationalDeviceCert       = { kConfigNamespace_ChipConfig, "op-device-cert" };
const AmebaConfig::Key AmebaConfig::kConfigKey_OperationalDeviceICACerts   = { kConfigNamespace_ChipConfig, "op-device-ca-certs" };
const AmebaConfig::Key AmebaConfig::kConfigKey_OperationalDevicePrivateKey = { kConfigNamespace_ChipConfig, "op-device-key" };
const AmebaConfig::Key AmebaConfig::kConfigKey_RegulatoryLocation          = { kConfigNamespace_ChipConfig, "regulatory-location" };
const AmebaConfig::Key AmebaConfig::kConfigKey_LocationCapability          = { kConfigNamespace_ChipConfig, "location-capability" };
const AmebaConfig::Key AmebaConfig::kConfigKey_CountryCode                 = { kConfigNamespace_ChipConfig, "country-code" };

// Keys stored in the Chip-counters namespace
const AmebaConfig::Key AmebaConfig::kCounterKey_RebootCount           = { kConfigNamespace_ChipCounters, "reboot-count" };
const AmebaConfig::Key AmebaConfig::kCounterKey_UpTime                = { kConfigNamespace_ChipCounters, "up-time" };
const AmebaConfig::Key AmebaConfig::kCounterKey_TotalOperationalHours = { kConfigNamespace_ChipCounters, "total-hours" };
const AmebaConfig::Key AmebaConfig::kCounterKey_BootReason            = { kConfigNamespace_ChipCounters, "boot-reason" };

CHIP_ERROR AmebaConfig::ReadConfigValue(Key key, bool & val)
{
    CHIP_ERROR err;
    int32_t error;
    uint8_t intVal;

    error = getPref_bool_new(key.Namespace, key.Name, &intVal);
    err   = AmebaUtils::MapError(error, AmebaErrorType::kDctError);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(DeviceLayer, "getPref_bool_new: %s/%s failed\n", StringOrNullMarker(key.Namespace),
                        StringOrNullMarker(key.Name));
    }

    val = (intVal != 0);
    return err;
}

CHIP_ERROR AmebaConfig::ReadConfigValue(Key key, uint32_t & val)
{
    CHIP_ERROR err;
    int32_t error;

    error = getPref_u32_new(key.Namespace, key.Name, &val);
    err   = AmebaUtils::MapError(error, AmebaErrorType::kDctError);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(DeviceLayer, "getPref_u32_new: %s/%s failed\n", StringOrNullMarker(key.Namespace),
                        StringOrNullMarker(key.Name));
    }

    return err;
}

CHIP_ERROR AmebaConfig::ReadConfigValue(Key key, uint64_t & val)
{
    CHIP_ERROR err;
    int32_t error;

    error = getPref_u64_new(key.Namespace, key.Name, &val);
    err   = AmebaUtils::MapError(error, AmebaErrorType::kDctError);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(DeviceLayer, "getPref_u64: %s/%s failed\n", StringOrNullMarker(key.Namespace),
                        StringOrNullMarker(key.Name));
    }

    return err;
}

CHIP_ERROR AmebaConfig::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    CHIP_ERROR err;
    int32_t error;

    error = getPref_str_new(key.Namespace, key.Name, buf, bufSize, &outLen);
    err   = AmebaUtils::MapError(error, AmebaErrorType::kDctError);
    outLen -= 1; // Don't count trailing null
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(DeviceLayer, "getPref_str_new: %s/%s failed\n", StringOrNullMarker(key.Namespace),
                        StringOrNullMarker(key.Name));
        outLen = 0;
    }

    return err;
}

CHIP_ERROR AmebaConfig::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    CHIP_ERROR err;
    int32_t error;

    error = getPref_bin_new(key.Namespace, key.Name, buf, bufSize, &outLen);
    err   = AmebaUtils::MapError(error, AmebaErrorType::kDctError);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(DeviceLayer, "getPref_bin_new: %s/%s failed\n", StringOrNullMarker(key.Namespace),
                        StringOrNullMarker(key.Name));
        outLen = 0;
    }

    return err;
}

CHIP_ERROR AmebaConfig::WriteConfigValue(Key key, bool val)
{
    CHIP_ERROR err;
    int32_t error;
    uint8_t value;

    if (val == 1)
        value = 1;
    else
        value = 0;

    if (checkExist(key.Name, key.Name))
    {
        err = ClearConfigValue(key);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "Warning, KVS leakage, failed to remove old KVS value");
        }
    }

    error = setPref_new(key.Namespace, key.Name, &value, 1);
    err   = AmebaUtils::MapError(error, AmebaErrorType::kDctError);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "setPref: %s/%s = %s failed\n", StringOrNullMarker(key.Namespace), StringOrNullMarker(key.Name),
                     value ? "true" : "false");
    }
    else
    {
        ChipLogProgress(DeviceLayer, "NVS set: %s/%s = %s", StringOrNullMarker(key.Namespace), StringOrNullMarker(key.Name),
                        val ? "true" : "false");
    }

    return err;
}

CHIP_ERROR AmebaConfig::WriteConfigValue(Key key, uint32_t val)
{
    CHIP_ERROR err;
    int32_t error;

    if (checkExist(key.Name, key.Name))
    {
        err = ClearConfigValue(key);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "Warning, KVS leakage, failed to remove old KVS value");
        }
    }

    error = setPref_new(key.Namespace, key.Name, (uint8_t *) &val, sizeof(uint32_t));
    err   = AmebaUtils::MapError(error, AmebaErrorType::kDctError);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "setPref: %s/%s = %d(0x%x) failed\n", StringOrNullMarker(key.Namespace),
                     StringOrNullMarker(key.Name), val, val);
    }
    else
    {
        ChipLogProgress(DeviceLayer, "NVS set: %s/%s = %" PRIu32 " (0x%" PRIX32 ")", StringOrNullMarker(key.Namespace),
                        StringOrNullMarker(key.Name), val, val);
    }

    return err;
}

CHIP_ERROR AmebaConfig::WriteConfigValue(Key key, uint64_t val)
{
    CHIP_ERROR err;
    int32_t error;

    if (checkExist(key.Name, key.Name))
    {
        err = ClearConfigValue(key);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "Warning, KVS leakage, failed to remove old KVS value");
        }
    }

    error = setPref_new(key.Namespace, key.Name, (uint8_t *) &val, sizeof(uint64_t));
    err   = AmebaUtils::MapError(error, AmebaErrorType::kDctError);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "setPref: %s/%s = %d(0x%x) failed\n", StringOrNullMarker(key.Namespace),
                     StringOrNullMarker(key.Name), val, val);
    }
    else
    {
        ChipLogProgress(DeviceLayer, "NVS set: %s/%s = %" PRIu64 " (0x%" PRIX64 ")", StringOrNullMarker(key.Namespace),
                        StringOrNullMarker(key.Name), val, val);
    }

    return err;
}

CHIP_ERROR AmebaConfig::WriteConfigValueStr(Key key, const char * str)
{
    CHIP_ERROR err;
    int32_t error;

    if (checkExist(key.Name, key.Name))
    {
        err = ClearConfigValue(key);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "Warning, KVS leakage, failed to remove old KVS value");
        }
    }

    error = setPref_new(key.Namespace, key.Name, (uint8_t *) str, strlen(str) + 1);
    err   = AmebaUtils::MapError(error, AmebaErrorType::kDctError);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "setPref: %s/%s = %s failed\n", StringOrNullMarker(key.Namespace), StringOrNullMarker(key.Name),
                     StringOrNullMarker(str));
    }
    else
    {
        ChipLogProgress(DeviceLayer, "NVS set: %s/%s = \"%s\"", StringOrNullMarker(key.Namespace), StringOrNullMarker(key.Name),
                        StringOrNullMarker(str));
    }

    return err;
}

CHIP_ERROR AmebaConfig::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    CHIP_ERROR err;
    chip::Platform::ScopedMemoryBuffer<char> strCopy;

    if (str != NULL)
    {
        strCopy.Calloc(strLen + 1);
        VerifyOrExit(strCopy, err = CHIP_ERROR_NO_MEMORY);
        Platform::CopyString(strCopy.Get(), strLen + 1, str);
    }
    err = AmebaConfig::WriteConfigValueStr(key, strCopy.Get());
exit:
    return err;
}

CHIP_ERROR AmebaConfig::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    CHIP_ERROR err;
    int32_t error;

    if (checkExist(key.Name, key.Name))
    {
        err = ClearConfigValue(key);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "Warning, KVS leakage, failed to remove old KVS value");
        }
    }

    error = setPref_new(key.Namespace, key.Name, (uint8_t *) data, dataLen);
    err   = AmebaUtils::MapError(error, AmebaErrorType::kDctError);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "setPref: %s/%s failed\n", StringOrNullMarker(key.Namespace), StringOrNullMarker(key.Name));
    }
    else
    {
        ChipLogProgress(DeviceLayer, "NVS set: %s/%s = (blob length %" PRId32 ")", StringOrNullMarker(key.Namespace),
                        StringOrNullMarker(key.Name), dataLen);
    }

    return err;
}

CHIP_ERROR AmebaConfig::ClearConfigValue(Key key)
{
    CHIP_ERROR err;
    int32_t error;

    error = deleteKey(key.Namespace, key.Name);
    err   = AmebaUtils::MapError(error, AmebaErrorType::kDctError);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(DeviceLayer, "%s : %s/%s failed\n", __FUNCTION__, StringOrNullMarker(key.Namespace),
                        StringOrNullMarker(key.Name));
    }
    else
    {
        ChipLogProgress(DeviceLayer, "NVS erase: %s/%s", StringOrNullMarker(key.Namespace), StringOrNullMarker(key.Name));
    }

    return err;
}

bool AmebaConfig::ConfigValueExists(Key key)
{
    return checkExist(key.Namespace, key.Name);
}

CHIP_ERROR AmebaConfig::InitNamespace()
{
    CHIP_ERROR err;
    int32_t error;

    error = registerPref();
    err   = AmebaUtils::MapError(error, AmebaErrorType::kDctError);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "DCT modules registration failed");
        goto exit;
    }

    error = registerPref2();
    err   = AmebaUtils::MapError(error, AmebaErrorType::kDctError);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "DCT2 modules registration failed");
    }

exit:
    return err;
}

CHIP_ERROR AmebaConfig::ClearNamespace()
{
    CHIP_ERROR err;
    int32_t error;

    error = clearPref();
    err   = AmebaUtils::MapError(error, AmebaErrorType::kDctError);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "DCT modules unregistration failed\n");
        goto exit;
    }

    error = clearPref2();
    err   = AmebaUtils::MapError(error, AmebaErrorType::kDctError);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "DCT2 modules unregistration failed\n");
    }

exit:
    return err;
}

void AmebaConfig::RunConfigUnitTest() {}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
