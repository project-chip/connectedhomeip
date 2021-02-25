#include <ifaddrs.h>
#include <net_if.h>

int getifaddrs(struct ifaddrs ** ifap)
{
    return 0;
}

void freeifaddrs(struct ifaddrs * ifp) {}