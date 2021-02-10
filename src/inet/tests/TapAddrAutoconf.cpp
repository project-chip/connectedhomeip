/* See Project CHIP LICENSE file for licensing information. */


#include "TapAddrAutoconf.h"

#include <ifaddrs.h>
#include <inttypes.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

int CollectTapAddresses(std::vector<char *> & addresses, const char * ifName)
{
#if CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS
    struct ifaddrs *addrsList, *curAddr;
    const int err = getifaddrs(&addrsList);

    if (err == 0)
    {
        curAddr = addrsList;

        while (curAddr)
        {
            if (strcmp(ifName, curAddr->ifa_name) == 0)
            {
                char buf[INET6_ADDRSTRLEN];
                const char * rv;
                char * tmp;
                rv = inet_ntop(curAddr->ifa_addr->sa_family,
                               curAddr->ifa_addr->sa_family == AF_INET6
                                   ? (const void *) &(((sockaddr_in6 *) curAddr->ifa_addr)->sin6_addr)
                                   : (const void *) &(((sockaddr_in *) curAddr->ifa_addr)->sin_addr),
                               buf, sizeof(buf));
                if (rv != NULL)
                {
                    tmp = (char *) malloc(strlen(buf));
                    strcpy(tmp, buf);
                    addresses.push_back(tmp);
                }
            }
            curAddr = curAddr->ifa_next;
        }
        freeifaddrs(addrsList);
    }
    else
    {
        return -1;
    }

    return 0;
#else
#error "Unsupported configuration: requires CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS"
#endif
}
