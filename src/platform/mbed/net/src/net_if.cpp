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