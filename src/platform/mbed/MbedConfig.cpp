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

#include "kvstore_global_api/kvstore_global_api.h"
#include "platform/mbed_error.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

// *** CAUTION ***: Changing the names or namespaces of these values will *break* existing devices.

// Note: An external mbed parameter could be useful so an application can put
// chip NVS values in a single place
#define CHIP_CONFIG_KV_STORE_PARTITION "/kv/"

// NVS namespaces used to store device configuration information.
#define CHIP_CONFIG_FACTORY_PREFIX "chip-factory-"
#define CHIP_CONFIG_CONFIG_PREFIX "chip-config-"
#define CHIP_CONFIG_COUNTER_PREFIX "chip-counters-"

#define FACTORY_KEY(key) CHIP_CONFIG_KV_STORE_PARTITION CHIP_CONFIG_FACTORY_PREFIX key
#define CONFIG_KEY(key) CHIP_CONFIG_KV_STORE_PARTITION CHIP_CONFIG_CONFIG_PREFIX key

const char MbedConfig::kConfigNamespace_ChipFactory[]  = CHIP_CONFIG_KV_STORE_PARTITION CHIP_CONFIG_FACTORY_PREFIX;
const char MbedConfig::kConfigNamespace_ChipConfig[]   = CHIP_CONFIG_KV_STORE_PARTITION CHIP_CONFIG_CONFIG_PREFIX;
const char MbedConfig::kConfigNamespace_ChipCounters[] = CHIP_CONFIG_KV_STORE_PARTITION CHIP_CONFIG_COUNTER_PREFIX;

// Keys stored in the chip-factory namespace
const MbedConfig::Key MbedConfig::kConfigKey_SerialNum           = { FACTORY_KEY("serial-num") };
const MbedConfig::Key MbedConfig::kConfigKey_MfrDeviceId         = { FACTORY_KEY("device-id") };
const MbedConfig::Key MbedConfig::kConfigKey_MfrDeviceCert       = { FACTORY_KEY("device-cert") };
const MbedConfig::Key MbedConfig::kConfigKey_MfrDeviceICACerts   = { FACTORY_KEY("device-ca-certs") };
const MbedConfig::Key MbedConfig::kConfigKey_MfrDevicePrivateKey = { FACTORY_KEY("device-key") };
const MbedConfig::Key MbedConfig::kConfigKey_ProductRevision     = { FACTORY_KEY("product-rev") };
const MbedConfig::Key MbedConfig::kConfigKey_ManufacturingDate   = { FACTORY_KEY("mfg-date") };
const MbedConfig::Key MbedConfig::kConfigKey_SetupPinCode        = { FACTORY_KEY("pin-code") };
const MbedConfig::Key MbedConfig::kConfigKey_SetupDiscriminator  = { FACTORY_KEY("discriminator") };

// Keys stored in the chip-config namespace
const MbedConfig::Key MbedConfig::kConfigKey_FabricId                    = { CONFIG_KEY("fabric-id") };
const MbedConfig::Key MbedConfig::kConfigKey_ServiceConfig               = { CONFIG_KEY("service-config") };
const MbedConfig::Key MbedConfig::kConfigKey_PairedAccountId             = { CONFIG_KEY("account-id") };
const MbedConfig::Key MbedConfig::kConfigKey_ServiceId                   = { CONFIG_KEY("service-id") };
const MbedConfig::Key MbedConfig::kConfigKey_GroupKeyIndex               = { CONFIG_KEY("group-key-index") };
const MbedConfig::Key MbedConfig::kConfigKey_LastUsedEpochKeyId          = { CONFIG_KEY("last-ek-id") };
const MbedConfig::Key MbedConfig::kConfigKey_FailSafeArmed               = { CONFIG_KEY("fail-safe-armed") };
const MbedConfig::Key MbedConfig::kConfigKey_WiFiStationSecType          = { CONFIG_KEY("sta-sec-type") };
const MbedConfig::Key MbedConfig::kConfigKey_OperationalDeviceId         = { CONFIG_KEY("op-device-id") };
const MbedConfig::Key MbedConfig::kConfigKey_OperationalDeviceCert       = { CONFIG_KEY("op-device-cert") };
const MbedConfig::Key MbedConfig::kConfigKey_OperationalDeviceICACerts   = { CONFIG_KEY("op-device-ca-certs") };
const MbedConfig::Key MbedConfig::kConfigKey_OperationalDevicePrivateKey = { CONFIG_KEY("op-device-key") };

CHIP_ERROR MbedConfig::ReadConfigValue(Key key, bool & val)
{
    if (!ConfigValueExists(key))
    {
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }

    size_t actual_size = 0;
    int err            = kv_get(key, reinterpret_cast<void *>(&val), sizeof(val), &actual_size);

    if (err != MBED_SUCCESS)
    {
        return CHIP_ERROR_INTERNAL;
    }

    if (actual_size != sizeof(val))
    {
        return CHIP_ERROR_BAD_REQUEST;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR MbedConfig::ReadConfigValue(Key key, uint32_t & val)
{
    if (!ConfigValueExists(key))
    {
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }

    size_t actual_size = 0;
    int err            = kv_get(key, reinterpret_cast<void *>(&val), sizeof(val), &actual_size);

    if (err != MBED_SUCCESS)
    {
        return CHIP_ERROR_INTERNAL;
    }

    if (actual_size != sizeof(val))
    {
        return CHIP_ERROR_BAD_REQUEST;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR MbedConfig::ReadConfigValue(Key key, uint64_t & val)
{
    if (!ConfigValueExists(key))
    {
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }

    size_t actual_size = 0;
    int err            = kv_get(key, &val, sizeof(val), &actual_size);

    if (err != MBED_SUCCESS)
    {
        return CHIP_ERROR_INTERNAL;
    }

    if (actual_size != sizeof(val))
    {
        return CHIP_ERROR_BAD_REQUEST;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR MbedConfig::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
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

CHIP_ERROR MbedConfig::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    if (!ConfigValueExists(key))
    {
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }

    int err = kv_get(key, reinterpret_cast<void *>(buf), bufSize, &outLen);

    if (err != MBED_SUCCESS)
    {
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR MbedConfig::WriteConfigValue(Key key, bool val)
{
    int err = kv_set(key, reinterpret_cast<const void *>(&val), sizeof(val), 0);
    return err == MBED_SUCCESS ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
}

CHIP_ERROR MbedConfig::WriteConfigValue(Key key, uint32_t val)
{
    int err = kv_set(key, reinterpret_cast<const void *>(&val), sizeof(val), 0);
    return err == MBED_SUCCESS ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
}

CHIP_ERROR MbedConfig::WriteConfigValue(Key key, uint64_t val)
{
    int err = kv_set(key, reinterpret_cast<void *>(&val), sizeof(val), 0);
    return err == MBED_SUCCESS ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
}

CHIP_ERROR MbedConfig::WriteConfigValueStr(Key key, const char * str)
{
    return WriteConfigValueBin(key, reinterpret_cast<const uint8_t *>(str), strlen(str));
}

CHIP_ERROR MbedConfig::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    return WriteConfigValueBin(key, reinterpret_cast<const uint8_t *>(str), strLen);
}

CHIP_ERROR MbedConfig::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    // Two different behavior: If the pointer is not null, the value is updated
    // or create. If the pointer is null, the key is removed if it exist.
    if (data != nullptr)
    {
        int err = kv_set(key, reinterpret_cast<const void *>(data), dataLen, 0);
        return err == MBED_SUCCESS ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
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

CHIP_ERROR MbedConfig::ClearConfigValue(Key key)
{
    int err = kv_remove(key);
    return err == MBED_SUCCESS ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
}

bool MbedConfig::ConfigValueExists(Key key)
{
    kv_info_t info;
    int err = kv_get_info(key, &info);
    return err == MBED_SUCCESS ? true : false;
}

CHIP_ERROR MbedConfig::FactoryResetConfig()
{
    // kv_reset is not used, we want to preserve other setting and factory
    // configuration
    auto err = ClearNamespace(kConfigNamespace_ChipConfig);
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }
    return ClearNamespace(kConfigNamespace_ChipCounters);
}

CHIP_ERROR MbedConfig::ConstructCounterKey(Key id, char * buf, size_t bufSize)
{
    auto length = snprintf(buf, bufSize - 1, CHIP_CONFIG_KV_STORE_PARTITION CHIP_CONFIG_COUNTER_PREFIX "%s", id);
    if (length < 0)
    {
        return CHIP_ERROR_INTERNAL;
    }
    else if (length > (bufSize - 1))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    else
    {
        return CHIP_NO_ERROR;
    }
}

CHIP_ERROR MbedConfig::ReadCounter(Key counterId, uint32_t & value)
{
    char key[50] = { 0 };
    auto err     = ConstructCounterKey(counterId, key, sizeof(key));
    if (err)
    {
        return err;
    }

    return ReadConfigValue(key, value);
}

CHIP_ERROR MbedConfig::WriteCounter(Key counterId, uint32_t value)
{
    char key[50] = { 0 };
    auto err     = ConstructCounterKey(counterId, key, sizeof(key));
    if (err)
    {
        return err;
    }

    return WriteConfigValue(key, value);
}

CHIP_ERROR MbedConfig::ClearNamespace(const char * ns)
{
    kv_iterator_t it;
    char key[50];
    int err = kv_iterator_open(&it, ns);
    if (err)
    {
        return CHIP_ERROR_INTERNAL;
    }
    while (kv_iterator_next(it, key, sizeof(key)) != MBED_ERROR_ITEM_NOT_FOUND)
    {
        kv_remove(key);
        memset(key, 0, sizeof(key));
    }

    err = kv_iterator_close(it);
    return err == MBED_SUCCESS ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
}

void MbedConfig::RunConfigUnitTest()
{
    // Run common unit test.
    ::chip::DeviceLayer::Internal::RunConfigUnitTest<MbedConfig>();
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
