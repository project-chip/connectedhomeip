/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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

#pragma once

#include <stddef.h>

#include <core/CHIPEncoding.h>
#include <core/CHIPError.h>
#include <support/Base64.h>
#include <support/CodeUtils.h>
#include <transport/raw/MessageHeader.h>

namespace chip {

template <uint16_t Capacity>
class SparseU64Array
{

public:
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
    const char * Serialize(char * buf, uint16_t & buflen)
    {
        if (buflen < SerializedSize() || buf == nullptr)
        {
            buflen = SerializedSize();
            return nullptr;
        }

        buflen = Base64Encode(reinterpret_cast<const uint8_t *>(mArray), sizeof(uint64_t) * mNextAvailable, buf);
        return buf;
    }

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
    CHIP_ERROR Deserialize(const char * serialized, uint16_t buflen)
    {
        CHIP_ERROR err     = CHIP_NO_ERROR;
        uint16_t decodelen = 0;
        VerifyOrExit(buflen <= MaxSerializedSize(), err = CHIP_ERROR_INVALID_ARGUMENT);

        decodelen = Base64Decode(serialized, buflen, reinterpret_cast<uint8_t *>(mArray));
        VerifyOrExit(decodelen <= sizeof(uint64_t) * Capacity, err = CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrExit(decodelen % sizeof(uint64_t) == 0, err = CHIP_ERROR_INVALID_ARGUMENT);

        mNextAvailable = decodelen / static_cast<uint16_t>(sizeof(uint64_t));
    exit:
        return err;
    }

    /**
     * @brief
     *   Get the length of string if the array is serialized.
     */
    uint16_t SerializedSize() { return BASE64_ENCODED_LEN(sizeof(uint64_t) * mNextAvailable); }

    /**
     * @brief
     *   Get the maximum length of string if the array was full and serialized.
     */
    uint16_t MaxSerializedSize() { return BASE64_ENCODED_LEN(sizeof(uint64_t) * Capacity); }

    /**
     * @brief
     *   Find a value in the array.
     *
     * @param[in] value Value to find
     * @return index of the value if found, or max length (Capacity) of the array
     */
    uint16_t Find(uint64_t value)
    {
        uint64_t valueLE = ToLE64(value);
        for (uint16_t i = 0; i < mNextAvailable; i++)
        {
            if (mArray[i] == valueLE)
            {
                return i;
            }
        }

        return Capacity;
    }

    /**
     * @brief
     *   Insert the value in the array. If the value is duplicate, it
     *   won't be inserted.
     *
     * @return CHIP_NO_ERROR in case of success, or the error code
     */
    CHIP_ERROR Insert(uint64_t value)
    {
        CHIP_ERROR err = CHIP_ERROR_INTERNAL;
        uint16_t index = FirstAvailableForUniqueId(value);
        if (index < Capacity)
        {
            mArray[index] = ToLE64(value);
            if (index == mNextAvailable)
            {
                mNextAvailable = index + 1;
            }
            err = CHIP_NO_ERROR;
        }
        return err;
    }

    /**
     * @brief
     *   Delete the value from the array.
     */
    void Remove(uint64_t value)
    {
        uint16_t index = Find(value);
        if (index < Capacity)
        {
            mArray[index] = 0;
            if ((index + 1) == mNextAvailable)
            {
                mNextAvailable = index;
                while (mNextAvailable > 0 && mArray[mNextAvailable - 1] == 0)
                    mNextAvailable--;
            }
        }
    }

private:
    uint64_t mArray[Capacity];
    uint16_t mNextAvailable = 0;

    uint16_t FirstAvailableForUniqueId(uint64_t value)
    {
        uint64_t valueLE   = ToLE64(value);
        uint16_t available = mNextAvailable;
        for (uint16_t i = 0; i < mNextAvailable; i++)
        {
            if (mArray[i] == valueLE)
            {
                return i;
            }

            if (mArray[i] == 0 && i < available)
            {
                available = i;
            }
        }

        return available;
    }

    uint64_t ToLE64(uint64_t value) { return Encoding::LittleEndian::Get64(reinterpret_cast<const uint8_t *>(&value)); }
};

} // namespace chip
