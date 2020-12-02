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

#include "SerializableIntegerSet.h"

#include <core/CHIPEncoding.h>

namespace chip {

const char * SerializableU64SetBase::SerializeBase64(char * buf, uint16_t & buflen)
{
    char * out = nullptr;
    size_t len = sizeof(uint64_t) * mNextAvailable;
    VerifyOrExit(buflen >= SerializedSize(), buflen = SerializedSize());
    VerifyOrExit(buf != nullptr, buflen = SerializedSize());

    // Swap to little endian order if needed.
    SwapByteOrderIfNeeded();

    buflen      = Base64Encode(reinterpret_cast<const uint8_t *>(mData), static_cast<uint16_t>(len), buf);
    buf[buflen] = '\0';
    out         = buf;

    // Swap back to the original order
    SwapByteOrderIfNeeded();

exit:
    return out;
}

CHIP_ERROR SerializableU64SetBase::DeserializeBase64(const char * serialized, uint16_t buflen)
{
    CHIP_ERROR err     = CHIP_NO_ERROR;
    uint16_t decodelen = 0;
    VerifyOrExit(buflen <= MaxSerializedSize(), err = CHIP_ERROR_INVALID_ARGUMENT);

    decodelen = Base64Decode(serialized, buflen, reinterpret_cast<uint8_t *>(mData));
    VerifyOrExit(decodelen <= sizeof(uint64_t) * mCapacity, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(decodelen % sizeof(uint64_t) == 0, err = CHIP_ERROR_INVALID_ARGUMENT);

    mNextAvailable = decodelen / static_cast<uint16_t>(sizeof(uint64_t));

    // Swap from little endian if needed
    SwapByteOrderIfNeeded();

exit:
    return err;
}

void SerializableU64SetBase::SwapByteOrderIfNeeded()
{
    /**
     *  The data is serialized in LittleEndian byte order in the set. This will enable
     *  different machine architectures to interpret a given set in a consistent manner,
     *  for serialize and deserialize operations.
     */
    if (nl::ByteOrder::GetCurrent() != nl::ByteOrder::LittleEndian)
    {
        for (uint16_t i = 0; i < mNextAvailable; i++)
        {
            mData[i] = Encoding::LittleEndian::HostSwap64(mData[i]);
        }
    }
}

uint16_t SerializableU64SetBase::FindIndex(uint64_t value)
{
    for (uint16_t i = 0; i < mNextAvailable; i++)
    {
        if (mData[i] == value)
        {
            return i;
        }
    }

    return mCapacity;
}

CHIP_ERROR SerializableU64SetBase::Insert(uint64_t value)
{
    CHIP_ERROR err = CHIP_ERROR_NO_MEMORY;
    uint16_t index = 0;
    VerifyOrExit(value != mEmptyValue, err = CHIP_ERROR_INVALID_ARGUMENT);

    index = FirstAvailableForUniqueId(value);
    if (index < mCapacity)
    {
        mData[index] = value;
        if (index == mNextAvailable)
        {
            mNextAvailable = static_cast<uint16_t>(index + 1);
        }
        err = CHIP_NO_ERROR;
    }

exit:
    return err;
}

void SerializableU64SetBase::Remove(uint64_t value)
{
    if (value != mEmptyValue)
    {
        uint16_t index = FindIndex(value);
        if (index < mCapacity)
        {
            mData[index] = mEmptyValue;
            if ((index + 1) == mNextAvailable)
            {
                mNextAvailable = index;
                while (mNextAvailable > 0 && mData[mNextAvailable - 1] == mEmptyValue)
                    mNextAvailable--;
            }
        }
    }
}

uint16_t SerializableU64SetBase::FirstAvailableForUniqueId(uint64_t value)
{
    uint16_t available = mNextAvailable;
    for (uint16_t i = 0; i < mNextAvailable; i++)
    {
        if (mData[i] == value)
        {
            return i;
        }

        if (mData[i] == mEmptyValue && i < available)
        {
            // Don't return here, as we want to make sure there are no duplicate
            // entries in the set.
            available = i;
        }
    }

    return available;
}

} // namespace chip
