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
 * This file implements Inet::UDPEndPoint using sockets.
 */

// Required to properly support underlying RFC3542-related fields to IPV6_PKTINFO
// on Darwin.
#define __APPLE_USE_RFC_3542
#include <inet/UDPEndPointImplSockets.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>

#if CHIP_SYSTEM_CONFIG_USE_POSIX_SOCKETS
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif // HAVE_SYS_SOCKET_H
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#endif // CHIP_SYSTEM_CONFIG_USE_POSIX_SOCKETS

#if CHIP_SYSTEM_CONFIG_USE_ZEPHYR_SOCKETS || CHIP_SYSTEM_CONFIG_USE_ZEPHYR_SOCKET_EXTENSIONS
#include "ZephyrSocket.h" // nogncheck
#endif

#include <cerrno>
#include <unistd.h>
#include <utility>

// SOCK_CLOEXEC not defined on all platforms, e.g. iOS/macOS:
#ifndef SOCK_CLOEXEC
#define SOCK_CLOEXEC 0
#endif

// On MbedOS, INADDR_ANY does not seem to exist...
#ifndef INADDR_ANY
#define INADDR_ANY 0
#endif

/*
 * Some systems define both IPV6_{ADD,DROP}_MEMBERSHIP and
 * IPV6_{JOIN,LEAVE}_GROUP while others only define
 * IPV6_{JOIN,LEAVE}_GROUP. Prefer the "_MEMBERSHIP" flavor for
 * parallelism with IPv4 and create the alias to the availabile
 * definitions.
 */
#if defined(IPV6_ADD_MEMBERSHIP)
#define INET_IPV6_ADD_MEMBERSHIP IPV6_ADD_MEMBERSHIP
#elif defined(IPV6_JOIN_GROUP)
#define INET_IPV6_ADD_MEMBERSHIP IPV6_JOIN_GROUP
#elif !CHIP_SYSTEM_CONFIG_USE_PLATFORM_MULTICAST_API
#error                                                                                                                             \
    "Neither IPV6_ADD_MEMBERSHIP nor IPV6_JOIN_GROUP are defined which are required for generalized IPv6 multicast group support."
#endif // IPV6_ADD_MEMBERSHIP

#if defined(IPV6_DROP_MEMBERSHIP)
#define INET_IPV6_DROP_MEMBERSHIP IPV6_DROP_MEMBERSHIP
#elif defined(IPV6_LEAVE_GROUP)
#define INET_IPV6_DROP_MEMBERSHIP IPV6_LEAVE_GROUP
#elif !CHIP_SYSTEM_CONFIG_USE_PLATFORM_MULTICAST_API
#error                                                                                                                             \
    "Neither IPV6_DROP_MEMBERSHIP nor IPV6_LEAVE_GROUP are defined which are required for generalized IPv6 multicast group support."
#endif // IPV6_DROP_MEMBERSHIP

namespace chip {
namespace Inet {

namespace {

CHIP_ERROR IPv6Bind(int socket, const IPAddress & address, uint16_t port, InterfaceId interface)
{
    struct sockaddr_in6 sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin6_family                        = AF_INET6;
    sa.sin6_port                          = htons(port);
    sa.sin6_addr                          = address.ToIPv6();
    InterfaceId::PlatformType interfaceId = interface.GetPlatformInterface();
    if (!CanCastTo<decltype(sa.sin6_scope_id)>(interfaceId))
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    sa.sin6_scope_id = static_cast<decltype(sa.sin6_scope_id)>(interfaceId);

    CHIP_ERROR status = CHIP_NO_ERROR;

    // NOLINTNEXTLINE(clang-analyzer-unix.StdCLibraryFunctions): Function called only with valid socket after GetSocket
    if (bind(socket, reinterpret_cast<const sockaddr *>(&sa), static_cast<unsigned>(sizeof(sa))) != 0)
    {
        status = CHIP_ERROR_POSIX(errno);
    }
    else
    {
#ifdef IPV6_MULTICAST_IF
        // Instruct the kernel that any messages to multicast destinations should be
        // sent down the interface specified by the caller.
        setsockopt(socket, IPPROTO_IPV6, IPV6_MULTICAST_IF, &interfaceId, sizeof(interfaceId));
#endif // defined(IPV6_MULTICAST_IF)
    }

#ifdef IPV6_MULTICAST_HOPS
    // Instruct the kernel that any messages to multicast destinations should be
    // set with the configured hop limit value.
    int hops = INET_CONFIG_IP_MULTICAST_HOP_LIMIT;
    setsockopt(socket, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &hops, sizeof(hops));
#endif // defined(IPV6_MULTICAST_HOPS)

    return status;
}

#if INET_CONFIG_ENABLE_IPV4
CHIP_ERROR IPv4Bind(int socket, const IPAddress & address, uint16_t port)
{
    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port   = htons(port);
    sa.sin_addr   = address.ToIPv4();

    CHIP_ERROR status = CHIP_NO_ERROR;

    // NOLINTNEXTLINE(clang-analyzer-unix.StdCLibraryFunctions): Function called only with valid socket after GetSocket
    if (bind(socket, reinterpret_cast<const sockaddr *>(&sa), static_cast<unsigned>(sizeof(sa))) != 0)
    {
        status = CHIP_ERROR_POSIX(errno);
    }
    else
    {
        // Allow socket transmitting broadcast packets.
        constexpr int enable = 1;
        setsockopt(socket, SOL_SOCKET, SO_BROADCAST, &enable, sizeof(enable));

#ifdef IP_MULTICAST_IF
        // Instruct the kernel that any messages to multicast destinations should be
        // sent down the interface to which the specified IPv4 address is bound.
        setsockopt(socket, IPPROTO_IP, IP_MULTICAST_IF, &sa, sizeof(sa));
#endif // defined(IP_MULTICAST_IF)
    }

#ifdef IP_MULTICAST_TTL
    // Instruct the kernel that any messages to multicast destinations should be
    // set with the configured hop limit value.
    constexpr int ttl = INET_CONFIG_IP_MULTICAST_HOP_LIMIT;
    setsockopt(socket, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));
#endif // defined(IP_MULTICAST_TTL)

    return status;
}
#endif // INET_CONFIG_ENABLE_IPV4

} // anonymous namespace

#if CHIP_SYSTEM_CONFIG_USE_PLATFORM_MULTICAST_API
UDPEndPointImplSockets::MulticastGroupHandler UDPEndPointImplSockets::sMulticastGroupHandler;
#endif // CHIP_SYSTEM_CONFIG_USE_PLATFORM_MULTICAST_API

CHIP_ERROR UDPEndPointImplSockets::BindImpl(IPAddressType addressType, const IPAddress & addr, uint16_t port, InterfaceId interface)
{
    // Make sure we have the appropriate type of socket.
    ReturnErrorOnFailure(GetSocket(addressType));

    if (addressType == IPAddressType::kIPv6)
    {
        ReturnErrorOnFailure(IPv6Bind(mSocket, addr, port, interface));
    }
#if INET_CONFIG_ENABLE_IPV4
    else if (addressType == IPAddressType::kIPv4)
    {
        ReturnErrorOnFailure(IPv4Bind(mSocket, addr, port));
    }
#endif // INET_CONFIG_ENABLE_IPV4
    else
    {
        return INET_ERROR_WRONG_ADDRESS_TYPE;
    }

    mBoundPort   = port;
    mBoundIntfId = interface;

    // If an ephemeral port was requested, retrieve the actual bound port.
    if (port == 0)
    {
        SockAddr boundAddr;
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

    return CHIP_NO_ERROR;
}

CHIP_ERROR UDPEndPointImplSockets::BindInterfaceImpl(IPAddressType addressType, InterfaceId interfaceId)
{
    // Make sure we have the appropriate type of socket.
    ReturnErrorOnFailure(GetSocket(addressType));

#if HAVE_SO_BINDTODEVICE
    CHIP_ERROR status = CHIP_NO_ERROR;

    if (interfaceId.IsPresent())
    {
        // Start filtering on the passed interface.
        char interfaceName[IF_NAMESIZE];
        if (if_indextoname(interfaceId.GetPlatformInterface(), interfaceName) == nullptr)
        {
            status = CHIP_ERROR_POSIX(errno);
        }
        else if (setsockopt(mSocket, SOL_SOCKET, SO_BINDTODEVICE, interfaceName, socklen_t(strlen(interfaceName))) == -1)
        {
            status = CHIP_ERROR_POSIX(errno);
        }
    }
    else
    {
        // Stop interface-based filtering.
        if (setsockopt(mSocket, SOL_SOCKET, SO_BINDTODEVICE, "", 0) == -1)
        {
            status = CHIP_ERROR_POSIX(errno);
        }
    }

    if (status == CHIP_NO_ERROR)
    {
        mBoundIntfId = interfaceId;
    }

    return status;
#else  // !HAVE_SO_BINDTODEVICE
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif // HAVE_SO_BINDTODEVICE
}

InterfaceId UDPEndPointImplSockets::GetBoundInterface() const
{
    return mBoundIntfId;
}

uint16_t UDPEndPointImplSockets::GetBoundPort() const
{
    return mBoundPort;
}

CHIP_ERROR UDPEndPointImplSockets::ListenImpl()
{
    // Wait for ability to read on this endpoint.
    auto * layer = static_cast<System::LayerSockets *>(&GetSystemLayer());
    ReturnErrorOnFailure(layer->SetCallback(mWatch, HandlePendingIO, reinterpret_cast<intptr_t>(this)));
    return layer->RequestCallbackOnPendingRead(mWatch);
}

CHIP_ERROR UDPEndPointImplSockets::SendMsgImpl(const IPPacketInfo * aPktInfo, System::PacketBufferHandle && msg)
{
    // Ensure packet buffer is not null
    VerifyOrReturnError(!msg.IsNull(), CHIP_ERROR_INVALID_ARGUMENT);

    // Make sure we have the appropriate type of socket based on the
    // destination address.
    ReturnErrorOnFailure(GetSocket(aPktInfo->DestAddress.Type()));

    // Ensure the destination address type is compatible with the endpoint address type.
    VerifyOrReturnError(mAddrType == aPktInfo->DestAddress.Type(), CHIP_ERROR_INVALID_ARGUMENT);

    // For now the entire message must fit within a single buffer.
    VerifyOrReturnError(!msg->HasChainedBuffer(), CHIP_ERROR_MESSAGE_TOO_LONG);

    struct iovec msgIOV;
    msgIOV.iov_base = msg->Start();
    msgIOV.iov_len  = msg->DataLength();

#if defined(IP_PKTINFO) || defined(IPV6_PKTINFO)
    uint8_t controlData[256];
    memset(controlData, 0, sizeof(controlData));
#endif // defined(IP_PKTINFO) || defined(IPV6_PKTINFO)

    struct msghdr msgHeader;
    memset(&msgHeader, 0, sizeof(msgHeader));
    msgHeader.msg_iov    = &msgIOV;
    msgHeader.msg_iovlen = 1;

    // Construct a sockaddr_in/sockaddr_in6 structure containing the destination information.
    SockAddr peerSockAddr;
    memset(&peerSockAddr, 0, sizeof(peerSockAddr));
    msgHeader.msg_name = &peerSockAddr;
    if (mAddrType == IPAddressType::kIPv6)
    {
        peerSockAddr.in6.sin6_family     = AF_INET6;
        peerSockAddr.in6.sin6_port       = htons(aPktInfo->DestPort);
        peerSockAddr.in6.sin6_addr       = aPktInfo->DestAddress.ToIPv6();
        InterfaceId::PlatformType intfId = aPktInfo->Interface.GetPlatformInterface();
        VerifyOrReturnError(CanCastTo<decltype(peerSockAddr.in6.sin6_scope_id)>(intfId), CHIP_ERROR_INCORRECT_STATE);
        peerSockAddr.in6.sin6_scope_id = static_cast<decltype(peerSockAddr.in6.sin6_scope_id)>(intfId);
        msgHeader.msg_namelen          = sizeof(sockaddr_in6);
    }
#if INET_CONFIG_ENABLE_IPV4
    else
    {
        peerSockAddr.in.sin_family = AF_INET;
        peerSockAddr.in.sin_port   = htons(aPktInfo->DestPort);
        peerSockAddr.in.sin_addr   = aPktInfo->DestAddress.ToIPv4();
        msgHeader.msg_namelen      = sizeof(sockaddr_in);
    }
#endif // INET_CONFIG_ENABLE_IPV4

    // If the endpoint has been bound to a particular interface,
    // and the caller didn't supply a specific interface to send
    // on, use the bound interface. This appears to be necessary
    // for messages to multicast addresses, which under Linux
    // don't seem to get sent out the correct interface, despite
    // the socket being bound.
    InterfaceId intf = aPktInfo->Interface;
    if (!intf.IsPresent())
    {
        intf = mBoundIntfId;
    }

#if INET_CONFIG_UDP_SOCKET_PKTINFO
    // If the packet should be sent over a specific interface, or with a specific source
    // address, construct an IP_PKTINFO/IPV6_PKTINFO "control message" to that effect
    // add add it to the message header.  If the local OS doesn't support IP_PKTINFO/IPV6_PKTINFO
    // fail with an error.
    if (intf.IsPresent() || aPktInfo->SrcAddress.Type() != IPAddressType::kAny)
    {
#if defined(IP_PKTINFO) || defined(IPV6_PKTINFO)
        msgHeader.msg_control    = controlData;
        msgHeader.msg_controllen = sizeof(controlData);

        struct cmsghdr * controlHdr      = CMSG_FIRSTHDR(&msgHeader);
        InterfaceId::PlatformType intfId = intf.GetPlatformInterface();

#if INET_CONFIG_ENABLE_IPV4

        if (mAddrType == IPAddressType::kIPv4)
        {
#if defined(IP_PKTINFO)
            controlHdr->cmsg_level = IPPROTO_IP;
            controlHdr->cmsg_type  = IP_PKTINFO;
            controlHdr->cmsg_len   = CMSG_LEN(sizeof(in_pktinfo));

            auto * pktInfo = reinterpret_cast<struct in_pktinfo *> CMSG_DATA(controlHdr);
            if (!CanCastTo<decltype(pktInfo->ipi_ifindex)>(intfId))
            {
                return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
            }

            pktInfo->ipi_ifindex  = static_cast<decltype(pktInfo->ipi_ifindex)>(intfId);
            pktInfo->ipi_spec_dst = aPktInfo->SrcAddress.ToIPv4();

            msgHeader.msg_controllen = CMSG_SPACE(sizeof(in_pktinfo));
#else  // !defined(IP_PKTINFO)
            return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif // !defined(IP_PKTINFO)
        }

#endif // INET_CONFIG_ENABLE_IPV4

        if (mAddrType == IPAddressType::kIPv6)
        {
#if defined(IPV6_PKTINFO)
            controlHdr->cmsg_level = IPPROTO_IPV6;
            controlHdr->cmsg_type  = IPV6_PKTINFO;
            controlHdr->cmsg_len   = CMSG_LEN(sizeof(in6_pktinfo));

            auto * pktInfo = reinterpret_cast<struct in6_pktinfo *> CMSG_DATA(controlHdr);
            if (!CanCastTo<decltype(pktInfo->ipi6_ifindex)>(intfId))
            {
                return CHIP_ERROR_UNEXPECTED_EVENT;
            }
            pktInfo->ipi6_ifindex = static_cast<decltype(pktInfo->ipi6_ifindex)>(intfId);
            pktInfo->ipi6_addr    = aPktInfo->SrcAddress.ToIPv6();

            msgHeader.msg_controllen = CMSG_SPACE(sizeof(in6_pktinfo));
#else  // !defined(IPV6_PKTINFO)
            return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif // !defined(IPV6_PKTINFO)
        }

#else  // !(defined(IP_PKTINFO) && defined(IPV6_PKTINFO))
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif // !(defined(IP_PKTINFO) && defined(IPV6_PKTINFO))
    }
#endif // INET_CONFIG_UDP_SOCKET_PKTINFO

    // Send IP packet.
    // NOLINTNEXTLINE(clang-analyzer-unix.StdCLibraryFunctions): GetSocket calls ensure mSocket is valid
    const ssize_t lenSent = sendmsg(mSocket, &msgHeader, 0);
    if (lenSent == -1)
    {
        return CHIP_ERROR_POSIX(errno);
    }

    size_t len = static_cast<size_t>(lenSent);

    if (len != msg->DataLength())
    {
        return CHIP_ERROR_OUTBOUND_MESSAGE_TOO_BIG;
    }
    return CHIP_NO_ERROR;
}

void UDPEndPointImplSockets::CloseImpl()
{
    if (mSocket != kInvalidSocketFd)
    {
        static_cast<System::LayerSockets *>(&GetSystemLayer())->StopWatchingSocket(&mWatch);
        close(mSocket);
        mSocket = kInvalidSocketFd;
    }
}

void UDPEndPointImplSockets::Free()
{
    Close();
    Release();
}

CHIP_ERROR UDPEndPointImplSockets::GetSocket(IPAddressType addressType)
{
    if (mSocket == kInvalidSocketFd)
    {
        constexpr int type     = (SOCK_DGRAM | SOCK_CLOEXEC);
        constexpr int protocol = 0;

        int family = PF_UNSPEC;

        switch (addressType)
        {
        case IPAddressType::kIPv6:
            family = PF_INET6;
            break;

#if INET_CONFIG_ENABLE_IPV4
        case IPAddressType::kIPv4:
            family = PF_INET;
            break;
#endif // INET_CONFIG_ENABLE_IPV4

        default:
            return INET_ERROR_WRONG_ADDRESS_TYPE;
        }

        mSocket = ::socket(family, type, protocol);
        if (mSocket == -1)
        {
            return CHIP_ERROR_POSIX(errno);
        }
        CHIP_ERROR err = static_cast<System::LayerSockets *>(&GetSystemLayer())->StartWatchingSocket(mSocket, &mWatch);
        if (err != CHIP_NO_ERROR)
        {
            // Our mWatch is not valid; make sure we never use it.
            close(mSocket);
            mSocket = kInvalidSocketFd;
            return err;
        }

        mAddrType = addressType;

        // NOTE WELL: the errors returned by setsockopt() here are not
        // returned as Inet layer CHIP_ERROR_POSIX(errno)
        // codes because they are normally expected to fail on some
        // platforms where the socket option code is defined in the
        // header files but not [yet] implemented. Certainly, there is
        // room to improve this by connecting the build configuration
        // logic up to check for implementations of these options and
        // to provide appropriate HAVE_xxxxx definitions accordingly.

        constexpr int one = 1;
        int res           = setsockopt(mSocket, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
        static_cast<void>(res);

#ifdef SO_REUSEPORT
        res = setsockopt(mSocket, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(one));
        if (res != 0)
        {
            ChipLogError(Inet, "SO_REUSEPORT failed: %d", errno);
        }
#endif // defined(SO_REUSEPORT)

        // If creating an IPv6 socket, tell the kernel that it will be
        // IPv6 only.  This makes it posible to bind two sockets to
        // the same port, one for IPv4 and one for IPv6.

#ifdef IPV6_V6ONLY
        if (addressType == IPAddressType::kIPv6)
        {
            res = setsockopt(mSocket, IPPROTO_IPV6, IPV6_V6ONLY, &one, sizeof(one));
            if (res != 0)
            {
                ChipLogError(Inet, "IPV6_V6ONLY failed: %d", errno);
            }
        }
#endif // defined(IPV6_V6ONLY)

#if INET_CONFIG_ENABLE_IPV4
#ifdef IP_PKTINFO
        if (addressType == IPAddressType::kIPv4)
        {
            res = setsockopt(mSocket, IPPROTO_IP, IP_PKTINFO, &one, sizeof(one));
            if (res != 0)
            {
                ChipLogError(Inet, "IP_PKTINFO failed: %d", errno);
            }
        }
#endif // defined(IP_PKTINFO)
#endif // INET_CONFIG_ENABLE_IPV4

#ifdef IPV6_RECVPKTINFO
        if (addressType == IPAddressType::kIPv6)
        {
            res = setsockopt(mSocket, IPPROTO_IPV6, IPV6_RECVPKTINFO, &one, sizeof(one));
            if (res != 0)
            {
                ChipLogError(Inet, "IPV6_PKTINFO failed: %d", errno);
            }
        }
#endif // defined(IPV6_RECVPKTINFO)

        // On systems that support it, disable the delivery of SIGPIPE
        // signals when writing to a closed socket.  This is mostly
        // needed on iOS which has the peculiar habit of sending
        // SIGPIPEs on unconnected UDP sockets.
#ifdef SO_NOSIGPIPE
        {
            res = setsockopt(mSocket, SOL_SOCKET, SO_NOSIGPIPE, &one, sizeof(one));
            if (res != 0)
            {
                ChipLogError(Inet, "SO_NOSIGPIPE failed: %d", errno);
            }
        }
#endif // defined(SO_NOSIGPIPE)
    }
    else if (mAddrType != addressType)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    return CHIP_NO_ERROR;
}

// static
void UDPEndPointImplSockets::HandlePendingIO(System::SocketEvents events, intptr_t data)
{
    reinterpret_cast<UDPEndPointImplSockets *>(data)->HandlePendingIO(events);
}

void UDPEndPointImplSockets::HandlePendingIO(System::SocketEvents events)
{
    if (mState != State::kListening || OnMessageReceived == nullptr || !events.Has(System::SocketEventFlags::kRead))
    {
        return;
    }

    CHIP_ERROR lStatus = CHIP_NO_ERROR;
    IPPacketInfo lPacketInfo;
    System::PacketBufferHandle lBuffer;

    lPacketInfo.Clear();
    lPacketInfo.DestPort  = mBoundPort;
    lPacketInfo.Interface = mBoundIntfId;

    lBuffer = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSizeWithoutReserve, 0);

    if (!lBuffer.IsNull())
    {
        struct iovec msgIOV;
        SockAddr lPeerSockAddr;
        uint8_t controlData[256];
        struct msghdr msgHeader;

        msgIOV.iov_base = lBuffer->Start();
        msgIOV.iov_len  = lBuffer->AvailableDataLength();

        memset(&lPeerSockAddr, 0, sizeof(lPeerSockAddr));

        memset(&msgHeader, 0, sizeof(msgHeader));

        msgHeader.msg_name       = &lPeerSockAddr;
        msgHeader.msg_namelen    = sizeof(lPeerSockAddr);
        msgHeader.msg_iov        = &msgIOV;
        msgHeader.msg_iovlen     = 1;
        msgHeader.msg_control    = controlData;
        msgHeader.msg_controllen = sizeof(controlData);

        ssize_t rcvLen = recvmsg(mSocket, &msgHeader, MSG_DONTWAIT);

        if (rcvLen == -1)
        {
            lStatus = CHIP_ERROR_POSIX(errno);
        }
        else if (lBuffer->AvailableDataLength() < static_cast<size_t>(rcvLen))
        {
            lStatus = CHIP_ERROR_INBOUND_MESSAGE_TOO_BIG;
        }
        else
        {
            lBuffer->SetDataLength(static_cast<uint16_t>(rcvLen));

            if (lPeerSockAddr.any.sa_family == AF_INET6)
            {
                lPacketInfo.SrcAddress = IPAddress(lPeerSockAddr.in6.sin6_addr);
                lPacketInfo.SrcPort    = ntohs(lPeerSockAddr.in6.sin6_port);
            }
#if INET_CONFIG_ENABLE_IPV4
            else if (lPeerSockAddr.any.sa_family == AF_INET)
            {
                lPacketInfo.SrcAddress = IPAddress(lPeerSockAddr.in.sin_addr);
                lPacketInfo.SrcPort    = ntohs(lPeerSockAddr.in.sin_port);
            }
#endif // INET_CONFIG_ENABLE_IPV4
            else
            {
                lStatus = CHIP_ERROR_INCORRECT_STATE;
            }
        }

        if (lStatus == CHIP_NO_ERROR)
        {
            for (struct cmsghdr * controlHdr = CMSG_FIRSTHDR(&msgHeader); controlHdr != nullptr;
                 controlHdr                  = CMSG_NXTHDR(&msgHeader, controlHdr))
            {
#if INET_CONFIG_ENABLE_IPV4
#ifdef IP_PKTINFO
                if (controlHdr->cmsg_level == IPPROTO_IP && controlHdr->cmsg_type == IP_PKTINFO)
                {
                    auto * inPktInfo = reinterpret_cast<struct in_pktinfo *> CMSG_DATA(controlHdr);
                    if (!CanCastTo<InterfaceId::PlatformType>(inPktInfo->ipi_ifindex))
                    {
                        lStatus = CHIP_ERROR_INCORRECT_STATE;
                        break;
                    }
                    lPacketInfo.Interface   = InterfaceId(static_cast<InterfaceId::PlatformType>(inPktInfo->ipi_ifindex));
                    lPacketInfo.DestAddress = IPAddress(inPktInfo->ipi_addr);
                    continue;
                }
#endif // defined(IP_PKTINFO)
#endif // INET_CONFIG_ENABLE_IPV4

#ifdef IPV6_PKTINFO
                if (controlHdr->cmsg_level == IPPROTO_IPV6 && controlHdr->cmsg_type == IPV6_PKTINFO)
                {
                    auto * in6PktInfo = reinterpret_cast<struct in6_pktinfo *> CMSG_DATA(controlHdr);
                    if (!CanCastTo<InterfaceId::PlatformType>(in6PktInfo->ipi6_ifindex))
                    {
                        lStatus = CHIP_ERROR_INCORRECT_STATE;
                        break;
                    }
                    lPacketInfo.Interface   = InterfaceId(static_cast<InterfaceId::PlatformType>(in6PktInfo->ipi6_ifindex));
                    lPacketInfo.DestAddress = IPAddress(in6PktInfo->ipi6_addr);
                    continue;
                }
#endif // defined(IPV6_PKTINFO)
            }
        }
    }
    else
    {
        lStatus = CHIP_ERROR_NO_MEMORY;
    }

    if (lStatus == CHIP_NO_ERROR)
    {
        lBuffer.RightSize();
        OnMessageReceived(this, std::move(lBuffer), &lPacketInfo);
    }
    else
    {
        if (OnReceiveError != nullptr && lStatus != CHIP_ERROR_POSIX(EAGAIN))
        {
            OnReceiveError(this, lStatus, nullptr);
        }
    }
}

#ifdef IPV6_MULTICAST_LOOP
static CHIP_ERROR SocketsSetMulticastLoopback(int aSocket, bool aLoopback, int aProtocol, int aOption)
{
    const unsigned int lValue = static_cast<unsigned int>(aLoopback);
    if (setsockopt(aSocket, aProtocol, aOption, &lValue, sizeof(lValue)) != 0)
    {
        return CHIP_ERROR_POSIX(errno);
    }

    return CHIP_NO_ERROR;
}
#endif // IPV6_MULTICAST_LOOP

static CHIP_ERROR SocketsSetMulticastLoopback(int aSocket, IPVersion aIPVersion, bool aLoopback)
{
#ifdef IPV6_MULTICAST_LOOP
    CHIP_ERROR lRetval;

    switch (aIPVersion)
    {

    case kIPVersion_6:
        lRetval = SocketsSetMulticastLoopback(aSocket, aLoopback, IPPROTO_IPV6, IPV6_MULTICAST_LOOP);
        break;

#if INET_CONFIG_ENABLE_IPV4 && defined(IP_MULTICAST_LOOP)
    case kIPVersion_4:
        lRetval = SocketsSetMulticastLoopback(aSocket, aLoopback, IPPROTO_IP, IP_MULTICAST_LOOP);
        break;
#endif // INET_CONFIG_ENABLE_IPV4 && defined(IP_MULTICAST_LOOP)

    default:
        lRetval = INET_ERROR_WRONG_ADDRESS_TYPE;
        break;
    }

    return (lRetval);
#else  // IPV6_MULTICAST_LOOP
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif // IPV6_MULTICAST_LOOP
}

CHIP_ERROR UDPEndPointImplSockets::SetMulticastLoopback(IPVersion aIPVersion, bool aLoopback)
{
    CHIP_ERROR lRetval = CHIP_ERROR_NOT_IMPLEMENTED;

    lRetval = SocketsSetMulticastLoopback(mSocket, aIPVersion, aLoopback);
    SuccessOrExit(lRetval);

exit:
    return (lRetval);
}

#if INET_CONFIG_ENABLE_IPV4

CHIP_ERROR UDPEndPointImplSockets::IPv4JoinLeaveMulticastGroupImpl(InterfaceId aInterfaceId, const IPAddress & aAddress, bool join)
{
    in_addr interfaceAddr;

    if (aInterfaceId.IsPresent())
    {
        IPAddress lInterfaceAddress;
        bool lInterfaceAddressFound = false;

        for (InterfaceAddressIterator lAddressIterator; lAddressIterator.HasCurrent(); lAddressIterator.Next())
        {
            IPAddress lCurrentAddress;
            if ((lAddressIterator.GetInterfaceId() == aInterfaceId) &&
                (lAddressIterator.GetAddress(lCurrentAddress) == CHIP_NO_ERROR))
            {
                if (lCurrentAddress.IsIPv4())
                {
                    lInterfaceAddressFound = true;
                    lInterfaceAddress      = lCurrentAddress;
                    break;
                }
            }
        }
        VerifyOrReturnError(lInterfaceAddressFound, INET_ERROR_ADDRESS_NOT_FOUND);

        interfaceAddr = lInterfaceAddress.ToIPv4();
    }
    else
    {
        interfaceAddr.s_addr = htonl(INADDR_ANY);
    }

#if INET_CONFIG_UDP_SOCKET_MREQN
    struct ip_mreqn lMulticastRequest;
    memset(&lMulticastRequest, 0, sizeof(lMulticastRequest));
    lMulticastRequest.imr_ifindex   = aInterfaceId.GetPlatformInterface(); /* Network interface index */
    lMulticastRequest.imr_address   = interfaceAddr;                       /* IP address of local interface */
    lMulticastRequest.imr_multiaddr = aAddress.ToIPv4();                   /* IP multicast group address*/

#else

    struct ip_mreq lMulticastRequest;
    memset(&lMulticastRequest, 0, sizeof(lMulticastRequest));
    lMulticastRequest.imr_interface = interfaceAddr;
    lMulticastRequest.imr_multiaddr = aAddress.ToIPv4();

#endif

    const int command = join ? IP_ADD_MEMBERSHIP : IP_DROP_MEMBERSHIP;
    if (setsockopt(mSocket, IPPROTO_IP, command, &lMulticastRequest, sizeof(lMulticastRequest)) != 0)
    {
        return CHIP_ERROR_POSIX(errno);
    }
    return CHIP_NO_ERROR;
}

#endif // INET_CONFIG_ENABLE_IPV4

CHIP_ERROR UDPEndPointImplSockets::IPv6JoinLeaveMulticastGroupImpl(InterfaceId aInterfaceId, const IPAddress & aAddress, bool join)
{
#if CHIP_SYSTEM_CONFIG_USE_PLATFORM_MULTICAST_API
    if (sMulticastGroupHandler != nullptr)
    {
        return sMulticastGroupHandler(aInterfaceId, aAddress, join ? MulticastOperation::kJoin : MulticastOperation::kLeave);
    }
#endif // CHIP_SYSTEM_CONFIG_USE_PLATFORM_MULTICAST_API

#ifdef IPV6_MULTICAST_IMPLEMENTED
    if (!aInterfaceId.IsPresent())
    {
        // Do it on all the viable interfaces.
        bool interfaceFound = false;

        InterfaceIterator interfaceIt;
        while (interfaceIt.Next())
        {
            if (!interfaceIt.SupportsMulticast() || !interfaceIt.IsUp())
            {
                continue;
            }

            InterfaceId interfaceId = interfaceIt.GetInterfaceId();

            IPAddress ifAddr;
            if (interfaceId.GetLinkLocalAddr(&ifAddr) != CHIP_NO_ERROR)
            {
                continue;
            }

            if (ifAddr.Type() != IPAddressType::kIPv6)
            {
                // Not the right sort of interface.
                continue;
            }

            interfaceFound = true;

            char ifName[InterfaceId::kMaxIfNameLength];
            interfaceIt.GetInterfaceName(ifName, sizeof(ifName));

            // Ignore errors here, except for logging, because we expect some of
            // these interfaces to not work, and some (e.g. loopback) to always
            // work.
            CHIP_ERROR err = IPv6JoinLeaveMulticastGroupImpl(interfaceId, aAddress, join);
            if (err == CHIP_NO_ERROR)
            {
                ChipLogDetail(Inet, "  %s multicast group on interface %s", (join ? "Joined" : "Left"), ifName);
            }
            else
            {
                ChipLogError(Inet, "  Failed to %s multicast group on interface %s", (join ? "join" : "leave"), ifName);
            }
        }

        if (interfaceFound)
        {
            // Assume we're good.
            return CHIP_NO_ERROR;
        }

        // Else go ahead and try to work with the default interface.
        ChipLogError(Inet, "No valid IPv6 multicast interface found");
    }

    const InterfaceId::PlatformType lIfIndex = aInterfaceId.GetPlatformInterface();

    struct ipv6_mreq lMulticastRequest;
    memset(&lMulticastRequest, 0, sizeof(lMulticastRequest));
    VerifyOrReturnError(CanCastTo<decltype(lMulticastRequest.ipv6mr_interface)>(lIfIndex), CHIP_ERROR_UNEXPECTED_EVENT);

    lMulticastRequest.ipv6mr_interface = static_cast<decltype(lMulticastRequest.ipv6mr_interface)>(lIfIndex);
    lMulticastRequest.ipv6mr_multiaddr = aAddress.ToIPv6();

    const int command = join ? INET_IPV6_ADD_MEMBERSHIP : INET_IPV6_DROP_MEMBERSHIP;
    if (setsockopt(mSocket, IPPROTO_IPV6, command, &lMulticastRequest, sizeof(lMulticastRequest)) != 0)
    {
        return CHIP_ERROR_POSIX(errno);
    }
    return CHIP_NO_ERROR;
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif
}

} // namespace Inet
} // namespace chip
