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

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <system/SystemFaultInjection.h>

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#include <lwip/ip.h>
#include <lwip/tcpip.h>
#include <lwip/udp.h>
#if CHIP_HAVE_CONFIG_H
#include <lwip/lwip_buildconfig.h> // nogncheck
#endif                             // CHIP_HAVE_CONFIG_H
#endif                             // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif // HAVE_SYS_SOCKET_H
#include <errno.h>
#include <net/if.h>
#include <netinet/in.h>
#include <unistd.h>

// SOCK_CLOEXEC not defined on all platforms, e.g. iOS/macOS:
#ifdef SOCK_CLOEXEC
#define SOCK_FLAGS SOCK_CLOEXEC
#else
#define SOCK_FLAGS 0
#endif

#if CHIP_SYSTEM_CONFIG_USE_ZEPHYR_SOCKET_EXTENSIONS
#include "ZephyrSocket.h"
#endif // CHIP_SYSTEM_CONFIG_USE_ZEPHYR_SOCKET_EXTENSIONS
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

#include "arpa-inet-compatibility.h"

#include <string.h>
#include <utility>

namespace chip {
namespace Inet {

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
static CHIP_ERROR LwIPBindInterface(struct udp_pcb * aUDP, InterfaceId intfId)
{
    CHIP_ERROR res = CHIP_NO_ERROR;

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

CHIP_ERROR UDPEndPoint::BindImpl(IPAddressType addrType, const IPAddress & addr, uint16_t port, InterfaceId intfId)
{
    // Lock LwIP stack
    LOCK_TCPIP_CORE();

    // Make sure we have the appropriate type of PCB.
    CHIP_ERROR res = GetPCB(addrType);

    // Bind the PCB to the specified address/port.
    if (res == CHIP_NO_ERROR)
    {
#if LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
        ip_addr_t ipAddr = addr.ToLwIPAddr();

        // TODO: IPAddress ANY has only one constant state, however addrType
        // has separate IPV4 and IPV6 'any' settings. This tries to correct
        // for this as LWIP default if IPv4 is compiled in is to consider
        // 'any == any_v4'
        //
        // We may want to consider having separate AnyV4 and AnyV6 constants
        // inside CHIP to resolve this ambiguity
        if ((addr.Type() == kIPAddressType_Any) && (addrType == kIPAddressType_IPv6))
        {
            ipAddr = *IP6_ADDR_ANY;
        }

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

    if (res == CHIP_NO_ERROR)
    {
        res = LwIPBindInterface(mUDP, intfId);
    }

    // Unlock LwIP stack
    UNLOCK_TCPIP_CORE();

    return res;
}

CHIP_ERROR UDPEndPoint::BindInterfaceImpl(IPAddressType addrType, InterfaceId intfId)
{
    // A lock is required because the LwIP thread may be referring to intf_filter,
    // while this code running in the Inet application is potentially modifying it.
    // NOTE: this only supports LwIP interfaces whose number is no bigger than 9.
    LOCK_TCPIP_CORE();

    // Make sure we have the appropriate type of PCB.
    CHIP_ERROR err = GetPCB(addrType);

    if (err == CHIP_NO_ERROR)
    {
        err = LwIPBindInterface(mUDP, intfId);
    }

    UNLOCK_TCPIP_CORE();

    return err;
}

InterfaceId UDPEndPoint::GetBoundInterface()
{
#if HAVE_LWIP_UDP_BIND_NETIF
    return netif_get_by_index(mUDP->netif_idx);
#else
    return mUDP->intf_filter;
#endif
}

uint16_t UDPEndPoint::GetBoundPort()
{
    return mUDP->local_port;
}

CHIP_ERROR UDPEndPoint::ListenImpl()
{
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

    return CHIP_NO_ERROR;
}

CHIP_ERROR UDPEndPoint::SendMsgImpl(const IPPacketInfo * pktInfo, System::PacketBufferHandle && msg)
{
    const IPAddress & destAddr = pktInfo->DestAddress;

    if (!msg.HasSoleOwnership())
    {
        // when retaining a buffer, the caller expects the msg to be unmodified.
        // LwIP stack will normally prepend the packet headers as the packet traverses
        // the UDP/IP/netif layers, which normally modifies the packet. We need to clone
        // msg into a fresh object in this case, and queues that for transmission, leaving
        // the original msg available after return.
        msg = msg.CloneData();
        VerifyOrReturnError(!msg.IsNull(), CHIP_ERROR_NO_MEMORY);
    }

    // Lock LwIP stack
    LOCK_TCPIP_CORE();

    // Make sure we have the appropriate type of PCB based on the destination address.
    CHIP_ERROR res = GetPCB(destAddr.Type());
    if (res != CHIP_NO_ERROR)
    {
        UNLOCK_TCPIP_CORE();
        return res;
    }

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
            lwipErr = udp_sendto_if(mUDP, System::LwIPPacketBufferView::UnsafeGetLwIPpbuf(msg), &lwipDestAddr, destPort, intfId);
        else
            lwipErr = udp_sendto(mUDP, System::LwIPPacketBufferView::UnsafeGetLwIPpbuf(msg), &lwipDestAddr, destPort);

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
                lwipErr =
                    udp_sendto_if_ip6(mUDP, System::LwIPPacketBufferView::UnsafeGetLwIPpbuf(msg), &lwipDestAddr, destPort, intfId);
            else
                lwipErr = udp_sendto_ip6(mUDP, System::LwIPPacketBufferView::UnsafeGetLwIPpbuf(msg), &lwipDestAddr, destPort);
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
                lwipErr =
                    udp_sendto_if(mUDP, System::LwIPPacketBufferView::UnsafeGetLwIPpbuf(msg), &lwipDestAddr, destPort, intfId);
            else
                lwipErr = udp_sendto(mUDP, System::LwIPPacketBufferView::UnsafeGetLwIPpbuf(msg), &lwipDestAddr, destPort);
        }

        ipX_addr_copy(mUDP->local_ip, boundAddr);

#endif // INET_CONFIG_ENABLE_IPV4
#endif // LWIP_VERSION_MAJOR <= 1 || LWIP_VERSION_MINOR >= 5

        if (lwipErr != ERR_OK)
            res = chip::System::MapErrorLwIP(lwipErr);
    }

    // Unlock LwIP stack
    UNLOCK_TCPIP_CORE();

    return res;
}

void UDPEndPoint::CloseImpl()
{
    // Lock LwIP stack
    LOCK_TCPIP_CORE();

    // Since UDP PCB is released synchronously here, but UDP endpoint itself might have to wait
    // for destruction asynchronously, there could be more allocated UDP endpoints than UDP PCBs.
    if (mUDP != NULL)
    {
        udp_remove(mUDP);
        mUDP              = NULL;
        mLwIPEndPointType = LwIPEndPointType::Unknown;
    }

    // Unlock LwIP stack
    UNLOCK_TCPIP_CORE();
}

void UDPEndPoint::Free()
{
    Close();
    DeferredFree(kReleaseDeferralErrorTactic_Die);
}

void UDPEndPoint::HandleDataReceived(System::PacketBufferHandle && msg)
{
    IPEndPointBasis::HandleDataReceived(std::move(msg));
}

CHIP_ERROR UDPEndPoint::GetPCB(IPAddressType addrType)
{
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
            return INET_ERROR_WRONG_ADDRESS_TYPE;
        }

        // Fail if the system has run out of PCBs.
        if (mUDP == NULL)
        {
            ChipLogError(Inet, "Unable to allocate UDP PCB");
            return CHIP_ERROR_NO_MEMORY;
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
            return INET_ERROR_WRONG_ADDRESS_TYPE;
        }
#else // LWIP_VERSION_MAJOR <= 1 && LWIP_VERSION_MINOR < 5
#if INET_CONFIG_ENABLE_IPV4
        pcbAddrType = PCB_ISIPV6(mUDP) ? kIPAddressType_IPv6 : kIPAddressType_IPv4;
#else  // !INET_CONFIG_ENABLE_IPV4
        pcbAddrType = kIPAddressType_IPv6;
#endif // !INET_CONFIG_ENABLE_IPV4
#endif // LWIP_VERSION_MAJOR <= 1 && LWIP_VERSION_MINOR < 5

        // Fail if the existing PCB is not the correct type.
        VerifyOrReturnError(addrType == pcbAddrType, INET_ERROR_WRONG_ADDRESS_TYPE);
    }

    return CHIP_NO_ERROR;
}

#if LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
void UDPEndPoint::LwIPReceiveUDPMessage(void * arg, struct udp_pcb * pcb, struct pbuf * p, const ip_addr_t * addr, u16_t port)
#else  // LWIP_VERSION_MAJOR <= 1 && LWIP_VERSION_MINOR < 5
void UDPEndPoint::LwIPReceiveUDPMessage(void * arg, struct udp_pcb * pcb, struct pbuf * p, ip_addr_t * addr, u16_t port)
#endif // LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
{
    UDPEndPoint * ep                 = static_cast<UDPEndPoint *>(arg);
    System::LayerLwIP * lSystemLayer = static_cast<System::LayerLwIP *>(ep->Layer().SystemLayer());
    IPPacketInfo * pktInfo           = NULL;
    System::PacketBufferHandle buf   = System::PacketBufferHandle::Adopt(p);
    if (buf->HasChainedBuffer())
    {
        // Try the simple expedient of flattening in-place.
        buf->CompactHead();
    }

    if (buf->HasChainedBuffer())
    {
        // Have to allocate a new big-enough buffer and copy.
        uint16_t messageSize            = buf->TotalLength();
        System::PacketBufferHandle copy = System::PacketBufferHandle::New(messageSize, 0);
        if (copy.IsNull() || buf->Read(copy->Start(), messageSize) != CHIP_NO_ERROR)
        {
            ChipLogError(Inet, "No memory to flatten incoming packet buffer chain of size %" PRIu16, buf->TotalLength());
            return;
        }
        buf = std::move(copy);
    }

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

    PostPacketBufferEvent(lSystemLayer, *ep, kInetEvent_UDPDataReceived, std::move(buf));
}

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS

CHIP_ERROR UDPEndPoint::BindImpl(IPAddressType addrType, const IPAddress & addr, uint16_t port, InterfaceId intfId)
{
    // Make sure we have the appropriate type of socket.
    ReturnErrorOnFailure(GetSocket(addrType));
    ReturnErrorOnFailure(IPEndPointBasis::Bind(addrType, addr, port, intfId));

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

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
    dispatch_queue_t dispatchQueue = static_cast<System::LayerSocketsLoop *>(Layer().SystemLayer())->GetDispatchQueue();
    if (dispatchQueue != nullptr)
    {
        unsigned long fd = static_cast<unsigned long>(mSocket);

        mReadableSource = dispatch_source_create(DISPATCH_SOURCE_TYPE_READ, fd, 0, dispatchQueue);
        ReturnErrorCodeIf(mReadableSource == nullptr, CHIP_ERROR_NO_MEMORY);

        dispatch_source_set_event_handler(mReadableSource, ^{
            this->HandlePendingIO(System::SocketEventFlags::kRead);
        });
        dispatch_resume(mReadableSource);
    }
#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH

    return CHIP_NO_ERROR;
}

CHIP_ERROR UDPEndPoint::BindInterfaceImpl(IPAddressType addrType, InterfaceId intfId)
{
    // Make sure we have the appropriate type of socket.
    ReturnErrorOnFailure(GetSocket(addrType));
    return IPEndPointBasis::BindInterface(addrType, intfId);
}

InterfaceId UDPEndPoint::GetBoundInterface()
{
    return mBoundIntfId;
}

uint16_t UDPEndPoint::GetBoundPort()
{
    return mBoundPort;
}

CHIP_ERROR UDPEndPoint::ListenImpl()
{
    // Wait for ability to read on this endpoint.
    auto layer = static_cast<System::LayerSockets *>(Layer().SystemLayer());
    ReturnErrorOnFailure(layer->SetCallback(mWatch, HandlePendingIO, reinterpret_cast<intptr_t>(this)));
    return layer->RequestCallbackOnPendingRead(mWatch);
}

CHIP_ERROR UDPEndPoint::SendMsgImpl(const IPPacketInfo * pktInfo, System::PacketBufferHandle && msg)
{
    // Make sure we have the appropriate type of socket based on the
    // destination address.
    ReturnErrorOnFailure(GetSocket(pktInfo->DestAddress.Type()));

    return IPEndPointBasis::SendMsg(pktInfo, std::move(msg));
}

void UDPEndPoint::CloseImpl()
{
    if (mSocket != kInvalidSocketFd)
    {
        static_cast<System::LayerSockets *>(Layer().SystemLayer())->StopWatchingSocket(&mWatch);
        close(mSocket);
        mSocket = kInvalidSocketFd;
    }

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
    if (mReadableSource)
    {
        dispatch_source_cancel(mReadableSource);
        dispatch_release(mReadableSource);
    }
#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH
}

void UDPEndPoint::Free()
{
    Close();
    Release();
}

CHIP_ERROR UDPEndPoint::GetSocket(IPAddressType aAddressType)
{
    constexpr int lType     = (SOCK_DGRAM | SOCK_FLAGS);
    constexpr int lProtocol = 0;

    return IPEndPointBasis::GetSocket(aAddressType, lType, lProtocol);
}

// static
void UDPEndPoint::HandlePendingIO(System::SocketEvents events, intptr_t data)
{
    reinterpret_cast<UDPEndPoint *>(data)->HandlePendingIO(events);
}

void UDPEndPoint::HandlePendingIO(System::SocketEvents events)
{
    if (mState == kState_Listening && OnMessageReceived != nullptr && events.Has(System::SocketEventFlags::kRead))
    {
        const uint16_t lPort = mBoundPort;

        IPEndPointBasis::HandlePendingIO(lPort);
    }
}

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

#if CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK

CHIP_ERROR UDPEndPoint::BindImpl(IPAddressType addrType, const IPAddress & addr, uint16_t port, InterfaceId intfId)
{
    nw_parameters_configure_protocol_block_t configure_tls;
    nw_parameters_t parameters;

    if (intfId != INET_NULL_INTERFACEID)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    configure_tls = NW_PARAMETERS_DISABLE_PROTOCOL;
    parameters    = nw_parameters_create_secure_udp(configure_tls, NW_PARAMETERS_DEFAULT_CONFIGURATION);

    ReturnErrorOnFailure(IPEndPointBasis::Bind(addrType, addr, port, parameters));

    mParameters = parameters;
    return CHIP_NO_ERROR;
}

CHIP_ERROR UDPEndPoint::BindInterfaceImpl(IPAddressType addrType, InterfaceId intfId)
{
    return INET_ERROR_UNKNOWN_INTERFACE;
}

InterfaceId UDPEndPoint::GetBoundInterface()
{
    return INET_NULL_INTERFACEID;
}

uint16_t UDPEndPoint::GetBoundPort()
{
    nw_endpoint_t endpoint = nw_parameters_copy_local_endpoint(mParameters);
    return nw_endpoint_get_port(endpoint);
}

CHIP_ERROR UDPEndPoint::ListenImpl()
{
    return StartListener();
}

CHIP_ERROR UDPEndPoint::SendMsgImpl(const IPPacketInfo * pktInfo, System::PacketBufferHandle && msg)
{
    return IPEndPointBasis::SendMsg(pktInfo, std::move(msg));
}

void UDPEndPoint::CloseImpl()
{
    IPEndPointBasis::ReleaseAll();
}

void UDPEndPoint::Free()
{
    Close();
    Release();
}

#endif // CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK

CHIP_ERROR UDPEndPoint::Bind(IPAddressType addrType, const IPAddress & addr, uint16_t port, InterfaceId intfId)
{
    if (mState != kState_Ready && mState != kState_Bound)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    if ((addr != IPAddress::Any) && (addr.Type() != kIPAddressType_Any) && (addr.Type() != addrType))
    {
        return INET_ERROR_WRONG_ADDRESS_TYPE;
    }

    ReturnErrorOnFailure(BindImpl(addrType, addr, port, intfId));

    mState = kState_Bound;

    return CHIP_NO_ERROR;
}

CHIP_ERROR UDPEndPoint::BindInterface(IPAddressType addrType, InterfaceId intfId)
{
    if (mState != kState_Ready && mState != kState_Bound)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    ReturnErrorOnFailure(BindInterfaceImpl(addrType, intfId));

    mState = kState_Bound;

    return CHIP_NO_ERROR;
}

CHIP_ERROR UDPEndPoint::Listen(OnMessageReceivedFunct onMessageReceived, OnReceiveErrorFunct onReceiveError, void * appState)
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

    ReturnErrorOnFailure(ListenImpl());

    mState = kState_Listening;

    return CHIP_NO_ERROR;
}

CHIP_ERROR UDPEndPoint::SendTo(const IPAddress & addr, uint16_t port, chip::System::PacketBufferHandle && msg, InterfaceId intfId)
{
    IPPacketInfo pktInfo;
    pktInfo.Clear();
    pktInfo.DestAddress = addr;
    pktInfo.DestPort    = port;
    pktInfo.Interface   = intfId;
    return SendMsg(&pktInfo, std::move(msg));
}

CHIP_ERROR UDPEndPoint::SendMsg(const IPPacketInfo * pktInfo, System::PacketBufferHandle && msg)
{
    INET_FAULT_INJECT(FaultInjection::kFault_Send, return INET_ERROR_UNKNOWN_INTERFACE;);
    INET_FAULT_INJECT(FaultInjection::kFault_SendNonCritical, return CHIP_ERROR_NO_MEMORY;);

    ReturnErrorOnFailure(SendMsgImpl(pktInfo, std::move(msg)));

    CHIP_SYSTEM_FAULT_INJECT_ASYNC_EVENT();

    return CHIP_NO_ERROR;
}

void UDPEndPoint::Close()
{
    if (mState != kState_Closed)
    {
        CloseImpl();
        mState = kState_Closed;
    }
}

void UDPEndPoint::Init(InetLayer * inetLayer)
{
    IPEndPointBasis::Init(inetLayer);
}

} // namespace Inet
} // namespace chip
