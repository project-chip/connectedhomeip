#include "DurationTimer.h"
#include <stdint.h>
#include <string>
#include <lib/support/logging/CHIPLogging.h>

#if defined  (CHIP_DEVICE_IS_NRF)  
#include <system/SystemClock.h>
#endif

#if defined  (CHIP_DEVICE_IS_ESP32) || defined (CHIP_DEVICE_IS_ESP32_2)
#include <time.h>
#endif

using namespace std;
using namespace std::literals;

//todo add description
namespace chip{
    namespace timing{

        #define DATETIME_PATTERN  ("%Y-%m-%dT%H:%M:%S")
        
        #define DATETIME_LEN (sizeof "1970-01-01T23:59:59")
        
        #define ISO8601_LEN (sizeof "1970-01-01T23:59:59.123456Z")
        
        #ifdef CHIP_DEVICE_IS_NRF
        /*const FabricTable * fabricTable;
        System::Clock::Seconds32 lastKnownGoodChipEpochTime;
        err = mFabricsTable->GetLastKnownGoodChipEpochTime(lastKnownGoodChipEpochTime);
        if (err != CHIP_NO_ERROR)
        {
            // If we have no time available, the Validity Policy will
            // determine what to do.
            ChipLogError(DeviceLayer, "Failed to retrieve Last Known Good UTC Time");
        }*/
        
        /**   DurationTimer implementations   */

        // member functions
        void DurationTimer::start()
        {
            chip::System::Clock::Timestamp tv1 = chip::System::SystemClock().GetMonotonicTimestamp();

            chip::System::Clock::ToTimeval(tv1, t1 );

            ChipLogDetail(DeviceLayer, "Timer: %s start time: %s ", label.c_str(), toTimeStr(&t1).c_str());
        }

        void DurationTimer::stop()
        {
        
            chip::System::Clock::Timestamp tv2 = chip::System::SystemClock().GetMonotonicTimestamp();

            chip::System::Clock::ToTimeval(tv2, t2 );

            ChipLogDetail(DeviceLayer, "Timer: %s stop time: %s ", label.c_str(), toTimeStr(&t2).c_str());
            
            duration();
        }

        double DurationTimer::duration()
        {
            double dur =  ((t2.tv_usec - t1.tv_usec) * 1e-6);

            string timestr = toTimeStr(&t2);

            ChipLogDetail(DeviceLayer, "Timer: %s TIME_SPENT (sec) %f , time: %s ", label.c_str(), dur, timestr.c_str() );

            return dur;
        }

        // utility method
        string DurationTimer::toTimeStr(timeval* time)
        {
            char * buff = new char[DATETIME_LEN];

            struct tm* tm_info = gmtime(&(time->tv_sec));
            strftime(buff, DATETIME_LEN, DATETIME_PATTERN, tm_info);
            char * str = new char[ISO8601_LEN];
            snprintf(str, ISO8601_LEN, " %s.%05ld", buff, time->tv_usec);
            
            return str; 
        }
        #endif

        #ifdef CHIP_DEVICE_IS_ESP32
         // member functions
        void DurationTimer::start()
        {
            clock_gettime(CLOCK_REALTIME, &t1);

            ChipLogDetail(DeviceLayer, "Timer: %s start time: %s ", label.c_str(), toTimeStr(&t1).c_str());
        }

        void DurationTimer::stop()
        {
            clock_gettime(CLOCK_REALTIME, &t2);

            ChipLogDetail(DeviceLayer, "Timer: %s stop time: %s ", label.c_str(), toTimeStr(&t2).c_str());
            
            duration();
        }

        double DurationTimer::duration()
        {
            double dur = (double)(t2.tv_sec - t1.tv_sec) + ((t2.tv_nsec - t1.tv_nsec) * 1e-6);

            string timestr = toTimeStr(&t2);

            ChipLogDetail(DeviceLayer, "Timer: %s TIME_SPENT (sec) %f , time: %s ", label.c_str(), dur, timestr.c_str() );

            return dur;
        }

        // utility method
        string DurationTimer::toTimeStr(timespec* time)
        {
            char * buff = new char[DATETIME_LEN];

            struct tm* tm_info = gmtime(&(time->tv_sec));

            strftime(buff, DATETIME_LEN, DATETIME_PATTERN, tm_info);

            char * str = new char[ISO8601_LEN];

            snprintf(str, ISO8601_LEN, " %s.%05ld", buff, time->tv_nsec);
            
            return str; 
        }
        #endif

        DurationTimer GetDefaultTimingInstance(string label)
        {
            return chip::timing::DurationTimer(label);      
        }

        DurationTimer * GetDefaultTimingInstancePtr(string label)
        {
            return new chip::timing::DurationTimer(label);            
        }

    }
}