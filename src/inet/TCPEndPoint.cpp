/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 *      This file implements the <tt>Inet::TCPEndPoint</tt> class,
 *      where the CHIP Inet Layer encapsulates methods for interacting
 *      with TCP transport endpoints (SOCK_DGRAM sockets on Linux and
 *      BSD-derived systems) or LwIP TCP protocol control blocks, as
 *      the system is configured accordingly.
 *
 */

#define __APPLE_USE_RFC_3542

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include "TCPEndPoint.h"

#include "InetFaultInjection.h"
#include <inet/InetLayer.h>

#include <support/CodeUtils.h>
#include <support/SafeInt.h>
#include <support/logging/CHIPLogging.h>
#include <system/SystemFaultInjection.h>

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#include <lwip/tcp.h>
#include <lwip/tcpip.h>
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

#include "arpa-inet-compatibility.h"

#include <stdio.h>
#include <string.h>
#include <utility>

// SOCK_CLOEXEC not defined on all platforms, e.g. iOS/macOS:
#ifdef SOCK_CLOEXEC
#define SOCK_FLAGS SOCK_CLOEXEC
#else
#define SOCK_FLAGS 0
#endif

#if defined(SOL_TCP)
// socket option level for Linux and BSD systems.
#define TCP_SOCKOPT_LEVEL SOL_TCP
#else
// socket option level for macOS & iOS systems.
#define TCP_SOCKOPT_LEVEL IPPROTO_TCP
#endif

#if defined(TCP_KEEPIDLE)
// socket option for Linux and BSD systems.
#define TCP_IDLE_INTERVAL_OPT_NAME TCP_KEEPIDLE
#else
// socket option for macOS & iOS systems.
#define TCP_IDLE_INTERVAL_OPT_NAME TCP_KEEPALIVE
#endif

/*
 * This logic to register a null operation callback with the LwIP TCP/IP task
 * ensures that the TCP timer loop is started when a connection is established,
 * which is necessary to ensure that initial SYN and SYN-ACK packets are
 * retransmitted during the 3-way handshake.
 */
#if CHIP_SYSTEM_CONFIG_USE_LWIP
namespace {

void nil_tcpip_callback(void * _aContext) {}

err_t start_tcp_timers(void)
{
    return tcpip_callback(nil_tcpip_callback, NULL);
}

} // anonymous namespace
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

namespace chip {
namespace Inet {

chip::System::ObjectPool<TCPEndPoint, INET_CONFIG_NUM_TCP_ENDPOINTS> TCPEndPoint::sPool;

CHIP_ERROR TCPEndPoint::Bind(IPAddressType addrType, const IPAddress & addr, uint16_t port, bool reuseAddr)
{
    CHIP_ERROR res = CHIP_NO_ERROR;

    if (State != kState_Ready)
        return CHIP_ERROR_INCORRECT_STATE;

    if (addr != IPAddress::Any && addr.Type() != kIPAddressType_Any && addr.Type() != addrType)
        return INET_ERROR_WRONG_ADDRESS_TYPE;

#if CHIP_SYSTEM_CONFIG_USE_LWIP

    // Lock LwIP stack
    LOCK_TCPIP_CORE();

    // Get the appropriate type of PCB.
    res = GetPCB(addrType);

    // Bind the PCB to the specified address/port.
    if (res == CHIP_NO_ERROR)
    {
        if (reuseAddr)
        {
            ip_set_option(mTCP, SOF_REUSEADDR);
        }

#if LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5

        ip_addr_t ipAddr;
        if (addr != IPAddress::Any)
        {
            ipAddr = addr.ToLwIPAddr();
        }
        else if (addrType == kIPAddressType_IPv6)
        {
            ipAddr = ip6_addr_any;
        }
#if INET_CONFIG_ENABLE_IPV4
        else if (addrType == kIPAddressType_IPv4)
        {
            ipAddr = ip_addr_any;
        }
#endif // INET_CONFIG_ENABLE_IPV4
        else
            res = INET_ERROR_WRONG_ADDRESS_TYPE;
        res = chip::System::MapErrorLwIP(tcp_bind(mTCP, &ipAddr, port));

#else // LWIP_VERSION_MAJOR <= 1 || LWIP_VERSION_MINOR >= 5

        if (addrType == kIPAddressType_IPv6)
        {
            ip6_addr_t ipv6Addr = addr.ToIPv6();
            res                 = chip::System::MapErrorLwIP(tcp_bind_ip6(mTCP, &ipv6Addr, port));
        }
#if INET_CONFIG_ENABLE_IPV4
        else if (addrType == kIPAddressType_IPv4)
        {
            ip_addr_t ipv4Addr = addr.ToIPv4();
            res                = chip::System::MapErrorLwIP(tcp_bind(mTCP, &ipv4Addr, port));
        }
#endif // INET_CONFIG_ENABLE_IPV4
        else
            res = INET_ERROR_WRONG_ADDRESS_TYPE;

#endif // LWIP_VERSION_MAJOR <= 1 || LWIP_VERSION_MINOR >= 5
    }

    // Unlock LwIP stack
    UNLOCK_TCPIP_CORE();

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS

    res = GetSocket(addrType);

    if (res == CHIP_NO_ERROR && reuseAddr)
    {
        int n = 1;
        setsockopt(mSocket.GetFD(), SOL_SOCKET, SO_REUSEADDR, &n, sizeof(n));

#ifdef SO_REUSEPORT
        // Enable SO_REUSEPORT.  This permits coexistence between an
        // untargetted CHIP client and other services that listen on
        // a CHIP port on a specific address (such as a CHIP client
        // with TARGETTED_LISTEN or TCP proxying services).  Note that
        // one of the costs of this implementation is the
        // non-deterministic connection dispatch when multple clients
        // listen on the address wih the same degreee of selectivity,
        // e.g. two untargetted-listen CHIP clients, or two
        // targetted-listen CHIP clients with the same node id.

        if (setsockopt(mSocket.GetFD(), SOL_SOCKET, SO_REUSEPORT, &n, sizeof(n)) != 0)
        {
            ChipLogError(Inet, "SO_REUSEPORT: %d", errno);
        }
#endif // defined(SO_REUSEPORT)
    }

    if (res == CHIP_NO_ERROR)
    {
        if (addrType == kIPAddressType_IPv6)
        {
            struct sockaddr_in6 sa;
            memset(&sa, 0, sizeof(sa));
            sa.sin6_family   = AF_INET6;
            sa.sin6_port     = htons(port);
            sa.sin6_flowinfo = 0;
            sa.sin6_addr     = addr.ToIPv6();
            sa.sin6_scope_id = 0;

            if (bind(mSocket.GetFD(), reinterpret_cast<const sockaddr *>(&sa), static_cast<unsigned>(sizeof(sa))) != 0)
                res = chip::System::MapErrorPOSIX(errno);
        }
#if INET_CONFIG_ENABLE_IPV4
        else if (addrType == kIPAddressType_IPv4)
        {
            struct sockaddr_in sa;
            memset(&sa, 0, sizeof(sa));
            sa.sin_family = AF_INET;
            sa.sin_port   = htons(port);
            sa.sin_addr   = addr.ToIPv4();

            if (bind(mSocket.GetFD(), reinterpret_cast<const sockaddr *>(&sa), static_cast<unsigned>(sizeof(sa))) != 0)
                res = chip::System::MapErrorPOSIX(errno);
        }
#endif // INET_CONFIG_ENABLE_IPV4
        else
            res = INET_ERROR_WRONG_ADDRESS_TYPE;
    }

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
    dispatch_queue_t dispatchQueue = SystemLayer().GetDispatchQueue();
    if (dispatchQueue != nullptr)
    {
        unsigned long fd = static_cast<unsigned long>(mSocket.GetFD());

        mReadableSource = dispatch_source_create(DISPATCH_SOURCE_TYPE_READ, fd, 0, dispatchQueue);
        ReturnErrorCodeIf(mReadableSource == nullptr, CHIP_ERROR_NO_MEMORY);

        mWriteableSource = dispatch_source_create(DISPATCH_SOURCE_TYPE_WRITE, fd, 0, dispatchQueue);
        ReturnErrorCodeIf(mWriteableSource == nullptr, CHIP_ERROR_NO_MEMORY);

        dispatch_source_set_event_handler(mReadableSource, ^{
            this->mSocket.SetPendingIO(System::SocketEventFlags::kRead);
            this->HandlePendingIO();
        });

        dispatch_source_set_event_handler(mWriteableSource, ^{
            this->mSocket.SetPendingIO(System::SocketEventFlags::kWrite);
            this->HandlePendingIO();
        });

        dispatch_resume(mReadableSource);
        dispatch_resume(mWriteableSource);
    }
#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

    if (res == CHIP_NO_ERROR)
    {
        State = kState_Bound;
    }

    return res;
}

CHIP_ERROR TCPEndPoint::Listen(uint16_t backlog)
{
    CHIP_ERROR res = CHIP_NO_ERROR;

    if (State != kState_Bound)
        return CHIP_ERROR_INCORRECT_STATE;

#if CHIP_SYSTEM_CONFIG_USE_LWIP

    // Start listening for incoming connections.
    mTCP              = tcp_listen(mTCP);
    mLwIPEndPointType = kLwIPEndPointType_TCP;

    tcp_arg(mTCP, this);

    tcp_accept(mTCP, LwIPHandleIncomingConnection);

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS

    if (listen(mSocket.GetFD(), backlog) != 0)
        res = chip::System::MapErrorPOSIX(errno);

    // Wait for ability to read on this endpoint.
    mSocket.SetCallback(HandlePendingIO, reinterpret_cast<intptr_t>(this));
    mSocket.RequestCallbackOnPendingRead();

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

    if (res == CHIP_NO_ERROR)
    {
        // Once Listening, bump the reference count.  The corresponding call to Release()
        // [or on LwIP, DeferredRelease()] will happen in DoClose().
        Retain();
        State = kState_Listening;
    }

    return res;
}

CHIP_ERROR TCPEndPoint::Connect(const IPAddress & addr, uint16_t port, InterfaceId intfId)
{
    CHIP_ERROR res = CHIP_NO_ERROR;

    if (State != kState_Ready && State != kState_Bound)
        return CHIP_ERROR_INCORRECT_STATE;

    IPAddressType addrType = addr.Type();

#if CHIP_SYSTEM_CONFIG_USE_LWIP

    // LwIP does not provides an API for initiating a TCP connection via a specific interface.
    // As a work-around, if the destination is an IPv6 link-local address, we bind the PCB
    // to the link local address associated with the source interface; however this is only
    // viable if the endpoint hasn't already been bound.
    if (intfId != INET_NULL_INTERFACEID)
    {
        IPAddress intfLLAddr;
        InetLayer & lInetLayer = Layer();

        if (!addr.IsIPv6LinkLocal() || State == kState_Bound)
            return CHIP_ERROR_NOT_IMPLEMENTED;

        res = lInetLayer.GetLinkLocalAddr(intfId, &intfLLAddr);
        if (res != CHIP_NO_ERROR)
            return res;

        res = Bind(kIPAddressType_IPv6, intfLLAddr, 0, true);
        if (res != CHIP_NO_ERROR)
            return res;
    }

    // Lock LwIP stack
    LOCK_TCPIP_CORE();

    res = GetPCB(addrType);

    if (res == CHIP_NO_ERROR)
    {
        tcp_arg(mTCP, this);
        tcp_err(mTCP, LwIPHandleError);

#if LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
        ip_addr_t lwipAddr = addr.ToLwIPAddr();
        res                = chip::System::MapErrorLwIP(tcp_connect(mTCP, &lwipAddr, port, LwIPHandleConnectComplete));
#else // LWIP_VERSION_MAJOR <= 1 || LWIP_VERSION_MINOR >= 5
        if (addrType == kIPAddressType_IPv6)
        {
            ip6_addr_t lwipAddr = addr.ToIPv6();
            res                 = chip::System::MapErrorLwIP(tcp_connect_ip6(mTCP, &lwipAddr, port, LwIPHandleConnectComplete));
        }
#if INET_CONFIG_ENABLE_IPV4
        else if (addrType == kIPAddressType_IPv4)
        {
            ip_addr_t lwipAddr = addr.ToIPv4();
            res                = chip::System::MapErrorLwIP(tcp_connect(mTCP, &lwipAddr, port, LwIPHandleConnectComplete));
        }
#endif // INET_CONFIG_ENABLE_IPV4
        else
            res = INET_ERROR_WRONG_ADDRESS_TYPE;
#endif // LWIP_VERSION_MAJOR <= 1 || LWIP_VERSION_MINOR >= 5

        // Ensure that TCP timers are started
        if (res == CHIP_NO_ERROR)
        {
            res = start_tcp_timers();
        }

        if (res == CHIP_NO_ERROR)
        {
            State = kState_Connecting;
            Retain();
        }
    }

    // Unlock LwIP stack
    UNLOCK_TCPIP_CORE();

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS

    res = GetSocket(addrType);
    if (res != CHIP_NO_ERROR)
        return res;

    if (intfId == INET_NULL_INTERFACEID)
    {
        // The behavior when connecting to an IPv6 link-local address without specifying an outbound
        // interface is ambiguous. So prevent it in all cases.
        if (addr.IsIPv6LinkLocal())
            return INET_ERROR_WRONG_ADDRESS_TYPE;
    }
    else
    {
        // Try binding to the interface

        // If destination is link-local then there is no need to bind to
        // interface or address on the interface.

        if (!addr.IsIPv6LinkLocal())
        {
#ifdef SO_BINDTODEVICE
            struct ::ifreq ifr;
            memset(&ifr, 0, sizeof(ifr));

            res = GetInterfaceName(intfId, ifr.ifr_name, sizeof(ifr.ifr_name));
            if (res != CHIP_NO_ERROR)
                return res;

            // Attempt to bind to the interface using SO_BINDTODEVICE which requires privileged access.
            // If the permission is denied(EACCES) because CHIP is running in a context
            // that does not have privileged access, choose a source address on the
            // interface to bind the connetion to.
            int r = setsockopt(mSocket.GetFD(), SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof(ifr));
            if (r < 0 && errno != EACCES)
            {
                return res = chip::System::MapErrorPOSIX(errno);
            }

            if (r < 0)
#endif // SO_BINDTODEVICE
            {
                // Attempting to initiate a connection via a specific interface is not allowed.
                // The only way to do this is to bind the local to an address on the desired
                // interface.
                res = BindSrcAddrFromIntf(addrType, intfId);
                if (res != CHIP_NO_ERROR)
                    return res;
            }
        }
    }

    // Disable generation of SIGPIPE.
#ifdef SO_NOSIGPIPE
    int n = 1;
    setsockopt(mSocket.GetFD(), SOL_SOCKET, SO_NOSIGPIPE, &n, sizeof(n));
#endif // defined(SO_NOSIGPIPE)

    // Enable non-blocking mode for the socket.
    int flags = fcntl(mSocket.GetFD(), F_GETFL, 0);
    fcntl(mSocket.GetFD(), F_SETFL, flags | O_NONBLOCK);

    socklen_t sockaddrsize       = 0;
    const sockaddr * sockaddrptr = nullptr;

    union
    {
        sockaddr any;
        sockaddr_in6 in6;
#if INET_CONFIG_ENABLE_IPV4
        sockaddr_in in;
#endif // INET_CONFIG_ENABLE_IPV4
    } sa;
    memset(&sa, 0, sizeof(sa));

    if (addrType == kIPAddressType_IPv6)
    {
        sa.in6.sin6_family   = AF_INET6;
        sa.in6.sin6_port     = htons(port);
        sa.in6.sin6_flowinfo = 0;
        sa.in6.sin6_addr     = addr.ToIPv6();
        sa.in6.sin6_scope_id = intfId;
        sockaddrsize         = sizeof(sockaddr_in6);
        sockaddrptr          = reinterpret_cast<const sockaddr *>(&sa.in6);
    }
#if INET_CONFIG_ENABLE_IPV4
    else if (addrType == kIPAddressType_IPv4)
    {
        sa.in.sin_family = AF_INET;
        sa.in.sin_port   = htons(port);
        sa.in.sin_addr   = addr.ToIPv4();
        sockaddrsize     = sizeof(sockaddr_in);
        sockaddrptr      = reinterpret_cast<const sockaddr *>(&sa.in);
    }
#endif // INET_CONFIG_ENABLE_IPV4
    else
        return INET_ERROR_WRONG_ADDRESS_TYPE;

    int conRes = connect(mSocket.GetFD(), sockaddrptr, sockaddrsize);

    if (conRes == -1 && errno != EINPROGRESS)
    {
        res = chip::System::MapErrorPOSIX(errno);
        DoClose(res, true);
        return res;
    }

    mSocket.SetCallback(HandlePendingIO, reinterpret_cast<intptr_t>(this));

    // Once Connecting or Connected, bump the reference count.  The corresponding Release()
    // [or on LwIP, DeferredRelease()] will happen in DoClose().
    Retain();

    if (conRes == 0)
    {
        State = kState_Connected;
        // Wait for ability to read on this endpoint.
        mSocket.RequestCallbackOnPendingRead();
        if (OnConnectComplete != nullptr)
            OnConnectComplete(this, CHIP_NO_ERROR);
    }
    else
    {
        State = kState_Connecting;
        // Wait for ability to write on this endpoint.
        mSocket.RequestCallbackOnPendingWrite();
    }

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

    StartConnectTimerIfSet();

    return res;
}

/**
 * @brief   Set timeout for Connect to succeed or return an error.
 *
 * @param[in]   connTimeoutMsecs
 *
 * @note
 *  Setting a value of zero means use system defaults.
 */
void TCPEndPoint::SetConnectTimeout(const uint32_t connTimeoutMsecs)
{
    mConnectTimeoutMsecs = connTimeoutMsecs;
}

void TCPEndPoint::StartConnectTimerIfSet()
{
    if (mConnectTimeoutMsecs > 0)
    {
        chip::System::Layer & lSystemLayer = SystemLayer();

        lSystemLayer.StartTimer(mConnectTimeoutMsecs, TCPConnectTimeoutHandler, this);
    }
}

void TCPEndPoint::StopConnectTimer()
{
    chip::System::Layer & lSystemLayer = SystemLayer();

    lSystemLayer.CancelTimer(TCPConnectTimeoutHandler, this);
}

void TCPEndPoint::TCPConnectTimeoutHandler(chip::System::Layer * aSystemLayer, void * aAppState, CHIP_ERROR aError)
{
    TCPEndPoint * tcpEndPoint = reinterpret_cast<TCPEndPoint *>(aAppState);

    VerifyOrDie((aSystemLayer != nullptr) && (tcpEndPoint != nullptr));

    // Close Connection as we have timed out and Connect has not returned to
    // stop this timer.
    tcpEndPoint->DoClose(INET_ERROR_TCP_CONNECT_TIMEOUT, false);
}

CHIP_ERROR TCPEndPoint::GetPeerInfo(IPAddress * retAddr, uint16_t * retPort) const
{
    CHIP_ERROR res = CHIP_NO_ERROR;

    if (!IsConnected())
        return CHIP_ERROR_INCORRECT_STATE;

#if CHIP_SYSTEM_CONFIG_USE_LWIP

    // Lock LwIP stack
    LOCK_TCPIP_CORE();

    if (mTCP != NULL)
    {
        *retPort = mTCP->remote_port;

#if LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
        *retAddr = IPAddress::FromLwIPAddr(mTCP->remote_ip);
#else // LWIP_VERSION_MAJOR <= 1 || LWIP_VERSION_MINOR >= 5
#if INET_CONFIG_ENABLE_IPV4
        *retAddr = PCB_ISIPV6(mTCP) ? IPAddress::FromIPv6(mTCP->remote_ip.ip6) : IPAddress::FromIPv4(mTCP->remote_ip.ip4);
#else  // !INET_CONFIG_ENABLE_IPV4
        *retAddr                    = IPAddress::FromIPv6(mTCP->remote_ip.ip6);
#endif // !INET_CONFIG_ENABLE_IPV4
#endif // LWIP_VERSION_MAJOR <= 1 || LWIP_VERSION_MINOR >= 5
    }
    else
        res = CHIP_ERROR_CONNECTION_ABORTED;

    // Unlock LwIP stack
    UNLOCK_TCPIP_CORE();

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS

    union
    {
        sockaddr any;
        sockaddr_in in;
        sockaddr_in6 in6;
    } sa;
    memset(&sa, 0, sizeof(sa));
    socklen_t saLen = sizeof(sa);

    if (getpeername(mSocket.GetFD(), &sa.any, &saLen) != 0)
        return chip::System::MapErrorPOSIX(errno);

    if (sa.any.sa_family == AF_INET6)
    {
        *retAddr = IPAddress::FromIPv6(sa.in6.sin6_addr);
        *retPort = ntohs(sa.in6.sin6_port);
    }
#if INET_CONFIG_ENABLE_IPV4
    else if (sa.any.sa_family == AF_INET)
    {
        *retAddr = IPAddress::FromIPv4(sa.in.sin_addr);
        *retPort = ntohs(sa.in.sin_port);
    }
#endif // INET_CONFIG_ENABLE_IPV4
    else
        return CHIP_ERROR_INCORRECT_STATE;

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

    return res;
}

CHIP_ERROR TCPEndPoint::GetLocalInfo(IPAddress * retAddr, uint16_t * retPort)
{
    CHIP_ERROR res = CHIP_NO_ERROR;

    if (!IsConnected())
        return CHIP_ERROR_INCORRECT_STATE;

#if CHIP_SYSTEM_CONFIG_USE_LWIP

    // Lock LwIP stack
    LOCK_TCPIP_CORE();

    if (mTCP != NULL)
    {
        *retPort = mTCP->local_port;

#if LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
        *retAddr = IPAddress::FromLwIPAddr(mTCP->local_ip);
#else // LWIP_VERSION_MAJOR <= 1 || LWIP_VERSION_MINOR >= 5
#if INET_CONFIG_ENABLE_IPV4
        *retAddr = PCB_ISIPV6(mTCP) ? IPAddress::FromIPv6(mTCP->local_ip.ip6) : IPAddress::FromIPv4(mTCP->local_ip.ip4);
#else  // !INET_CONFIG_ENABLE_IPV4
        *retAddr                    = IPAddress::FromIPv6(mTCP->local_ip.ip6);
#endif // !INET_CONFIG_ENABLE_IPV4
#endif // LWIP_VERSION_MAJOR <= 1 || LWIP_VERSION_MINOR >= 5
    }
    else
        res = CHIP_ERROR_CONNECTION_ABORTED;

    // Unlock LwIP stack
    UNLOCK_TCPIP_CORE();

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS

    union
    {
        sockaddr any;
        sockaddr_in6 in6;
#if INET_CONFIG_ENABLE_IPV4
        sockaddr_in in;
#endif // INET_CONFIG_ENABLE_IPV4
    } sa;

    memset(&sa, 0, sizeof(sa));
    socklen_t saLen = sizeof(sa);

    if (getsockname(mSocket.GetFD(), &sa.any, &saLen) != 0)
        return chip::System::MapErrorPOSIX(errno);

    if (sa.any.sa_family == AF_INET6)
    {
        *retAddr = IPAddress::FromIPv6(sa.in6.sin6_addr);
        *retPort = ntohs(sa.in6.sin6_port);
    }
#if INET_CONFIG_ENABLE_IPV4
    else if (sa.any.sa_family == AF_INET)
    {
        *retAddr = IPAddress::FromIPv4(sa.in.sin_addr);
        *retPort = ntohs(sa.in.sin_port);
    }
#endif // INET_CONFIG_ENABLE_IPV4
    else
        return CHIP_ERROR_INCORRECT_STATE;

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

    return res;
}

CHIP_ERROR TCPEndPoint::GetInterfaceId(InterfaceId * retInterface)
{
    if (!IsConnected())
        return CHIP_ERROR_INCORRECT_STATE;

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    // TODO: Does netif_get_by_index(mTCP->netif_idx) do the right thing?  I
    // can't quite tell whether LwIP supports a specific interface id for TCP at
    // all.  For now just claim no particular interface id.
    *retInterface = INET_NULL_INTERFACEID;
    return CHIP_NO_ERROR;
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    union
    {
        sockaddr any;
        sockaddr_in6 in6;
#if INET_CONFIG_ENABLE_IPV4
        sockaddr_in in;
#endif // INET_CONFIG_ENABLE_IPV4
    } sa;

    memset(&sa, 0, sizeof(sa));
    socklen_t saLen = sizeof(sa);

    if (getpeername(mSocket.GetFD(), &sa.any, &saLen) != 0)
    {
        return chip::System::MapErrorPOSIX(errno);
    }

    if (sa.any.sa_family == AF_INET6)
    {
        if (IPAddress::FromIPv6(sa.in6.sin6_addr).IsIPv6LinkLocal())
        {
            *retInterface = sa.in6.sin6_scope_id;
        }
        else
        {
            // TODO: Is there still a meaningful interface id in this case?
            *retInterface = INET_NULL_INTERFACEID;
        }
        return CHIP_NO_ERROR;
    }

#if INET_CONFIG_ENABLE_IPV4
    if (sa.any.sa_family == AF_INET)
    {
        // No interface id available for IPv4 sockets.
        *retInterface = INET_NULL_INTERFACEID;
    }
#endif // INET_CONFIG_ENABLE_IPV4

    return CHIP_ERROR_INCORRECT_STATE;

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

    *retInterface = INET_NULL_INTERFACEID;
    return CHIP_NO_ERROR;
}

CHIP_ERROR TCPEndPoint::Send(System::PacketBufferHandle && data, bool push)
{
    CHIP_ERROR res = CHIP_NO_ERROR;

    if (State != kState_Connected && State != kState_ReceiveShutdown)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    if (mSendQueue.IsNull())
    {
        mSendQueue = std::move(data);
#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
        // Wait for ability to write on this endpoint.
        mSocket.RequestCallbackOnPendingWrite();
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS
    }
    else
    {
        mSendQueue->AddToEnd(std::move(data));
    }

#if CHIP_SYSTEM_CONFIG_USE_LWIP

#if INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT
    if (!mUserTimeoutTimerRunning)
    {
        // Timer was not running before this send. So, start
        // the timer.

        StartTCPUserTimeoutTimer();
    }
#endif // INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

    if (push)
        res = DriveSending();

    return res;
}

void TCPEndPoint::DisableReceive()
{
    ReceiveEnabled = false;
}

void TCPEndPoint::EnableReceive()
{
    ReceiveEnabled = true;

    DriveReceiving();

#if CHIP_SYSTEM_CONFIG_USE_IO_THREAD
    // Wake the thread waiting for I/O so that it can include the socket.
    SystemLayer().WakeIOThread();
#endif // CHIP_SYSTEM_CONFIG_USE_IO_THREAD
}

/**
 *  TCPEndPoint::EnableNoDelay
 *
 *  @brief
 *    Switch off nagle buffering algorithm in TCP by setting the
 *    TCP_NODELAY socket options.
 *
 */

CHIP_ERROR TCPEndPoint::EnableNoDelay()
{
    CHIP_ERROR res = CHIP_NO_ERROR;

    if (!IsConnected())
        return CHIP_ERROR_INCORRECT_STATE;

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    // Lock LwIP stack
    LOCK_TCPIP_CORE();

    if (mTCP != NULL)
        tcp_nagle_disable(mTCP);
    else
        res = CHIP_ERROR_CONNECTION_ABORTED;

    // Unlock LwIP stack
    UNLOCK_TCPIP_CORE();

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    {
        int val;

#ifdef TCP_NODELAY
        // Disable TCP Nagle buffering by setting TCP_NODELAY socket option to true
        val = 1;
        if (setsockopt(mSocket.GetFD(), TCP_SOCKOPT_LEVEL, TCP_NODELAY, &val, sizeof(val)) != 0)
            return chip::System::MapErrorPOSIX(errno);
#endif // defined(TCP_NODELAY)
    }

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

    return res;
}

CHIP_ERROR TCPEndPoint::EnableKeepAlive(uint16_t interval, uint16_t timeoutCount)
{
    CHIP_ERROR res = CHIP_NO_ERROR;

    if (!IsConnected())
        return CHIP_ERROR_INCORRECT_STATE;

#if CHIP_SYSTEM_CONFIG_USE_LWIP

#if LWIP_TCP_KEEPALIVE

    // Lock LwIP stack
    LOCK_TCPIP_CORE();

    if (mTCP != NULL)
    {
        // Set the idle interval
        mTCP->keep_idle = (uint32_t) interval * 1000;

        // Set the probe retransmission interval.
        mTCP->keep_intvl = (uint32_t) interval * 1000;

        // Set the probe timeout count
        mTCP->keep_cnt = timeoutCount;

        // Enable keepalives for the connection.
        ip_set_option(mTCP, SOF_KEEPALIVE);
    }
    else
        res = CHIP_ERROR_CONNECTION_ABORTED;

    // Unlock LwIP stack
    UNLOCK_TCPIP_CORE();

#else // LWIP_TCP_KEEPALIVE

    res = CHIP_ERROR_NOT_IMPLEMENTED;

#endif // LWIP_TCP_KEEPALIVE

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    {
        int val;

        // Set the idle interval
        val = interval;
        if (setsockopt(mSocket.GetFD(), TCP_SOCKOPT_LEVEL, TCP_IDLE_INTERVAL_OPT_NAME, &val, sizeof(val)) != 0)
            return chip::System::MapErrorPOSIX(errno);

        // Set the probe retransmission interval.
        val = interval;
        if (setsockopt(mSocket.GetFD(), TCP_SOCKOPT_LEVEL, TCP_KEEPINTVL, &val, sizeof(val)) != 0)
            return chip::System::MapErrorPOSIX(errno);

        // Set the probe timeout count
        val = timeoutCount;
        if (setsockopt(mSocket.GetFD(), TCP_SOCKOPT_LEVEL, TCP_KEEPCNT, &val, sizeof(val)) != 0)
            return chip::System::MapErrorPOSIX(errno);

        // Enable keepalives for the connection.
        val = 1; // enable
        if (setsockopt(mSocket.GetFD(), SOL_SOCKET, SO_KEEPALIVE, &val, sizeof(val)) != 0)
            return chip::System::MapErrorPOSIX(errno);
    }

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

    return res;
}

/**
 *  TCPEndPoint::DisableKeepAlive
 *
 *  @brief
 *    Disable TCP keepalive probes on the associated TCP connection.
 *
 *  @note
 *    This method can only be called when the endpoint is in one of the connected states.
 *
 *    This method does nothing if keepalives have not been enabled on the endpoint.
 */

CHIP_ERROR TCPEndPoint::DisableKeepAlive()
{
    CHIP_ERROR res = CHIP_NO_ERROR;

    if (!IsConnected())
        return CHIP_ERROR_INCORRECT_STATE;

#if CHIP_SYSTEM_CONFIG_USE_LWIP

#if LWIP_TCP_KEEPALIVE

    // Lock LwIP stack
    LOCK_TCPIP_CORE();

    if (mTCP != NULL)
    {
        // Disable keepalives on the connection.
        ip_reset_option(mTCP, SOF_KEEPALIVE);
    }
    else
        res = CHIP_ERROR_CONNECTION_ABORTED;

    // Unlock LwIP stack
    UNLOCK_TCPIP_CORE();

#else // LWIP_TCP_KEEPALIVE

    res = CHIP_ERROR_NOT_IMPLEMENTED;

#endif // LWIP_TCP_KEEPALIVE

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS

    {
        int val;

        // Disable keepalives on the connection.
        val = 0; // disable
        if (setsockopt(mSocket.GetFD(), SOL_SOCKET, SO_KEEPALIVE, &val, sizeof(val)) != 0)
            return chip::System::MapErrorPOSIX(errno);
    }

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

    return res;
}

/**
 *  TCPEndPoint::SetUserTimeout
 *
 *  @brief   Set the TCP user timeout socket option.
 *
 *  @details
 *    When the value is greater than 0, it specifies the maximum amount of
 *    time in milliseconds that transmitted data may remain
 *    unacknowledged before TCP will forcibly close the
 *    corresponding connection. If the option value is specified as 0,
 *    TCP will use the system default.
 *    See RFC 5482, for further details.
 *
 *  @note
 *    This method can only be called when the endpoint is in one of the connected states.
 *
 *    This method can be called multiple times to adjust the keepalive interval or timeout
 *    count.
 */
CHIP_ERROR TCPEndPoint::SetUserTimeout(uint32_t userTimeoutMillis)
{
    CHIP_ERROR res = CHIP_NO_ERROR;

    if (!IsConnected())
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

#if INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT

    // Store the User timeout configuration if it is being overridden.

    mUserTimeoutMillis = userTimeoutMillis;

#else // !INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS

#if defined(TCP_USER_TIMEOUT)
    // Set the user timeout
    uint32_t val = userTimeoutMillis;
    if (setsockopt(mSocket.GetFD(), TCP_SOCKOPT_LEVEL, TCP_USER_TIMEOUT, &val, sizeof(val)) != 0)
        return chip::System::MapErrorPOSIX(errno);
#else  // TCP_USER_TIMEOUT
    res = CHIP_ERROR_NOT_IMPLEMENTED;
#endif // defined(TCP_USER_TIMEOUT)

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    res = CHIP_ERROR_NOT_IMPLEMENTED;
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#endif // !INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT

    return res;
}

CHIP_ERROR TCPEndPoint::AckReceive(uint16_t len)
{
    CHIP_ERROR res = CHIP_NO_ERROR;

    if (!IsConnected())
        return CHIP_ERROR_INCORRECT_STATE;

#if CHIP_SYSTEM_CONFIG_USE_LWIP

    // Lock LwIP stack
    LOCK_TCPIP_CORE();

    if (mTCP != NULL)
        tcp_recved(mTCP, len);
    else
        res = CHIP_ERROR_CONNECTION_ABORTED;

    // Unlock LwIP stack
    UNLOCK_TCPIP_CORE();

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS

    // nothing to do for sockets case

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

    return res;
}

CHIP_ERROR TCPEndPoint::SetReceivedDataForTesting(System::PacketBufferHandle && data)
{
    if (!IsConnected())
        return CHIP_ERROR_INCORRECT_STATE;

    mRcvQueue = std::move(data);

    return CHIP_NO_ERROR;
}

uint32_t TCPEndPoint::PendingSendLength()
{
    if (!mSendQueue.IsNull())
        return mSendQueue->TotalLength();
    return 0;
}

uint32_t TCPEndPoint::PendingReceiveLength()
{
    if (!mRcvQueue.IsNull())
        return mRcvQueue->TotalLength();
    return 0;
}

CHIP_ERROR TCPEndPoint::Shutdown()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (!IsConnected())
        return CHIP_ERROR_INCORRECT_STATE;

    // If fully connected, enter the SendShutdown state.
    if (State == kState_Connected)
    {
        State = kState_SendShutdown;
        DriveSending();
    }

    // Otherwise, if the peer has already closed their end of the connection,
    else if (State == kState_ReceiveShutdown)
        err = DoClose(err, false);

    return err;
}

CHIP_ERROR TCPEndPoint::Close()
{
    // Clear the receive queue.
    mRcvQueue = nullptr;

    // Suppress closing callbacks, since the application explicitly called Close().
    OnConnectionClosed = nullptr;
    OnPeerClose        = nullptr;
    OnConnectComplete  = nullptr;

    // Perform a graceful close.
    return DoClose(CHIP_NO_ERROR, true);
}

void TCPEndPoint::Abort()
{
    // Suppress closing callbacks, since the application explicitly called Abort().
    OnConnectionClosed = nullptr;
    OnPeerClose        = nullptr;
    OnConnectComplete  = nullptr;

    DoClose(CHIP_ERROR_CONNECTION_ABORTED, true);
}

void TCPEndPoint::Free()
{
    CHIP_ERROR err;

    // Ensure no callbacks to the app after this point.
    OnAcceptError        = nullptr;
    OnConnectComplete    = nullptr;
    OnConnectionReceived = nullptr;
    OnConnectionClosed   = nullptr;
    OnPeerClose          = nullptr;
    OnDataReceived       = nullptr;
    OnDataSent           = nullptr;

    // Ensure the end point is Closed or Closing.
    err = Close();
    if (err != CHIP_NO_ERROR)
        Abort();

    // Release the Retain() that happened when the end point was allocated
    // [on LwIP, the object may still be alive if DoClose() used the
    // EndPointBasis::DeferredFree() method.]
    Release();
}

#if INET_TCP_IDLE_CHECK_INTERVAL > 0
void TCPEndPoint::SetIdleTimeout(uint32_t timeoutMS)
{
    uint32_t newIdleTimeout = (timeoutMS + (INET_TCP_IDLE_CHECK_INTERVAL - 1)) / INET_TCP_IDLE_CHECK_INTERVAL;
    InetLayer & lInetLayer  = Layer();
    bool isIdleTimerRunning = lInetLayer.IsIdleTimerRunning();

    if (newIdleTimeout > UINT16_MAX)
        newIdleTimeout = UINT16_MAX;
    mIdleTimeout = mRemainingIdleTime = static_cast<uint16_t>(newIdleTimeout);

    if (!isIdleTimerRunning && mIdleTimeout)
    {
        SystemLayer().StartTimer(INET_TCP_IDLE_CHECK_INTERVAL, InetLayer::HandleTCPInactivityTimer, &lInetLayer);
    }
}
#endif // INET_TCP_IDLE_CHECK_INTERVAL > 0

bool TCPEndPoint::IsConnected(int state)
{
    return state == kState_Connected || state == kState_SendShutdown || state == kState_ReceiveShutdown || state == kState_Closing;
}

void TCPEndPoint::Init(InetLayer * inetLayer)
{
    InitEndPointBasis(*inetLayer);

    ReceiveEnabled = true;

    // Initialize to zero for using system defaults.
    mConnectTimeoutMsecs = 0;

#if INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT
    mUserTimeoutMillis = INET_CONFIG_DEFAULT_TCP_USER_TIMEOUT_MSEC;

    mUserTimeoutTimerRunning = false;

#if INET_CONFIG_ENABLE_TCP_SEND_IDLE_CALLBACKS
    mIsTCPSendIdle = true;

    mTCPSendQueuePollPeriodMillis = INET_CONFIG_TCP_SEND_QUEUE_POLL_INTERVAL_MSEC;

    mTCPSendQueueRemainingPollCount = MaxTCPSendQueuePolls();

    OnTCPSendIdleChanged = NULL;
#endif // INET_CONFIG_ENABLE_TCP_SEND_IDLE_CALLBACKS

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    mBytesWrittenSinceLastProbe = 0;

    mLastTCPKernelSendQueueLen = 0;
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

#endif // INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    mUnackedLength = 0;
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP
}

CHIP_ERROR TCPEndPoint::DriveSending()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

#if CHIP_SYSTEM_CONFIG_USE_LWIP

    // Lock LwIP stack
    LOCK_TCPIP_CORE();

    // If the connection hasn't been aborted ...
    if (mTCP != NULL)
    {
        err_t lwipErr;

        // Determine the current send window size. This is the maximum amount we can write to the connection.
        uint16_t sendWindowSize = tcp_sndbuf(mTCP);

        // If there's data to be sent and the send window is open...
        bool canSend = (RemainingToSend() > 0 && sendWindowSize > 0);
        if (canSend)
        {
            // Find first packet buffer with remaining data to send by skipping
            // all sent but un-acked data.
            TCPEndPoint::BufferOffset startOfUnsent = FindStartOfUnsent();

            // While there's data to be sent and a window to send it in...
            do
            {
                VerifyOrDie(!startOfUnsent.buffer.IsNull());

                uint16_t bufDataLen = startOfUnsent.buffer->DataLength();

                // Get a pointer to the start of unsent data within the first buffer on the unsent queue.
                const uint8_t * sendData = startOfUnsent.buffer->Start() + startOfUnsent.offset;

                // Determine the amount of data to send from the current buffer.
                uint16_t sendLen = static_cast<uint16_t>(bufDataLen - startOfUnsent.offset);
                if (sendLen > sendWindowSize)
                    sendLen = sendWindowSize;

                // Call LwIP to queue the data to be sent, telling it if there's more data to come.
                // Data is queued in-place as a reference within the source packet buffer. It is
                // critical that the underlying packet buffer not be freed until the data
                // is acknowledged, otherwise retransmissions could use an invalid
                // backing. Using TCP_WRITE_FLAG_COPY would eliminate this requirement, but overall
                // requires many more memory allocations which may be problematic when very
                // memory-constrained or when using pool-based allocations.
                lwipErr = tcp_write(mTCP, sendData, sendLen, (canSend) ? TCP_WRITE_FLAG_MORE : 0);
                if (lwipErr != ERR_OK)
                {
                    err = chip::System::MapErrorLwIP(lwipErr);
                    break;
                }
                // Start accounting for the data sent as yet-to-be-acked.
                // This cast is safe, because mUnackedLength + sendLen <= bufDataLen, which fits in uint16_t.
                mUnackedLength = static_cast<uint16_t>(mUnackedLength + sendLen);

                // Adjust the unsent data offset by the length of data that was written.
                // If the entire buffer has been sent advance to the next one.
                // This cast is safe, because startOfUnsent.offset + sendLen <= bufDataLen, which fits in uint16_t.
                startOfUnsent.offset = static_cast<uint16_t>(startOfUnsent.offset + sendLen);
                if (startOfUnsent.offset == bufDataLen)
                {
                    startOfUnsent.buffer.Advance();
                    startOfUnsent.offset = 0;
                }

                // Adjust the remaining window size.
                sendWindowSize = static_cast<uint16_t>(sendWindowSize - sendLen);

                // Determine if there's more data to be sent after this buffer.
                canSend = (RemainingToSend() > 0 && sendWindowSize > 0);
            } while (canSend);

            // Call LwIP to send the queued data.
            INET_FAULT_INJECT(FaultInjection::kFault_Send, err = chip::System::MapErrorLwIP(ERR_RTE));

            if (err == CHIP_NO_ERROR)
            {
                lwipErr = tcp_output(mTCP);

                if (lwipErr != ERR_OK)
                    err = chip::System::MapErrorLwIP(lwipErr);
            }
        }

        if (err == CHIP_NO_ERROR)
        {
            // If in the SendShutdown state and the unsent queue is now empty, shutdown the PCB for sending.
            if (State == kState_SendShutdown && (RemainingToSend() == 0))
            {
                lwipErr = tcp_shutdown(mTCP, 0, 1);
                if (lwipErr != ERR_OK)
                    err = chip::System::MapErrorLwIP(lwipErr);
            }
        }
    }

    else
        err = CHIP_ERROR_CONNECTION_ABORTED;

    // Unlock LwIP stack
    UNLOCK_TCPIP_CORE();

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS

#ifdef MSG_NOSIGNAL
    const int sendFlags = MSG_NOSIGNAL;
#else
    const int sendFlags = 0;
#endif

    // Pretend send() fails in the while loop below
    INET_FAULT_INJECT(FaultInjection::kFault_Send, {
        err = chip::System::MapErrorPOSIX(EIO);
        DoClose(err, false);
        return err;
    });

    while (!mSendQueue.IsNull())
    {
        uint16_t bufLen = mSendQueue->DataLength();

        ssize_t lenSentRaw = send(mSocket.GetFD(), mSendQueue->Start(), bufLen, sendFlags);

        if (lenSentRaw == -1)
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
                err = (errno == EPIPE) ? INET_ERROR_PEER_DISCONNECTED : chip::System::MapErrorPOSIX(errno);
            break;
        }

        if (lenSentRaw < 0 || lenSentRaw > bufLen)
        {
            err = CHIP_ERROR_INCORRECT_STATE;
            break;
        }

        // Cast is safe because bufLen is uint16_t.
        uint16_t lenSent = static_cast<uint16_t>(lenSentRaw);

        // Mark the connection as being active.
        MarkActive();

        if (lenSent < bufLen)
        {
            mSendQueue->ConsumeHead(lenSent);
        }
        else
        {
            mSendQueue.FreeHead();
            if (mSendQueue.IsNull())
            {
                // Do not wait for ability to write on this endpoint.
                mSocket.ClearCallbackOnPendingWrite();
            }
        }

        if (OnDataSent != nullptr)
            OnDataSent(this, lenSent);

#if INET_CONFIG_ENABLE_TCP_SEND_IDLE_CALLBACKS
        // TCP Send is not Idle; Set state and notify if needed

        SetTCPSendIdleAndNotifyChange(false);
#endif // INET_CONFIG_ENABLE_TCP_SEND_IDLE_CALLBACKS

#if INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT
        mBytesWrittenSinceLastProbe += lenSent;

        bool isProgressing = false;

        err = CheckConnectionProgress(isProgressing);
        if (err != CHIP_NO_ERROR)
        {
            break;
        }

        if (!mUserTimeoutTimerRunning)
        {
            // Timer was not running before this write. So, start
            // the timer.

            StartTCPUserTimeoutTimer();
        }
        else if (isProgressing)
        {
            // Progress is being made. So, shift the timer
            // forward if it was started.

            RestartTCPUserTimeoutTimer();
        }
#endif // INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT

        if (lenSent < bufLen)
            break;
    }

    if (err == CHIP_NO_ERROR)
    {
        // If we're in the SendShutdown state and the send queue is now empty, shutdown writing on the socket.
        if (State == kState_SendShutdown && mSendQueue.IsNull())
        {
            if (shutdown(mSocket.GetFD(), SHUT_WR) != 0)
                err = chip::System::MapErrorPOSIX(errno);
        }
    }

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

    if (err != CHIP_NO_ERROR)
        DoClose(err, false);

    CHIP_SYSTEM_FAULT_INJECT_ASYNC_EVENT();

    return err;
}

void TCPEndPoint::DriveReceiving()
{
    // If there's data in the receive queue and the app is ready to receive it then call the app's callback
    // with the entire receive queue.
    if (!mRcvQueue.IsNull() && ReceiveEnabled && OnDataReceived != nullptr)
    {
        // Acknowledgement is done after handling the buffers to allow the
        // application processing to throttle flow.
        uint16_t ackLength = mRcvQueue->TotalLength();
        CHIP_ERROR err     = OnDataReceived(this, std::move(mRcvQueue));
        if (err != CHIP_NO_ERROR)
        {
            DoClose(err, false);
            return;
        }
        AckReceive(ackLength);
    }

    // If the connection is closing, and the receive queue is now empty, call DoClose() to complete
    // the process of closing the connection.
    if (State == kState_Closing && mRcvQueue.IsNull())
        DoClose(CHIP_NO_ERROR, false);
}

void TCPEndPoint::HandleConnectComplete(CHIP_ERROR err)
{
    // If the connect succeeded enter the Connected state and call the app's callback.
    if (err == CHIP_NO_ERROR)
    {
        // Stop the TCP Connect timer in case it is still running.
        StopConnectTimer();

        // Mark the connection as being active.
        MarkActive();

        State = kState_Connected;

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
        // Wait for ability to read or write on this endpoint.
        mSocket.RequestCallbackOnPendingRead();
        mSocket.RequestCallbackOnPendingWrite();
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

        if (OnConnectComplete != nullptr)
            OnConnectComplete(this, CHIP_NO_ERROR);
    }

    // Otherwise, close the connection with an error.
    else
        DoClose(err, false);
}

CHIP_ERROR TCPEndPoint::DoClose(CHIP_ERROR err, bool suppressCallback)
{
    int oldState = State;

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    struct linger lingerStruct;
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

    // If in one of the connected states (Connected, LocalShutdown, PeerShutdown or Closing)
    // AND this is a graceful close (i.e. not prompted by an error)
    // AND there is data waiting to be processed on either the send or receive queues
    // ... THEN enter the Closing state, allowing the queued data to drain,
    // ... OTHERWISE go straight to the Closed state.
    if (IsConnected() && err == CHIP_NO_ERROR && (!mSendQueue.IsNull() || !mRcvQueue.IsNull()))
        State = kState_Closing;
    else
        State = kState_Closed;

    // Stop the Connect timer in case it is still running.

    StopConnectTimer();

    // If not making a state transition, return immediately.
    if (State == oldState)
        return CHIP_NO_ERROR;

#if CHIP_SYSTEM_CONFIG_USE_LWIP

    // Lock LwIP stack
    LOCK_TCPIP_CORE();

    // If the LwIP PCB hasn't been closed yet...
    if (mTCP != NULL)
    {
        // If the endpoint was a connection endpoint (vs. a listening endpoint)...
        if (oldState != kState_Listening)
        {
            // Prevent further callbacks for incoming data.  This has the effect of instructing
            // LwIP to discard any further data received from the peer.
            tcp_recv(mTCP, NULL);

            // If entering the Closed state...
            if (State == kState_Closed)
            {
                // Prevent further callbacks to the error handler.
                //
                // Note: It is important to understand that LwIP can continue to make callbacks after
                // a PCB has been closed via the tcp_close() API. In particular, LwIP will continue
                // to call the 'data sent' callback to signal the acknowledgment of data that was
                // sent, but not acknowledged, prior to the close call. Additionally, LwIP will call
                // the error callback if the peer fails to respond in a timely manner to the either
                // sent data or the FIN. Unfortunately, there is no callback in the case where the
                // connection closes successfully. Because of this, it is impossible know definitively
                // when LwIP will no longer make callbacks to its user. Thus we must block further
                // callbacks to prevent them from happening after the endpoint has been freed.
                //
                tcp_err(mTCP, NULL);

                // If the endpoint is being closed without error, THEN call tcp_close() to close the underlying
                // TCP connection gracefully, preserving any in-transit send data.
                if (err == CHIP_NO_ERROR)
                {
                    tcp_close(mTCP);
                }

                // OTHERWISE, call tcp_abort() to abort the TCP connection, discarding any in-transit data.
                else
                {
                    tcp_abort(mTCP);
                }

                // Discard the reference to the PCB to ensure there is no further interaction with it
                // after this point.
                mTCP              = NULL;
                mLwIPEndPointType = kLwIPEndPointType_Unknown;
            }
        }

        // OTHERWISE the endpoint was being used for listening, so simply close it.
        else
        {
            tcp_close(mTCP);

            // Discard the reference to the PCB to ensure there is no further interaction with it
            // after this point.
            mTCP              = NULL;
            mLwIPEndPointType = kLwIPEndPointType_Unknown;
        }
    }

    // Unlock LwIP stack
    UNLOCK_TCPIP_CORE();

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS

    // If the socket hasn't been closed already...
    if (mSocket.HasFD())
    {
        // If entering the Closed state
        // OR if entering the Closing state, and there's no unsent data in the send queue
        // THEN close the socket.
        if (State == kState_Closed || (State == kState_Closing && mSendQueue.IsNull()))
        {
            // If aborting the connection, ensure we send a TCP RST.
            if (IsConnected(oldState) && err != CHIP_NO_ERROR)
            {
                lingerStruct.l_onoff  = 1;
                lingerStruct.l_linger = 0;

                if (setsockopt(mSocket.GetFD(), SOL_SOCKET, SO_LINGER, &lingerStruct, sizeof(lingerStruct)) != 0)
                    ChipLogError(Inet, "SO_LINGER: %d", errno);
            }

            if (mSocket.Close() != 0 && err == CHIP_NO_ERROR)
                err = chip::System::MapErrorPOSIX(errno);
        }
    }

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
    if (mReadableSource)
    {
        dispatch_source_cancel(mReadableSource);
        dispatch_release(mReadableSource);
    }
    if (mWriteableSource)
    {
        dispatch_source_cancel(mWriteableSource);
        dispatch_release(mWriteableSource);
    }
#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH

    // Clear any results from select() that indicate pending I/O for the socket.
    mSocket.ClearPendingIO();

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

#if INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT
    // Stop the TCP UserTimeout timer if it is running.
    StopTCPUserTimeoutTimer();
#endif // INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT

    // If entering the Closed state...
    if (State == kState_Closed)
    {
        // Clear clear the send and receive queues.
        mSendQueue = nullptr;
        mRcvQueue  = nullptr;
#if CHIP_SYSTEM_CONFIG_USE_LWIP
        mUnackedLength = 0;
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

        // Call the appropriate app callback if allowed.
        if (!suppressCallback)
        {
            if (oldState == kState_Connecting)
            {
                if (OnConnectComplete != nullptr)
                    OnConnectComplete(this, err);
            }
            else if ((oldState == kState_Connected || oldState == kState_SendShutdown || oldState == kState_ReceiveShutdown ||
                      oldState == kState_Closing) &&
                     OnConnectionClosed != nullptr)
                OnConnectionClosed(this, err);
        }

        // Decrement the ref count that was added when the connection started (in Connect()) or listening started (in Listen()).
        //
        // When using LwIP, post a callback to Release() rather than calling it directly. Since up-calls
        // from LwIP are delivered as events (via the LwIP* methods), we must ensure that all events have been
        // cleared from the queue before the end point gets freed, otherwise we'll end up accessing freed memory.
        // We achieve this by first preventing further up-calls from LwIP (via the call to tcp_abort() above)
        // and then queuing the Release() call to happen after all existing events have been processed.
        //
        if (oldState != kState_Ready && oldState != kState_Bound)
        {
#if CHIP_SYSTEM_CONFIG_USE_LWIP
            DeferredFree(kReleaseDeferralErrorTactic_Ignore);
#else  // !CHIP_SYSTEM_CONFIG_USE_LWIP
            Release();
#endif // !CHIP_SYSTEM_CONFIG_USE_LWIP
        }
    }

    return err;
}

#if INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT
void TCPEndPoint::TCPUserTimeoutHandler(chip::System::Layer * aSystemLayer, void * aAppState, CHIP_ERROR aError)
{
    TCPEndPoint * tcpEndPoint = reinterpret_cast<TCPEndPoint *>(aAppState);

    VerifyOrDie((aSystemLayer != nullptr) && (tcpEndPoint != nullptr));

    // Set the timer running flag to false
    tcpEndPoint->mUserTimeoutTimerRunning = false;

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    CHIP_ERROR err     = CHIP_NO_ERROR;
    bool isProgressing = false;
    err                = tcpEndPoint->CheckConnectionProgress(isProgressing);
    SuccessOrExit(err);

    if (tcpEndPoint->mLastTCPKernelSendQueueLen == 0)
    {
#if INET_CONFIG_ENABLE_TCP_SEND_IDLE_CALLBACKS
        // If the kernel TCP send queue as well as the TCPEndPoint
        // send queue have been flushed then notify application
        // that all data has been acknowledged.

        if (tcpEndPoint->mSendQueue.IsNull())
        {
            tcpEndPoint->SetTCPSendIdleAndNotifyChange(true);
        }
#endif // INET_CONFIG_ENABLE_TCP_SEND_IDLE_CALLBACKS
    }
    else
    // There is data in the TCP Send Queue
    {
        if (isProgressing)
        {
            // Data is flowing, so restart the UserTimeout timer
            // to shift it forward while also resetting the max
            // poll count.

            tcpEndPoint->StartTCPUserTimeoutTimer();
        }
        else
        {
#if INET_CONFIG_ENABLE_TCP_SEND_IDLE_CALLBACKS
            // Data flow is not progressing.
            // Decrement the remaining max TCP send queue polls.

            tcpEndPoint->mTCPSendQueueRemainingPollCount--;

            VerifyOrExit(tcpEndPoint->mTCPSendQueueRemainingPollCount != 0, err = INET_ERROR_TCP_USER_TIMEOUT);

            // Restart timer to poll again

            tcpEndPoint->ScheduleNextTCPUserTimeoutPoll(tcpEndPoint->mTCPSendQueuePollPeriodMillis);
#else
            // Close the connection as the TCP UserTimeout has expired

            ExitNow(err = INET_ERROR_TCP_USER_TIMEOUT);
#endif // !INET_CONFIG_ENABLE_TCP_SEND_IDLE_CALLBACKS
        }
    }

exit:

    if (err != CHIP_NO_ERROR)
    {
        // Close the connection as the TCP UserTimeout has expired

        tcpEndPoint->DoClose(err, false);
    }
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    // Close Connection as we have timed out and there is still
    // data not sent out successfully.

    tcpEndPoint->DoClose(INET_ERROR_TCP_USER_TIMEOUT, false);
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP
}

void TCPEndPoint::ScheduleNextTCPUserTimeoutPoll(uint32_t aTimeOut)
{
    chip::System::Layer & lSystemLayer = SystemLayer();

    lSystemLayer.StartTimer(aTimeOut, TCPUserTimeoutHandler, this);
}

#if INET_CONFIG_ENABLE_TCP_SEND_IDLE_CALLBACKS
void TCPEndPoint::SetTCPSendIdleAndNotifyChange(bool aIsTCPSendIdle)
{
    if (mIsTCPSendIdle != aIsTCPSendIdle)
    {
        ChipLogDetail(Inet, "TCP con send channel idle state changed : %s", aIsTCPSendIdle ? "false->true" : "true->false");

        // Set the current Idle state
        mIsTCPSendIdle = aIsTCPSendIdle;

        if (OnTCPSendIdleChanged)
        {
            OnTCPSendIdleChanged(this, mIsTCPSendIdle);
        }
    }
}
#endif // INET_CONFIG_ENABLE_TCP_SEND_IDLE_CALLBACKS

void TCPEndPoint::StartTCPUserTimeoutTimer()
{
    uint32_t timeOut = mUserTimeoutMillis;

#if INET_CONFIG_ENABLE_TCP_SEND_IDLE_CALLBACKS
    // Set timeout to the poll interval

    timeOut = mTCPSendQueuePollPeriodMillis;

    // Reset the poll count

    mTCPSendQueueRemainingPollCount = MaxTCPSendQueuePolls();
#endif // INET_CONFIG_ENABLE_TCP_SEND_IDLE_CALLBACKS

    ScheduleNextTCPUserTimeoutPoll(timeOut);

    mUserTimeoutTimerRunning = true;
}

void TCPEndPoint::StopTCPUserTimeoutTimer()
{
    chip::System::Layer & lSystemLayer = SystemLayer();

    lSystemLayer.CancelTimer(TCPUserTimeoutHandler, this);

    mUserTimeoutTimerRunning = false;
}

void TCPEndPoint::RestartTCPUserTimeoutTimer()
{
    StopTCPUserTimeoutTimer();

    StartTCPUserTimeoutTimer();
}

#endif // INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT

#if CHIP_SYSTEM_CONFIG_USE_LWIP

uint16_t TCPEndPoint::RemainingToSend()
{
    if (mSendQueue.IsNull())
    {
        return 0;
    }
    else
    {
        // We can never have reported more unacked data than there is pending
        // in the send queue! This would indicate a critical accounting bug.
        VerifyOrDie(mUnackedLength <= mSendQueue->TotalLength());

        return static_cast<uint16_t>(mSendQueue->TotalLength() - mUnackedLength);
    }
}

TCPEndPoint::BufferOffset TCPEndPoint::FindStartOfUnsent()
{
    // Find first packet buffer with remaining data to send by skipping
    // all sent but un-acked data. This is necessary because of the Consume()
    // call in HandleDataSent(), which potentially releases backing memory for
    // fully-sent packet buffers, causing an invalidation of all possible
    // offsets one might have cached. The TCP acnowledgements may come back
    // with a variety of sizes depending on prior activity, and size of the
    // send window. The only way to ensure we get the correct offsets into
    // unsent data while retaining the buffers that have un-acked data is to
    // traverse all sent-but-unacked data in the chain to reach the beginning
    // of ready-to-send data.
    TCPEndPoint::BufferOffset startOfUnsent(mSendQueue.Retain());
    uint16_t leftToSkip = mUnackedLength;

    VerifyOrDie(leftToSkip < mSendQueue->TotalLength());

    while (leftToSkip > 0)
    {
        VerifyOrDie(!startOfUnsent.buffer.IsNull());
        uint16_t bufDataLen = startOfUnsent.buffer->DataLength();
        if (leftToSkip >= bufDataLen)
        {
            // We have more to skip than current packet buffer size.
            // Follow the chain to continue.
            startOfUnsent.buffer.Advance();
            leftToSkip = static_cast<uint16_t>(leftToSkip - bufDataLen);
        }
        else
        {
            // Done skipping all data, currentUnsentBuf is first packet buffer
            // containing unsent data.
            startOfUnsent.offset = leftToSkip;
            leftToSkip           = 0;
        }
    }

    return startOfUnsent;
}

CHIP_ERROR TCPEndPoint::GetPCB(IPAddressType addrType)
{
    // IMMPORTANT: This method MUST be called with the LwIP stack LOCKED!

#if LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
    if (mTCP == NULL)
    {
        switch (addrType)
        {
        case kIPAddressType_IPv6:
            mTCP = tcp_new_ip_type(IPADDR_TYPE_V6);
            break;

#if INET_CONFIG_ENABLE_IPV4
        case kIPAddressType_IPv4:
            mTCP = tcp_new_ip_type(IPADDR_TYPE_V4);
            break;
#endif // INET_CONFIG_ENABLE_IPV4

        default:
            return INET_ERROR_WRONG_ADDRESS_TYPE;
        }

        if (mTCP == NULL)
        {
            return CHIP_ERROR_NO_MEMORY;
        }
        else
        {
            mLwIPEndPointType = kLwIPEndPointType_TCP;
        }
    }
    else
    {
        switch (IP_GET_TYPE(&mTCP->local_ip))
        {
        case IPADDR_TYPE_V6:
            if (addrType != kIPAddressType_IPv6)
                return INET_ERROR_WRONG_ADDRESS_TYPE;
            break;

#if INET_CONFIG_ENABLE_IPV4
        case IPADDR_TYPE_V4:
            if (addrType != kIPAddressType_IPv4)
                return INET_ERROR_WRONG_ADDRESS_TYPE;
            break;
#endif // INET_CONFIG_ENABLE_IPV4

        default:
            break;
        }
    }
#else // LWIP_VERSION_MAJOR <= 1 || LWIP_VERSION_MINOR >= 5
    if (mTCP == NULL)
    {
        if (addrType == kIPAddressType_IPv6)
            mTCP = tcp_new_ip6();
#if INET_CONFIG_ENABLE_IPV4
        else if (addrType == kIPAddressType_IPv4)
            mTCP = tcp_new();
#endif // INET_CONFIG_ENABLE_IPV4
        else
            return INET_ERROR_WRONG_ADDRESS_TYPE;
        if (mTCP == NULL)
        {
            return CHIP_ERROR_NO_MEMORY;
        }
        else
        {
            mLwIPEndPointType = kLwIPEndPointType_TCP;
        }
    }
    else
    {
#if INET_CONFIG_ENABLE_IPV4
        const IPAddressType pcbType = PCB_ISIPV6(mTCP) ? kIPAddressType_IPv6 : kIPAddressType_IPv4;
#else  // !INET_CONFIG_ENABLE_IPV4
        const IPAddressType pcbType = kIPAddressType_IPv6;
#endif // !INET_CONFIG_ENABLE_IPV4
        if (addrType != pcbType)
            return INET_ERROR_WRONG_ADDRESS_TYPE;
    }
#endif // LWIP_VERSION_MAJOR <= 1 || LWIP_VERSION_MINOR >= 5

    return CHIP_NO_ERROR;
}

void TCPEndPoint::HandleDataSent(uint16_t lenSent)
{
    if (IsConnected())
    {
        // Ensure we do not have internal inconsistency in the lwIP, which
        // could cause invalid pointer accesses.
        if (lenSent > mUnackedLength)
        {
            ChipLogError(Inet, "Got more ACKed bytes (%d) than were pending (%d)", (int) lenSent, (int) mUnackedLength);
            DoClose(CHIP_ERROR_UNEXPECTED_EVENT, false);
            return;
        }
        else if (mSendQueue.IsNull())
        {
            ChipLogError(Inet, "Got ACK for %d bytes but data backing gone", (int) lenSent);
            DoClose(CHIP_ERROR_UNEXPECTED_EVENT, false);
            return;
        }

        // Consume data off the head of the send queue equal to the amount of data being acknowledged.
        mSendQueue.Consume(lenSent);
        mUnackedLength = static_cast<uint16_t>(mUnackedLength - lenSent);

#if INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT
        // Only change the UserTimeout timer if lenSent > 0,
        // indicating progress being made in sending data
        // across.
        if (lenSent > 0)
        {
            if (RemainingToSend() == 0)
            {
                // If the output queue has been flushed then stop the timer.

                StopTCPUserTimeoutTimer();

#if INET_CONFIG_ENABLE_TCP_SEND_IDLE_CALLBACKS
                // Notify up if all outstanding data has been acknowledged

                SetTCPSendIdleAndNotifyChange(true);
#endif // INET_CONFIG_ENABLE_TCP_SEND_IDLE_CALLBACKS
            }
            else
            {
                // Progress is being made. So, shift the timer
                // forward if it was started.
                RestartTCPUserTimeoutTimer();
            }
        }
#endif // INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT

        // Mark the connection as being active.
        MarkActive();

        // If requested, call the app's OnDataSent callback.
        if (OnDataSent != NULL)
            OnDataSent(this, lenSent);

        // If unsent data exists, attempt to send it now...
        if (RemainingToSend() > 0)
            DriveSending();

        // If in the closing state and the send queue is now empty, attempt to transition to closed.
        if ((State == kState_Closing) && (RemainingToSend() == 0))
            DoClose(CHIP_NO_ERROR, false);
    }
}

void TCPEndPoint::HandleDataReceived(System::PacketBufferHandle && buf)
{
    // Only receive new data while in the Connected or SendShutdown states.
    if (State == kState_Connected || State == kState_SendShutdown)
    {
        // Mark the connection as being active.
        MarkActive();

        // If we received a data buffer, queue it on the receive queue.  If there's already data in
        // the queue, compact the data into the head buffer.
        if (!buf.IsNull())
        {
            if (mRcvQueue.IsNull())
            {
                mRcvQueue = std::move(buf);
            }
            else
            {
                mRcvQueue->AddToEnd(std::move(buf));
                mRcvQueue->CompactHead();
            }
        }

        // Otherwise buf == NULL means the other side closed the connection, so ...
        else
        {

            // If in the Connected state and the app has provided an OnPeerClose callback,
            // enter the ReceiveShutdown state.  Providing an OnPeerClose callback allows
            // the app to decide whether to keep the send side of the connection open after
            // the peer has closed. If no OnPeerClose is provided, we assume that the app
            // wants to close both directions and automatically enter the Closing state.
            if (State == kState_Connected && OnPeerClose != NULL)
                State = kState_ReceiveShutdown;
            else
                State = kState_Closing;

            // Call the app's OnPeerClose.
            if (OnPeerClose != NULL)
                OnPeerClose(this);
        }

        // Drive the received data into the app.
        DriveReceiving();
    }
}

void TCPEndPoint::HandleIncomingConnection(TCPEndPoint * conEP)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    IPAddress peerAddr;
    uint16_t peerPort;

    if (State == kState_Listening)
    {
        // If there's no callback available, fail with an error.
        if (OnConnectionReceived == NULL)
            err = CHIP_ERROR_NO_CONNECTION_HANDLER;

        // Extract the peer's address information.
        if (err == CHIP_NO_ERROR)
            err = conEP->GetPeerInfo(&peerAddr, &peerPort);

        // If successful, call the app's callback function.
        if (err == CHIP_NO_ERROR)
            OnConnectionReceived(this, conEP, peerAddr, peerPort);

        // Otherwise clean up and call the app's error callback.
        else if (OnAcceptError != NULL)
            OnAcceptError(this, err);
    }
    else
        err = CHIP_ERROR_INCORRECT_STATE;

    // If something failed above, abort and free the connection end point.
    if (err != CHIP_NO_ERROR)
        conEP->Free();
}

void TCPEndPoint::HandleError(CHIP_ERROR err)
{
    if (State == kState_Listening)
    {
        if (OnAcceptError != NULL)
            OnAcceptError(this, err);
    }
    else
        DoClose(err, false);
}

err_t TCPEndPoint::LwIPHandleConnectComplete(void * arg, struct tcp_pcb * tpcb, err_t lwipErr)
{
    err_t res = ERR_OK;

    if (arg != NULL)
    {
        CHIP_ERROR conErr;
        TCPEndPoint * ep                   = static_cast<TCPEndPoint *>(arg);
        chip::System::Layer & lSystemLayer = ep->SystemLayer();

        if (lwipErr == ERR_OK)
        {
            // Setup LwIP callback functions for data transmission.
            tcp_recv(ep->mTCP, LwIPHandleDataReceived);
            tcp_sent(ep->mTCP, LwIPHandleDataSent);
        }

        // Post callback to HandleConnectComplete.
        conErr = chip::System::MapErrorLwIP(lwipErr);
        if (lSystemLayer.PostEvent(*ep, kInetEvent_TCPConnectComplete, (uintptr_t) conErr) != CHIP_NO_ERROR)
            res = ERR_ABRT;
    }
    else
        res = ERR_ABRT;

    if (res != ERR_OK)
        tcp_abort(tpcb);

    return res;
}

err_t TCPEndPoint::LwIPHandleIncomingConnection(void * arg, struct tcp_pcb * tpcb, err_t lwipErr)
{
    CHIP_ERROR err = chip::System::MapErrorLwIP(lwipErr);

    if (arg != NULL)
    {
        TCPEndPoint * listenEP             = static_cast<TCPEndPoint *>(arg);
        TCPEndPoint * conEP                = NULL;
        chip::System::Layer & lSystemLayer = listenEP->SystemLayer();

        // Tell LwIP we've accepted the connection so it can decrement the listen PCB's pending_accepts counter.
        tcp_accepted(listenEP->mTCP);

        // If we did in fact receive a connection, rather than an error, attempt to allocate an end point object.
        //
        // NOTE: Although most of the LwIP callbacks defer the real work to happen on the endpoint's thread
        // (by posting events to the thread's event queue) we can't do that here because as soon as this
        // function returns, LwIP is free to begin calling callbacks on the new PCB. For that to work we need
        // to have an end point associated with the PCB.
        //
        if (err == CHIP_NO_ERROR)
        {
            InetLayer & lInetLayer = listenEP->Layer();

            err = lInetLayer.NewTCPEndPoint(&conEP);
        }

        // Ensure that TCP timers have been started
        if (err == CHIP_NO_ERROR)
        {
            err = start_tcp_timers();
        }

        // If successful in allocating an end point...
        if (err == CHIP_NO_ERROR)
        {
            // Put the new end point into the Connected state.
            conEP->State             = kState_Connected;
            conEP->mTCP              = tpcb;
            conEP->mLwIPEndPointType = kLwIPEndPointType_TCP;
            conEP->Retain();

            // Setup LwIP callback functions for the new PCB.
            tcp_arg(tpcb, conEP);
            tcp_recv(tpcb, LwIPHandleDataReceived);
            tcp_sent(tpcb, LwIPHandleDataSent);
            tcp_err(tpcb, LwIPHandleError);

            // Post a callback to the HandleConnectionReceived() function, passing it the new end point.
            if (lSystemLayer.PostEvent(*listenEP, kInetEvent_TCPConnectionReceived, (uintptr_t) conEP) != CHIP_NO_ERROR)
            {
                err = CHIP_ERROR_CONNECTION_ABORTED;
                conEP->Release(); // for the Retain() above
                conEP->Release(); // for the Retain() in NewTCPEndPoint()
            }
        }

        // Otherwise, there was an error accepting the connection, so post a callback to the HandleError function.
        else
            lSystemLayer.PostEvent(*listenEP, kInetEvent_TCPError, (uintptr_t) err);
    }
    else
        err = CHIP_ERROR_CONNECTION_ABORTED;

    if (err != CHIP_NO_ERROR && tpcb != NULL)
    {
        tcp_abort(tpcb);
        return ERR_ABRT;
    }
    else
    {
        return ERR_OK;
    }
}

err_t TCPEndPoint::LwIPHandleDataReceived(void * arg, struct tcp_pcb * tpcb, struct pbuf * p, err_t err)
{
    err_t res = ERR_OK;

    if (arg != NULL)
    {
        TCPEndPoint * ep                   = static_cast<TCPEndPoint *>(arg);
        chip::System::Layer & lSystemLayer = ep->SystemLayer();

        // Post callback to HandleDataReceived.
        if (lSystemLayer.PostEvent(*ep, kInetEvent_TCPDataReceived, (uintptr_t) p) != CHIP_NO_ERROR)
            res = ERR_ABRT;
    }
    else
        res = ERR_ABRT;

    if (res != ERR_OK)
        tcp_abort(tpcb);

    return res;
}

err_t TCPEndPoint::LwIPHandleDataSent(void * arg, struct tcp_pcb * tpcb, u16_t len)
{
    err_t res = ERR_OK;

    if (arg != NULL)
    {
        TCPEndPoint * ep                   = static_cast<TCPEndPoint *>(arg);
        chip::System::Layer & lSystemLayer = ep->SystemLayer();

        // Post callback to HandleDataReceived.
        if (lSystemLayer.PostEvent(*ep, kInetEvent_TCPDataSent, (uintptr_t) len) != CHIP_NO_ERROR)
            res = ERR_ABRT;
    }
    else
        res = ERR_ABRT;

    if (res != ERR_OK)
        tcp_abort(tpcb);

    return res;
}

void TCPEndPoint::LwIPHandleError(void * arg, err_t lwipErr)
{
    if (arg != NULL)
    {
        TCPEndPoint * ep                   = static_cast<TCPEndPoint *>(arg);
        chip::System::Layer & lSystemLayer = ep->SystemLayer();

        // At this point LwIP has already freed the PCB.  Since the thread that owns the TCPEndPoint may
        // try to use the PCB before it receives the TCPError event posted below, we set the PCB to NULL
        // as a means to signal the other thread that the connection has been aborted.  The implication
        // of this is that the mTCP field is shared state between the two threads and thus must only be
        // accessed with the LwIP lock held.
        ep->mTCP              = NULL;
        ep->mLwIPEndPointType = kLwIPEndPointType_Unknown;

        // Post callback to HandleError.
        CHIP_ERROR err = chip::System::MapErrorLwIP(lwipErr);
        lSystemLayer.PostEvent(*ep, kInetEvent_TCPError, (uintptr_t) err);
    }
}

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS

CHIP_ERROR TCPEndPoint::BindSrcAddrFromIntf(IPAddressType addrType, InterfaceId intfId)
{
    // If we are trying to make a TCP connection over a 'specified target interface',
    // then we bind the TCPEndPoint to an IP address on that target interface
    // and use that address as the source address for that connection. This is
    // done in the event that directly binding the connection to the target
    // interface is not allowed due to insufficient privileges.
    VerifyOrReturnError(State != kState_Bound, CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    bool ipAddrFound = false;
    for (InterfaceAddressIterator addrIter; addrIter.HasCurrent(); addrIter.Next())
    {
        const IPAddress curAddr     = addrIter.GetAddress();
        const InterfaceId curIntfId = addrIter.GetInterface();

        if (curIntfId == intfId)
        {
            // Search for an IPv4 address on the TargetInterface

#if INET_CONFIG_ENABLE_IPV4
            if (addrType == kIPAddressType_IPv4)
            {
                if (curAddr.IsIPv4())
                {
                    // Bind to the IPv4 address of the TargetInterface
                    ipAddrFound = true;
                    ReturnErrorOnFailure(Bind(kIPAddressType_IPv4, curAddr, 0, true));

                    break;
                }
            }
#endif // INET_CONFIG_ENABLE_IPV4
            if (addrType == kIPAddressType_IPv6)
            {
                // Select an IPv6 address on the interface that is not
                // a link local or a multicast address.
                // TODO: Define a proper IPv6GlobalUnicast address checker.
                if (!curAddr.IsIPv4() && !curAddr.IsIPv6LinkLocal() && !curAddr.IsMulticast())
                {
                    // Bind to the IPv6 address of the TargetInterface
                    ipAddrFound = true;
                    ReturnErrorOnFailure(Bind(kIPAddressType_IPv6, curAddr, 0, true));

                    break;
                }
            }
        }
    }

    VerifyOrReturnError(ipAddrFound, CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    return CHIP_NO_ERROR;
}

CHIP_ERROR TCPEndPoint::GetSocket(IPAddressType addrType)
{
    if (!mSocket.HasFD())
    {
        int family;
        if (addrType == kIPAddressType_IPv6)
            family = PF_INET6;
#if INET_CONFIG_ENABLE_IPV4
        else if (addrType == kIPAddressType_IPv4)
            family = PF_INET;
#endif // INET_CONFIG_ENABLE_IPV4
        else
            return INET_ERROR_WRONG_ADDRESS_TYPE;
        const int fd = ::socket(family, SOCK_STREAM | SOCK_FLAGS, 0);
        if (fd == -1)
            return chip::System::MapErrorPOSIX(errno);
        mSocket.Attach(fd);
        mAddrType = addrType;

        // If creating an IPv6 socket, tell the kernel that it will be IPv6 only.  This makes it
        // posible to bind two sockets to the same port, one for IPv4 and one for IPv6.
#ifdef IPV6_V6ONLY
        if (family == PF_INET6)
        {
            int one = 1;
            setsockopt(mSocket.GetFD(), IPPROTO_IPV6, IPV6_V6ONLY, &one, sizeof(one));
        }
#endif // defined(IPV6_V6ONLY)

        // On systems that support it, disable the delivery of SIGPIPE signals when writing to a closed
        // socket.
#ifdef SO_NOSIGPIPE
        {
            int one = 1;
            int res = setsockopt(mSocket.GetFD(), SOL_SOCKET, SO_NOSIGPIPE, &one, sizeof(one));
            if (res != 0)
            {
                ChipLogError(Inet, "SO_NOSIGPIPE: %d", errno);
            }
        }
#endif // defined(SO_NOSIGPIPE)
    }
    else if (mAddrType != addrType)
        return CHIP_ERROR_INCORRECT_STATE;

    return CHIP_NO_ERROR;
}

// static
void TCPEndPoint::HandlePendingIO(System::WatchableSocket & socket)
{
    reinterpret_cast<TCPEndPoint *>(socket.GetCallbackData())->HandlePendingIO();
}

void TCPEndPoint::HandlePendingIO()
{
    // Prevent the end point from being freed while in the middle of a callback.
    Retain();

    // If in the Listening state, and the app is ready to receive a connection, and there is a connection
    // ready to be received on the socket, process the incoming connection.
    if (State == kState_Listening)
    {
        if (OnConnectionReceived != nullptr && mSocket.HasPendingRead())
        {
            HandleIncomingConnection();
        }
    }

    // If in the processes of initiating a connection...
    else if (State == kState_Connecting)
    {
        // The socket being writable indicates the connection has completed (successfully or otherwise).
        if (mSocket.HasPendingWrite())
        {
            // Get the connection result from the socket.
            int osConRes;
            socklen_t optLen = sizeof(osConRes);
            if (getsockopt(mSocket.GetFD(), SOL_SOCKET, SO_ERROR, &osConRes, &optLen) != 0)
                osConRes = errno;
            CHIP_ERROR conRes = chip::System::MapErrorPOSIX(osConRes);

            // Process the connection result.
            HandleConnectComplete(conRes);
        }
    }

    else
    {
        // If in a state where sending is allowed, and there is data to be sent, and the socket is ready for
        // writing, drive outbound data into the connection.
        if (IsConnected() && !mSendQueue.IsNull() && mSocket.HasPendingWrite())
            DriveSending();

        // If in a state were receiving is allowed, and the app is ready to receive data, and data is ready
        // on the socket, receive inbound data from the connection.
        if ((State == kState_Connected || State == kState_SendShutdown) && ReceiveEnabled && OnDataReceived != nullptr &&
            mSocket.HasPendingRead())
            ReceiveData();
    }

    mSocket.ClearPendingIO();

    Release();
}

void TCPEndPoint::ReceiveData()
{
    System::PacketBufferHandle rcvBuf;
    bool isNewBuf = true;

    if (mRcvQueue.IsNull())
        rcvBuf = System::PacketBufferHandle::New(kMaxReceiveMessageSize, 0);
    else
    {
        rcvBuf = mRcvQueue->Last();
        if (rcvBuf->AvailableDataLength() == 0)
        {
            rcvBuf = System::PacketBufferHandle::New(kMaxReceiveMessageSize, 0);
        }
        else
        {
            isNewBuf = false;
            rcvBuf->CompactHead();
        }
    }

    if (rcvBuf.IsNull())
    {
        DoClose(CHIP_ERROR_NO_MEMORY, false);
        return;
    }

    // Attempt to receive data from the socket.
    ssize_t rcvLen = recv(mSocket.GetFD(), rcvBuf->Start() + rcvBuf->DataLength(), rcvBuf->AvailableDataLength(), 0);

#if INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT
    CHIP_ERROR err;
    bool isProgressing = false;

    err = CheckConnectionProgress(isProgressing);
    if (err != CHIP_NO_ERROR)
    {
        DoClose(err, false);

        return;
    }

    if (mLastTCPKernelSendQueueLen == 0)
    {
        // If the output queue has been flushed then stop the timer.

        StopTCPUserTimeoutTimer();

#if INET_CONFIG_ENABLE_TCP_SEND_IDLE_CALLBACKS
        // Notify up if all outstanding data has been acknowledged

        if (mSendQueue.IsNull())
        {
            SetTCPSendIdleAndNotifyChange(true);
        }
#endif // INET_CONFIG_ENABLE_TCP_SEND_IDLE_CALLBACKS
    }
    else if (isProgressing && mUserTimeoutTimerRunning)
    {
        // Progress is being made. So, shift the timer
        // forward if it was started.
        RestartTCPUserTimeoutTimer();
    }
#endif // INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT
    // If an error occurred, abort the connection.
    if (rcvLen < 0)
    {
        int systemErrno = errno;
        if (systemErrno == EAGAIN)
        {
            // Note: in this case, we opt to not retry the recv call,
            // and instead we expect that the read flags will get
            // reset correctly upon a subsequent return from the
            // select call.
            ChipLogError(Inet, "recv: EAGAIN, will retry");

            return;
        }

        DoClose(chip::System::MapErrorPOSIX(systemErrno), false);
    }

    else
    {
        // Mark the connection as being active.
        MarkActive();

        // If the peer closed their end of the connection...
        if (rcvLen == 0)
        {
            // If in the Connected state and the app has provided an OnPeerClose callback,
            // enter the ReceiveShutdown state.  Providing an OnPeerClose callback allows
            // the app to decide whether to keep the send side of the connection open after
            // the peer has closed. If no OnPeerClose is provided, we assume that the app
            // wants to close both directions and automatically enter the Closing state.
            if (State == kState_Connected && OnPeerClose != nullptr)
                State = kState_ReceiveShutdown;
            else
                State = kState_Closing;
            // Do not wait for ability to read on this endpoint.
            mSocket.ClearCallbackOnPendingRead();
            // Call the app's OnPeerClose.
            if (OnPeerClose != nullptr)
                OnPeerClose(this);
        }

        // Otherwise, add the new data onto the receive queue.
        else
        {
            VerifyOrDie(rcvLen > 0);
            size_t newDataLength = rcvBuf->DataLength() + static_cast<size_t>(rcvLen);
            VerifyOrDie(CanCastTo<uint16_t>(newDataLength));
            if (isNewBuf)
            {
                rcvBuf->SetDataLength(static_cast<uint16_t>(newDataLength));
                rcvBuf.RightSize();
                if (mRcvQueue.IsNull())
                    mRcvQueue = std::move(rcvBuf);
                else
                    mRcvQueue->AddToEnd(std::move(rcvBuf));
            }
            else
            {
                rcvBuf->SetDataLength(static_cast<uint16_t>(newDataLength), mRcvQueue);
            }
        }
    }

    // Drive any received data into the app.
    DriveReceiving();
}

void TCPEndPoint::HandleIncomingConnection()
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    TCPEndPoint * conEP = nullptr;
    IPAddress peerAddr;
    uint16_t peerPort;

    union
    {
        sockaddr any;
        sockaddr_in in;
        sockaddr_in6 in6;
    } sa;
    memset(&sa, 0, sizeof(sa));
    socklen_t saLen = sizeof(sa);

    // Accept the new connection.
    int conSocket = accept(mSocket.GetFD(), &sa.any, &saLen);
    if (conSocket == -1)
        err = chip::System::MapErrorPOSIX(errno);

    // If there's no callback available, fail with an error.
    if (err == CHIP_NO_ERROR && OnConnectionReceived == nullptr)
        err = CHIP_ERROR_NO_CONNECTION_HANDLER;

    // Extract the peer's address information.
    if (err == CHIP_NO_ERROR)
    {
        if (sa.any.sa_family == AF_INET6)
        {
            peerAddr = IPAddress::FromIPv6(sa.in6.sin6_addr);
            peerPort = ntohs(sa.in6.sin6_port);
        }
#if INET_CONFIG_ENABLE_IPV4
        else if (sa.any.sa_family == AF_INET)
        {
            peerAddr = IPAddress::FromIPv4(sa.in.sin_addr);
            peerPort = ntohs(sa.in.sin_port);
        }
#endif // INET_CONFIG_ENABLE_IPV4
        else
            err = CHIP_ERROR_INCORRECT_STATE;
    }

    // Attempt to allocate an end point object.
    if (err == CHIP_NO_ERROR)
    {
        InetLayer & lInetLayer = Layer();

        err = lInetLayer.NewTCPEndPoint(&conEP);
    }

    // If all went well...
    if (err == CHIP_NO_ERROR)
    {
        // Put the new end point into the Connected state.
        conEP->mSocket.Attach(conSocket);
        conEP->State = kState_Connected;
#if INET_CONFIG_ENABLE_IPV4
        conEP->mAddrType = (sa.any.sa_family == AF_INET6) ? kIPAddressType_IPv6 : kIPAddressType_IPv4;
#else  // !INET_CONFIG_ENABLE_IPV4
        conEP->mAddrType = kIPAddressType_IPv6;
#endif // !INET_CONFIG_ENABLE_IPV4
        conEP->Retain();

        // Wait for ability to read on this endpoint.
        conEP->mSocket.SetCallback(HandlePendingIO, reinterpret_cast<intptr_t>(conEP));
        conEP->mSocket.RequestCallbackOnPendingRead();

        // Call the app's callback function.
        OnConnectionReceived(this, conEP, peerAddr, peerPort);
    }

    // Otherwise immediately close the connection, clean up and call the app's error callback.
    else
    {
        if (conSocket != -1)
            close(conSocket);
        if (conEP != nullptr)
        {
            if (conEP->State == kState_Connected)
                conEP->Release();
            conEP->Release();
        }
        if (OnAcceptError != nullptr)
            OnAcceptError(this, err);
    }
}

#if INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT
/**
 *  This function probes the TCP output queue and checks if data is successfully
 *  being transferred to the other end.
 */
CHIP_ERROR TCPEndPoint::CheckConnectionProgress(bool & isProgressing)
{
    int currPendingBytesRaw = 0;
    uint32_t currPendingBytes; // Will be initialized once we know it's safe.

    // Fetch the bytes pending successful transmission in the TCP out queue.

    if (ioctl(mSocket.GetFD(), TIOCOUTQ, &currPendingBytesRaw) < 0)
    {
        return chip::System::MapErrorPOSIX(errno);
    }

    if (!CanCastTo<uint32_t>(currPendingBytesRaw))
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    currPendingBytes = static_cast<uint32_t>(currPendingBytesRaw);

    if ((currPendingBytes != 0) && (mBytesWrittenSinceLastProbe + mLastTCPKernelSendQueueLen == currPendingBytes))
    {
        // No progress has been made

        isProgressing = false;
    }
    else
    {
        // Data is flowing successfully

        isProgressing = true;
    }

    // Reset the value of the bytes written since the last probe into the tcp
    // outqueue was made and update the last tcp outqueue sample.

    mBytesWrittenSinceLastProbe = 0;

    mLastTCPKernelSendQueueLen = currPendingBytes;

    return CHIP_NO_ERROR;
}
#endif // INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

} // namespace Inet
} // namespace chip
