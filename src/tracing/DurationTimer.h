#pragma once

#include <string>
#include <time.h>
#include <iostream>
#include <stdio.h>

using namespace std;

//todo add description
namespace chip{
    namespace timing{
        
        template <typename T>
        class DurationTimer{
            protected:
                T t1;
                T t2;
                string label;
                //todo revisit module references
                uint8_t module;
            public:    
                //constructors
                DurationTimer(uint8_t mod, string s ){
                   module = mod;
                   label = s;
                }
                DurationTimer(string s ){
                   label = s;
                }
                void start();
                void stop();
                double duration(); 
                static double duration_calc(T start, T stop); 
        };

        //todo add description
        class TimespecTimer : public DurationTimer<timespec>{
                
                private:
                    char* toTimeStr(timespec time);

                public:
                    //constructors
                    TimespecTimer(uint8_t mod, string s ):  DurationTimer(mod, s){};
                    TimespecTimer( string s ): DurationTimer(s){};

                    //member functions
                    void start();
                    void stop();
                    double duration(); 

                    double static duration_calc(timespec start, timespec stop) ;
        };

    }
}