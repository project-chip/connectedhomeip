#include <arpa/inet.h>
#include <net_if.h>

static inline char * inet_ntop(sa_family_t family, const void * src, char * dst, size_t size)
{
    return NULL;
}

static inline int inet_pton(sa_family_t family, const char * src, void * dst)
{
    return 0;
}