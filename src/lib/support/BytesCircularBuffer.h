/*
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

#pragma once

#include <cstddef>

#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>

namespace chip {

/**
 * @brief A circular buffer to store byte sequences.
 *
 * This circular buffer provides a queue like interface to push/pop byte sequences
 * from the buffer. When there is no space to push a new byte sequence, the oldest
 * byte sequences will be discarded silently to free up space until there is enough
 * for the new byte sequence.
 */
class BytesCircularBuffer
{
public:
    /**
     * @brief Create a byte sequence circular buffer.
     *
     * @param storage  The underlying storage. This class doesn't take the ownership of the storage.
     * @param capacity The length of the storage.
     */
    BytesCircularBuffer(uint8_t * storage, size_t capacity) : mStorage(storage), mCapacity(capacity)
    {
        VerifyOrDie(mCapacity > sizeof(SizeType) + 1);
    }

    /**
     *  @brief Push a byte sequence into the circular buffer. When there is no
     *    space to push a new byte sequence, the oldest byte sequences will be
     *    discarded silently to free up space until there is enough for the new
     *    byte sequence.
     *
     *  @returns CHIP_NO_ERROR if successful
     *           CHIP_ERROR_INVALID_ARGUMENT if the payload is too large to fit into the buffer
     */
    CHIP_ERROR Push(const ByteSpan & payload);

    /// @brief Same as previous Push, but payload can be spread into 2 spans.
    CHIP_ERROR Push(const ByteSpan & payload1, const ByteSpan & payload2);

    /** @brief discard the oldest byte sequence in the buffer.
     *
     *  @returns CHIP_NO_ERROR if successful
     *           CHIP_ERROR_INCORRECT_STATE if the buffer is empty
     */
    CHIP_ERROR Pop();

    bool IsEmpty() const;

    /** @brief get the length of the front element. */
    size_t GetFrontSize() const;

    /** @brief read the front element into dest.
     *
     *  @returns CHIP_NO_ERROR if successful
     *           CHIP_ERROR_INCORRECT_STATE if the buffer is empty
     *           CHIP_ERROR_INVALID_ARGUMENT if the length of dest is less than GetFrontSize */
    CHIP_ERROR ReadFront(MutableByteSpan & dest) const;

private:
    void Read(uint8_t * dest, size_t length, size_t offset) const; // read length bytes into dest
    void Write(const uint8_t * source, size_t length);
    void Drop(size_t length);
    size_t StorageAvailable() const; // returns number of bytes available
    size_t StorageUsed() const;      // returns number of bytes stored

    /** @brief advance dataLocation by amount, wrap around on mCapacity
     *
     *  @pre amount must be less than mCapacity
     */
    size_t Advance(size_t dataLocation, size_t amount) const;

    // Internal storage. Arranged by packets with following structure:
    // | Size (2 bytes) | Byte sequence (size bytes) |
    uint8_t * const mStorage;
    const size_t mCapacity;

    using SizeType = uint16_t;

    // Both mDataStart and mDataEnd range from [0, mCapacity). mDataEnd is pointing past the last element.
    // When mDataStart == mDataEnd, the buffer is empty
    // When mDataStart < mDataEnd, the actual data is stored in [mDataStart, mDataEnd)
    // When mDataStart > mDataEnd, the actual data is stored in [mDataStart, mCapacity) ++ [0, mDataEnd)
    size_t mDataStart = 0;
    size_t mDataEnd   = 0;
};

} // namespace chip
