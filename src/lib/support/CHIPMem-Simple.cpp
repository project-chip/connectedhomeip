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

#include <lib/support/VerificationMacrosNoLogging.h>
#include <system/SystemMutex.h>

namespace chip {
namespace Platform {

namespace {

void * gPrivateHeap = nullptr;

#if CHIP_SYSTEM_CONFIG_NO_LOCKING

class HeapLocked
{
public:
    HeapLocked() {}
    ~HeapLocked() {}
};

#else

chip::System::Mutex gHeapLock;

class HeapLocked
{
public:
    HeapLocked() { gHeapLock.Lock(); }
    ~HeapLocked() { gHeapLock.Unlock(); }
};
#endif

} // namespace

CHIP_ERROR MemoryAllocatorInit(void * buf, size_t bufSize)
{
    ReturnErrorCodeIf(buf == nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorCodeIf(gPrivateHeap != nullptr, CHIP_ERROR_INCORRECT_STATE);

    PrivateHeapInit(buf, bufSize);
    gPrivateHeap = buf;

#if CHIP_SYSTEM_CONFIG_NO_LOCKING
    return CHIP_NO_ERROR;
#else
    return chip::System::Mutex::Init(gHeapLock);
#endif
}

void MemoryAllocatorShutdown()
{
    gPrivateHeap = nullptr;
}

void * MemoryAlloc(size_t size)
{
    HeapLocked lock;

    if (gPrivateHeap == nullptr)
    {
        return nullptr;
    }

    return PrivateHeapAlloc(gPrivateHeap, size);
}

void * MemoryCalloc(size_t num, size_t size)
{
    size_t total = num * size;

    // check for multiplication overflow
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
    HeapLocked lock;

    if (gPrivateHeap == nullptr)
    {
        return nullptr;
    }

    return PrivateHeapRealloc(gPrivateHeap, p, size);
}

void MemoryFree(void * p)
{
    HeapLocked lock;

    if (gPrivateHeap == nullptr)
    {
        return;
    }
    PrivateHeapFree(p);
}

} // namespace Platform
} // namespace chip
