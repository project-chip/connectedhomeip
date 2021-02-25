#ifndef MBED_POSIX_ARPA_INET_H
#define MBED_POSIX_ARPA_INET_H

#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline char * inet_ntop(sa_family_t family, const void * src, char * dst, size_t size)
{
    return NULL;
}

static inline int inet_pton(sa_family_t family, const char * src, void * dst)
{
    return 0;
}

#ifdef __cplusplus
}
#endif

#endif /* MBED_POSIX_ARPA_INET_H */