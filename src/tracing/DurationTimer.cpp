#include "DurationTimer.h"
#include <stdint.h>
//#include <time.h>
//#include <sys/time.h>
#include <string>
#include <lib/support/logging/CHIPLogging.h>
//#include <system/SystemClock.h>
//#include <src/system/SystemClock.h>
//#include <platform/CHIPDeviceLayer.h>

using namespace std;
using namespace chip;
//todo add description
namespace chip{
    namespace timing{
        /**   TimespecTimer implementations   */

                //static function
            double TimespecTimer::duration_calc(uint64_t start, uint64_t stop)
        {
            //return (stop.tv_sec - start.tv_sec) + ((stop.tv_usec - start.tv_usec) * 1e-9);
            //start.
            return (stop - start);
        }

        // member functions
        void TimespecTimer::start()
        {
            t1 = chip::System::SystemClock().GetMonotonicMilliseconds64().count();

            // todo use logging instead of stdout
            cout << "Timer " << label << " start " << chip::System::SystemClock().GetClock_RealTimeMS(t1) << '\n';
        }

        void TimespecTimer::stop()
        {
            t2 = chip::System::SystemClock().GetMonotonicMilliseconds64().count();

            cout << "Timer " << label << " stop " << chip::System::SystemClock().GetClock_RealTimeMS(t2) << '\n';
            duration();
        }

        double TimespecTimer::duration()
        {
            double dur = duration_calc(t1, t2);
            cout << "Timer " << label << " TIME_SPENT (sec) " << dur << '\n';
            return dur;
        }


    }
}