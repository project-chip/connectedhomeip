#ifndef MBED_POSIX_ARPA_INET_H
#define MBED_POSIX_ARPA_INET_H

#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Max length of the IPv4 address as a string. Defined by POSIX. */
#define INET_ADDRSTRLEN 16
/** Max length of the IPv6 address as a string. Takes into account possible
 * mapped IPv4 addresses.
 */
#define INET6_ADDRSTRLEN 46

#define INADDRSZ 4
#define IN6ADDRSZ 16
#define INT16SZ 2

char * inet_ntop(sa_family_t family, const void * src, char * dst, size_t size);
int inet_pton(sa_family_t family, const char * src, void * dst);

#ifdef __cplusplus
}
#endif

#endif /* MBED_POSIX_ARPA_INET_H */
