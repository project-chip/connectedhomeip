#include <net_if.h>

struct if_nameindex * mbed_if_nameindex(void)
{
    return NULL;
}

char * mbed_if_indextoname(unsigned int ifindex, char * ifname)
{
    return NULL;
}

unsigned int mbed_if_nametoindex(const char * ifname)
{
    return 0;
}

void mbed_if_freenameindex(struct if_nameindex * ptr) {}

int mbed_getifaddrs(struct ifaddrs ** ifap)
{
    return 0;
}

void mbed_freeifaddrs(struct ifaddrs * ifp)
{
    return;
}

char * mbed_inet_ntop(sa_family_t family, const void * src, char * dst, size_t size)
{
    return NULL;
}

int mbed_inet_pton(sa_family_t family, const char * src, void * dst)
{
    return 0;
}