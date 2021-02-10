/* See Project CHIP LICENSE file for licensing information. */


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
    // Here we do things like mbedtls_platform_set_calloc_free(), depending on configuration.
    return err;
}

void MemoryShutdown()
{
    if ((memoryInitializationCount > 0) && (--memoryInitializationCount == 0))
    {
        // Here we undo things like mbedtls_platform_set_calloc_free()
        MemoryAllocatorShutdown();
    }
}

} // namespace Platform
} // namespace chip
