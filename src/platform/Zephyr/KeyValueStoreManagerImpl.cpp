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
 *          Platform-specific key value storage implementation for Zephyr
 */

#include <platform/KeyValueStoreManager.h>
#include <support/CodeUtils.h>
#include <support/ReturnMacros.h>
#include <support/logging/CHIPLogging.h>

#include <settings/settings.h>
#include <logging/log.h>

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {

KeyValueStoreManagerImpl KeyValueStoreManagerImpl::sInstance;

void KeyValueStoreManagerImpl::Init()
{
    VerifyOrDie(settings_subsys_init() == 0);
}

struct ReadEntry
{
    void * destination;            // NOTE: can be nullptr in which case `readSize` should still be returned
    size_t destinationBufferSize;  // size of destination buffer
    size_t readSize;               // [out] size of read entry value
    CHIP_ERROR result;             // [out] read result
};

static int LoadEntryCallback(const char * name, size_t entrySize, settings_read_cb readCb, void * cbArg, void * param)
{
    // If requested a key X, process just node X and ignore all its descendants: X/*
    if (settings_name_next(name, nullptr) > 0)
        return 0;

    ReadEntry & entry = *reinterpret_cast<ReadEntry *>(param);

    // Found requested key
    const ssize_t bytesRead = readCb(cbArg, entry.destination, entry.destinationBufferSize);
    entry.readSize          = bytesRead > 0 ? bytesRead : 0;

    if (entrySize > entry.destinationBufferSize)
    {
        entry.result = CHIP_ERROR_BUFFER_TOO_SMALL;
    }
    else
    {
        entry.result = bytesRead > 0 ? CHIP_NO_ERROR : CHIP_ERROR_PERSISTED_STORAGE_FAILED;
    }

    return 0;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Get(const char * key, void * value, size_t value_size, size_t * read_bytes_size,
                                          size_t offset_bytes) const
{
    VerifyOrReturnError(key, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(value, CHIP_ERROR_INVALID_ARGUMENT);
    // Offset and partial reads are not supported. Support can be added in the future if this is needed.
    VerifyOrDie(offset_bytes == 0);

    ReadEntry entry{ value, value_size, 0, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND };
    settings_load_subtree_direct(key, LoadEntryCallback, &entry);

    // Assign readSize only in case read_bytes_size is not nullptr, as it is optional argument
    if (read_bytes_size)
        *read_bytes_size = entry.readSize;

    return entry.result;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Put(const char * key, const void * value, size_t value_size)
{
    VerifyOrReturnError(key, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(value, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(value_size > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(settings_save_one(key, value, value_size) == 0, CHIP_ERROR_PERSISTED_STORAGE_FAILED);

    return CHIP_NO_ERROR;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Delete(const char * key)
{
    if (settings_delete(key) != 0)
        return CHIP_ERROR_PERSISTED_STORAGE_FAILED;

    return CHIP_NO_ERROR;
}

} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
