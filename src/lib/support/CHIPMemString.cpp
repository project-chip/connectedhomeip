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
 *      This file implements string operations that allocate heap memory.
 */

#include <string.h>

#include <support/CHIPMem.h>
#include <support/CHIPMemString.h>

namespace chip {
namespace Platform {

char * MemoryAllocString(const char * string, size_t length)
{
    if (string && !length)
    {
        length = strlen(string);
    }
    return CopyString(static_cast<char *>(MemoryAlloc(length + 1)), string, length);
}

char * CopyString(char * dest, const char * source, size_t length)
{
    if (dest)
    {
        strncpy(dest, source ? source : "", length);
        dest[length] = 0;
    }
    return dest;
}

} // namespace Platform
} // namespace chip
