/*
 * Copyright (c) 2020 Project CHIP Authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 *    @file
 *          Provides a C-callable wrapper around CHIPMem.h.
 */

#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPPlatformMemory.h>

extern "C" {

extern int CHIPPlatformMemoryInit(void * buf, size_t bufSize)
{
    return static_cast<int>(chip::Platform::MemoryInit(buf, bufSize).GetError().AsInteger());
}

extern void CHIPPlatformMemoryShutdown()
{
    return chip::Platform::MemoryShutdown();
}

#if CHIP_SYSTEM_CONFIG_TYPED_MALLOC

extern void * CHIPPlatformMemoryAllocTyped(size_t size, malloc_type_id_t typeId)
{
    return chip::Platform::MemoryAllocTyped(size, typeId);
}

extern void * CHIPPlatformMemoryCallocTyped(size_t num, size_t size, malloc_type_id_t typeId)
{
    return chip::Platform::MemoryCallocTyped(num, size, typeId);
}

extern void * CHIPPlatformMemoryReallocTyped(void * p, size_t size, malloc_type_id_t typeId)
{
    return chip::Platform::MemoryReallocTyped(p, size, typeId);
}

#else

extern void * CHIPPlatformMemoryAlloc(size_t size)
{
    return chip::Platform::MemoryAlloc(size);
}

extern void * CHIPPlatformMemoryCalloc(size_t num, size_t size)
{
    return chip::Platform::MemoryCalloc(num, size);
}

extern void * CHIPPlatformMemoryRealloc(void * p, size_t size)
{
    return chip::Platform::MemoryRealloc(p, size);
}

#endif

extern void CHIPPlatformMemoryFree(void * p)
{
    return chip::Platform::MemoryFree(p);
}
}
