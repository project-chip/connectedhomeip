/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      Defines a data structure that can store a set of uint64_t values
 *      in not contiguous indexes. The set is initialized with a constant
 *      capacity. The data structure supports serialize/deserialize operations,
 *      where, serialize converts the set to a base64 string, and deserializing
 *      the base64 string reconstructs the original set.
 *
 *      The data is stored such that serialized data can be deserialized correctly
 *      on different machine architectures.
 *
 */

#pragma once

#include <core/CHIPEncoding.h>
#include <support/Base64.h>
#include <support/CodeUtils.h>

namespace chip {

class SerializableU64SetBase
{

public:
    SerializableU64SetBase(uint64_t * data, uint16_t capacity) : mData(data), mCapacity(capacity), mNextAvailable(0) {}

    /**
     * @brief
     *   Serialize the sparse array into a base 64 encoded string.
     *   Only the values till mNextAvailable index are encoded.
     *   The empty indexes between 0, and mNextAvailable, are also
     *   encoded.
     *
     * @param[in] buf Buffer where serialized string is written
     * @param[in] buflen Length of buf
     * @return pointer to buf, or nullptr in case of error
     */
    const char * SerializeBase64(char * buf, uint16_t & buflen);

    /**
     * @brief
     *   Deserialize a base64 encoded string into the sparse array.
     *   The mNextAvailable index is calculated based on how many
     *   values are in the deserialized array.
     *
     * @param[in] serialized Serialized buffer
     * @param[in] buflen Length of buffer
     * @return CHIP_NO_ERROR in case of success, or the error code
     */
    CHIP_ERROR DeserializeBase64(const char * serialized, uint16_t buflen);

    /**
     * @brief
     *   Get the length of string if the array is serialized.
     */
    uint16_t SerializedSize() { return static_cast<uint16_t>(BASE64_ENCODED_LEN(sizeof(uint64_t) * mNextAvailable)); }

    /**
     * @brief
     *   Get the maximum length of string if the array were full and serialized.
     */
    uint16_t MaxSerializedSize() { return static_cast<uint16_t>(BASE64_ENCODED_LEN(sizeof(uint64_t) * mCapacity)); }

    /**
     * @brief
     *   Check if the value is in the array.
     *
     * @param[in] value Value to find
     * @return True, if it's prsent in the array.
     */
    bool Contains(uint64_t value) { return Find(value) != mCapacity; }

    /**
     * @brief
     *   Insert the value in the array. If the value is duplicate, it
     *   won't be inserted.
     *
     * @return CHIP_NO_ERROR in case of success, or the error code
     */
    CHIP_ERROR Insert(uint64_t value);

    /**
     * @brief
     *   Delete the value from the array.
     */
    void Remove(uint64_t value);

private:
    uint64_t * const mData;
    const uint16_t mCapacity;
    uint16_t mNextAvailable;

    uint16_t FirstAvailableForUniqueId(uint64_t value);

    /**
     * @brief
     *   Find a value in the array.
     *
     * @param[in] value Value to find
     * @return index of the value if found, or max length (mCapacity) of the array
     */
    uint16_t Find(uint64_t value);

    uint64_t ToLE64(uint64_t value) { return Encoding::LittleEndian::Get64(reinterpret_cast<const uint8_t *>(&value)); }
};

template <uint16_t kCapacity>
class SerializableU64Set : public SerializableU64SetBase
{
public:
    SerializableU64Set() : SerializableU64SetBase(mBuffer, kCapacity)
    {
        nlSTATIC_ASSERT_PRINT(kCapacity < UINT16_MAX / sizeof(uint64_t),
                              "Serializable u64 set capacity cannot be more than UINT16_MAX / sizeof(uint64_t)");
        CHIP_ZERO_AT(mBuffer);
    }

private:
    uint64_t mBuffer[kCapacity];
};

} // namespace chip
