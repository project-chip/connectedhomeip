
//#include <stdint.h>
#include <system/DurationTimer.h>

//using namespace std::literals;

namespace chip {
namespace timing {

#if (DURATION_SHOW_TIME)
#define DATETIME_PATTERN ("%Y-%m-%dT%H:%M:%S")
#define DATETIME_LEN (sizeof "1970-01-01T23:59:59")
#define ISO8601_LEN (sizeof "time: 1970-01-01T23:59:59.123456Z")
#endif

// member functions
void DurationTimer::start()
{
    t1 = chip::System::SystemClock().GetMonotonicTimestamp();

    //ChipLogDetail(DeviceLayer, "Timer: %s start %s ", label.c_str(), toTimeStr(&t1).c_str());
    printf("Timer: %s start %s \n", label.c_str(), toTimeStr(t1).c_str());

}

void DurationTimer::stop()
{

    t2 = chip::System::SystemClock().GetMonotonicTimestamp();

    //ChipLogDetail(DeviceLayer, "Timer: %s stop %s ", label.c_str(), toTimeStr(&t2).c_str());
     printf("Timer: %s stop %s \n", label.c_str(), toTimeStr(t2).c_str());

    duration();
}

double DurationTimer::duration()
{
    chip::System::Clock::Milliseconds64 millis = std::chrono::duration_cast<chip::System::Clock::Milliseconds64>(t2-t1);
    chip::System::Clock::Seconds32 seconds = std::chrono::duration_cast<chip::System::Clock::Seconds32>(t2-t1);
    millis -= seconds;
    suseconds_t omillis = static_cast<suseconds_t>(millis.count());

    //printf("Timer: %s %s TIME_SPENT (millisec) %d \n", label.c_str(), toTimeStr(t2).c_str(), ((int)omillis) );
    printf("Timer: %s TIME_SPENT (millisec) %d \n", label.c_str(), ((int)omillis) );

    return (double)millis.count();
}

// utility method
std::string DurationTimer::toTimeStr(chip::System::Clock::Timestamp time)
{
    #if (!DURATION_SHOW_TIME)
    std::string str;
    #else
    char * str = new char[ISO8601_LEN];//TODO look into Direct leak of 68 byte(s) in 2 object(s) allocated from:
    chip::System::Clock::Milliseconds64 in = std::chrono::duration_cast<chip::System::Clock::Milliseconds64>(time);
    chip::System::Clock::Seconds32 seconds = std::chrono::duration_cast<chip::System::Clock::Seconds32>(time);
    in -= seconds;
    time_t  oseconds = static_cast<time_t>(seconds.count());
    suseconds_t omillis = static_cast<suseconds_t>(in.count());
    char * buff = new char[DATETIME_LEN];

    struct tm * tm_info = gmtime(&oseconds);
    strftime(buff, DATETIME_LEN, DATETIME_PATTERN, tm_info);
    //snprintf(str, ISO8601_LEN, "time: %s.%05ld", buff, omillis);
    snprintf(str, ISO8601_LEN, "time: %s.%03d", buff, ((int)omillis));
    #endif
    return str;
}

DurationTimer GetDefaultTimingInstance(std::string label)
{
    return chip::timing::DurationTimer(label);
}

DurationTimer * GetDefaultTimingInstancePtr(std::string label)
{
    return new chip::timing::DurationTimer(label);
}

} // namespace timing
} // namespace chip