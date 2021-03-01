/* See Project CHIP LICENSE file for licensing information. */
#include <support/CHIPPlatformMemory.h>

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <openthread/platform/memory.h>

extern "C" void *otPlatCAlloc(size_t aNum, size_t aSize)
{
    return CHIPPlatformMemoryCalloc(aNum, aSize);
}

extern "C" void otPlatFree(void *aPtr)
{
    CHIPPlatformMemoryFree(aPtr);
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD
