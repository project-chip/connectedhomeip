#pragma once

#include <string>

/**
 * CHIP_DEVICE_IS_ESP32
 *
 * If 1, enable then assume esp32 device.
*/
//#ifndef CHIP_DEVICE_IS_ESP32
//#define CHIP_DEVICE_IS_ESP32 1
//#endif
 
/**
 * CHIP_DEVICE_IS_NRF
 *
 * If 1, enable then assume nRF device.
*/ 

#ifndef CHIP_DEVICE_IS_NRF
#define CHIP_DEVICE_IS_NRF 1
#undef CHIP_DEVICE_IS_ESP32
#endif

#if defined  (CHIP_DEVICE_IS_NRF)  
#include <system/SystemClock.h>
#endif

#if defined  (CHIP_DEVICE_IS_ESP32) || defined (CHIP_DEVICE_IS_ESP32_2)
#include <time.h>
#endif

using namespace std;

//todo add description
namespace chip{
    namespace timing{
        
        //todo add description
        class DurationTimer {
                #ifdef CHIP_DEVICE_IS_NRF
                private:
                    string toTimeStr(timeval* time);
                protected:
                    timeval t1;
                    timeval t2;
                    string label;
                #endif

                #ifdef CHIP_DEVICE_IS_ESP32
                private:
                    string toTimeStr(timespec* time);
                protected:
                    timespec t1;
                    timespec t2;
                    string label;
                #endif    

                public:
                    //constructors
                    DurationTimer( string s ){
                        label =  s;
                    }
                    ~DurationTimer()=default;
                    //member functions
                    void start();
                    void stop();
                    double duration(); 

        };

        DurationTimer GetDefaultTimingInstance(string label);
        DurationTimer * GetDefaultTimingInstancePtr(string label);

    }
}