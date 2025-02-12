/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
#include <platform/CHIPDeviceLayer.h>

#include <esp_heap_caps.h>
#include <stdlib.h>

namespace chip {
namespace Platform {

CHIP_ERROR MemoryAllocatorInit(void * buf, size_t bufSize)
{
    return CHIP_NO_ERROR;
}

void MemoryAllocatorShutdown() {}

void * MemoryAlloc(size_t size)
{
#ifdef CONFIG_CHIP_MEM_ALLOC_MODE_INTERNAL
    return heap_caps_malloc(size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
#elif defined(CONFIG_CHIP_MEM_ALLOC_MODE_EXTERNAL)
    return heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
#else
    return malloc(size);
#endif
}

void * MemoryAlloc(size_t size, bool isLongTermAlloc)
{
#ifdef CONFIG_CHIP_MEM_ALLOC_MODE_INTERNAL
    return heap_caps_malloc(size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
#elif defined(CONFIG_CHIP_MEM_ALLOC_MODE_EXTERNAL)
    return heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
#else
    return malloc(size);
#endif
}

void * MemoryCalloc(size_t num, size_t size)
{
#ifdef CONFIG_CHIP_MEM_ALLOC_MODE_INTERNAL
    return heap_caps_calloc(num, size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
#elif defined(CONFIG_CHIP_MEM_ALLOC_MODE_EXTERNAL)
    return heap_caps_calloc(num, size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
#else
    return calloc(num, size);
#endif
}

void * MemoryRealloc(void * p, size_t size)
{
#ifdef CONFIG_CHIP_MEM_ALLOC_MODE_INTERNAL
    p = heap_caps_realloc(p, size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
#elif defined(CONFIG_CHIP_MEM_ALLOC_MODE_EXTERNAL)
    p = heap_caps_realloc(p, size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
#else
    p = realloc(p, size);
#endif
    return p;
}

void MemoryFree(void * p)
{
#ifdef CONFIG_MATTER_MEM_ALLOC_MODE_DEFAULT
    free(p);
#else
    heap_caps_free(p);
#endif
}

bool MemoryInternalCheckPointer(const void * p, size_t min_size)
{
    return (p != nullptr);
}

} // namespace Platform
} // namespace chip
