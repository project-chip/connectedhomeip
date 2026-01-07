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
 *          Platform-specific implementation of KVS for linux.
 */

#pragma once

#include <platform/Linux/CHIPLinuxStorage.h>
#include <platform/Linux/PosixConfig.h>

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {

class KeyValueStoreManagerImpl : public KeyValueStoreManager
{
public:
    /**
     * @brief
     * Initalize the KVS, must be called before using.
     */
    CHIP_ERROR Init(const char * file) { return ChipLinuxStorageDelegate()->Init(file); }

    CHIP_ERROR _Get(const char * key, void * value, size_t value_size, size_t * read_bytes_size = nullptr, size_t offset = 0);
    CHIP_ERROR _Delete(const char * key);
    CHIP_ERROR _Put(const char * key, const void * value, size_t value_size);

private:
    static DeviceLayer::Internal::ChipLinuxStorage * ChipLinuxStorageDelegate()
    {
        return DeviceLayer::Internal::PosixConfig::GetStorageForNamespace(DeviceLayer::Internal::PosixConfig::kConfigKey_KVS);
    }
    // DeviceLayer::Internal::ChipLinuxStorage mStorage;

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
