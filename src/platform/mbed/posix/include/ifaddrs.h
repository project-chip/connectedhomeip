#ifndef MBED_POSIX_IFADDRS_H
#define MBED_POSIX_IFADDRS_H

#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

struct ifaddrs
{
    struct ifaddrs * ifa_next; /* Pointer to the next structure.  */
    char * ifa_name;           /* Name of this network interface.  */
    unsigned int ifa_flags;    /* Flags as from SIOCGIFFLAGS ioctl.  */

    struct sockaddr * ifa_addr;    /* Network address of this interface.  */
    struct sockaddr * ifa_netmask; /* Netmask of this interface.  */
    union
    {
        /* At most one of the following two is valid.  If the IFF_BROADCAST
           bit is set in `ifa_flags', then `ifa_broadaddr' is valid.  If the
           IFF_POINTOPOINT bit is set, then `ifa_dstaddr' is valid.
           It is never the case that both these bits are set at once.  */
        struct sockaddr * ifu_broadaddr; /* Broadcast address of this interface. */
        struct sockaddr * ifu_dstaddr;   /* Point-to-point destination address.  */
    } ifa_ifu;
    void * ifa_data; /* Address-specific data (may be unused).  */
};

int getifaddrs(struct ifaddrs ** ifap);
void freeifaddrs(struct ifaddrs * ifp);

#ifdef __cplusplus
}
#endif

#endif /* MBED_POSIX_IFADDRS_H */
