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
 *          Platform-specific key value storage implementation.
 */

#pragma once

#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <platform/CHIPDeviceLayer.h>
#include <slist.h>

#include "CYW30739Config.h"

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {

class KeyValueStoreManagerImpl final : public KeyValueStoreManager
{
    // Allow the KeyValueStoreManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend class KeyValueStoreManager;

public:
    CHIP_ERROR Init(void);

    // NOTE: Currently this platform does not support partial and offset reads
    //       these will return CHIP_ERROR_NOT_IMPLEMENTED.
    CHIP_ERROR _Get(const char * key, void * value, size_t value_size, size_t * read_bytes_size = nullptr, size_t offset = 0);

    CHIP_ERROR _Delete(const char * key);

    CHIP_ERROR _Put(const char * key, const void * value, size_t value_size);

    CHIP_ERROR EraseAll(void);

private:
    using Config = Internal::CYW30739Config;

    static constexpr uint8_t mMaxEntryCount = 128;

    struct KeyStorage
    {
        KeyStorage(const char * key = nullptr);

        constexpr size_t GetSize() const { return sizeof(mValueSize) + strlen(mKey); }
        bool IsMatchKey(const char * key) const;

        uint16_t mValueSize;
        char mKey[PersistentStorageDelegate::kKeyLengthMax + 1];
    };

    struct KeyConfigIdEntry : public slist_node_t
    {
        KeyConfigIdEntry(uint8_t configID, const KeyStorage & keyStorage) : mStorage(keyStorage), mConfigID(configID) {}

        constexpr Config::Key GetValueConfigKey() const
        {
            return Internal::CYW30739ConfigKey(Config::kChipKvsValue_KeyBase, mConfigID);
        }
        constexpr Config::Key GetKeyConfigKey() const
        {
            return Internal::CYW30739ConfigKey(Config::kChipKvsKey_KeyBase, mConfigID);
        }
        constexpr KeyConfigIdEntry * Next() const { return static_cast<KeyConfigIdEntry *>(next); }
        constexpr uint8_t NextConfigID() const { return mConfigID + 1; }
        constexpr uint16_t GetValueSize() const { return mStorage.mValueSize; }
        constexpr void SetValueSize(uint16_t valueSize) { mStorage.mValueSize = valueSize; }

        KeyStorage mStorage;
        uint8_t mConfigID;
    };

    KeyConfigIdEntry * AllocateEntry(const char * key);
    KeyConfigIdEntry * FindEntry(const char * key, Optional<uint8_t> * freeConfigID = nullptr);

    // ===== Members for internal use by the following friends.
    friend KeyValueStoreManager & KeyValueStoreMgr();
    friend KeyValueStoreManagerImpl & KeyValueStoreMgrImpl();

    static KeyValueStoreManagerImpl sInstance;

    slist_node_t mKeyConfigIdList;
};

/**
 * Returns the public interface of the KeyValueStoreManager singleton object.
 *
 * Chip applications should use this to access features of the KeyValueStoreManager object
 * that are common to all platforms.
 */
inline KeyValueStoreManager & KeyValueStoreMgr(void)
{
    return KeyValueStoreManagerImpl::sInstance;
}

/**
 * Returns the platform-specific implementation of the KeyValueStoreManager singleton object.
 *
 * Chip applications can use this to gain access to features of the KeyValueStoreManager
 * that are specific to the ESP32 platform.
 */
inline KeyValueStoreManagerImpl & KeyValueStoreMgrImpl(void)
{
    return KeyValueStoreManagerImpl::sInstance;
}

} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
