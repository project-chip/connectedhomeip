#include <arpa/inet.h>
#include <net_if.h>

char * inet_ntop(sa_family_t family, const void * src, char * dst, size_t size)
{
    return mbed_inet_ntop(family, src, dst, size);
}

int inet_pton(sa_family_t family, const char * src, void * dst)
{
    return mbed_inet_pton(family, src, dst);
}
