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

#include <core/CHIPConfig.h>
#include <support/CHIPMem.h>
#include <support/CHIPPlatformMemory.h>

#include <atomic>
#include <stdlib.h>

// Decide whether MemoryInit and MemoryShutdown should configure mbedtls.
#ifdef MBEDTLS_CONFIG_FILE
#include MBEDTLS_CONFIG_FILE
#if defined(MBEDTLS_PLATFORM_MEMORY) && !defined(MBEDTLS_PLATFORM_STD_CALLOC)
#define USE_MBEDTLS_PLATFORM_SET_CALLOC_FREE 1
#include <mbedtls/platform.h>
#endif
#endif

namespace chip {
namespace Platform {

extern CHIP_ERROR MemoryAllocatorInit(void * buf, size_t bufSize);
extern void MemoryAllocatorShutdown();

static std::atomic_int memoryInitializationCount{ 0 };

CHIP_ERROR MemoryInit(void * buf, size_t bufSize)
{
    if (memoryInitializationCount++ > 0)
    {
        return CHIP_NO_ERROR;
    }
    // Initialize the allocator.
    CHIP_ERROR err = MemoryAllocatorInit(buf, bufSize);
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }
    // Initialization specific to platform configuration.
#ifdef USE_MBEDTLS_PLATFORM_SET_CALLOC_FREE
    mbedtls_platform_set_calloc_free(CHIPPlatformMemoryCalloc, CHIPPlatformMemoryFree);
#endif
    return err;
}

void MemoryShutdown()
{
    if ((memoryInitializationCount > 0) && (--memoryInitializationCount == 0))
    {
        // Finalization specific to platform configuration.
#ifdef USE_MBEDTLS_PLATFORM_SET_CALLOC_FREE
        mbedtls_platform_set_calloc_free(nullptr, nullptr);
#endif
        MemoryAllocatorShutdown();
    }
}

} // namespace Platform
} // namespace chip
