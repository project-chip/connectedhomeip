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
 *          Platform-specific key value storage implementation for ESP32
 */

#include <platform/ESP32/ScopedNvsHandle.h>
#include <platform/KeyValueStoreManager.h>

#include <algorithm>
#include <string.h>

#include "nvs.h"
#include "nvs_flash.h"
#include <lib/support/CodeUtils.h>

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {

KeyValueStoreManagerImpl KeyValueStoreManagerImpl::sInstance;

CHIP_ERROR KeyValueStoreManagerImpl::_Get(const char * key, void * value, size_t value_size, size_t * read_bytes_size,
                                          size_t offset_bytes)
{
    VerifyOrReturnError(value, CHIP_ERROR_INVALID_ARGUMENT);

    // Offset and partial reads are not supported in nvs, for now just return NOT_IMPLEMENTED. Support can be added in the
    // future if this is needed.
    VerifyOrReturnError(offset_bytes == 0, CHIP_ERROR_NOT_IMPLEMENTED);

    Internal::ScopedNvsHandle handle;
    ReturnErrorOnFailure(handle.Open(kNamespace, NVS_READONLY));
    ReturnMappedErrorOnFailure(nvs_get_blob(handle, key, value, &value_size));

    if (read_bytes_size)
    {
        *read_bytes_size = value_size;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Put(const char * key, const void * value, size_t value_size)
{
    VerifyOrReturnError(value, CHIP_ERROR_INVALID_ARGUMENT);

    Internal::ScopedNvsHandle handle;
    ReturnErrorOnFailure(handle.Open(kNamespace, NVS_READWRITE));

    ReturnMappedErrorOnFailure(nvs_set_blob(handle, key, value, value_size));

    // Commit the value to the persistent store.
    ReturnMappedErrorOnFailure(nvs_commit(handle));

    return CHIP_NO_ERROR;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Delete(const char * key)
{
    Internal::ScopedNvsHandle handle;

    ReturnErrorOnFailure(handle.Open(kNamespace, NVS_READWRITE));

    ReturnMappedErrorOnFailure(nvs_erase_key(handle, key));

    // Commit the value to the persistent store.
    ReturnMappedErrorOnFailure(nvs_commit(handle));

    return CHIP_NO_ERROR;
}

} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
