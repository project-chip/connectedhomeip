/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <climits>
#include <stdint.h>
#include <string.h>

namespace chip {
namespace Encoding {

template <class Derived>
class BufferWriterBase
{
public:
    /// Append a null terminated string, exclude the null terminator
    /// This does NOT care about endianess
    BufferWriterBase & Put(const char * s)
    {
        static_assert(CHAR_BIT == 8, "We're assuming char and uint8_t are the same size");
        while (*s != 0)
        {
            Put8(static_cast<uint8_t>(*s++));
        }
        return *this;
    }

    /// Raw append a buffer, regardless of endianess
    BufferWriterBase & Put(const void * buf, size_t len)
    {
        size_t available = Available();

        if (available > 0)
        {
            memmove(mBuf + mNeeded, buf, available < len ? available : len);
        }

        mNeeded += len;

        return *this;
    }

    BufferWriterBase & Skip(size_t len)
    {
        mNeeded += len;
        return *this;
    }

    /// Append a single byte
    BufferWriterBase & Put(uint8_t c)
    {
        if (mNeeded < mSize)
        {
            mBuf[mNeeded] = c;
        }
        ++mNeeded;
        return *this;
    }

    // write an integer into a buffer, in an endian specific way

    BufferWriterBase & Put8(uint8_t c) { return static_cast<Derived *>(this)->Put(c); }
    BufferWriterBase & Put16(uint16_t x) { return static_cast<Derived *>(this)->EndianPut(x, sizeof(x)); }
    BufferWriterBase & Put32(uint32_t x) { return static_cast<Derived *>(this)->EndianPut(x, sizeof(x)); }
    BufferWriterBase & Put64(uint64_t x) { return static_cast<Derived *>(this)->EndianPut(x, sizeof(x)); }

    /// Number of bytes required to satisfy all calls to Put() so far
    size_t Needed() const { return mNeeded; }

    /// Number of bytes still available for writing
    size_t Available() const { return mSize < mNeeded ? 0 : mSize - mNeeded; }

    /// Whether the input fit in the buffer
    bool Fit() const
    {
        size_t _;
        return Fit(_);
    }

    /// Returns whether the input fit in the buffer, outputs what was actually written
    bool Fit(size_t & actuallyWritten) const
    {
        actuallyWritten = mSize >= mNeeded ? mNeeded : mSize;
        return mSize >= mNeeded;
    }

    /// Size of the output buffer
    size_t Size() const { return mSize; }

    uint8_t * Buffer() { return mBuf; }
    const uint8_t * Buffer() const { return mBuf; }

protected:
    friend Derived;

    uint8_t * mBuf;
    size_t mSize;
    size_t mNeeded;

    BufferWriterBase(uint8_t * buf, size_t len) : mBuf(buf), mSize(len), mNeeded(0) {}
    BufferWriterBase(const BufferWriterBase & other) = default;
    BufferWriterBase & operator=(const BufferWriterBase & other) = default;
};

namespace LittleEndian {

class BufferWriter : public BufferWriterBase<BufferWriter>
{
public:
    BufferWriter(uint8_t * buf, size_t len) : BufferWriterBase<BufferWriter>(buf, len) {}
    BufferWriter(const BufferWriter & other) = default;
    BufferWriter & operator=(const BufferWriter & other) = default;

    BufferWriter & EndianPut(uint64_t x, size_t size)
    {
        while (size-- > 0)
        {
            uint8_t c = x & 0xff;
            Put(c);
            x >>= 8;
        }
        return *this;
    }
};

} // namespace LittleEndian

namespace BigEndian {

class BufferWriter : public BufferWriterBase<BufferWriter>
{
public:
    BufferWriter(uint8_t * buf, size_t len) : BufferWriterBase<BufferWriter>(buf, len) {}
    BufferWriter(const BufferWriter & other) = default;
    BufferWriter & operator=(const BufferWriter & other) = default;

    BufferWriter & EndianPut(uint64_t x, size_t size)
    {
        while (size-- > 0)
        {
            uint8_t c = (x >> (size * 8)) & 0xff;
            Put(c);
        }
        return *this;
    }
};

} // namespace BigEndian

} // namespace Encoding
} // namespace chip
