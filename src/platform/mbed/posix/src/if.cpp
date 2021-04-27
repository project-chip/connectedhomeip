
#include <net/if.h>
#include <net_if.h>
#include <stddef.h>

struct if_nameindex * if_nameindex(void)
{
    return mbed_if_nameindex();
}

char * if_indextoname(unsigned int ifindex, char * ifname)
{
    return mbed_if_indextoname(ifindex, ifname);
}

unsigned int if_nametoindex(const char * ifname)
{
    return mbed_if_nametoindex(ifname);
}

void if_freenameindex(struct if_nameindex * ptr)
{
    mbed_if_freenameindex(ptr);
}
