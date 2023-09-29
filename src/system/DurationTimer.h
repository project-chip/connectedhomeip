#pragma once

#include <string>
#include <system/SystemClock.h>

using namespace std;

//todo add description
namespace chip{
    namespace timing{
        
        //todo add description
        class DurationTimer {
                
                private:
                    string toTimeStr(timeval* time);
                protected:
                    timeval t1;
                    timeval t2;
                    string label;
                public:
                    //constructors
                    //TimespecTimer(uint8_t mod, string s ):  DurationTimer(mod, s){};
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