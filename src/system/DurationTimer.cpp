#include "DurationTimer.h"
#include <stdint.h>
#include <string>
#include <lib/support/logging/CHIPLogging.h>

//#include <chrono>
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
        /**   TimespecTimer implementations   */

        // member functions
        void TimespecTimer::start()
        {
            chip::System::Clock::Timestamp tv1 = chip::System::SystemClock().GetMonotonicTimestamp();

            chip::System::Clock::ToTimeval(tv1, t1 );
            // todo use logging instead of stdout

            ChipLogDetail(DeviceLayer, "Timer: %s start %ld", label, t1.tv_usec);
            //ChipLogDetail(DeviceLayer, "Timer: start %ld", t1.tv_usec);
        }

        void TimespecTimer::stop()
        {
        
            chip::System::Clock::Timestamp tv2 = chip::System::SystemClock().GetMonotonicTimestamp();

            chip::System::Clock::ToTimeval(tv2, t2 );
            //ChipLogDetail(DeviceLayer, "Timer: %s stop %ld, ts: %s ", label, t2.tv_usec,TimespecTimer::toTimeStr(t2));
            ChipLogDetail(DeviceLayer, "Timer: %s stop %ld ", label, t2.tv_usec);
            //ChipLogDetail(DeviceLayer, "Timer: stop %ld, ts: %s ", t2.tv_usec,TimespecTimer::toTimeStr(t2));
            duration();

            //TimespecTimer::~TimespecTimer();
        }

        double TimespecTimer::duration()
        {
            double dur =  ((t2.tv_usec - t1.tv_usec) * 1e-6);

            chip::System::Clock::Seconds32 ts;
            //ChipLogDetail(DeviceLayer, "Timer: %s TIME_SPENT (sec) %f , ts: %s ", label, dur, TimespecTimer::toTimeStr(t2) );
            ChipLogDetail(DeviceLayer, "Timer: %s TIME_SPENT (sec) %f ", label, dur );
            //ChipLogDetail(DeviceLayer, "Timer: TIME_SPENT (sec) %f , ts: %s ", dur, TimespecTimer::toTimeStr(t2) );
            return dur;
        }

        #define DATETIME_PATTERN  ("%Y-%m-%dT%H:%M:%SZ")
        #define DATETIME_LEN (sizeof "1970-01-01T23:59:59.")
        #define ISO8601_LEN (sizeof "1970-01-01T23:59:59.123456Z")

        // utility method
        char * TimespecTimer::toTimeStr(timeval time)
        {
            char buff[DATETIME_LEN];
            strftime(buff, sizeof buff, DATETIME_PATTERN, gmtime(&time.tv_sec));
            char * str = new char[sizeof buff + 1];
            snprintf(str, ISO8601_LEN, " %s.%05ld", buff, time.tv_usec);
            return str; // timeval_to_str( buff, 0, &time);
        }

        

    }
}