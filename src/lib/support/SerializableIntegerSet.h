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

#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>

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
     *   Serialize the sparse array by calling a callback with a ByteSpan to
     *   serialize.  We ensure that this ByteSpan is architecture-agnostic, so
     *   it can be deserialized anywhere later.
     *
     *   Only the values till mNextAvailable index are encoded.
     *   The empty indexes between 0, and mNextAvailable, are also
     *   encoded.
     *
     * @param[in] callback the serialization callback to call.
     */
    template <typename F>
    CHIP_ERROR Serialize(F callback)
    {
        // Ensure that we are holding little-endian data while the serialization
        // callback runs.
        SwapByteOrderIfNeeded();

        CHIP_ERROR err = callback(ByteSpan(reinterpret_cast<uint8_t *>(mData), SerializedSize()));

        SwapByteOrderIfNeeded();
        return err;
    }

    /**
     * @brief
     *   Deserialize a previously serialized byte buffer into the sparse array.
     *   The mNextAvailable index is calculated based on how many
     *   values are in the deserialized array.
     *
     * @param[in] serialized Serialized buffer
     */
    CHIP_ERROR Deserialize(ByteSpan serialized);

    /**
     * @brief
     *   Get the length of the byte data if the array is serialized.
     */
    size_t SerializedSize() const { return sizeof(uint64_t) * mNextAvailable; }

    /**
     * @brief
     *   Get the maximum length of the byte data if the array were full and serialized.
     */
    size_t MaxSerializedSize() const { return sizeof(uint64_t) * mCapacity; }

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
