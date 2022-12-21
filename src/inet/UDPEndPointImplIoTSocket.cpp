/*
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <inet/UDPEndPointImplIoTSocket.h>
#include <iot_socket.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#include "lwip/if_api.h"
#include "lwip/ip4.h"
#include "lwip/ip6.h"
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

namespace chip {
namespace System {
CHIP_ERROR MapErrorIoTSocket(int32_t errorCode);
}

namespace Inet {

using System::MapErrorIoTSocket;

namespace {

CHIP_ERROR IPv6Bind(int socket, const IPAddress & address, uint16_t port, InterfaceId interface)
{
    iot_in6_addr iot_address;
    iot_address       = address.ToIPv6();
    CHIP_ERROR status = CHIP_NO_ERROR;
    int32_t retcode   = iotSocketBind(socket, (uint8_t *) &iot_address, sizeof(iot_address), port);
    if (retcode)
    {
        status = MapErrorIoTSocket(retcode);
    }
    else
    {
        if (interface.GetPlatformInterface())
        {
            // Instruct the kernel that any messages to multicast destinations should be
            // sent down the interface specified by the caller.
            int32_t index;
#if CHIP_SYSTEM_CONFIG_USE_LWIP
            index = interface.GetPlatformInterface()->num + 1;
#else
#error "must provide mapping to interface index"
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP
            retcode = iotSocketSetOpt(socket, IOT_SOCKET_IPV6_MULTICAST_IF, &index, sizeof(int32_t));
            if (retcode)
            {
                ChipLogError(Inet, "Call to iotSocketSetOpt IOT_SOCKET_IPV6_MULTICAST_IF failed: %ld", retcode);
            }
        }
    }

    // Instruct the kernel that any messages to multicast destinations should be
    // set with the configured hop limit value.
    int32_t hops = INET_CONFIG_IP_MULTICAST_HOP_LIMIT;
    iotSocketSetOpt(socket, IOT_SOCKET_IPV6_MULTICAST_HOPS, &hops, sizeof(hops));

    return status;
}

#if INET_CONFIG_ENABLE_IPV4
CHIP_ERROR IPv4Bind(int socket, const IPAddress & address, uint16_t port)
{
    struct iot_in_addr iot_address;
    iot_address       = address.ToIPv4();
    CHIP_ERROR status = CHIP_NO_ERROR;
    int32_t retcode   = iotSocketBind(socket, (uint8_t *) &iot_address, sizeof(iot_address), port);
    if (retcode)
    {
        status = MapErrorIoTSocket(retcode);
    }
    else
    {
        // Allow socket transmitting broadcast packets.
        constexpr int32_t enable = 1;
        iotSocketSetOpt(socket, IOT_SOCKET_SO_BROADCAST, &enable, sizeof(enable));

        // Instruct the kernel that any messages to multicast destinations should be
        // sent down the interface to which the specified IPv4 address is bound.
        iotSocketSetOpt(socket, IOT_SOCKET_IP_MULTICAST_IF, &iot_address, sizeof(iot_address));
    }

    // Instruct the kernel that any messages to multicast destinations should be
    // set with the configured hop limit value.
    constexpr int32_t ttl = INET_CONFIG_IP_MULTICAST_HOP_LIMIT;
    iotSocketSetOpt(socket, IOT_SOCKET_IP_MULTICAST_TTL, &ttl, sizeof(ttl));

    return status;
}
#endif // INET_CONFIG_ENABLE_IPV4

} // anonymous namespace

#if CHIP_SYSTEM_CONFIG_USE_PLATFORM_MULTICAST_API
UDPEndPointImplIoTSocket::MulticastGroupHandler UDPEndPointImplIoTSocket::sJoinMulticastGroupHandler;
UDPEndPointImplIoTSocket::MulticastGroupHandler UDPEndPointImplIoTSocket::sLeaveMulticastGroupHandler;
#endif // CHIP_SYSTEM_CONFIG_USE_PLATFORM_MULTICAST_API

CHIP_ERROR UDPEndPointImplIoTSocket::BindImpl(IPAddressType addressType, const IPAddress & addr, uint16_t port,
                                              InterfaceId interface)
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

    mBoundPort      = port;
    mBoundInterface = interface;

    // If an ephemeral port was requested, retrieve the actual bound port.
    if (port == 0)
    {
        if (addressType == IPAddressType::kIPv6)
        {
            struct iot_in6_addr iot_address;
            uint32_t iot_address_len = sizeof(iot_address);
            iotSocketGetSockName(mSocket, (uint8_t *) &iot_address, &iot_address_len, &mBoundPort);
        }
#if INET_CONFIG_ENABLE_IPV4
        else if (addressType == IPAddressType::kIPv4)
        {
            struct iot_in6_addr iot_address;
            uint32_t iot_address_len = sizeof(iot_address);
            iotSocketGetSockName(mSocket, (uint8_t *) &iot_address, &iot_address_len, &mBoundPort);
        }
#endif // INET_CONFIG_ENABLE_IPV4
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR UDPEndPointImplIoTSocket::BindInterfaceImpl(IPAddressType addressType, InterfaceId interfaceId)
{
    // Make sure we have the appropriate type of socket.
    ReturnErrorOnFailure(GetSocket(addressType));

#if HAVE_SO_BINDTODEVICE
    CHIP_ERROR status = CHIP_NO_ERROR;
    int32_t retcode;

    if (interfaceId.IsPresent())
    {
        // Start filtering on the passed interface.
        char interfaceName[IF_NAMESIZE];
        if (if_indextoname(interfaceId.GetPlatformInterface(), interfaceName) == nullptr)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        retcode = iotSocketSetOpt(mSocket, IOT_SOCKET_SO_BINDTODEVICE, interfaceName, uint32_t(strlen(interfaceName)));
        if (retcode)
        {
            return MapErrorIoTSocket(retcode);
        }
    }
    else
    {
        // Stop interface-based filtering.
        if (retcode = iotSocketSetOpt(mSocket, IOT_SOCKET_SO_BINDTODEVICE, "", 0) != 0)
        {
            return MapErrorIoTSocket(retcode);
        }
    }

    mBoundInterface = interfaceId;

    return CHIP_NO_ERROR;
#else  // !HAVE_SO_BINDTODEVICE
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif // HAVE_SO_BINDTODEVICE
}

InterfaceId UDPEndPointImplIoTSocket::GetBoundInterface() const
{
    return mBoundInterface;
}

uint16_t UDPEndPointImplIoTSocket::GetBoundPort() const
{
    return mBoundPort;
}

CHIP_ERROR UDPEndPointImplIoTSocket::ListenImpl()
{
    // Wait for ability to read on this endpoint.
    auto * layer = static_cast<System::LayerOpenIoTSDK *>(&GetSystemLayer());
    return layer->EnableSelectCallback(this, true, false);
}

CHIP_ERROR UDPEndPointImplIoTSocket::SendMsgImpl(const IPPacketInfo * aPktInfo, System::PacketBufferHandle && msg)
{
    // Make sure we have the appropriate type of socket based on the
    // destination address.
    ReturnErrorOnFailure(GetSocket(aPktInfo->DestAddress.Type()));

    // Ensure the destination address type is compatible with the endpoint address type.
    VerifyOrReturnError(mSocketFamily ==
                            ((aPktInfo->DestAddress.Type() == IPAddressType::kIPv6) ? IOT_SOCKET_AF_INET6 : IOT_SOCKET_AF_INET),
                        CHIP_ERROR_INVALID_ARGUMENT);

    // For now the entire message must fit within a single buffer.
    VerifyOrReturnError(!msg->HasChainedBuffer(), CHIP_ERROR_MESSAGE_TOO_LONG);

    iot_iovec msgIOV;
    msgIOV.iov_base = msg->Start();
    msgIOV.iov_len  = msg->DataLength();

    uint8_t controlData[256];
    memset(controlData, 0, sizeof(controlData));

    iot_msghdr msgHeader;
    memset(&msgHeader, 0, sizeof(msgHeader));
    msgHeader.msg_iov    = &msgIOV;
    msgHeader.msg_iovlen = 1;

    // Construct a sockaddr_in/sockaddr_in6 structure containing the destination information.
    iot_sockaddr_in_any peerSockAddr;
    memset(&peerSockAddr, 0, sizeof(peerSockAddr));
    msgHeader.msg_name = &peerSockAddr;
    if (mSocketFamily == IOT_SOCKET_AF_INET6)
    {
        peerSockAddr.in6.sin6_len    = sizeof(iot_sockaddr_in6);
        peerSockAddr.in6.sin6_family = IOT_SOCKET_AF_INET6;
        peerSockAddr.in6.sin6_port   = htons(aPktInfo->DestPort);
        peerSockAddr.in6.sin6_addr   = aPktInfo->DestAddress.ToIPv6();
        msgHeader.msg_namelen        = sizeof(iot_sockaddr_in6);
    }
#if INET_CONFIG_ENABLE_IPV4
    else
    {
        peerSockAddr.in.sin_len    = sizeof(iot_sockaddr_in);
        peerSockAddr.in.sin_family = IOT_SOCKET_AF_INET;
        peerSockAddr.in.sin_port   = htons(aPktInfo->DestPort);
        peerSockAddr.in.sin_addr   = aPktInfo->DestAddress.ToIPv4();
        msgHeader.msg_namelen      = sizeof(iot_sockaddr_in);
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
        intf = mBoundInterface;
    }

    if (mSocketFamily == IOT_SOCKET_AF_INET6)
    {
#if CHIP_SYSTEM_CONFIG_USE_LWIP
        // this translates to LWIP interface zone which is equal to the index which maps to netif->num+1
        peerSockAddr.in6.sin6_scope_id = intf.GetPlatformInterface()->num + 1;
#else
#error "must provide translation between implementation interface and interface zone"
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP
    }

    // If the packet should be sent over a specific interface, or with a specific source
    // address, construct an IOT_SOCKET_IP_PKTINFO/IOT_SOCKET_IPV6_PKTINFO "control message" to that effect
    // add add it to the message header.  If the local OS doesn't support IOT_SOCKET_IP_PKTINFO/IOT_SOCKET_IPV6_PKTINFO
    // fail with an error.

    // iotSocketSendMsg doesn't support pktinfo, we can't specify the address, fail if we try to
    if (aPktInfo->SrcAddress.Type() != IPAddressType::kAny)
    {
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }
    else if (intf.IsPresent())
    {
        // if we have a bound interface we'll try to send on that
        if (mBoundInterface.IsPresent())
        {
            if (mBoundInterface.GetPlatformInterface() != intf.GetPlatformInterface())
            {
                // if we're trying to send the packet on an interafce different than bound then fail
                return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
            }
        }
        else // we check if we will send on the same interface as requested
        {
            InterfaceId::PlatformType default_platform_iface;
#if CHIP_SYSTEM_CONFIG_USE_LWIP
            // only way to legitimately get the default iface is to use ip route
            if (mSocketFamily == IOT_SOCKET_AF_INET6)
            {
                const iot_in6_addr addr6 = aPktInfo->DestAddress.ToIPv6();
                ip6_addr_t lwip_addr6;
                memcpy(&lwip_addr6, &addr6, sizeof(iot_in6_addr));
                lwip_addr6.zone        = intf.GetPlatformInterface()->num + 1;
                default_platform_iface = ip6_route(nullptr, &lwip_addr6);
            }
#if INET_CONFIG_ENABLE_IPV4
            else
            {
                const iot_in_addr addr = aPktInfo->DestAddress.ToIPv4();
                default_platform_iface = ip4_route((const ip4_addr_t *) &addr);
            }
#endif // INET_CONFIG_ENABLE_IPV4
            if (!default_platform_iface)
            {
                return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
            }
#else
#error "must provide default interface or enable support for pkt info in iotSocketSendMsg"
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP
            if (default_platform_iface != intf.GetPlatformInterface())
            {
                // if we're trying to send a packet on a non default interface then fail
                return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
            }
        }
    }

    // Send IP packet.
    const ssize_t lenSent = iotSocketSendMsg(mSocket, &msgHeader, 0);
    if (lenSent < 0)
    {
        return MapErrorIoTSocket(lenSent);
    }
    if (lenSent != msg->DataLength())
    {
        return CHIP_ERROR_OUTBOUND_MESSAGE_TOO_BIG;
    }
    return CHIP_NO_ERROR;
}

void UDPEndPointImplIoTSocket::CloseImpl()
{
    if (mSocket != kInvalidSocketFd)
    {
        static_cast<System::LayerOpenIoTSDK *>(&GetSystemLayer())->DisableSelectCallback(this, true, true);
        iotSocketClose(mSocket);
        mSocket = kInvalidSocketFd;
    }
}

void UDPEndPointImplIoTSocket::Free()
{
    Close();
    Release();
}

CHIP_ERROR UDPEndPointImplIoTSocket::GetSocket(IPAddressType addressType)
{
    if (mSocket == kInvalidSocketFd)
    {
        int family;

        switch (addressType)
        {
        case IPAddressType::kIPv6:
            family = IOT_SOCKET_AF_INET6;
            break;

#if INET_CONFIG_ENABLE_IPV4
        case IPAddressType::kIPv4:
            family = IOT_SOCKET_AF_INET;
            break;
#endif // INET_CONFIG_ENABLE_IPV4

        default:
            return INET_ERROR_WRONG_ADDRESS_TYPE;
        }

        mSocket = iotSocketCreate(family, IOT_SOCKET_SOCK_DGRAM, IOT_SOCKET_IPPROTO_UDP);
        if (mSocket < 0)
        {
            return MapErrorIoTSocket(mSocket);
        }

        mSocketFamily = family;

        // NOTE: some return codes are logged only and not fatal

        constexpr int32_t one = 1;
        int32_t res           = iotSocketSetOpt(mSocket, IOT_SOCKET_SO_REUSEADDR, &one, sizeof(one));
        if (res != 0)
        {
            ChipLogError(Inet, "SO_REUSEADDR failed: %ld", res);
        }

        // If creating an IPv6 socket, tell the kernel that it will be
        // IPv6 only.  This makes it posible to iotSocketBind two sockets to
        // the same port, one for IPv4 and one for IPv6.

        if (addressType == IPAddressType::kIPv6)
        {
            res = iotSocketSetOpt(mSocket, IOT_SOCKET_IPV6_V6ONLY, &one, sizeof(one));
            if (res != 0)
            {
                ChipLogError(Inet, "IPV6_V6ONLY failed: %ld", res);
            }
        }

#if INET_CONFIG_ENABLE_IPV4
        if (addressType == IPAddressType::kIPv4)
        {
            res = iotSocketSetOpt(mSocket, IOT_SOCKET_IP_PKTINFO, &one, sizeof(one));
            if (res != 0)
            {
                ChipLogError(Inet, "IOT_SOCKET_IP_PKTINFO failed: %ld", res);
            }
        }
#endif // INET_CONFIG_ENABLE_IPV4

        if (addressType == IPAddressType::kIPv6)
        {
            res = iotSocketSetOpt(mSocket, IOT_SOCKET_IPV6_PKTINFO, &one, sizeof(one));
            if (res != 0)
            {
                ChipLogError(Inet, "IOT_SOCKET_IPV6_PKTINFO failed: %ld", res);
            }
        }
    }
    else
    {
        if (addressType == IPAddressType::kIPv6 && mSocketFamily != IOT_SOCKET_AF_INET6)
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }
        if (addressType == IPAddressType::kIPv4 && mSocketFamily != IOT_SOCKET_AF_INET)
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }
    }

    return CHIP_NO_ERROR;
}

void UDPEndPointImplIoTSocket::SelectCallback(void * readMask, void * writeMask, void * exceptionMask)
{
    bool readEvent = iotSocketMaskIsSet(mSocket, readMask);

    if (mState != State::kListening || OnMessageReceived == nullptr || !readEvent)
    {
        return;
    }

    CHIP_ERROR lStatus = CHIP_NO_ERROR;
    IPPacketInfo lPacketInfo;
    System::PacketBufferHandle lBuffer;

    lPacketInfo.Clear();
    lPacketInfo.DestPort = mBoundPort;

    lBuffer = System::PacketBufferHandle::New(System::PacketBuffer::kMaxSizeWithoutReserve, 0);

    if (!lBuffer.IsNull())
    {
        iot_iovec msgIOV;
        iot_sockaddr_in_any lPeerSockAddr;
        uint8_t controlData[256];
        struct iot_msghdr msgHeader;

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

        int32_t rcvLen = iotSocketRecvMsg(mSocket, &msgHeader, IOT_SOCKET_MSG_DONTWAIT);

        if (rcvLen < 0)
        {
            lStatus = MapErrorIoTSocket(rcvLen);
        }
        else if (rcvLen > lBuffer->AvailableDataLength())
        {
            lStatus = CHIP_ERROR_INBOUND_MESSAGE_TOO_BIG;
        }
        else
        {
            lBuffer->SetDataLength(static_cast<uint16_t>(rcvLen));

            if (lPeerSockAddr.in6.sin6_family == IOT_SOCKET_AF_INET6)
            {
                lPacketInfo.SrcAddress = IPAddress(lPeerSockAddr.in6.sin6_addr);
                lPacketInfo.SrcPort    = ntohs(lPeerSockAddr.in6.sin6_port);
            }
#if INET_CONFIG_ENABLE_IPV4
            else if (lPeerSockAddr.in.sin_family == IOT_SOCKET_AF_INET)
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
            for (iot_cmsghdr * controlHdr = IOT_CMSG_FIRSTHDR(&msgHeader); controlHdr != nullptr;
                 controlHdr               = IOT_CMSG_NXTHDR(&msgHeader, controlHdr))
            {
                // the control message contains the pktinfo that has the index of the interface,
                // we need to translate that index into a pointer to the interface struct
#if INET_CONFIG_ENABLE_IPV4
                if (controlHdr->cmsg_level == IOT_SOCKET_LEVEL_IPPROTO_IP && controlHdr->cmsg_type == IOT_SOCKET_IP_PKTINFO)
                {
                    iot_in_pktinfo * inPktInfo = (iot_in_pktinfo *) IOT_CMSG_DATA(controlHdr);
                    InterfaceId iface;
#if CHIP_SYSTEM_CONFIG_USE_LWIP
                    char name[IF_NAMESIZE];
                    if (lwip_if_indextoname(inPktInfo->ipi_ifindex, name))
                    {
                        if (InterfaceId::InterfaceNameToId(name, iface) == CHIP_NO_ERROR)
                        {
                            if (!iface.IsPresent())
                            {
                                lStatus = CHIP_ERROR_INCORRECT_STATE;
                                break;
                            }
                        }
                    }
#else
#error "must provide translation between implementation interface index and chip InterfaceId"
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP
                    lPacketInfo.Interface   = iface;
                    lPacketInfo.DestAddress = IPAddress(inPktInfo->ipi_addr);
                    continue;
                }
#endif // INET_CONFIG_ENABLE_IPV4
                if (controlHdr->cmsg_level == IOT_SOCKET_LEVEL_IPPROTO_IPV6 && controlHdr->cmsg_type == IOT_SOCKET_IPV6_PKTINFO)
                {
                    iot_in6_pktinfo * in6PktInfo = (iot_in6_pktinfo *) IOT_CMSG_DATA(controlHdr);
                    InterfaceId iface;
#if CHIP_SYSTEM_CONFIG_USE_LWIP
                    char name[IF_NAMESIZE];
                    if (lwip_if_indextoname(in6PktInfo->ipi6_ifindex, name))
                    {
                        if (InterfaceId::InterfaceNameToId(name, iface) == CHIP_NO_ERROR)
                        {
                            if (!iface.IsPresent())
                            {
                                lStatus = CHIP_ERROR_INCORRECT_STATE;
                                break;
                            }
                        }
                    }
#else
#error "must provide translation between implementation interface index and chip InterfaceId"
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP
                    lPacketInfo.Interface   = iface;
                    lPacketInfo.DestAddress = IPAddress(in6PktInfo->ipi6_addr);
                    continue;
                }
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
        if (OnReceiveError != nullptr && lStatus != CHIP_ERROR_BUSY)
        {
            OnReceiveError(this, lStatus, nullptr);
        }
    }
}

static CHIP_ERROR SocketsSetMulticastLoopback(int aSocket, bool aLoopback, int aProtocol, int aOption)
{
    (void) aProtocol;
    const int32_t lValue = static_cast<unsigned int>(aLoopback);
    int32_t retcode      = iotSocketSetOpt(aSocket, aOption, &lValue, sizeof(lValue));
    if (retcode)
    {
        return MapErrorIoTSocket(retcode);
    }

    return CHIP_NO_ERROR;
}

static CHIP_ERROR SocketsSetMulticastLoopback(int aSocket, IPVersion aIPVersion, bool aLoopback)
{
    CHIP_ERROR lRetval;

    switch (aIPVersion)
    {
    case kIPVersion_6:
        lRetval = SocketsSetMulticastLoopback(aSocket, aLoopback, 0, IOT_SOCKET_IPV6_MULTICAST_LOOP);
        break;

#if INET_CONFIG_ENABLE_IPV4
    case kIPVersion_4:
        lRetval = SocketsSetMulticastLoopback(aSocket, aLoopback, 0, IOT_SOCKET_IP_MULTICAST_LOOP);
        break;
#endif // INET_CONFIG_ENABLE_IPV4

    default:
        lRetval = INET_ERROR_WRONG_ADDRESS_TYPE;
        break;
    }

    return (lRetval);
}

CHIP_ERROR UDPEndPointImplIoTSocket::SetMulticastLoopback(IPVersion aIPVersion, bool aLoopback)
{
    CHIP_ERROR lRetval = CHIP_ERROR_NOT_IMPLEMENTED;

    lRetval = SocketsSetMulticastLoopback(mSocket, aIPVersion, aLoopback);
    SuccessOrExit(lRetval);

exit:
    return (lRetval);
}

#if INET_CONFIG_ENABLE_IPV4

CHIP_ERROR UDPEndPointImplIoTSocket::IPv4JoinLeaveMulticastGroupImpl(InterfaceId aInterfaceId, const IPAddress & aAddress,
                                                                     bool join)
{
    IPAddress lInterfaceAddress;
    bool lInterfaceAddressFound = false;

    for (InterfaceAddressIterator lAddressIterator; lAddressIterator.HasCurrent(); lAddressIterator.Next())
    {
        IPAddress lCurrentAddress;
        if ((lAddressIterator.GetInterfaceId() == aInterfaceId) && (lAddressIterator.GetAddress(lCurrentAddress) == CHIP_NO_ERROR))
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

    iot_ip_mreq lMulticastRequest;
    lMulticastRequest.imr_interface = lInterfaceAddress.ToIPv4();
    lMulticastRequest.imr_multiaddr = aAddress.ToIPv4();

    const int command = join ? IOT_SOCKET_IP_ADD_MEMBERSHIP : IOT_SOCKET_IP_DROP_MEMBERSHIP;
    int32_t retcode   = iotSocketSetOpt(mSocket, command, &lMulticastRequest, sizeof(lMulticastRequest));
    if (retcode)
    {
        return MapErrorIoTSocket(retcode);
    }
    return CHIP_NO_ERROR;
}

#endif // INET_CONFIG_ENABLE_IPV4

CHIP_ERROR UDPEndPointImplIoTSocket::IPv6JoinLeaveMulticastGroupImpl(InterfaceId aInterfaceId, const IPAddress & aAddress,
                                                                     bool join)
{
#if CHIP_SYSTEM_CONFIG_USE_PLATFORM_MULTICAST_API
    MulticastGroupHandler handler = join ? sJoinMulticastGroupHandler : sLeaveMulticastGroupHandler;
    if (handler != nullptr)
    {
        return handler(aInterfaceId, aAddress);
    }
#endif // CHIP_SYSTEM_CONFIG_USE_PLATFORM_MULTICAST_API

    const InterfaceId::PlatformType lIfIndex = aInterfaceId.GetPlatformInterface();

    iot_ipv6_mreq lMulticastRequest;
#if CHIP_SYSTEM_CONFIG_USE_LWIP
    // this translates to LWIP interface zone which is equal to the index which maps to netif->num+1
    lMulticastRequest.ipv6mr_interface = (uint32_t) lIfIndex->num + 1;
#else
#error "must provide translation between implementation interface and interface zone"
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

    lMulticastRequest.ipv6mr_multiaddr = aAddress.ToIPv6();

    const int command = join ? IOT_SOCKET_IPV6_ADD_MEMBERSHIP : IOT_SOCKET_IPV6_DROP_MEMBERSHIP;
    int32_t retcode   = iotSocketSetOpt(mSocket, command, &lMulticastRequest, sizeof(lMulticastRequest));
    if (retcode)
    {
        return MapErrorIoTSocket(retcode);
    }
    return CHIP_NO_ERROR;
}

} // namespace Inet
} // namespace chip
