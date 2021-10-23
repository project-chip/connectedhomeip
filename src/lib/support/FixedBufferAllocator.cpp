/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "FixedBufferAllocator.h"

#include <cstring>

namespace chip {
uint8_t * FixedBufferAllocator::Alloc(size_t count)
{
    if (mBegin + count > mEnd)
    {
        mAnyAllocFailed = true;
        return nullptr;
    }

    uint8_t * ptr = mBegin;
    mBegin += count;
    return ptr;
}

uint8_t * FixedBufferAllocator::Clone(const void * data, size_t dataLen)
{
    uint8_t * ptr = Alloc(dataLen);

    if (ptr != nullptr)
    {
        memcpy(ptr, data, dataLen);
    }

    return ptr;
}

char * FixedBufferAllocator::Clone(const char * str)
{
    return reinterpret_cast<char *>(Clone(str, strlen(str) + 1));
}
} // namespace chip
