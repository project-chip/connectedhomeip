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

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/Linux/CHIPLinuxStorage.h>

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {

KeyValueStoreManagerImpl KeyValueStoreManagerImpl::sInstance;

CHIP_ERROR KeyValueStoreManagerImpl::_Get(const char * key, void * value, size_t value_size, size_t * read_bytes_size,
                                          size_t offset_bytes)
{
    size_t read_size;

    // Copy data into value buffer
    VerifyOrReturnError(value != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    // On linux read first without a buffer which returns the size, and then
    // use a local buffer to read the entire object, which allows partial and
    // offset reads.
    CHIP_ERROR err = ChipLinuxStorageDelegate()->ReadValueBin(key, nullptr, 0, read_size);
    if (err == CHIP_ERROR_KEY_NOT_FOUND)
    {
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }
    if ((err != CHIP_NO_ERROR) && (err != CHIP_ERROR_BUFFER_TOO_SMALL))
    {
        return err;
    }
    if (offset_bytes > read_size)
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    Platform::ScopedMemoryBuffer<uint8_t> buf;
    VerifyOrReturnError(buf.Alloc(read_size), CHIP_ERROR_NO_MEMORY);
    ReturnErrorOnFailure(ChipLinuxStorageDelegate()->ReadValueBin(key, buf.Get(), read_size, read_size));

    size_t total_size_to_read = read_size - offset_bytes;
    size_t copy_size          = std::min(value_size, total_size_to_read);
    if (read_bytes_size != nullptr)
    {
        *read_bytes_size = copy_size;
    }
    ::memcpy(value, buf.Get() + offset_bytes, copy_size);

    return (value_size < total_size_to_read) ? CHIP_ERROR_BUFFER_TOO_SMALL : CHIP_NO_ERROR;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Put(const char * key, const void * value, size_t value_size)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = ChipLinuxStorageDelegate()->WriteValueBin(key, reinterpret_cast<const uint8_t *>(value), value_size);
    SuccessOrExit(err);

    // Commit the value to the persistent store.
    err = ChipLinuxStorageDelegate()->Commit();
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Delete(const char * key)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    err            = ChipLinuxStorageDelegate()->ClearValue(key);

    if (err == CHIP_ERROR_KEY_NOT_FOUND)
    {
        ExitNow(err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    }
    SuccessOrExit(err);

    // Commit the value to the persistent store.
    err = ChipLinuxStorageDelegate()->Commit();
    SuccessOrExit(err);

exit:
    return err;
}

} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
