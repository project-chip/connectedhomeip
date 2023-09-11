/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This file implements heap memory allocation APIs for CHIP. These functions are platform
 *      specific and might be C Standard Library heap functions re-direction in most of cases.
 *
 */

//#include <lib/core/CHIPConfig.h>
#include "matter_pal.h"
#include <lib/support/CHIPMem.h>
#include <platform/PlatformManager.h>

#include <atomic>
#include <cstdio>
#include <cstring>
#include <stdlib.h>

#if CHIP_CONFIG_MEMORY_MGMT_PLATFORM

namespace chip {
namespace Platform {

#define VERIFY_INITIALIZED() VerifyInitialized(__func__)

static std::atomic_int memoryInitialized{ 0 };

static void VerifyInitialized(const char * func)
{
#if 0
    if (!memoryInitialized)
    {
        fprintf(stderr, "ABORT: chip::Platform::%s() called before chip::Platform::MemoryInit()\n", func);
        abort();
    }
#endif
}

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
    void * ptr;
    VERIFY_INITIALIZED();
    ptr = os_zalloc(size); // NULL;
    return ptr;
}

void * MemoryAlloc(size_t size, bool isLongTermAlloc)
{
    void * ptr;
    VERIFY_INITIALIZED();
    ptr = os_zalloc(size);
    return ptr;
}

void * MemoryCalloc(size_t num, size_t size)
{
    VERIFY_INITIALIZED();
    void * ptr = os_zalloc(num * size);
    return ptr;
}

void * MemoryRealloc(void * p, size_t size)
{
    VERIFY_INITIALIZED();
    p = os_realloc(p, size);
    return p;
}

void MemoryFree(void * p)
{
    VERIFY_INITIALIZED();
    os_free(p);
}

bool MemoryInternalCheckPointer(const void * p, size_t min_size)
{
    return (p != nullptr);
}

} // namespace Platform
} // namespace chip

#endif // CHIP_CONFIG_MEMORY_MGMT_PLATFORM
