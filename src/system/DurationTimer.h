#pragma once

#include <string>
//#include <time.h>
//#include <sys/time.h>
//#include <iostream>
//#include <stdio.h>
//#include <chrono>
//#include <ctime>
#include <system/SystemClock.h>

using namespace std;

//using namespace chip::System;

//todo add description
namespace chip{
    namespace timing{
        
        //todo add description
        class TimespecTimer {
                
                private:
                    string toTimeStr(timeval* time);
                protected:
                    timeval t1;
                    timeval t2;
                    string label;
                public:
                    //constructors
                    //TimespecTimer(uint8_t mod, string s ):  DurationTimer(mod, s){};
                    TimespecTimer( string s ){
                        label =  s;
                    }
                    ~TimespecTimer()=default;
                    //member functions
                    void start();
                    void stop();
                    double duration(); 
                    //string toTimeStr(timeval* time);
                    //void toDateTime(char* timestr, timeval* time);

        };

    }
}