/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <system/SystemError.h>

#include <zephyr/settings/settings.h>

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {
namespace {

struct ReadEntry
{
    void * destination;           // destination address
    size_t destinationBufferSize; // size of destination buffer
    size_t readSize;              // [out] size of read entry value
    CHIP_ERROR result;            // [out] read result
};

struct DeleteSubtreeEntry
{
    int result;
};

// Random magic bytes to represent an empty value.
// It is needed because Zephyr settings subsystem does not distinguish an empty value from no value.
constexpr uint8_t kEmptyValue[]  = { 0x22, 0xa6, 0x54, 0xd1, 0x39 };
constexpr size_t kEmptyValueSize = sizeof(kEmptyValue);

// Prefix the input key with CHIP_DEVICE_CONFIG_SETTINGS_KEY "/"
CHIP_ERROR MakeFullKey(char (&fullKey)[SETTINGS_MAX_NAME_LEN + 1], const char * key)
{
    VerifyOrReturnError(key != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    strcpy(fullKey, CHIP_DEVICE_CONFIG_SETTINGS_KEY "/");

    char * dest    = fullKey + strlen(CHIP_DEVICE_CONFIG_SETTINGS_KEY "/");
    char * destEnd = fullKey + SETTINGS_MAX_NAME_LEN;

    while (*key != '\0')
    {
        char keyChar = *key++;
        bool escape  = keyChar == '\\' || keyChar == '=';

        if (keyChar == '=')
        {
            // '=' character is forbidden in a Zephyr setting key, so it must be escaped with "\e".
            keyChar = 'e';
        }

        if (escape)
        {
            VerifyOrReturnError(dest < destEnd, CHIP_ERROR_INVALID_ARGUMENT);
            *dest++ = '\\';
        }

        VerifyOrReturnError(dest < destEnd, CHIP_ERROR_INVALID_ARGUMENT);
        *dest++ = keyChar;
    }

    *dest = 0;

    return CHIP_NO_ERROR;
}

int LoadEntryCallback(const char * name, size_t entrySize, settings_read_cb readCb, void * cbArg, void * param)
{
    ReadEntry & entry = *static_cast<ReadEntry *>(param);

    // If requested key X, process just node X and ignore all its descendants: X/*
    if (name != nullptr && *name != '\0')
        return 0;

    // Found requested key.
    uint8_t emptyValue[kEmptyValueSize];

    if (entrySize == kEmptyValueSize && readCb(cbArg, emptyValue, kEmptyValueSize) == kEmptyValueSize &&
        memcmp(emptyValue, kEmptyValue, kEmptyValueSize) == 0)
    {
        // Special case - an empty value represented by known magic bytes.
        entry.result = CHIP_NO_ERROR;

        // Return 1 to stop processing further keys
        return 1;
    }

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

    // Return 1 to stop processing further keys
    return 1;
}

int DeleteSubtreeCallback(const char * name, size_t /* entrySize */, settings_read_cb /* readCb */, void * /* cbArg */,
                          void * param)
{
    DeleteSubtreeEntry & entry = *static_cast<DeleteSubtreeEntry *>(param);
    char fullKey[SETTINGS_MAX_NAME_LEN + 1];

    // name comes from Zephyr settings subsystem so it is guaranteed to fit in the buffer.
    (void) snprintf(fullKey, sizeof(fullKey), CHIP_DEVICE_CONFIG_SETTINGS_KEY "/%s", StringOrNullMarker(name));
    const int result = settings_delete(fullKey);

    // Return the first error, but continue removing remaining keys anyway.
    if (entry.result == 0)
    {
        entry.result = result;
    }

    return 0;
}

} // namespace

KeyValueStoreManagerImpl KeyValueStoreManagerImpl::sInstance;

void KeyValueStoreManagerImpl::Init()
{
    VerifyOrDie(settings_subsys_init() == 0);
}

CHIP_ERROR KeyValueStoreManagerImpl::_Get(const char * key, void * value, size_t value_size, size_t * read_bytes_size,
                                          size_t offset_bytes) const
{
    // Offset and partial reads are not supported, for now just return NOT_IMPLEMENTED.
    // Support can be added in the future if this is needed.
    VerifyOrReturnError(offset_bytes == 0, CHIP_ERROR_NOT_IMPLEMENTED);

    char fullKey[SETTINGS_MAX_NAME_LEN + 1];
    ReturnErrorOnFailure(MakeFullKey(fullKey, key));

    ReadEntry entry{ value, value_size, 0, CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND };
    settings_load_subtree_direct(fullKey, LoadEntryCallback, &entry);

    // Assign readSize only in case read_bytes_size is not nullptr, as it is optional argument
    if (read_bytes_size)
    {
        *read_bytes_size = entry.readSize;
    }

    return entry.result;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Put(const char * key, const void * value, size_t value_size)
{
    char fullKey[SETTINGS_MAX_NAME_LEN + 1];
    ReturnErrorOnFailure(MakeFullKey(fullKey, key));

    if (value_size == 0)
    {
        value      = kEmptyValue;
        value_size = kEmptyValueSize;
    }

    VerifyOrReturnError(settings_save_one(fullKey, value, value_size) == 0, CHIP_ERROR_PERSISTED_STORAGE_FAILED);

    return CHIP_NO_ERROR;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Delete(const char * key)
{
    char fullKey[SETTINGS_MAX_NAME_LEN + 1];
    ReturnErrorOnFailure(MakeFullKey(fullKey, key));

    ReturnErrorCodeIf(Get(key, nullptr, 0) == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND,
                      CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    ReturnErrorCodeIf(settings_delete(fullKey) != 0, CHIP_ERROR_PERSISTED_STORAGE_FAILED);

    return CHIP_NO_ERROR;
}

CHIP_ERROR KeyValueStoreManagerImpl::DoFactoryReset()
{
    DeleteSubtreeEntry entry{ /* success */ 0 };
    int result = settings_load_subtree_direct(CHIP_DEVICE_CONFIG_SETTINGS_KEY, DeleteSubtreeCallback, &entry);

    if (result == 0)
    {
        result = entry.result;
    }

    return System::MapErrorZephyr(result);
}

} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
