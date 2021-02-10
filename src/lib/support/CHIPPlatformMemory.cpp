/* See Project CHIP LICENSE file for licensing information. */

/**
 *    @file
 *          Provides a C-callable wrapper around CHIPMem.h.
 */

#include <support/CHIPMem.h>
#include <support/CHIPPlatformMemory.h>

extern "C" {

extern int CHIPPlatformMemoryInit(void * buf, size_t bufSize)
{
    return static_cast<int>(chip::Platform::MemoryInit(buf, bufSize));
}

extern void CHIPPlatformMemoryShutdown()
{
    return chip::Platform::MemoryShutdown();
}

extern void * CHIPPlatformMemoryAllocLongTerm(size_t size, bool isLongTermAlloc)
{
    return chip::Platform::MemoryAlloc(size, isLongTermAlloc);
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
