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
 * This file implements Inet::TCPEndPoint using sockets.
 */

#include <inet/TCPEndPointImplSockets.h>

#include <inet/InetFaultInjection.h>
#include <inet/arpa-inet-compatibility.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>
#include <system/SystemFaultInjection.h>

#include <stdio.h>
#include <string.h>
#include <utility>

#include <errno.h>
#include <fcntl.h>
#include <net/if.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

// SOCK_CLOEXEC not defined on all platforms, e.g. iOS/macOS:
#ifndef SOCK_CLOEXEC
#define SOCK_CLOEXEC 0
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

namespace chip {
namespace Inet {

CHIP_ERROR TCPEndPointImplSockets::BindImpl(IPAddressType addrType, const IPAddress & addr, uint16_t port, bool reuseAddr)
{
    CHIP_ERROR res = GetSocket(addrType);

    if (res == CHIP_NO_ERROR && reuseAddr)
    {
        int n = 1;
        setsockopt(mSocket, SOL_SOCKET, SO_REUSEADDR, &n, sizeof(n));

#ifdef SO_REUSEPORT
        // Enable SO_REUSEPORT.  This permits coexistence between an
        // untargetted CHIP client and other services that listen on
        // a CHIP port on a specific address (such as a CHIP client
        // with TARGETED_LISTEN or TCP proxying services).  Note that
        // one of the costs of this implementation is the
        // non-deterministic connection dispatch when multple clients
        // listen on the address with the same degreee of selectivity,
        // e.g. two untargetted-listen CHIP clients, or two
        // targeted-listen CHIP clients with the same node id.

        if (setsockopt(mSocket, SOL_SOCKET, SO_REUSEPORT, &n, sizeof(n)) != 0)
        {
            ChipLogError(Inet, "SO_REUSEPORT: %d", errno);
        }
#endif // defined(SO_REUSEPORT)
    }

    if (res == CHIP_NO_ERROR)
    {
        if (addrType == IPAddressType::kIPv6)
        {
            struct sockaddr_in6 sa;
            memset(&sa, 0, sizeof(sa));
            sa.sin6_family   = AF_INET6;
            sa.sin6_port     = htons(port);
            sa.sin6_flowinfo = 0;
            sa.sin6_addr     = addr.ToIPv6();
            sa.sin6_scope_id = 0;

            if (bind(mSocket, reinterpret_cast<const sockaddr *>(&sa), static_cast<unsigned>(sizeof(sa))) != 0)
            {
                res = CHIP_ERROR_POSIX(errno);
            }
        }
#if INET_CONFIG_ENABLE_IPV4
        else if (addrType == IPAddressType::kIPv4)
        {
            struct sockaddr_in sa;
            memset(&sa, 0, sizeof(sa));
            sa.sin_family = AF_INET;
            sa.sin_port   = htons(port);
            sa.sin_addr   = addr.ToIPv4();

            if (bind(mSocket, reinterpret_cast<const sockaddr *>(&sa), static_cast<unsigned>(sizeof(sa))) != 0)
            {
                res = CHIP_ERROR_POSIX(errno);
            }
        }
#endif // INET_CONFIG_ENABLE_IPV4
        else
        {
            res = INET_ERROR_WRONG_ADDRESS_TYPE;
        }
    }

    return res;
}

CHIP_ERROR TCPEndPointImplSockets::ListenImpl(uint16_t backlog)
{
    if (listen(mSocket, backlog) != 0)
    {
        return CHIP_ERROR_POSIX(errno);
    }

    // Enable non-blocking mode for the socket.
    int flags = fcntl(mSocket, F_GETFL, 0);
    fcntl(mSocket, F_SETFL, flags | O_NONBLOCK);

    // Wait for ability to read on this endpoint.
    CHIP_ERROR res = static_cast<System::LayerSockets &>(GetSystemLayer())
                         .SetCallback(mWatch, HandlePendingIO, reinterpret_cast<intptr_t>(this));
    if (res == CHIP_NO_ERROR)
    {
        res = static_cast<System::LayerSockets &>(GetSystemLayer()).RequestCallbackOnPendingRead(mWatch);
    }

    return res;
}

CHIP_ERROR TCPEndPointImplSockets::ConnectImpl(const IPAddress & addr, uint16_t port, InterfaceId intfId)
{
    IPAddressType addrType = addr.Type();

    ReturnErrorOnFailure(GetSocket(addrType));

    if (!intfId.IsPresent())
    {
        // The behavior when connecting to an IPv6 link-local address without specifying an outbound
        // interface is ambiguous. So prevent it in all cases.
        if (addr.IsIPv6LinkLocal())
        {
            return INET_ERROR_WRONG_ADDRESS_TYPE;
        }
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

            ReturnErrorOnFailure(intfId.GetInterfaceName(ifr.ifr_name, sizeof(ifr.ifr_name)));

            // Attempt to bind to the interface using SO_BINDTODEVICE which requires privileged access.
            // If the permission is denied(EACCES) because CHIP is running in a context
            // that does not have privileged access, choose a source address on the
            // interface to bind the connetion to.
            int r = setsockopt(mSocket, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof(ifr));
            if (r < 0 && errno != EACCES)
            {
                return CHIP_ERROR_POSIX(errno);
            }

            if (r < 0)
#endif // SO_BINDTODEVICE
            {
                // Attempting to initiate a connection via a specific interface is not allowed.
                // The only way to do this is to bind the local to an address on the desired
                // interface.
                ReturnErrorOnFailure(BindSrcAddrFromIntf(addrType, intfId));
            }
        }
    }

    // Disable generation of SIGPIPE.
#ifdef SO_NOSIGPIPE
    int n = 1;
    setsockopt(mSocket, SOL_SOCKET, SO_NOSIGPIPE, &n, sizeof(n));
#endif // defined(SO_NOSIGPIPE)

    // Enable non-blocking mode for the socket.
    int flags = fcntl(mSocket, F_GETFL, 0);
    fcntl(mSocket, F_SETFL, flags | O_NONBLOCK);

    socklen_t sockaddrsize       = 0;
    const sockaddr * sockaddrptr = nullptr;

    SockAddr sa;
    memset(&sa, 0, sizeof(sa));

    if (addrType == IPAddressType::kIPv6)
    {
        sa.in6.sin6_family   = AF_INET6;
        sa.in6.sin6_port     = htons(port);
        sa.in6.sin6_flowinfo = 0;
        sa.in6.sin6_addr     = addr.ToIPv6();
        sa.in6.sin6_scope_id = intfId.GetPlatformInterface();
        sockaddrsize         = sizeof(sockaddr_in6);
        sockaddrptr          = reinterpret_cast<const sockaddr *>(&sa.in6);
    }
#if INET_CONFIG_ENABLE_IPV4
    else if (addrType == IPAddressType::kIPv4)
    {
        sa.in.sin_family = AF_INET;
        sa.in.sin_port   = htons(port);
        sa.in.sin_addr   = addr.ToIPv4();
        sockaddrsize     = sizeof(sockaddr_in);
        sockaddrptr      = reinterpret_cast<const sockaddr *>(&sa.in);
    }
#endif // INET_CONFIG_ENABLE_IPV4
    else
    {
        return INET_ERROR_WRONG_ADDRESS_TYPE;
    }

    int conRes = connect(mSocket, sockaddrptr, sockaddrsize);

    if (conRes == -1 && errno != EINPROGRESS)
    {
        CHIP_ERROR res = CHIP_ERROR_POSIX(errno);
        DoClose(res, true);
        return res;
    }

    ReturnErrorOnFailure(static_cast<System::LayerSockets &>(GetSystemLayer())
                             .SetCallback(mWatch, HandlePendingIO, reinterpret_cast<intptr_t>(this)));

    // Once Connecting or Connected, bump the reference count.  The corresponding Release() will happen in DoClose().
    Retain();

    if (conRes == 0)
    {
        mState = State::kConnected;
        // Wait for ability to read on this endpoint.
        ReturnErrorOnFailure(static_cast<System::LayerSockets &>(GetSystemLayer()).RequestCallbackOnPendingRead(mWatch));
        if (OnConnectComplete != nullptr)
        {
            OnConnectComplete(this, CHIP_NO_ERROR);
        }
    }
    else
    {
        mState = State::kConnecting;
        // Wait for ability to write on this endpoint.
        ReturnErrorOnFailure(static_cast<System::LayerSockets &>(GetSystemLayer()).RequestCallbackOnPendingWrite(mWatch));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR TCPEndPointImplSockets::GetPeerInfo(IPAddress * retAddr, uint16_t * retPort) const
{
    return GetSocketInfo(getpeername, retAddr, retPort);
}

CHIP_ERROR TCPEndPointImplSockets::GetLocalInfo(IPAddress * retAddr, uint16_t * retPort) const
{
    return GetSocketInfo(getsockname, retAddr, retPort);
}

CHIP_ERROR TCPEndPointImplSockets::GetSocketInfo(int getname(int, sockaddr *, socklen_t *), IPAddress * retAddr,
                                                 uint16_t * retPort) const
{
    VerifyOrReturnError(IsConnected(), CHIP_ERROR_INCORRECT_STATE);

    SockAddr sa;
    memset(&sa, 0, sizeof(sa));
    socklen_t saLen = sizeof(sa);

    if (getname(mSocket, &sa.any, &saLen) != 0)
    {
        return CHIP_ERROR_POSIX(errno);
    }

    if (sa.any.sa_family == AF_INET6)
    {
        *retAddr = IPAddress(sa.in6.sin6_addr);
        *retPort = ntohs(sa.in6.sin6_port);
        return CHIP_NO_ERROR;
    }

#if INET_CONFIG_ENABLE_IPV4
    if (sa.any.sa_family == AF_INET)
    {
        *retAddr = IPAddress(sa.in.sin_addr);
        *retPort = ntohs(sa.in.sin_port);
        return CHIP_NO_ERROR;
    }
#endif // INET_CONFIG_ENABLE_IPV4

    return CHIP_ERROR_INCORRECT_STATE;
}

CHIP_ERROR TCPEndPointImplSockets::GetInterfaceId(InterfaceId * retInterface)
{
    VerifyOrReturnError(IsConnected(), CHIP_ERROR_INCORRECT_STATE);

    SockAddr sa;
    memset(&sa, 0, sizeof(sa));
    socklen_t saLen = sizeof(sa);

    if (getpeername(mSocket, &sa.any, &saLen) != 0)
    {
        return CHIP_ERROR_POSIX(errno);
    }

    if (sa.any.sa_family == AF_INET6)
    {
        if (IPAddress(sa.in6.sin6_addr).IsIPv6LinkLocal())
        {
            *retInterface = InterfaceId(sa.in6.sin6_scope_id);
        }
        else
        {
            // TODO: Is there still a meaningful interface id in this case?
            *retInterface = InterfaceId::Null();
        }
        return CHIP_NO_ERROR;
    }

#if INET_CONFIG_ENABLE_IPV4
    if (sa.any.sa_family == AF_INET)
    {
        // No interface id available for IPv4 sockets.
        *retInterface = InterfaceId::Null();
        return CHIP_NO_ERROR;
    }
#endif // INET_CONFIG_ENABLE_IPV4

    *retInterface = InterfaceId::Null();
    return INET_ERROR_WRONG_ADDRESS_TYPE;
}

CHIP_ERROR TCPEndPointImplSockets::SendQueuedImpl(bool queueWasEmpty)
{
    if (queueWasEmpty)
    {
        // Wait for ability to write on this endpoint.
        return static_cast<System::LayerSockets &>(GetSystemLayer()).RequestCallbackOnPendingWrite(mWatch);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR TCPEndPointImplSockets::EnableNoDelay()
{
    VerifyOrReturnError(IsConnected(), CHIP_ERROR_INCORRECT_STATE);

#ifdef TCP_NODELAY
    // Disable TCP Nagle buffering by setting TCP_NODELAY socket option to true
    int val = 1;
    if (setsockopt(mSocket, TCP_SOCKOPT_LEVEL, TCP_NODELAY, &val, sizeof(val)) != 0)
    {
        return CHIP_ERROR_POSIX(errno);
    }
#endif // defined(TCP_NODELAY)

    return CHIP_NO_ERROR;
}

CHIP_ERROR TCPEndPointImplSockets::EnableKeepAlive(uint16_t interval, uint16_t timeoutCount)
{
    VerifyOrReturnError(IsConnected(), CHIP_ERROR_INCORRECT_STATE);

    // Set the idle interval
    int val = interval;
    if (setsockopt(mSocket, TCP_SOCKOPT_LEVEL, TCP_IDLE_INTERVAL_OPT_NAME, &val, sizeof(val)) != 0)
    {
        return CHIP_ERROR_POSIX(errno);
    }

    // Set the probe retransmission interval.
    val = interval;
    if (setsockopt(mSocket, TCP_SOCKOPT_LEVEL, TCP_KEEPINTVL, &val, sizeof(val)) != 0)
    {
        return CHIP_ERROR_POSIX(errno);
    }

    // Set the probe timeout count
    val = timeoutCount;
    if (setsockopt(mSocket, TCP_SOCKOPT_LEVEL, TCP_KEEPCNT, &val, sizeof(val)) != 0)
    {
        return CHIP_ERROR_POSIX(errno);
    }

    // Enable keepalives for the connection.
    val = 1; // enable
    if (setsockopt(mSocket, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof(val)) != 0)
    {
        return CHIP_ERROR_POSIX(errno);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR TCPEndPointImplSockets::DisableKeepAlive()
{
    VerifyOrReturnError(IsConnected(), CHIP_ERROR_INCORRECT_STATE);

    // Disable keepalives on the connection.
    int val = 0; // disable
    if (setsockopt(mSocket, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof(val)) != 0)
    {
        return CHIP_ERROR_POSIX(errno);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR TCPEndPointImplSockets::AckReceive(uint16_t len)
{
    VerifyOrReturnError(IsConnected(), CHIP_ERROR_INCORRECT_STATE);

    // nothing to do for sockets case
    return CHIP_NO_ERROR;
}

CHIP_ERROR TCPEndPointImplSockets::SetUserTimeoutImpl(uint32_t userTimeoutMillis)
{
#if defined(TCP_USER_TIMEOUT)
    // Set the user timeout
    uint32_t val = userTimeoutMillis;
    if (setsockopt(mSocket, TCP_SOCKOPT_LEVEL, TCP_USER_TIMEOUT, &val, sizeof(val)) != 0)
    {
        return CHIP_ERROR_POSIX(errno);
    }
    return CHIP_NO_ERROR;
#else  // TCP_USER_TIMEOUT
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif // defined(TCP_USER_TIMEOUT)
}

CHIP_ERROR TCPEndPointImplSockets::DriveSendingImpl()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

#ifdef MSG_NOSIGNAL
    const int sendFlags = MSG_NOSIGNAL;
#else
    const int sendFlags = 0;
#endif

    // Pretend send() fails in the while loop below
    INET_FAULT_INJECT(FaultInjection::kFault_Send, {
        err = CHIP_ERROR_POSIX(EIO);
        DoClose(err, false);
        return err;
    });

    while (!mSendQueue.IsNull())
    {
        uint16_t bufLen = mSendQueue->DataLength();

        ssize_t lenSentRaw = send(mSocket, mSendQueue->Start(), bufLen, sendFlags);

        if (lenSentRaw == -1)
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
                err = (errno == EPIPE) ? INET_ERROR_PEER_DISCONNECTED : CHIP_ERROR_POSIX(errno);
            }
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
                err = static_cast<System::LayerSockets &>(GetSystemLayer()).ClearCallbackOnPendingWrite(mWatch);
                if (err != CHIP_NO_ERROR)
                {
                    break;
                }
            }
        }

        if (OnDataSent != nullptr)
        {
            OnDataSent(this, lenSent);
        }

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
        {
            break;
        }
    }

    if (err == CHIP_NO_ERROR)
    {
        // If we're in the SendShutdown state and the send queue is now empty, shutdown writing on the socket.
        if (mState == State::kSendShutdown && mSendQueue.IsNull())
        {
            if (shutdown(mSocket, SHUT_WR) != 0)
            {
                err = CHIP_ERROR_POSIX(errno);
            }
        }
    }

    return err;
}

void TCPEndPointImplSockets::HandleConnectCompleteImpl()
{
    // Wait for ability to read or write on this endpoint.
    CHIP_ERROR err = static_cast<System::LayerSockets &>(GetSystemLayer()).RequestCallbackOnPendingRead(mWatch);
    if (err == CHIP_NO_ERROR)
    {
        err = static_cast<System::LayerSockets &>(GetSystemLayer()).RequestCallbackOnPendingWrite(mWatch);
    }
    if (err != CHIP_NO_ERROR)
    {
        DoClose(err, false);
        return;
    }
}

void TCPEndPointImplSockets::DoCloseImpl(CHIP_ERROR err, State oldState)
{
    struct linger lingerStruct;

    // If the socket hasn't been closed already...
    if (mSocket != kInvalidSocketFd)
    {
        // If entering the Closed state
        // OR if entering the Closing state, and there's no unsent data in the send queue
        // THEN close the socket.
        if (mState == State::kClosed || (mState == State::kClosing && mSendQueue.IsNull()))
        {
            // If aborting the connection, ensure we send a TCP RST.
            if (IsConnected(oldState) && err != CHIP_NO_ERROR)
            {
                lingerStruct.l_onoff  = 1;
                lingerStruct.l_linger = 0;

                if (setsockopt(mSocket, SOL_SOCKET, SO_LINGER, &lingerStruct, sizeof(lingerStruct)) != 0)
                {
                    ChipLogError(Inet, "SO_LINGER: %d", errno);
                }
            }

            static_cast<System::LayerSockets &>(GetSystemLayer()).StopWatchingSocket(&mWatch);
            close(mSocket);
            mSocket = kInvalidSocketFd;
        }
    }
}

#if INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT
void TCPEndPointImplSockets::TCPUserTimeoutHandler()
{
    // Set the timer running flag to false
    mUserTimeoutTimerRunning = false;

    bool isProgressing = false;
    CHIP_ERROR err     = CheckConnectionProgress(isProgressing);

    if (err == CHIP_NO_ERROR && mLastTCPKernelSendQueueLen != 0)
    {
        // There is data in the TCP Send Queue
        if (isProgressing)
        {
            // Data is flowing, so restart the UserTimeout timer
            // to shift it forward while also resetting the max
            // poll count.

            StartTCPUserTimeoutTimer();
        }
        else
        {
            // Close the connection as the TCP UserTimeout has expired
            err = INET_ERROR_TCP_USER_TIMEOUT;
        }
    }

    if (err != CHIP_NO_ERROR)
    {
        // Close the connection as the TCP UserTimeout has expired
        DoClose(err, false);
    }
}
#endif // INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT

CHIP_ERROR TCPEndPointImplSockets::BindSrcAddrFromIntf(IPAddressType addrType, InterfaceId intfId)
{
    // If we are trying to make a TCP connection over a 'specified target interface',
    // then we bind the TCPEndPoint to an IP address on that target interface
    // and use that address as the source address for that connection. This is
    // done in the event that directly binding the connection to the target
    // interface is not allowed due to insufficient privileges.
    VerifyOrReturnError(mState != State::kBound, CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    bool ipAddrFound = false;
    for (InterfaceAddressIterator addrIter; addrIter.HasCurrent(); addrIter.Next())
    {
        IPAddress curAddr;
        if ((addrIter.GetInterfaceId() == intfId) && (addrIter.GetAddress(curAddr) == CHIP_NO_ERROR))
        {
            // Search for an IPv4 address on the TargetInterface

#if INET_CONFIG_ENABLE_IPV4
            if (addrType == IPAddressType::kIPv4)
            {
                if (curAddr.IsIPv4())
                {
                    // Bind to the IPv4 address of the TargetInterface
                    ipAddrFound = true;
                    ReturnErrorOnFailure(Bind(IPAddressType::kIPv4, curAddr, 0, true));

                    break;
                }
            }
#endif // INET_CONFIG_ENABLE_IPV4
            if (addrType == IPAddressType::kIPv6)
            {
                // Select an IPv6 address on the interface that is not
                // a link local or a multicast address.
                // TODO: Define a proper IPv6GlobalUnicast address checker.
                if (!curAddr.IsIPv4() && !curAddr.IsIPv6LinkLocal() && !curAddr.IsMulticast())
                {
                    // Bind to the IPv6 address of the TargetInterface
                    ipAddrFound = true;
                    ReturnErrorOnFailure(Bind(IPAddressType::kIPv6, curAddr, 0, true));

                    break;
                }
            }
        }
    }

    VerifyOrReturnError(ipAddrFound, CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    return CHIP_NO_ERROR;
}

CHIP_ERROR TCPEndPointImplSockets::GetSocket(IPAddressType addrType)
{
    if (mSocket == kInvalidSocketFd)
    {
        int family;
        if (addrType == IPAddressType::kIPv6)
        {
            family = PF_INET6;
#if INET_CONFIG_ENABLE_IPV4
        }
        else if (addrType == IPAddressType::kIPv4)
        {
            family = PF_INET;
#endif // INET_CONFIG_ENABLE_IPV4
        }
        else
        {
            return INET_ERROR_WRONG_ADDRESS_TYPE;
        }
        mSocket = ::socket(family, SOCK_STREAM | SOCK_CLOEXEC, 0);
        if (mSocket == -1)
        {
            return CHIP_ERROR_POSIX(errno);
        }
        ReturnErrorOnFailure(static_cast<System::LayerSockets &>(GetSystemLayer()).StartWatchingSocket(mSocket, &mWatch));
        mAddrType = addrType;

        // If creating an IPv6 socket, tell the kernel that it will be IPv6 only.  This makes it
        // posible to bind two sockets to the same port, one for IPv4 and one for IPv6.
#ifdef IPV6_V6ONLY
        if (family == PF_INET6)
        {
            int one = 1;
            setsockopt(mSocket, IPPROTO_IPV6, IPV6_V6ONLY, &one, sizeof(one));
        }
#endif // defined(IPV6_V6ONLY)

        // On systems that support it, disable the delivery of SIGPIPE signals when writing to a closed
        // socket.
#ifdef SO_NOSIGPIPE
        {
            int one = 1;
            int res = setsockopt(mSocket, SOL_SOCKET, SO_NOSIGPIPE, &one, sizeof(one));
            if (res != 0)
            {
                ChipLogError(Inet, "SO_NOSIGPIPE: %d", errno);
            }
        }
#endif // defined(SO_NOSIGPIPE)
    }
    else if (mAddrType != addrType)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    return CHIP_NO_ERROR;
}

// static
void TCPEndPointImplSockets::HandlePendingIO(System::SocketEvents events, intptr_t data)
{
    reinterpret_cast<TCPEndPointImplSockets *>(data)->HandlePendingIO(events);
}

void TCPEndPointImplSockets::HandlePendingIO(System::SocketEvents events)
{
    // Prevent the end point from being freed while in the middle of a callback.
    Retain();

    // If in the Listening state, and the app is ready to receive a connection, and there is a connection
    // ready to be received on the socket, process the incoming connection.
    if (mState == State::kListening)
    {
        if (OnConnectionReceived != nullptr && events.Has(System::SocketEventFlags::kRead))
        {
            HandleIncomingConnection();
        }
    }

    // If in the processes of initiating a connection...
    else if (mState == State::kConnecting)
    {
        // The socket being writable indicates the connection has completed (successfully or otherwise).
        if (events.Has(System::SocketEventFlags::kWrite))
        {
#if !__MBED__
            // Get the connection result from the socket.
            int osConRes;
            socklen_t optLen = sizeof(osConRes);
            if (getsockopt(mSocket, SOL_SOCKET, SO_ERROR, &osConRes, &optLen) != 0)
            {
                osConRes = errno;
            }
#else
            // On Mbed OS, connect blocks and never returns EINPROGRESS
            // The socket option SO_ERROR is not available.
            int osConRes     = 0;
#endif
            CHIP_ERROR conRes = CHIP_ERROR_POSIX(osConRes);

            // Process the connection result.
            HandleConnectComplete(conRes);
        }
    }

    else
    {
        // If in a state where sending is allowed, and there is data to be sent, and the socket is ready for
        // writing, drive outbound data into the connection.
        if (IsConnected() && !mSendQueue.IsNull() && events.Has(System::SocketEventFlags::kWrite))
        {
            DriveSending();
        }

        // If in a state were receiving is allowed, and the app is ready to receive data, and data is ready
        // on the socket, receive inbound data from the connection.
        if ((mState == State::kConnected || mState == State::kSendShutdown) && mReceiveEnabled && OnDataReceived != nullptr &&
            events.Has(System::SocketEventFlags::kRead))
        {
            ReceiveData();
        }
    }

    Release();
}

void TCPEndPointImplSockets::ReceiveData()
{
    System::PacketBufferHandle rcvBuf;
    bool isNewBuf = true;

    if (mRcvQueue.IsNull())
    {
        rcvBuf = System::PacketBufferHandle::New(kMaxReceiveMessageSize, 0);
    }
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
    ssize_t rcvLen = recv(mSocket, rcvBuf->Start() + rcvBuf->DataLength(), rcvBuf->AvailableDataLength(), 0);

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

        DoClose(CHIP_ERROR_POSIX(systemErrno), false);
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
            if (mState == State::kConnected && OnPeerClose != nullptr)
            {
                mState = State::kReceiveShutdown;
            }
            else
            {
                mState = State::kClosing;
            }
            // Do not wait for ability to read on this endpoint.
            (void) static_cast<System::LayerSockets &>(GetSystemLayer()).ClearCallbackOnPendingRead(mWatch);
            // Call the app's OnPeerClose.
            if (OnPeerClose != nullptr)
            {
                OnPeerClose(this);
            }
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
                {
                    mRcvQueue = std::move(rcvBuf);
                }
                else
                {
                    mRcvQueue->AddToEnd(std::move(rcvBuf));
                }
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

void TCPEndPointImplSockets::HandleIncomingConnection()
{
    CHIP_ERROR err                 = CHIP_NO_ERROR;
    TCPEndPointImplSockets * conEP = nullptr;
    IPAddress peerAddr;
    uint16_t peerPort;

    SockAddr sa;
    memset(&sa, 0, sizeof(sa));
    socklen_t saLen = sizeof(sa);

    // Accept the new connection.
    int conSocket = accept(mSocket, &sa.any, &saLen);
    if (conSocket == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            return;
        }

        err = CHIP_ERROR_POSIX(errno);
    }

    // If there's no callback available, fail with an error.
    if (err == CHIP_NO_ERROR && OnConnectionReceived == nullptr)
    {
        err = CHIP_ERROR_NO_CONNECTION_HANDLER;
    }

    // Extract the peer's address information.
    if (err == CHIP_NO_ERROR)
    {
        if (sa.any.sa_family == AF_INET6)
        {
            peerAddr = IPAddress(sa.in6.sin6_addr);
            peerPort = ntohs(sa.in6.sin6_port);
        }
#if INET_CONFIG_ENABLE_IPV4
        else if (sa.any.sa_family == AF_INET)
        {
            peerAddr = IPAddress(sa.in.sin_addr);
            peerPort = ntohs(sa.in.sin_port);
        }
#endif // INET_CONFIG_ENABLE_IPV4
        else
        {
            err = CHIP_ERROR_INCORRECT_STATE;
        }
    }

    // Attempt to allocate an end point object.
    if (err == CHIP_NO_ERROR)
    {
        TCPEndPoint * connectEndPoint = nullptr;
        err                           = GetEndPointManager().NewEndPoint(&connectEndPoint);
        conEP                         = static_cast<TCPEndPointImplSockets *>(connectEndPoint);
    }

    // If all went well...
    if (err == CHIP_NO_ERROR)
    {
        // Put the new end point into the Connected state.
        conEP->mSocket = conSocket;
        err            = static_cast<System::LayerSockets &>(GetSystemLayer()).StartWatchingSocket(conSocket, &conEP->mWatch);
        if (err == CHIP_NO_ERROR)
        {
            conEP->mState = State::kConnected;
#if INET_CONFIG_ENABLE_IPV4
            conEP->mAddrType = (sa.any.sa_family == AF_INET6) ? IPAddressType::kIPv6 : IPAddressType::kIPv4;
#else  // !INET_CONFIG_ENABLE_IPV4
            conEP->mAddrType = IPAddressType::kIPv6;
#endif // !INET_CONFIG_ENABLE_IPV4
            conEP->Retain();

            // Wait for ability to read on this endpoint.
            auto & conEPLayer = static_cast<System::LayerSockets &>(conEP->GetSystemLayer());
            err               = conEPLayer.SetCallback(conEP->mWatch, HandlePendingIO, reinterpret_cast<intptr_t>(conEP));
            if (err == CHIP_NO_ERROR)
            {
                err = conEPLayer.RequestCallbackOnPendingRead(conEP->mWatch);
            }
            if (err == CHIP_NO_ERROR)
            {
                // Call the app's callback function.
                OnConnectionReceived(this, conEP, peerAddr, peerPort);
                return;
            }
        }
    }

    // Otherwise immediately close the connection, clean up and call the app's error callback.
    if (conSocket != -1)
    {
        close(conSocket);
    }
    if (conEP != nullptr)
    {
        if (conEP->mState == State::kConnected)
        {
            conEP->Release();
        }
        conEP->Release();
    }
    if (OnAcceptError != nullptr)
    {
        OnAcceptError(this, err);
    }
}

#if INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT
/**
 *  This function probes the TCP output queue and checks if data is successfully
 *  being transferred to the other end.
 */
CHIP_ERROR TCPEndPointImplSockets::CheckConnectionProgress(bool & isProgressing)
{
    int currPendingBytesRaw = 0;
    uint32_t currPendingBytes; // Will be initialized once we know it's safe.

    // Fetch the bytes pending successful transmission in the TCP out queue.

#ifdef __APPLE__
    socklen_t len = sizeof(currPendingBytesRaw);
    if (getsockopt(mSocket, SOL_SOCKET, SO_NWRITE, &currPendingBytesRaw, &len) < 0)
#else
    if (ioctl(mSocket, TIOCOUTQ, &currPendingBytesRaw) < 0)
#endif
    {
        return CHIP_ERROR_POSIX(errno);
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

} // namespace Inet
} // namespace chip
