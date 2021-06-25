/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *          Platform-specific key value storage implementation for Mbed OS
 */

#include <cstring>
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include "MbedConfig.h"
#include "support/CHIPMem.h"
#include <platform/KeyValueStoreManager.h>

using chip::DeviceLayer::Internal::MbedConfig;

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {

static const char prefix[] = "/kv/chip-kvs-";

class KeyBuilder
{
public:
    KeyBuilder(const char * key)
    {
        auto ret = snprintf(buffer, sizeof(buffer), "%s%s", prefix, key);
        valid    = (ret > 0) && ((size_t) ret <= (sizeof(buffer) - 1));
    }

    const char * str() const { return valid ? buffer : nullptr; }

private:
    char buffer[100];
    bool valid;
};

// NOTE: Currently this platform does not support partial and offset reads
//       these will return CHIP_ERROR_NOT_IMPLEMENTED.
CHIP_ERROR
KeyValueStoreManagerImpl::_Get(const char * key, void * value, size_t value_size, size_t * read_bytes_size, size_t offset)
{
    if (offset > 0)
    {
        // Offset and partial reads are not supported.
        // Support can be added in the future if this is needed.
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    KeyBuilder key_builder(key);
    if (!key_builder.str())
    {
        return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
    }

    auto err = MbedConfig::ReadConfigValueBin(key_builder.str(), reinterpret_cast<uint8_t *>(value), value_size, *read_bytes_size);
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }
    return err;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Delete(const char * key)
{
    KeyBuilder key_builder(key);
    if (!key_builder.str())
    {
        return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
    }

    auto err = MbedConfig::ClearConfigValue(key_builder.str());
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }
    return err;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Put(const char * key, const void * value, size_t value_size)
{
    KeyBuilder key_builder(key);
    if (!key_builder.str())
    {
        return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
    }

    return MbedConfig::WriteConfigValueBin(key_builder.str(), reinterpret_cast<const uint8_t *>(value), value_size);
}

KeyValueStoreManagerImpl KeyValueStoreManagerImpl::sInstance;

} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
