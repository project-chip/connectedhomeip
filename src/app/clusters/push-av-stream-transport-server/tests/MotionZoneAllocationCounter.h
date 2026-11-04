#pragma once

#include <cstddef>

namespace chip::app::Clusters::PushAvStreamTransport {

class MotionZoneAllocationCounter
{
public:
    static void Begin();
    static void End();
    static size_t Calls();
    static size_t RequestedBytes();
};

} // namespace chip::app::Clusters::PushAvStreamTransport
