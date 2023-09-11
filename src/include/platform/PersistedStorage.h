/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2016-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
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

#include <lib/core/CHIPConfig.h>
#include <lib/core/CHIPError.h>

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

} // namespace PersistedStorage
} // namespace Platform
} // namespace chip
