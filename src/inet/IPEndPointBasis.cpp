/*
 *
 *    Copyright (c) 2018 Google LLC.
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
 *    @file
 *      This header file implements the <tt>nl::Inet::IPEndPointBasis</tt>
 *      class, an intermediate, non-instantiable basis class
 *      supporting other IP-based end points.
 *
 */

// define to ensure we have the IPV6_PKTINFO
#define __APPLE_USE_RFC_3542

#include <InetLayer/IPEndPointBasis.h>

#include <string.h>

#include <InetLayer/EndPointBasis.h>
#include <InetLayer/InetInterface.h>
#include <InetLayer/InetLayer.h>

#include <Weave/Support/CodeUtils.h>

#if WEAVE_SYSTEM_CONFIG_USE_LWIP
#if INET_CONFIG_ENABLE_IPV4
#include <lwip/igmp.h>
#endif // INET_CONFIG_ENABLE_IPV4
#include <lwip/init.h>
#include <lwip/ip.h>
#include <lwip/mld6.h>
#include <lwip/netif.h>
#include <lwip/raw.h>
#include <lwip/udp.h>
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP

#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/ioctl.h>
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif // HAVE_SYS_SOCKET_H

/*
 * Some systems define both IPV6_{ADD,DROP}_MEMBERSHIP and
 * IPV6_{JOIN,LEAVE}_GROUP while others only define
 * IPV6_{JOIN,LEAVE}_GROUP. Prefer the "_MEMBERSHIP" flavor for
 * parallelism with IPv4 and create the alias to the availabile
 * definitions.
 */
#if !defined(IPV6_ADD_MEMBERSHIP) && defined(IPV6_JOIN_GROUP)
#define INET_IPV6_ADD_MEMBERSHIP IPV6_JOIN_GROUP
#elif defined(IPV6_ADD_MEMBERSHIP)
#define INET_IPV6_ADD_MEMBERSHIP IPV6_ADD_MEMBERSHIP
#else
#error "Neither IPV6_ADD_MEMBERSHIP nor IPV6_JOIN_GROUP are defined which are required for generalized IPv6 multicast group support."
#endif // !defined(IPV6_ADD_MEMBERSHIP) && defined(IPV6_JOIN_GROUP)

#if !defined(IPV6_DROP_MEMBERSHIP) && defined(IPV6_LEAVE_GROUP)
#define INET_IPV6_DROP_MEMBERSHIP IPV6_LEAVE_GROUP
#elif defined(IPV6_DROP_MEMBERSHIP)
#define INET_IPV6_DROP_MEMBERSHIP IPV6_DROP_MEMBERSHIP
#else
#error "Neither IPV6_DROP_MEMBERSHIP nor IPV6_LEAVE_GROUP are defined which are required for generalized IPv6 multicast group support."
#endif // !defined(IPV6_DROP_MEMBERSHIP) && defined(IPV6_LEAVE_GROUP)
#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS

namespace nl {
namespace Inet {

using Weave::System::PacketBuffer;

#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS
union PeerSockAddr
{
    sockaddr     any;
    sockaddr_in  in;
    sockaddr_in6 in6;
};
#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS

#if WEAVE_SYSTEM_CONFIG_USE_LWIP
#if INET_CONFIG_ENABLE_IPV4
#define LWIP_IPV4_ADDR_T           ip4_addr_t
#define IPV4_TO_LWIPADDR(aAddress) (aAddress).ToIPv4()
#endif // INET_CONFIG_ENABLE_IPV4
#define LWIP_IPV6_ADDR_T           ip6_addr_t
#define IPV6_TO_LWIPADDR(aAddress) (aAddress).ToIPv6()

#if !defined(RAW_FLAGS_MULTICAST_LOOP) || !defined(UDP_FLAGS_MULTICAST_LOOP) || !defined(raw_clear_flags) || !defined(raw_set_flags) || !defined(udp_clear_flags) || !defined(udp_set_flags)
#define HAVE_LWIP_MULTICAST_LOOP 0
#else
#define HAVE_LWIP_MULTICAST_LOOP 1
#endif // !defined(RAW_FLAGS_MULTICAST_LOOP) || !defined(UDP_FLAGS_MULTICAST_LOOP) || !defined(raw_clear_flags) || !defined(raw_set_flags) || !defined(udp_clear_flags) || !defined(udp_set_flags)
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP

static INET_ERROR CheckMulticastGroupArgs(InterfaceId aInterfaceId, const IPAddress &aAddress)
{
    INET_ERROR          lRetval = INET_NO_ERROR;
    bool                lIsPresent, lIsMulticast;

    lIsPresent = IsInterfaceIdPresent(aInterfaceId);
    VerifyOrExit(lIsPresent, lRetval = INET_ERROR_UNKNOWN_INTERFACE);

    lIsMulticast = aAddress.IsMulticast();
    VerifyOrExit(lIsMulticast, lRetval = INET_ERROR_WRONG_ADDRESS_TYPE);

exit:
    return (lRetval);
}

#if WEAVE_SYSTEM_CONFIG_USE_LWIP
#if INET_CONFIG_ENABLE_IPV4
#if LWIP_IPV4 && LWIP_IGMP
static INET_ERROR LwIPIPv4JoinLeaveMulticastGroup(InterfaceId aInterfaceId, const IPAddress &aAddress, err_t (*aMethod)(struct netif *, const LWIP_IPV4_ADDR_T *))
{
    INET_ERROR        lRetval = INET_NO_ERROR;
    err_t             lStatus;
    struct netif *    lNetif;
    LWIP_IPV4_ADDR_T  lIPv4Address;

    lNetif = IPEndPointBasis::FindNetifFromInterfaceId(aInterfaceId);
    VerifyOrExit(lNetif != NULL, lRetval = INET_ERROR_UNKNOWN_INTERFACE);

    lIPv4Address = IPV4_TO_LWIPADDR(aAddress);

    lStatus = aMethod(lNetif, &lIPv4Address);

    switch (lStatus)
    {

    case ERR_MEM:
        lRetval = INET_ERROR_NO_MEMORY;
        break;

    default:
        lRetval = Weave::System::MapErrorLwIP(lStatus);
        break;

    }

exit:
    return (lRetval);
}
#endif // LWIP_IPV4 && LWIP_IGMP
#endif // INET_CONFIG_ENABLE_IPV4

#if LWIP_IPV6_MLD && LWIP_IPV6_ND && LWIP_IPV6
static INET_ERROR LwIPIPv6JoinLeaveMulticastGroup(InterfaceId aInterfaceId, const IPAddress &aAddress, err_t (*aMethod)(struct netif *, const LWIP_IPV6_ADDR_T *))
{
    INET_ERROR        lRetval = INET_NO_ERROR;
    err_t             lStatus;
    struct netif *    lNetif;
    LWIP_IPV6_ADDR_T  lIPv6Address;

    lNetif = IPEndPointBasis::FindNetifFromInterfaceId(aInterfaceId);
    VerifyOrExit(lNetif != NULL, lRetval = INET_ERROR_UNKNOWN_INTERFACE);

    lIPv6Address = IPV6_TO_LWIPADDR(aAddress);

    lStatus = aMethod(lNetif, &lIPv6Address);

    switch (lStatus)
    {

    case ERR_MEM:
        lRetval = INET_ERROR_NO_MEMORY;
        break;

    default:
        lRetval = Weave::System::MapErrorLwIP(lStatus);
        break;

    }

exit:
    return (lRetval);
}
#endif // LWIP_IPV6_MLD && LWIP_IPV6_ND && LWIP_IPV6
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP

#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS
static INET_ERROR SocketsSetMulticastLoopback(int aSocket, bool aLoopback, int aProtocol, int aOption)
{
    INET_ERROR    lRetval = INET_NO_ERROR;
    int           lStatus;
    unsigned int  lValue = aLoopback;

    lStatus = setsockopt(aSocket, aProtocol, aOption, &lValue, sizeof (lValue));
    VerifyOrExit(lStatus == 0, lRetval = Weave::System::MapErrorPOSIX(errno));

exit:
    return (lRetval);
}

static INET_ERROR SocketsSetMulticastLoopback(int aSocket, IPVersion aIPVersion, bool aLoopback)
{
    INET_ERROR  lRetval;

    switch (aIPVersion)
    {

    case kIPVersion_6:
        lRetval = SocketsSetMulticastLoopback(aSocket, aLoopback, IPPROTO_IPV6, IPV6_MULTICAST_LOOP);
        break;

#if INET_CONFIG_ENABLE_IPV4
    case kIPVersion_4:
        lRetval = SocketsSetMulticastLoopback(aSocket, aLoopback, IPPROTO_IP, IP_MULTICAST_LOOP);
        break;
#endif // INET_CONFIG_ENABLE_IPV4

    default:
        lRetval = INET_ERROR_WRONG_ADDRESS_TYPE;
        break;

    }

    return (lRetval);
}

#if INET_CONFIG_ENABLE_IPV4
static INET_ERROR SocketsIPv4JoinLeaveMulticastGroup(int aSocket, InterfaceId aInterfaceId, const IPAddress &aAddress, int aCommand)
{
    INET_ERROR       lRetval = INET_NO_ERROR;
    IPAddress        lInterfaceAddress;
    bool             lInterfaceAddressFound = false;
    struct ip_mreq   lMulticastRequest;

    for (InterfaceAddressIterator lAddressIterator; lAddressIterator.HasCurrent(); lAddressIterator.Next())
    {
        const IPAddress lCurrentAddress = lAddressIterator.GetAddress();

        if (lAddressIterator.GetInterface() == aInterfaceId)
        {
            if (lCurrentAddress.IsIPv4())
            {
                lInterfaceAddressFound = true;
                lInterfaceAddress = lCurrentAddress;
                break;
            }
        }
    }

    VerifyOrExit(lInterfaceAddressFound, lRetval = INET_ERROR_ADDRESS_NOT_FOUND);

    memset(&lMulticastRequest, 0, sizeof (lMulticastRequest));
    lMulticastRequest.imr_interface = lInterfaceAddress.ToIPv4();
    lMulticastRequest.imr_multiaddr = aAddress.ToIPv4();

    lRetval = setsockopt(aSocket, IPPROTO_IP, aCommand, &lMulticastRequest, sizeof (lMulticastRequest));
    VerifyOrExit(lRetval == 0, lRetval = Weave::System::MapErrorPOSIX(errno));

exit:
    return (lRetval);
}
#endif // INET_CONFIG_ENABLE_IPV4

static INET_ERROR SocketsIPv6JoinLeaveMulticastGroup(int aSocket, InterfaceId aInterfaceId, const IPAddress &aAddress, int aCommand)
{
    INET_ERROR       lRetval = INET_NO_ERROR;
    const int        lIfIndex = static_cast<int>(aInterfaceId);
    struct ipv6_mreq lMulticastRequest;

    memset(&lMulticastRequest, 0, sizeof (lMulticastRequest));
    lMulticastRequest.ipv6mr_interface = lIfIndex;
    lMulticastRequest.ipv6mr_multiaddr = aAddress.ToIPv6();

    lRetval = setsockopt(aSocket, IPPROTO_IPV6, aCommand, &lMulticastRequest, sizeof (lMulticastRequest));
    VerifyOrExit(lRetval == 0, lRetval = Weave::System::MapErrorPOSIX(errno));

exit:
    return (lRetval);
}
#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS

/**
 *  @brief Set whether IP multicast traffic should be looped back.
 *
 *  @param[in]   aIPVersion
 *
 *  @param[in]   aLoop
 *
 *  @retval  INET_NO_ERROR
 *       success: multicast loopback behavior set
 *  @retval  other
 *       another system or platform error
 *
 *  @details
 *     Set whether or not IP multicast traffic should be looped back
 *     to this endpoint.
 *
 */
INET_ERROR IPEndPointBasis::SetMulticastLoopback(IPVersion aIPVersion, bool aLoopback)
{
    INET_ERROR          lRetval = INET_ERROR_NOT_IMPLEMENTED;

#if WEAVE_SYSTEM_CONFIG_USE_LWIP || WEAVE_SYSTEM_CONFIG_USE_SOCKETS
#if WEAVE_SYSTEM_CONFIG_USE_LWIP
#if !HAVE_LWIP_MULTICAST_LOOP
#pragma message "\n \
The version of LwIP appears older than that required for multicast loopback support.\n \
Please upgrade your version of LwIP for SetMulticastLoopback support."
    lRetval = INET_ERROR_NOT_SUPPORTED;
#else
    if (aLoopback)
    {
        switch (mLwIPEndPointType)
        {

#if INET_CONFIG_ENABLE_RAW_ENDPOINT
        case kLwIPEndPointType_Raw:
            raw_set_flags(mRaw, RAW_FLAGS_MULTICAST_LOOP);
            break;
#endif // INET_CONFIG_ENABLE_RAW_ENDPOINT

#if INET_CONFIG_ENABLE_UDP_ENDPOINT
        case kLwIPEndPointType_UDP:
            udp_set_flags(mUDP, UDP_FLAGS_MULTICAST_LOOP);
            break;
#endif // INET_CONFIG_ENABLE_UDP_ENDPOINT

        default:
            lRetval = INET_ERROR_NOT_SUPPORTED;
            break;

        }
    }
    else
    {
        switch (mLwIPEndPointType)
        {

#if INET_CONFIG_ENABLE_RAW_ENDPOINT
        case kLwIPEndPointType_Raw:
            raw_clear_flags(mRaw, RAW_FLAGS_MULTICAST_LOOP);
            break;
#endif // INET_CONFIG_ENABLE_RAW_ENDPOINT

#if INET_CONFIG_ENABLE_UDP_ENDPOINT
        case kLwIPEndPointType_UDP:
            udp_clear_flags(mUDP, UDP_FLAGS_MULTICAST_LOOP);
            break;
#endif // INET_CONFIG_ENABLE_UDP_ENDPOINT

        default:
            lRetval = INET_ERROR_NOT_SUPPORTED;
            break;

        }
    }

    lRetval = INET_NO_ERROR;
#endif // !HAVE_LWIP_MULTICAST_LOOP
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP

#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS
    lRetval = SocketsSetMulticastLoopback(mSocket, aIPVersion, aLoopback);
    SuccessOrExit(lRetval);

exit:
#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP || WEAVE_SYSTEM_CONFIG_USE_SOCKETS
    return (lRetval);
}

/**
 *  @brief Join an IP multicast group.
 *
 *  @param[in]   aInterfaceId  the indicator of the network interface to
 *                             add to the multicast group
 *
 *  @param[in]   aAddress      the multicast group to add the
 *                             interface to
 *
 *  @retval  INET_NO_ERROR
 *       success: multicast group removed
 *
 *  @retval  INET_ERROR_UNKNOWN_INTERFACE
 *       unknown network interface, \c aInterfaceId
 *
 *  @retval  INET_ERROR_WRONG_ADDRESS_TYPE
 *       \c aAddress is not \c kIPAddressType_IPv4 or
 *       \c kIPAddressType_IPv6 or is not multicast
 *
 *  @retval  other
 *       another system or platform error
 *
 *  @details
 *     Join the endpoint to the supplied multicast group on the
 *     specified interface.
 *
 */
INET_ERROR IPEndPointBasis::JoinMulticastGroup(InterfaceId aInterfaceId, const IPAddress &aAddress)
{
    const IPAddressType lAddrType = aAddress.Type();
    INET_ERROR          lRetval = INET_ERROR_NOT_IMPLEMENTED;

#if WEAVE_SYSTEM_CONFIG_USE_LWIP || WEAVE_SYSTEM_CONFIG_USE_SOCKETS
    lRetval = CheckMulticastGroupArgs(aInterfaceId, aAddress);
    SuccessOrExit(lRetval);

    switch (lAddrType)
    {

#if INET_CONFIG_ENABLE_IPV4
    case kIPAddressType_IPv4:
        {
#if WEAVE_SYSTEM_CONFIG_USE_LWIP
#if LWIP_IPV4 && LWIP_IGMP
            lRetval = LwIPIPv4JoinLeaveMulticastGroup(aInterfaceId, aAddress, igmp_joingroup_netif);
#else // LWIP_IPV4 && LWIP_IGMP
#pragma message "\n \
Please enable LWIP_IPV4 and LWIP_IGMP for IPv4 JoinMulticastGroup and LeaveMulticastGroup support."
            lRetval = INET_ERROR_NOT_SUPPORTED;
#endif // LWIP_IPV4 && LWIP_IGMP
            SuccessOrExit(lRetval);
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP

#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS
            lRetval = SocketsIPv4JoinLeaveMulticastGroup(mSocket, aInterfaceId, aAddress, IP_ADD_MEMBERSHIP);
            SuccessOrExit(lRetval);
#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS
        }
        break;
#endif // INET_CONFIG_ENABLE_IPV4

    case kIPAddressType_IPv6:
        {
#if WEAVE_SYSTEM_CONFIG_USE_LWIP
#if LWIP_IPV6_MLD && LWIP_IPV6_ND && LWIP_IPV6
            lRetval = LwIPIPv6JoinLeaveMulticastGroup(aInterfaceId, aAddress, mld6_joingroup_netif);
#else // LWIP_IPV6_MLD && LWIP_IPV6_ND && LWIP_IPV6
#pragma message "\n \
Please enable LWIP_IPV6_MLD && LWIP_IPV6_ND && LWIP_IPV6 for IPv6 JoinMulticastGroup and LeaveMulticastGroup support."
            lRetval = INET_ERROR_NOT_SUPPORTED;
#endif // LWIP_IPV6_MLD && LWIP_IPV6_ND && LWIP_IPV6
            SuccessOrExit(lRetval);
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP

#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS
            lRetval = SocketsIPv6JoinLeaveMulticastGroup(mSocket, aInterfaceId, aAddress, INET_IPV6_ADD_MEMBERSHIP);
            SuccessOrExit(lRetval);
#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS
        }
        break;

    default:
        lRetval = INET_ERROR_WRONG_ADDRESS_TYPE;
        break;
    }

exit:
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP || WEAVE_SYSTEM_CONFIG_USE_SOCKETS
    return (lRetval);
}

/**
 *  @brief Leave an IP multicast group.
 *
 *  @param[in]   aInterfaceId  the indicator of the network interface to
 *                             remove from the multicast group
 *
 *  @param[in]   aAddress      the multicast group to remove the
 *                             interface from
 *
 *  @retval  INET_NO_ERROR
 *       success: multicast group removed
 *
 *  @retval  INET_ERROR_UNKNOWN_INTERFACE
 *       unknown network interface, \c aInterfaceId
 *
 *  @retval  INET_ERROR_WRONG_ADDRESS_TYPE
 *       \c aAddress is not \c kIPAddressType_IPv4 or
 *       \c kIPAddressType_IPv6 or is not multicast
 *
 *  @retval  other
 *       another system or platform error
 *
 *  @details
 *     Remove the endpoint from the supplied multicast group on the
 *     specified interface.
 *
 */
INET_ERROR IPEndPointBasis::LeaveMulticastGroup(InterfaceId aInterfaceId, const IPAddress &aAddress)
{
    const IPAddressType lAddrType = aAddress.Type();
    INET_ERROR          lRetval = INET_ERROR_NOT_IMPLEMENTED;

#if WEAVE_SYSTEM_CONFIG_USE_LWIP || WEAVE_SYSTEM_CONFIG_USE_SOCKETS
    lRetval = CheckMulticastGroupArgs(aInterfaceId, aAddress);
    SuccessOrExit(lRetval);

    switch (lAddrType)
    {

#if INET_CONFIG_ENABLE_IPV4
    case kIPAddressType_IPv4:
        {
#if WEAVE_SYSTEM_CONFIG_USE_LWIP
#if LWIP_IPV4 && LWIP_IGMP
            lRetval = LwIPIPv4JoinLeaveMulticastGroup(aInterfaceId, aAddress, igmp_leavegroup_netif);
#else // LWIP_IPV4 && LWIP_IGMP
#pragma message "\n \
Please enable LWIP_IPV4 and LWIP_IGMP for IPv4 JoinMulticastGroup and LeaveMulticastGroup support."
            lRetval = INET_ERROR_NOT_SUPPORTED;
#endif // LWIP_IPV4 && LWIP_IGMP
            SuccessOrExit(lRetval);
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP

#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS
            lRetval = SocketsIPv4JoinLeaveMulticastGroup(mSocket, aInterfaceId, aAddress, IP_DROP_MEMBERSHIP);
            SuccessOrExit(lRetval);
#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS
        }
        break;
#endif // INET_CONFIG_ENABLE_IPV4

    case kIPAddressType_IPv6:
        {
#if WEAVE_SYSTEM_CONFIG_USE_LWIP
#if LWIP_IPV6_MLD && LWIP_IPV6_ND && LWIP_IPV6
            lRetval = LwIPIPv6JoinLeaveMulticastGroup(aInterfaceId, aAddress, mld6_leavegroup_netif);
#else // LWIP_IPV6_MLD && LWIP_IPV6_ND && LWIP_IPV6
#pragma message "\n \
Please enable LWIP_IPV6_MLD && LWIP_IPV6_ND && LWIP_IPV6 for IPv6 JoinMulticastGroup and LeaveMulticastGroup support."
            lRetval = INET_ERROR_NOT_SUPPORTED;
#endif // LWIP_IPV6_MLD && LWIP_IPV6_ND && LWIP_IPV6
            SuccessOrExit(lRetval);
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP

#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS
            lRetval = SocketsIPv6JoinLeaveMulticastGroup(mSocket, aInterfaceId, aAddress, INET_IPV6_DROP_MEMBERSHIP);
            SuccessOrExit(lRetval);
#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS
        }
        break;

    default:
        lRetval = INET_ERROR_WRONG_ADDRESS_TYPE;
        break;
    }

exit:
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP || WEAVE_SYSTEM_CONFIG_USE_SOCKETS
    return (lRetval);
}

void IPEndPointBasis::Init(InetLayer *aInetLayer)
{
    InitEndPointBasis(*aInetLayer);

#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS
    mBoundIntfId = INET_NULL_INTERFACEID;
#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS
}

#if WEAVE_SYSTEM_CONFIG_USE_LWIP
void IPEndPointBasis::HandleDataReceived(PacketBuffer *aBuffer)
{
    if ((mState == kState_Listening) && (OnMessageReceived != NULL))
    {
        const IPPacketInfo *pktInfo = GetPacketInfo(aBuffer);

        if (pktInfo != NULL)
        {
            const IPPacketInfo pktInfoCopy = *pktInfo;  // copy the address info so that the app can free the
                                                        // PacketBuffer without affecting access to address info.
            OnMessageReceived(this, aBuffer, &pktInfoCopy);
        }
        else
        {
            if (OnReceiveError != NULL)
                OnReceiveError(this, INET_ERROR_INBOUND_MESSAGE_TOO_BIG, NULL);
            PacketBuffer::Free(aBuffer);
        }
    }
    else
    {
        PacketBuffer::Free(aBuffer);
    }
}

/**
 *  @brief Get LwIP IP layer source and destination addressing information.
 *
 *  @param[in]   aBuffer       the packet buffer containing the IP message
 *
 *  @returns  a pointer to the address information on success; otherwise,
 *            NULL if there is insufficient space in the packet for
 *            the address information.
 *
 *  @details
 *     When using LwIP information about the packet is 'hidden' in the
 *     reserved space before the start of the data in the packet
 *     buffer. This is necessary because the system layer events only
 *     have two arguments, which in this case are used to convey the
 *     pointer to the end point and the pointer to the buffer.
 *
 *     In most cases this trick of storing information before the data
 *     works because the first buffer in an LwIP IP message contains
 *     the space that was used for the Ethernet/IP/UDP headers. However,
 *     given the current size of the IPPacketInfo structure (40 bytes),
 *     it is possible for there to not be enough room to store the
 *     structure along with the payload in a single packet buffer. In
 *     practice, this should only happen for extremely large IPv4
 *     packets that arrive without an Ethernet header.
 *
 */
IPPacketInfo *IPEndPointBasis::GetPacketInfo(PacketBuffer *aBuffer)
{
    uintptr_t       lStart;
    uintptr_t       lPacketInfoStart;
    IPPacketInfo *  lPacketInfo = NULL;

    if (!aBuffer->EnsureReservedSize(sizeof (IPPacketInfo) + 3))
        goto done;

    lStart           = (uintptr_t)aBuffer->Start();
    lPacketInfoStart = lStart - sizeof (IPPacketInfo);

    // Align to a 4-byte boundary

    lPacketInfo      = reinterpret_cast<IPPacketInfo *>(lPacketInfoStart & ~(sizeof (uint32_t) - 1));

 done:
    return (lPacketInfo);
}
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP

#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS
INET_ERROR IPEndPointBasis::Bind(IPAddressType aAddressType, IPAddress aAddress, uint16_t aPort, InterfaceId aInterfaceId)
{
    INET_ERROR lRetval = INET_NO_ERROR;

    if (aAddressType == kIPAddressType_IPv6)
    {
        struct sockaddr_in6 sa;

        memset(&sa, 0, sizeof (sa));

        sa.sin6_family   = AF_INET6;
        sa.sin6_port     = htons(aPort);
        sa.sin6_flowinfo = 0;
        sa.sin6_addr     = aAddress.ToIPv6();
        sa.sin6_scope_id = aInterfaceId;

        if (bind(mSocket, (const sockaddr *) &sa, (unsigned) sizeof (sa)) != 0)
            lRetval = Weave::System::MapErrorPOSIX(errno);

        // Instruct the kernel that any messages to multicast destinations should be
        // sent down the interface specified by the caller.
#ifdef IPV6_MULTICAST_IF
        if (lRetval == INET_NO_ERROR)
            setsockopt(mSocket, IPPROTO_IPV6, IPV6_MULTICAST_IF, &aInterfaceId, sizeof (aInterfaceId));
#endif // defined(IPV6_MULTICAST_IF)

        // Instruct the kernel that any messages to multicast destinations should be
        // set with the configured hop limit value.
#ifdef IPV6_MULTICAST_HOPS
        int hops = INET_CONFIG_IP_MULTICAST_HOP_LIMIT;
        setsockopt(mSocket, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &hops, sizeof (hops));
#endif // defined(IPV6_MULTICAST_HOPS)
    }
#if INET_CONFIG_ENABLE_IPV4
    else if (aAddressType == kIPAddressType_IPv4)
    {
        struct sockaddr_in sa;
        int enable = 1;

        memset(&sa, 0, sizeof (sa));

        sa.sin_family = AF_INET;
        sa.sin_port   = htons(aPort);
        sa.sin_addr   = aAddress.ToIPv4();

        if (bind(mSocket, (const sockaddr *) &sa, (unsigned) sizeof (sa)) != 0)
            lRetval = Weave::System::MapErrorPOSIX(errno);

        // Instruct the kernel that any messages to multicast destinations should be
        // sent down the interface to which the specified IPv4 address is bound.
#ifdef IP_MULTICAST_IF
        if (lRetval == INET_NO_ERROR)
            setsockopt(mSocket, IPPROTO_IP, IP_MULTICAST_IF, &sa, sizeof (sa));
#endif // defined(IP_MULTICAST_IF)

        // Instruct the kernel that any messages to multicast destinations should be
        // set with the configured hop limit value.
#ifdef IP_MULTICAST_TTL
        int ttl = INET_CONFIG_IP_MULTICAST_HOP_LIMIT;
        setsockopt(mSocket, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof (ttl));
#endif // defined(IP_MULTICAST_TTL)

        // Allow socket transmitting broadcast packets.
        if (lRetval == INET_NO_ERROR)
            setsockopt(mSocket, SOL_SOCKET, SO_BROADCAST, &enable, sizeof (enable));
    }
#endif // INET_CONFIG_ENABLE_IPV4
    else
        lRetval = INET_ERROR_WRONG_ADDRESS_TYPE;

    return (lRetval);
}

INET_ERROR IPEndPointBasis::BindInterface(IPAddressType aAddressType, InterfaceId aInterfaceId)
{
    INET_ERROR lRetval = INET_NO_ERROR;

#if HAVE_SO_BINDTODEVICE
    if (aInterfaceId == INET_NULL_INTERFACEID)
    {
        //Stop interface-based filtering.
        if (setsockopt(mSocket, SOL_SOCKET, SO_BINDTODEVICE, "", 0) == -1)
        {
            lRetval = Weave::System::MapErrorPOSIX(errno);
        }
    }
    else
    {
        //Start filtering on the passed interface.
        char lInterfaceName[IF_NAMESIZE];

        if (if_indextoname(aInterfaceId, lInterfaceName) == NULL)
        {
            lRetval = Weave::System::MapErrorPOSIX(errno);
        }

        if (lRetval == INET_NO_ERROR && setsockopt(mSocket, SOL_SOCKET, SO_BINDTODEVICE, lInterfaceName, strlen(lInterfaceName)) == -1)
        {
            lRetval = Weave::System::MapErrorPOSIX(errno);
        }
    }

    if (lRetval == INET_NO_ERROR)
        mBoundIntfId = aInterfaceId;

#else // !HAVE_SO_BINDTODEVICE
    lRetval = INET_ERROR_NOT_IMPLEMENTED;
#endif // HAVE_SO_BINDTODEVICE

    return (lRetval);
}

INET_ERROR IPEndPointBasis::SendMsg(const IPPacketInfo *aPktInfo, Weave::System::PacketBuffer *aBuffer, uint16_t aSendFlags)
{
    INET_ERROR     res = INET_NO_ERROR;
    PeerSockAddr   peerSockAddr;
    struct iovec   msgIOV;
    uint8_t        controlData[256];
    struct msghdr  msgHeader;
    InterfaceId    intfId = aPktInfo->Interface;

    // Ensure the destination address type is compatible with the endpoint address type.
    VerifyOrExit(mAddrType == aPktInfo->DestAddress.Type(), res = INET_ERROR_BAD_ARGS);

    // For now the entire message must fit within a single buffer.
    VerifyOrExit(aBuffer->Next() == NULL, res = INET_ERROR_MESSAGE_TOO_LONG);

    memset(&msgHeader, 0, sizeof (msgHeader));

    msgIOV.iov_base      = aBuffer->Start();
    msgIOV.iov_len       = aBuffer->DataLength();
    msgHeader.msg_iov    = &msgIOV;
    msgHeader.msg_iovlen = 1;

    // Construct a sockaddr_in/sockaddr_in6 structure containing the destination information.
    memset(&peerSockAddr, 0, sizeof (peerSockAddr));
    msgHeader.msg_name = &peerSockAddr;
    if (mAddrType == kIPAddressType_IPv6)
    {
        peerSockAddr.in6.sin6_family    = AF_INET6;
        peerSockAddr.in6.sin6_port      = htons(aPktInfo->DestPort);
        peerSockAddr.in6.sin6_flowinfo  = 0;
        peerSockAddr.in6.sin6_addr      = aPktInfo->DestAddress.ToIPv6();
        peerSockAddr.in6.sin6_scope_id  = aPktInfo->Interface;
        msgHeader.msg_namelen           = sizeof(sockaddr_in6);
    }
#if INET_CONFIG_ENABLE_IPV4
    else
    {
        peerSockAddr.in.sin_family      = AF_INET;
        peerSockAddr.in.sin_port        = htons(aPktInfo->DestPort);
        peerSockAddr.in.sin_addr        = aPktInfo->DestAddress.ToIPv4();
        msgHeader.msg_namelen           = sizeof(sockaddr_in);
    }
#endif // INET_CONFIG_ENABLE_IPV4

    // If the endpoint has been bound to a particular interface,
    // and the caller didn't supply a specific interface to send
    // on, use the bound interface. This appears to be necessary
    // for messages to multicast addresses, which under Linux
    // don't seem to get sent out the correct interface, despite
    // the socket being bound.
    if (intfId == INET_NULL_INTERFACEID)
        intfId = mBoundIntfId;

    // If the packet should be sent over a specific interface, or with a specific source
    // address, construct an IP_PKTINFO/IPV6_PKTINFO "control message" to that effect
    // add add it to the message header.  If the local OS doesn't support IP_PKTINFO/IPV6_PKTINFO
    // fail with an error.
    if (intfId != INET_NULL_INTERFACEID || aPktInfo->SrcAddress.Type() != kIPAddressType_Any)
    {
#if defined(IP_PKTINFO) || defined(IPV6_PKTINFO)
        memset(controlData, 0, sizeof(controlData));
        msgHeader.msg_control = controlData;
        msgHeader.msg_controllen = sizeof(controlData);

        struct cmsghdr *controlHdr = CMSG_FIRSTHDR(&msgHeader);

#if INET_CONFIG_ENABLE_IPV4

        if (mAddrType == kIPAddressType_IPv4)
        {
#if defined(IP_PKTINFO)
            controlHdr->cmsg_level = IPPROTO_IP;
            controlHdr->cmsg_type  = IP_PKTINFO;
            controlHdr->cmsg_len   = CMSG_LEN(sizeof(in_pktinfo));

            struct in_pktinfo *pktInfo = (struct in_pktinfo *)CMSG_DATA(controlHdr);
            pktInfo->ipi_ifindex = intfId;
            pktInfo->ipi_spec_dst = aPktInfo->SrcAddress.ToIPv4();

            msgHeader.msg_controllen = CMSG_SPACE(sizeof(in_pktinfo));
#else // !defined(IP_PKTINFO)
            ExitNow(res = INET_ERROR_NOT_SUPPORTED);
#endif // !defined(IP_PKTINFO)
        }

#endif // INET_CONFIG_ENABLE_IPV4

        if (mAddrType == kIPAddressType_IPv6)
        {
#if defined(IPV6_PKTINFO)
            controlHdr->cmsg_level = IPPROTO_IPV6;
            controlHdr->cmsg_type  = IPV6_PKTINFO;
            controlHdr->cmsg_len   = CMSG_LEN(sizeof(in6_pktinfo));

            struct in6_pktinfo *pktInfo = (struct in6_pktinfo *)CMSG_DATA(controlHdr);
            pktInfo->ipi6_ifindex = intfId;
            pktInfo->ipi6_addr = aPktInfo->SrcAddress.ToIPv6();

            msgHeader.msg_controllen = CMSG_SPACE(sizeof(in6_pktinfo));
#else // !defined(IPV6_PKTINFO)
            ExitNow(res = INET_ERROR_NOT_SUPPORTED);
#endif // !defined(IPV6_PKTINFO)
        }

#else // !(defined(IP_PKTINFO) && defined(IPV6_PKTINFO))

        ExitNow(res = INET_ERROR_NOT_SUPPORTED);

#endif // !(defined(IP_PKTINFO) && defined(IPV6_PKTINFO))
    }

    // Send IP packet.
    {
        const ssize_t lenSent = sendmsg(mSocket, &msgHeader, 0);
        if (lenSent == -1)
            res = Weave::System::MapErrorPOSIX(errno);
        else if (lenSent != aBuffer->DataLength())
            res = INET_ERROR_OUTBOUND_MESSAGE_TRUNCATED;
    }

exit:
    return (res);
}

INET_ERROR IPEndPointBasis::GetSocket(IPAddressType aAddressType, int aType, int aProtocol)
{
    INET_ERROR res = INET_NO_ERROR;

    if (mSocket == INET_INVALID_SOCKET_FD)
    {
        const int one = 1;
        int family;

        switch (aAddressType)
        {
        case kIPAddressType_IPv6:
            family = PF_INET6;
            break;

#if INET_CONFIG_ENABLE_IPV4
        case kIPAddressType_IPv4:
            family = PF_INET;
            break;
#endif // INET_CONFIG_ENABLE_IPV4

        default:
            return INET_ERROR_WRONG_ADDRESS_TYPE;
        }

        mSocket = ::socket(family, aType, aProtocol);
        if (mSocket == -1)
            return Weave::System::MapErrorPOSIX(errno);

        mAddrType = aAddressType;

        // NOTE WELL: the errors returned by setsockopt() here are not
        // returned as Inet layer Weave::System::MapErrorPOSIX(errno)
        // codes because they are normally expected to fail on some
        // platforms where the socket option code is defined in the
        // header files but not [yet] implemented. Certainly, there is
        // room to improve this by connecting the build configuration
        // logic up to check for implementations of these options and
        // to provide appropriate HAVE_xxxxx definitions accordingly.

        res = setsockopt(mSocket, SOL_SOCKET, SO_REUSEADDR,  (void*)&one, sizeof (one));
        static_cast<void>(res);

#ifdef SO_REUSEPORT
        res = setsockopt(mSocket, SOL_SOCKET, SO_REUSEPORT,  (void*)&one, sizeof (one));
        if (res != 0)
        {
            WeaveLogError(Inet, "SO_REUSEPORT failed: %d", errno);
        }
#endif // defined(SO_REUSEPORT)

        // If creating an IPv6 socket, tell the kernel that it will be
        // IPv6 only.  This makes it posible to bind two sockets to
        // the same port, one for IPv4 and one for IPv6.

#ifdef IPV6_V6ONLY
        if (aAddressType == kIPAddressType_IPv6)
        {
            res = setsockopt(mSocket, IPPROTO_IPV6, IPV6_V6ONLY, (void *) &one, sizeof (one));
            if (res != 0)
            {
                WeaveLogError(Inet, "IPV6_V6ONLY failed: %d", errno);
            }
        }
#endif // defined(IPV6_V6ONLY)

#if INET_CONFIG_ENABLE_IPV4
#ifdef IP_PKTINFO
        if (aAddressType == kIPAddressType_IPv4)
        {
            res = setsockopt(mSocket, IPPROTO_IP, IP_PKTINFO, (void *) &one, sizeof (one));
            if (res != 0)
            {
                WeaveLogError(Inet, "IP_PKTINFO failed: %d", errno);
            }
        }
#endif // defined(IP_PKTINFO)
#endif // INET_CONFIG_ENABLE_IPV4

#ifdef IPV6_RECVPKTINFO
        if (aAddressType == kIPAddressType_IPv6)
        {
            res = setsockopt(mSocket, IPPROTO_IPV6, IPV6_RECVPKTINFO, (void *) &one, sizeof (one));
            if (res != 0)
            {
                WeaveLogError(Inet, "IPV6_PKTINFO failed: %d", errno);
            }
        }
#endif // defined(IPV6_RECVPKTINFO)

        // On systems that support it, disable the delivery of SIGPIPE
        // signals when writing to a closed socket.  This is mostly
        // needed on iOS which has the peculiar habit of sending
        // SIGPIPEs on unconnected UDP sockets.
#ifdef SO_NOSIGPIPE
        {
            res = setsockopt(mSocket, SOL_SOCKET, SO_NOSIGPIPE, &one, sizeof (one));
            if (res != 0)
            {
                WeaveLogError(Inet, "SO_NOSIGPIPE failed: %d", errno);
            }
        }
#endif // defined(SO_NOSIGPIPE)

    }
    else if (mAddrType != aAddressType)
    {
        return INET_ERROR_INCORRECT_STATE;
    }

    return INET_NO_ERROR;
}

SocketEvents IPEndPointBasis::PrepareIO(void)
{
    SocketEvents res;

    if (mState == kState_Listening && OnMessageReceived != NULL)
        res.SetRead();

    return res;
}

void IPEndPointBasis::HandlePendingIO(uint16_t aPort)
{
    INET_ERROR      lStatus = INET_NO_ERROR;
    IPPacketInfo    lPacketInfo;
    PacketBuffer *  lBuffer;

    lPacketInfo.Clear();
    lPacketInfo.DestPort = aPort;

    lBuffer = PacketBuffer::New(0);

    if (lBuffer != NULL)
    {
        struct iovec msgIOV;
        PeerSockAddr lPeerSockAddr;
        uint8_t controlData[256];
        struct msghdr msgHeader;

        msgIOV.iov_base = lBuffer->Start();
        msgIOV.iov_len = lBuffer->AvailableDataLength();

        memset(&lPeerSockAddr, 0, sizeof (lPeerSockAddr));

        memset(&msgHeader, 0, sizeof (msgHeader));

        msgHeader.msg_name = &lPeerSockAddr;
        msgHeader.msg_namelen = sizeof (lPeerSockAddr);
        msgHeader.msg_iov = &msgIOV;
        msgHeader.msg_iovlen = 1;
        msgHeader.msg_control = controlData;
        msgHeader.msg_controllen = sizeof (controlData);

        ssize_t rcvLen = recvmsg(mSocket, &msgHeader, MSG_DONTWAIT);

        if (rcvLen < 0)
        {
            lStatus = Weave::System::MapErrorPOSIX(errno);
        }
        else if (rcvLen > lBuffer->AvailableDataLength())
        {
            lStatus = INET_ERROR_INBOUND_MESSAGE_TOO_BIG;
        }
        else
        {
            lBuffer->SetDataLength((uint16_t) rcvLen);

            if (lPeerSockAddr.any.sa_family == AF_INET6)
            {
                lPacketInfo.SrcAddress = IPAddress::FromIPv6(lPeerSockAddr.in6.sin6_addr);
                lPacketInfo.SrcPort = ntohs(lPeerSockAddr.in6.sin6_port);
            }
#if INET_CONFIG_ENABLE_IPV4
            else if (lPeerSockAddr.any.sa_family == AF_INET)
            {
                lPacketInfo.SrcAddress = IPAddress::FromIPv4(lPeerSockAddr.in.sin_addr);
                lPacketInfo.SrcPort = ntohs(lPeerSockAddr.in.sin_port);
            }
#endif // INET_CONFIG_ENABLE_IPV4
            else
            {
                lStatus = INET_ERROR_INCORRECT_STATE;
            }
        }

        if (lStatus == INET_NO_ERROR)
        {
            for (struct cmsghdr *controlHdr = CMSG_FIRSTHDR(&msgHeader);
                 controlHdr != NULL;
                 controlHdr = CMSG_NXTHDR(&msgHeader, controlHdr))
            {
#if INET_CONFIG_ENABLE_IPV4
#ifdef IP_PKTINFO
                if (controlHdr->cmsg_level == IPPROTO_IP && controlHdr->cmsg_type == IP_PKTINFO)
                {
                    struct in_pktinfo *inPktInfo = (struct in_pktinfo *)CMSG_DATA(controlHdr);
                    lPacketInfo.Interface = inPktInfo->ipi_ifindex;
                    lPacketInfo.DestAddress = IPAddress::FromIPv4(inPktInfo->ipi_addr);
                    continue;
                }
#endif // defined(IP_PKTINFO)
#endif // INET_CONFIG_ENABLE_IPV4

#ifdef IPV6_PKTINFO
                if (controlHdr->cmsg_level == IPPROTO_IPV6 && controlHdr->cmsg_type == IPV6_PKTINFO)
                {
                    struct in6_pktinfo *in6PktInfo = (struct in6_pktinfo *)CMSG_DATA(controlHdr);
                    lPacketInfo.Interface = in6PktInfo->ipi6_ifindex;
                    lPacketInfo.DestAddress = IPAddress::FromIPv6(in6PktInfo->ipi6_addr);
                    continue;
                }
#endif // defined(IPV6_PKTINFO)
            }
        }
    }
    else
    {
        lStatus = INET_ERROR_NO_MEMORY;
    }

    if (lStatus == INET_NO_ERROR)
        OnMessageReceived(this, lBuffer, &lPacketInfo);
    else
    {
        PacketBuffer::Free(lBuffer);
        if (OnReceiveError != NULL
            && lStatus != Weave::System::MapErrorPOSIX(EAGAIN)
           )
            OnReceiveError(this, lStatus, NULL);
    }

    return;
}
#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS

} // namespace Inet
} // namespace nl
