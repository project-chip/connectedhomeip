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
#include <platform/Linux/CHIPLinuxStorage.h>
#include <platform/Linux/FilesystemStorageLocationProvider.h>
#include <platform/Linux/PosixConfig.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

static ChipLinuxStorage gChipLinuxDataStorage;
static ChipLinuxStorage gChipLinuxFactoryStorage;
static ChipLinuxStorage gChipLinuxConfigStorage;
static ChipLinuxStorage gChipLinuxCountersStorage;

// *** CAUTION ***: Changing the names or namespaces of these values will *break* existing devices.

// NVS namespaces used to store device configuration information.
const char PosixConfig::kConfigNamespace_ChipFactory[]  = "chip-factory";
const char PosixConfig::kConfigNamespace_ChipConfig[]   = "chip-config";
const char PosixConfig::kConfigNamespace_ChipCounters[] = "chip-counters";
const char PosixConfig::kConfigNamespace_ChipKVS[]      = "chip-kvs";

// Keys stored in the Chip-KVS namespace
const PosixConfig::Key PosixConfig::kConfigKey_KVS = { kConfigNamespace_ChipKVS, "default" };

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
const PosixConfig::Key PosixConfig::kConfigKey_ServiceConfig        = { kConfigNamespace_ChipConfig, "service-config" };
const PosixConfig::Key PosixConfig::kConfigKey_PairedAccountId      = { kConfigNamespace_ChipConfig, "account-id" };
const PosixConfig::Key PosixConfig::kConfigKey_ServiceId            = { kConfigNamespace_ChipConfig, "service-id" };
const PosixConfig::Key PosixConfig::kConfigKey_LastUsedEpochKeyId   = { kConfigNamespace_ChipConfig, "last-ek-id" };
const PosixConfig::Key PosixConfig::kConfigKey_FailSafeArmed        = { kConfigNamespace_ChipConfig, "fail-safe-armed" };
const PosixConfig::Key PosixConfig::kConfigKey_RegulatoryLocation   = { kConfigNamespace_ChipConfig, "regulatory-location" };
const PosixConfig::Key PosixConfig::kConfigKey_CountryCode          = { kConfigNamespace_ChipConfig, "country-code" };
const PosixConfig::Key PosixConfig::kConfigKey_LocationCapability   = { kConfigNamespace_ChipConfig, "location-capability" };
const PosixConfig::Key PosixConfig::kConfigKey_ConfigurationVersion = { kConfigNamespace_ChipConfig, "configuration-version" };
const PosixConfig::Key PosixConfig::kConfigKey_UniqueId             = { kConfigNamespace_ChipConfig, "unique-id" };

// Keys stored in the Chip-counters namespace
const PosixConfig::Key PosixConfig::kCounterKey_RebootCount           = { kConfigNamespace_ChipCounters, "reboot-count" };
const PosixConfig::Key PosixConfig::kCounterKey_UpTime                = { kConfigNamespace_ChipCounters, "up-time" };
const PosixConfig::Key PosixConfig::kCounterKey_TotalOperationalHours = { kConfigNamespace_ChipCounters,
                                                                          "total-operational-hours" };
const PosixConfig::Key PosixConfig::kCounterKey_BootReason            = { kConfigNamespace_ChipCounters, "boot-reason" };

ChipLinuxStorage * PosixConfig::GetStorageForNamespace(Key key)
{
    if (strcmp(key.Namespace, kConfigNamespace_ChipKVS) == 0)
        return &gChipLinuxDataStorage;

    if (strcmp(key.Namespace, kConfigNamespace_ChipFactory) == 0)
        return &gChipLinuxFactoryStorage;

    if (strcmp(key.Namespace, kConfigNamespace_ChipConfig) == 0)
        return &gChipLinuxConfigStorage;

    if (strcmp(key.Namespace, kConfigNamespace_ChipCounters) == 0)
        return &gChipLinuxCountersStorage;

    return nullptr;
}

CHIP_ERROR PosixConfig::Init()
{
    std::string fn = chip::DeviceLayer::GetFilesystemStorageLocationProvider().GetKVSDataLocation();
    fn += "/";
    fn += CHIP_DEFAULT_DATA_FILENAME;
    return PersistedStorage::KeyValueStoreMgrImpl().Init(fn.c_str());
}

CHIP_ERROR PosixConfig::ReadConfigValue(Key key, bool & val)
{
    CHIP_ERROR err;
    ChipLinuxStorage * storage;
    uint32_t intVal;

    storage = GetStorageForNamespace(key);
    VerifyOrExit(storage != nullptr, err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND);

    err = storage->ReadValue(key.Name, intVal);
    if (err == CHIP_ERROR_KEY_NOT_FOUND)
    {
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    SuccessOrExit(err);

    val = (intVal != 0);

exit:
    return err;
}

CHIP_ERROR PosixConfig::ReadConfigValue(Key key, uint16_t & val)
{
    CHIP_ERROR err;
    ChipLinuxStorage * storage;

    storage = GetStorageForNamespace(key);
    VerifyOrExit(storage != nullptr, err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND);

    err = storage->ReadValue(key.Name, val);
    if (err == CHIP_ERROR_KEY_NOT_FOUND)
    {
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR PosixConfig::ReadConfigValue(Key key, uint32_t & val)
{
    CHIP_ERROR err;
    ChipLinuxStorage * storage;

    storage = GetStorageForNamespace(key);
    VerifyOrExit(storage != nullptr, err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND);

    err = storage->ReadValue(key.Name, val);
    if (err == CHIP_ERROR_KEY_NOT_FOUND)
    {
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR PosixConfig::ReadConfigValue(Key key, uint64_t & val)
{
    CHIP_ERROR err;
    ChipLinuxStorage * storage;

    storage = GetStorageForNamespace(key);
    VerifyOrExit(storage != nullptr, err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND);

    // Special case the MfrDeviceId value, optionally allowing it to be read as a blob containing
    // a 64-bit big-endian integer, instead of a u64 value.
    if (key == kConfigKey_MfrDeviceId)
    {
        uint8_t deviceIdBytes[sizeof(uint64_t)];
        size_t deviceIdLen = sizeof(deviceIdBytes);
        size_t deviceIdOutLen;
        err = storage->ReadValueBin(key.Name, deviceIdBytes, deviceIdLen, deviceIdOutLen);
        if (err == CHIP_NO_ERROR)
        {
            VerifyOrExit(deviceIdOutLen == sizeof(deviceIdBytes), err = CHIP_ERROR_INCORRECT_STATE);
            val = Encoding::BigEndian::Get64(deviceIdBytes);
            ExitNow();
        }
    }

    err = storage->ReadValue(key.Name, val);
    if (err == CHIP_ERROR_KEY_NOT_FOUND)
    {
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR PosixConfig::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    CHIP_ERROR err;
    ChipLinuxStorage * storage;

    storage = GetStorageForNamespace(key);
    VerifyOrExit(storage != nullptr, err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND);

    err = storage->ReadValueStr(key.Name, buf, bufSize, outLen);
    if (err == CHIP_ERROR_KEY_NOT_FOUND && key == kConfigKey_UniqueId)
    {
        // Special case for UniqueId, which used to be (erroneously) stored in the Factory namespace.
        // If it is not found in the Config namespace, try reading from Factory. If we find it there,
        // we will copy it to the Config namespace and write an empty value to the Factory namespace.
        err = gChipLinuxFactoryStorage.ReadValueStr(kConfigKey_UniqueId.Name, buf, bufSize, outLen);
        if (err == CHIP_NO_ERROR && outLen > 0)
        {
            // If any of these steps fail, ignore the error and just return the existing
            // err == CHIP_NO_ERROR, since we successfully returned the UniqueId from Factory.
            SuccessOrExit(/* ignored = */ storage->WriteValueStr(kConfigKey_UniqueId.Name, buf));
            SuccessOrExit(/* ignored = */ storage->Commit());
            SuccessOrExit(/* ignored = */ gChipLinuxFactoryStorage.WriteValueStr(kConfigKey_UniqueId.Name, ""));
            SuccessOrExit(/* ignored = */ gChipLinuxFactoryStorage.Commit());
            ChipLogProgress(DeviceLayer, "NVS migrated %s from %s to %s namespace", key.Name, kConfigNamespace_ChipFactory,
                            key.Namespace);
        }
    }
    if (err == CHIP_ERROR_KEY_NOT_FOUND)
    {
        outLen = 0;
        err    = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    else if (err == CHIP_ERROR_BUFFER_TOO_SMALL)
    {
        err = (buf == nullptr) ? CHIP_NO_ERROR : CHIP_ERROR_BUFFER_TOO_SMALL;
    }

exit:
    return err;
}

CHIP_ERROR PosixConfig::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    CHIP_ERROR err;
    ChipLinuxStorage * storage;

    storage = GetStorageForNamespace(key);
    VerifyOrExit(storage != nullptr, err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND);

    err = storage->ReadValueBin(key.Name, buf, bufSize, outLen);
    if (err == CHIP_ERROR_KEY_NOT_FOUND)
    {
        outLen = 0;
        err    = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    else if (err == CHIP_ERROR_BUFFER_TOO_SMALL)
    {
        err = (buf == nullptr) ? CHIP_NO_ERROR : CHIP_ERROR_BUFFER_TOO_SMALL;
    }
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR PosixConfig::WriteConfigValue(Key key, bool val)
{
    CHIP_ERROR err;
    ChipLinuxStorage * storage;

    storage = GetStorageForNamespace(key);
    VerifyOrExit(storage != nullptr, err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND);

    err = storage->WriteValue(key.Name, val);
    SuccessOrExit(err);

    // Commit the value to the persistent store.
    err = storage->Commit();
    SuccessOrExit(err);

    ChipLogProgress(DeviceLayer, "NVS set: %s/%s = %s", StringOrNullMarker(key.Namespace), StringOrNullMarker(key.Name),
                    val ? "true" : "false");

exit:
    return err;
}

CHIP_ERROR PosixConfig::WriteConfigValue(Key key, uint16_t val)
{
    CHIP_ERROR err;
    ChipLinuxStorage * storage;

    storage = GetStorageForNamespace(key);
    VerifyOrExit(storage != nullptr, err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND);

    err = storage->WriteValue(key.Name, val);
    SuccessOrExit(err);

    // Commit the value to the persistent store.
    err = storage->Commit();
    SuccessOrExit(err);

    ChipLogProgress(DeviceLayer, "NVS set: %s/%s = %u (0x%X)", StringOrNullMarker(key.Namespace), StringOrNullMarker(key.Name), val,
                    val);

exit:
    return err;
}

CHIP_ERROR PosixConfig::WriteConfigValue(Key key, uint32_t val)
{
    CHIP_ERROR err;
    ChipLinuxStorage * storage;

    storage = GetStorageForNamespace(key);
    VerifyOrExit(storage != nullptr, err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND);

    err = storage->WriteValue(key.Name, val);
    SuccessOrExit(err);

    // Commit the value to the persistent store.
    err = storage->Commit();
    SuccessOrExit(err);

    ChipLogProgress(DeviceLayer, "NVS set: %s/%s = %" PRIu32 " (0x%" PRIX32 ")", StringOrNullMarker(key.Namespace),
                    StringOrNullMarker(key.Name), val, val);

exit:
    return err;
}

CHIP_ERROR PosixConfig::WriteConfigValue(Key key, uint64_t val)
{
    CHIP_ERROR err;
    ChipLinuxStorage * storage;

    storage = GetStorageForNamespace(key);
    VerifyOrExit(storage != nullptr, err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND);

    err = storage->WriteValue(key.Name, val);
    SuccessOrExit(err);

    // Commit the value to the persistent store.
    err = storage->Commit();
    SuccessOrExit(err);

    ChipLogProgress(DeviceLayer, "NVS set: %s/%s = %" PRIu64 " (0x%" PRIX64 ")", StringOrNullMarker(key.Namespace),
                    StringOrNullMarker(key.Name), val, val);

exit:
    return err;
}

CHIP_ERROR PosixConfig::WriteConfigValueStr(Key key, const char * str)
{
    CHIP_ERROR err;
    ChipLinuxStorage * storage;

    if (str != nullptr)
    {
        storage = GetStorageForNamespace(key);
        VerifyOrExit(storage != nullptr, err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND);

        err = storage->WriteValueStr(key.Name, str);
        SuccessOrExit(err);

        // Commit the value to the persistent store.
        err = storage->Commit();
        SuccessOrExit(err);

        ChipLogProgress(DeviceLayer, "NVS set: %s/%s = \"%s\"", StringOrNullMarker(key.Namespace), StringOrNullMarker(key.Name),
                        str);
    }

    else
    {
        err = ClearConfigValue(key);
        SuccessOrExit(err);
    }

exit:
    return err;
}

CHIP_ERROR PosixConfig::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
#if CHIP_CONFIG_MEMORY_MGMT_MALLOC
    CHIP_ERROR err;
    char * strCopy = nullptr;

    if (str != nullptr)
    {
        strCopy = strndup(str, strLen);
        VerifyOrExit(strCopy != nullptr, err = CHIP_ERROR_NO_MEMORY);
    }

    err = PosixConfig::WriteConfigValueStr(key, strCopy);

exit:
    if (strCopy != nullptr)
    {
        free(strCopy);
    }
    return err;
#else
#error "Unsupported CHIP_CONFIG_MEMORY_MGMT configuration"
#endif
}

CHIP_ERROR PosixConfig::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    CHIP_ERROR err;
    ChipLinuxStorage * storage;

    if (data != nullptr)
    {
        storage = GetStorageForNamespace(key);
        VerifyOrExit(storage != nullptr, err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND);

        err = storage->WriteValueBin(key.Name, data, dataLen);
        SuccessOrExit(err);

        // Commit the value to the persistent store.
        err = storage->Commit();
        SuccessOrExit(err);

        ChipLogProgress(DeviceLayer, "NVS set: %s/%s = (blob length %u)", StringOrNullMarker(key.Namespace),
                        StringOrNullMarker(key.Name), static_cast<unsigned int>(dataLen));
    }
    else
    {
        err = ClearConfigValue(key);
        SuccessOrExit(err);
    }

exit:
    return err;
}

CHIP_ERROR PosixConfig::ClearConfigValue(Key key)
{
    CHIP_ERROR err;
    ChipLinuxStorage * storage;

    storage = GetStorageForNamespace(key);
    VerifyOrExit(storage != nullptr, err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND);

    err = storage->ClearValue(key.Name);
    if (err == CHIP_ERROR_KEY_NOT_FOUND)
    {
        ExitNow(err = CHIP_NO_ERROR);
    }
    SuccessOrExit(err);

    // Commit the value to the persistent store.
    err = storage->Commit();
    SuccessOrExit(err);

    ChipLogProgress(DeviceLayer, "NVS erase: %s/%s", StringOrNullMarker(key.Namespace), StringOrNullMarker(key.Name));

exit:
    return err;
}

bool PosixConfig::ConfigValueExists(Key key)
{
    ChipLinuxStorage * storage;

    storage = GetStorageForNamespace(key);
    if (storage == nullptr)
        return false;

    return storage->HasValue(key.Name);
}

CHIP_ERROR PosixConfig::EnsureNamespace(const char * ns)
{
    CHIP_ERROR err             = CHIP_NO_ERROR;
    ChipLinuxStorage * storage = nullptr;

    if (strcmp(ns, kConfigNamespace_ChipFactory) == 0)
    {
        storage        = &gChipLinuxFactoryStorage;
        std::string fn = chip::DeviceLayer::GetFilesystemStorageLocationProvider().GetFactoryDataLocation();
        fn += "/chip_factory.ini";
        err = storage->Init(fn.c_str());
    }
    else if (strcmp(ns, kConfigNamespace_ChipConfig) == 0)
    {
        storage        = &gChipLinuxConfigStorage;
        std::string fn = chip::DeviceLayer::GetFilesystemStorageLocationProvider().GetConfigDataLocation();
        fn += "/chip_config.ini";
        err = storage->Init(fn.c_str());
    }
    else if (strcmp(ns, kConfigNamespace_ChipCounters) == 0)
    {
        storage        = &gChipLinuxCountersStorage;
        std::string fn = chip::DeviceLayer::GetFilesystemStorageLocationProvider().GetCountersDataLocation();
        fn += "/chip_counters.ini";
        err = storage->Init(fn.c_str());
    }

    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR PosixConfig::ClearNamespace(const char * ns)
{
    CHIP_ERROR err             = CHIP_NO_ERROR;
    ChipLinuxStorage * storage = nullptr;

    if (strcmp(ns, kConfigNamespace_ChipConfig) == 0)
    {
        storage = &gChipLinuxConfigStorage;
    }
    else if (strcmp(ns, kConfigNamespace_ChipCounters) == 0)
    {
        storage = &gChipLinuxCountersStorage;
    }

    VerifyOrExit(storage != nullptr, err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND);

    err = storage->ClearAll();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Storage ClearAll failed: %" CHIP_ERROR_FORMAT, err.Format());
    }
    SuccessOrExit(err);

    err = storage->Commit();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Storage Commit failed: %" CHIP_ERROR_FORMAT, err.Format());
    }

exit:
    return err;
}

CHIP_ERROR PosixConfig::FactoryResetConfig()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ChipLinuxStorage * storage;

    ChipLogProgress(DeviceLayer, "Performing factory reset configuration");

    storage = &gChipLinuxConfigStorage;
    if (storage == nullptr)
    {
        ChipLogError(DeviceLayer, "Storage get failed");
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    SuccessOrExit(err);

    err = storage->ClearAll();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Storage ClearAll failed: %" CHIP_ERROR_FORMAT, err.Format());
    }
    SuccessOrExit(err);

    err = storage->Commit();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Storage Commit failed: %" CHIP_ERROR_FORMAT, err.Format());
    }

exit:
    return err;
}

CHIP_ERROR PosixConfig::FactoryResetCounters()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ChipLinuxStorage * storage;

    ChipLogProgress(DeviceLayer, "Performing factory reset counters");

    storage = &gChipLinuxCountersStorage;
    if (storage == nullptr)
    {
        ChipLogError(DeviceLayer, "Storage get failed");
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    SuccessOrExit(err);

    err = storage->ClearAll();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Storage ClearAll failed: %" CHIP_ERROR_FORMAT, err.Format());
    }
    SuccessOrExit(err);

    err = storage->Commit();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Storage Commit failed: %" CHIP_ERROR_FORMAT, err.Format());
    }

exit:
    return err;
}

void PosixConfig::RunConfigUnitTest()
{
    // Run common unit test.
    ::chip::DeviceLayer::Internal::RunConfigUnitTest<PosixConfig>();
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
