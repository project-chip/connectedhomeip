#include "DurationTimer.h"
#include <stdint.h>
#include <time.h>
#include <string>
#include <lib/support/logging/CHIPLogging.h>

using namespace std;

//todo add description
namespace chip{
    namespace timing{
        /**   TimespecTimer implementations   */

        //static function
        double TimespecTimer::duration_calc(timespec start, timespec stop){
            return (stop.tv_sec - start.tv_sec)+((stop.tv_nsec - start.tv_nsec) * 1e-9);
        }

        //member functions
        void TimespecTimer::start(){
            struct timespec now;
            clock_gettime(CLOCK_REALTIME, &now);
            t1 = now;
            //todo use logging instead of stdout
            cout <<  "Timer "<< label << " start " << toTimeStr( now ) << '\n';
        }

        void TimespecTimer::stop(){
            struct timespec now;
            clock_gettime(CLOCK_REALTIME, &now) ;
            t2 = now;
            cout <<  "Timer "<< label << " stop " << toTimeStr( now ) << '\n';
            duration();
        }

        double TimespecTimer::duration(){
            double dur = duration_calc(t1,t2);
            cout <<  "Timer "<< label << " TIME_SPENT (sec) " << dur << '\n';
            return dur;
        }

        //utility method
        char* TimespecTimer::toTimeStr(timespec time){
            char buff[100];
            strftime(buff, sizeof buff, "%D %T", gmtime(&time.tv_sec) );
            char* str = new char[ sizeof buff + 1];
            //todo revisit this size
            snprintf(str, sizeof(buff)+4+21 , " %s.%09ld", buff, time.tv_nsec );
            return str;
        }

    }
}