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
 *      This file implements heap memory allocation APIs for CHIP. These functions are platform
 *      specific and might be C Standard Library heap functions re-direction in most of cases.
 *
 */

#include <core/CHIPConfig.h>
#include <support/CHIPMem.h>

#include <stdlib.h>

#ifndef NDEBUG
#include <atomic>
#include <cstdio>
#endif

#if CHIP_CONFIG_MEMORY_MGMT_MALLOC

namespace chip {
namespace Platform {

#ifdef NDEBUG

#define VERIFY_INITIALIZED()

#else

#define VERIFY_INITIALIZED() VerifyInitialized(__func__)

static std::atomic_int memoryInitialized{ 0 };

static void VerifyInitialized(const char * func)
{
    if (!memoryInitialized)
    {
        fprintf(stderr, "ABORT: chip::Platform::%s() called before chip::Platform::MemoryInit()\n", func);
        abort();
    }
}

#endif

CHIP_ERROR MemoryAllocatorInit(void * buf, size_t bufSize)
{
#ifndef NDEBUG
    if (memoryInitialized++ > 0)
    {
        fprintf(stderr, "ABORT: chip::Platform::MemoryInit() called twice.\n");
        abort();
    }
#endif
    return CHIP_NO_ERROR;
}

void MemoryAllocatorShutdown()
{
#ifndef NDEBUG
    if (--memoryInitialized < 0)
    {
        fprintf(stderr, "ABORT: chip::Platform::MemoryShutdown() called twice.\n");
        abort();
    }
#endif
}

void * MemoryAlloc(size_t size)
{
    VERIFY_INITIALIZED();
    return MemoryAlloc(size, false);
}

void * MemoryAlloc(size_t size, bool isLongTermAlloc)
{
    VERIFY_INITIALIZED();
    return malloc(size);
}

void * MemoryCalloc(size_t num, size_t size)
{
    VERIFY_INITIALIZED();
    return calloc(num, size);
}

void * MemoryRealloc(void * p, size_t size)
{
    VERIFY_INITIALIZED();
    return realloc(p, size);
}

void MemoryFree(void * p)
{
    VERIFY_INITIALIZED();
    free(p);
}

} // namespace Platform
} // namespace chip

#endif // CHIP_CONFIG_MEMORY_MGMT_MALLOC
