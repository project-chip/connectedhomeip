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

#include <core/CHIPError.h>
#include <support/Span.h>

namespace chip {

/**
 * @brief A circular buffer to store strings.
 *
 * This circular buffer provides a queue like interface push/pop strings from
 * the buffer. When there is no space to push a new string, the oldest string
 * will be discarded silently to free up space.
 *
 * NOTE: this class never handles trailing zero.
 */
class StringCircularBuffer
{
public:
    /**
     * @brief Create a string circular buffer.
     *
     * @param storage  The underlying storage. This class doesn't take the ownership of the storage.
     * @param capacity The length of the storage.
     */
    StringCircularBuffer(uint8_t * storage, size_t capacity) : mStorage(storage), mCapacity(capacity), mDataStart(0), mDataEnd(0) {}

    CHIP_ERROR Push(const ByteSpan & payload);
    CHIP_ERROR Pop();

    bool IsEmpty() const;

    /** @brief get the length of the front element. */
    size_t GetFrontSize() const;

    /** @brief read the front element into dest. The length of dest must be equal or larger than GetFrontSize */
    CHIP_ERROR ReadFront(uint8_t * dest) const;

private:
    void Read(uint8_t * dest, size_t length, size_t offset) const; // read length bytes into dest
    void Write(const uint8_t * source, size_t length);
    void Drop(size_t length);
    size_t Size() const; // returns number of bytes stored
    std::size_t Advance(std::size_t dataLocation, size_t amount) const; // advance dataLocation by amount, wrap around on mCapacity

    // Internal storage. Arranged by packets with following structure:
    // | Size (2 bytes) | String data (size bytes) |
    uint8_t * mStorage;
    size_t mCapacity;

    using SizeType = uint16_t;

    std::size_t mDataStart;
    std::size_t mDataEnd;
};

} // namespace chip
