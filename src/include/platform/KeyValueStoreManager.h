/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 * @file
 *
 * @brief
 *   API function declarations for using persistent key value storage.
 */

#pragma once

#include <climits>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#include <lib/core/CHIPError.h>
#include <platform/CHIPDeviceBuildConfig.h>

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {
class KeyValueStoreManagerImpl;

class KeyValueStoreManager
{
public:
    /**
     * @brief
     *   Reads the value of an entry in the KVS. The value is read into the
     *   provided buffer and the number of bytes read is returned. If desired,
     *   the read can be started at an offset.
     *
     *   If the output buffer is too small for the value, Get returns
     *   CHIP_ERROR_BUFFER_TOO_SMALL with the number of bytes read returned in
     *   read_bytes_size, which should be the buffer_size.
     *
     *   The remainder of the  value can be read by calling get with an offset.
     *
     * @param[in]     key               The name of the key to get, this is a
     *                                  null-terminated string.
     * @param[in,out] buffer            A buffer to read the value into.
     * @param[in]     buffer_size       The size of the buffer in bytes.
     * @param[in]     read_bytes_size   The number of bytes which were
     *                                  copied into the buffer. Optionally can
     *                                  provide nullptr if not needed.
     * @param[in]     offset_bytes      The offset byte index to start the read.
     *
     * @return CHIP_NO_ERROR the entry was successfully read
     *         CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND the key is not
     *                                                       present in the KVS
     *         CHIP_ERROR_INTEGRITY_CHECK_FAILED found the entry, but the data
     *                                           was corrupted
     *         CHIP_ERROR_BUFFER_TOO_SMALL the buffer could not fit the entire
     *                                     value, but as many bytes as possible
     *                                     were written to it
     *         CHIP_ERROR_WELL_UNINITIALIZED the KVS is not initialized
     *         CHIP_ERROR_INVALID_ARGUMENT key is empty or too long or value is
     *                                     too large
     */
    CHIP_ERROR Get(const char * key, void * buffer, size_t buffer_size, size_t * read_bytes_size = nullptr,
                   size_t offset_bytes = 0);

    /**
     * @brief
     * This overload of Get accepts a pointer to a trivially copyable object.
     * The size of the object is inferred from the type.
     *
     * @param[in]      key     The name of the key in to get, this is a
     *                         null-terminated string.
     * @param[in,out]  value   Pointer to a trivially copyable object.
     *
     * @return CHIP_NO_ERROR the entry was successfully read
     *         CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND the key is not
     *                                                      present in the KVS
     *         CHIP_ERROR_INTEGRITY_CHECK_FAILED found the entry, but the data
     *                                           was corrupted
     *         CHIP_ERROR_BUFFER_TOO_SMALL the buffer could not fit the entire
     *                                     value, but as many bytes as possible
     *                                     were written to it
     *         CHIP_ERROR_WELL_UNINITIALIZED the KVS is not initialized
     *         CHIP_ERROR_INVALID_ARGUMENT key is empty or too long or value is
     *                                     too large
     */
    template <typename T>
    CHIP_ERROR Get(const char * key, T * value)
    {
        static_assert(std::is_trivially_copyable<T>(), "KVS values must copyable");
        static_assert(!std::is_pointer<T>(), "KVS values cannot be pointers");
        static_assert(CHAR_BIT == 8, "Current implementation assumes 8 bit.");
        return Get(key, value, sizeof(T));
    }

    /**
     * @brief
     * Adds a key-value entry to the KVS. If the key was already present, its
     * value is overwritten.
     *
     * @param[in]  key          The name of the key to update, this is a
     *                          null-terminated string.
     * @param[in]  value        Pointer to the data.
     * @param[in]  value_size   Size of the data.
     *
     * @return CHIP_NO_ERROR the entry was successfully added or updated
     *         CHIP_ERROR_INTEGRITY_CHECK_FAILED checksum validation failed after
     *                                           writing the data
     *         CHIP_ERROR_PERSISTED_STORAGE_FAILED failed to write the value.
     *         CHIP_ERROR_WELL_UNINITIALIZED the KVS is not initialized
     *         CHIP_ERROR_INVALID_ARGUMENT key is empty or too long or value is
     *                                     too large
     */
    CHIP_ERROR Put(const char * key, const void * value, size_t value_size);

    /**
     * @brief
     * This overload of Put accepts a reference to a trivially copyable object.
     * The size of the object is inferred from the type.
     *
     * @param[in]  key     The name of the key to update, this is a
     *                     null-terminated string.
     * @param[in]  value   Reference of a trivially copyable object.
     *
     * @return CHIP_NO_ERROR the entry was successfully added or updated
     *         CHIP_ERROR_INTEGRITY_CHECK_FAILED checksum validation failed after
     *                                           writing the data
     *         CHIP_ERROR_PERSISTED_STORAGE_FAILED failed to write the value.
     *         CHIP_ERROR_WELL_UNINITIALIZED the KVS is not initialized
     *         CHIP_ERROR_INVALID_ARGUMENT key is empty or too long or value is
     *                                     too large
     */
    template <typename T>
    CHIP_ERROR Put(const char * key, const T & value)
    {
        static_assert(std::is_trivially_copyable<T>(), "KVS values must copyable");
        static_assert(!std::is_pointer<T>(), "KVS values cannot be pointers");
        static_assert(CHAR_BIT == 8, "Current implementation assumes 8 bit.");
        return Put(key, &value, sizeof(T));
    }

    /**
     * @brief
     * Removes a key-value entry from the KVS.
     *
     * @param[in]      key       The name of the key to delete, this is a
     *                           null-terminated string.
     *
     * @return CHIP_NO_ERROR the entry was successfully deleted.
     *         CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND the key is not
     *                                                      present in the KVS
     *         CHIP_ERROR_INTEGRITY_CHECK_FAILED checksum validation failed after
     *                                           erasing data
     *         CHIP_ERROR_PERSISTED_STORAGE_FAILED failed to erase the value.
     *         CHIP_ERROR_WELL_UNINITIALIZED the KVS is not initialized
     *         CHIP_ERROR_INVALID_ARGUMENT key is empty or too long
     */
    CHIP_ERROR Delete(const char * key);

private:
    using ImplClass = ::chip::DeviceLayer::PersistedStorage::KeyValueStoreManagerImpl;

protected:
    // Construction/destruction limited to subclasses.
    KeyValueStoreManager()  = default;
    ~KeyValueStoreManager() = default;

    // No copy, move or assignment.
    KeyValueStoreManager(const KeyValueStoreManager &)  = delete;
    KeyValueStoreManager(const KeyValueStoreManager &&) = delete;
    KeyValueStoreManager & operator=(const KeyValueStoreManager &) = delete;
};

/**
 * Returns a reference to the public interface of the KeyValueStoreManager singleton object.
 *
 * chip application should use this to access features of the KeyValueStoreManager object
 * that are common to all platforms.
 */
extern KeyValueStoreManager & KeyValueStoreMgr();

/**
 * Returns the platform-specific implementation of the KeyValueStoreManager singleton object.
 *
 * chip applications can use this to gain access to features of the KeyValueStoreManager
 * that are specific to the selected platform.
 */
extern KeyValueStoreManagerImpl & KeyValueStoreMgrImpl();

} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip

/* Include a header file containing the implementation of the KeyValueStoreManager
 * object for the selected platform.
 */
#ifdef EXTERNAL_KEYVALUESTOREMANAGERIMPL_HEADER
#include EXTERNAL_KEYVALUESTOREMANAGERIMPL_HEADER
#elif defined(CHIP_DEVICE_LAYER_TARGET)
#define KEYVALUESTOREMANAGERIMPL_HEADER <platform/CHIP_DEVICE_LAYER_TARGET/KeyValueStoreManagerImpl.h>
#include KEYVALUESTOREMANAGERIMPL_HEADER
#endif // defined(CHIP_DEVICE_LAYER_TARGET)

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {
inline CHIP_ERROR KeyValueStoreManager::Put(const char * key, const void * value, size_t value_size)
{
    return static_cast<ImplClass *>(this)->_Put(key, value, value_size);
}

inline CHIP_ERROR KeyValueStoreManager::Get(const char * key, void * buffer, size_t buffer_size, size_t * read_bytes_size,
                                            size_t offset_bytes)
{
    return static_cast<ImplClass *>(this)->_Get(key, buffer, buffer_size, read_bytes_size, offset_bytes);
}

inline CHIP_ERROR KeyValueStoreManager::Delete(const char * key)
{
    return static_cast<ImplClass *>(this)->_Delete(key);
}

} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
