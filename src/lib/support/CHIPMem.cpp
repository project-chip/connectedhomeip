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
 *      This file implements parts of the CHIP allocation API that are
 *      independent of the configured allocator.
 *
 */

#include <lib/core/CHIPConfig.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPPlatformMemory.h>
#include <system/SystemMutex.h>

#include <atomic>
#include <stdlib.h>

namespace chip {
namespace Platform {

extern CHIP_ERROR MemoryAllocatorInit(void * buf, size_t bufSize);
extern void MemoryAllocatorShutdown();

#if !CHIP_SYSTEM_CONFIG_NO_LOCKING
static chip::System::Mutex gMemoryLock;
#endif
static int memoryInitializationCount{ 0 };

CHIP_ERROR MemoryInit(void * buf, size_t bufSize)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
#if !CHIP_SYSTEM_CONFIG_NO_LOCKING
    gMemoryLock.Lock();
#endif
    do
    {
        if (memoryInitializationCount++ > 0)
        {
            // Already initialized
            break;
        }
        // Initialize the allocator.
        err = MemoryAllocatorInit(buf, bufSize);
        if (err != CHIP_NO_ERROR)
        {
            break;
        }
        // Here we do things like mbedtls_platform_set_calloc_free(), depending on configuration.
    } while (0);
#if !CHIP_SYSTEM_CONFIG_NO_LOCKING
    gMemoryLock.Unlock();
#endif
    return err;
}

void MemoryShutdown()
{
#if !CHIP_SYSTEM_CONFIG_NO_LOCKING
    gMemoryLock.Lock();
#endif
    if ((memoryInitializationCount > 0) && (--memoryInitializationCount == 0))
    {
        // Here we undo things like mbedtls_platform_set_calloc_free()
        MemoryAllocatorShutdown();
    }
#if !CHIP_SYSTEM_CONFIG_NO_LOCKING
    gMemoryLock.Unlock();
#endif
}

} // namespace Platform
} // namespace chip
