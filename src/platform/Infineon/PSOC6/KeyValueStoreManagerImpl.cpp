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
 *          Platform-specific key value storage implementation for PSOC6
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

CHIP_ERROR KeyValueStoreManagerImpl::Init()
{
    cy_rslt_t result = mtb_key_value_store_init(&kvstore_obj);
    init_success     = (CY_RSLT_SUCCESS == result) ? true : false;

    return ConvertCyResultToChip(result);
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
        return CHIP_ERROR_UNINITIALIZED;
    }

    // Get the value size
    result = mtb_kvstore_read(const_cast<mtb_kvstore_t *>(&kvstore_obj), key, NULL, &actual_size);

    // If fail, return the failure code to the caller.
    // If success, but the value pointer is NULL, then the caller only wanted to know if the key
    // exists and/or the size of the key's value. Set read_bytes_size (if non-NULL) and then return.
    if ((result != CY_RSLT_SUCCESS) || (value == NULL))
    {
        if (read_bytes_size != nullptr)
        {
            *read_bytes_size = static_cast<size_t>(actual_size);
        }

        return ConvertCyResultToChip(result);
    }

    // If actual size is zero, there is no value to read, case this function can return.
    if (actual_size == 0)
    {
        if (read_bytes_size != nullptr)
        {
            *read_bytes_size = actual_size; // The calling matter api expects this to always be set
        }
        return CHIP_NO_ERROR;
    }

    // If the actual size of the stored key is larger than the buffer the caller provided, as indicated by value_size,
    // then we need to copy as many bytes of the value as we can into the return buffer.
    // Since the return buffer is too small, allocate storage big enough. Will be freed later in this function.
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
            // Caller may ask for more than what was originally stored, so we need to zero out the
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

    // If we allocated space for a value larger than the caller anticipated,
    // then we need to copy as many bytes as we can into their provided buffer
    // (e.g. value). After the copy, free our temporary buffer in local_value.
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

        // If the actual size of the value (minus any offset) is larger than the buffer
        // provided to us, as defined by value_size, then we return the too small error code.
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
        return CHIP_ERROR_UNINITIALIZED;
    }

    cy_rslt_t result;

    // This if statement is checking for a situation where the caller provided us a non-NULL value whose
    // size is 0. Per the SyncSetKeyValue definition, it is valid to pass a non-NULL value with size 0.
    // This will result in a key being written to storage, but with an empty value. However, the
    // mtb-kvstore does not allow this. Instead, if you want to store a key with an empty value,
    // mtb-kvstore requires you to pass NULL for value and 0 for size. So, this logic is translating
    // between the two requirements.
    if (value != NULL && static_cast<size_t>(value_size) == 0)
    {
        result = mtb_kvstore_write(&kvstore_obj, key, NULL, 0);
    }
    else
    {
        result = mtb_kvstore_write(&kvstore_obj, key, (uint8_t *) value, static_cast<size_t>(value_size));
    }

    return ConvertCyResultToChip(result);
}

CHIP_ERROR KeyValueStoreManagerImpl::_Delete(const char * key)
{
    if (!init_success)
    {
        return CHIP_ERROR_UNINITIALIZED;
    }

    // Matter KVStore requires that a delete called on a key that doesn't exist return an error
    // indicating no such key exists. mtb-kvstore returns success when asked to delete a key
    // that doesn't exist. To translate between these two requirements, we use mtb_kvstore_read,
    // which returns MTB_KVSTORE_ITEM_NOT_FOUND_ERROR if the key doesn't exist. We only call
    // mtb_kvstore_delete if the key actually exists.
    cy_rslt_t result = mtb_kvstore_read(&kvstore_obj, key, NULL, NULL);
    if (result == CY_RSLT_SUCCESS)
    {
        result = mtb_kvstore_delete(&kvstore_obj, key);
    }

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

CHIP_ERROR KeyValueStoreManagerImpl::Erase(void)
{
    if (!init_success)
    {
        return CHIP_ERROR_UNINITIALIZED;
    }

    cy_rslt_t result = mtb_kvstore_reset(&kvstore_obj);
    return ConvertCyResultToChip(result);
}
} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
