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
 *          Platform-specific key value storage implementation for MW320
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include "lib/support/CHIPMem.h"
#include <platform/KeyValueStoreManager.h>
#include <platform/nxp/mw320/MW320Config.h>
#include <string>
extern "C" {
#include <network_flash_storage.h>
}

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {

/* TODO: adjust this value */
#define MAX_NO_OF_KEYS 255

KeyValueStoreManagerImpl KeyValueStoreManagerImpl::sInstance;

#define kMaxKeyValueBytes 2048
CHIP_ERROR KeyValueStoreManagerImpl::_Get(const char * key, void * value, size_t value_size, size_t * read_bytes_size,
                                          size_t offset_bytes)
{
    uint32_t ret;
    uint32_t read_size = kMaxKeyValueBytes;
    uint8_t buf[kMaxKeyValueBytes];

    VerifyOrReturnError(key != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(read_bytes_size != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    // 1. Casting away the "const" because of the difference of the interface definition. The API won't change the value
    // 2. read_size is always less than kMaxKeyValueBytes
    ret = ::get_saved_wifi_network((char *) key, buf, &read_size);
    VerifyOrReturnError(ret == 0, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

    if ((value == nullptr) || (value_size == 0))
    {
        *read_bytes_size = read_size;
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    size_t copy_size = std::min(value_size, (size_t)(read_size - offset_bytes));
    ::memcpy(value, &buf[offset_bytes], copy_size);

    *read_bytes_size = copy_size;

    if ((read_size - offset_bytes) > value_size)
    {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Put(const char * key, const void * value, size_t value_size)
{
    uint32_t ret;
    ret = ::save_wifi_network((char *) key, (uint8_t *) value, value_size);
    VerifyOrReturnError(ret == 0, CHIP_ERROR_PERSISTED_STORAGE_FAILED);

    return CHIP_NO_ERROR;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Delete(const char * key)
{
    uint32_t ret;
    ret = ::reset_saved_wifi_network((char *) key);
    VerifyOrReturnError(ret == 0, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

    return CHIP_NO_ERROR;
}

} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
