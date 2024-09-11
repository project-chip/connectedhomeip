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
 *      This file contains platform specific implementations for stdlib malloc/calloc/realloc/free
 *      functions, so that CHIPPlatformMemory* works as intended with the platform's heap.
 */

#include "FreeRTOS.h"
#include "task.h"
#include <cstring>
#include <stdint.h>
#include <stdlib.h>

#ifndef NDEBUG
#include <atomic>
#include <cstdio>
#endif

#include <src/lib/support/logging/CHIPLogging.h>

#if CHIP_CONFIG_MEMORY_DEBUG_DMALLOC
#include <dmalloc.h>
#include <lib/support/SafeInt.h>
#endif // CHIP_CONFIG_MEMORY_DEBUG_DMALLOC

#include "fsl_component_mem_manager.h"

/* Assumes 8bit bytes! */
#define heapBITS_PER_BYTE ((size_t) 8)

/* Define the linked list structure.  This is used to link free blocks in order
of their memory address. */
typedef struct A_BLOCK_LINK
{
    struct A_BLOCK_LINK * pxNextFreeBlock; /*<< The next free block in the list. */
    size_t xBlockSize;                     /*<< The size of the free block. */
} BlockLink_t;

/* The size of the structure placed at the beginning of each allocated memory
block must by correctly byte aligned. */
static const size_t xHeapStructSize =
    (sizeof(BlockLink_t) + ((size_t) (portBYTE_ALIGNMENT - 1))) & ~((size_t) portBYTE_ALIGNMENT_MASK);

/* Gets set to the top bit of an size_t type.  When this bit in the xBlockSize
member of an BlockLink_t structure is set then the block belongs to the
application.  When the bit is free the block is still part of the free heap
space. */
static size_t xBlockAllocatedBit = ((size_t) 1) << ((sizeof(size_t) * heapBITS_PER_BYTE) - 1);

extern "C" {

void * __wrap_malloc(size_t size)
{
    return pvPortMalloc(size);
}

void __wrap_free(void * ptr)
{
    vPortFree(ptr);
}

void * __wrap_calloc(size_t num, size_t size)
{
    size_t total_size = num * size;

    // Handle overflow from (num * size)
    if ((size != 0) && ((total_size / size) != num))
    {
        return nullptr;
    }

    void * ptr = pvPortMalloc(total_size);
    if (ptr)
    {
        memset(ptr, 0, total_size);
    }
    else
    {
        ChipLogError(DeviceLayer, "__wrap_calloc: Could not allocate memory!");
    }

    return ptr;
}

void * __wrap_realloc(void * ptr, size_t new_size)
{
    void * new_ptr = NULL;

    if (new_size)
    {
        /* MML will only realloc a new pointer if the size is greater than old pointer size.*/
        vTaskSuspendAll();
        new_ptr = MEM_BufferRealloc(ptr, new_size);
        xTaskResumeAll();

        if (!new_ptr)
        {
            ChipLogError(DeviceLayer, "__wrap_realloc: Could not allocate memory!");
            return NULL;
        }
    }

    return new_ptr;
}

void * __wrap__malloc_r(void * REENT, size_t size)
{
    return __wrap_malloc(size);
}

void __wrap__free_r(void * REENT, void * ptr)
{
    __wrap_free(ptr);
}

void * __wrap__realloc_r(void * REENT, void * ptr, size_t new_size)
{
    return __wrap_realloc(ptr, new_size);
}

} // extern "C"
