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

#include <support/CHIPMem.h>
#include <support/CHIPPlatformMemory.h>

extern "C" {

extern int CHIPPlatformMemoryInit(void * buf, size_t bufSize)
{
    return static_cast<int>(chip::ChipError::AsInteger(chip::Platform::MemoryInit(buf, bufSize)));
}

extern void CHIPPlatformMemoryShutdown()
{
    return chip::Platform::MemoryShutdown();
}

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

extern void CHIPPlatformMemoryFree(void * p)
{
    return chip::Platform::MemoryFree(p);
}
}
