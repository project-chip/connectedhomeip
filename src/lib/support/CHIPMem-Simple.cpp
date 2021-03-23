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

#include "CHIPMem.h"
#include "PrivateHeap.h"

#include <string.h>

#include <support/CodeUtils.h>

namespace {

void * gPrivateHeap = nullptr;

} // namespace

CHIP_ERROR MemoryInit(void * buf, size_t bufSize)
{
    ReturnErrorCodeIf(buf == nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    PrivateHeapInit(buf, bufSize);
    gPrivateHeap = buf;

    return CHIP_NO_ERROR;
}

void MemoryShutdown()
{
    gPrivateHeap = nullptr;
}

void * MemoryAlloc(size_t size)
{
    if (gPrivateHeap == nullptr)
    {
        return nullptr;
    }

    return PrivateHeapAlloc(gPrivateHeap, size);
}

void * MemoryCalloc(size_t num, size_t size)
{
    size_t total = num * size;

    // check is for multiplication overflow
    if (size != total / num)
    {
        return nullptr;
    }

    void * result = MemoryAlloc(total);
    if (result != nullptr)
    {
        memset(result, 0, total);
    }
    return result;
}

void * MemoryRealloc(void * p, size_t size)
{
    if (gPrivateHeap == nullptr)
    {
        return nullptr;
    }

    return PrivateHeapRealloc(gPrivateHeap, p, size);
}

void MemoryFree(void * p)
{
    if (gPrivateHeap == nullptr)
    {
        return;
    }
    PrivateHeapFree(p);
}
P