/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 *      This file implements the <tt>Inet::RawEndPoint</tt> class,
 *      where the CHIP Inet Layer encapsulates methods for interacting
 *      interacting with IP network endpoints (SOCK_RAW sockets
 *      on Linux and BSD-derived systems) or LwIP raw protocol
 *      control blocks, as the system is configured accordingly.
 *
 */

#define __APPLE_USE_RFC_3542

#include "RawEndPoint.h"

#include "InetFaultInjection.h"
#include <inet/InetLayer.h>

#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>
#include <system/SystemFaultInjection.h>

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#include <lwip/ip.h>
#include <lwip/raw.h>
#include <lwip/tcpip.h>
#if CHIP_HAVE_CONFIG_H
#include <lwip/lwip_buildconfig.h> // nogncheck
#endif                             // CHIP_HAVE_CONFIG_H
#endif                             // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
#include <system/SystemSockets.h>
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif // HAVE_SYS_SOCKET_H
#include <errno.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>
#if HAVE_NETINET_ICMP6_H
#include <netinet/icmp6.h>
#endif // HAVE_NETINET_ICMP6_H
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

// SOCK_CLOEXEC not defined on all platforms, e.g. iOS/macOS:
#ifdef SOCK_CLOEXEC
#define SOCK_FLAGS SOCK_CLOEXEC
#else
#define SOCK_FLAGS 0
#endif

#include <string.h>
#include <utility>

namespace chip {
namespace Inet {

chip::System::ObjectPool<RawEndPoint, INET_CONFIG_NUM_RAW_ENDPOINTS> RawEndPoint::sPool;

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
static CHIP_ERROR LwIPBindInterface(struct raw_pcb * aRaw, InterfaceId intfId)
{
    CHIP_ERROR res = CHIP_NO_ERROR;

#if HAVE_LWIP_RAW_BIND_NETIF
    if (!IsInterfaceIdPresent(intfId))
        raw_bind_netif(aRaw, NULL);
    else
    {
        struct netif * netifp = IPEndPointBasis::FindNetifFromInterfaceId(intfId);

        if (netifp == NULL)
            res = INET_ERROR_UNKNOWN_INTERFACE;
        else
            raw_bind_netif(aRaw, netifp);
    }
#else
    if (!IsInterfaceIdPresent(intfId))
        aRaw->intf_filter = NULL;
    else
    {
        struct netif * netifp = IPEndPointBasis::FindNetifFromInterfaceId(intfId);

        if (netifp == NULL)
            res = INET_ERROR_UNKNOWN_INTERFACE;
        else
            aRaw->intf_filter = netifp;
    }
#endif // HAVE_LWIP_RAW_BIND_NETIF

    return res;
}
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

/**
 * @brief   Bind the endpoint to an interface IP address.
 *
 * @param[in]   addrType    the protocol version of the IP address
 * @param[in]   addr        the IP address (must be an interface address)
 * @param[in]   intfId      an optional network interface indicator
 *
 * @retval  CHIP_NO_ERROR               success: endpoint bound to address
 * @retval  CHIP_ERROR_INCORRECT_STATE  endpoint has been bound previously
 * @retval  CHIP_ERROR_NO_MEMORY        insufficient memory for endpoint
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
CHIP_ERROR RawEndPoint::Bind(IPAddressType addrType, const IPAddress & addr, InterfaceId intfId)
{
    if (mState != kState_Ready && mState != kState_Bound)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    if ((addr != IPAddress::Any) && (addr.Type() != kIPAddressType_Any) && (addr.Type() != addrType))
    {
        return INET_ERROR_WRONG_ADDRESS_TYPE;
    }

#if CHIP_SYSTEM_CONFIG_USE_LWIP

    // Lock LwIP stack
    LOCK_TCPIP_CORE();

    // Make sure we have the appropriate type of PCB.
    CHIP_ERROR res = GetPCB(addrType);

    // Bind the PCB to the specified address.
    if (res == CHIP_NO_ERROR)
    {
#if LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
        ip_addr_t ipAddr = addr.ToLwIPAddr();
#if INET_CONFIG_ENABLE_IPV4
        lwip_ip_addr_type lType = IPAddress::ToLwIPAddrType(addrType);
        IP_SET_TYPE_VAL(ipAddr, lType);
#endif // INET_CONFIG_ENABLE_IPV4
        res = chip::System::MapErrorLwIP(raw_bind(mRaw, &ipAddr));
#else // LWIP_VERSION_MAJOR <= 1 && LWIP_VERSION_MINOR < 5
        if (addrType == kIPAddressType_IPv6)
        {
            ip6_addr_t ipv6Addr = addr.ToIPv6();
            res                 = chip::System::MapErrorLwIP(raw_bind_ip6(mRaw, &ipv6Addr));
        }
#if INET_CONFIG_ENABLE_IPV4
        else if (addrType == kIPAddressType_IPv4)
        {
            ip4_addr_t ipv4Addr = addr.ToIPv4();
            res                 = chip::System::MapErrorLwIP(raw_bind(mRaw, &ipv4Addr));
        }
#endif // INET_CONFIG_ENABLE_IPV4
        else
            res = INET_ERROR_WRONG_ADDRESS_TYPE;
#endif // LWIP_VERSION_MAJOR <= 1 || LWIP_VERSION_MINOR >= 5
    }

    if (res == CHIP_NO_ERROR)
    {
        res = LwIPBindInterface(mRaw, intfId);
    }

    // Unlock LwIP stack
    UNLOCK_TCPIP_CORE();

    ReturnErrorOnFailure(res);

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    // Make sure we have the appropriate type of socket.
    ReturnErrorOnFailure(GetSocket(addrType));
    ReturnErrorOnFailure(IPEndPointBasis::Bind(addrType, addr, 0, intfId));

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
    dispatch_queue_t dispatchQueue = SystemLayer().GetDispatchQueue();
    if (dispatchQueue != nullptr)
    {
        unsigned long fd = static_cast<unsigned long>(mSocket.GetFD());

        mReadableSource = dispatch_source_create(DISPATCH_SOURCE_TYPE_READ, fd, 0, dispatchQueue);
        ReturnErrorCodeIf(mReadableSource == nullptr, CHIP_ERROR_NO_MEMORY);

        dispatch_source_set_event_handler(mReadableSource, ^{
            this->mSocket.SetPendingIO(System::SocketEventFlags::kRead);
            this->HandlePendingIO();
        });

        dispatch_resume(mReadableSource);
    }
#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH

    mBoundIntfId = intfId;
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

    mState = kState_Bound;

    return CHIP_NO_ERROR;
}

/**
 * Bind the raw endpoint to an IPv6 link-local scope address at the specified
 * interface index.  Also sets various IPv6 socket options appropriate for
 * transmitting packets to and from on-link destinations.
 *
 * @param[in]   intfId  An InterfaceId to identify the scope of the address.
 * @param[in]   addr    An IPv6 link-local scope IPAddress object.
 *
 * @retval  CHIP_NO_ERROR               success: endpoint bound to address
 * @retval  CHIP_ERROR_INCORRECT_STATE  endpoint has been bound previously
 * @retval  CHIP_ERROR_NO_MEMORY        insufficient memory for endpoint
 *
 * @retval  INET_ERROR_WRONG_PROTOCOL_TYPE
 *      \c addrType does not match \c IPVer.
 *
 * @retval  INET_ERROR_WRONG_ADDRESS_TYPE
 *      \c addr is not an IPv6 link-local address or \c intfId is
 *      \c INET_NULL_INTERFACEID.
 *
 * @retval  other                   another system or platform error
 *
 * @details
 *  Binds the endpoint to the IPv6 link-local address \c addr on the
 *  network interface indicated by \c intfId.
 *
 *  On LwIP, this method must not be called with the LwIP stack lock
 *  already acquired.
 */
CHIP_ERROR RawEndPoint::BindIPv6LinkLocal(InterfaceId intfId, const IPAddress & addr)
{
    CHIP_ERROR res = CHIP_NO_ERROR;

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    const int lIfIndex = static_cast<int>(intfId);
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

    if (mState != kState_Ready && mState != kState_Bound)
    {
        res = CHIP_ERROR_INCORRECT_STATE;
        goto ret;
    }

    if (!addr.IsIPv6LinkLocal())
    {
        res = INET_ERROR_WRONG_ADDRESS_TYPE;
        goto ret;
    }

#if CHIP_SYSTEM_CONFIG_USE_LWIP

    // Lock LwIP stack
    LOCK_TCPIP_CORE();

    // Make sure we have the appropriate type of PCB.
    res = GetPCB(addr.Type());

    // Bind the PCB to the specified address.
    if (res == CHIP_NO_ERROR)
    {
#if LWIP_VERSION_MAJOR > 1
        ip_addr_t ipAddr = addr.ToLwIPAddr();
        res              = chip::System::MapErrorLwIP(raw_bind(mRaw, &ipAddr));
#else  // LWIP_VERSION_MAJOR <= 1
        ip6_addr_t ipv6Addr = addr.ToIPv6();
        res                 = chip::System::MapErrorLwIP(raw_bind_ip6(mRaw, &ipv6Addr));
#endif // LWIP_VERSION_MAJOR <= 1

        if (res != CHIP_NO_ERROR)
        {
            raw_remove(mRaw);
            mRaw              = NULL;
            mLwIPEndPointType = kLwIPEndPointType_Unknown;
        }
    }

    // Unlock LwIP stack
    UNLOCK_TCPIP_CORE();

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS

    static const int sInt255 = 255;

    // Make sure we have the appropriate type of socket.
    res = GetSocket(kIPAddressType_IPv6);
    if (res != CHIP_NO_ERROR)
    {
        goto ret;
    }

    if (::setsockopt(mSocket.GetFD(), IPPROTO_IPV6, IPV6_MULTICAST_IF, &lIfIndex, sizeof(lIfIndex)) != 0)
    {
        goto optfail;
    }

    if (::setsockopt(mSocket.GetFD(), IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &sInt255, sizeof(sInt255)) != 0)
    {
        goto optfail;
    }

    if (::setsockopt(mSocket.GetFD(), IPPROTO_IPV6, IPV6_UNICAST_HOPS, &sInt255, sizeof(sInt255)) != 0)
    {
        goto optfail;
    }

    mAddrType = kIPAddressType_IPv6;
    goto ret;

optfail:
    res = chip::System::MapErrorPOSIX(errno);
    mSocket.Close();
    mAddrType = kIPAddressType_Unknown;

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

ret:
    if (res == CHIP_NO_ERROR)
    {
        mState = kState_Bound;
    }

    return res;
}

/**
 * @brief   Prepare the endpoint to receive ICMP messages.
 *
 * @param[in]  onMessageReceived   The endpoint's message reception event handling function delegate.
 * @param[in]  onReceiveError      The endpoint's receive error event handling function delegate.
 * @param[in]  appState            Application state pointer.
 *
 * @retval  CHIP_NO_ERROR   always returned.
 *
 * @details
 *  If \c mState is already \c kState_Listening, then no operation is
 *  performed, otherwise the \c mState is set to \c kState_Listening and
 *  the endpoint is prepared to received ICMPv6 messages, according to the
 *  semantics of the platform.
 *
 *  On LwIP, this method must not be called with the LwIP stack lock
 *  already acquired
 */
CHIP_ERROR RawEndPoint::Listen(IPEndPointBasis::OnMessageReceivedFunct onMessageReceived,
                               IPEndPointBasis::OnReceiveErrorFunct onReceiveError, void * appState)
{
    if (mState == kState_Listening)
    {
        return CHIP_NO_ERROR;
    }

    if (mState != kState_Bound)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    OnMessageReceived = onMessageReceived;
    OnReceiveError    = onReceiveError;
    AppState          = appState;

#if CHIP_SYSTEM_CONFIG_USE_LWIP

    // Lock LwIP stack
    LOCK_TCPIP_CORE();

#if LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
    raw_recv(mRaw, LwIPReceiveRawMessage, this);
#else  // LWIP_VERSION_MAJOR <= 1 && LWIP_VERSION_MINOR < 5
    if (PCB_ISIPV6(mRaw))
        raw_recv_ip6(mRaw, LwIPReceiveRawMessage, this);
    else
        raw_recv(mRaw, LwIPReceiveRawMessage, this);
#endif // LWIP_VERSION_MAJOR <= 1 || LWIP_VERSION_MINOR >= 5

    // Unlock LwIP stack
    UNLOCK_TCPIP_CORE();

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

    mState = kState_Listening;

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    // Wait for ability to read on this endpoint.
    mSocket.SetCallback(HandlePendingIO, reinterpret_cast<intptr_t>(this));
    mSocket.RequestCallbackOnPendingRead();
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

    return CHIP_NO_ERROR;
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
void RawEndPoint::Close()
{
    if (mState != kState_Closed)
    {
#if CHIP_SYSTEM_CONFIG_USE_LWIP

        // Lock LwIP stack
        LOCK_TCPIP_CORE();

        // Since Raw PCB is released synchronously here, but Raw endpoint itself might have to wait
        // for destruction asynchronously, there could be more allocated Raw endpoints than Raw PCBs.
        if (mRaw != NULL)
        {
            raw_remove(mRaw);
            mRaw              = NULL;
            mLwIPEndPointType = kLwIPEndPointType_Unknown;
        }

        // Unlock LwIP stack
        UNLOCK_TCPIP_CORE();

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS

        if (mSocket.HasFD())
        {
            mSocket.Close();
        }

        // Clear any results from select() that indicate pending I/O for the socket.
        mSocket.ClearPendingIO();

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
        if (mReadableSource)
        {
            dispatch_source_cancel(mReadableSource);
            dispatch_release(mReadableSource);
        }
#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

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
void RawEndPoint::Free()
{
    Close();

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    DeferredFree(kReleaseDeferralErrorTactic_Die);
#else  // !CHIP_SYSTEM_CONFIG_USE_LWIP
    Release();
#endif // !CHIP_SYSTEM_CONFIG_USE_LWIP
}

/**
 *  A synonym for <tt>SendTo(addr, INET_NULL_INTERFACEID, msg,
 *  sendFlags)</tt>.
 */
CHIP_ERROR RawEndPoint::SendTo(const IPAddress & addr, chip::System::PacketBufferHandle && msg, uint16_t sendFlags)
{
    return SendTo(addr, INET_NULL_INTERFACEID, std::move(msg), sendFlags);
}

/**
 * @brief   Send an ICMP message to the specified destination address.
 *
 * @param[in]   addr        the destination IP address
 * @param[in]   intfId      an optional network interface indicator
 * @param[in]   msg         the packet buffer containing the UDP message
 * @param[in]   sendFlags   optional transmit option flags
 *
 * @retval  CHIP_NO_ERROR
 *      success: \c msg is queued for transmit.
 *
 * @retval  CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE
 *      the system does not support the requested operation.
 *
 * @retval  INET_ERROR_WRONG_ADDRESS_TYPE
 *      the destination address and the bound interface address do not
 *      have matching protocol versions or address type.
 *
 * @retval  CHIP_ERROR_MESSAGE_TOO_LONG
 *      \c msg does not contain the whole ICMP message.
 *
 * @retval  CHIP_ERROR_OUTBOUND_MESSAGE_TOO_BIG
 *      On some platforms, only a truncated portion of \c msg was queued
 *      for transmit.
 *
 * @retval  other                   another system or platform error
 *
 * @details
 *      Send the ICMP message in \c msg to the destination given in \c addr.
 */
CHIP_ERROR RawEndPoint::SendTo(const IPAddress & addr, InterfaceId intfId, chip::System::PacketBufferHandle && msg,
                               uint16_t sendFlags)
{
    IPPacketInfo pktInfo;
    pktInfo.Clear();
    pktInfo.DestAddress = addr;
    pktInfo.Interface   = intfId;
    return SendMsg(&pktInfo, std::move(msg), sendFlags);
}

/**
 * @brief   Send an ICMP message to the specified destination.
 *
 * @param[in]   pktInfo     destination information for the message
 * @param[in]   msg         the packet buffer containing the UDP message
 * @param[in]   sendFlags   optional transmit option flags
 *
 * @retval  CHIP_NO_ERROR
 *      success: \c msg is queued for transmit.
 *
 * @retval  CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE
 *      the system does not support the requested operation.
 *
 * @retval  INET_ERROR_WRONG_ADDRESS_TYPE
 *      the destination address and the bound interface address do not
 *      have matching protocol versions or address type.
 *
 * @retval  CHIP_ERROR_MESSAGE_TOO_LONG
 *      \c msg does not contain the whole ICMP message.
 *
 * @retval  CHIP_ERROR_OUTBOUND_MESSAGE_TOO_BIG
 *      On some platforms, only a truncated portion of \c msg was queued
 *      for transmit.
 *
 * @retval  other                   another system or platform error
 *
 * @details
 *      Send the ICMP message \c msg using the destination information given in \c addr.
 */
CHIP_ERROR RawEndPoint::SendMsg(const IPPacketInfo * pktInfo, chip::System::PacketBufferHandle && msg, uint16_t sendFlags)
{
    CHIP_ERROR res         = CHIP_NO_ERROR;
    const IPAddress & addr = pktInfo->DestAddress;

    INET_FAULT_INJECT(FaultInjection::kFault_Send, return INET_ERROR_UNKNOWN_INTERFACE;);
    INET_FAULT_INJECT(FaultInjection::kFault_SendNonCritical, return CHIP_ERROR_NO_MEMORY;);

    // Do not allow sending an IPv4 address on an IPv6 end point and
    // vice versa.

    if (IPVer == kIPVersion_6 && addr.Type() != kIPAddressType_IPv6)
    {
        return INET_ERROR_WRONG_ADDRESS_TYPE;
    }
#if INET_CONFIG_ENABLE_IPV4
    if (IPVer == kIPVersion_4 && addr.Type() != kIPAddressType_IPv4)
    {
        return INET_ERROR_WRONG_ADDRESS_TYPE;
    }
#endif // INET_CONFIG_ENABLE_IPV4

#if CHIP_SYSTEM_CONFIG_USE_LWIP

    // Lock LwIP stack
    LOCK_TCPIP_CORE();

    // Make sure we have the appropriate type of PCB based on the destination address.
    res = GetPCB(addr.Type());
    SuccessOrExit(res);

    // Send the message to the specified address/port.
    {
        err_t lwipErr = ERR_VAL;

#if LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
        ip_addr_t ipAddr = addr.ToLwIPAddr();

        lwipErr = raw_sendto(mRaw, System::LwIPPacketBufferView::UnsafeGetLwIPpbuf(msg), &ipAddr);
#else // LWIP_VERSION_MAJOR <= 1 && LWIP_VERSION_MINOR < 5
        if (PCB_ISIPV6(mRaw))
        {
            ip6_addr_t ipv6Addr = addr.ToIPv6();

            lwipErr = raw_sendto_ip6(mRaw, System::LwIPPacketBufferView::UnsafeGetLwIPpbuf(msg), &ipv6Addr);
        }
#if INET_CONFIG_ENABLE_IPV4
        else
        {
            ip4_addr_t ipv4Addr = addr.ToIPv4();

            lwipErr = raw_sendto(mRaw, System::LwIPPacketBufferView::UnsafeGetLwIPpbuf(msg), &ipv4Addr);
        }
#endif // INET_CONFIG_ENABLE_IPV4
#endif // LWIP_VERSION_MAJOR <= 1 || LWIP_VERSION_MINOR >= 5

        if (lwipErr != ERR_OK)
            res = chip::System::MapErrorLwIP(lwipErr);
    }

    // Unlock LwIP stack
    UNLOCK_TCPIP_CORE();
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    // Make sure we have the appropriate type of socket based on the
    // destination address.

    res = GetSocket(addr.Type());
    SuccessOrExit(res);

    res = IPEndPointBasis::SendMsg(pktInfo, std::move(msg), sendFlags);
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

exit:
    CHIP_SYSTEM_FAULT_INJECT_ASYNC_EVENT();

    return res;
}

/**
 * @brief   Set the ICMP6 filter parameters in the network stack.
 *
 * @param[in]   numICMPTypes    length of array at \c aICMPTypes
 * @param[in]   aICMPTypes      the set of ICMPv6 type codes to filter.
 *
 * @retval  CHIP_NO_ERROR                   success: filter parameters set
 * @retval  CHIP_ERROR_NOT_IMPLEMENTED      system does not implement
 * @retval  INET_ERROR_WRONG_ADDRESS_TYPE   endpoint not IPv6 type
 * @retval  INET_ERROR_WRONG_PROTOCOL_TYPE  endpoint not ICMP6 type
 *
 * @retval  other                   another system or platform error
 *
 * @details
 *  Apply the ICMPv6 filtering parameters for the codes in \c aICMPTypes to
 *  the underlying endpoint in the system networking stack.
 */
CHIP_ERROR RawEndPoint::SetICMPFilter(uint8_t numICMPTypes, const uint8_t * aICMPTypes)
{
#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
#if !(HAVE_NETINET_ICMP6_H && HAVE_ICMP6_FILTER)
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif //!(HAVE_NETINET_ICMP6_H && HAVE_ICMP6_FILTER)
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

    VerifyOrReturnError(IPVer == kIPVersion_6, INET_ERROR_WRONG_ADDRESS_TYPE);
    VerifyOrReturnError(IPProto == kIPProtocol_ICMPv6, INET_ERROR_WRONG_PROTOCOL_TYPE);
    VerifyOrReturnError((numICMPTypes == 0 && aICMPTypes == nullptr) || (numICMPTypes != 0 && aICMPTypes != nullptr),
                        CHIP_ERROR_INVALID_ARGUMENT);

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    LOCK_TCPIP_CORE();
    NumICMPTypes = numICMPTypes;
    ICMPTypes    = aICMPTypes;
    UNLOCK_TCPIP_CORE();
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
#if HAVE_NETINET_ICMP6_H && HAVE_ICMP6_FILTER
    struct icmp6_filter filter;
    if (numICMPTypes > 0)
    {
        ICMP6_FILTER_SETBLOCKALL(&filter);
        for (int j = 0; j < numICMPTypes; ++j)
        {
            ICMP6_FILTER_SETPASS(aICMPTypes[j], &filter);
        }
    }
    else
    {
        ICMP6_FILTER_SETPASSALL(&filter);
    }
    if (setsockopt(mSocket.GetFD(), IPPROTO_ICMPV6, ICMP6_FILTER, &filter, sizeof(filter)) == -1)
    {
        return chip::System::MapErrorPOSIX(errno);
    }
#endif // HAVE_NETINET_ICMP6_H && HAVE_ICMP6_FILTER
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

    return CHIP_NO_ERROR;
}

/**
 * @brief   Bind the endpoint to a network interface.
 *
 * @param[in]   addrType    the protocol version of the IP address.
 *
 * @param[in]   intfId      indicator of the network interface.
 *
 * @retval  CHIP_NO_ERROR               success: endpoint bound to address
 * @retval  CHIP_ERROR_NO_MEMORY        insufficient memory for endpoint
 * @retval  CHIP_ERROR_NOT_IMPLEMENTED  system implementation not complete.
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
CHIP_ERROR RawEndPoint::BindInterface(IPAddressType addrType, InterfaceId intfId)
{
    // A lock is required because the LwIP thread may be referring to intf_filter,
    // while this code running in the Inet application is potentially modifying it.
    // NOTE: this only supports LwIP interfaces whose number is no bigger than 9.

    if (mState != kState_Ready && mState != kState_Bound)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    LOCK_TCPIP_CORE();

    // Make sure we have the appropriate type of PCB.
    CHIP_ERROR err = GetPCB(addrType);

    if (err == CHIP_NO_ERROR)
    {
        err = LwIPBindInterface(mRaw, intfId);
    }

    UNLOCK_TCPIP_CORE();

    ReturnErrorOnFailure(err);

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    // Make sure we have the appropriate type of socket.
    ReturnErrorOnFailure(GetSocket(addrType));
    ReturnErrorOnFailure(IPEndPointBasis::BindInterface(addrType, intfId));
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

    mState = kState_Bound;

    return CHIP_NO_ERROR;
}

void RawEndPoint::Init(InetLayer * inetLayer, IPVersion ipVer, IPProtocol ipProto)
{
    IPEndPointBasis::Init(inetLayer);

    IPVer   = ipVer;
    IPProto = ipProto;
}

/**
 * Get the bound interface on this endpoint.
 *
 * @return InterfaceId   The bound interface id.
 */
InterfaceId RawEndPoint::GetBoundInterface()
{
#if CHIP_SYSTEM_CONFIG_USE_LWIP
#if HAVE_LWIP_RAW_BIND_NETIF
    return netif_get_by_index(mRaw->netif_idx);
#else
    return mRaw->intf_filter;
#endif
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    return mBoundIntfId;
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS
}

#if CHIP_SYSTEM_CONFIG_USE_LWIP

void RawEndPoint::HandleDataReceived(System::PacketBufferHandle && msg)
{
    IPEndPointBasis::HandleDataReceived(std::move(msg));
}

CHIP_ERROR RawEndPoint::GetPCB(IPAddressType addrType)
{
    // IMPORTANT: This method MUST be called with the LwIP stack LOCKED!

#if LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
    if (mRaw == nullptr)
    {
        switch (addrType)
        {
        case kIPAddressType_IPv6:
#if INET_CONFIG_ENABLE_IPV4
        case kIPAddressType_IPv4:
#endif // INET_CONFIG_ENABLE_IPV4
            mRaw = raw_new_ip_type(IPAddress::ToLwIPAddrType(addrType), IPProto);
            break;

        default:
            return INET_ERROR_WRONG_ADDRESS_TYPE;
        }

        if (mRaw == NULL)
        {
            ChipLogError(Inet, "raw_new_ip_type failed");
            return CHIP_ERROR_NO_MEMORY;
        }

        mLwIPEndPointType = kLwIPEndPointType_Raw;
    }
    else
    {
        const lwip_ip_addr_type lLwIPAddrType = static_cast<lwip_ip_addr_type>(IP_GET_TYPE(&mRaw->local_ip));

        switch (lLwIPAddrType)
        {
        case IPADDR_TYPE_V6:
            VerifyOrReturnError(addrType == kIPAddressType_IPv6, INET_ERROR_WRONG_ADDRESS_TYPE);
            break;

#if INET_CONFIG_ENABLE_IPV4
        case IPADDR_TYPE_V4:
            VerifyOrReturnError(addrType == kIPAddressType_IPv4, INET_ERROR_WRONG_ADDRESS_TYPE);
            break;
#endif // INET_CONFIG_ENABLE_IPV4

        default:
            break;
        }
    }
#else // LWIP_VERSION_MAJOR <= 1 && LWIP_VERSION_MINOR < 5
    if (mRaw == NULL)
    {
        if (IPVer == kIPVersion_6)
        {
            mRaw = raw_new_ip6(IPProto);
            if (mRaw != NULL)
                ip_set_option(mRaw, SOF_REUSEADDR);
        }
#if INET_CONFIG_ENABLE_IPV4
        else if (IPVer == kIPVersion_4)
        {
            mRaw = raw_new(IPProto);
        }
#endif // INET_CONFIG_ENABLE_IPV4
        else
        {
            return INET_ERROR_WRONG_ADDRESS_TYPE;
        }

        if (mRaw == NULL)
        {
            ChipLogError(Inet, "raw_new failed");
            return CHIP_ERROR_NO_MEMORY;
        }

        mLwIPEndPointType = kLwIPEndPointType_Raw;
    }
    else
    {
#if INET_CONFIG_ENABLE_IPV4
        const IPAddressType pcbType = PCB_ISIPV6(mRaw) ? kIPAddressType_IPv6 : kIPAddressType_IPv4;
#else  // !INET_CONFIG_ENABLE_IPV4
        const IPAddressType pcbType = kIPAddressType_IPv6;
#endif // !INET_CONFIG_ENABLE_IPV4

        if (addrType != pcbType)
        {
            return INET_ERROR_WRONG_ADDRESS_TYPE;
        }
    }
#endif // LWIP_VERSION_MAJOR <= 1 || LWIP_VERSION_MINOR >= 5

    return CHIP_NO_ERROR;
}

/* This function is executed when a raw_pcb is listening and an IP datagram (v4 or v6) is received.
 * NOTE: currently ICMPv4 filtering is currently not implemented, but it can easily be added later.
 * This fn() may be executed concurrently with SetICMPFilter()
 * - this fn() runs in the LwIP thread (and the lock has already been taken)
 * - SetICMPFilter() runs in the Inet thread.
 * Returns non-zero if and only if ownership of the pbuf has been taken.
 */
#if LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
u8_t RawEndPoint::LwIPReceiveRawMessage(void * arg, struct raw_pcb * pcb, struct pbuf * p, const ip_addr_t * addr)
#else  // LWIP_VERSION_MAJOR <= 1 && LWIP_VERSION_MINOR < 5
u8_t RawEndPoint::LwIPReceiveRawMessage(void * arg, struct raw_pcb * pcb, struct pbuf * p, ip_addr_t * addr)
#endif // LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
{
    RawEndPoint * ep                   = static_cast<RawEndPoint *>(arg);
    chip::System::Layer & lSystemLayer = ep->SystemLayer();
    IPPacketInfo * pktInfo             = NULL;
    uint8_t enqueue                    = 1;
    System::PacketBufferHandle buf     = System::PacketBufferHandle::Adopt(p);

    // Filtering based on the saved ICMP6 types (the only protocol currently supported.)
    if ((ep->IPVer == kIPVersion_6) && (ep->IPProto == kIPProtocol_ICMPv6))
    {
        if (ep->NumICMPTypes > 0)
        { // When no filter is defined, let all ICMPv6 packets pass
          // The type is the first 8 bits field of an ICMP (v4 or v6) packet
            uint8_t icmp_type       = *(buf->Start() + ip_current_header_tot_len());
            uint8_t icmp_type_found = 0;
            for (int j = 0; j < ep->NumICMPTypes; ++j)
            {
                if (ep->ICMPTypes[j] == icmp_type)
                {
                    icmp_type_found = 1;
                    break;
                }
            }
            if (!icmp_type_found)
            {
                enqueue = 0; // do not eat it
            }
        }
    }

    if (enqueue)
    {
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
            pktInfo->SrcPort   = 0;
            pktInfo->DestPort  = 0;
        }

        PostPacketBufferEvent(lSystemLayer, *ep, kInetEvent_RawDataReceived, std::move(buf));
    }

    return enqueue;
}

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
CHIP_ERROR RawEndPoint::GetSocket(IPAddressType aAddressType)
{
    constexpr int lType = (SOCK_RAW | SOCK_FLAGS);
    int lProtocol;

    switch (aAddressType)
    {
    case kIPAddressType_IPv6:
        lProtocol = IPPROTO_ICMPV6;
        break;

#if INET_CONFIG_ENABLE_IPV4
    case kIPAddressType_IPv4:
        lProtocol = IPPROTO_ICMP;
        break;
#endif // INET_CONFIG_ENABLE_IPV4

    default:
        return INET_ERROR_WRONG_ADDRESS_TYPE;
    }

    return IPEndPointBasis::GetSocket(aAddressType, lType, lProtocol);
}

// static
void RawEndPoint::HandlePendingIO(System::WatchableSocket & socket)
{
    reinterpret_cast<RawEndPoint *>(socket.GetCallbackData())->HandlePendingIO();
}

void RawEndPoint::HandlePendingIO()
{
    if (mState == kState_Listening && OnMessageReceived != nullptr && mSocket.HasPendingRead())
    {
        const uint16_t lPort = 0;

        IPEndPointBasis::HandlePendingIO(lPort);
    }

    mSocket.ClearPendingIO();
}

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

} // namespace Inet
} // namespace chip
