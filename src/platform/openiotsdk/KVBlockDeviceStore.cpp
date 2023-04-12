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
 *          Open IoT SDK key-value storage base on flash TDBStore.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <lib/core/CHIPEncoding.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include <platform/openiotsdk/KVBlockDeviceStore.h>
#include <platform/openiotsdk/OpenIoTSDKPort.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

// *** CAUTION ***: Changing the names or namespaces of these values will *break* existing devices.

// NVS namespaces used to store device configuration information.
#define CHIP_CONFIG_FACTORY_PREFIX "chip-factory-"
#define CHIP_CONFIG_CONFIG_PREFIX "chip-config-"
#define CHIP_CONFIG_COUNTER_PREFIX "chip-counters-"

#define FACTORY_KEY(key) CHIP_CONFIG_FACTORY_PREFIX key
#define CONFIG_KEY(key) CHIP_CONFIG_CONFIG_PREFIX key
#define COUNTER_KEY(key) CHIP_CONFIG_COUNTER_PREFIX key

// Keys stored in the chip-factory namespace
const KVBlockDeviceStore::Key KVBlockDeviceStore::kConfigKey_SerialNum             = { FACTORY_KEY("serial-num") };
const KVBlockDeviceStore::Key KVBlockDeviceStore::kConfigKey_MfrDeviceId           = { FACTORY_KEY("device-id") };
const KVBlockDeviceStore::Key KVBlockDeviceStore::kConfigKey_MfrDeviceCert         = { FACTORY_KEY("device-cert") };
const KVBlockDeviceStore::Key KVBlockDeviceStore::kConfigKey_MfrDeviceICACerts     = { FACTORY_KEY("device-ca-certs") };
const KVBlockDeviceStore::Key KVBlockDeviceStore::kConfigKey_MfrDevicePrivateKey   = { FACTORY_KEY("device-key") };
const KVBlockDeviceStore::Key KVBlockDeviceStore::kConfigKey_HardwareVersion       = { FACTORY_KEY("hardware-ver") };
const KVBlockDeviceStore::Key KVBlockDeviceStore::kConfigKey_ManufacturingDate     = { FACTORY_KEY("mfg-date") };
const KVBlockDeviceStore::Key KVBlockDeviceStore::kConfigKey_SetupPinCode          = { FACTORY_KEY("pin-code") };
const KVBlockDeviceStore::Key KVBlockDeviceStore::kConfigKey_SetupDiscriminator    = { FACTORY_KEY("discriminator") };
const KVBlockDeviceStore::Key KVBlockDeviceStore::kConfigKey_Spake2pIterationCount = { FACTORY_KEY("iteration-count") };
const KVBlockDeviceStore::Key KVBlockDeviceStore::kConfigKey_Spake2pSalt           = { FACTORY_KEY("salt") };
const KVBlockDeviceStore::Key KVBlockDeviceStore::kConfigKey_Spake2pVerifier       = { FACTORY_KEY("verifier") };
const KVBlockDeviceStore::Key KVBlockDeviceStore::kConfigKey_VendorId              = { FACTORY_KEY("vendor-id") };
const KVBlockDeviceStore::Key KVBlockDeviceStore::kConfigKey_ProductId             = { FACTORY_KEY("product-id") };

// Keys stored in the chip-config namespace
const KVBlockDeviceStore::Key KVBlockDeviceStore::kConfigKey_ServiceConfig      = { CONFIG_KEY("service-config") };
const KVBlockDeviceStore::Key KVBlockDeviceStore::kConfigKey_PairedAccountId    = { CONFIG_KEY("account-id") };
const KVBlockDeviceStore::Key KVBlockDeviceStore::kConfigKey_ServiceId          = { CONFIG_KEY("service-id") };
const KVBlockDeviceStore::Key KVBlockDeviceStore::kConfigKey_LastUsedEpochKeyId = { CONFIG_KEY("last-ek-id") };
const KVBlockDeviceStore::Key KVBlockDeviceStore::kConfigKey_FailSafeArmed      = { CONFIG_KEY("fail-safe-armed") };
const KVBlockDeviceStore::Key KVBlockDeviceStore::kConfigKey_WiFiStationSecType = { CONFIG_KEY("sta-sec-type") };
const KVBlockDeviceStore::Key KVBlockDeviceStore::kConfigKey_RegulatoryLocation = { CONFIG_KEY("regulatory-location") };
const KVBlockDeviceStore::Key KVBlockDeviceStore::kConfigKey_CountryCode        = { CONFIG_KEY("country-code") };
const KVBlockDeviceStore::Key KVBlockDeviceStore::kConfigKey_LocationCapability = { CONFIG_KEY("location-capability") };
const KVBlockDeviceStore::Key KVBlockDeviceStore::kConfigKey_UniqueId           = { CONFIG_KEY("unique-id") };

// Keys stored in the Chip-counters namespace
const KVBlockDeviceStore::Key KVBlockDeviceStore::kCounterKey_RebootCount           = { COUNTER_KEY("reboot-count") };
const KVBlockDeviceStore::Key KVBlockDeviceStore::kCounterKey_UpTime                = { COUNTER_KEY("up-time") };
const KVBlockDeviceStore::Key KVBlockDeviceStore::kCounterKey_TotalOperationalHours = { COUNTER_KEY("total-operational-hours") };
const KVBlockDeviceStore::Key KVBlockDeviceStore::kCounterKey_BootReason            = { COUNTER_KEY("boot-reason") };

using iotsdk::storage::kv_status;
using iotsdk::storage::KVStore;

iotsdk::storage::TDBStore * KVBlockDeviceStore::tdb = nullptr;

CHIP_ERROR KVBlockDeviceStore::Init(void)
{
    if (tdb)
    {
        return CHIP_NO_ERROR;
    }

    // Create a TDBStore using the underlying storage
    tdb = new iotsdk::storage::TDBStore(GetBlockDevice());

    if (!tdb)
    {
        return CHIP_ERROR_INTERNAL;
    }

    // KVStore uses dual stage initialization so we can handle any errors
    // Call the `init` method to setup the TDBStore
    kv_status err = tdb->init();
    if (err != kv_status::OK)
    {
        delete tdb;
        // zero tdb as we use it keep track of init
        tdb = nullptr;
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR KVBlockDeviceStore::ReadConfigValue(Key key, bool & val)
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

CHIP_ERROR KVBlockDeviceStore::ReadConfigValue(Key key, uint32_t & val)
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

CHIP_ERROR KVBlockDeviceStore::ReadConfigValue(Key key, uint64_t & val)
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

CHIP_ERROR KVBlockDeviceStore::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
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

CHIP_ERROR KVBlockDeviceStore::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    return ReadConfigValueBin(key, buf, bufSize, outLen, 0);
}

CHIP_ERROR KVBlockDeviceStore::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen, size_t offset)
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

    err = tdb->get(key, reinterpret_cast<void *>(buf), bufSize, &outLen, offset);
    if (err != kv_status::OK)
    {
        return CHIP_ERROR_INTERNAL;
    }

    if (bufSize < info.size - offset)
    {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR KVBlockDeviceStore::ReadConfigValueCounter(Key counterId, uint32_t & value)
{
    char key[50] = { 0 };
    auto err     = ConstructCounterKey(counterId, key, sizeof(key));
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    return ReadConfigValue(key, value);
}

CHIP_ERROR KVBlockDeviceStore::WriteConfigValue(Key key, bool val)
{
    if (Init() != CHIP_NO_ERROR)
    {
        return CHIP_ERROR_INTERNAL;
    }

    kv_status err = tdb->set(key, reinterpret_cast<const void *>(&val), sizeof(val), 0);
    return err == kv_status::OK ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
}

CHIP_ERROR KVBlockDeviceStore::WriteConfigValue(Key key, uint32_t val)
{
    if (Init() != CHIP_NO_ERROR)
    {
        return CHIP_ERROR_INTERNAL;
    }

    kv_status err = tdb->set(key, reinterpret_cast<const void *>(&val), sizeof(val), 0);
    return err == kv_status::OK ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
}

CHIP_ERROR KVBlockDeviceStore::WriteConfigValue(Key key, uint64_t val)
{
    if (Init() != CHIP_NO_ERROR)
    {
        return CHIP_ERROR_INTERNAL;
    }

    kv_status err = tdb->set(key, reinterpret_cast<void *>(&val), sizeof(val), 0);
    return err == kv_status::OK ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
}

CHIP_ERROR KVBlockDeviceStore::WriteConfigValueStr(Key key, const char * str)
{
    return WriteConfigValueBin(key, reinterpret_cast<const uint8_t *>(str), strlen(str));
}

CHIP_ERROR KVBlockDeviceStore::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    return WriteConfigValueBin(key, reinterpret_cast<const uint8_t *>(str), strLen);
}

CHIP_ERROR KVBlockDeviceStore::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
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

CHIP_ERROR KVBlockDeviceStore::WriteConfigValueCounter(Key counterId, uint32_t value)
{
    char key[50] = { 0 };
    auto err     = ConstructCounterKey(counterId, key, sizeof(key));
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    return WriteConfigValue(key, value);
}

CHIP_ERROR KVBlockDeviceStore::ClearConfigValue(Key key)
{
    if (Init() != CHIP_NO_ERROR)
    {
        return CHIP_ERROR_INTERNAL;
    }

    kv_status err = tdb->remove(key);
    if (err == kv_status::ITEM_NOT_FOUND)
    {
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    return err == kv_status::OK ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
}

CHIP_ERROR KVBlockDeviceStore::FactoryResetConfig()
{
    // tdb->reset is not used, we want to preserve other setting and factory
    // configuration
    auto err = ClearNamespace(CHIP_CONFIG_CONFIG_PREFIX);
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }
    return ClearNamespace(CHIP_CONFIG_COUNTER_PREFIX);
}

void KVBlockDeviceStore::RunConfigUnitTest()
{
    // Run common unit test.
    ::chip::DeviceLayer::Internal::RunConfigUnitTest<KVBlockDeviceStore>();
}

bool KVBlockDeviceStore::ConfigValueExists(Key key)
{
    if (Init() != CHIP_NO_ERROR)
    {
        return false;
    }

    KVStore::info_t info;
    kv_status err = tdb->get_info(key, &info);
    return err == kv_status::OK ? true : false;
}

CHIP_ERROR KVBlockDeviceStore::ConstructCounterKey(Key id, char * buf, size_t bufSize)
{
    auto length = snprintf(buf, bufSize - 1, CHIP_CONFIG_COUNTER_PREFIX "%s", id);
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

CHIP_ERROR KVBlockDeviceStore::ClearNamespace(const char * ns)
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

KVBlockDeviceStoreKeyBuilder::KVBlockDeviceStoreKeyBuilder(const char * key)
{
    // Check sign by sign if key contains illegal characters
    // Each illegal character will be replaced by '!' + capital encoded letter value
    char * out = buffer + strlen(buffer);
    char * illegal_ptr;
    while ((out < buffer + sizeof(buffer) - 3) && *key) // 2 chars for potential illegal char + 1 for \0
    {
        illegal_ptr = strchr(illegalCharacters, *key);
        if (illegal_ptr)
        {
            *out++ = '!';
            *out++ = static_cast<char>('A' + (int) (illegal_ptr - illegalCharacters));
        }
        else
        {
            *out++ = *key;
        }
        key++;
    }
    valid = true;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
