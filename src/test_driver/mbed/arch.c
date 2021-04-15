#include "platform/mbed_wait_api.h"

// This file provides some missing function implementations which are platform specific

void usleep(unsigned int usec) 
{
    wait_us(usec);
}