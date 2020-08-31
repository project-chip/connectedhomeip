/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Google LLC.
 *    Copyright (c) 2013-2018 Nest Labs, Inc.
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
 *      This file implements the <tt>Inet::UDPEndPoint</tt>
 *      class, where the CHIP Inet Layer encapsulates methods for
 *      interacting with UDP transport endpoints (SOCK_DGRAM sockets
 *      on Linux and BSD-derived systems) or LwIP UDP protocol
 *      control blocks, as the system is configured accordingly.
 *
 */

#define __APPLE_USE_RFC_3542
#include "UDPEndPoint.h"

#include "InetFaultInjection.h"
#include <inet/InetLayer.h>

#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>
#include <system/SystemFaultInjection.h>

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#include <lwip/ip.h>
#include <lwip/tcpip.h>
#include <lwip/udp.h>
#if CHIP_SEPARATE_CONFIG_H
#include <lwip/lwip_buildconfig.h>
#endif // CHIP_SEPARATE_CONFIG_H
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
#include <sys/select.h>
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif // HAVE_SYS_SOCKET_H
#include <errno.h>
#include <net/if.h>
#include <netinet/in.h>
#include <unistd.h>
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

#if CHIP_SYSTEM_CONFIG_USE_ZEPHYR_SOCKET_EXTENSIONS
#include "ZephyrSocket.h"
#endif // CHIP_SYSTEM_CONFIG_USE_ZEPHYR_SOCKET_EXTENSIONS

#include "arpa-inet-compatibility.h"

#include <string.h>

// SOCK_CLOEXEC not defined on all platforms, e.g. iOS/MacOS:
#ifdef SOCK_CLOEXEC
#define SOCK_FLAGS SOCK_CLOEXEC
#else
#define SOCK_FLAGS 0
#endif

namespace chip {
namespace Inet {

using chip::System::PacketBuffer;

chip::System::ObjectPool<UDPEndPoint, INET_CONFIG_NUM_UDP_ENDPOINTS> UDPEndPoint::sPool;

#if CHIP_SYSTEM_CONFIG_USE_LWIP
/*
 * Note that for LwIP InterfaceId is already defined to be 'struct
 * netif'; consequently, some of the checking performed here could
 * conceivably be optimized out and the HAVE_LWIP_UDP_BIND_NETIF case
 * could simply be:
 *
 *   udp_bind_netif(aUDP, intfId);
 *
 */
static INET_ERROR LwIPBindInterface(struct udp_pcb * aUDP, InterfaceId intfId)
{
    INET_ERROR res = INET_NO_ERROR;

#if HAVE_LWIP_UDP_BIND_NETIF
    if (!IsInterfaceIdPresent(intfId))
        udp_bind_netif(aUDP, NULL);
    else
    {
        struct netif * netifp = IPEndPointBasis::FindNetifFromInterfaceId(intfId);

        if (netifp == NULL)
            res = INET_ERROR_UNKNOWN_INTERFACE;
        else
            udp_bind_netif(aUDP, netifp);
    }
#else
    if (!IsInterfaceIdPresent(intfId))
        aUDP->intf_filter = NULL;
    else
    {
        struct netif * netifp = IPEndPointBasis::FindNetifFromInterfaceId(intfId);

        if (netifp == NULL)
            res = INET_ERROR_UNKNOWN_INTERFACE;
        else
            aUDP->intf_filter = netifp;
    }
#endif // HAVE_LWIP_UDP_BIND_NETIF

    return res;
}
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

/**
 * @brief   Bind the endpoint to an interface IP address.
 *
 * @param[in]   addrType    the protocol version of the IP address
 * @param[in]   addr        the IP address (must be an interface address)
 * @param[in]   port        the UDP port
 * @param[in]   intfId      an optional network interface indicator
 *
 * @retval  INET_NO_ERROR               success: endpoint bound to address
 * @retval  INET_ERROR_INCORRECT_STATE  endpoint has been bound previously
 * @retval  INET_NO_MEMORY              insufficient memory for endpoint
 *
 * @retval  INET_ERROR_UNKNOWN_INTERFACE
 *      On some platforms, the optionally specified interface is not
 *      present.
 *
 * @retval  INET_ERROR_WRONG_PROTOCOL_TYPE
 *      \c addrType does not match \c IPVer.
 *
 * @retval  INET_ERROR_WRONG_ADDRESS_TYPE
 *      \c addrType is \c kIPAddressType_Any, or the type of \c addr is not
 *      equal to \c addrType.
 *
 * @retval  other                   another system or platform error
 *
 * @details
 *  Binds the endpoint to the specified network interface IP address.
 *
 *  On LwIP, this method must not be called with the LwIP stack lock
 *  already acquired.
 */
INET_ERROR UDPEndPoint::Bind(IPAddressType addrType, IPAddress addr, uint16_t port, InterfaceId intfId)
{
    INET_ERROR res = INET_NO_ERROR;

#if CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK

    nw_parameters_configure_protocol_block_t configure_tls;
    nw_parameters_t parameters;

#endif // CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK

    if (mState != kState_Ready && mState != kState_Bound)
    {
        res = INET_ERROR_INCORRECT_STATE;
        goto exit;
    }

    if ((addr != IPAddress::Any) && (addr.Type() != kIPAddressType_Any) && (addr.Type() != addrType))
    {
        res = INET_ERROR_WRONG_ADDRESS_TYPE;
        goto exit;
    }

#if CHIP_SYSTEM_CONFIG_USE_LWIP

    // Lock LwIP stack
    LOCK_TCPIP_CORE();

    // Make sure we have the appropriate type of PCB.
    res = GetPCB(addrType);

    // Bind the PCB to the specified address/port.
    if (res == INET_NO_ERROR)
    {
#if LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
        ip_addr_t ipAddr = addr.ToLwIPAddr();
#if INET_CONFIG_ENABLE_IPV4
        lwip_ip_addr_type lType = IPAddress::ToLwIPAddrType(addrType);
        IP_SET_TYPE_VAL(ipAddr, lType);
#endif // INET_CONFIG_ENABLE_IPV4
        res = chip::System::MapErrorLwIP(udp_bind(mUDP, &ipAddr, port));
#else // LWIP_VERSION_MAJOR <= 1 && LWIP_VERSION_MINOR < 5
        if (addrType == kIPAddressType_IPv6)
        {
            ip6_addr_t ipv6Addr = addr.ToIPv6();
            res                 = chip::System::MapErrorLwIP(udp_bind_ip6(mUDP, &ipv6Addr, port));
        }
#if INET_CONFIG_ENABLE_IPV4
        else if (addrType == kIPAddressType_IPv4)
        {
            ip4_addr_t ipv4Addr = addr.ToIPv4();
            res                 = chip::System::MapErrorLwIP(udp_bind(mUDP, &ipv4Addr, port));
        }
#endif // INET_CONFIG_ENABLE_IPV4
        else
            res = INET_ERROR_WRONG_ADDRESS_TYPE;
#endif // LWIP_VERSION_MAJOR <= 1 || LWIP_VERSION_MINOR >= 5
    }

    if (res == INET_NO_ERROR)
    {
        res = LwIPBindInterface(mUDP, intfId);
    }

    // Unlock LwIP stack
    UNLOCK_TCPIP_CORE();

    SuccessOrExit(res);

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS

    // Make sure we have the appropriate type of socket.
    res = GetSocket(addrType);
    SuccessOrExit(res);

    res = IPEndPointBasis::Bind(addrType, addr, port, intfId);
    SuccessOrExit(res);

    mBoundPort   = port;
    mBoundIntfId = intfId;

    // If an ephemeral port was requested, retrieve the actual bound port.
    if (port == 0)
    {
        union
        {
            struct sockaddr any;
            struct sockaddr_in in;
            struct sockaddr_in6 in6;
        } boundAddr;
        socklen_t boundAddrLen = sizeof(boundAddr);

        if (getsockname(mSocket, &boundAddr.any, &boundAddrLen) == 0)
        {
            if (boundAddr.any.sa_family == AF_INET)
            {
                mBoundPort = ntohs(boundAddr.in.sin_port);
            }
            else if (boundAddr.any.sa_family == AF_INET6)
            {
                mBoundPort = ntohs(boundAddr.in6.sin6_port);
            }
        }
    }

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

#if CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK

    if (intfId != INET_NULL_INTERFACEID)
    {
        res = INET_ERROR_NOT_IMPLEMENTED;
        goto exit;
    }

    configure_tls = NW_PARAMETERS_DISABLE_PROTOCOL;
    parameters    = nw_parameters_create_secure_udp(configure_tls, NW_PARAMETERS_DEFAULT_CONFIGURATION);

    res = IPEndPointBasis::Bind(addrType, addr, port, parameters);
    SuccessOrExit(res);

    mParameters = parameters;

#endif // CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK

    if (res == INET_NO_ERROR)
    {
        mState = kState_Bound;
    }

exit:
    return res;
}

/**
 * @brief   Prepare the endpoint to receive UDP messages.
 *
 * @retval  INET_NO_ERROR   success: endpoint ready to receive messages.
 * @retval  INET_ERROR_INCORRECT_STATE  endpoint is already listening.
 *
 * @details
 *  If \c State is already \c kState_Listening, then no operation is
 *  performed, otherwise the \c mState is set to \c kState_Listening and
 *  the endpoint is prepared to received UDP messages, according to the
 *  semantics of the platform.
 *
 *  On LwIP, this method must not be called with the LwIP stack lock
 *  already acquired
 */
INET_ERROR UDPEndPoint::Listen(void)
{
    INET_ERROR res = INET_NO_ERROR;

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    chip::System::Layer & lSystemLayer = SystemLayer();
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

    if (mState == kState_Listening)
    {
        res = INET_NO_ERROR;
        goto exit;
    }

    if (mState != kState_Bound)
    {
        res = INET_ERROR_INCORRECT_STATE;
        goto exit;
    }

#if CHIP_SYSTEM_CONFIG_USE_LWIP

    // Lock LwIP stack
    LOCK_TCPIP_CORE();

#if LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
    udp_recv(mUDP, LwIPReceiveUDPMessage, this);
#else  // LWIP_VERSION_MAJOR <= 1 && LWIP_VERSION_MINOR < 5
    if (PCB_ISIPV6(mUDP))
        udp_recv_ip6(mUDP, LwIPReceiveUDPMessage, this);
    else
        udp_recv(mUDP, LwIPReceiveUDPMessage, this);
#endif // LWIP_VERSION_MAJOR <= 1 || LWIP_VERSION_MINOR >= 5

    // Unlock LwIP stack
    UNLOCK_TCPIP_CORE();

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS

    // Wake the thread calling select so that it starts selecting on the new socket.
    lSystemLayer.WakeSelect();

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

#if CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK

    res = StartListener();
    SuccessOrExit(res);

#endif // CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK

    if (res == INET_NO_ERROR)
    {
        mState = kState_Listening;
    }

exit:
    return res;
}

/**
 * @brief   Close the endpoint.
 *
 * @details
 *  If <tt>mState != kState_Closed</tt>, then closes the endpoint, removing
 *  it from the set of endpoints eligible for communication events.
 *
 *  On LwIP systems, this method must not be called with the LwIP stack
 *  lock already acquired.
 */
void UDPEndPoint::Close(void)
{
    if (mState != kState_Closed)
    {
#if CHIP_SYSTEM_CONFIG_USE_LWIP

        // Lock LwIP stack
        LOCK_TCPIP_CORE();

        // Since UDP PCB is released synchronously here, but UDP endpoint itself might have to wait
        // for destruction asynchronously, there could be more allocated UDP endpoints than UDP PCBs.
        if (mUDP != NULL)
        {
            udp_remove(mUDP);
            mUDP              = NULL;
            mLwIPEndPointType = kLwIPEndPointType_Unknown;
        }

        // Unlock LwIP stack
        UNLOCK_TCPIP_CORE();

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS

        if (mSocket != INET_INVALID_SOCKET_FD)
        {
            chip::System::Layer & lSystemLayer = SystemLayer();

            // Wake the thread calling select so that it recognizes the socket is closed.
            lSystemLayer.WakeSelect();

            close(mSocket);
            mSocket = INET_INVALID_SOCKET_FD;
        }

        // Clear any results from select() that indicate pending I/O for the socket.
        mPendingIO.Clear();

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

#if CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK
        IPEndPointBasis::ReleaseAll();
#endif // CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK

        mState = kState_Closed;
    }
}

/**
 * @brief   Close the endpoint and recycle its memory.
 *
 * @details
 *  Invokes the \c Close method, then invokes the
 *  <tt>InetLayerBasis::Release</tt> method to return the object to its
 *  memory pool.
 *
 *  On LwIP systems, this method must not be called with the LwIP stack
 *  lock already acquired.
 */
void UDPEndPoint::Free(void)
{
    Close();

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    DeferredFree(kReleaseDeferralErrorTactic_Die);
#else  // !CHIP_SYSTEM_CONFIG_USE_LWIP
    Release();
#endif // !CHIP_SYSTEM_CONFIG_USE_LWIP
}

/**
 *  A synonym for <tt>SendTo(addr, port, INET_NULL_INTERFACEID, msg, sendFlags)</tt>.
 */
INET_ERROR UDPEndPoint::SendTo(IPAddress addr, uint16_t port, chip::System::PacketBuffer * msg, uint16_t sendFlags)
{
    return SendTo(addr, port, INET_NULL_INTERFACEID, msg, sendFlags);
}

/**
 * @brief   Send a UDP message to the specified destination address.
 *
 * @param[in]   addr        the destination IP address
 * @param[in]   port        the destination UDP port
 * @param[in]   intfId      an optional network interface indicator
 * @param[in]   msg         the packet buffer containing the UDP message
 * @param[in]   sendFlags   optional transmit option flags
 *
 * @retval  INET_NO_ERROR       success: \c msg is queued for transmit.
 *
 * @retval  INET_ERROR_NOT_SUPPORTED
 *      the system does not support the requested operation.
 *
 * @retval  INET_ERROR_WRONG_ADDRESS_TYPE
 *      the destination address and the bound interface address do not
 *      have matching protocol versions or address type.
 *
 * @retval  INET_ERROR_MESSAGE_TOO_LONG
 *      \c msg does not contain the whole UDP message.
 *
 * @retval  INET_ERROR_OUTBOUND_MESSAGE_TRUNCATED
 *      On some platforms, only a truncated portion of \c msg was queued
 *      for transmit.
 *
 * @retval  other
 *      another system or platform error
 *
 * @details
 *      If possible, then this method sends the UDP message \c msg to the
 *      destination \c addr (with \c intfId used as the scope
 *      identifier for IPv6 link-local destinations) and \c port with the
 *      transmit option flags encoded in \c sendFlags.
 *
 *      Where <tt>(sendFlags & kSendFlag_RetainBuffer) != 0</tt>, calls
 *      <tt>chip::System::PacketBuffer::Free</tt> on behalf of the caller, otherwise this
 *      method deep-copies \c msg into a fresh object, and queues that for
 *      transmission, leaving the original \c msg available after return.
 */
INET_ERROR UDPEndPoint::SendTo(IPAddress addr, uint16_t port, InterfaceId intfId, chip::System::PacketBuffer * msg,
                               uint16_t sendFlags)
{
    IPPacketInfo pktInfo;
    pktInfo.Clear();
    pktInfo.DestAddress = addr;
    pktInfo.DestPort    = port;
    pktInfo.Interface   = intfId;
    return SendMsg(&pktInfo, msg, sendFlags);
}

/**
 * @brief   Send a UDP message to a specified destination.
 *
 * @param[in]   pktInfo     source and destination information for the UDP message
 * @param[in]   msg         a packet buffer containing the UDP message
 * @param[in]   sendFlags   optional transmit option flags
 *
 * @retval  INET_NO_ERROR
 *      success: \c msg is queued for transmit.
 *
 * @retval  INET_ERROR_NOT_SUPPORTED
 *      the system does not support the requested operation.
 *
 * @retval  INET_ERROR_WRONG_ADDRESS_TYPE
 *      the destination address and the bound interface address do not
 *      have matching protocol versions or address type.
 *
 * @retval  INET_ERROR_MESSAGE_TOO_LONG
 *      \c msg does not contain the whole UDP message.
 *
 * @retval  INET_ERROR_OUTBOUND_MESSAGE_TRUNCATED
 *      On some platforms, only a truncated portion of \c msg was queued
 *      for transmit.
 *
 * @retval  other
 *      another system or platform error
 *
 * @details
 *      Send the UDP message in \c msg to the destination address and port given in
 *      \c pktInfo.  If \c pktInfo contains an interface id, the message will be sent
 *      over the specified interface.  If \c pktInfo contains a source address, the
 *      given address will be used as the source of the UDP message.
 *
 *      Where <tt>(sendFlags & kSendFlag_RetainBuffer) != 0</tt>, calls
 *      <tt>chip::System::PacketBuffer::Free</tt> on behalf of the caller, otherwise this
 *      method deep-copies \c msg into a fresh object, and queues that for
 *      transmission, leaving the original \c msg available after return.
 */
INET_ERROR UDPEndPoint::SendMsg(const IPPacketInfo * pktInfo, PacketBuffer * msg, uint16_t sendFlags)
{
    INET_ERROR res             = INET_NO_ERROR;
    const IPAddress & destAddr = pktInfo->DestAddress;

    INET_FAULT_INJECT(FaultInjection::kFault_Send, if ((sendFlags & kSendFlag_RetainBuffer) == 0) PacketBuffer::Free(msg);
                      return INET_ERROR_UNKNOWN_INTERFACE;);
    INET_FAULT_INJECT(FaultInjection::kFault_SendNonCritical,
                      if ((sendFlags & kSendFlag_RetainBuffer) == 0) PacketBuffer::Free(msg);
                      return INET_ERROR_NO_MEMORY;);

#if CHIP_SYSTEM_CONFIG_USE_LWIP

    if (sendFlags & kSendFlag_RetainBuffer)
    {
        // when retaining a buffer, the caller expects the msg to be
        // unmodified.  LwIP stack will normally prepend the packet
        // headers as the packet traverses the UDP/IP/netif layers,
        // which normally modifies the packet.  We prepend a small
        // pbuf to the beginning of the pbuf chain, s.t. all headers
        // are added to the temporary space, just large enough to hold
        // the transport headers. Careful reader will note:
        //
        // * we're actually oversizing the reserved space, the
        //   transport header is large enough for the TCP header which
        //   is larger than the UDP header, but it seemed cleaner than
        //   the combination of PBUF_IP for reserve space, UDP_HLEN
        //   for payload, and post allocation adjustment of the header
        //   space).
        //
        // * the code deviates from the existing PacketBuffer
        //   abstractions and needs to reach into the underlying pbuf
        //   code.  The code in PacketBuffer also forces us to perform
        //   (effectively) a reinterpret_cast rather than a
        //   static_cast.  JIRA WEAV-811 is filed to track the
        //   re-architecting of the memory management.

        pbuf * msgCopy = pbuf_alloc(PBUF_TRANSPORT, 0, PBUF_RAM);

        if (msgCopy == NULL)
        {
            return INET_ERROR_NO_MEMORY;
        }

        pbuf_chain(msgCopy, (pbuf *) msg);
        msg = (PacketBuffer *) msgCopy;
    }

    // Lock LwIP stack
    LOCK_TCPIP_CORE();

    // Make sure we have the appropriate type of PCB based on the destination address.
    res = GetPCB(destAddr.Type());
    SuccessOrExit(res);

    // Send the message to the specified address/port.
    // If an outbound interface has been specified, call a specific version of the UDP sendto()
    // function that accepts the target interface.
    // If a source address has been specified, temporarily override the local_ip of the PCB.
    // This results in LwIP using the given address being as the source address for the generated
    // packet, as if the PCB had been bound to that address.
    {
        err_t lwipErr              = ERR_VAL;
        const IPAddress & srcAddr  = pktInfo->SrcAddress;
        const uint16_t & destPort  = pktInfo->DestPort;
        const InterfaceId & intfId = pktInfo->Interface;

#if LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5

        ip_addr_t lwipSrcAddr  = srcAddr.ToLwIPAddr();
        ip_addr_t lwipDestAddr = destAddr.ToLwIPAddr();

        ip_addr_t boundAddr;
        ip_addr_copy(boundAddr, mUDP->local_ip);

        if (!ip_addr_isany(&lwipSrcAddr))
        {
            ip_addr_copy(mUDP->local_ip, lwipSrcAddr);
        }

        if (intfId != INET_NULL_INTERFACEID)
            lwipErr = udp_sendto_if(mUDP, (pbuf *) msg, &lwipDestAddr, destPort, intfId);
        else
            lwipErr = udp_sendto(mUDP, (pbuf *) msg, &lwipDestAddr, destPort);

        ip_addr_copy(mUDP->local_ip, boundAddr);

#else // LWIP_VERSION_MAJOR <= 1 && LWIP_VERSION_MINOR < 5

        ipX_addr_t boundAddr;
        ipX_addr_copy(boundAddr, mUDP->local_ip);

        if (PCB_ISIPV6(mUDP))
        {
            ip6_addr_t lwipSrcAddr  = srcAddr.ToIPv6();
            ip6_addr_t lwipDestAddr = destAddr.ToIPv6();

            if (!ip6_addr_isany(&lwipSrcAddr))
            {
                ipX_addr_copy(mUDP->local_ip, *ip6_2_ipX(&lwipSrcAddr));
            }

            if (intfId != INET_NULL_INTERFACEID)
                lwipErr = udp_sendto_if_ip6(mUDP, (pbuf *) msg, &lwipDestAddr, destPort, intfId);
            else
                lwipErr = udp_sendto_ip6(mUDP, (pbuf *) msg, &lwipDestAddr, destPort);
        }

#if INET_CONFIG_ENABLE_IPV4

        else
        {
            ip4_addr_t lwipSrcAddr  = srcAddr.ToIPv4();
            ip4_addr_t lwipDestAddr = destAddr.ToIPv4();
            ipX_addr_t boundAddr;

            if (!ip_addr_isany(&lwipSrcAddr))
            {
                ipX_addr_copy(mUDP->local_ip, *ip_2_ipX(&lwipSrcAddr));
            }

            if (intfId != INET_NULL_INTERFACEID)
                lwipErr = udp_sendto_if(mUDP, (pbuf *) msg, &lwipDestAddr, destPort, intfId);
            else
                lwipErr = udp_sendto(mUDP, (pbuf *) msg, &lwipDestAddr, destPort);
        }

        ipX_addr_copy(mUDP->local_ip, boundAddr);

#endif // INET_CONFIG_ENABLE_IPV4
#endif // LWIP_VERSION_MAJOR <= 1 || LWIP_VERSION_MINOR >= 5

        if (lwipErr != ERR_OK)
            res = chip::System::MapErrorLwIP(lwipErr);
    }

    // Unlock LwIP stack
    UNLOCK_TCPIP_CORE();

    PacketBuffer::Free(msg);
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS

    // Make sure we have the appropriate type of socket based on the
    // destination address.

    res = GetSocket(destAddr.Type());
    SuccessOrExit(res);

    res = IPEndPointBasis::SendMsg(pktInfo, msg, sendFlags);

    if ((sendFlags & kSendFlag_RetainBuffer) == 0)
        PacketBuffer::Free(msg);
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

#if CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK
    res = IPEndPointBasis::SendMsg(pktInfo, msg, sendFlags);

    if ((sendFlags & kSendFlag_RetainBuffer) == 0)
        PacketBuffer::Free(msg);
#endif // CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK

exit:
    CHIP_SYSTEM_FAULT_INJECT_ASYNC_EVENT();

    return res;
}

/**
 * @brief   Bind the endpoint to a network interface.
 *
 * @param[in]   addrType    the protocol version of the IP address.
 *
 * @param[in]   intf        indicator of the network interface.
 *
 * @retval  INET_NO_ERROR               success: endpoint bound to address
 * @retval  INET_NO_MEMORY              insufficient memory for endpoint
 * @retval  INET_ERROR_NOT_IMPLEMENTED  system implementation not complete.
 *
 * @retval  INET_ERROR_UNKNOWN_INTERFACE
 *      On some platforms, the interface is not present.
 *
 * @retval  other                   another system or platform error
 *
 * @details
 *  Binds the endpoint to the specified network interface IP address.
 *
 *  On LwIP, this method must not be called with the LwIP stack lock
 *  already acquired.
 */
INET_ERROR UDPEndPoint::BindInterface(IPAddressType addrType, InterfaceId intfId)
{
    INET_ERROR err = INET_NO_ERROR;

    if (mState != kState_Ready && mState != kState_Bound)
        return INET_ERROR_INCORRECT_STATE;

#if CHIP_SYSTEM_CONFIG_USE_LWIP

    // A lock is required because the LwIP thread may be referring to intf_filter,
    // while this code running in the Inet application is potentially modifying it.
    // NOTE: this only supports LwIP interfaces whose number is no bigger than 9.
    LOCK_TCPIP_CORE();

    // Make sure we have the appropriate type of PCB.
    err = GetPCB(addrType);
    SuccessOrExit(err);

    err = LwIPBindInterface(mUDP, intfId);

    UNLOCK_TCPIP_CORE();

    SuccessOrExit(err);

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    // Make sure we have the appropriate type of socket.
    err = GetSocket(addrType);
    SuccessOrExit(err);

    err = IPEndPointBasis::BindInterface(addrType, intfId);
    SuccessOrExit(err);
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

#if CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK
    err = INET_ERROR_UNKNOWN_INTERFACE;
    SuccessOrExit(err);
#endif // CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK

    if (err == INET_NO_ERROR)
    {
        mState = kState_Bound;
    }
exit:
    return err;
}

void UDPEndPoint::Init(InetLayer * inetLayer)
{
    IPEndPointBasis::Init(inetLayer);
}

/**
 * Get the bound interface on this endpoint.
 *
 * @return InterfaceId   The bound interface id.
 */
InterfaceId UDPEndPoint::GetBoundInterface(void)
{
#if CHIP_SYSTEM_CONFIG_USE_LWIP
#if HAVE_LWIP_UDP_BIND_NETIF
    return netif_get_by_index(mUDP->netif_idx);
#else
    return mUDP->intf_filter;
#endif
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    return mBoundIntfId;
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

#if CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK
    return INET_NULL_INTERFACEID;
#endif // CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK
}

uint16_t UDPEndPoint::GetBoundPort(void)
{
#if CHIP_SYSTEM_CONFIG_USE_LWIP
    return mUDP->local_port;
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    return mBoundPort;
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

#if CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK
    nw_endpoint_t endpoint = nw_parameters_copy_local_endpoint(mParameters);
    return nw_endpoint_get_port(endpoint);
#endif // CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK
}

#if CHIP_SYSTEM_CONFIG_USE_LWIP

void UDPEndPoint::HandleDataReceived(PacketBuffer * msg)
{
    IPEndPointBasis::HandleDataReceived(msg);
}

INET_ERROR UDPEndPoint::GetPCB(IPAddressType addrType)
{
    INET_ERROR err = INET_NO_ERROR;

    // IMPORTANT: This method MUST be called with the LwIP stack LOCKED!

    // If a PCB hasn't been allocated yet...
    if (mUDP == NULL)
    {
        // Allocate a PCB of the appropriate type.
        if (addrType == kIPAddressType_IPv6)
        {
#if LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
            mUDP = udp_new_ip_type(IPADDR_TYPE_V6);
#else  // LWIP_VERSION_MAJOR <= 1 && LWIP_VERSION_MINOR < 5
            mUDP = udp_new_ip6();
#endif // LWIP_VERSION_MAJOR <= 1 || LWIP_VERSION_MINOR >= 5
        }
#if INET_CONFIG_ENABLE_IPV4
        else if (addrType == kIPAddressType_IPv4)
        {
#if LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
            mUDP = udp_new_ip_type(IPADDR_TYPE_V4);
#else  // LWIP_VERSION_MAJOR <= 1 && LWIP_VERSION_MINOR < 5
            mUDP = udp_new();
#endif // LWIP_VERSION_MAJOR <= 1 || LWIP_VERSION_MINOR >= 5
        }
#endif // INET_CONFIG_ENABLE_IPV4
        else
        {
            ExitNow(err = INET_ERROR_WRONG_ADDRESS_TYPE);
        }

        // Fail if the system has run out of PCBs.
        if (mUDP == NULL)
        {
            ChipLogError(Inet, "Unable to allocate UDP PCB");
            ExitNow(err = INET_ERROR_NO_MEMORY);
        }

        // Allow multiple bindings to the same port.
        ip_set_option(mUDP, SOF_REUSEADDR);
    }

    // Otherwise, verify that the existing PCB is the correct type...
    else
    {
        IPAddressType pcbAddrType;

        // Get the address type of the existing PCB.
#if LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
        switch (static_cast<lwip_ip_addr_type>(IP_GET_TYPE(&mUDP->local_ip)))
        {
        case IPADDR_TYPE_V6:
            pcbAddrType = kIPAddressType_IPv6;
            break;
#if INET_CONFIG_ENABLE_IPV4
        case IPADDR_TYPE_V4:
            pcbAddrType = kIPAddressType_IPv4;
            break;
#endif // INET_CONFIG_ENABLE_IPV4
        default:
            ExitNow(err = INET_ERROR_WRONG_ADDRESS_TYPE);
        }
#else // LWIP_VERSION_MAJOR <= 1 && LWIP_VERSION_MINOR < 5
#if INET_CONFIG_ENABLE_IPV4
        pcbAddrType = PCB_ISIPV6(mUDP) ? kIPAddressType_IPv6 : kIPAddressType_IPv4;
#else  // !INET_CONFIG_ENABLE_IPV4
        pcbAddrType = kIPAddressType_IPv6;
#endif // !INET_CONFIG_ENABLE_IPV4
#endif // LWIP_VERSION_MAJOR <= 1 && LWIP_VERSION_MINOR < 5

        // Fail if the existing PCB is not the correct type.
        VerifyOrExit(addrType == pcbAddrType, err = INET_ERROR_WRONG_ADDRESS_TYPE);
    }

exit:
    return err;
}

#if LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
void UDPEndPoint::LwIPReceiveUDPMessage(void * arg, struct udp_pcb * pcb, struct pbuf * p, const ip_addr_t * addr, u16_t port)
#else  // LWIP_VERSION_MAJOR <= 1 && LWIP_VERSION_MINOR < 5
void UDPEndPoint::LwIPReceiveUDPMessage(void * arg, struct udp_pcb * pcb, struct pbuf * p, ip_addr_t * addr, u16_t port)
#endif // LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
{
    UDPEndPoint * ep                   = static_cast<UDPEndPoint *>(arg);
    PacketBuffer * buf                 = reinterpret_cast<PacketBuffer *>(static_cast<void *>(p));
    chip::System::Layer & lSystemLayer = ep->SystemLayer();
    IPPacketInfo * pktInfo             = NULL;

    pktInfo = GetPacketInfo(buf);
    if (pktInfo != NULL)
    {
#if LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
        pktInfo->SrcAddress  = IPAddress::FromLwIPAddr(*addr);
        pktInfo->DestAddress = IPAddress::FromLwIPAddr(*ip_current_dest_addr());
#else // LWIP_VERSION_MAJOR <= 1
        if (PCB_ISIPV6(pcb))
        {
            pktInfo->SrcAddress = IPAddress::FromIPv6(*(ip6_addr_t *) addr);
            pktInfo->DestAddress = IPAddress::FromIPv6(*ip6_current_dest_addr());
        }
#if INET_CONFIG_ENABLE_IPV4
        else
        {
            pktInfo->SrcAddress = IPAddress::FromIPv4(*addr);
            pktInfo->DestAddress = IPAddress::FromIPv4(*ip_current_dest_addr());
        }
#endif // INET_CONFIG_ENABLE_IPV4
#endif // LWIP_VERSION_MAJOR <= 1

        pktInfo->Interface = ip_current_netif();
        pktInfo->SrcPort   = port;
        pktInfo->DestPort  = pcb->local_port;
    }

    if (lSystemLayer.PostEvent(*ep, kInetEvent_UDPDataReceived, (uintptr_t) buf) != INET_NO_ERROR)
        PacketBuffer::Free(buf);
}

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
INET_ERROR UDPEndPoint::GetSocket(IPAddressType aAddressType)
{
    INET_ERROR lRetval  = INET_NO_ERROR;
    const int lType     = (SOCK_DGRAM | SOCK_FLAGS);
    const int lProtocol = 0;

    lRetval = IPEndPointBasis::GetSocket(aAddressType, lType, lProtocol);
    SuccessOrExit(lRetval);

exit:
    return (lRetval);
}

SocketEvents UDPEndPoint::PrepareIO(void)
{
    return (IPEndPointBasis::PrepareIO());
}

void UDPEndPoint::HandlePendingIO(void)
{
    if (mState == kState_Listening && OnMessageReceived != NULL && mPendingIO.IsReadable())
    {
        const uint16_t lPort = mBoundPort;

        IPEndPointBasis::HandlePendingIO(lPort);
    }

    mPendingIO.Clear();
}

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

} // namespace Inet
} // namespace chip
