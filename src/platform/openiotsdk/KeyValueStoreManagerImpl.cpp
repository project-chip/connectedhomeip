/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
 *          Provides an implementation of the key-value storage manager class
 *          for Open IOT SDK platform.
 */

#include <cstring>
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <lib/support/CHIPMem.h>
#include <platform/KeyValueStoreManager.h>
#include <platform/openiotsdk/OpenIoTSDKConfig.h>

using chip::DeviceLayer::Internal::OpenIoTSDKConfig;

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {

class KeyBuilder
{
public:
    KeyBuilder(const char * key)
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
                *out++ = 'A' + (int) (illegal_ptr - illegalCharacters);
            }
            else
            {
                *out++ = *key;
            }
            key++;
        }
        valid = true;
    }

    const char * str() const { return valid ? buffer : nullptr; }

private:
    char buffer[100] = "chip-kvs-";
    bool valid;
    // Mbed KV storage does not accept these characters in the key definition
    const char * illegalCharacters = " */?:;\"|<>\\";
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

    auto err =
        OpenIoTSDKConfig::ReadConfigValueBin(key_builder.str(), reinterpret_cast<uint8_t *>(value), value_size, *read_bytes_size);
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

    auto err = OpenIoTSDKConfig::ClearConfigValue(key_builder.str());
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

    return OpenIoTSDKConfig::WriteConfigValueBin(key_builder.str(), reinterpret_cast<const uint8_t *>(value), value_size);
}

KeyValueStoreManagerImpl KeyValueStoreManagerImpl::sInstance;

} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
