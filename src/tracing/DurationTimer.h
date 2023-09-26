#pragma once

#include <string>
//#include <time.h>
//#include <sys/time.h>
#include <iostream>
#include <stdio.h>
#include <chrono>
#include <ctime>

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
                    std::chrono::time_point<std::chrono::system_clock> t1;
                    std::chrono::time_point<std::chrono::system_clock> t2;
                    string * label;
                public:
                    //constructors
                    //TimespecTimer(uint8_t mod, string s ):  DurationTimer(mod, s){};
                    TimespecTimer( string s ){
                        label = &s;
                    }
                    ~TimespecTimer(){
                        delete label;
                        //t1=0;
                        //t2=0;
                    }
                    //member functions
                    void start();
                    void stop();
                    double duration(); 

        };

    }
}