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
     * Number of octets we have remaining to read.  Can be useful for logging.
     */
    uint16_t Remaining() const { return mAvailable; }

    /**
     * Test whether we have at least the given number of octets left to read.
     * This takes a size_t, not uint16_t, to make life a bit simpler for
     * consumers and avoid casting.
     */
    bool HasAtLeast(size_t octets) const { return octets <= Remaining(); }

    /**
     * The reader status.  Once the status becomes a failure status, all later
     * read operations become no-ops and the status continues to be a failure
     * status.
     */
    CHECK_RETURN_VALUE
    CHIP_ERROR StatusCode() const { return mStatus; }

    /**
     * Read a single 8-bit unsigned integer.
     *
     * @param [out] dest Where the 8-bit integer goes.
     *
     * @note The read can put the reader in a failed-status state if there are
     *       not enough octets available.  Callers must either continue to do
     *       more reads on the return value or check its status to see whether
     *       the sequence of reads that has been performed succeeded.
     */
    CHECK_RETURN_VALUE
    Reader & Read8(uint8_t * dest)
    {
        RawRead(dest);
        return *this;
    }

    /**
     * Read a single 16-bit unsigned integer.
     *
     * @param [out] dest Where the 16-bit integer goes.
     *
     * @note The read can put the reader in a failed-status state if there are
     *       not enough octets available.  Callers must either continue to do
     *       more reads on the return value or check its status to see whether
     *       the sequence of reads that has been performed succeeded.
     */
    CHECK_RETURN_VALUE
    Reader & Read16(uint16_t * dest)
    {
        RawRead(dest);
        return *this;
    }

    /**
     * Read a single 32-bit unsigned integer.
     *
     * @param [out] dest Where the 32-bit integer goes.
     *
     * @note The read can put the reader in a failed-status state if there are
     *       not enough octets available.  Callers must either continue to do
     *       more reads on the return value or check its status to see whether
     *       the sequence of reads that has been performed succeeded.
     */
    CHECK_RETURN_VALUE
    Reader & Read32(uint32_t * dest)
    {
        RawRead(dest);
        return *this;
    }

    /**
     * Read a single 64-bit unsigned integer.
     *
     * @param [out] dest Where the 64-bit integer goes.
     *
     * @note The read can put the reader in a failed-status state if there are
     *       not enough octets available.  Callers must either continue to do
     *       more reads on the return value or check its status to see whether
     *       the sequence of reads that has been performed succeeded.
     */
    CHECK_RETURN_VALUE
    Reader & Read64(uint64_t * dest)
    {
        RawRead(dest);
        return *this;
    }

    /**
     * Helper for our various APIs so we don't have to write out various logic
     * multiple times.  This is public so that consumers that want to read into
     * whatever size a logical thing they are reading into has don't have to
     * hardcode the right API.  This is meant for other reader classes that
     * delegate to this one.
     */
    template <typename T>
    void RawRead(T * retval);

private:
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

    /**
     * Our current status.
     */
    CHIP_ERROR mStatus = CHIP_NO_ERROR;
};

} // namespace LittleEndian
} // namespace Encoding
} // namespace chip
