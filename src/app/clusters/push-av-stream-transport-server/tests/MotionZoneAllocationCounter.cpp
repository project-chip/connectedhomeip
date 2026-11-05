#include "MotionZoneAllocationCounter.h"
#include <cstdlib>

#if defined(CHIP_TEST_MOTION_ZONE_ALLOCATION_TRACKING)
namespace {
thread_local bool gTracking         = false;
thread_local size_t gCalls          = 0;
thread_local size_t gRequestedBytes = 0;
thread_local size_t gFailedCalls     = 0;
thread_local size_t gPlatformCalls   = 0;
thread_local size_t gSuccessfulBytes = 0;
thread_local size_t gMaximumBlock    = SIZE_MAX;
thread_local size_t gFailedAttempt   = 0;
} // namespace

namespace chip::app::Clusters::PushAvStreamTransport {

void MotionZoneAllocationCounter::Begin()
{
    gCalls          = 0;
    gRequestedBytes = 0;
    gFailedCalls     = 0;
    gPlatformCalls   = 0;
    gSuccessfulBytes = 0;
    gTracking       = true;
}

void MotionZoneAllocationCounter::End()
{
    gTracking = false;
}

size_t MotionZoneAllocationCounter::Calls()
{
    return gCalls;
}

size_t MotionZoneAllocationCounter::RequestedBytes()
{
    return gRequestedBytes;
}

size_t MotionZoneAllocationCounter::FailedCalls()
{
    return gFailedCalls;
}
size_t MotionZoneAllocationCounter::PlatformCalls()
{
    return gPlatformCalls;
}
size_t MotionZoneAllocationCounter::SuccessfulBytes()
{
    return gSuccessfulBytes;
}

void MotionZoneAllocationCounter::SetLimits(size_t maximumBlock, size_t failedAttempt)
{
    gMaximumBlock  = maximumBlock;
    gFailedAttempt = failedAttempt;
}

} // namespace chip::app::Clusters::PushAvStreamTransport

extern "C" void * __real__Znwm(size_t bytes);
extern "C" void * __real__ZN4chip8Platform11MemoryAllocEm(size_t bytes);

extern "C" void * __wrap__ZN4chip8Platform11MemoryAllocEm(size_t bytes)
{
    if (gTracking)
    {
        ++gCalls;
        ++gPlatformCalls;
        gRequestedBytes += bytes;
        if (bytes > gMaximumBlock || gPlatformCalls == gFailedAttempt)
        {
            ++gFailedCalls;
            return nullptr;
        }
    }
    void * allocation = __real__ZN4chip8Platform11MemoryAllocEm(bytes);
    if (gTracking)
    {
        if (allocation != nullptr)
        {
            gSuccessfulBytes += bytes;
        }
        else
        {
            ++gFailedCalls;
        }
    }
    return allocation;
}

extern "C" void * __wrap__Znwm(size_t bytes)
{
    if (gTracking && bytes > gMaximumBlock)
    {
        std::abort();
    }
    void * allocation = __real__Znwm(bytes);
    if (gTracking)
    {
        ++gCalls;
        gRequestedBytes += bytes;
        gSuccessfulBytes += bytes;
    }
    return allocation;
}
#endif
