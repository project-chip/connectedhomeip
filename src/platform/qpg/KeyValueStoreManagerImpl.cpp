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
 *          Platform-specific key value storage implementation
 *          for Qorvo QPG platforms
 */

#include <lib/core/DataModelTypes.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <platform/KeyValueStoreManager.h>

#include "qvCHIP.h"

const uint8_t kNumberOfFabrics = CHIP_CONFIG_MAX_FABRICS;

/** @brief Is KVS key sensitive
 *
 *  @param[in] key Storage key
 *
 *  @return @true if @key is sensitive
 */
static bool KvsIsKeySensitive(const char * key)
{
    // currently we only encrypt NOC private keys
    // NOC keys are of the form: "f/%x/n"
    for (chip::FabricIndex i = 0; i < kNumberOfFabrics; i++)
    {
        // FabricIndex fabricIndex = fb.GetFabricIndex();
        chip::FabricIndex fabricIndex = i;
        if (!strcmp(key, chip::StorageKeyName::Formatted("f/%x/n", fabricIndex).KeyName()))
        {
            return true;
        }
    }
    return false;
}

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {

KeyValueStoreManagerImpl KeyValueStoreManagerImpl::sInstance;

CHIP_ERROR KeyValueStoreManagerImpl::_Get(const char * key, void * value, size_t value_size, size_t * read_bytes_size,
                                          size_t offset_bytes)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    qvStatus_t result;
    size_t actual_read_bytes;
    bool isSensitive = KvsIsKeySensitive(key);

    VerifyOrExit((key != NULL) && (value != NULL), err = CHIP_ERROR_INVALID_ARGUMENT);

    // Encrypted keys cannot be retrieved in chunks
    VerifyOrExit(!(isSensitive && offset_bytes), err = CHIP_ERROR_INVALID_ARGUMENT);

    result = qvCHIP_KvsGet(key, value, value_size, &actual_read_bytes, offset_bytes, isSensitive);
    if (result == QV_STATUS_BUFFER_TOO_SMALL && isSensitive)
    {
        // Encrypted keys cannot be retrieved in chunks
        err = CHIP_ERROR_INVALID_ARGUMENT;
    }
    else if (result == QV_STATUS_BUFFER_TOO_SMALL)
    {
        err = CHIP_ERROR_BUFFER_TOO_SMALL;
    }
    else if (result == QV_STATUS_INVALID_ARGUMENT)
    {
        err = CHIP_ERROR_INVALID_ARGUMENT;
    }
    else if (result != QV_STATUS_NO_ERROR)
    {
        err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }

    if (read_bytes_size)
    {
        *read_bytes_size = actual_read_bytes;
    }

exit:
    return err;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Put(const char * key, const void * value, size_t value_size)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    qvStatus_t result;

    VerifyOrExit((key != NULL) && (value != NULL), err = CHIP_ERROR_INVALID_ARGUMENT);

    result = qvCHIP_KvsPut(key, value, value_size, KvsIsKeySensitive(key));
    if (result == QV_STATUS_INVALID_ARGUMENT)
    {
        err = CHIP_ERROR_INVALID_ARGUMENT;
    }
    if (result != QV_STATUS_NO_ERROR)
    {
        err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }

exit:
    return err;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Delete(const char * key)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    qvStatus_t result;

    VerifyOrExit(key != NULL, err = CHIP_ERROR_INVALID_ARGUMENT);

    result = qvCHIP_KvsDelete(key);
    if (result != QV_STATUS_NO_ERROR)
    {
        err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }

exit:
    return err;
}

} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
