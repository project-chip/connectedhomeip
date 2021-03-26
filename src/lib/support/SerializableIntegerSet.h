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

#include <support/Base64.h>
#include <support/CodeUtils.h>

// BASE64_ENCODED_LEN doesn't account for null termination of the string.
// So, we are adding 1 extra byte to the size requirement.
#define CHIP_MAX_SERIALIZED_SIZE_U64(count) static_cast<uint16_t>(BASE64_ENCODED_LEN(sizeof(uint64_t) * (count)) + 1)

namespace chip {

class SerializableU64SetBase
{

public:
    SerializableU64SetBase(uint64_t * data, uint16_t capacity, uint64_t emptyValue) :
        mData(data), mCapacity(capacity), mEmptyValue(emptyValue), mNextAvailable(0)
    {
        for (uint16_t i = 0; i < capacity; i++)
        {
            data[i] = emptyValue;
        }
    }

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
    uint16_t SerializedSize() { return CHIP_MAX_SERIALIZED_SIZE_U64(mNextAvailable); }

    /**
     * @brief
     *   Get the maximum length of string if the array were full and serialized.
     */
    uint16_t MaxSerializedSize() { return CHIP_MAX_SERIALIZED_SIZE_U64(mCapacity); }

    /**
     * @brief
     *   Check if the value is in the array.
     *
     * @param[in] value Value to find
     * @return True, if it's prsent in the array.
     */
    bool Contains(uint64_t value) { return FindIndex(value) != mCapacity; }

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
    const uint64_t mEmptyValue;
    uint16_t mNextAvailable;

    uint16_t FirstAvailableForUniqueId(uint64_t value);

    /**
     * @brief
     *   Find index of the value in the array.
     *
     * @param[in] value Value to find
     * @return index of the value if found, or max length (mCapacity) of the array
     */
    uint16_t FindIndex(uint64_t value);

    void SwapByteOrderIfNeeded();
};

template <uint16_t kCapacity, uint64_t kEmptyValue = 0>
class SerializableU64Set : public SerializableU64SetBase
{
public:
    SerializableU64Set() : SerializableU64SetBase(mBuffer, kCapacity, kEmptyValue)
    {
        /**
         * Check that requested capacity (kCapacity) will not exceed maximum number of uint64_t
         * values that can fit in a meory of size UINT16_MAX. This is required, since APIs in
         * this class are using uint16_t type for buffer sizes.
         */
        nlSTATIC_ASSERT_PRINT(kCapacity < UINT16_MAX / sizeof(uint64_t),
                              "Serializable u64 set capacity cannot be more than UINT16_MAX / sizeof(uint64_t)");
    }

private:
    uint64_t mBuffer[kCapacity];
};

} // namespace chip
