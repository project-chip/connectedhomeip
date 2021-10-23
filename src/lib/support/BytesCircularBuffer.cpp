/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <lib/support/BytesCircularBuffer.h>

#include <algorithm>
#include <limits>
#include <nlassert.h>
#include <string.h>

#include <lib/support/CodeUtils.h>

namespace chip {

size_t BytesCircularBuffer::Advance(size_t dataLocation, size_t amount) const
{
    dataLocation += amount;
    if (dataLocation >= mCapacity)
        dataLocation -= mCapacity;
    return dataLocation;
}

void BytesCircularBuffer::Read(uint8_t * dest, size_t length, size_t offset) const
{
    // length is an instance of SizeType
    // offset is maximized at sizeof(SizeType) for all use cases.
    static_assert(std::numeric_limits<SizeType>::max() < std::numeric_limits<size_t>::max() - sizeof(SizeType),
                  "SizeType too large, may cause overflow");
    VerifyOrDie(StorageUsed() >= offset + length);

    size_t start       = Advance(mDataStart, offset);
    size_t firstPiece  = std::min(mCapacity - start, length);
    size_t secondPiece = length - firstPiece;
    ::memcpy(dest, mStorage + start, firstPiece);
    ::memcpy(dest + firstPiece, mStorage, secondPiece);
}

void BytesCircularBuffer::Write(const uint8_t * source, size_t length)
{
    // Always reserve 1 byte to prevent mDataStart == mDataEnd because then it would be
    // ambiguous whether we have 0 bytes or mCapacity bytes stored.
    VerifyOrDie(StorageAvailable() - 1 >= length);

    size_t firstPiece  = std::min(mCapacity - mDataEnd, length);
    size_t secondPiece = length - firstPiece;
    ::memcpy(mStorage + mDataEnd, source, firstPiece);
    ::memcpy(mStorage, source + firstPiece, secondPiece);
    mDataEnd = Advance(mDataEnd, length);
}

void BytesCircularBuffer::Drop(size_t length)
{
    VerifyOrDie(StorageUsed() >= length);
    mDataStart = Advance(mDataStart, length);
}

size_t BytesCircularBuffer::StorageAvailable() const
{
    return mCapacity - StorageUsed();
}

size_t BytesCircularBuffer::StorageUsed() const
{
    if (mDataStart <= mDataEnd)
    {
        return mDataEnd - mDataStart;
    }
    else
    {
        return mCapacity + mDataEnd - mDataStart;
    }
}

CHIP_ERROR BytesCircularBuffer::Push(const ByteSpan & payload)
{
    size_t length = payload.size();
    if (length > std::numeric_limits<SizeType>::max())
        return CHIP_ERROR_INVALID_ARGUMENT;

    static_assert(std::numeric_limits<SizeType>::max() < std::numeric_limits<size_t>::max() - (sizeof(SizeType) + 1),
                  "SizeType too large, may cause overflow");
    size_t storageNeed = length + sizeof(SizeType) + 1;
    if (storageNeed > mCapacity)
        return CHIP_ERROR_INVALID_ARGUMENT;

    // Free up space until there is enough space.
    while (storageNeed > StorageAvailable())
    {
        VerifyOrDie(Pop() == CHIP_NO_ERROR);
    }

    SizeType size = static_cast<SizeType>(length);
    Write(reinterpret_cast<uint8_t *>(&size), sizeof(size));
    Write(payload.data(), length);

    return CHIP_NO_ERROR;
}

CHIP_ERROR BytesCircularBuffer::Pop()
{
    if (IsEmpty())
        return CHIP_ERROR_INCORRECT_STATE;

    size_t length = GetFrontSize();
    Drop(sizeof(SizeType));
    Drop(length);

    return CHIP_NO_ERROR;
}

bool BytesCircularBuffer::IsEmpty() const
{
    return StorageUsed() == 0;
}

size_t BytesCircularBuffer::GetFrontSize() const
{
    if (IsEmpty())
        return 0;

    SizeType length;
    Read(reinterpret_cast<uint8_t *>(&length), sizeof(length), 0 /* offset */);
    return length;
}

CHIP_ERROR BytesCircularBuffer::ReadFront(MutableByteSpan & dest) const
{
    if (IsEmpty())
        return CHIP_ERROR_INCORRECT_STATE;

    size_t length = GetFrontSize();
    if (dest.size() < length)
        return CHIP_ERROR_INVALID_ARGUMENT;

    dest = dest.SubSpan(0, length);

    Read(dest.data(), length, sizeof(SizeType) /* offset */);
    return CHIP_NO_ERROR;
}

} // namespace chip
