/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
 *          Utilities for interacting with the the Open IoT SDK key-value storage.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <lib/core/CHIPEncoding.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

// *** CAUTION ***: Changing the names or namespaces of these values will *break* existing devices.

#define STR_EXPAND(tok) #tok

// Note: An external mbed parameter could be useful so an application can put
// chip NVS values in a single place
#define CHIP_CONFIG_KV_STORE_PARTITION STR_EXPAND(MBED_CONF_STORAGE_DEFAULT_KV)

// NVS namespaces used to store device configuration information.
#define CHIP_CONFIG_FACTORY_PREFIX "chip-factory-"
#define CHIP_CONFIG_CONFIG_PREFIX "chip-config-"
#define CHIP_CONFIG_COUNTER_PREFIX "chip-counters-"

#define FACTORY_KEY(key) CHIP_CONFIG_KV_STORE_PARTITION CHIP_CONFIG_FACTORY_PREFIX key
#define CONFIG_KEY(key) CHIP_CONFIG_KV_STORE_PARTITION CHIP_CONFIG_CONFIG_PREFIX key

const char OpenIoTSDKConfig::kConfigNamespace_ChipFactory[]  = CHIP_CONFIG_KV_STORE_PARTITION CHIP_CONFIG_FACTORY_PREFIX;
const char OpenIoTSDKConfig::kConfigNamespace_ChipConfig[]   = CHIP_CONFIG_KV_STORE_PARTITION CHIP_CONFIG_CONFIG_PREFIX;
const char OpenIoTSDKConfig::kConfigNamespace_ChipCounters[] = CHIP_CONFIG_KV_STORE_PARTITION CHIP_CONFIG_COUNTER_PREFIX;

// Keys stored in the chip-factory namespace
const OpenIoTSDKConfig::Key OpenIoTSDKConfig::kConfigKey_SerialNum             = { FACTORY_KEY("serial-num") };
const OpenIoTSDKConfig::Key OpenIoTSDKConfig::kConfigKey_MfrDeviceId           = { FACTORY_KEY("device-id") };
const OpenIoTSDKConfig::Key OpenIoTSDKConfig::kConfigKey_MfrDeviceCert         = { FACTORY_KEY("device-cert") };
const OpenIoTSDKConfig::Key OpenIoTSDKConfig::kConfigKey_MfrDeviceICACerts     = { FACTORY_KEY("device-ca-certs") };
const OpenIoTSDKConfig::Key OpenIoTSDKConfig::kConfigKey_MfrDevicePrivateKey   = { FACTORY_KEY("device-key") };
const OpenIoTSDKConfig::Key OpenIoTSDKConfig::kConfigKey_HardwareVersion       = { FACTORY_KEY("hardware-ver") };
const OpenIoTSDKConfig::Key OpenIoTSDKConfig::kConfigKey_ManufacturingDate     = { FACTORY_KEY("mfg-date") };
const OpenIoTSDKConfig::Key OpenIoTSDKConfig::kConfigKey_SetupPinCode          = { FACTORY_KEY("pin-code") };
const OpenIoTSDKConfig::Key OpenIoTSDKConfig::kConfigKey_SetupDiscriminator    = { FACTORY_KEY("discriminator") };
const OpenIoTSDKConfig::Key OpenIoTSDKConfig::kConfigKey_Spake2pIterationCount = { FACTORY_KEY("iteration-count") };
const OpenIoTSDKConfig::Key OpenIoTSDKConfig::kConfigKey_Spake2pSalt           = { FACTORY_KEY("salt") };
const OpenIoTSDKConfig::Key OpenIoTSDKConfig::kConfigKey_Spake2pVerifier       = { FACTORY_KEY("verifier") };

// Keys stored in the chip-config namespace
const OpenIoTSDKConfig::Key OpenIoTSDKConfig::kConfigKey_ServiceConfig      = { CONFIG_KEY("service-config") };
const OpenIoTSDKConfig::Key OpenIoTSDKConfig::kConfigKey_PairedAccountId    = { CONFIG_KEY("account-id") };
const OpenIoTSDKConfig::Key OpenIoTSDKConfig::kConfigKey_ServiceId          = { CONFIG_KEY("service-id") };
const OpenIoTSDKConfig::Key OpenIoTSDKConfig::kConfigKey_LastUsedEpochKeyId = { CONFIG_KEY("last-ek-id") };
const OpenIoTSDKConfig::Key OpenIoTSDKConfig::kConfigKey_FailSafeArmed      = { CONFIG_KEY("fail-safe-armed") };
const OpenIoTSDKConfig::Key OpenIoTSDKConfig::kConfigKey_WiFiStationSecType = { CONFIG_KEY("sta-sec-type") };
const OpenIoTSDKConfig::Key OpenIoTSDKConfig::kConfigKey_RegulatoryLocation = { CONFIG_KEY("regulatory-location") };
const OpenIoTSDKConfig::Key OpenIoTSDKConfig::kConfigKey_CountryCode        = { CONFIG_KEY("country-code") };
const OpenIoTSDKConfig::Key OpenIoTSDKConfig::kConfigKey_UniqueId           = { CONFIG_KEY("unique-id") };

using iotsdk::storage::kv_status;
using iotsdk::storage::KVStore;

CHIP_ERROR OpenIoTSDKConfig::ReadConfigValue(Key key, bool & val)
{
    if (Init() != CHIP_NO_ERROR)
    {
        return CHIP_ERROR_INTERNAL;
    }

    if (!ConfigValueExists(key))
    {
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }

    size_t actual_size = 0;
    kv_status err      = tdb->get(key, reinterpret_cast<void *>(&val), sizeof(val), &actual_size);

    if (err != kv_status::OK)
    {
        return CHIP_ERROR_INTERNAL;
    }

    if (actual_size != sizeof(val))
    {
        return CHIP_ERROR_BAD_REQUEST;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR OpenIoTSDKConfig::ReadConfigValue(Key key, uint32_t & val)
{
    if (Init() != CHIP_NO_ERROR)
    {
        return CHIP_ERROR_INTERNAL;
    }

    if (!ConfigValueExists(key))
    {
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }

    size_t actual_size = 0;
    kv_status err      = tdb->get(key, reinterpret_cast<void *>(&val), sizeof(val), &actual_size);

    if (err != kv_status::OK)
    {
        return CHIP_ERROR_INTERNAL;
    }

    if (actual_size != sizeof(val))
    {
        return CHIP_ERROR_BAD_REQUEST;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR OpenIoTSDKConfig::ReadConfigValue(Key key, uint64_t & val)
{
    if (Init() != CHIP_NO_ERROR)
    {
        return CHIP_ERROR_INTERNAL;
    }

    if (!ConfigValueExists(key))
    {
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }

    size_t actual_size = 0;
    kv_status err      = tdb->get(key, &val, sizeof(val), &actual_size);

    if (err != kv_status::OK)
    {
        return CHIP_ERROR_INTERNAL;
    }

    if (actual_size != sizeof(val))
    {
        return CHIP_ERROR_BAD_REQUEST;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR OpenIoTSDKConfig::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    CHIP_ERROR err = ReadConfigValueBin(key, reinterpret_cast<uint8_t *>(buf), bufSize, outLen);
    // Note: The system expect the trailing null to be added.
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }
    if (outLen >= bufSize)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    buf[outLen] = 0;
    return CHIP_NO_ERROR;
}

CHIP_ERROR OpenIoTSDKConfig::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    if (Init() != CHIP_NO_ERROR)
    {
        return CHIP_ERROR_INTERNAL;
    }

    if (!ConfigValueExists(key))
    {
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }

    KVStore::info_t info;

    kv_status err = tdb->get_info(key, &info);
    if (err != kv_status::OK)
    {
        return CHIP_ERROR_INTERNAL;
    }

    err = tdb->get(key, reinterpret_cast<void *>(buf), bufSize, &outLen);
    if (err != kv_status::OK)
    {
        return CHIP_ERROR_INTERNAL;
    }

    if (bufSize < info.size)
    {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR OpenIoTSDKConfig::WriteConfigValue(Key key, bool val)
{
    if (Init() != CHIP_NO_ERROR)
    {
        return CHIP_ERROR_INTERNAL;
    }

    kv_status err = tdb->set(key, reinterpret_cast<const void *>(&val), sizeof(val), 0);
    return err == kv_status::OK ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
}

CHIP_ERROR OpenIoTSDKConfig::WriteConfigValue(Key key, uint32_t val)
{
    if (Init() != CHIP_NO_ERROR)
    {
        return CHIP_ERROR_INTERNAL;
    }

    kv_status err = tdb->set(key, reinterpret_cast<const void *>(&val), sizeof(val), 0);
    return err == kv_status::OK ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
}

CHIP_ERROR OpenIoTSDKConfig::WriteConfigValue(Key key, uint64_t val)
{
    if (Init() != CHIP_NO_ERROR)
    {
        return CHIP_ERROR_INTERNAL;
    }

    kv_status err = tdb->set(key, reinterpret_cast<void *>(&val), sizeof(val), 0);
    return err == kv_status::OK ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
}

CHIP_ERROR OpenIoTSDKConfig::WriteConfigValueStr(Key key, const char * str)
{
    return WriteConfigValueBin(key, reinterpret_cast<const uint8_t *>(str), strlen(str));
}

CHIP_ERROR OpenIoTSDKConfig::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    return WriteConfigValueBin(key, reinterpret_cast<const uint8_t *>(str), strLen);
}

CHIP_ERROR OpenIoTSDKConfig::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    if (Init() != CHIP_NO_ERROR)
    {
        return CHIP_ERROR_INTERNAL;
    }

    // Two different behavior: If the pointer is not null, the value is updated
    // or create. If the pointer is null, the key is removed if it exist.
    if (data != nullptr)
    {
        kv_status err = tdb->set(key, reinterpret_cast<const void *>(data), dataLen, 0);
        return err == kv_status::OK ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
    }
    else if (ConfigValueExists(key))
    {
        return ClearConfigValue(key);
    }
    else
    {
        // Nothing to do, data is null and the key does not exist.
        return CHIP_NO_ERROR;
    }
}

CHIP_ERROR OpenIoTSDKConfig::ClearConfigValue(Key key)
{
    if (Init() != CHIP_NO_ERROR)
    {
        return CHIP_ERROR_INTERNAL;
    }

    kv_status err = tdb->remove(key);
    if (err == kv_status::ITEM_NOT_FOUND)
    {
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }
    return err == kv_status::OK ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
}

bool OpenIoTSDKConfig::ConfigValueExists(Key key)
{
    if (Init() != CHIP_NO_ERROR)
    {
        return false;
    }

    KVStore::info_t info;
    kv_status err = tdb->get_info(key, &info);
    return err == kv_status::OK ? true : false;
}

CHIP_ERROR OpenIoTSDKConfig::FactoryResetConfig()
{
    // tdb->reset is not used, we want to preserve other setting and factory
    // configuration
    auto err = ClearNamespace(kConfigNamespace_ChipConfig);
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }
    return ClearNamespace(kConfigNamespace_ChipCounters);
}

CHIP_ERROR OpenIoTSDKConfig::ConstructCounterKey(Key id, char * buf, size_t bufSize)
{
    auto length = snprintf(buf, bufSize - 1, CHIP_CONFIG_KV_STORE_PARTITION CHIP_CONFIG_COUNTER_PREFIX "%s", id);
    if (length < 0)
    {
        return CHIP_ERROR_INTERNAL;
    }
    else if ((size_t) length > (bufSize - 1))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    else
    {
        return CHIP_NO_ERROR;
    }
}

CHIP_ERROR OpenIoTSDKConfig::ReadCounter(Key counterId, uint32_t & value)
{
    char key[50] = { 0 };
    auto err     = ConstructCounterKey(counterId, key, sizeof(key));
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    return ReadConfigValue(key, value);
}

CHIP_ERROR OpenIoTSDKConfig::WriteCounter(Key counterId, uint32_t value)
{
    char key[50] = { 0 };
    auto err     = ConstructCounterKey(counterId, key, sizeof(key));
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    return WriteConfigValue(key, value);
}

CHIP_ERROR OpenIoTSDKConfig::ClearNamespace(const char * ns)
{
    if (Init() != CHIP_NO_ERROR)
    {
        return CHIP_ERROR_INTERNAL;
    }

    iotsdk::storage::KVStore::iterator_t it;
    char key[iotsdk::storage::KVStore::MAX_KEY_SIZE];
    kv_status err = tdb->iterator_open(&it, ns);
    if (err != kv_status::OK)
    {
        return CHIP_ERROR_INTERNAL;
    }

    while (true)
    {
        err = tdb->iterator_next(it, key, sizeof(key));
        if (err == kv_status::OK)
        {
            tdb->remove(key);
            memset(key, 0, sizeof(key));
        }
        else if (err == kv_status::ITEM_NOT_FOUND)
        {
            break;
        }
        else
        {
            (void) tdb->iterator_close(it);
            return CHIP_ERROR_INTERNAL;
        }
    }

    err = tdb->iterator_close(it);
    return err == kv_status::OK ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
}

void OpenIoTSDKConfig::RunConfigUnitTest()
{
    // Run common unit test.
    ::chip::DeviceLayer::Internal::RunConfigUnitTest<OpenIoTSDKConfig>();
}

CHIP_ERROR OpenIoTSDKConfig::Init(void)
{
    if (tdb)
    {
        return CHIP_NO_ERROR;
    }

    flash_bd = new iotsdk::storage::FlashIAPBlockDevice(get_ram_drive_instance(), 0, 0);

    if (!flash_bd)
    {
        return CHIP_ERROR_INTERNAL;
    }

    // Create a TDBStore using the underlying storage
    tdb = new iotsdk::storage::TDBStore(flash_bd);

    if (!tdb)
    {
        delete flash_bd;
        return CHIP_ERROR_INTERNAL;
    }

    // KVStore uses dual stage initialization so we can handle any errors
    // Call the `init` method to setup the TDBStore
    kv_status err = tdb->init();
    if (err != kv_status::OK)
    {
        delete flash_bd;
        delete tdb;
        // zero tdb as we use it keep track of init
        tdb = nullptr;
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

iotsdk::storage::TDBStore * OpenIoTSDKConfig::tdb                 = nullptr;
iotsdk::storage::FlashIAPBlockDevice * OpenIoTSDKConfig::flash_bd = nullptr;

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
