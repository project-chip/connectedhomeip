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
 *          Platform-specific key value storage implementation for P6
 */

#include "cy_result.h"
#include <platform/KeyValueStoreManager.h>

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {

KeyValueStoreManagerImpl KeyValueStoreManagerImpl::sInstance;

KeyValueStoreManagerImpl::KeyValueStoreManagerImpl()
{
    cy_rslt_t result = mtb_key_value_store_init(&kvstore_obj);
    init_success     = (CY_RSLT_SUCCESS == result) ? true : false;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Get(const char * key, void * value, size_t value_size, size_t * read_bytes_size,
                                          size_t offset_bytes) const
{
    uint8_t * local_value;
    uint32_t actual_size;
    uint32_t size;
    cy_rslt_t result;

    if (!init_success)
    {
        return CHIP_ERROR_WELL_UNINITIALIZED;
    }

    // Get the value size
    result = mtb_kvstore_read(const_cast<mtb_kvstore_t *>(&kvstore_obj), key, NULL, &actual_size);

    if ((result != CY_RSLT_SUCCESS) || (value == NULL))
    {
        if (read_bytes_size != nullptr)
        {
            *read_bytes_size = static_cast<size_t>(actual_size);
        }

        return ConvertCyResultToChip(result);
    }

    if ((actual_size > value_size) || (offset_bytes != 0))
    {
        size = actual_size;

        local_value = (uint8_t *) malloc(actual_size);

        if (local_value == NULL)
        {
            return CHIP_ERROR_INTERNAL;
        }
    }
    else
    {
        size = value_size;

        local_value = (uint8_t *) value;

        if (actual_size < value_size)
        {
            // They may ask for more than what was originally stored, so we need to zero out the
            // entire value to account for that.
            memset(&((uint8_t *) value)[actual_size], 0, value_size - actual_size);
        }
    }

    // Read the value
    result = mtb_kvstore_read(const_cast<mtb_kvstore_t *>(&kvstore_obj), key, local_value, &size);

    if (result != CY_RSLT_SUCCESS)
    {
        return ConvertCyResultToChip(result);
    }

    if (local_value != value)
    {
        memcpy(value, &local_value[offset_bytes], value_size);
        free(local_value);
    }

    if (actual_size > value_size)
    {
        if (read_bytes_size != nullptr)
        {
            *read_bytes_size = static_cast<size_t>(value_size);
        }

        if ((actual_size - offset_bytes) > value_size)
        {
            return CHIP_ERROR_BUFFER_TOO_SMALL;
        }
    }
    else if (read_bytes_size != nullptr)
    {
        *read_bytes_size = static_cast<size_t>(size);
    }

    return ConvertCyResultToChip(result);
}

CHIP_ERROR KeyValueStoreManagerImpl::_Put(const char * key, const void * value, size_t value_size)
{
    if (!init_success)
    {
        return CHIP_ERROR_WELL_UNINITIALIZED;
    }

    cy_rslt_t result = mtb_kvstore_write(&kvstore_obj, key, static_cast<const uint8_t *>(value), value_size);
    return ConvertCyResultToChip(result);
}

CHIP_ERROR KeyValueStoreManagerImpl::_Delete(const char * key)
{
    if (!init_success)
    {
        return CHIP_ERROR_WELL_UNINITIALIZED;
    }

    cy_rslt_t result = mtb_kvstore_delete(&kvstore_obj, key);
    return ConvertCyResultToChip(result);
}

CHIP_ERROR KeyValueStoreManagerImpl::ConvertCyResultToChip(cy_rslt_t err) const
{
    switch (err)
    {
    case CY_RSLT_SUCCESS:
        return CHIP_NO_ERROR;
    case MTB_KVSTORE_BAD_PARAM_ERROR:
        return CHIP_ERROR_INVALID_ARGUMENT;
    case MTB_KVSTORE_STORAGE_FULL_ERROR: // Can't find a better CHIP error to translate this into
    case MTB_KVSTORE_MEM_ALLOC_ERROR:
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    case MTB_KVSTORE_INVALID_DATA_ERROR:
    case MTB_KVSTORE_ERASED_DATA_ERROR:
        return CHIP_ERROR_INTEGRITY_CHECK_FAILED;
    case MTB_KVSTORE_ITEM_NOT_FOUND_ERROR:
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    case MTB_KVSTORE_ALIGNMENT_ERROR:
    default:
        return CHIP_ERROR_INTERNAL;
    }
    return CHIP_ERROR_INTERNAL;
}

} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
