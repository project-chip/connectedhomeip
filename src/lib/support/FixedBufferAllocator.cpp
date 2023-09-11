/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
