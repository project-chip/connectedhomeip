/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#include <lib/support/Span.h>

namespace chip {
namespace Encoding {

class BufferWriter
{
public:
    BufferWriter(uint8_t * buf, size_t len) : mBuf(buf), mSize(len), mNeeded(0)
    {
        if (buf == nullptr)
        {
            mSize = 0;
        }
    }
    BufferWriter(MutableByteSpan buf) : BufferWriter(buf.data(), buf.size()) {}
    BufferWriter(const BufferWriter & other)             = default;
    BufferWriter & operator=(const BufferWriter & other) = default;

    /// Append a null terminated string, exclude the null terminator
    BufferWriter & Put(const char * s);

    /// Raw append a buffer, regardless of endianess.
    /// This is memmove-safe: if `buf` points to the underlying buffer, where output
    /// will be written, and the overlap is legal for a memmove to have worked properly,
    /// then this method will properly copy data.
    BufferWriter & Put(const void * buf, size_t len);

    /// Append a single byte
    BufferWriter & Put(uint8_t c);

    BufferWriter & Skip(size_t len)
    {
        mNeeded += len;
        return *this;
    }

    /// Number of bytes required to satisfy all calls to Put() so far
    inline size_t Needed() const { return mNeeded; }

    /// Alias to Needed() for code clarity: current writing position for the buffer.
    inline size_t WritePos() const { return Needed(); }

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

    BufferWriter & Format(const char * format, ...) ENFORCE_FORMAT(2, 3)
    {
        va_list args;
        va_start(args, format);
        VFormat(format, args);
        va_end(args);
        return *this;
    }

    void Reset() { mNeeded = 0; }

    /// Since this uses vsnprintf internally, on overflow
    /// this will write one less byte that strictly can be
    /// written (since null terminator will be in the binary data)
    BufferWriter & VFormat(const char * format, va_list args) ENFORCE_FORMAT(2, 0);

    /// Assume a specific size for the buffer instead of mSize
    ///
    /// This is to allow avoiding off-by-one overflow truncation
    /// when we know the underlying buffer size is larger.
    BufferWriter & VFormatWithSize(size_t size, const char * format, va_list args) ENFORCE_FORMAT(3, 0);

protected:
    uint8_t * mBuf;
    size_t mSize;
    size_t mNeeded;
};

template <class Derived>
class EndianBufferWriterBase : public BufferWriter
{
public:
    // typed BufferWriter forwards

    Derived & Put(const char * s) { return static_cast<Derived &>(BufferWriter::Put(s)); }
    Derived & Put(const void * buf, size_t len) { return static_cast<Derived &>(BufferWriter::Put(buf, len)); }
    Derived & Put(uint8_t c) { return static_cast<Derived &>(BufferWriter::Put(c)); }
    Derived & Skip(size_t len) { return static_cast<Derived &>(BufferWriter::Skip(len)); }

    // write an integer into a buffer, in an endian-specific way

    Derived & Put8(uint8_t c) { return static_cast<Derived *>(this)->Put(c); }
    Derived & Put16(uint16_t x) { return static_cast<Derived *>(this)->EndianPut(x, sizeof(x)); }
    Derived & Put32(uint32_t x) { return static_cast<Derived *>(this)->EndianPut(x, sizeof(x)); }
    Derived & Put64(uint64_t x) { return static_cast<Derived *>(this)->EndianPut(x, sizeof(x)); }

    Derived & PutSigned8(int8_t x) { return static_cast<Derived *>(this)->EndianPutSigned(x, sizeof(x)); }
    Derived & PutSigned16(int16_t x) { return static_cast<Derived *>(this)->EndianPutSigned(x, sizeof(x)); }
    Derived & PutSigned32(int32_t x) { return static_cast<Derived *>(this)->EndianPutSigned(x, sizeof(x)); }
    Derived & PutSigned64(int64_t x) { return static_cast<Derived *>(this)->EndianPutSigned(x, sizeof(x)); }

protected:
    EndianBufferWriterBase(uint8_t * buf, size_t len) : BufferWriter(buf, len) {}
    EndianBufferWriterBase(MutableByteSpan buf) : BufferWriter(buf.data(), buf.size()) {}
    EndianBufferWriterBase(const EndianBufferWriterBase & other)             = default;
    EndianBufferWriterBase & operator=(const EndianBufferWriterBase & other) = default;
};

namespace LittleEndian {

class BufferWriter : public EndianBufferWriterBase<BufferWriter>
{
public:
    BufferWriter(uint8_t * buf, size_t len) : EndianBufferWriterBase<BufferWriter>(buf, len)
    {
        static_assert((-1 & 3) == 3, "LittleEndian::BufferWriter only works with 2's complement architectures.");
    }
    BufferWriter(MutableByteSpan buf) : EndianBufferWriterBase<BufferWriter>(buf) {}
    BufferWriter(const BufferWriter & other)             = default;
    BufferWriter & operator=(const BufferWriter & other) = default;
    BufferWriter & EndianPut(uint64_t x, size_t size);
    BufferWriter & EndianPutSigned(int64_t x, size_t size);
};

} // namespace LittleEndian

namespace BigEndian {

class BufferWriter : public EndianBufferWriterBase<BufferWriter>
{
public:
    BufferWriter(uint8_t * buf, size_t len) : EndianBufferWriterBase<BufferWriter>(buf, len)
    {
        static_assert((-1 & 3) == 3, "BigEndian::BufferWriter only works with 2's complement architectures.");
    }
    BufferWriter(MutableByteSpan buf) : EndianBufferWriterBase<BufferWriter>(buf) {}
    BufferWriter(const BufferWriter & other)             = default;
    BufferWriter & operator=(const BufferWriter & other) = default;
    BufferWriter & EndianPut(uint64_t x, size_t size);
    BufferWriter & EndianPutSigned(int64_t x, size_t size);
};

} // namespace BigEndian

} // namespace Encoding
} // namespace chip
