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

#include "SparseArray.h"

namespace chip {

const char * SparseU64ArrayBase::SerializeBase64(char * buf, uint16_t & buflen)
{
    char * out = nullptr;
    size_t len = sizeof(uint64_t) * mNextAvailable;
    VerifyOrExit(buflen >= SerializedSize(), buflen = SerializedSize());
    VerifyOrExit(buf != nullptr, buflen = SerializedSize());

    buflen = Base64Encode(reinterpret_cast<const uint8_t *>(mData), static_cast<uint16_t>(len), buf);
    out    = buf;

exit:
    return out;
}

CHIP_ERROR SparseU64ArrayBase::DeserializeBase64(const char * serialized, uint16_t buflen)
{
    CHIP_ERROR err     = CHIP_NO_ERROR;
    uint16_t decodelen = 0;
    VerifyOrExit(buflen <= MaxSerializedSize(), err = CHIP_ERROR_INVALID_ARGUMENT);

    decodelen = Base64Decode(serialized, buflen, reinterpret_cast<uint8_t *>(mData));
    VerifyOrExit(decodelen <= sizeof(uint64_t) * mCapacity, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(decodelen % sizeof(uint64_t) == 0, err = CHIP_ERROR_INVALID_ARGUMENT);

    mNextAvailable = decodelen / static_cast<uint16_t>(sizeof(uint64_t));
exit:
    return err;
}

uint16_t SparseU64ArrayBase::Find(uint64_t value)
{
    uint64_t valueLE = ToLE64(value);
    for (uint16_t i = 0; i < mNextAvailable; i++)
    {
        if (mData[i] == valueLE)
        {
            return i;
        }
    }

    return mCapacity;
}

CHIP_ERROR SparseU64ArrayBase::Insert(uint64_t value)
{
    CHIP_ERROR err = CHIP_ERROR_NO_MEMORY;
    uint16_t index = FirstAvailableForUniqueId(value);
    if (index < mCapacity)
    {
        mData[index] = ToLE64(value);
        if (index == mNextAvailable)
        {
            mNextAvailable = static_cast<uint16_t>(index + 1);
        }
        err = CHIP_NO_ERROR;
    }
    return err;
}

void SparseU64ArrayBase::Remove(uint64_t value)
{
    uint16_t index = Find(value);
    if (index < mCapacity)
    {
        mData[index] = 0;
        if ((index + 1) == mNextAvailable)
        {
            mNextAvailable = index;
            while (mNextAvailable > 0 && mData[mNextAvailable - 1] == 0)
                mNextAvailable--;
        }
    }
}

uint16_t SparseU64ArrayBase::FirstAvailableForUniqueId(uint64_t value)
{
    uint64_t valueLE   = ToLE64(value);
    uint16_t available = mNextAvailable;
    for (uint16_t i = 0; i < mNextAvailable; i++)
    {
        if (mData[i] == valueLE)
        {
            return i;
        }

        if (mData[i] == 0 && i < available)
        {
            available = i;
        }
    }

    return available;
}

} // namespace chip
