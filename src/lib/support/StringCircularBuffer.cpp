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

#include <support/StringCircularBuffer.h>

#include <limits>
#include <nlassert.h>
#include <string.h>

#include <support/CodeUtils.h>

namespace chip {

std::size_t StringCircularBuffer::Advance(std::size_t dataLocation, size_t amount) const
{
    dataLocation += amount;
    if (dataLocation >= mCapacity)
        dataLocation -= mCapacity;
    return dataLocation;
}

void StringCircularBuffer::Read(uint8_t * dest, size_t length, size_t offset) const
{
    VerifyOrDie(Size() >= offset + length);

    std::size_t start = Advance(mDataStart, offset);
    if (mCapacity - start >= length)
    {
        ::memcpy(dest, mStorage + start, length);
    }
    else
    {
        size_t firstPiece  = mCapacity - start;
        size_t secondPiece = length - firstPiece;
        ::memcpy(dest, mStorage + start, firstPiece);
        ::memcpy(dest + firstPiece, mStorage, secondPiece);
    }
}

void StringCircularBuffer::Write(const uint8_t * source, size_t length)
{
    // Always reserve 1 byte to prevent mDataStart == mDataEnd.
    VerifyOrDie(mCapacity - Size() - 1 >= length);

    if (mCapacity - mDataEnd >= length)
    {
        ::memcpy(mStorage + mDataEnd, source, length);
        mDataEnd = Advance(mDataEnd, length); // Use advance in case that mDataEnd wrap to 0
    }
    else
    {
        size_t firstPiece  = mCapacity - mDataEnd;
        size_t secondPiece = length - firstPiece;
        ::memcpy(mStorage + mDataEnd, source, firstPiece);
        ::memcpy(mStorage, source + firstPiece, secondPiece);
        mDataEnd = secondPiece;
    }
}

void StringCircularBuffer::Drop(size_t length)
{
    VerifyOrDie(Size() >= length);
    mDataStart = Advance(mDataStart, length);
}

size_t StringCircularBuffer::Size() const
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

CHIP_ERROR StringCircularBuffer::Push(const ByteSpan & payload)
{
    size_t length = payload.size();
    if (length + sizeof(SizeType) + 1 > mCapacity)
        return CHIP_ERROR_INVALID_ARGUMENT;
    if (length > std::numeric_limits<SizeType>::max())
        return CHIP_ERROR_INVALID_ARGUMENT;

    // Free up space until there is enough space.
    while (length + sizeof(SizeType) + 1 > mCapacity - Size())
    {
        Pop();
    }

    SizeType size = static_cast<SizeType>(length);
    Write(reinterpret_cast<uint8_t *>(&size), sizeof(size));
    Write(payload.data(), length);

    return CHIP_NO_ERROR;
}

CHIP_ERROR StringCircularBuffer::Pop()
{
    if (IsEmpty())
        return CHIP_ERROR_INCORRECT_STATE;

    size_t length = GetFrontSize();
    Drop(sizeof(SizeType));
    Drop(length);

    return CHIP_NO_ERROR;
}

bool StringCircularBuffer::IsEmpty() const
{
    return Size() == 0;
}

size_t StringCircularBuffer::GetFrontSize() const
{
    if (IsEmpty())
        return 0;

    SizeType length;
    Read(reinterpret_cast<uint8_t *>(&length), sizeof(length), 0);
    return length;
}

CHIP_ERROR StringCircularBuffer::ReadFront(uint8_t * dest) const
{
    if (IsEmpty())
        return CHIP_ERROR_INCORRECT_STATE;

    size_t length = GetFrontSize();
    Read(dest, length, sizeof(SizeType));
    return CHIP_NO_ERROR;
}

} // namespace chip
