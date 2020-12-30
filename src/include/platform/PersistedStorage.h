/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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
 *   API function declarations for reading from and writing to persistent storage.
 *   Platforms will be responsible for implementing the read/write details.
 */

#pragma once

#include <climits>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#include <core/CHIPConfig.h>
#include <core/CHIPError.h>

namespace chip {
namespace Platform {
namespace PersistedStorage {

// Persistent storage key type is const char * in core config, however
// it is uint8_t/uint16_t on other platforms (EFR32 and nRF5 respectively)
typedef CHIP_CONFIG_PERSISTED_STORAGE_KEY_TYPE Key;

namespace internal {
template <typename T>
struct EmptyKey
{
    static constexpr T value = 0; // handles numeric values
};

template <>
struct EmptyKey<const char *>
{
    static constexpr const char * value = nullptr;
};

} // namespace internal

constexpr Key kEmptyKey = internal::EmptyKey<Key>::value;

/**
 *  @brief
 *    Read integer value of a key from persistent storage.
 *    Platform is responsible for validating aKey.
 *
 *  @param[in]     aKey      A key to a persistently-stored value.
 *  @param[in,out] aValue    A reference to an integer value.
 *
 *  @return CHIP_ERROR_INVALID_ARGUMENT if aKey is NULL
 *          CHIP_ERROR_INVALID_STRING_LENGTH if aKey exceeds
 *                  CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH
 *          CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND if aKey does not exist
 *          CHIP_NO_ERROR otherwise
 */
CHIP_ERROR Read(Key aKey, uint32_t & aValue);

/**
 *  @brief
 *    Write the integer value of a key to persistent storage.
 *    Platform is responsible for validating aKey.
 *    If aKey does not exist, it will be created and assigned aValue.
 *    Otherwise any existing value of aKey will be replaced with aValue.
 *
 *  @param[in] aKey      A key to a persistently-stored value.
 *  @param[in] aValue    The value.
 *
 *  @return CHIP_ERROR_INVALID_ARGUMENT if aKey is NULL
 *          CHIP_ERROR_INVALID_STRING_LENGTH if aKey exceeds
 *                  CHIP_CONFIG_PERSISTED_STORAGE_MAX_KEY_LENGTH
 *          CHIP_NO_ERROR otherwise
 */
CHIP_ERROR Write(Key aKey, uint32_t aValue);

class KeyValueStore
{
public:
    virtual ~KeyValueStore() = default;
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
    virtual CHIP_ERROR Get(const char * key, void * buffer, size_t buffer_size, size_t * read_bytes_size = nullptr,
                           size_t offset_bytes = 0) const = 0;

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
    virtual CHIP_ERROR Put(const char * key, const void * value, size_t value_size) = 0;

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
    virtual CHIP_ERROR Delete(const char * key) = 0;
};

/**
 * @brief
 * Returns a reference to the singleton for the default Chip KVS storage.
 */
KeyValueStore & ChipKeyValueStoreInstance();

} // namespace PersistedStorage
} // namespace Platform
} // namespace chip
