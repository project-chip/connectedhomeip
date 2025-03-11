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
    return malloc(size);
}

void * MemoryAlloc(size_t size, bool isLongTermAlloc)
{
    return malloc(size);
}

void * MemoryCalloc(size_t num, size_t size)
{
    return calloc(num, size);
}

void * MemoryRealloc(void * p, size_t size)
{
    return realloc(p, size);
}

void MemoryFree(void * p)
{
    free(p);
}

bool MemoryInternalCheckPointer(const void * p, size_t min_size)
{
    // We don't have a way to know if p is allocated from the heap, just do a null check here.
    return (p != nullptr);
}

} // namespace Platform
} // namespace chip
