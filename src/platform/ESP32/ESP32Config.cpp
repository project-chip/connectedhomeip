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
const ESP32Config::Key ESP32Config::kConfigKey_SerialNum             = { kConfigNamespace_ChipFactory, "serial-num" };
const ESP32Config::Key ESP32Config::kConfigKey_MfrDeviceId           = { kConfigNamespace_ChipFactory, "device-id" };
const ESP32Config::Key ESP32Config::kConfigKey_MfrDeviceCert         = { kConfigNamespace_ChipFactory, "device-cert" };
const ESP32Config::Key ESP32Config::kConfigKey_MfrDeviceICACerts     = { kConfigNamespace_ChipFactory, "device-ca-certs" };
const ESP32Config::Key ESP32Config::kConfigKey_MfrDevicePrivateKey   = { kConfigNamespace_ChipFactory, "device-key" };
const ESP32Config::Key ESP32Config::kConfigKey_HardwareVersion       = { kConfigNamespace_ChipFactory, "hardware-ver" };
const ESP32Config::Key ESP32Config::kConfigKey_HardwareVersionString = { kConfigNamespace_ChipFactory, "hw-ver-str" };
const ESP32Config::Key ESP32Config::kConfigKey_ManufacturingDate     = { kConfigNamespace_ChipFactory, "mfg-date" };
const ESP32Config::Key ESP32Config::kConfigKey_SetupPinCode          = { kConfigNamespace_ChipFactory, "pin-code" };
const ESP32Config::Key ESP32Config::kConfigKey_SetupDiscriminator    = { kConfigNamespace_ChipFactory, "discriminator" };
const ESP32Config::Key ESP32Config::kConfigKey_Spake2pIterationCount = { kConfigNamespace_ChipFactory, "iteration-count" };
const ESP32Config::Key ESP32Config::kConfigKey_Spake2pSalt           = { kConfigNamespace_ChipFactory, "salt" };
const ESP32Config::Key ESP32Config::kConfigKey_Spake2pVerifier       = { kConfigNamespace_ChipFactory, "verifier" };
const ESP32Config::Key ESP32Config::kConfigKey_DACCert               = { kConfigNamespace_ChipFactory, "dac-cert" };
const ESP32Config::Key ESP32Config::kConfigKey_DACPrivateKey         = { kConfigNamespace_ChipFactory, "dac-key" };
const ESP32Config::Key ESP32Config::kConfigKey_DACPublicKey          = { kConfigNamespace_ChipFactory, "dac-pub-key" };
const ESP32Config::Key ESP32Config::kConfigKey_PAICert               = { kConfigNamespace_ChipFactory, "pai-cert" };
const ESP32Config::Key ESP32Config::kConfigKey_CertDeclaration       = { kConfigNamespace_ChipFactory, "cert-dclrn" };
const ESP32Config::Key ESP32Config::kConfigKey_VendorId              = { kConfigNamespace_ChipFactory, "vendor-id" };
const ESP32Config::Key ESP32Config::kConfigKey_VendorName            = { kConfigNamespace_ChipFactory, "vendor-name" };
const ESP32Config::Key ESP32Config::kConfigKey_ProductId             = { kConfigNamespace_ChipFactory, "product-id" };
const ESP32Config::Key ESP32Config::kConfigKey_ProductName           = { kConfigNamespace_ChipFactory, "product-name" };
const ESP32Config::Key ESP32Config::kConfigKey_ProductLabel          = { kConfigNamespace_ChipFactory, "product-label" };
const ESP32Config::Key ESP32Config::kConfigKey_ProductURL            = { kConfigNamespace_ChipFactory, "product-url" };
const ESP32Config::Key ESP32Config::kConfigKey_SupportedCalTypes     = { kConfigNamespace_ChipFactory, "cal-types" };
const ESP32Config::Key ESP32Config::kConfigKey_SupportedLocaleSize   = { kConfigNamespace_ChipFactory, "locale-sz" };
const ESP32Config::Key ESP32Config::kConfigKey_RotatingDevIdUniqueId = { kConfigNamespace_ChipFactory, "rd-id-uid" };
const ESP32Config::Key ESP32Config::kConfigKey_ProductFinish         = { kConfigNamespace_ChipFactory, "product-finish" };
const ESP32Config::Key ESP32Config::kConfigKey_ProductColor          = { kConfigNamespace_ChipFactory, "product-color" };
const ESP32Config::Key ESP32Config::kConfigKey_PartNumber            = { kConfigNamespace_ChipFactory, "part-number" };
const ESP32Config::Key ESP32Config::kConfigKey_LocationCapability    = { kConfigNamespace_ChipFactory, "loc-capability" };
const ESP32Config::Key ESP32Config::kConfigKey_PrimaryDeviceType     = { kConfigNamespace_ChipFactory, "device-type" };

// Keys stored in the chip-config namespace
const ESP32Config::Key ESP32Config::kConfigKey_ServiceConfig      = { kConfigNamespace_ChipConfig, "service-config" };
const ESP32Config::Key ESP32Config::kConfigKey_PairedAccountId    = { kConfigNamespace_ChipConfig, "account-id" };
const ESP32Config::Key ESP32Config::kConfigKey_ServiceId          = { kConfigNamespace_ChipConfig, "service-id" };
const ESP32Config::Key ESP32Config::kConfigKey_LastUsedEpochKeyId = { kConfigNamespace_ChipConfig, "last-ek-id" };
const ESP32Config::Key ESP32Config::kConfigKey_FailSafeArmed      = { kConfigNamespace_ChipConfig, "fail-safe-armed" };
const ESP32Config::Key ESP32Config::kConfigKey_RegulatoryLocation = { kConfigNamespace_ChipConfig, "reg-location" };
const ESP32Config::Key ESP32Config::kConfigKey_CountryCode        = { kConfigNamespace_ChipConfig, "country-code" };
const ESP32Config::Key ESP32Config::kConfigKey_UniqueId           = { kConfigNamespace_ChipConfig, "unique-id" };
const ESP32Config::Key ESP32Config::kConfigKey_LockUser           = { kConfigNamespace_ChipConfig, "lock-user" };
const ESP32Config::Key ESP32Config::kConfigKey_Credential         = { kConfigNamespace_ChipConfig, "credential" };
const ESP32Config::Key ESP32Config::kConfigKey_LockUserName       = { kConfigNamespace_ChipConfig, "lock-user-name" };
const ESP32Config::Key ESP32Config::kConfigKey_CredentialData     = { kConfigNamespace_ChipConfig, "credential-data" };
const ESP32Config::Key ESP32Config::kConfigKey_UserCredentials    = { kConfigNamespace_ChipConfig, "user-credential" };
const ESP32Config::Key ESP32Config::kConfigKey_WeekDaySchedules   = { kConfigNamespace_ChipConfig, "week-day-sched" };
const ESP32Config::Key ESP32Config::kConfigKey_YearDaySchedules   = { kConfigNamespace_ChipConfig, "year-day-sched" };
const ESP32Config::Key ESP32Config::kConfigKey_HolidaySchedules   = { kConfigNamespace_ChipConfig, "holiday-sched" };

// Keys stored in the Chip-counters namespace
const ESP32Config::Key ESP32Config::kCounterKey_RebootCount           = { kConfigNamespace_ChipCounters, "reboot-count" };
const ESP32Config::Key ESP32Config::kCounterKey_UpTime                = { kConfigNamespace_ChipCounters, "up-time" };
const ESP32Config::Key ESP32Config::kCounterKey_TotalOperationalHours = { kConfigNamespace_ChipCounters, "total-hours" };

const char * ESP32Config::GetPartitionLabelByNamespace(const char * ns)
{
    if (strcmp(ns, kConfigNamespace_ChipFactory) == 0)
    {
        return CHIP_DEVICE_CONFIG_CHIP_FACTORY_NAMESPACE_PARTITION;
    }
    else if (strcmp(ns, kConfigNamespace_ChipConfig) == 0)
    {
        return CHIP_DEVICE_CONFIG_CHIP_CONFIG_NAMESPACE_PARTITION;
    }
    else if (strcmp(ns, kConfigNamespace_ChipCounters) == 0)
    {
        return CHIP_DEVICE_CONFIG_CHIP_COUNTERS_NAMESPACE_PARTITION;
    }

    return NVS_DEFAULT_PART_NAME;
}

CHIP_ERROR ESP32Config::ReadConfigValue(Key key, bool & val)
{
    ScopedNvsHandle handle;
    uint32_t intVal;

    ReturnErrorOnFailure(handle.Open(key.Namespace, NVS_READONLY, GetPartitionLabelByNamespace(key.Namespace)));

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

    ReturnErrorOnFailure(handle.Open(key.Namespace, NVS_READONLY, GetPartitionLabelByNamespace(key.Namespace)));

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

    ReturnErrorOnFailure(handle.Open(key.Namespace, NVS_READONLY, GetPartitionLabelByNamespace(key.Namespace)));

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

    ReturnErrorOnFailure(handle.Open(key.Namespace, NVS_READONLY, GetPartitionLabelByNamespace(key.Namespace)));

    outLen = bufSize;

    // If buf is null, nvs_get_str() sets the outLen to required length to fit the string
    esp_err_t err = nvs_get_str(handle, key.Name, buf, &outLen);
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        outLen = 0;
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    if (buf != NULL)
    {
        if (err == ESP_ERR_NVS_INVALID_LENGTH)
        {
            return CHIP_ERROR_BUFFER_TOO_SMALL;
        }
        ReturnErrorCodeIf(buf[outLen - 1] != 0, CHIP_ERROR_INVALID_STRING_LENGTH);
        ReturnMappedErrorOnFailure(err);
    }

    outLen -= 1; // Don't count trailing nul.

    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32Config::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    ScopedNvsHandle handle;

    ReturnErrorOnFailure(handle.Open(key.Namespace, NVS_READONLY, GetPartitionLabelByNamespace(key.Namespace)));

    outLen = bufSize;
    // If buf is null, nvs_get_blob() sets the outLen to required length to fit the blob
    esp_err_t err = nvs_get_blob(handle, key.Name, buf, &outLen);
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        outLen = 0;
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    if (buf != NULL)
    {
        if (err == ESP_ERR_NVS_INVALID_LENGTH)
        {
            return CHIP_ERROR_BUFFER_TOO_SMALL;
        }
        ReturnMappedErrorOnFailure(err);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32Config::WriteConfigValue(Key key, bool val)
{
    ScopedNvsHandle handle;

    ReturnErrorOnFailure(handle.Open(key.Namespace, NVS_READWRITE, GetPartitionLabelByNamespace(key.Namespace)));
    ReturnMappedErrorOnFailure(nvs_set_u32(handle, key.Name, val ? 1 : 0));

    // Commit the value to the persistent store.
    ReturnMappedErrorOnFailure(nvs_commit(handle));

    ChipLogProgress(DeviceLayer, "NVS set: %s/%s = %s", StringOrNullMarker(key.Namespace), StringOrNullMarker(key.Name),
                    val ? "true" : "false");
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32Config::WriteConfigValue(Key key, uint32_t val)
{
    ScopedNvsHandle handle;

    ReturnErrorOnFailure(handle.Open(key.Namespace, NVS_READWRITE, GetPartitionLabelByNamespace(key.Namespace)));
    ReturnMappedErrorOnFailure(nvs_set_u32(handle, key.Name, val));

    // Commit the value to the persistent store.
    ReturnMappedErrorOnFailure(nvs_commit(handle));

    ChipLogProgress(DeviceLayer, "NVS set: %s/%s = %" PRIu32 " (0x%" PRIX32 ")", StringOrNullMarker(key.Namespace),
                    StringOrNullMarker(key.Name), val, val);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32Config::WriteConfigValue(Key key, uint64_t val)
{
    ScopedNvsHandle handle;

    ReturnErrorOnFailure(handle.Open(key.Namespace, NVS_READWRITE, GetPartitionLabelByNamespace(key.Namespace)));
    ReturnMappedErrorOnFailure(nvs_set_u64(handle, key.Name, val));

    // Commit the value to the persistent store.
    ReturnMappedErrorOnFailure(nvs_commit(handle));

    ChipLogProgress(DeviceLayer, "NVS set: %s/%s = %" PRIu64 " (0x%" PRIX64 ")", StringOrNullMarker(key.Namespace),
                    StringOrNullMarker(key.Name), val, val);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32Config::WriteConfigValueStr(Key key, const char * str)
{
    if (str != NULL)
    {
        ScopedNvsHandle handle;

        ReturnErrorOnFailure(handle.Open(key.Namespace, NVS_READWRITE, GetPartitionLabelByNamespace(key.Namespace)));
        ReturnMappedErrorOnFailure(nvs_set_str(handle, key.Name, str));

        // Commit the value to the persistent store.
        ReturnMappedErrorOnFailure(nvs_commit(handle));

        ChipLogProgress(DeviceLayer, "NVS set: %s/%s = \"%s\"", StringOrNullMarker(key.Namespace), StringOrNullMarker(key.Name),
                        str);
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
        Platform::CopyString(strCopy.Get(), strLen + 1, str);
    }
    return ESP32Config::WriteConfigValueStr(key, strCopy.Get());
}

CHIP_ERROR ESP32Config::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    ScopedNvsHandle handle;

    if (data != NULL)
    {
        ReturnErrorOnFailure(handle.Open(key.Namespace, NVS_READWRITE, GetPartitionLabelByNamespace(key.Namespace)));
        ReturnMappedErrorOnFailure(nvs_set_blob(handle, key.Name, data, dataLen));

        // Commit the value to the persistent store.
        ReturnMappedErrorOnFailure(nvs_commit(handle));

        ChipLogProgress(DeviceLayer, "NVS set: %s/%s = (blob length %u)", StringOrNullMarker(key.Namespace),
                        StringOrNullMarker(key.Name), dataLen);
        return CHIP_NO_ERROR;
    }

    return ClearConfigValue(key);
}

CHIP_ERROR ESP32Config::ClearConfigValue(Key key)
{
    ScopedNvsHandle handle;

    ReturnErrorOnFailure(handle.Open(key.Namespace, NVS_READWRITE, GetPartitionLabelByNamespace(key.Namespace)));

    esp_err_t err = nvs_erase_key(handle, key.Name);
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        return CHIP_NO_ERROR;
    }
    ReturnMappedErrorOnFailure(err);

    // Commit the value to the persistent store.
    ReturnMappedErrorOnFailure(nvs_commit(handle));

    ChipLogProgress(DeviceLayer, "NVS erase: %s/%s", StringOrNullMarker(key.Namespace), StringOrNullMarker(key.Name));
    return CHIP_NO_ERROR;
}

bool ESP32Config::ConfigValueExists(Key key)
{
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
    nvs_iterator_t iterator = NULL;
    esp_err_t err           = nvs_entry_find(GetPartitionLabelByNamespace(key.Namespace), key.Namespace, NVS_TYPE_ANY, &iterator);
    for (; iterator && err == ESP_OK; err = nvs_entry_next(&iterator))
#else
    nvs_iterator_t iterator = nvs_entry_find(GetPartitionLabelByNamespace(key.Namespace), key.Namespace, NVS_TYPE_ANY);
    for (; iterator; iterator = nvs_entry_next(iterator))
#endif
    {
        nvs_entry_info_t info;
        nvs_entry_info(iterator, &info);
        if (strcmp(info.key, key.Name) == 0)
        {
            nvs_release_iterator(iterator);
            return true;
        }
    }
    // if nvs_entry_find() or nvs_entry_next() returns NULL, then no need to release the iterator.
    return false;
}

CHIP_ERROR ESP32Config::EnsureNamespace(const char * ns)
{
    ScopedNvsHandle handle;

    CHIP_ERROR err = handle.Open(ns, NVS_READONLY, GetPartitionLabelByNamespace(ns));
    if (err == CHIP_NO_ERROR)
    {
        return CHIP_NO_ERROR;
    }
    if (err == ESP32Utils::MapError(ESP_ERR_NVS_NOT_FOUND))
    {
        ReturnErrorOnFailure(handle.Open(ns, NVS_READWRITE, GetPartitionLabelByNamespace(ns)));
        ReturnMappedErrorOnFailure(nvs_commit(handle));
        return CHIP_NO_ERROR;
    }
    return err;
}

CHIP_ERROR ESP32Config::ClearNamespace(const char * ns)
{
    ScopedNvsHandle handle;

    ReturnErrorOnFailure(handle.Open(ns, NVS_READWRITE, GetPartitionLabelByNamespace(ns)));
    ReturnMappedErrorOnFailure(nvs_erase_all(handle));
    ReturnMappedErrorOnFailure(nvs_commit(handle));
    return CHIP_NO_ERROR;
}

void ESP32Config::RunConfigUnitTest() {}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
