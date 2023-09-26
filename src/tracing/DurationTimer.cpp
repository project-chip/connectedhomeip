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


using namespace std;
using namespace std::literals;

//todo add description
namespace chip{
    namespace timing{
        /**   TimespecTimer implementations   */

        // member functions
        void TimespecTimer::start()
        {
            //System::Clock::Timestamp now = System::SystemClock().GetMonotonicTimestamp();
             t1 = std::chrono::system_clock::now();
           
            // todo use logging instead of stdout
            const std::time_t t_c = std::chrono::system_clock::to_time_t(t1);
            cout << "Timer " << (*label) << " start " << std::put_time(std::localtime(&t_c), "%F %T.\n") << std::flush;
        }

        void TimespecTimer::stop()
        {
        
            t2 = std::chrono::system_clock::now();

            const std::time_t t_c = std::chrono::system_clock::to_time_t(t2);
            cout << "Timer " << (*label) << " start " << std::put_time(std::localtime(&t_c), "%F %T.\n") << std::flush;
            duration();

            TimespecTimer::~TimespecTimer();
        }

        double TimespecTimer::duration()
        {
            double dur = (t2 - t1) / 1ms;
            cout << "Timer " << *label << " TIME_SPENT (msec) " << dur << '\n';
            return dur;
        }

        

    }
}