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
 */

#include <platform/KeyValueStoreManager.h>

#include <lib/support/CodeUtils.h>
#include <platform_nvram.h>
#include <string.h>

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {

KeyValueStoreManagerImpl KeyValueStoreManagerImpl::sInstance;

CHIP_ERROR KeyValueStoreManagerImpl::_Get(const char * key, void * value, size_t value_size, size_t * read_bytes_size,
                                          size_t offset_bytes)
{
    CHIP_ERROR err                    = CHIP_NO_ERROR;
    KeyEntryStorage * keyEntryStorage = NULL;
    uint16_t nvramID                  = 0;
    wiced_result_t result;
    uint16_t byte_count;

    VerifyOrReturnError(offset_bytes == 0, CHIP_ERROR_NOT_IMPLEMENTED);

    const size_t keyLength = strnlen(key, CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH);
    VerifyOrExit(keyLength != 0 && keyLength <= CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH &&
                     value_size <= CHIP_CONFIG_PERSISTED_STORAGE_MAX_VALUE_LENGTH,
                 err = CHIP_ERROR_INVALID_ARGUMENT);

    keyEntryStorage = new KeyEntryStorage();
    VerifyOrExit(keyEntryStorage != NULL, ChipLogError(DeviceLayer, "%s new KeyEntryStorage", __func__);
                 err = CHIP_ERROR_NO_MEMORY;);

    SuccessOrExit(err = keyEntryStorage->FindKeyNvramID(nvramID, key));

    byte_count = wiced_hal_read_nvram(nvramID, value_size, (uint8_t *) value, &result);
    VerifyOrExit(result == WICED_SUCCESS, ChipLogError(DeviceLayer, "%s wiced_hal_read_nvram %u", __func__, result);
                 err = CHIP_ERROR_INTEGRITY_CHECK_FAILED;);

    if (read_bytes_size != NULL)
    {
        *read_bytes_size = byte_count;
    }

exit:
    delete keyEntryStorage;

    return err;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Put(const char * key, const void * value, size_t value_size)
{
    CHIP_ERROR err                    = CHIP_NO_ERROR;
    KeyEntryStorage * keyEntryStorage = NULL;
    uint16_t nvramID                  = 0;
    wiced_result_t result;
    uint16_t byte_count;

    const size_t keyLength = strnlen(key, CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH + 1);
    VerifyOrExit(keyLength != 0 && keyLength <= CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH &&
                     value_size <= CHIP_CONFIG_PERSISTED_STORAGE_MAX_VALUE_LENGTH,
                 err = CHIP_ERROR_INVALID_ARGUMENT);

    keyEntryStorage = new KeyEntryStorage();
    VerifyOrExit(keyEntryStorage != NULL, ChipLogError(DeviceLayer, "%s new KeyEntryStorage", __func__);
                 err = CHIP_ERROR_NO_MEMORY;);

    err = keyEntryStorage->AllocateEntry(nvramID, key, keyLength);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(DeviceLayer, "%s AllocateEntry %s", __func__, ErrorStr(err)));

    byte_count = wiced_hal_write_nvram(nvramID, value_size, (uint8_t *) value, &result);
    VerifyOrExit(byte_count == value_size && result == WICED_SUCCESS,
                 ChipLogError(DeviceLayer, "%s wiced_hal_write_nvram %u", __func__, result);
                 keyEntryStorage->ReleaseEntry(key); err = CHIP_ERROR_PERSISTED_STORAGE_FAILED;);

exit:
    delete keyEntryStorage;

    return err;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Delete(const char * key)
{
    CHIP_ERROR err                    = CHIP_NO_ERROR;
    KeyEntryStorage * keyEntryStorage = NULL;
    uint16_t nvramID                  = 0;

    const size_t keyLength = strnlen(key, CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH);
    VerifyOrExit(keyLength != 0 && keyLength <= CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH, err = CHIP_ERROR_INVALID_ARGUMENT);

    keyEntryStorage = new KeyEntryStorage();
    VerifyOrExit(keyEntryStorage != NULL, ChipLogError(DeviceLayer, "%s new KeyEntryStorage", __func__);
                 err = CHIP_ERROR_NO_MEMORY;);

    SuccessOrExit(err = keyEntryStorage->FindKeyNvramID(nvramID, key));

    keyEntryStorage->ReleaseEntry(key);

exit:
    delete keyEntryStorage;

    return err;
}

bool KeyValueStoreManagerImpl::KeyEntry::IsMatchKey(const char * key)
{
    return mIsValid && strncmp(mKey, key, sizeof(mKey)) == 0;
}

KeyValueStoreManagerImpl::KeyEntryStorage::KeyEntryStorage(void) : mIsDirty(false)
{
    wiced_result_t result;
    const uint16_t byte_count =
        wiced_hal_read_nvram(PLATFORM_NVRAM_ID_ENTRY_INFO_STRING, sizeof(mKeyEntries), (uint8_t *) mKeyEntries, &result);
    if (byte_count != sizeof(mKeyEntries) || result != WICED_SUCCESS)
    {
        memset(mKeyEntries, 0, sizeof(mKeyEntries));
    }
}

KeyValueStoreManagerImpl::KeyEntryStorage::~KeyEntryStorage(void)
{
    if (mIsDirty)
    {
        wiced_result_t result;
        const uint16_t byte_count =
            wiced_hal_write_nvram(PLATFORM_NVRAM_ID_ENTRY_INFO_STRING, sizeof(mKeyEntries), (uint8_t *) mKeyEntries, &result);
        if (byte_count != sizeof(mKeyEntries) || result != WICED_SUCCESS)
        {
            ChipLogError(DeviceLayer, "%s wiced_hal_write_nvram %u", __func__, result);
        }
    }
}

CHIP_ERROR KeyValueStoreManagerImpl::KeyEntryStorage::AllocateEntry(uint16_t & nvramID, const char * key, size_t keyLength)
{
    ReturnErrorCodeIf(keyLength == 0 || keyLength > sizeof(mKeyEntries[0].mKey), CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorCodeIf(FindKeyNvramID(nvramID, key) == CHIP_NO_ERROR, CHIP_NO_ERROR);

    for (uint8_t i = 0; i < ArraySize(mKeyEntries); i++)
    {
        if (!mKeyEntries[i].mIsValid)
        {
            mKeyEntries[i].mIsValid = true;
            memcpy(mKeyEntries[i].mKey, key, keyLength);
            mIsDirty = true;
            nvramID  = PLATFORM_NVRAM_ID_ENTRY_DATA_STRING + i;
            return CHIP_NO_ERROR;
        }
    }

    return CHIP_ERROR_NO_MEMORY;
}

void KeyValueStoreManagerImpl::KeyEntryStorage::ReleaseEntry(const char * key)
{
    for (uint8_t i = 0; i < ArraySize(mKeyEntries); i++)
    {
        if (mKeyEntries[i].IsMatchKey(key))
        {
            mKeyEntries[i].mIsValid = false;
            mIsDirty                = true;
            break;
        }
    }
}

CHIP_ERROR KeyValueStoreManagerImpl::KeyEntryStorage::FindKeyNvramID(uint16_t & nvramID, const char * key)
{
    for (uint8_t i = 0; i < ArraySize(mKeyEntries); i++)
    {
        if (mKeyEntries[i].IsMatchKey(key))
        {
            nvramID = PLATFORM_NVRAM_ID_ENTRY_DATA_STRING + i;
            return CHIP_NO_ERROR;
        }
    }
    return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
}

} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
