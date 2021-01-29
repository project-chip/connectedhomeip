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
 *          Platform-specific implementatiuon of KVS for linux.
 */

#include <platform/KeyValueStoreManager.h>

#include <algorithm>
#include <string.h>

#include <platform/Linux/CHIPLinuxStorage.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {

KeyValueStoreManagerImpl KeyValueStoreManagerImpl::sInstance;

CHIP_ERROR KeyValueStoreManagerImpl::_Get(const char * key, void * value, size_t value_size, size_t * read_bytes_size,
                                          size_t offset_bytes)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    size_t read_size;
    size_t copy_size;

    // On linux read first without a buffer which returns the size, and then
    // use a local buffer to read the entire object, which allows partial and
    // offset reads.
    err = mStorage.ReadValueBin(key, nullptr, 0, read_size);
    uint8_t buf[read_size];
    if (err == CHIP_ERROR_KEY_NOT_FOUND)
    {
        ExitNow(err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    }
    err = mStorage.ReadValueBin(key, buf, read_size, read_size);
    SuccessOrExit(err);

    // Copy data into value buffer
    if (!value)
    {
        ExitNow(err = CHIP_ERROR_INVALID_ARGUMENT);
    }
    copy_size = std::min(value_size, read_size - offset_bytes);
    if (read_bytes_size)
    {
        *read_bytes_size = copy_size;
    }
    ::memcpy(value, buf + offset_bytes, copy_size);

exit:
    return err;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Put(const char * key, const void * value, size_t value_size)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = mStorage.WriteValueBin(key, reinterpret_cast<const uint8_t *>(value), value_size);
    SuccessOrExit(err);

    // Commit the value to the persistent store.
    err = mStorage.Commit();
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Delete(const char * key)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    err            = mStorage.ClearValue(key);

    if (err == CHIP_ERROR_KEY_NOT_FOUND)
    {
        ExitNow(err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    }
    SuccessOrExit(err);

    // Commit the value to the persistent store.
    err = mStorage.Commit();
    SuccessOrExit(err);

exit:
    return err;
}

} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
