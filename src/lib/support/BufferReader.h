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
 *    Utility classes for safely reading from size-limited buffers.
 */

#pragma once

#include <climits>
#include <lib/core/CHIPEncoding.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <stdint.h>

namespace chip {
namespace Encoding {
namespace LittleEndian {

/**
 *  @class Reader
 *
 *  Simple reader for reading little-endian things out of buffers.
 */
class Reader
{
public:
    /**
     * Create a data model reader from a given buffer and length.
     *
     * @param buffer The octet buffer to read from.  The caller must ensure
     *               (most simply by allocating the reader on the stack) that
     *               the buffer outlives the reader.  The buffer is allowed to
     *               be null if buf_len is 0.
     * @param buf_len The number of octets in the buffer.
     */
    Reader(const uint8_t * buffer, uint16_t buf_len) : mBufStart(buffer), mReadPtr(buffer), mAvailable(buf_len) {}

    /**
     * Number of octets we have read so far.  This might be able to go away once
     * we do less switching back and forth between DataModelReader and raw
     * buffers.
     */
    uint16_t OctetsRead() const { return static_cast<uint16_t>(mReadPtr - mBufStart); }

    /**
     * Read a single 8-bit unsigned integer.
     *
     * @param [out] dest Where the 8-bit integer goes.
     *
     * @return Whether the read succeeded.  The read can fail if there are not
     *         enough octets available.
     */
    CHECK_RETURN_VALUE
    CHIP_ERROR Read8(uint8_t * dest) { return Read(dest); }

    /**
     * Read a single 16-bit unsigned integer.
     *
     * @param [out] dest Where the 16-bit integer goes.
     *
     * @return Whether the read succeeded.  The read can fail if there are not
     *         enough octets available.
     */
    CHECK_RETURN_VALUE
    CHIP_ERROR Read16(uint16_t * dest) { return Read(dest); }

    /**
     * Read a single 32-bit unsigned integer.
     *
     * @param [out] dest Where the 32-bit integer goes.
     *
     * @return Whether the read succeeded.  The read can fail if there are not
     *         enough octets available.
     */
    CHECK_RETURN_VALUE
    CHIP_ERROR Read32(uint32_t * dest) { return Read(dest); }

    /**
     * Read a single 64-bit unsigned integer.
     *
     * @param [out] dest Where the 64-bit integer goes.
     *
     * @return Whether the read succeeded.  The read can fail if there are not
     *         enough octets available.
     */
    CHECK_RETURN_VALUE
    CHIP_ERROR Read64(uint64_t * dest) { return Read(dest); }

protected:
    template <typename T>
    CHIP_ERROR Read(T * retval)
    {
        static_assert(CHAR_BIT == 8, "Our various sizeof checks rely on bytes and octets being the same thing");

        static constexpr size_t data_size = sizeof(T);

        if (mAvailable < data_size)
        {
            return CHIP_ERROR_BUFFER_TOO_SMALL;
        }

        Read(mReadPtr, retval);
        mAvailable = static_cast<uint16_t>(mAvailable - data_size);
        return CHIP_NO_ERROR;
    }

private:
    // These helper methods return void and put the value being read into an
    // outparam because that allows us to easily overload on the type of the
    // thing being read.
    void Read(const uint8_t *& p, uint8_t * dest) { *dest = Encoding::Read8(p); }
    void Read(const uint8_t *& p, uint16_t * dest) { *dest = Encoding::LittleEndian::Read16(p); }
    void Read(const uint8_t *& p, uint32_t * dest) { *dest = Encoding::LittleEndian::Read32(p); }
    void Read(const uint8_t *& p, uint64_t * dest) { *dest = Encoding::LittleEndian::Read64(p); }

    /**
     * Our buffer start.
     */
    const uint8_t * const mBufStart;

    /**
     * Our current read point.
     */
    const uint8_t * mReadPtr;

    /**
     * The number of octets we can still read starting at mReadPtr.
     */
    uint16_t mAvailable;
};

} // namespace LittleEndian
} // namespace Encoding
} // namespace chip
