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

#include <platform/ESP32/ESP32Config.h>

#include <lib/core/CHIPEncoding.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/ESP32/ESP32Utils.h>
#include <platform/ESP32/ScopedNvsHandle.h>

#include "nvs.h"
#include "nvs_flash.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

// *** CAUTION ***: Changing the names or namespaces of these values will *break* existing devices.

// NVS namespaces used to store device configuration information.
const char ESP32Config::kConfigNamespace_ChipFactory[]  = "chip-factory";
const char ESP32Config::kConfigNamespace_ChipConfig[]   = "chip-config";
const char ESP32Config::kConfigNamespace_ChipCounters[] = "chip-counters";

// Keys stored in the chip-factory namespace
const ESP32Config::Key ESP32Config::kConfigKey_SerialNum           = { kConfigNamespace_ChipFactory, "serial-num" };
const ESP32Config::Key ESP32Config::kConfigKey_MfrDeviceId         = { kConfigNamespace_ChipFactory, "device-id" };
const ESP32Config::Key ESP32Config::kConfigKey_MfrDeviceCert       = { kConfigNamespace_ChipFactory, "device-cert" };
const ESP32Config::Key ESP32Config::kConfigKey_MfrDeviceICACerts   = { kConfigNamespace_ChipFactory, "device-ca-certs" };
const ESP32Config::Key ESP32Config::kConfigKey_MfrDevicePrivateKey = { kConfigNamespace_ChipFactory, "device-key" };
const ESP32Config::Key ESP32Config::kConfigKey_ProductRevision     = { kConfigNamespace_ChipFactory, "product-rev" };
const ESP32Config::Key ESP32Config::kConfigKey_ManufacturingDate   = { kConfigNamespace_ChipFactory, "mfg-date" };
const ESP32Config::Key ESP32Config::kConfigKey_SetupPinCode        = { kConfigNamespace_ChipFactory, "pin-code" };
const ESP32Config::Key ESP32Config::kConfigKey_SetupDiscriminator  = { kConfigNamespace_ChipFactory, "discriminator" };

// Keys stored in the chip-config namespace
const ESP32Config::Key ESP32Config::kConfigKey_FabricId                    = { kConfigNamespace_ChipConfig, "fabric-id" };
const ESP32Config::Key ESP32Config::kConfigKey_ServiceConfig               = { kConfigNamespace_ChipConfig, "service-config" };
const ESP32Config::Key ESP32Config::kConfigKey_PairedAccountId             = { kConfigNamespace_ChipConfig, "account-id" };
const ESP32Config::Key ESP32Config::kConfigKey_ServiceId                   = { kConfigNamespace_ChipConfig, "service-id" };
const ESP32Config::Key ESP32Config::kConfigKey_GroupKeyIndex               = { kConfigNamespace_ChipConfig, "group-key-index" };
const ESP32Config::Key ESP32Config::kConfigKey_LastUsedEpochKeyId          = { kConfigNamespace_ChipConfig, "last-ek-id" };
const ESP32Config::Key ESP32Config::kConfigKey_FailSafeArmed               = { kConfigNamespace_ChipConfig, "fail-safe-armed" };
const ESP32Config::Key ESP32Config::kConfigKey_WiFiStationSecType          = { kConfigNamespace_ChipConfig, "sta-sec-type" };
const ESP32Config::Key ESP32Config::kConfigKey_OperationalDeviceId         = { kConfigNamespace_ChipConfig, "op-device-id" };
const ESP32Config::Key ESP32Config::kConfigKey_OperationalDeviceCert       = { kConfigNamespace_ChipConfig, "op-device-cert" };
const ESP32Config::Key ESP32Config::kConfigKey_OperationalDeviceICACerts   = { kConfigNamespace_ChipConfig, "op-device-ca-certs" };
const ESP32Config::Key ESP32Config::kConfigKey_OperationalDevicePrivateKey = { kConfigNamespace_ChipConfig, "op-device-key" };
const ESP32Config::Key ESP32Config::kConfigKey_RegulatoryLocation          = { kConfigNamespace_ChipConfig, "regulatory-location" };
const ESP32Config::Key ESP32Config::kConfigKey_CountryCode                 = { kConfigNamespace_ChipConfig, "country-code" };
const ESP32Config::Key ESP32Config::kConfigKey_Breadcrumb                  = { kConfigNamespace_ChipConfig, "breadcrumb" };

// Prefix used for NVS keys that contain Chip group encryption keys.
const char ESP32Config::kGroupKeyNamePrefix[] = "gk-";

CHIP_ERROR ESP32Config::ReadConfigValue(Key key, bool & val)
{
    ScopedNvsHandle handle;
    uint32_t intVal;

    ReturnErrorOnFailure(handle.Open(key.Namespace, NVS_READONLY));

    esp_err_t err = nvs_get_u32(handle, key.Name, &intVal);
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    ReturnMappedErrorOnFailure(err);

    val = (intVal != 0);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32Config::ReadConfigValue(Key key, uint32_t & val)
{
    ScopedNvsHandle handle;

    ReturnErrorOnFailure(handle.Open(key.Namespace, NVS_READONLY));

    esp_err_t err = nvs_get_u32(handle, key.Name, &val);
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    ReturnMappedErrorOnFailure(err);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32Config::ReadConfigValue(Key key, uint64_t & val)
{
    ScopedNvsHandle handle;

    ReturnErrorOnFailure(handle.Open(key.Namespace, NVS_READONLY));

    // Special case the MfrDeviceId value, optionally allowing it to be read as a blob containing
    // a 64-bit big-endian integer, instead of a u64 value.
    //
    // The ESP32 development environment provides a tool for pre-populating the NVS partition using
    // values from a CSV file.  This tool is convenient for provisioning devices during manufacturing.
    // However currently the tool does not support pre-populating u64 values such as MfrDeviceId.
    // Thus we allow MfrDeviceId to be stored as a blob instead.
    //
    if (key == kConfigKey_MfrDeviceId)
    {
        uint8_t deviceIdBytes[sizeof(uint64_t)];
        size_t deviceIdLen = sizeof(deviceIdBytes);
        esp_err_t err      = nvs_get_blob(handle, key.Name, deviceIdBytes, &deviceIdLen);
        if (err == ESP_OK)
        {
            VerifyOrReturnError(deviceIdLen == sizeof(deviceIdBytes), ESP32Utils::MapError(ESP_ERR_NVS_INVALID_LENGTH));
            val = Encoding::BigEndian::Get64(deviceIdBytes);
            return CHIP_NO_ERROR;
        }
    }

    esp_err_t err = nvs_get_u64(handle, key.Name, &val);
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    ReturnMappedErrorOnFailure(err);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32Config::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    ScopedNvsHandle handle;

    ReturnErrorOnFailure(handle.Open(key.Namespace, NVS_READONLY));

    outLen        = bufSize;
    esp_err_t err = nvs_get_str(handle, key.Name, buf, &outLen);
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        outLen = 0;
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    if (err == ESP_ERR_NVS_INVALID_LENGTH && buf != NULL)
    {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }
    ReturnMappedErrorOnFailure(err);

    outLen -= 1; // Don't count trailing nul.

    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32Config::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    ScopedNvsHandle handle;

    ReturnErrorOnFailure(handle.Open(key.Namespace, NVS_READONLY));

    outLen        = bufSize;
    esp_err_t err = nvs_get_blob(handle, key.Name, buf, &outLen);
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        outLen = 0;
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    else if (err == ESP_ERR_NVS_INVALID_LENGTH && buf != NULL)
    {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }
    ReturnMappedErrorOnFailure(err);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32Config::WriteConfigValue(Key key, bool val)
{
    ScopedNvsHandle handle;

    ReturnErrorOnFailure(handle.Open(key.Namespace, NVS_READWRITE));
    ReturnMappedErrorOnFailure(nvs_set_u32(handle, key.Name, val ? 1 : 0));

    // Commit the value to the persistent store.
    ReturnMappedErrorOnFailure(nvs_commit(handle));

    ChipLogProgress(DeviceLayer, "NVS set: %s/%s = %s", key.Namespace, key.Name, val ? "true" : "false");
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32Config::WriteConfigValue(Key key, uint32_t val)
{
    ScopedNvsHandle handle;

    ReturnErrorOnFailure(handle.Open(key.Namespace, NVS_READWRITE));
    ReturnMappedErrorOnFailure(nvs_set_u32(handle, key.Name, val));

    // Commit the value to the persistent store.
    ReturnMappedErrorOnFailure(nvs_commit(handle));

    ChipLogProgress(DeviceLayer, "NVS set: %s/%s = %" PRIu32 " (0x%" PRIX32 ")", key.Namespace, key.Name, val, val);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32Config::WriteConfigValue(Key key, uint64_t val)
{
    ScopedNvsHandle handle;

    ReturnErrorOnFailure(handle.Open(key.Namespace, NVS_READWRITE));
    ReturnMappedErrorOnFailure(nvs_set_u64(handle, key.Name, val));

    // Commit the value to the persistent store.
    ReturnMappedErrorOnFailure(nvs_commit(handle));

    ChipLogProgress(DeviceLayer, "NVS set: %s/%s = %" PRIu64 " (0x%" PRIX64 ")", key.Namespace, key.Name, val, val);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32Config::WriteConfigValueStr(Key key, const char * str)
{
    if (str != NULL)
    {
        ScopedNvsHandle handle;

        ReturnErrorOnFailure(handle.Open(key.Namespace, NVS_READWRITE));
        ReturnMappedErrorOnFailure(nvs_set_str(handle, key.Name, str));

        // Commit the value to the persistent store.
        ReturnMappedErrorOnFailure(nvs_commit(handle));

        ChipLogProgress(DeviceLayer, "NVS set: %s/%s = \"%s\"", key.Namespace, key.Name, str);
        return CHIP_NO_ERROR;
    }

    return ClearConfigValue(key);
}

CHIP_ERROR ESP32Config::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    chip::Platform::ScopedMemoryBuffer<char> strCopy;

    if (str != NULL)
    {
        strCopy.Calloc(strLen + 1);
        VerifyOrReturnError(strCopy, CHIP_ERROR_NO_MEMORY);
        strncpy(strCopy.Get(), str, strLen);
    }
    return ESP32Config::WriteConfigValueStr(key, strCopy.Get());
}

CHIP_ERROR ESP32Config::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    ScopedNvsHandle handle;

    if (data != NULL)
    {
        ReturnErrorOnFailure(handle.Open(key.Namespace, NVS_READWRITE));
        ReturnMappedErrorOnFailure(nvs_set_blob(handle, key.Name, data, dataLen));

        // Commit the value to the persistent store.
        ReturnMappedErrorOnFailure(nvs_commit(handle));

        ChipLogProgress(DeviceLayer, "NVS set: %s/%s = (blob length %" PRId32 ")", key.Namespace, key.Name, dataLen);
        return CHIP_NO_ERROR;
    }

    return ClearConfigValue(key);
}

CHIP_ERROR ESP32Config::ClearConfigValue(Key key)
{
    ScopedNvsHandle handle;

    ReturnErrorOnFailure(handle.Open(key.Namespace, NVS_READWRITE));

    esp_err_t err = nvs_erase_key(handle, key.Name);
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        return CHIP_NO_ERROR;
    }
    ReturnMappedErrorOnFailure(err);

    // Commit the value to the persistent store.
    ReturnMappedErrorOnFailure(nvs_commit(handle));

    ChipLogProgress(DeviceLayer, "NVS erase: %s/%s", key.Namespace, key.Name);
    return CHIP_NO_ERROR;
}

bool ESP32Config::ConfigValueExists(Key key)
{
    ScopedNvsHandle handle;

    if (handle.Open(key.Namespace, NVS_READONLY) != CHIP_NO_ERROR)
    {
        return false;
    }

    // This code is a rather unfortunate consequence of the limitations
    // in the ESP NVS API.  As defined, there is no API for determining
    // whether a particular key exists.  Furthermore, calling one of the
    // nvs_get_* APIs will result in a ESP_ERR_NVS_NOT_FOUND in the case
    // where the key exists, but the requested data type does not match.
    // (This is true despite the existence of the ESP_ERR_NVS_TYPE_MISMATCH
    // error, which would seem to be the obvious correct response).
    //
    // Thus the solution is to exhaustively check for the key using
    // each possible value type.
    esp_err_t err;
    {
        uint8_t v;
        err = nvs_get_u8(handle, key.Name, &v);
    }
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        int8_t v;
        err = nvs_get_i8(handle, key.Name, &v);
    }
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        uint16_t v;
        err = nvs_get_u16(handle, key.Name, &v);
    }
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        int16_t v;
        err = nvs_get_i16(handle, key.Name, &v);
    }
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        uint32_t v;
        err = nvs_get_u32(handle, key.Name, &v);
    }
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        int32_t v;
        err = nvs_get_i32(handle, key.Name, &v);
    }
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        uint64_t v;
        err = nvs_get_u64(handle, key.Name, &v);
    }
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        int64_t v;
        err = nvs_get_i64(handle, key.Name, &v);
    }
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        size_t sz;
        err = nvs_get_str(handle, key.Name, NULL, &sz);
    }
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        size_t sz;
        err = nvs_get_blob(handle, key.Name, NULL, &sz);
    }

    // In the case of blob and string, ESP_ERR_NVS_INVALID_LENGTH means
    // the key exists.
    if (err == ESP_ERR_NVS_INVALID_LENGTH)
    {
        err = ESP_OK;
    }

    return err == ESP_OK;
}

CHIP_ERROR ESP32Config::EnsureNamespace(const char * ns)
{
    ScopedNvsHandle handle;

    CHIP_ERROR err = handle.Open(ns, NVS_READONLY);
    if (err == CHIP_NO_ERROR)
    {
        return CHIP_NO_ERROR;
    }
    if (err == ESP32Utils::MapError(ESP_ERR_NVS_NOT_FOUND))
    {
        ReturnErrorOnFailure(handle.Open(ns, NVS_READWRITE));
        ReturnMappedErrorOnFailure(nvs_commit(handle));
        return CHIP_NO_ERROR;
    }
    return err;
}

CHIP_ERROR ESP32Config::ClearNamespace(const char * ns)
{
    ScopedNvsHandle handle;

    ReturnErrorOnFailure(handle.Open(ns, NVS_READWRITE));
    ReturnMappedErrorOnFailure(nvs_erase_all(handle));
    ReturnMappedErrorOnFailure(nvs_commit(handle));
    return CHIP_NO_ERROR;
}

void ESP32Config::RunConfigUnitTest() {}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
