#pragma once

#include <string>
#include <system/SystemConfig.h>
#include <system/SystemClock.h>
#include <lib/support/logging/CHIPLogging.h>

/**
 * @file
 * 
 *  Uses SystemClock.h to capture start and stop timestamp and calculate the duration in milliseconds.
*/

#define DURATION_SHOW_TIME 0

namespace chip {
namespace timing {

class DurationTimer
{
private:
    std::string toTimeStr(chip::System::Clock::Timestamp time);

protected:
    chip::System::Clock::Timestamp  t1;
    chip::System::Clock::Timestamp  t2;
    std::string label;

public:
    // constructor
    /**
     * Constructor sets label as an identifier of a unique process.
    */
    DurationTimer(std::string s) { label = s; }

    //default destructor
    ~DurationTimer() = default;

    // member functions
    /** 
     * Sets t1 as start time.
     * */
    void start();

    /** 
     * Sets t2 as stop time.
     * */
    void stop();

    /** 
     * Calculates difference between t2 and t1.
     * */
    double duration();
};

DurationTimer GetDefaultTimingInstance(std::string label);
DurationTimer * GetDefaultTimingInstancePtr(std::string label);

} // namespace timing
} // namespace chip