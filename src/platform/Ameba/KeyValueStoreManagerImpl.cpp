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
 *          Platform-specific key value storage implementation for Ameba
 */
/* this file behaves like a config.h, comes first */
#include "FreeRTOS.h"
#include "chip_porting.h"
#include <platform/Ameba/AmebaUtils.h>
#include <platform/KeyValueStoreManager.h>
#include <support/CodeUtils.h>

using namespace chip::DeviceLayer::Internal;

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {

KeyValueStoreManagerImpl KeyValueStoreManagerImpl::sInstance;

CHIP_ERROR KeyValueStoreManagerImpl::_Get(const char * key, void * value, size_t value_size, size_t * read_bytes_size,
                                          size_t offset_bytes)
{
    CHIP_ERROR err;
    int32_t error;

    if (!value)
    {
        err = CHIP_ERROR_INVALID_ARGUMENT;
        goto exit;
    }

    if (offset_bytes > 0)
    {
        // Offset and partial reads are not supported in nvs, for now just return NOT_IMPLEMENTED. Support can be added in the
        // future if this is needed.
        err = CHIP_ERROR_NOT_IMPLEMENTED;
        goto exit;
    }

    if (read_bytes_size)
    {
        error = getPref_bin_new(key, key, (uint8_t *) value, value_size, read_bytes_size);
        err   = AmebaUtils::MapError(error, AmebaErrorType::kDctError);
    }
    else
    {
        size_t * dummy_read_bytes_size = (size_t *) pvPortMalloc(sizeof(size_t));
        if (!dummy_read_bytes_size)
        {
            err = CHIP_ERROR_INTERNAL;
            goto exit;
        }
        error = getPref_bin_new(key, key, (uint8_t *) value, value_size, dummy_read_bytes_size);
        err   = AmebaUtils::MapError(error, AmebaErrorType::kDctError);
        vPortFree(dummy_read_bytes_size);
    }

exit:
    return err;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Put(const char * key, const void * value, size_t value_size)
{
    CHIP_ERROR err;
    int32_t error;

    if (!value)
    {
        err = CHIP_ERROR_INVALID_ARGUMENT;
        goto exit;
    }

    deleteKey(key, key); // delete key if it exists
    error = setPref_new(key, key, (uint8_t *) value, value_size);
    err   = AmebaUtils::MapError(error, AmebaErrorType::kDctError);

exit:
    return err;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Delete(const char * key)
{
    int32_t error  = deleteKey(key, key);
    CHIP_ERROR err = AmebaUtils::MapError(error, AmebaErrorType::kDctError);

    return err;
}

} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
