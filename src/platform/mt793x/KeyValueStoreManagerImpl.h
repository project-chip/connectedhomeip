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
 *          Platform-specific key value storage implementation for Genio.
 *
 */

#pragma once

#include "nvdm.h"

/* ignore GCC Wconversion warnings for pigweed */
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#endif

//#include <pw_kvs/crc16_checksum.h>
//#include <pw_kvs/flash_memory.h>
//#include <pw_kvs/key_value_store.h>

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {

/**
 *
 * MT793x KVS implementation
 */

class KeyValueStoreManagerImpl final : public KeyValueStoreManager
{
    // Allow the KeyValueStoreManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend class KeyValueStoreManager;

public:
    void Init()
    { /*nvdm_init();*/
    }

    CHIP_ERROR _Get(const char * key, void * value, size_t value_size, size_t * read_bytes_size = nullptr, size_t offset = 0) const;

    CHIP_ERROR _Delete(const char * key);

    /**
     * @brief
     * Erases all data in the KVS partition, KVS needs to be initialized after
     * this operation.
     *
     * @return CHIP_NO_ERROR the partiton was erased.
     *         CHIP_ERROR_TIMEOUT timed out while doing erase.
     *         CHIP_ERROR_ACCESS_DENIED flash locked, erase failed.
     */
    CHIP_ERROR ErasePartition();

    CHIP_ERROR _Put(const char * key, const void * value, size_t value_size);

private:
    const char * kNamespace = "CHIP_KVS";
    static CHIP_ERROR MapNvdmStatus(nvdm_status_t nvdm_status);
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
 * that are specific to the Genio platform.
 */
inline KeyValueStoreManagerImpl & KeyValueStoreMgrImpl(void)
{
    return KeyValueStoreManagerImpl::sInstance;
}

} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
