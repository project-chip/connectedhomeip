#include "MotionZoneAllocationCounter.h"

namespace {
thread_local bool gTracking         = false;
thread_local size_t gCalls          = 0;
thread_local size_t gRequestedBytes = 0;
} // namespace

namespace chip::app::Clusters::PushAvStreamTransport {

void MotionZoneAllocationCounter::Begin()
{
    gCalls          = 0;
    gRequestedBytes = 0;
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

} // namespace chip::app::Clusters::PushAvStreamTransport

#if defined(CHIP_TEST_MOTION_ZONE_ALLOCATION_TRACKING)
extern "C" void * __real__Znwm(size_t bytes);

extern "C" void * __wrap__Znwm(size_t bytes)
{
    void * allocation = __real__Znwm(bytes);
    if (gTracking)
    {
        ++gCalls;
        gRequestedBytes += bytes;
    }
    return allocation;
}
#endif
