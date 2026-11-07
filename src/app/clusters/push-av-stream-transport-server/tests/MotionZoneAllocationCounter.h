#pragma once

#include <cstddef>
#include <cstdint>

namespace chip::app::Clusters::PushAvStreamTransport {

class MotionZoneAllocationCounter
{
public:
#if defined(CHIP_TEST_MOTION_ZONE_ALLOCATION_TRACKING)
    static void Begin();
    static void End();
    static size_t Calls();
    static size_t RequestedBytes();
    static size_t FailedCalls();
    static size_t PlatformCalls();
    static size_t SuccessfulBytes();
    static void SetLimits(size_t maximumBlock = SIZE_MAX, size_t failedAttempt = 0);
#else
    static void Begin() {}
    static void End() {}
    static void SetLimits(size_t = SIZE_MAX, size_t = 0) {}
#endif
};

} // namespace chip::app::Clusters::PushAvStreamTransport
