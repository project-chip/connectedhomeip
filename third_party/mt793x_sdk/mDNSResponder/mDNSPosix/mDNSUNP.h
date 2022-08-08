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

#ifndef __mDNSUNP_h
#define __mDNSUNP_h

#include "mdns_porting.h"
//#include <sys/socket.h>
#include <lwip/sockets.h>
//#include <net/if.h>
//#include <netinet/in.h>

// Added
#define MSG_MCAST 0x200

#ifdef HAVE_LINUX
#include <linux/socket.h>
#define IPV6_2292_PKTINFO IPV6_2292PKTINFO
#define IPV6_2292_HOPLIMIT IPV6_2292HOPLIMIT
#else
// The following are the supported non-linux posix OSes -
// netbsd, freebsd and openbsd.
#if HAVE_IPV6
#define IPV6_2292_PKTINFO 19
#define IPV6_2292_HOPLIMIT 20
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef NOT_HAVE_SOCKLEN_T
typedef unsigned int socklen_t;
#endif

#if !defined(_SS_MAXSIZE)
#if HAVE_IPV6
#define sockaddr_storage sockaddr_in6
#else
#define sockaddr_storage sockaddr
#endif // HAVE_IPV6
#endif // !defined(_SS_MAXSIZE)

#ifndef NOT_HAVE_SA_LEN
#define GET_SA_LEN(X)                                                                                                              \
    (sizeof(struct sockaddr) > ((struct sockaddr *) &(X))->sa_len ? sizeof(struct sockaddr) : ((struct sockaddr *) &(X))->sa_len)
#elif HAVE_IPV6
#define GET_SA_LEN(X)                                                                                                              \
    (((struct sockaddr *) &(X))->sa_family == AF_INET        ? sizeof(struct sockaddr_in)                                          \
         : ((struct sockaddr *) &(X))->sa_family == AF_INET6 ? sizeof(struct sockaddr_in6)                                         \
                                                             : sizeof(struct sockaddr))
#else
#define GET_SA_LEN(X) (((struct sockaddr *) &(X))->sa_family == AF_INET ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr))
#endif

#define IFI_NAME 16 /* same as IFNAMSIZ in <net/if.h> */
#define IFI_HADDR 8 /* allow for 64-bit EUI-64 in future */

// Renamed from my_in_pktinfo because in_pktinfo is used by Linux.

struct my_in_pktinfo
{
    struct sockaddr_storage ipi_addr;
    int ipi_ifindex;           /* received interface index */
    char ipi_ifname[IFI_NAME]; /* received interface name  */
};

/* From the text (Stevens, section 20.2): */
/* 'As an example of recvmsg we will write a function named recvfrom_flags that */
/* is similar to recvfrom but also returns: */
/*	1. the returned msg_flags value, */
/*	2. the destination addres of the received datagram (from the IP_RECVDSTADDR socket option, and */
/*	3. the index of the interface on which the datagram was received (the IP_RECVIF socket option).' */
extern ssize_t recvfrom_flags(int fd, void * ptr, size_t nbytes, int * flagsp, struct sockaddr * sa, socklen_t * salenptr,
                              struct my_in_pktinfo * pktp, u_char * ttl, int isIPv6);

struct ifi_info
{
    char ifi_name[IFI_NAME];     /* interface name, null terminated */
    u_char ifi_haddr[IFI_HADDR]; /* hardware address */
    u_short ifi_hlen;            /* #bytes in hardware address: 0, 6, 8 */
    short ifi_flags;             /* IFF_xxx constants from <net/if.h> */
    short ifi_myflags;           /* our own IFI_xxx flags */
    int ifi_index;               /* interface index */
    struct sockaddr * ifi_addr;  /* primary address */
    struct sockaddr * ifi_netmask;
    struct sockaddr * ifi_brdaddr; /* broadcast address */
    struct sockaddr * ifi_dstaddr; /* destination address */
    struct ifi_info * ifi_next;    /* next of these structures */
};

#if defined(AF_INET6) && HAVE_IPV6 && HAVE_LINUX
#define PROC_IFINET6_PATH "/proc/net/if_inet6"
extern struct ifi_info * get_ifi_info_linuxv6(int family, int doaliases);
#endif

#if defined(AF_INET6) && HAVE_IPV6
#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN 46 /*Maximum length of IPv6 address */
#endif
#endif

#define IFI_ALIAS 1 /* ifi_addr is an alias */

/* From the text (Stevens, section 16.6): */
/* 'Since many programs need to know all the interfaces on a system, we will develop a */
/* function of our own named get_ifi_info that returns a linked list of structures, one */
/* for each interface that is currently "up."' */
extern struct ifi_info * get_ifi_info(int family, int doaliases);

/* 'The free_ifi_info function, which takes a pointer that was */
/* returned by get_ifi_info and frees all the dynamic memory.' */
extern void free_ifi_info(struct ifi_info *);

#ifdef NOT_HAVE_DAEMON
extern int daemon(int nochdir, int noclose);
#endif

#ifdef __cplusplus
}
#endif

#endif
