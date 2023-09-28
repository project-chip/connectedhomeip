#include "DurationTimer.h"
#include <stdint.h>
#include <string>
#include <lib/support/logging/CHIPLogging.h>

#include <chrono>
//#include <ctime>
//#include <iomanip>
//#include <algorithm>
//#include <sys/time.h>

#include <system/SystemClock.h>


using namespace std;
using namespace std::literals;

//todo add description
namespace chip{
    namespace timing{
        #define DATETIME_PATTERN  ("%Y-%m-%dT%H:%M:%S")
        #define DATETIME_LEN (sizeof "1970-01-01T23:59:59")
        #define ISO8601_LEN (sizeof "1970-01-01T23:59:59.123456Z")
        /**   TimespecTimer implementations   */

        // member functions
        void TimespecTimer::start()
        {
            chip::System::Clock::Timestamp tv1 = chip::System::SystemClock().GetMonotonicTimestamp();

            chip::System::Clock::ToTimeval(tv1, t1 );

            //ChipLogDetail(DeviceLayer, "Timer: %s start %ld", label.c_str(), t1.tv_usec);
            ChipLogDetail(DeviceLayer, "Timer: %s start time: %s ", label.c_str(), toTimeStr(&t1).c_str());
            //ChipLogDetail(DeviceLayer, "Timer: start %ld", t1.tv_usec);
        }

        void TimespecTimer::stop()
        {
        
            chip::System::Clock::Timestamp tv2 = chip::System::SystemClock().GetMonotonicTimestamp();

            chip::System::Clock::ToTimeval(tv2, t2 );

            //std::chrono::hh_mm_ss hms { tv2 }
            //string tfs = std::format("{:%T}", tv2);
            //ChipLogDetail(DeviceLayer, "Timer: %s stop %s ", label.c_str(), tfs);

            ChipLogDetail(DeviceLayer, "Timer: %s stop time: %s ", label.c_str(), toTimeStr(&t2).c_str());
            //ChipLogDetail(DeviceLayer, "Timer: %s stop %lld - %ld ", label.c_str(), t2.tv_sec, t2.tv_usec);
            duration();

        
        }

        double TimespecTimer::duration()
        {
            double dur =  ((t2.tv_usec - t1.tv_usec) * 1e-6);

            string timestr = toTimeStr(&t2);
            ChipLogDetail(DeviceLayer, "Timer: %s TIME_SPENT (sec) %f , time: %s ", label.c_str(), dur, timestr.c_str() );

            return dur;
        }

        // utility method
        string TimespecTimer::toTimeStr(timeval* time)
        {
            char * buff = new char[DATETIME_LEN];
            //struct tm* tm_info = localtime(&(time->tv_sec));
            struct tm* tm_info = gmtime(&(time->tv_sec));
            strftime(buff, DATETIME_LEN, DATETIME_PATTERN, tm_info);
            //string str(buff, DATETIME_LEN );
            char * str = new char[ISO8601_LEN];
            snprintf(str, ISO8601_LEN, " %s.%05ld", buff, time->tv_usec);
            
            return str; 
        }

    }
}