#ifndef CY_WCM_SHIM
#define CY_WCM_SHIM

// Shim to get cy wcm code to compile using the (older) CHIP-provided lwip.
struct ifreq
{
    char ifr_name[6];
};

#define SO_BINDTODEVICE 42

// Include the real cy_wcm.h
#include "libs/wifi-connection-manager/include/cy_wcm.h"

#endif
