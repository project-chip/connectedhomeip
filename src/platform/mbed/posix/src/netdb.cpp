#include <net_dns.h>
#include <netdb.h>

int getaddrinfo(const char * nodename, const char * servname, const struct addrinfo * hints, struct addrinfo ** res)
{
    return mbed_getaddrinfo(nodename, servname, hints, res);
}

void freeaddrinfo(struct addrinfo * ai)
{
    return mbed_freeaddrinfo(ai);
}
