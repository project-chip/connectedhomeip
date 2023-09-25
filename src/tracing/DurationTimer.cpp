#include "DurationTimer.h"
#include <stdint.h>
//#include <time.h>
#include <sys/time.h>
#include <string>
#include <lib/support/logging/CHIPLogging.h>

using namespace std;

//todo add description
namespace chip{
    namespace timing{
        /**   TimespecTimer implementations   */

                //static function
            double TimespecTimer::duration_calc(timeval start, timeval stop)
        {
            return (stop.tv_sec - start.tv_sec) + ((stop.tv_usec - start.tv_usec) * 1e-9);
        }

        // member functions
        void TimespecTimer::start()
        {
            struct timeval now;
            gettimeofday(&now, NULL);
            t1 = now;

            // todo use logging instead of stdout
            cout << "Timer " << label << " start " << toTimeStr(now) << '\n';
        }

        void TimespecTimer::stop()
        {
            struct timeval now;
            gettimeofday(&now, NULL);
            t2 = now;

            cout << "Timer " << label << " stop " << toTimeStr(now) << '\n';
            duration();
        }

        double TimespecTimer::duration()
        {
            double dur = duration_calc(t1, t2);
            cout << "Timer " << label << " TIME_SPENT (sec) " << dur << '\n';
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