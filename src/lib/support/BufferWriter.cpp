/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "BufferWriter.h"

namespace chip {
namespace Encoding {

BufferWriter & BufferWriter::Put(const char * s)
{
    static_assert(CHAR_BIT == 8, "We're assuming char and uint8_t are the same size");
    return Put(s, strlen(s));
}

BufferWriter & BufferWriter::Put(const void * buf, size_t len)
{
    size_t available = Available();

    if (available > 0)
    {
        memmove(mBuf + mNeeded, buf, available < len ? available : len);
    }

    mNeeded += len;
    return *this;
}

BufferWriter & BufferWriter::Put(uint8_t c)
{
    if (mNeeded < mSize)
    {
        mBuf[mNeeded] = c;
    }
    ++mNeeded;
    return *this;
}

LittleEndian::BufferWriter & LittleEndian::BufferWriter::EndianPut(uint64_t x, size_t size)
{
    while (size > 0)
    {
        Put(static_cast<uint8_t>(x & 0xff));
        x >>= 8;
        size--;
    }
    return *this;
}

LittleEndian::BufferWriter & LittleEndian::BufferWriter::EndianPutSigned(int64_t x, size_t size)
{
    while (size > 0)
    {
        Put(static_cast<uint8_t>(x & 0xff));
        x >>= 8;
        size--;
    }
    return *this;
}

BigEndian::BufferWriter & BigEndian::BufferWriter::EndianPut(uint64_t x, size_t size)
{
    while (size-- > 0)
    {
        Put(static_cast<uint8_t>((x >> (size * 8)) & 0xff));
    }
    return *this;
}

BigEndian::BufferWriter & BigEndian::BufferWriter::EndianPutSigned(int64_t x, size_t size)
{
    while (size-- > 0)
    {
        Put(static_cast<uint8_t>((x >> (size * 8)) & 0xff));
    }
    return *this;
}

} // namespace Encoding
} // namespace chip
