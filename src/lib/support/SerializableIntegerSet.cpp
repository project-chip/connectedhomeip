/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "SerializableIntegerSet.h"

#include <lib/core/CHIPEncoding.h>
#include <lib/support/CodeUtils.h>

namespace chip {

CHIP_ERROR SerializableU64SetBase::Deserialize(ByteSpan serialized)
{
    VerifyOrReturnError(serialized.size() <= MaxSerializedSize(), CHIP_ERROR_INVALID_ARGUMENT);
    memcpy(mData, serialized.data(), serialized.size());
    mNextAvailable = static_cast<uint16_t>(serialized.size() / sizeof(uint64_t));

    // Our serialized data is always little-endian; swap to native.
    SwapByteOrderIfNeeded();
    return CHIP_NO_ERROR;
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
    VerifyOrReturnError(value != mEmptyValue, CHIP_ERROR_INVALID_ARGUMENT);

    const uint16_t index = FirstAvailableForUniqueId(value);
    if (index < mCapacity)
    {
        mData[index] = value;
        if (index == mNextAvailable)
        {
            mNextAvailable = static_cast<uint16_t>(index + 1);
        }
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_NO_MEMORY;
}

void SerializableU64SetBase::Remove(uint64_t value)
{
    if (value != mEmptyValue)
    {
        const uint16_t index = FindIndex(value);
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
