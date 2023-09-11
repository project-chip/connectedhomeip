/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This file defines string operations that allocate heap memory.
 */

#pragma once

#include <algorithm>
#include <stdlib.h>
#include <string.h>

#include <lib/support/ScopedBuffer.h>
#include <lib/support/Span.h>

namespace chip {
namespace Platform {

/**
 * Copies a C-style string.
 *
 * This differs from `strncpy()` in some important ways:
 *  - `dest` can be nullptr, in which case no copy is attempted, and the function returns nullptr.
 *  - A non-nullptr result is always null-terminated.
 *
 * @param[in]  dest             Destination string buffer or nullptr.
 *
 * @param[in]  destLength       Maximum length to be copied. Will need space for null terminator as
 *                              well (string will be truncated if it does not fit). If 0 this method
 *                              is a noop.
 *
 * @param[in]  source           String to be copied.
 *
 * @retval  Same as `dest`.
 */
inline void CopyString(char * dest, size_t destLength, const char * source)
{
    if (dest && destLength)
    {
        strncpy(dest, source, destLength);
        dest[destLength - 1] = 0;
    }
}

/**
 * Convenience method for CopyString to auto-detect destination size.
 */
template <size_t N>
inline void CopyString(char (&dest)[N], const char * source)
{
    CopyString(dest, N, source);
}

/**
 * Creates a null-terminated string from a ByteSpan.
 * If dest is nullptr, no copy happens. Non-nullptr result is always null-terminated.
 *
 * @param[in]  dest             Destination string buffer or nullptr.
 *
 * @param[in]  destLength       Maximum length to be copied. Will need space for null terminator as
 *                              well (string will be truncated if it does not fit). If 0 this method
 *                              is a noop.
 *
 * @param[in]  source           Data to be copied.
 */
inline void CopyString(char * dest, size_t destLength, ByteSpan source)
{
    if (dest && destLength)
    {
        size_t maxChars = std::min(destLength - 1, source.size());
        memcpy(dest, source.data(), maxChars);
        dest[maxChars] = '\0';
    }
}

/**
 * Convenience method for CopyString to auto-detect destination size.
 */
template <size_t N>
inline void CopyString(char (&dest)[N], ByteSpan source)
{
    CopyString(dest, N, source);
}

/**
 * Creates a null-terminated string from a CharSpan.
 * If dest is nullptr, no copy happens. Non-nullptr result is always null-terminated.
 *
 * @param[in]  dest             Destination string buffer or nullptr.
 *
 * @param[in]  destLength       Maximum length to be copied. Will need space for null terminator as
 *                              well (string will be truncated if it does not fit). If 0 this method
 *                              is a noop.
 *
 * @param[in]  source           Data to be copied.
 */
inline void CopyString(char * dest, size_t destLength, CharSpan source)
{
    if (dest && destLength)
    {
        size_t maxChars = std::min(destLength - 1, source.size());
        memcpy(dest, source.data(), maxChars);
        dest[maxChars] = '\0';
    }
}

/**
 * Convenience method for CopyString to auto-detect destination size.
 */
template <size_t N>
inline void CopyString(char (&dest)[N], CharSpan source)
{
    CopyString(dest, N, source);
}

/**
 * This function copies a C-style string to memory newly allocated by Platform::MemoryAlloc().
 *
 * @param[in]  string           String to be copied.
 *
 * @param[in]  length           Length to be copied. Like `strncpy()`, if the `string` is shorter
 *                              than `length`, then the remaining space will be filled with null
 *                              bytes. Like `strndup()` but unlike `strncpy()`, the result is always
 *                              null-terminated.
 *
 * @retval  Pointer to a null-terminated string in case of success.
 * @retval  `nullptr` if memory allocation fails.
 *
 */
inline char * MemoryAllocString(const char * string, size_t length)
{
    size_t lengthWithNull = length + 1;
    char * result         = static_cast<char *>(MemoryAlloc(lengthWithNull));
    CopyString(result, lengthWithNull, string);
    return result;
}

/**
 * Represents a C string in a ScopedMemoryBuffer.
 */

class ScopedMemoryString : public ScopedMemoryBuffer<char>
{
public:
    /**
     * Create a ScopedMemoryString.
     *
     * @param[in]  string           String to be copied.
     *
     * @param[in]  length           Length to be copied. Like `strncpy()`, if the `string` is shorter than
     *                              `length`, then the remaining space will be filled with null bytes. Like
     *                              `strndup()` but unlike `strncpy()`, the result is always null-terminated.
     */
    ScopedMemoryString(const char * string, size_t length)
    {
        size_t lengthWithNull = length + 1;

        // We must convert the source string to a CharSpan, so we call the
        // version of CopyString that handles unterminated strings.
        CopyString(Alloc(lengthWithNull).Get(), lengthWithNull, CharSpan(string, length));
    }
};

} // namespace Platform
} // namespace chip
