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
#include <lib/support/Span.h>
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
     * Create a buffer reader from a given buffer and length.
     *
     * @param buffer The octet buffer from which to read.  The caller must ensure
     *               (most simply by allocating the reader on the stack) that
     *               the buffer outlives the reader. If `buffer` is nullptr,
     *               length is automatically overridden to zero, to avoid accesses.
     * @param buf_len The number of octets in the buffer.
     */
    Reader(const uint8_t * buffer, size_t buf_len) : mBufStart(buffer), mReadPtr(buffer), mAvailable(buf_len)
    {
        if (mBufStart == nullptr)
        {
            mAvailable = 0;
        }
    }

    /**
     * Create a buffer reader from a given byte span.
     *
     * @param buffer The octet buffer byte span from which to read.  The caller must ensure
     *               that the buffer outlives the reader.  The buffer's ByteSpan .data() pointer
     *               is is nullptr, length is automatically overridden to zero, to avoid accesses.
     */
    Reader(const ByteSpan & buffer) : Reader(buffer.data(), buffer.size()) {}

    /**
     * Number of octets we have read so far.
     */
    size_t OctetsRead() const { return static_cast<size_t>(mReadPtr - mBufStart); }

    /**
     * Number of octets we have remaining to read.
     */
    size_t Remaining() const { return mAvailable; }

    /**
     * Test whether we have at least the given number of octets left to read.
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
     * @return false if the reader is in error, true if the reader is OK.
     */
    bool IsSuccess() const { return StatusCode() == CHIP_NO_ERROR; }

    /**
     * Read a bool, assuming single byte storage.
     *
     * @param [out] dest Where the 8-bit integer goes.
     *
     * @note The read can put the reader in a failed-status state if there are
     *       not enough octets available.  Callers must either continue to do
     *       more reads on the return value or check its status to see whether
     *       the sequence of reads that has been performed succeeded.
     */
    CHECK_RETURN_VALUE
    Reader & ReadBool(bool * dest)
    {
        static_assert(sizeof(bool) == 1, "Expect single-byte bools");
        RawReadLowLevelBeCareful(dest);
        return *this;
    }

    /**
     * Read a char, assuming single byte storage.
     *
     * @param [out] dest Where the char just read should be placed.
     *
     * @note The read can put the reader in a failed-status state if there are
     *       not enough octets available.  Callers must either continue to do
     *       more reads on the return value or check its status to see whether
     *       the sequence of reads that has been performed succeeded.
     */
    CHECK_RETURN_VALUE
    Reader & ReadChar(char * dest)
    {
        static_assert(sizeof(char) == 1, "Expect single-byte chars");
        RawReadLowLevelBeCareful(dest);
        return *this;
    }

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
        RawReadLowLevelBeCareful(dest);
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
        RawReadLowLevelBeCareful(dest);
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
        RawReadLowLevelBeCareful(dest);
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
        RawReadLowLevelBeCareful(dest);
        return *this;
    }

    /**
     * Read a single 8-bit signed integer.
     *
     * @param [out] dest Where the 8-bit integer goes.
     *
     * @note The read can put the reader in a failed-status state if there are
     *       not enough octets available.  Callers must either continue to do
     *       more reads on the return value or check its status to see whether
     *       the sequence of reads that has been performed succeeded.
     */
    CHECK_RETURN_VALUE
    Reader & ReadSigned8(int8_t * dest)
    {
        RawReadLowLevelBeCareful(dest);
        return *this;
    }

    /**
     * Read a single 16-bit signed integer.
     *
     * @param [out] dest Where the 16-bit integer goes.
     *
     * @note The read can put the reader in a failed-status state if there are
     *       not enough octets available.  Callers must either continue to do
     *       more reads on the return value or check its status to see whether
     *       the sequence of reads that has been performed succeeded.
     */
    CHECK_RETURN_VALUE
    Reader & ReadSigned16(int16_t * dest)
    {
        RawReadLowLevelBeCareful(dest);
        return *this;
    }

    /**
     * Read a single 32-bit signed integer.
     *
     * @param [out] dest Where the 32-bit integer goes.
     *
     * @note The read can put the reader in a failed-status state if there are
     *       not enough octets available.  Callers must either continue to do
     *       more reads on the return value or check its status to see whether
     *       the sequence of reads that has been performed succeeded.
     */
    CHECK_RETURN_VALUE
    Reader & ReadSigned32(int32_t * dest)
    {
        RawReadLowLevelBeCareful(dest);
        return *this;
    }

    /**
     * Read a single 64-bit signed integer.
     *
     * @param [out] dest Where the 64-bit integer goes.
     *
     * @note The read can put the reader in a failed-status state if there are
     *       not enough octets available.  Callers must either continue to do
     *       more reads on the return value or check its status to see whether
     *       the sequence of reads that has been performed succeeded.
     */
    CHECK_RETURN_VALUE
    Reader & ReadSigned64(int64_t * dest)
    {
        RawReadLowLevelBeCareful(dest);
        return *this;
    }

    /**
     * Read a byte string from the BufferReader
     *
     * @param [out] dest Where the bytes read
     * @param [in] size How many bytes to read
     *
     * @note The read can put the reader in a failed-status state if there are
     *       not enough octets available.  Callers must either continue to do
     *       more reads on the return value or check its status to see whether
     *       the sequence of reads that has been performed succeeded.
     */
    CHECK_RETURN_VALUE
    Reader & ReadBytes(uint8_t * dest, size_t size);

    /**
     * Helper for our various APIs so we don't have to write out various logic
     * multiple times.  This is public so that consumers that want to read into
     * whatever size a logical thing they are reading into has don't have to
     * hardcode the right API.  This is meant for other reader classes that
     * delegate to this one.
     */
    template <typename T>
    void RawReadLowLevelBeCareful(T * retval);

    /**
     * Advance the Reader forward by the specified number of octets.
     *
     * @param len The number of octets to skip.
     *
     * @note If the len argument is greater than the number of available octets
     *       remaining, the Reader will advance to the end of the buffer
     *       without entering a failed-status state.
     */
    Reader & Skip(size_t len)
    {
        len = ::chip::min(len, mAvailable);
        mReadPtr += len;
        mAvailable = static_cast<size_t>(mAvailable - len);
        return *this;
    }

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
    size_t mAvailable;

    /**
     * Our current status.
     */
    CHIP_ERROR mStatus = CHIP_NO_ERROR;
};

} // namespace LittleEndian
} // namespace Encoding
} // namespace chip
