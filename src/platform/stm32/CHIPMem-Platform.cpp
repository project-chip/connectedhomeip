/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Nest Labs, Inc.
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

/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#include <lib/support/CHIPMem.h>
#include <lib/support/logging/CHIPLogging.h>

#include "cmsis_os.h"
#include "mbedtls/platform.h"
#include "task.h"

#include <atomic>
#include <cstdio>
#include <cstring>
#include <stdlib.h>

#if CHIP_CONFIG_MEMORY_MGMT_PLATFORM

extern "C" void memMonitoringTrackAlloc(void * ptr, size_t size);
extern "C" void memMonitoringTrackFree(void * ptr, size_t size);

#ifndef trackAlloc
#define trackAlloc(pvAddress, uiSize) memMonitoringTrackAlloc(pvAddress, uiSize)
#endif
#ifndef trackFree
#define trackFree(pvAddress, uiSize) memMonitoringTrackFree(pvAddress, uiSize)
#endif

#define USE_FREERTOS

using namespace std;

// Define the new operator for C++ to use the freeRTOS memory management
// functions.
//
void * operator new(size_t size)
{
    void * p;
#ifdef USE_FREERTOS
    if (uxTaskGetNumberOfTasks())
        p = pvPortMalloc(size);
    else
        p = malloc(size);

#else
    p = malloc(size);

#endif
#ifdef __EXCEPTIONS
    if (p == 0)                 // did pvPortMalloc succeed?
        throw std::bad_alloc(); // ANSI/ISO compliant behavior
#endif
    return p;
}

//
// Define the delete operator for C++ to use the freeRTOS memory
// functions.
//
void operator delete(void * p)
{
#ifdef USE_FREERTOS
    if (uxTaskGetNumberOfTasks())
        vPortFree(p);
    else
        free(p);
#else
    free(p);
#endif
    p = NULL;
}

void * operator new[](size_t size)
{
    void * p;
#ifdef USE_FREERTOS
    if (uxTaskGetNumberOfTasks())
        p = pvPortMalloc(size);
    else
        p = malloc(size);

#else
    p = malloc(size);

#endif
#ifdef __EXCEPTIONS
    if (p == 0)                 // did pvPortMalloc succeed?
        throw std::bad_alloc(); // ANSI/ISO compliant behavior
#endif
    return p;
}

//
// Define the delete operator for C++ to use the freeRTOS memory
// functions. THIS IS NOT OPTIONAL!
//
void operator delete[](void * p)
{
#ifdef USE_FREERTOS
    if (uxTaskGetNumberOfTasks())
        vPortFree(p);
    else
        free(p);
#else
    free(p);
#endif
    p = NULL;
}

namespace chip {
namespace Platform {

#define VERIFY_INITIALIZED() VerifyInitialized(__func__)

static std::atomic_int memoryInitialized{ 0 };

static void VerifyInitialized(const char * func)
{
    if (!memoryInitialized)
    {
        ChipLogError(DeviceLayer, "ABORT: chip::Platform::%s() called before chip::Platform::MemoryInit()\n", func);
        abort();
    }
}

CHIP_ERROR MemoryAllocatorInit(void * buf, size_t bufSize)
{
    if (memoryInitialized++ > 0)
    {
        ChipLogError(DeviceLayer, "ABORT: chip::Platform::MemoryInit() called twice.\n");
        abort();
    }

    return CHIP_NO_ERROR;
}

void MemoryAllocatorShutdown()
{
    if (--memoryInitialized < 0)
    {
        ChipLogError(DeviceLayer, "ABORT: chip::Platform::MemoryShutdown() called twice.\n");
        abort();
    }
}

void * MemoryAlloc(size_t size)
{
    void * ptr;
    VERIFY_INITIALIZED();
    ptr = mbedtls_calloc(1, size);
    trackAlloc(ptr, size);
    return ptr;
}

void * MemoryAlloc(size_t size, bool isLongTermAlloc)
{
    void * ptr;
    VERIFY_INITIALIZED();
    ptr = mbedtls_calloc(1, size);
    trackAlloc(ptr, size);
    return ptr;
}

void * MemoryCalloc(size_t num, size_t size)
{
    VERIFY_INITIALIZED();

    void * ptr = mbedtls_calloc(1, size);
    trackAlloc(ptr, size * num);
    return ptr;
}

void * MemoryRealloc(void * p, size_t size)
{
    VERIFY_INITIALIZED();

    p = realloc(p, size);
    return p;
}

void MemoryFree(void * p)
{
    VERIFY_INITIALIZED();
    mbedtls_free(p);
    trackFree(p, 0);
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
