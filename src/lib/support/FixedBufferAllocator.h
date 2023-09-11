/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstddef>
#include <cstdint>

namespace chip {
/**
 * Memory allocator that uses a fixed-size buffer.
 *
 * This class allocates subsequent memory regions out of a fixed-size buffer.
 * Deallocation of specific regions is unsupported and it is assumed that the entire
 * buffer will be released at once.
 */
class FixedBufferAllocator
{
public:
    FixedBufferAllocator() = default;
    FixedBufferAllocator(uint8_t * buffer, size_t capacity) { Init(buffer, capacity); }

    template <size_t N>
    explicit FixedBufferAllocator(uint8_t (&buffer)[N])
    {
        Init(buffer);
    }

    void Init(uint8_t * buffer, size_t capacity)
    {
        mBegin          = buffer;
        mEnd            = buffer + capacity;
        mAnyAllocFailed = false;
    }

    template <size_t N>
    void Init(uint8_t (&buffer)[N])
    {
        Init(buffer, N);
    }

    /**
     * Allocate a specified number of bytes.
     *
     * @param count     Number of bytes to allocate.
     * @return          Pointer to the allocated memory region or nullptr on failure.
     */
    uint8_t * Alloc(size_t count);

    /**
     * Allocate memory for the specified data and copy the data into the allocated region.
     *
     * @param data      Pointer to the data to be copied into the allocated memory region.
     * @param dataLen   Size of the data to be copied into the allocated memory region.
     * @return          Pointer to the allocated memory region or nullptr on failure.
     */
    uint8_t * Clone(const void * data, size_t dataLen);

    /**
     * Allocate memory for the specified string and copy the string, including
     * the null-character, into the allocated region.
     *
     * @param str       Pointer to the string to be copied into the allocated memory region.
     * @return          Pointer to the allocated memory region or nullptr on failure.
     */
    char * Clone(const char * str);

    /**
     * Returns whether any allocation has failed so far.
     */
    bool AnyAllocFailed() const { return mAnyAllocFailed; }

private:
    FixedBufferAllocator(const FixedBufferAllocator &) = delete;
    void operator=(const FixedBufferAllocator &) = delete;

    uint8_t * mBegin     = nullptr;
    uint8_t * mEnd       = nullptr;
    bool mAnyAllocFailed = false;
};

} // namespace chip
