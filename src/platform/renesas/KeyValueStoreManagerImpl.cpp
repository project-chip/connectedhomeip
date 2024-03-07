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

#include <platform/KeyValueStoreManager.h>
#include <unordered_map>
#include <string>

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {

//#define STUBBED_KVS

constexpr size_t MAX_DATA_LENGTH = 256;

/** Singleton instance of the KeyValueStoreManager implementation object.
 */
KeyValueStoreManagerImpl KeyValueStoreManagerImpl::sInstance;
struct value_entry
{
        size_t length;
        uint8_t data[MAX_DATA_LENGTH];
};
std::unordered_map<std::string, value_entry> keyvaluestore;

CHIP_ERROR KeyValueStoreManagerImpl::_Get(const char * key, void * value, size_t value_size, size_t * read_bytes_size, size_t offset)
{
#ifndef STUBBED_KVS
    auto kventry = keyvaluestore.find(key);
    if (kventry == keyvaluestore.end())
    {
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }
    if (kventry->second.length > value_size)
    {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }
    memcpy(value, kventry->second.data, std::min(kventry->second.length, value_size));
    if (read_bytes_size != nullptr)
    {
        *read_bytes_size = kventry->second.length;
    }
    return CHIP_NO_ERROR;
#else
    return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
#endif
}

CHIP_ERROR KeyValueStoreManagerImpl::_Delete(const char * key)
{
#ifndef STUBBED_KVS
    auto kventry = keyvaluestore.find(key);
    if (kventry == keyvaluestore.end())
    {
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }
    keyvaluestore.erase(kventry);
#endif
    return CHIP_NO_ERROR;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Put(const char * key, const void * value, size_t value_size)
{
#ifndef STUBBED_KVS
    if (value_size <= MAX_DATA_LENGTH)
    {
        value_entry valueentry;
        valueentry.length = value_size;
        memcpy(valueentry.data, value, value_size);
        keyvaluestore[key] = valueentry;
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_INVALID_ARGUMENT;
#else
    return CHIP_NO_ERROR;
#endif
}

CHIP_ERROR KeyValueStoreManagerImpl::Init()
{
    return CHIP_NO_ERROR;
};

} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
