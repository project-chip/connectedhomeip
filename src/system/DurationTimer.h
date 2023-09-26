#pragma once

#include <string>
//#include <time.h>
//#include <sys/time.h>
#include <iostream>
#include <stdio.h>
#include <chrono>
#include <ctime>
#include <system/SystemClock.h>

using namespace std;

//using namespace chip::System;

//todo add description
namespace chip{
    namespace timing{
        
        //todo add description
        class TimespecTimer {
                
                private:
                    //char* toTimeStr(std::chrono::time_point time);
                protected:
                    chip::System::Clock::Timestamp t1;
                    chip::System::Clock::Timestamp t2;
                    string * label;
                public:
                    //constructors
                    //TimespecTimer(uint8_t mod, string s ):  DurationTimer(mod, s){};
                    TimespecTimer( string s ){
                        label = &s;
                    }
                    ~TimespecTimer(){
                        delete label;
                        t1=1;
                        t2=1;
                    }
                    //member functions
                    void start();
                    void stop();
                    long long int duration(); 

        };

    }
}