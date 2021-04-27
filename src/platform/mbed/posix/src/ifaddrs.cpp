#include <ifaddrs.h>
#include <net_if.h>

int getifaddrs(struct ifaddrs ** ifap)
{
    return mbed_getifaddrs(ifap);
}

void freeifaddrs(struct ifaddrs * ifp)
{
    mbed_freeifaddrs(ifp);
}
