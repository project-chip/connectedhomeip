/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2002-2004 Apple Computer, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "lwip/inet.h"
#include <stdio.h>
#include "mDNSUNP.h"

#include <errno.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
//#include <sys/uio.h>
//#include <sys/ioctl.h>
#include <signal.h>
#include <stdio.h>

/* Some weird platforms derived from 4.4BSD Lite (e.g. EFI) need the ALIGN(P)
   macro, usually defined in <sys/param.h> or someplace like that, to make sure the
   CMSG_NXTHDR macro is well-formed. On such platforms, the symbol NEED_ALIGN_MACRO
   should be set to the name of the header to include to get the ALIGN(P) macro.
 */
#ifdef NEED_ALIGN_MACRO
#include NEED_ALIGN_MACRO
#endif

/* Solaris defined SIOCGIFCONF etc in <sys/sockio.h> but
   other platforms don't even have that include file.  So,
   if we haven't yet got a definition, let's try to find
   <sys/sockio.h>.
 */

//#ifndef SIOCGIFCONF
//    #include <sys/sockio.h>
//#endif

/* sockaddr_dl is only referenced if we're using IP_RECVIF,
   so only include the header in that case.
 */

#ifdef  IP_RECVIF
    #include <net/if_dl.h>
#endif

//#if defined(AF_INET6) && HAVE_IPV6 && !HAVE_LINUX
//#include <net/if_var.h>
//#include <netinet/in_var.h>
// Note: netinet/in_var.h implicitly includes netinet6/in6_var.h for us
//#endif

#if defined(AF_INET6) && HAVE_IPV6 && HAVE_LINUX
//#include <netdb.h>
//#include <arpa/inet.h>

/* Converts a prefix length to IPv6 network mask */
void plen_to_mask(int plen, char *addr) {
    int i;
    int colons=7; /* Number of colons in IPv6 address */
    int bits_in_block=16; /* Bits per IPv6 block */
    for(i=0; i<=colons; i++) {
        int block, ones=0xffff, ones_in_block;
        if (plen>bits_in_block) ones_in_block=bits_in_block;
        else ones_in_block=plen;
        block = ones & (ones << (bits_in_block-ones_in_block));
        i==0 ? sprintf(addr, "%x", block) : sprintf(addr, "%s:%x", addr, block);
        plen -= ones_in_block;
    }
}

/* Gets IPv6 interface information from the /proc filesystem in linux*/
struct ifi_info *get_ifi_info_linuxv6(int family, int doaliases)
{
    struct ifi_info *ifi, *ifihead, **ifipnext, *ifipold, **ifiptr;
#if 0
    FILE *fp;
    char addr[8][5];
    int flags, myflags, index, plen, scope;
    char ifname[9], lastname[IFNAMSIZ];
    char addr6[32+7+1]; /* don't forget the seven ':' */
    struct addrinfo hints, *res0;
    struct sockaddr_in6 *sin6;
    struct in6_addr *addrptr;
    int err;
    int sockfd = -1;
    struct ifreq ifr;

    res0=NULL;
    ifihead = NULL;
    ifipnext = &ifihead;
    lastname[0] = 0;

    if ((fp = fopen(PROC_IFINET6_PATH, "r")) != NULL) {
        sockfd = socket(AF_INET6, SOCK_DGRAM, 0);
        if (sockfd < 0) {
            goto gotError;
        }
        while (fscanf(fp,
                      "%4s%4s%4s%4s%4s%4s%4s%4s %02x %02x %02x %02x %8s\n",
                      addr[0],addr[1],addr[2],addr[3],
                      addr[4],addr[5],addr[6],addr[7],
                      &index, &plen, &scope, &flags, ifname) != EOF) {

            myflags = 0;
            if (strncmp(lastname, ifname, IFNAMSIZ) == 0) {
                if (doaliases == 0)
                    continue;   /* already processed this interface */
                myflags = IFI_ALIAS;
            }
            memcpy(lastname, ifname, IFNAMSIZ);
            ifi = (struct ifi_info*)calloc(1, sizeof(struct ifi_info));
            if (ifi == NULL) {
                goto gotError;
            }

            ifipold   = *ifipnext;       /* need this later */
            ifiptr    = ifipnext;
            *ifipnext = ifi;            /* prev points to this new one */
            ifipnext = &ifi->ifi_next;  /* pointer to next one goes here */

            sprintf(addr6, "%s:%s:%s:%s:%s:%s:%s:%s",
                    addr[0],addr[1],addr[2],addr[3],
                    addr[4],addr[5],addr[6],addr[7]);

            /* Add address of the interface */
            memset(&hints, 0, sizeof(hints));
            hints.ai_family = AF_INET6;
            hints.ai_flags = AI_NUMERICHOST;
            err = getaddrinfo(addr6, NULL, &hints, &res0);
            if (err) {
                goto gotError;
            }
            ifi->ifi_addr = calloc(1, sizeof(struct sockaddr_in6));
            if (ifi->ifi_addr == NULL) {
                goto gotError;
            }
            memcpy(ifi->ifi_addr, res0->ai_addr, sizeof(struct sockaddr_in6));

            /* Add netmask of the interface */
            char ipv6addr[INET6_ADDRSTRLEN];
            plen_to_mask(plen, ipv6addr);
            ifi->ifi_netmask = calloc(1, sizeof(struct sockaddr_in6));
            if (ifi->ifi_addr == NULL) {
                goto gotError;
            }
            sin6=calloc(1, sizeof(struct sockaddr_in6));
            addrptr=calloc(1, sizeof(struct in6_addr));
            inet_pton(family, ipv6addr, addrptr);
            sin6->sin6_family=family;
            sin6->sin6_addr=*addrptr;
            sin6->sin6_scope_id=scope;
            memcpy(ifi->ifi_netmask, sin6, sizeof(struct sockaddr_in6));
            free(sin6);


            /* Add interface name */
            memcpy(ifi->ifi_name, ifname, IFI_NAME);

            /* Add interface index */
            ifi->ifi_index = index;

            /* Add interface flags*/
            memcpy(ifr.ifr_name, ifname, IFNAMSIZ);
            if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0) {
                if (errno == EADDRNOTAVAIL) {
                    /*
                     * If the main interface is configured with no IP address but
                     * an alias interface exists with an IP address, you get
                     * EADDRNOTAVAIL for the main interface
                     */
                    free(ifi->ifi_addr);
                    free(ifi);
                    ifipnext  = ifiptr;
                    *ifipnext = ifipold;
                    continue;
                } else {
                    goto gotError;
                }
            }
            ifi->ifi_flags = ifr.ifr_flags;
            freeaddrinfo(res0);
            res0=NULL;
        }
    }
    goto done;

gotError:
    if (ifihead != NULL) {
        free_ifi_info(ifihead);
        ifihead = NULL;
    }
    if (res0 != NULL) {
        freeaddrinfo(res0);
        res0=NULL;
    }
done:
    if (sockfd != -1) {
        assert(close(sockfd) == 0);
    }
#endif
	// vchen FIXME : return uninitialized pointer?
    return(ifihead);    /* pointer to first structure in linked list */
}
#endif // defined(AF_INET6) && HAVE_IPV6 && HAVE_LINUX

struct ifi_info *get_ifi_info(int family, int doaliases)
{
    int junk;
    struct ifi_info     *ifi, *ifihead, **ifipnext, *ifipold, **ifiptr;
    ifihead = NULL;
#if 0
    int sockfd, sockf6, len, lastlen, flags, myflags;
#ifdef NOT_HAVE_IF_NAMETOINDEX
    int index = 200;
#endif
    char                *ptr, *buf, lastname[IFNAMSIZ], *cptr;
    struct ifconf ifc;
    struct ifreq        *ifr, ifrcopy;
    struct sockaddr_in  *sinptr;

#if defined(AF_INET6) && HAVE_IPV6
    struct sockaddr_in6 *sinptr6;
#endif

#if defined(AF_INET6) && HAVE_IPV6 && HAVE_LINUX
    if (family == AF_INET6) return get_ifi_info_linuxv6(family, doaliases);
#endif

    sockfd = -1;
    sockf6 = -1;
    buf = NULL;
    ifihead = NULL;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        goto gotError;
    }

    lastlen = 0;
    len = 100 * sizeof(struct ifreq);   /* initial buffer size guess */
    for ( ; ; ) {
        buf = (char*)malloc(len);
        if (buf == NULL) {
            goto gotError;
        }
        ifc.ifc_len = len;
        ifc.ifc_buf = buf;
        if (ioctl(sockfd, SIOCGIFCONF, &ifc) < 0) {
            if (errno != EINVAL || lastlen != 0) {
                goto gotError;
            }
        } else {
            if (ifc.ifc_len == lastlen)
                break;      /* success, len has not changed */
            lastlen = ifc.ifc_len;
        }
        len += 10 * sizeof(struct ifreq);   /* increment */
        free(buf);
    }
    ifihead = NULL;
    ifipnext = &ifihead;
    lastname[0] = 0;
/* end get_ifi_info1 */

/* include get_ifi_info2 */
    for (ptr = buf; ptr < buf + ifc.ifc_len; ) {
        ifr = (struct ifreq *) ptr;

        /* Advance to next one in buffer */
        if (sizeof(struct ifreq) > sizeof(ifr->ifr_name) + GET_SA_LEN(ifr->ifr_addr))
            ptr += sizeof(struct ifreq);
        else
            ptr += sizeof(ifr->ifr_name) + GET_SA_LEN(ifr->ifr_addr);

//      fprintf(stderr, "intf %p name=%s AF=%d\n", index, ifr->ifr_name, ifr->ifr_addr.sa_family);

        if (ifr->ifr_addr.sa_family != family)
            continue;   /* ignore if not desired address family */

        myflags = 0;
        if ( (cptr = strchr(ifr->ifr_name, ':')) != NULL)
            *cptr = 0;      /* replace colon will null */
        if (strncmp(lastname, ifr->ifr_name, IFNAMSIZ) == 0) {
            if (doaliases == 0)
                continue;   /* already processed this interface */
            myflags = IFI_ALIAS;
        }
        memcpy(lastname, ifr->ifr_name, IFNAMSIZ);

        ifrcopy = *ifr;
        if (ioctl(sockfd, SIOCGIFFLAGS, &ifrcopy) < 0) {
            goto gotError;
        }

        flags = ifrcopy.ifr_flags;
        if ((flags & IFF_UP) == 0)
            continue;   /* ignore if interface not up */

        ifi = (struct ifi_info*)calloc(1, sizeof(struct ifi_info));
        if (ifi == NULL) {
            goto gotError;
        }
        ifipold   = *ifipnext;       /* need this later */
        ifiptr    = ifipnext;
        *ifipnext = ifi;             /* prev points to this new one */
        ifipnext  = &ifi->ifi_next;  /* pointer to next one goes here */

        ifi->ifi_flags = flags;     /* IFF_xxx values */
        ifi->ifi_myflags = myflags; /* IFI_xxx values */
#ifndef NOT_HAVE_IF_NAMETOINDEX
        ifi->ifi_index = if_nametoindex(ifr->ifr_name);
#else
        ifrcopy = *ifr;
#ifdef SIOCGIFINDEX
        if ( 0 >= ioctl(sockfd, SIOCGIFINDEX, &ifrcopy))
            ifi->ifi_index = ifrcopy.ifr_index;
        else
#endif
        ifi->ifi_index = index++;       /* SIOCGIFINDEX is broken on Solaris 2.5ish, so fake it */
#endif
        memcpy(ifi->ifi_name, ifr->ifr_name, IFI_NAME);
        ifi->ifi_name[IFI_NAME-1] = '\0';
/* end get_ifi_info2 */
/* include get_ifi_info3 */
        switch (ifr->ifr_addr.sa_family) {
        case AF_INET:
            sinptr = (struct sockaddr_in *) &ifr->ifr_addr;
            if (ifi->ifi_addr == NULL) {
                ifi->ifi_addr = (struct sockaddr*)calloc(1, sizeof(struct sockaddr_in));
                if (ifi->ifi_addr == NULL) {
                    goto gotError;
                }
                memcpy(ifi->ifi_addr, sinptr, sizeof(struct sockaddr_in));

#ifdef  SIOCGIFNETMASK
                if (ioctl(sockfd, SIOCGIFNETMASK, &ifrcopy) < 0) {
                    if (errno == EADDRNOTAVAIL) {
                        /*
                         * If the main interface is configured with no IP address but
                         * an alias interface exists with an IP address, you get
                         * EADDRNOTAVAIL for the main interface
                         */
                        free(ifi->ifi_addr);
                        free(ifi);
                        ifipnext  = ifiptr;
                        *ifipnext = ifipold;
                        continue;
                    } else {
                        goto gotError;
                    }
                }

                ifi->ifi_netmask = (struct sockaddr*)calloc(1, sizeof(struct sockaddr_in));
                if (ifi->ifi_netmask == NULL) goto gotError;
                sinptr = (struct sockaddr_in *) &ifrcopy.ifr_addr;
                /* The BSD ioctls (including Mac OS X) stick some weird values in for sin_len and sin_family */
#ifndef NOT_HAVE_SA_LEN
                sinptr->sin_len    = sizeof(struct sockaddr_in);
#endif
                sinptr->sin_family = AF_INET;
                memcpy(ifi->ifi_netmask, sinptr, sizeof(struct sockaddr_in));
#endif

#ifdef  SIOCGIFBRDADDR
                if (flags & IFF_BROADCAST) {
                    if (ioctl(sockfd, SIOCGIFBRDADDR, &ifrcopy) < 0) {
                        goto gotError;
                    }
                    sinptr = (struct sockaddr_in *) &ifrcopy.ifr_broadaddr;
                    /* The BSD ioctls (including Mac OS X) stick some weird values in for sin_len and sin_family */
#ifndef NOT_HAVE_SA_LEN
                    sinptr->sin_len    = sizeof( struct sockaddr_in );
#endif
                    sinptr->sin_family = AF_INET;
                    ifi->ifi_brdaddr = (struct sockaddr*)calloc(1, sizeof(struct sockaddr_in));
                    if (ifi->ifi_brdaddr == NULL) {
                        goto gotError;
                    }
                    memcpy(ifi->ifi_brdaddr, sinptr, sizeof(struct sockaddr_in));
                }
#endif

#ifdef  SIOCGIFDSTADDR
                if (flags & IFF_POINTOPOINT) {
                    if (ioctl(sockfd, SIOCGIFDSTADDR, &ifrcopy) < 0) {
                        goto gotError;
                    }
                    sinptr = (struct sockaddr_in *) &ifrcopy.ifr_dstaddr;
                    /* The BSD ioctls (including Mac OS X) stick some weird values in for sin_len and sin_family */
#ifndef NOT_HAVE_SA_LEN
                    sinptr->sin_len    = sizeof( struct sockaddr_in );
#endif
                    sinptr->sin_family = AF_INET;
                    ifi->ifi_dstaddr = (struct sockaddr*)calloc(1, sizeof(struct sockaddr_in));
                    if (ifi->ifi_dstaddr == NULL) {
                        goto gotError;
                    }
                    memcpy(ifi->ifi_dstaddr, sinptr, sizeof(struct sockaddr_in));
                }
#endif
            }
            break;

#if defined(AF_INET6) && HAVE_IPV6
        case AF_INET6:
            sinptr6 = (struct sockaddr_in6 *) &ifr->ifr_addr;
            if (ifi->ifi_addr == NULL) {
                ifi->ifi_addr = calloc(1, sizeof(struct sockaddr_in6));
                if (ifi->ifi_addr == NULL) {
                    goto gotError;
                }

                /* Some platforms (*BSD) inject the prefix in IPv6LL addresses */
                /* We need to strip that out */
                if (IN6_IS_ADDR_LINKLOCAL(&sinptr6->sin6_addr))
                    sinptr6->sin6_addr.s6_addr[2] = sinptr6->sin6_addr.s6_addr[3] = 0;
                memcpy(ifi->ifi_addr, sinptr6, sizeof(struct sockaddr_in6));

#ifdef  SIOCGIFNETMASK_IN6
                {
                    struct in6_ifreq ifr6;
                    if (sockf6 == -1)
                        sockf6 = socket(AF_INET6, SOCK_DGRAM, 0);
                    memset(&ifr6, 0, sizeof(ifr6));
                    memcpy(&ifr6.ifr_name,           &ifr->ifr_name, sizeof(ifr6.ifr_name          ));
                    memcpy(&ifr6.ifr_ifru.ifru_addr, &ifr->ifr_addr, sizeof(ifr6.ifr_ifru.ifru_addr));
                    if (ioctl(sockf6, SIOCGIFNETMASK_IN6, &ifr6) < 0) {
                        if (errno == EADDRNOTAVAIL) {
                            /*
                             * If the main interface is configured with no IP address but
                             * an alias interface exists with an IP address, you get
                             * EADDRNOTAVAIL for the main interface
                             */
                            free(ifi->ifi_addr);
                            free(ifi);
                            ifipnext  = ifiptr;
                            *ifipnext = ifipold;
                            continue;
                        } else {
                            goto gotError;
                        }
                    }
                    ifi->ifi_netmask = (struct sockaddr*)calloc(1, sizeof(struct sockaddr_in6));
                    if (ifi->ifi_netmask == NULL) goto gotError;
                    sinptr6 = (struct sockaddr_in6 *) &ifr6.ifr_ifru.ifru_addr;
                    memcpy(ifi->ifi_netmask, sinptr6, sizeof(struct sockaddr_in6));
                }
#endif
            }
            break;
#endif

        default:
            break;
        }
    }
    goto done;

gotError:
    if (ifihead != NULL) {
        free_ifi_info(ifihead);
        ifihead = NULL;
    }

done:
    if (buf != NULL) {
        free(buf);
    }
    if (sockfd != -1) {
        junk = close(sockfd);
        assert(junk == 0);
    }
    if (sockf6 != -1) {
        junk = close(sockf6);
        assert(junk == 0);
    }
#endif
	// vchen FIXME : return uninitialized pointer?
    return(ifihead);    /* pointer to first structure in linked list */
}
/* end get_ifi_info3 */
#if 0
/* include free_ifi_info */
void
free_ifi_info(struct ifi_info *ifihead)
{
    struct ifi_info *ifi, *ifinext;

    for (ifi = ifihead; ifi != NULL; ifi = ifinext) {
        if (ifi->ifi_addr != NULL)
            free(ifi->ifi_addr);
        if (ifi->ifi_netmask != NULL)
            free(ifi->ifi_netmask);
        if (ifi->ifi_brdaddr != NULL)
            free(ifi->ifi_brdaddr);
        if (ifi->ifi_dstaddr != NULL)
            free(ifi->ifi_dstaddr);
        ifinext = ifi->ifi_next;    /* can't fetch ifi_next after free() */
        free(ifi);                  /* the ifi_info{} itself */
    }
}
/* end free_ifi_info */
#endif 
ssize_t
recvfrom_flags(int fd, void *ptr, size_t nbytes, int *flagsp,
               struct sockaddr *sa, socklen_t *salenptr, struct my_in_pktinfo *pktp, u_char *ttl, int isIPv6)
{
//    struct msghdr msg;
//    struct iovec iov[1];
    ssize_t n;

if(!isIPv6)/* lwIP */
{
    //lwip_recvfrom(int s, void *mem, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen)
    struct sockaddr dest = {0};
    socklen_t destlen = sizeof(struct sockaddr);

    if ((n = lwip_recvfrom_ext(fd, ptr, nbytes, *flagsp, sa, salenptr, &dest, &destlen)) < 0 )
        return n;
    if (IN_MULTICAST(((struct sockaddr_in *)&dest)->sin_addr.s_addr))
        *flagsp |= MSG_MCAST;
    /* pktp->ipi_addr = destination IP address of this incoming packet */
    struct sockaddr_in *sin = (struct sockaddr_in*)&pktp->ipi_addr;  
    sin->sin_family = AF_INET; 
    sin->sin_addr = ((struct sockaddr_in *)&dest)->sin_addr;
    sin->sin_port = 0; 
    // pktp->ipi_addr = ((struct sockaddr_in *)&dest)->sin_addr;
    pktp->ipi_ifindex = 0; /* there's only one interface in 7687 */
    memset(pktp->ipi_ifname, 0, sizeof(pktp->ipi_ifname)); /* ?? */
    *ttl = 255; /* caller ignore this return value */
    return n;
}
else /* lwIP */
{
    //lwip_recvfrom(int s, void *mem, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen)
    struct sockaddr_in6 dest = {0};
    struct ip6_addr targetaddr6;
    socklen_t destlen = sizeof(struct sockaddr_in6);

    if ((n = lwip_recvfrom_ext(fd, ptr, nbytes, *flagsp, sa, salenptr, (struct sockaddr*)&dest, &destlen)) < 0 )
        return n;
    inet6_addr_to_ip6addr(&targetaddr6, &((&dest)->sin6_addr));
    if (ip6_addr_ismulticast(&targetaddr6))
    {
        *flagsp |= MSG_MCAST;
    }
    /* pktp->ipi_addr = destination IP address of this incoming packet */
    struct sockaddr_in6 *sin6 = (struct sockaddr_in6*)&pktp->ipi_addr;
    sin6->sin6_family = AF_INET6;
    sin6->sin6_addr = (&dest)->sin6_addr;
    sin6->sin6_port = 0;
    // pktp->ipi_addr = ((struct sockaddr_in *)&dest)->sin_addr;
    pktp->ipi_ifindex = 0; /* there's only one interface in 7687 */
    memset(pktp->ipi_ifname, 0, sizeof(pktp->ipi_ifname)); /* ?? */
    *ttl = 255; /* caller ignore this return value */
    return n;
}
}

// **********************************************************************************************

// daemonize the process. Adapted from "Unix Network Programming" vol 1 by Stevens, section 12.4.
// Returns 0 on success, -1 on failure.

#if 0
#ifdef NOT_HAVE_DAEMON
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/signal.h>

int daemon(int nochdir, int noclose)
{
    switch (fork())
    {
    case -1: return (-1);       // Fork failed
    case 0:  break;             // Child -- continue
    default: _exit(0);          // Parent -- exit
    }

    if (setsid() == -1) return(-1);

    signal(SIGHUP, SIG_IGN);

    switch (fork())             // Fork again, primarily for reasons of Unix trivia
    {
    case -1: return (-1);       // Fork failed
    case 0:  break;             // Child -- continue
    default: _exit(0);          // Parent -- exit
    }

    if (!nochdir) (void)chdir("/");
    umask(0);

    if (!noclose)
    {
        int fd = open("/dev/null", O_RDWR, 0);
        if (fd != -1)
        {
            // Avoid unnecessarily duplicating a file descriptor to itself
            if (fd != STDIN_FILENO) (void)dup2(fd, STDIN_FILENO);
            if (fd != STDOUT_FILENO) (void)dup2(fd, STDOUT_FILENO);
            if (fd != STDERR_FILENO) (void)dup2(fd, STDERR_FILENO);
            if (fd != STDIN_FILENO && fd != STDOUT_FILENO && fd != STDERR_FILENO)
                (void)close (fd);
        }
    }
    return (0);
}
#endif /* NOT_HAVE_DAEMON */
#endif
