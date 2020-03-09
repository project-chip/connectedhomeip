/*
 *
 *    Copyright (c) 2014-2017 Nest Labs, Inc.
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 * @file
 *  This header file defines the <tt>nl::Inet::TunEndPoint</tt> class, where
 *  the Nest Inet Layer encapsulates its foundational logic for the Weave
 *  tunneling agent.
 */

#ifndef TUNENDPOINT_H
#define TUNENDPOINT_H

#include <InetLayer/EndPointBasis.h>
#include <InetLayer/IPAddress.h>
#include <InetLayer/IPPrefix.h>

#include <SystemLayer/SystemPacketBuffer.h>

#if WEAVE_SYSTEM_CONFIG_USE_LWIP
#include "lwip/netif.h"
#include <lwip/ip_addr.h>
#include <lwip/ip.h>
#include <lwip/tcpip.h>
#include <lwip/ip6.h>
#include <lwip/ip6_route_table.h>
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP

#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS
#include <net/if.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/ip6.h>
#include <netinet/in.h>
#include <net/route.h>

#if HAVE_LINUX_IF_TUN_H
#include <linux/if_tun.h>
#endif // HAVE_LINUX_IF_TUN_H

#if HAVE_LINUX_ROUTE_H
#include <linux/route.h>
#endif // HAVE_LINUX_ROUTE_H

#if HAVE_LINUX_IPV6_ROUTE_H
#include <linux/ipv6_route.h>
#endif // HAVE_LINUX_IPV6_ROUTE_H
#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS

/** Define portable flags for closing file descriptor upon
 * inheritance by an external sub-process. This is only a
 * safety guard. Code that spawns a child process must make
 * sure that any unneeded inherited file descriptor is closed.
 */
#ifdef O_CLOEXEC
#define NL_O_CLOEXEC O_CLOEXEC
#else
#define NL_O_CLOEXEC 0
#endif

#ifdef SOCK_CLOEXEC
#define NL_SOCK_CLOEXEC SOCK_CLOEXEC
#else
#define NL_SOCK_CLOEXEC 0
#endif

namespace nl {
namespace Inet {

class InetLayer;
class IPPacketInfo;

/**
 * @brief   Objects of this class represent tunnel interfaces.
 *
 * @details
 *  Nest Inet Layer encapsulates methods for interacting on both POSIX systems
 *  and LwIP systems with an IP-in-IP tunneling mechanism for supporting the
 *  Weave tunnel agent.
 */
class NL_DLL_EXPORT TunEndPoint: public EndPointBasis
{
    friend class InetLayer;

public:

    /**
     * @brief   Basic dynamic state of the underlying tunnel.
     *
     * @details
     *  Objects are initialized in the "open" state, proceed to the "closed"
     *  state when they are ready to be recycled.
     */
    enum
    {
        kState_Open                   = 0,
        kState_Closed                 = 1
    } mState;

    /**
     * @brief   Operation when setting route to tunnel interface.
     *
     * @details
     *  Values of this enumerated type are used when setting a route for the
     *  encapsulated tunnel.
     */
    typedef enum RouteOp
    {
        kRouteTunIntf_Add             = 0,  /**< Add route for a prefix. */
        kRouteTunIntf_Del             = 1   /**< Remove route for a prefix. */
    } RouteOp;

    /** Pointer to application-specific state object. */
    void *mAppState;

    void Init(InetLayer *inetLayer);

#if WEAVE_SYSTEM_CONFIG_USE_LWIP
    INET_ERROR Open(void);
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP

#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS
    INET_ERROR Open(const char *intfName);
#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS

    /** Close the tunnel and release handle on the object. */
    void Free(void);

    INET_ERROR Send(Weave::System::PacketBuffer *message);

    bool IsInterfaceUp(void) const;

    INET_ERROR InterfaceUp(void);

    INET_ERROR InterfaceDown(void);

    /**
     * @brief   Type of packet receive event handler.
     *
     * @details
     *  Type of delegate to a higher layer to act upon receipt of an IPv6
     *  packet from the tunnel.
     *
     * @param[in] endPoint        A pointer to the TunEndPoint object.
     * @param[in] message         A pointer to the Weave::System::PacketBuffer message object.
     */
    typedef void (*OnPacketReceivedFunct)(TunEndPoint *endPoint, Weave::System::PacketBuffer *message);

    /** The endpoint's packet receive event handler delegate. */
    OnPacketReceivedFunct OnPacketReceived;

    /**
     * @brief   Type of error event handler.
     *
     * @details
     *  Type of delegate to a higher layer to act upon error processing an IPv6
     *  packet from the tunnel.
     *
     * @param[in] endPoint      The TunEndPoint object.
     * @param[in] err           Error code reported.
     */
    typedef void (*OnReceiveErrorFunct)(TunEndPoint *endPoint, INET_ERROR err);
    OnReceiveErrorFunct OnReceiveError;

    InterfaceId GetTunnelInterfaceId(void);

private:

    TunEndPoint(void);                                  // not defined
    TunEndPoint(const TunEndPoint&);                    // not defined
    ~TunEndPoint(void);                                 // not defined

    static Weave::System::ObjectPool<TunEndPoint, INET_CONFIG_NUM_TUN_ENDPOINTS> sPool;

    /** Close the tunnel. */
    void Close(void);

    // Function that performs some sanity tests for IPv6 packets.
    INET_ERROR CheckV6Sanity(Weave::System::PacketBuffer *message);
    // Function for sending the IPv6 packets over Linux sockets or LwIP.
    INET_ERROR TunDevSendMessage(Weave::System::PacketBuffer *msg);

#if WEAVE_SYSTEM_CONFIG_USE_LWIP
    // Network interface structure holding the tunnel interface in LwIP.
    struct netif mTunNetIf;

    INET_ERROR TunDevOpen(void);
    void HandleDataReceived(Weave::System::PacketBuffer *msg);

    static err_t LwIPPostToInetEventQ(struct netif *netif, struct pbuf *p);
#if LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
#if LWIP_IPV4
    static err_t LwIPOutputIPv4(struct netif *netif, struct pbuf *p, const ip4_addr_t *addr);
#endif // LWIP_IPV4
#if LWIP_IPV6
    static err_t LwIPOutputIPv6(struct netif *netif, struct pbuf *p, const ip6_addr_t *addr);
#endif // LWIP_IPV6
#else // LWIP_VERSION_MAJOR <= 1 || LWIP_VERSION_MINOR < 5
    static err_t LwIPReceiveTunMessage(struct netif *netif, struct pbuf *p, ip4_addr_t *addr);
#if LWIP_IPV6
    static err_t LwIPReceiveTunV6Message(struct netif *netif, struct pbuf *p, ip6_addr_t *addr);
#endif // LWIP_IPV6
#endif // LWIP_VERSION_MAJOR <= 1 || LWIP_VERSION_MINOR < 5
    static err_t TunInterfaceNetifInit(struct netif *netif);
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP

#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS
    //Tunnel interface name
    char tunIntfName[IFNAMSIZ];

    INET_ERROR TunDevOpen(const char *interfaceName);
    void TunDevClose(void);
    INET_ERROR TunDevRead(Weave::System::PacketBuffer *msg);
    static int TunGetInterface(int fd, struct ::ifreq *ifr);

    SocketEvents PrepareIO(void);
    void HandlePendingIO(void);
#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS

};

} // namespace Inet
} // namespace nl

#endif // !defined(TUNENDPOINT_H)
