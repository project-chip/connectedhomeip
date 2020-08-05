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
 *    basic double-ended queue, intended to be embedded as a member
 *     of an object
 */

#ifndef CHIP_BUFBOUND_H
#define CHIP_BUFBOUND_H

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
    size_t mLen;
    size_t mWritten;

public:
    BufBound(uint8_t * buf, size_t len) : mBuf(buf), mLen(len), mWritten(0) {}

    /*
     * @brief append a single byte
     */
    size_t Put(uint8_t c)
    {
        if (mWritten < mLen)
        {
            mBuf[mWritten] = c;
        }
        return ++mWritten;
    }

    /*
     * @brief append a null terminated string, exclude the null term
     */
    size_t Put(const char * s)
    {
        while (*s != 0)
        {
            Put(*s++);
        }
        return mWritten;
    }

    size_t PutLE(uint64_t x, size_t size)
    {
        while (size-- > 0)
        {
            uint8_t c = x & 0xff;
            Put(c);
            x >>= 8;
        }
        return mWritten;
    }

    /*
     * @brief write integer x into the buffer, least significant byte first
     */
    size_t PutLE64(uint64_t x) { return PutLE(x, sizeof(x)); }
    size_t PutLE32(uint32_t x) { return PutLE(x, sizeof(x)); }
    size_t PutLE16(uint16_t x) { return PutLE(x, sizeof(x)); }

    size_t PutBE(uint64_t x, size_t size)
    {
        while (size-- > 0)
        {
            uint8_t c = (x >> (size * 8)) & 0xff;
            Put(c);
        }
        return mWritten;
    }

    /*
     * @brief write integer x into the buffer, most significant byte first
     */
    size_t PutBE64(uint64_t x) { return PutBE(x, sizeof(x)); }
    size_t PutBE32(uint32_t x) { return PutBE(x, sizeof(x)); }
    size_t PutBE16(uint16_t x) { return PutBE(x, sizeof(x)); }

    /*
     * @brief append a buffer
     */
    size_t Put(const uint8_t * buf, size_t len) { return Put(reinterpret_cast<const void *>(buf), len); }
    size_t Put(const void * buf, size_t len)
    {
        size_t available = Available();

        memmove(mBuf + mWritten, buf, available < len ? available : len);

        mWritten += len;

        return mWritten;
    }

    /*
     * @brief how much was or would have been written
     */
    size_t Written() { return mWritten; }
    /*
     * @brief bytes available
     */
    size_t Available() { return mLen < mWritten ? 0 : mLen - mWritten; }
    /*
     * @brief whether the input fit in the buffer
     */
    bool Fit() { return mLen >= mWritten; }
};

} // namespace chip

#endif /* CHIP_BUFBOUND */
