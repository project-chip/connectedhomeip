/*
 * SPDX-FileCopyrightText: (c) 2020-2021 Project CHIP Authors
 * SPDX-FileCopyrightText: (c) 2019 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This file implements heap memory allocation APIs for CHIP. These functions are platform
 *      specific and might be C Standard Library heap functions re-direction in most of cases.
 *
 */

//#include <core/CHIPConfig.h>
#include <lib/support/CHIPMem.h>

#include <FreeRTOS.h>
#include <portable.h>
#include <task.h>

#include <atomic>
#include <cstdio>
#include <cstring>
#include <stdlib.h>

extern "C" {
#include <bflb_platform.h>
#include <blog.h>
};

#if CHIP_CONFIG_MEMORY_MGMT_PLATFORM

extern "C" void memMonitoringTrackAlloc(void * ptr, size_t size);
extern "C" void memMonitoringTrackFree(void * ptr, size_t size);

#ifndef trackAlloc
#define trackAlloc(pvAddress, uiSize) memMonitoringTrackAlloc(pvAddress, uiSize)
#endif
#ifndef trackFree
#define trackFree(pvAddress, uiSize) memMonitoringTrackFree(pvAddress, uiSize)
#endif

namespace chip {
namespace Platform {

#define VERIFY_INITIALIZED() VerifyInitialized(__func__)

static std::atomic_int memoryInitialized{ 0 };

static void VerifyInitialized(const char * func)
{
    if (!memoryInitialized)
    {
        log_error("ABORT: chip::Platform::%s() called before chip::Platform::MemoryInit()\r\n", func);
        abort();
    }
}

CHIP_ERROR MemoryAllocatorInit(void * buf, size_t bufSize)
{
#ifndef NDEBUG
    if (memoryInitialized++ > 0)
    {
        log_error("ABORT: chip::Platform::MemoryInit() called twice.\r\n");
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
        log_error("ABORT: chip::Platform::MemoryShutdown() called twice.\r\n");
        abort();
    }
#endif
}

void * MemoryAlloc(size_t size)
{
    void * ptr;
    VERIFY_INITIALIZED();
    ptr = pvPortMalloc(size);
    trackAlloc(ptr, size);
    return ptr;
}

void * MemoryAlloc(size_t size, bool isLongTermAlloc)
{
    void * ptr;
    VERIFY_INITIALIZED();
    ptr = pvPortMalloc(size);
    trackAlloc(ptr, size);
    return ptr;
}

void * MemoryCalloc(size_t num, size_t size)
{
    void * ptr;
    VERIFY_INITIALIZED();
    ptr = pvPortCalloc(num, size);
    trackAlloc(ptr, size * num);
    return ptr;
}

void * MemoryRealloc(void * p, size_t size)
{
    VERIFY_INITIALIZED();
    return pvPortRealloc(p, size);
}

void MemoryFree(void * p)
{
    VERIFY_INITIALIZED();
    trackFree(p, 0);
    vPortFree(p);
}

bool MemoryInternalCheckPointer(const void * p, size_t min_size)
{
    return (p != nullptr);
}

} // namespace Platform
} // namespace chip

extern "C" void memMonitoringTrackAlloc(void * ptr, size_t size) {}

extern "C" void memMonitoringTrackFree(void * ptr, size_t size) {}

#endif // CHIP_CONFIG_MEMORY_MGMT_PLATFORM
