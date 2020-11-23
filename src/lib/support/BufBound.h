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

/**
 *  @file
 *    BufBound manages serial writes to a buffer, guarding that the bounds of
 *     the buffer are not exceeded.
 *
 */

#pragma once

#include <climits>
#include <stdint.h>
#include <string.h>

namespace chip {

/**
 *  @class BufBound
 *
 *  simple buffer writer
 */
class BufBound
{
private:
    uint8_t * mBuf;
    size_t mSize;
    size_t mNeeded;

public:
    BufBound(uint8_t * buf, size_t len) : mBuf(buf), mSize(len), mNeeded(0) {}
    BufBound(const BufBound & other) = default;

    /*
     * @brief append a single byte
     */
    BufBound & Put(uint8_t c)
    {
        if (mNeeded < mSize)
        {
            mBuf[mNeeded] = c;
        }
        ++mNeeded;
        return *this;
    }

    /*
     * @brief append a null terminated string, exclude the null term
     */
    BufBound & Put(const char * s)
    {
        static_assert(CHAR_BIT == 8, "We're assuming char and uint8_t are the same size");
        while (*s != 0)
        {
            Put(static_cast<uint8_t>(*s++));
        }
        return *this;
    }
    BufBound & Put8(uint8_t c) { return Put(c); }

    BufBound & Put(uint64_t x, size_t size)
    {
        while (size-- > 0)
        {
            uint8_t c = x & 0xff;
            Put(c);
            x >>= 8;
        }
        return *this;
    }

    /*
     * @brief write integer x into the buffer, least significant byte first
     */
    BufBound & Put64(uint64_t x) { return Put(x, sizeof(x)); }
    BufBound & Put32(uint32_t x) { return Put(x, sizeof(x)); }
    BufBound & Put16(uint16_t x) { return Put(x, sizeof(x)); }

    BufBound & PutBE(uint64_t x, size_t size)
    {
        while (size-- > 0)
        {
            uint8_t c = (x >> (size * 8)) & 0xff;
            Put(c);
        }
        return *this;
    }

    /*
     * @brief write integer x into the buffer, most significant byte first
     */
    BufBound & PutBE64(uint64_t x) { return PutBE(x, sizeof(x)); }
    BufBound & PutBE32(uint32_t x) { return PutBE(x, sizeof(x)); }
    BufBound & PutBE16(uint16_t x) { return PutBE(x, sizeof(x)); }

    /*
     * @brief append a buffer
     */
    BufBound & Put(const uint8_t * buf, size_t len) { return Put(reinterpret_cast<const void *>(buf), len); }
    BufBound & Put(const void * buf, size_t len)
    {
        size_t available = Available();

        memmove(mBuf + mNeeded, buf, available < len ? available : len);

        mNeeded += len;

        return *this;
    }

    /*
     * @brief number of bytes required to satisfy all calls to Put() so far
     */
    size_t Needed() const { return mNeeded; }
    /*
     * @brief bytes available
     */
    size_t Available() const { return mSize < mNeeded ? 0 : mSize - mNeeded; }

    /*
     * @brief whether the input fit in the buffer
     */
    bool Fit() const
    {
        size_t _;
        return Fit(_);
    }

    /*
     * @brief returns whether the input fit in the buffer, outputs what was
     *        actually written
     */
    bool Fit(size_t & actually_written) const
    {
        actually_written = mSize >= mNeeded ? mNeeded : mSize;
        return mSize >= mNeeded;
    }

    /*
     * @brief Size of the buffer
     */
    size_t Size() const { return mSize; }
};

} // namespace chip
