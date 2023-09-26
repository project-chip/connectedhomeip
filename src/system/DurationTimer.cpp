#include "DurationTimer.h"
#include <stdint.h>
//#include <time.h>
//#include <sys/time.h>
#include <string>
#include <lib/support/logging/CHIPLogging.h>

#include <chrono>
#include <ctime>
#include <iomanip>
#include <algorithm>

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
             t1 = chip::System::SystemClock().GetMonotonicTimestamp();
           
            // todo use logging instead of stdout
            //const std::time_t t_c = std::chrono::system_clock::to_time_t(t1);
            //cout << "Timer " << (*label) << " start " << std::put_time(std::localtime(&t_c), "%F %T.\n") << std::flush;
        }

        void TimespecTimer::stop()
        {
        
            t2 = chip::System::SystemClock().GetMonotonicTimestamp();

            //const std::time_t t_c = std::chrono::system_clock::to_time_t(t2);
            //cout << "Timer " << (*label) << " start " << std::put_time(std::localtime(&t_c), "%F %T.\n") << std::flush;
            duration();

            TimespecTimer::~TimespecTimer();
        }

        long long int TimespecTimer::duration()
        {
            long long int dur = (t2.count() - t1.count());
            cout << "Timer " << *label << " TIME_SPENT (msec) " << dur << '\n';
            return dur;
        }

        

    }
}