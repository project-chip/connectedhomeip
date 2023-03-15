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
 *          Platform-specific key value storage implementation for EFR32.
 *
 */

#pragma once
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <system/SystemClock.h>
#include <system/SystemLayer.h>

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
    CHIP_ERROR _Put(const char * key, const void * value, size_t value_size);
    CHIP_ERROR _Get(const char * key, void * value, size_t value_size, size_t * read_bytes_size = nullptr, size_t offset = 0) const;
    CHIP_ERROR _Delete(const char * key);
    void ErasePartition(void);

    static constexpr size_t kMaxEntries = KVS_MAX_ENTRIES;

    static void ForceKeyMapSave();
    static void KvsMapMigration();

private:
    static void OnScheduledKeyMapSave(System::Layer * systemLayer, void * appState);

    void ScheduleKeyMapSave(void);
    bool IsValidKvsNvm3Key(const uint32_t nvm3Key) const;
    uint16_t hashKvsKeyString(const char * key) const;
    CHIP_ERROR MapKvsKeyToNvm3(const char * key, uint16_t hash, uint32_t & nvm3Key, bool isSlotNeeded = false) const;

    //  ===== Members for internal use by the following friends.
    friend KeyValueStoreManager & KeyValueStoreMgr();
    friend KeyValueStoreManagerImpl & KeyValueStoreMgrImpl();

    static KeyValueStoreManagerImpl sInstance;
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
