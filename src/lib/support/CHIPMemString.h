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

#ifndef CHIP_MEM_STRING_H
#define CHIP_MEM_STRING_H

#include <stdlib.h>

namespace chip {
namespace Platform {

/**
 * This function copies a C-style string to memory newly allocated by Platform::MemoryAlloc().
 *
 * @param[in]  string           String to be copied.
 *
 * @param[in]  length           Length of the string to be copied. If zero, `strlen(string)`
 *                              will be used. Like `strncpy()`, if the `string` is shorter
 *                              then the remaining space up to `length` will be filled with
 *                              null bytes.
 *
 * @retval  Pointer to a null-terminated string in case of success.
 * @retval  `nullptr` if memory allocation fails.
 *
 */
extern char * MemoryAllocString(const char * string, size_t length = 0);

} // namespace Platform
} // namespace chip

#endif // CHIP_MEM_STRING_H
