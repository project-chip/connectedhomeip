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
 *          Platform-specific key value storage implementation for Bee
 */
/* this file behaves like a config.h, comes first */
#include "FreeRTOS.h"
#include "chip_porting.h"
#include <platform/KeyValueStoreManager.h>
#include <support/CodeUtils.h>

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {

KeyValueStoreManagerImpl KeyValueStoreManagerImpl::sInstance;

CHIP_ERROR KeyValueStoreManagerImpl::_Get(const char * key, void * value, size_t value_size, size_t * read_bytes_size,
                                          size_t offset_bytes)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    int32_t result = -1;
    size_t actual_read_bytes = 0;

    VerifyOrExit(key, err = CHIP_ERROR_INVALID_ARGUMENT);
    if(value_size)
    {
        VerifyOrExit(value, err = CHIP_ERROR_INVALID_ARGUMENT);
    }
    else
    {
        if(matter_kvs_key_find(key))
        {
            return err = CHIP_ERROR_BUFFER_TOO_SMALL;
        }
        return  err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }

    if (offset_bytes > 0)
    {
        // Offset and partial reads are not supported in nvs, for now just return NOT_IMPLEMENTED. Support can be added in the
        // future if this is needed.
        return (err = CHIP_ERROR_NOT_IMPLEMENTED);
    }

    result = matter_kvs_get(key, value, value_size, &actual_read_bytes);
    if(MATTER_KVS_LOOKUP_NOT_FOUND == result)//to add 
    {
        err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }
    if(read_bytes_size)
    {
        *read_bytes_size = actual_read_bytes;
    }

exit:
    return err;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Put(const char * key, const void * value, size_t value_size)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
	int32_t error;
    int32_t result = -1;

    //ChipLogError(DeviceLayer, "[KeyValueStoreManagerImpl_Put] Bee4 put key(%s)", key);
    VerifyOrExit((key != NULL) && (value != NULL), err = CHIP_ERROR_INVALID_ARGUMENT);
    result = matter_kvs_put(key, value, value_size);
    if(MATTER_KVS_LOOKUP_NOT_FOUND == result)//to add
    {
        //ChipLogError(DeviceLayer, "[KeyValueStoreManagerImpl_Put] Bee4 put key");
        err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }

exit:
    return err;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Delete(const char * key)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    int32_t result = -1;
    //ChipLogProgress(DeviceLayer, "[KeyValueStoreManagerImpl_Delete][INFO] Bee4 delete key (%s)", key);
    VerifyOrExit(key != NULL, err = CHIP_ERROR_INVALID_ARGUMENT);
    result = matter_kvs_key_delete(key);
    if(MATTER_KVS_LOOKUP_NOT_FOUND == result)//to add
    {
       // ChipLogProgress(DeviceLayer, "[KeyValueStoreManagerImpl_Delete][INFO] Bee4 delete key");
        err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }

exit:
    return err;
}

} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
