/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
    while (size > 0)
    {
        size--;
        Put(static_cast<uint8_t>((x >> (size * 8)) & 0xff));
    }
    return *this;
}

BigEndian::BufferWriter & BigEndian::BufferWriter::EndianPutSigned(int64_t x, size_t size)
{
    while (size > 0)
    {
        size--;
        Put(static_cast<uint8_t>((x >> (size * 8)) & 0xff));
    }
    return *this;
}

} // namespace Encoding
} // namespace chip
