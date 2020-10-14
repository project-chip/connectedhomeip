/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 *    @file
 *      This file defines string operations that allocate heap memory.
 */

#pragma once

#include <stdlib.h>
#include <string.h>

#include <support/ScopedBuffer.h>

namespace chip {
namespace Platform {

/**
 * Copies a C-style string.
 *
 * This differs from `strncpy()` in some important ways:
 *  - `dest` can be nullptr, in which case no copy is attempted, and the function returns nullptr.
 *  - A non-nullptr result is always null-terminated.
 *
 * @param[in]  dest             Destination string buffer (which must be at least `length`+1 bytes)
 *                              or nullptr.
 *
 * @param[in]  source           String to be copied.
 *
 * @param[in]  length           Length to be copied.
 *
 * @retval  Same as `dest`.
 */
inline char * CopyString(char * dest, const char * source, size_t length)
{
    if (dest)
    {
        strncpy(dest, source, length);
        dest[length] = 0;
    }
    return dest;
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
    return CopyString(static_cast<char *>(MemoryAlloc(length + 1)), string, length);
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
    ScopedMemoryString(const char * string, size_t length) { CopyString(Alloc(length + 1).Get(), string, length); }
};

} // namespace Platform
} // namespace chip
