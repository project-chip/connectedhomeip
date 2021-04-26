#ifndef MBED_NET_NET_DNS_H
#define MBED_NET_NET_DNS_H

#include <netdb.h>

#ifdef __cplusplus
extern "C" {
#endif

int mbed_getaddrinfo(const char * nodename, const char * servname, const struct addrinfo * hints, struct addrinfo ** res);

void mbed_freeaddrinfo(struct addrinfo * ai);

#ifdef __cplusplus
}
#endif

#endif /* MBED_NET_NET_DNS_H */
