#pragma once

#include <devices/my-sensor/WindowCovering.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip::app {

class LoggingWindowCovering : public WindowCovering, public Clusters::WindowCoveringDelegate
{
public:
    explicit LoggingWindowCovering(TimerDelegate & timerDelegate);
    ~LoggingWindowCovering() override = default;

    // IdentifyDelegate implementation
    // TODO: ADD On<> callbacks in WindowCoveringDelegate
};

} // namespace chip::app
