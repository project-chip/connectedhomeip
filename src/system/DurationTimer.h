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
                    //char* toTimeStr(std::chrono::time_point time);
                protected:
                    timeval t1;
                    timeval t2;
                    const char * label;
                    //string s;
                public:
                    //constructors
                    //TimespecTimer(uint8_t mod, string s ):  DurationTimer(mod, s){};
                    TimespecTimer( string s ){
                        //label =  s.c_str();
                        label =  s.data();
                        //label =  s;
                    }
                    ~TimespecTimer(){
                        delete label;
                        //t1=1;
                        //t2=1;
                    }
                    //member functions
                    void start();
                    void stop();
                    double duration(); 
                    char * toTimeStr(timeval time);

        };

    }
}