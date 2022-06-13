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

#include <inet/InetConfig.h>

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {

class KeyValueStoreManagerImpl final : public KeyValueStoreManager
{
    // Allow the KeyValueStoreManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend class KeyValueStoreManager;

public:
    // NOTE: Currently this platform does not support partial and offset reads
    //       these will return CHIP_ERROR_NOT_IMPLEMENTED.
    CHIP_ERROR _Get(const char * key, void * value, size_t value_size, size_t * read_bytes_size = nullptr, size_t offset = 0);

    CHIP_ERROR _Delete(const char * key);

    CHIP_ERROR _Put(const char * key, const void * value, size_t value_size);

    CHIP_ERROR EraseAll(void);

private:
    static constexpr uint8_t mMaxEntryCount = 1 +                   /* global event id counter */
        1 +                                                         /* FabricIndexInfo */
        CHIP_CONFIG_MAX_FABRICS +                                   /* FabricNOC */
        CHIP_CONFIG_MAX_FABRICS +                                   /* FabricICAC */
        CHIP_CONFIG_MAX_FABRICS +                                   /* FabricRCAC */
        CHIP_CONFIG_MAX_FABRICS +                                   /* FabricMetadata */
        CHIP_CONFIG_MAX_FABRICS +                                   /* FabricOpKey */
        1 +                                                         /* AccessControlList */
        CHIP_CONFIG_EXAMPLE_ACCESS_CONTROL_MAX_ENTRIES_PER_FABRIC + /* AccessControlEntry */
        1 +                                                         /* GroupDataCounter */
        1 +                                                         /* GroupControlCounter */
        1 +                                                         /* FabricTable */
        4 +                                                         /* FabricGroups */
        CHIP_CONFIG_MAX_FABRICS * 3 +                               /* FabricKeyset */
        1 +                                                         /* OTADefaultProviders */
        1 +                                                         /* OTACurrentProvider */
        1 +                                                         /* OTAUpdateToken */
        1 +                                                         /* OTACurrentUpdateState */
        1 +                                                         /* OTATargetVersion */
        16;                                                         /* AttributeValue */

    struct KeyEntry
    {
        bool mIsValid;
        char mKey[CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH];

        bool IsMatchKey(const char * key);
    };

    class KeyEntryStorage
    {
    public:
        KeyEntryStorage(void);
        ~KeyEntryStorage(void);

        CHIP_ERROR AllocateEntry(uint16_t & nvramID, const char * key, size_t keyLength);
        void ReleaseEntry(const char * key);
        CHIP_ERROR FindKeyNvramID(uint16_t & nvramID, const char * key);

    private:
        KeyEntry mKeyEntries[mMaxEntryCount];
        bool mIsDirty;
    };

    // ===== Members for internal use by the following friends.
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
