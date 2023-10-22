
#include <stdint.h>
#include <string>
#include <system/DurationTimer.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace std::literals;

namespace chip {
namespace timing {

#define DATETIME_PATTERN ("%Y-%m-%dT%H:%M:%S")

#define DATETIME_LEN (sizeof "1970-01-01T23:59:59")

#define ISO8601_LEN (sizeof "time: 1970-01-01T23:59:59.123456Z")

//#if CHIP_DEVICE_USES_SYS_TIME
// member functions
void DurationTimer::start()
{
    t1 = chip::System::SystemClock().GetMonotonicTimestamp();

    //chip::System::Clock::ToTimeval(tv1, t1);

    //ChipLogDetail(DeviceLayer, "Timer: %s start %s ", label.c_str(), toTimeStr(&t1).c_str());
    printf("Timer: %s start %s \n", label.c_str(), toTimeStr(t1).c_str());

}

void DurationTimer::stop()
{

    t2 = chip::System::SystemClock().GetMonotonicTimestamp();

    //chip::System::Clock::ToTimeval(tv2, t2);

    //ChipLogDetail(DeviceLayer, "Timer: %s stop %s ", label.c_str(), toTimeStr(&t2).c_str());
     printf("Timer: %s stop %s \n", label.c_str(), toTimeStr(t2).c_str());

    duration();
}

double DurationTimer::duration()
{
    //double dur = ((t2 - t1) * 1e-6);
    chip::System::Clock::Milliseconds64 millis = std::chrono::duration_cast<chip::System::Clock::Milliseconds64>(t2-t1);
    chip::System::Clock::Seconds32 seconds = std::chrono::duration_cast<chip::System::Clock::Seconds32>(t2-t1);
    //printf("Timer: secs %d  millis %lld  \n", seconds.count(), ((long long int)millis.count()) );
    millis -= seconds;
    //printf("Timer: secs %d  millis %d  \n", seconds.count(), ((int)millis.count()) );
    
    //time_t  oseconds = static_cast<time_t>(seconds.count());
    auto dur =  millis.count();//t2.count() - t1.count();
    suseconds_t omillis = static_cast<suseconds_t>(dur);

    std::string timestr = toTimeStr(t2);

    //ChipLogDetail(DeviceLayer, "Timer: %s TIME_SPENT (sec) %f , %s ", label.c_str(), dur, timestr.c_str());
    printf("Timer: %s %s TIME_SPENT (millisec) %d \n", label.c_str(),timestr.c_str(), ((int)omillis) );

    return (double)dur;
}

// utility method
std::string DurationTimer::toTimeStr(chip::System::Clock::Timestamp time)
{
    //long  seconds = (long) std::chrono::duration_cast<chip::System::Clock::Seconds32>(&time).count();
    //long  millis = (long) std::chrono::duration_cast<chip::System::Clock::Milliseconds64>(&time).count();
    chip::System::Clock::Milliseconds64 in = std::chrono::duration_cast<chip::System::Clock::Milliseconds64>(time);
    //chip::System::Clock::Milliseconds64 in = (chip::System::Clock::Milliseconds64) (time);
    chip::System::Clock::Seconds32 seconds = std::chrono::duration_cast<chip::System::Clock::Seconds32>(time);
    in -= seconds;
   //printf("Timer: in %lld  seconds %d  \n", in.count(), seconds.count() );
    time_t  oseconds = static_cast<time_t>(seconds.count());
    suseconds_t omillis = static_cast<suseconds_t>(in.count());
   //printf("Timer: oseconds %lld  omillis %ld  \n", oseconds, omillis );
    char * buff = new char[DATETIME_LEN];

    struct tm * tm_info = gmtime(&oseconds);
    strftime(buff, DATETIME_LEN, DATETIME_PATTERN, tm_info);
    char * str = new char[ISO8601_LEN];//TODO look into Direct leak of 68 byte(s) in 2 object(s) allocated from:
    //snprintf(str, ISO8601_LEN, "time: %s.%05ld", buff, omillis);
    snprintf(str, ISO8601_LEN, "time: %s.%d", buff, ((int)omillis));

    return str;
}
//#endif
/*#if CHIP_DEVICE_USES_TIME_H
// member functions
void DurationTimer::start()
{
    clock_gettime(CLOCK_REALTIME, &t1);

    printf("Timer: %s start %s \n", label.c_str(), toTimeStr(&t1).c_str());
}

void DurationTimer::stop()
{
    clock_gettime(CLOCK_REALTIME, &t2);

    printf("Timer: %s stop %s \n", label.c_str(), toTimeStr(&t2).c_str());

    duration();
}

double DurationTimer::duration()
{
    double dur = (double) (t2.tv_sec - t1.tv_sec) + ((t2.tv_nsec - t1.tv_nsec) * 1e-9);
    // printf("Timer: t2.sec= %lld t1.sec= %lld : t2.nsec= %ld  t1.nsec= %ld  \n", t2.tv_sec, t1.tv_sec, t2.tv_nsec, t1.tv_nsec);
    string timestr = toTimeStr(&t2);

    printf("Timer: %s TIME_SPENT (sec) %f , %s \n", label.c_str(), dur, timestr.c_str());

    return dur;
}

// utility method
string DurationTimer::toTimeStr(timespec * time)
{
    char * buff = new char[DATETIME_LEN];

    struct tm * tm_info = gmtime(&(time->tv_sec));

    strftime(buff, DATETIME_LEN, DATETIME_PATTERN, tm_info);

    char * str = new char[ISO8601_LEN];

    snprintf(str, ISO8601_LEN, "time: %s.%05ld", buff, time->tv_nsec);

    return str;
}
#endif*/

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