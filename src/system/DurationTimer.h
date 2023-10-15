#pragma once

#include <string>

#include <system/SystemConfig.h>
/**
 * Uses SystemClock.h HAVE_CLOCK_GETTIME to determine board type i.e. esp32 or nRF etc.
 * CHIP_DEVICE_USES_SYS_TIME flag indicates nRF board whose clock implementation is available in Syste,Clock.h.
 * CHIP_DEVICE_USES_TIME_H flag for esp32 uses a time.h timespec
*/

//#if CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME
//#define CHIP_DEVICE_USES_SYS_TIME 1
//#else
#if (CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME && (CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS || CHIP_SYSTEM_CONFIG_USE_SOCKETS))
#define CHIP_DEVICE_USES_SYS_TIME 1
#else //CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS
#define CHIP_DEVICE_USES_TIME_H 1
#endif //CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS
//#else //CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME
//#define CHIP_DEVICE_USES_TIME_H 1
//#endif //CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME

#if CHIP_DEVICE_USES_SYS_TIME
#include <system/SystemClock.h>
#endif

#if CHIP_DEVICE_USES_TIME_H
#include <time.h>
#endif

using namespace std;

// todo add description
namespace chip {
namespace timing {

// todo add description
class DurationTimer
{
#ifdef CHIP_DEVICE_USES_SYS_TIME
private:
    string toTimeStr(timeval * time);
    void ToTimeval(chip::System::Clock::Microseconds64 in, timeval & out);

protected:
    timeval t1;
    timeval t2;
    string label;
#endif

#ifdef CHIP_DEVICE_USES_TIME_H
private:
    string toTimeStr(timespec * time);

protected:
    timespec t1;
    timespec t2;
    string label;
#endif

public:
    // constructors
    DurationTimer(string s) { label = s; }
    ~DurationTimer() = default;
    // member functions
    void start();
    void stop();
    double duration();
};

DurationTimer GetDefaultTimingInstance(string label);
DurationTimer * GetDefaultTimingInstancePtr(string label);

} // namespace timing
} // namespace chip