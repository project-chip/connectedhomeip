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

using namespace ::chip::DeviceLayer::Internal;

namespace {
constexpr size_t kMaxPersistedValueLengthSupported = 2048;
} // namespace

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {

KeyValueStoreManagerImpl KeyValueStoreManagerImpl::sInstance;

CHIP_ERROR KeyValueStoreManagerImpl::Init(void)
{
    INIT_SLIST_NODE(&mKeyConfigIdList);

    CHIP_ERROR err = CYW30739Config::Init();
    SuccessOrExit(err);

    for (uint8_t configID = 0; configID < mMaxEntryCount; configID++)
    {
        char key[CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH];
        memset(key, 0, sizeof(key));
        size_t keyLength;
        err = CYW30739Config::ReadConfigValueStr(CYW30739ConfigKey(Config::kChipKvsKey_KeyBase, configID), key, sizeof(key),
                                                 keyLength);
        if (err != CHIP_NO_ERROR)
            continue;

        KeyConfigIdEntry * entry = Platform::New<KeyConfigIdEntry>(configID, key, keyLength);
        VerifyOrExit(entry != nullptr, err = CHIP_ERROR_NO_MEMORY);

        slist_add_tail(entry, &mKeyConfigIdList);
    }

    err = CHIP_NO_ERROR;

exit:
    if (err != CHIP_NO_ERROR)
        EraseAll();

    return err;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Get(const char * key, void * value, size_t value_size, size_t * read_bytes_size,
                                          size_t offset_bytes)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    const KeyConfigIdEntry * entry;

    VerifyOrReturnError(offset_bytes == 0, CHIP_ERROR_NOT_IMPLEMENTED);

    const size_t keyLength = strnlen(key, CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH);
    VerifyOrExit(keyLength != 0 && keyLength <= CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH &&
                     value_size <= kMaxPersistedValueLengthSupported,
                 err = CHIP_ERROR_INVALID_ARGUMENT);

    entry = FindEntry(key);
    VerifyOrExit(entry != nullptr, err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);
    VerifyOrExit(value_size != 0, err = CHIP_ERROR_BUFFER_TOO_SMALL);

    size_t byte_count;
    err = CYW30739Config::ReadConfigValueBin(entry->GetValueConfigKey(), value, value_size, byte_count);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(DeviceLayer, "%s ReadConfigValueBin %s", __func__, ErrorStr(err));
                 err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

    if (read_bytes_size != nullptr)
    {
        *read_bytes_size = byte_count;
    }

exit:
    return err;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Put(const char * key, const void * value, size_t value_size)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    const KeyConfigIdEntry * entry;

    const size_t keyLength = strnlen(key, CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH + 1);
    VerifyOrExit(keyLength != 0 && keyLength <= CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH &&
                     value_size <= kMaxPersistedValueLengthSupported,
                 err = CHIP_ERROR_INVALID_ARGUMENT);

    entry = AllocateEntry(key, keyLength);
    VerifyOrExit(entry != nullptr, ChipLogError(DeviceLayer, "%s AllocateEntry %s", __func__, ErrorStr(err));
                 err = CHIP_ERROR_NO_MEMORY);

    SuccessOrExit(err = CYW30739Config::WriteConfigValueBin(entry->GetValueConfigKey(), value, value_size));

    SuccessOrExit(err = CYW30739Config::WriteConfigValueStr(entry->GetKeyConfigKey(), key, keyLength));

exit:
    return err;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Delete(const char * key)
{
    CHIP_ERROR err;
    KeyConfigIdEntry * entry;

    const size_t keyLength = strnlen(key, CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH);
    VerifyOrExit(keyLength != 0 && keyLength <= CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH, err = CHIP_ERROR_INVALID_ARGUMENT);

    entry = FindEntry(key);
    VerifyOrExit(entry != nullptr, err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

    err = CYW30739Config::ClearConfigValue(entry->GetKeyConfigKey());
    VerifyOrExit(ChipError::IsSuccess(err), err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

    err = CYW30739Config::ClearConfigValue(entry->GetValueConfigKey());
    VerifyOrExit(ChipError::IsSuccess(err), err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND);

    slist_del(entry, &mKeyConfigIdList);
    Platform::Delete(entry);

exit:
    return err;
}

CHIP_ERROR KeyValueStoreManagerImpl::EraseAll(void)
{
    KeyConfigIdEntry * entry;
    while ((entry = static_cast<KeyConfigIdEntry *>(slist_get(&mKeyConfigIdList))) != nullptr)
    {
        CYW30739Config::ClearConfigValue(entry->GetKeyConfigKey());
        CYW30739Config::ClearConfigValue(entry->GetValueConfigKey());
        Platform::Delete(entry);
    }
    return CHIP_NO_ERROR;
}

KeyValueStoreManagerImpl::KeyConfigIdEntry::KeyConfigIdEntry(uint8_t configID, const char * key, size_t keyLength) :
    mConfigID(configID)
{
    memset(mKey, 0, sizeof(mKey));
    memcpy(mKey, key, keyLength);
}

bool KeyValueStoreManagerImpl::KeyConfigIdEntry::IsMatchKey(const char * key) const
{
    return strncmp(mKey, key, sizeof(mKey)) == 0;
}

KeyValueStoreManagerImpl::KeyConfigIdEntry * KeyValueStoreManagerImpl::AllocateEntry(const char * key, size_t keyLength)
{
    Optional<uint8_t> freeConfigID;
    KeyConfigIdEntry * newEntry = FindEntry(key, &freeConfigID);
    ReturnErrorCodeIf(newEntry != nullptr, newEntry);
    ReturnErrorCodeIf(!freeConfigID.HasValue(), nullptr);

    newEntry = Platform::New<KeyConfigIdEntry>(freeConfigID.Value(), key, keyLength);
    ReturnErrorCodeIf(newEntry == nullptr, nullptr);

    KeyConfigIdEntry * entry = static_cast<KeyConfigIdEntry *>(slist_tail(&mKeyConfigIdList));
    if (entry == nullptr)
    {
        slist_add_tail(newEntry, &mKeyConfigIdList);
        return newEntry;
    }

    /*
     * The list was built in ascending order by the config ID.
     * Find the entry before which the new entry will be inserted.
     */
    do
    {
        entry = entry->Next();

        if (newEntry->mConfigID < entry->mConfigID)
        {
            slist_add_before(newEntry, entry, &mKeyConfigIdList);
            return newEntry;
        }

    } while (entry != slist_tail(&mKeyConfigIdList));

    slist_add_tail(newEntry, &mKeyConfigIdList);

    return newEntry;
}

KeyValueStoreManagerImpl::KeyConfigIdEntry * KeyValueStoreManagerImpl::FindEntry(const char * key, Optional<uint8_t> * freeConfigID)
{
    KeyConfigIdEntry * entry = static_cast<KeyConfigIdEntry *>(slist_tail(&mKeyConfigIdList));
    if (entry == nullptr)
    {
        if (freeConfigID != nullptr)
            freeConfigID->SetValue(0);
        return nullptr;
    }

    do
    {
        entry = entry->Next();

        if (entry->IsMatchKey(key))
            return entry;

        if (freeConfigID != nullptr && !freeConfigID->HasValue() && entry != slist_tail(&mKeyConfigIdList))
        {
            if (entry->NextConfigID() < entry->Next()->mConfigID)
                freeConfigID->SetValue(entry->NextConfigID());
        }

    } while (entry != slist_tail(&mKeyConfigIdList));

    if (freeConfigID != nullptr && !freeConfigID->HasValue())
    {
        if (entry->NextConfigID() < mMaxEntryCount)
            freeConfigID->SetValue(entry->NextConfigID());
    }

    return nullptr;
}

} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
