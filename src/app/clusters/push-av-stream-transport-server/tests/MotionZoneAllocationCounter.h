#pragma once

#include <cstddef>

namespace chip::app::Clusters::PushAvStreamTransport {

class MotionZoneAllocationCounter
{
public:
#if defined(CHIP_TEST_MOTION_ZONE_ALLOCATION_TRACKING)
    static void Begin();
    static void End();
    static size_t Calls();
    static size_t RequestedBytes();
#else
    static void Begin() {}
    static void End() {}
#endif
};

} // namespace chip::app::Clusters::PushAvStreamTransport
