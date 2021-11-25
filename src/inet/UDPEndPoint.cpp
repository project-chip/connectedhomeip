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
#include <inet/UDPEndPoint.h>

#include <inet/IPPacketInfo.h>
#include <inet/InetFaultInjection.h>
#include <inet/arpa-inet-compatibility.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>
#include <system/SystemFaultInjection.h>

#include <cstring>
#include <utility>

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#if CHIP_HAVE_CONFIG_H
#include <lwip/lwip_buildconfig.h> // nogncheck
#endif                             // CHIP_HAVE_CONFIG_H

#if INET_CONFIG_ENABLE_IPV4
#include <lwip/igmp.h>
#endif // INET_CONFIG_ENABLE_IPV4

#include <lwip/init.h>
#include <lwip/ip.h>
#include <lwip/mld6.h>
#include <lwip/netif.h>
#include <lwip/raw.h>
#include <lwip/tcpip.h>
#include <lwip/udp.h>

#if !defined(RAW_FLAGS_MULTICAST_LOOP) || !defined(UDP_FLAGS_MULTICAST_LOOP) || !defined(raw_clear_flags) ||                       \
    !defined(raw_set_flags) || !defined(udp_clear_flags) || !defined(udp_set_flags)
#define HAVE_LWIP_MULTICAST_LOOP 0
#else
#define HAVE_LWIP_MULTICAST_LOOP 1
#endif // !defined(RAW_FLAGS_MULTICAST_LOOP) || !defined(UDP_FLAGS_MULTICAST_LOOP) || !defined(raw_clear_flags) ||
       // !defined(raw_set_flags) || !defined(udp_clear_flags) || !defined(udp_set_flags)

// unusual define check for LWIP_IPV6_ND is because espressif fork
// of LWIP does not define the _ND constant.
#if LWIP_IPV6_MLD && (!defined(LWIP_IPV6_ND) || LWIP_IPV6_ND) && LWIP_IPV6
#define HAVE_IPV6_MULTICAST
#else
// Within Project CHIP multicast support is highly desirable: used for mDNS
// as well as group communication.
#undef HAVE_IPV6_MULTICAST
#endif
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif // HAVE_SYS_SOCKET_H

#include <cerrno>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <unistd.h>

// SOCK_CLOEXEC not defined on all platforms, e.g. iOS/macOS:
#ifndef SOCK_CLOEXEC
#define SOCK_CLOEXEC 0
#endif

#if CHIP_SYSTEM_CONFIG_USE_ZEPHYR_SOCKET_EXTENSIONS
#include "ZephyrSocket.h"
#endif // CHIP_SYSTEM_CONFIG_USE_ZEPHYR_SOCKET_EXTENSIONS

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
#elif !__ZEPHYR__
#error                                                                                                                             \
    "Neither IPV6_ADD_MEMBERSHIP nor IPV6_JOIN_GROUP are defined which are required for generalized IPv6 multicast group support."
#endif // IPV6_ADD_MEMBERSHIP

#if defined(IPV6_DROP_MEMBERSHIP)
#define INET_IPV6_DROP_MEMBERSHIP IPV6_DROP_MEMBERSHIP
#elif defined(IPV6_LEAVE_GROUP)
#define INET_IPV6_DROP_MEMBERSHIP IPV6_LEAVE_GROUP
#elif !__ZEPHYR__
#error                                                                                                                             \
    "Neither IPV6_DROP_MEMBERSHIP nor IPV6_LEAVE_GROUP are defined which are required for generalized IPv6 multicast group support."
#endif // IPV6_DROP_MEMBERSHIP
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

#if CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK
#define INET_PORTSTRLEN 6
#endif // CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK

namespace chip {
namespace Inet {

#if CHIP_SYSTEM_CONFIG_USE_LWIP || CHIP_SYSTEM_CONFIG_USE_SOCKETS

namespace {

CHIP_ERROR CheckMulticastGroupArgs(InterfaceId aInterfaceId, const IPAddress & aAddress)
{
    VerifyOrReturnError(aInterfaceId.IsPresent(), INET_ERROR_UNKNOWN_INTERFACE);
    VerifyOrReturnError(aAddress.IsMulticast(), INET_ERROR_WRONG_ADDRESS_TYPE);
    return CHIP_NO_ERROR;
}

} // anonymous namespace

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP || CHIP_SYSTEM_CONFIG_USE_SOCKETS

#if CHIP_SYSTEM_CONFIG_USE_LWIP

CHIP_ERROR UDPEndPointImplLwIP::BindImpl(IPAddressType addressType, const IPAddress & address, uint16_t port,
                                         InterfaceId interfaceId)
{
    // Lock LwIP stack
    LOCK_TCPIP_CORE();

    // Make sure we have the appropriate type of PCB.
    CHIP_ERROR res = GetPCB(addressType);

    // Bind the PCB to the specified address/port.
    if (res == CHIP_NO_ERROR)
    {
#if LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
        ip_addr_t ipAddr = address.ToLwIPAddr();

        // TODO: IPAddress ANY has only one constant state, however addressType
        // has separate IPV4 and IPV6 'any' settings. This tries to correct
        // for this as LWIP default if IPv4 is compiled in is to consider
        // 'any == any_v4'
        //
        // We may want to consider having separate AnyV4 and AnyV6 constants
        // inside CHIP to resolve this ambiguity
        if ((address.Type() == IPAddressType::kAny) && (addressType == IPAddressType::kIPv6))
        {
            ipAddr = *IP6_ADDR_ANY;
        }

        res = chip::System::MapErrorLwIP(udp_bind(mUDP, &ipAddr, port));
#else // LWIP_VERSION_MAJOR <= 1 && LWIP_VERSION_MINOR < 5
        if (addressType == IPAddressType::kIPv6)
        {
            ip6_addr_t ipv6Addr = address.ToIPv6();
            res                 = chip::System::MapErrorLwIP(udp_bind_ip6(mUDP, &ipv6Addr, port));
        }
#if INET_CONFIG_ENABLE_IPV4
        else if (addressType == IPAddressType::kIPv4)
        {
            ip4_addr_t ipv4Addr = address.ToIPv4();
            res                 = chip::System::MapErrorLwIP(udp_bind(mUDP, &ipv4Addr, port));
        }
#endif // INET_CONFIG_ENABLE_IPV4
        else
            res = INET_ERROR_WRONG_ADDRESS_TYPE;
#endif // LWIP_VERSION_MAJOR <= 1 || LWIP_VERSION_MINOR >= 5
    }

    if (res == CHIP_NO_ERROR)
    {
        res = LwIPBindInterface(mUDP, interfaceId);
    }

    // Unlock LwIP stack
    UNLOCK_TCPIP_CORE();

    return res;
}

CHIP_ERROR UDPEndPointImplLwIP::BindInterfaceImpl(IPAddressType addrType, InterfaceId intfId)
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

CHIP_ERROR UDPEndPointImplLwIP::LwIPBindInterface(struct udp_pcb * aUDP, InterfaceId intfId)
{
    struct netif * netifp = nullptr;
    if (intfId.IsPresent())
    {
        netifp = UDPEndPointImplLwIP::FindNetifFromInterfaceId(intfId);
        if (netifp == nullptr)
        {
            return INET_ERROR_UNKNOWN_INTERFACE;
        }
    }

    udp_bind_netif(aUDP, netifp);
    return CHIP_NO_ERROR;
}

InterfaceId UDPEndPointImplLwIP::GetBoundInterface() const
{
#if HAVE_LWIP_UDP_BIND_NETIF
    return InterfaceId(netif_get_by_index(mUDP->netif_idx));
#else
    return InterfaceId(mUDP->intf_filter);
#endif
}

uint16_t UDPEndPointImplLwIP::GetBoundPort() const
{
    return mUDP->local_port;
}

CHIP_ERROR UDPEndPointImplLwIP::ListenImpl()
{
    // Lock LwIP stack
    LOCK_TCPIP_CORE();

#if LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
    udp_recv(mUDP, LwIPReceiveUDPMessage, this);
#else  // LWIP_VERSION_MAJOR <= 1 && LWIP_VERSION_MINOR < 5
    if (PCB_ISIPV6(mUDP))
    {
        udp_recv_ip6(mUDP, LwIPReceiveUDPMessage, this);
    }
    else
    {
        udp_recv(mUDP, LwIPReceiveUDPMessage, this);
    }
#endif // LWIP_VERSION_MAJOR <= 1 || LWIP_VERSION_MINOR >= 5

    // Unlock LwIP stack
    UNLOCK_TCPIP_CORE();

    return CHIP_NO_ERROR;
}

CHIP_ERROR UDPEndPointImplLwIP::SendMsgImpl(const IPPacketInfo * pktInfo, System::PacketBufferHandle && msg)
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

    if (intfId.IsPresent())
    {
        lwipErr = udp_sendto_if(mUDP, System::LwIPPacketBufferView::UnsafeGetLwIPpbuf(msg), &lwipDestAddr, destPort,
                                intfId.GetPlatformInterface());
    }
    else
    {
        lwipErr = udp_sendto(mUDP, System::LwIPPacketBufferView::UnsafeGetLwIPpbuf(msg), &lwipDestAddr, destPort);
    }

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

        if (intfId.IsPresent())
        {
            lwipErr =
                udp_sendto_if_ip6(mUDP, System::LwIPPacketBufferView::UnsafeGetLwIPpbuf(msg), &lwipDestAddr, destPort, intfId);
        }
        else
        {
            lwipErr = udp_sendto_ip6(mUDP, System::LwIPPacketBufferView::UnsafeGetLwIPpbuf(msg), &lwipDestAddr, destPort);
        }
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

        if (intfId.IsPresent())
        {
            lwipErr = udp_sendto_if(mUDP, System::LwIPPacketBufferView::UnsafeGetLwIPpbuf(msg), &lwipDestAddr, destPort, intfId);
        }
        else
        {
            lwipErr = udp_sendto(mUDP, System::LwIPPacketBufferView::UnsafeGetLwIPpbuf(msg), &lwipDestAddr, destPort);
        }
    }

    ipX_addr_copy(mUDP->local_ip, boundAddr);

#endif // INET_CONFIG_ENABLE_IPV4
#endif // LWIP_VERSION_MAJOR <= 1 || LWIP_VERSION_MINOR >= 5

    // Unlock LwIP stack
    UNLOCK_TCPIP_CORE();

    if (lwipErr != ERR_OK)
    {
        res = chip::System::MapErrorLwIP(lwipErr);
    }

    return res;
}

void UDPEndPointImplLwIP::CloseImpl()
{

    // Lock LwIP stack
    LOCK_TCPIP_CORE();

    // Since UDP PCB is released synchronously here, but UDP endpoint itself might have to wait
    // for destruction asynchronously, there could be more allocated UDP endpoints than UDP PCBs.
    if (mUDP != nullptr)
    {
        udp_remove(mUDP);
        mUDP              = nullptr;
        mLwIPEndPointType = LwIPEndPointType::Unknown;
    }

    // Unlock LwIP stack
    UNLOCK_TCPIP_CORE();
}

void UDPEndPointImplLwIP::Free()
{
    Close();
    Release();
}

void UDPEndPointImplLwIP::HandleDataReceived(System::PacketBufferHandle && msg)
{
    if ((mState == State::kListening) && (OnMessageReceived != nullptr))
    {
        const IPPacketInfo * pktInfo = GetPacketInfo(msg);

        if (pktInfo != nullptr)
        {
            const IPPacketInfo pktInfoCopy = *pktInfo; // copy the address info so that the app can free the
                                                       // PacketBuffer without affecting access to address info.
            OnMessageReceived(this, std::move(msg), &pktInfoCopy);
        }
        else
        {
            if (OnReceiveError != nullptr)
            {
                OnReceiveError(this, CHIP_ERROR_INBOUND_MESSAGE_TOO_BIG, nullptr);
            }
        }
    }
}

CHIP_ERROR UDPEndPointImplLwIP::GetPCB(IPAddressType addrType)
{
    // IMPORTANT: This method MUST be called with the LwIP stack LOCKED!

    // If a PCB hasn't been allocated yet...
    if (mUDP == nullptr)
    {
        // Allocate a PCB of the appropriate type.
        if (addrType == IPAddressType::kIPv6)
        {
#if LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
            mUDP = udp_new_ip_type(IPADDR_TYPE_V6);
#else  // LWIP_VERSION_MAJOR <= 1 && LWIP_VERSION_MINOR < 5
            mUDP = udp_new_ip6();
#endif // LWIP_VERSION_MAJOR <= 1 || LWIP_VERSION_MINOR >= 5
        }
#if INET_CONFIG_ENABLE_IPV4
        else if (addrType == IPAddressType::kIPv4)
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
        if (mUDP == nullptr)
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
            pcbAddrType = IPAddressType::kIPv6;
            break;
#if INET_CONFIG_ENABLE_IPV4
        case IPADDR_TYPE_V4:
            pcbAddrType = IPAddressType::kIPv4;
            break;
#endif // INET_CONFIG_ENABLE_IPV4
        default:
            return INET_ERROR_WRONG_ADDRESS_TYPE;
        }
#else // LWIP_VERSION_MAJOR <= 1 && LWIP_VERSION_MINOR < 5
#if INET_CONFIG_ENABLE_IPV4
        pcbAddrType = PCB_ISIPV6(mUDP) ? IPAddressType::kIPv6 : IPAddressType::kIPv4;
#else  // !INET_CONFIG_ENABLE_IPV4
        pcbAddrType = IPAddressType::kIPv6;
#endif // !INET_CONFIG_ENABLE_IPV4
#endif // LWIP_VERSION_MAJOR <= 1 && LWIP_VERSION_MINOR < 5

        // Fail if the existing PCB is not the correct type.
        VerifyOrReturnError(addrType == pcbAddrType, INET_ERROR_WRONG_ADDRESS_TYPE);
    }

    return CHIP_NO_ERROR;
}

#if LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
void UDPEndPointImplLwIP::LwIPReceiveUDPMessage(void * arg, struct udp_pcb * pcb, struct pbuf * p, const ip_addr_t * addr,
                                                u16_t port)
#else  // LWIP_VERSION_MAJOR <= 1 && LWIP_VERSION_MINOR < 5
void UDPEndPointImplLwIP::LwIPReceiveUDPMessage(void * arg, struct udp_pcb * pcb, struct pbuf * p, ip_addr_t * addr, u16_t port)
#endif // LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
{
    UDPEndPointImplLwIP * ep       = static_cast<UDPEndPointImplLwIP *>(arg);
    IPPacketInfo * pktInfo         = nullptr;
    System::PacketBufferHandle buf = System::PacketBufferHandle::Adopt(p);
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
    if (pktInfo != nullptr)
    {
#if LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
        pktInfo->SrcAddress  = IPAddress(*addr);
        pktInfo->DestAddress = IPAddress(*ip_current_dest_addr());
#else // LWIP_VERSION_MAJOR <= 1
        if (PCB_ISIPV6(pcb))
        {
            pktInfo->SrcAddress  = IPAddress(*(ip6_addr_t *) addr);
            pktInfo->DestAddress = IPAddress(*ip6_current_dest_addr());
        }
#if INET_CONFIG_ENABLE_IPV4
        else
        {
            pktInfo->SrcAddress  = IPAddress(*addr);
            pktInfo->DestAddress = IPAddress(*ip_current_dest_addr());
        }
#endif // INET_CONFIG_ENABLE_IPV4
#endif // LWIP_VERSION_MAJOR <= 1

        pktInfo->Interface = InterfaceId(ip_current_netif());
        pktInfo->SrcPort   = port;
        pktInfo->DestPort  = pcb->local_port;
    }

    ep->Retain();
    CHIP_ERROR err = ep->GetSystemLayer().ScheduleLambda([ep, p = System::LwIPPacketBufferView::UnsafeGetLwIPpbuf(buf)] {
        ep->HandleDataReceived(System::PacketBufferHandle::Adopt(p));
        ep->Release();
    });
    if (err == CHIP_NO_ERROR)
    {
        // If ScheduleLambda() succeeded, it has ownership of the buffer, so we need to release it (without freeing it).
        static_cast<void>(std::move(buf).UnsafeRelease());
    }
    else
    {
        ep->Release();
    }
}

CHIP_ERROR UDPEndPointImplLwIP::SetMulticastLoopback(IPVersion aIPVersion, bool aLoopback)
{
#if HAVE_LWIP_MULTICAST_LOOP
    if (mLwIPEndPointType == LwIPEndPointType::UDP)
    {
        if (aLoopback)
        {
            udp_set_flags(mUDP, UDP_FLAGS_MULTICAST_LOOP);
        }
        else
        {
            udp_clear_flags(mUDP, UDP_FLAGS_MULTICAST_LOOP);
        }
        return CHIP_NO_ERROR;
    }
#endif // HAVE_LWIP_MULTICAST_LOOP
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

#if INET_CONFIG_ENABLE_IPV4
CHIP_ERROR UDPEndPointImplLwIP::IPv4JoinLeaveMulticastGroupImpl(InterfaceId aInterfaceId, const IPAddress & aAddress, bool join)
{
#if LWIP_IPV4 && LWIP_IGMP
    const auto method = join ? igmp_joingroup_netif : igmp_leavegroup_netif;

    struct netif * const lNetif = FindNetifFromInterfaceId(aInterfaceId);
    VerifyOrReturnError(lNetif != nullptr, INET_ERROR_UNKNOWN_INTERFACE);

    const ip4_addr_t lIPv4Address = aAddress.ToIPv4();
    const err_t lStatus           = method(lNetif, &lIPv4Address);
    if (lStatus == ERR_MEM)
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    return chip::System::MapErrorLwIP(lStatus);
#else  // LWIP_IPV4 && LWIP_IGMP
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif // LWIP_IPV4 && LWIP_IGMP
}
#endif // INET_CONFIG_ENABLE_IPV4

CHIP_ERROR UDPEndPointImplLwIP::IPv6JoinLeaveMulticastGroupImpl(InterfaceId aInterfaceId, const IPAddress & aAddress, bool join)
{
#ifdef HAVE_IPV6_MULTICAST
    const auto method = join ? mld6_joingroup_netif : mld6_leavegroup_netif;

    struct netif * const lNetif = FindNetifFromInterfaceId(aInterfaceId);
    VerifyOrReturnError(lNetif != nullptr, INET_ERROR_UNKNOWN_INTERFACE);

    const ip6_addr_t lIPv6Address = aAddress.ToIPv6();
    const err_t lStatus           = method(lNetif, &lIPv6Address);
    if (lStatus == ERR_MEM)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    return chip::System::MapErrorLwIP(lStatus);
#else  // HAVE_IPV6_MULTICAST
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif // HAVE_IPV6_MULTICAST
}

struct netif * UDPEndPointImplLwIP::FindNetifFromInterfaceId(InterfaceId aInterfaceId)
{
    struct netif * lRetval = nullptr;

#if LWIP_VERSION_MAJOR >= 2 && LWIP_VERSION_MINOR >= 0 && defined(NETIF_FOREACH)
    NETIF_FOREACH(lRetval)
    {
        if (lRetval == aInterfaceId.GetPlatformInterface())
        {
            break;
        }
    }
#else  // LWIP_VERSION_MAJOR < 2 || !defined(NETIF_FOREACH)
    for (lRetval = netif_list; lRetval != nullptr && lRetval != aInterfaceId.GetPlatformInterface(); lRetval = lRetval->next)
        ;
#endif // LWIP_VERSION_MAJOR >= 2 && LWIP_VERSION_MINOR >= 0 && defined(NETIF_FOREACH)

    return (lRetval);
}

IPPacketInfo * UDPEndPointImplLwIP::GetPacketInfo(const System::PacketBufferHandle & aBuffer)
{
    if (!aBuffer->EnsureReservedSize(sizeof(IPPacketInfo) + 3))
    {
        return nullptr;
    }

    uintptr_t lStart           = (uintptr_t) aBuffer->Start();
    uintptr_t lPacketInfoStart = lStart - sizeof(IPPacketInfo);

    // Align to a 4-byte boundary
    return reinterpret_cast<IPPacketInfo *>(lPacketInfoStart & ~(sizeof(uint32_t) - 1));
}

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS

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
UDPEndPointImplSockets::MulticastGroupHandler UDPEndPointImplSockets::sJoinMulticastGroupHandler;
UDPEndPointImplSockets::MulticastGroupHandler UDPEndPointImplSockets::sLeaveMulticastGroupHandler;
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

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
    dispatch_queue_t dispatchQueue = static_cast<System::LayerSocketsLoop *>(&GetSystemLayer())->GetDispatchQueue();
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

    // Send IP packet.
    const ssize_t lenSent = sendmsg(mSocket, &msgHeader, 0);
    if (lenSent == -1)
    {
        return CHIP_ERROR_POSIX(errno);
    }
    if (lenSent != msg->DataLength())
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

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
    if (mReadableSource)
    {
        dispatch_source_cancel(mReadableSource);
        dispatch_release(mReadableSource);
    }
#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH
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
        ReturnErrorOnFailure(static_cast<System::LayerSockets *>(&GetSystemLayer())->StartWatchingSocket(mSocket, &mWatch));

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
    lPacketInfo.DestPort = mBoundPort;

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

        if (rcvLen < 0)
        {
            lStatus = CHIP_ERROR_POSIX(errno);
        }
        else if (rcvLen > lBuffer->AvailableDataLength())
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

#if IP_MULTICAST_LOOP || IPV6_MULTICAST_LOOP
static CHIP_ERROR SocketsSetMulticastLoopback(int aSocket, bool aLoopback, int aProtocol, int aOption)
{
    const unsigned int lValue = static_cast<unsigned int>(aLoopback);
    if (setsockopt(aSocket, aProtocol, aOption, &lValue, sizeof(lValue)) != 0)
    {
        return CHIP_ERROR_POSIX(errno);
    }

    return CHIP_NO_ERROR;
}
#endif // IP_MULTICAST_LOOP || IPV6_MULTICAST_LOOP

static CHIP_ERROR SocketsSetMulticastLoopback(int aSocket, IPVersion aIPVersion, bool aLoopback)
{
#ifdef IPV6_MULTICAST_LOOP
    CHIP_ERROR lRetval;

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
    IPAddress lInterfaceAddress;
    bool lInterfaceAddressFound = false;

    for (InterfaceAddressIterator lAddressIterator; lAddressIterator.HasCurrent(); lAddressIterator.Next())
    {
        const IPAddress lCurrentAddress = lAddressIterator.GetAddress();

        if (lAddressIterator.GetInterfaceId() == aInterfaceId)
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

    struct ip_mreq lMulticastRequest;
    memset(&lMulticastRequest, 0, sizeof(lMulticastRequest));
    lMulticastRequest.imr_interface = lInterfaceAddress.ToIPv4();
    lMulticastRequest.imr_multiaddr = aAddress.ToIPv4();

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
    MulticastGroupHandler handler = join ? sJoinMulticastGroupHandler : sLeaveMulticastGroupHandler;
    if (handler != nullptr)
    {
        return handler(aInterfaceId, aAddress);
    }
#endif // CHIP_SYSTEM_CONFIG_USE_PLATFORM_MULTICAST_API

#if defined(INET_IPV6_ADD_MEMBERSHIP) && defined(INET_IPV6_DROP_MEMBERSHIP)
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
#else  // defined(INET_IPV6_ADD_MEMBERSHIP) && defined(INET_IPV6_DROP_MEMBERSHIP)
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif // defined(INET_IPV6_ADD_MEMBERSHIP) && defined(INET_IPV6_DROP_MEMBERSHIP)
}

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

#if CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK

CHIP_ERROR UDPEndPointImplNetworkFramework::BindImpl(IPAddressType addressType, const IPAddress & address, uint16_t port,
                                                     InterfaceId intfId)
{
    nw_parameters_configure_protocol_block_t configure_tls;
    nw_parameters_t parameters;

    if (intfId.IsPresent())
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    configure_tls = NW_PARAMETERS_DISABLE_PROTOCOL;
    parameters    = nw_parameters_create_secure_udp(configure_tls, NW_PARAMETERS_DEFAULT_CONFIGURATION);
    VerifyOrReturnError(parameters != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    ReturnErrorOnFailure(ConfigureProtocol(addressType, parameters));

    nw_endpoint_t endpoint = nullptr;
    CHIP_ERROR res         = GetEndPoint(endpoint, addressType, address, port);
    nw_parameters_set_local_endpoint(parameters, endpoint);
    nw_release(endpoint);
    ReturnErrorOnFailure(res);

    mDispatchQueue = dispatch_queue_create("inet_dispatch_global", DISPATCH_QUEUE_CONCURRENT);
    VerifyOrReturnError(mDispatchQueue != nullptr, CHIP_ERROR_NO_MEMORY);
    dispatch_retain(mDispatchQueue);

    mConnectionSemaphore = dispatch_semaphore_create(0);
    VerifyOrReturnError(mConnectionSemaphore != nullptr, CHIP_ERROR_NO_MEMORY);
    dispatch_retain(mConnectionSemaphore);

    mSendSemaphore = dispatch_semaphore_create(0);
    VerifyOrReturnError(mSendSemaphore != nullptr, CHIP_ERROR_NO_MEMORY);
    dispatch_retain(mSendSemaphore);

    mAddrType   = addressType;
    mConnection = nullptr;
    mParameters = parameters;

    return CHIP_NO_ERROR;
}

CHIP_ERROR UDPEndPointImplNetworkFramework::BindInterfaceImpl(IPAddressType addrType, InterfaceId intfId)
{
    return INET_ERROR_UNKNOWN_INTERFACE;
}

InterfaceId UDPEndPointImplNetworkFramework::GetBoundInterface() const
{
    return InterfaceId::Null();
}

uint16_t UDPEndPointImplNetworkFramework::GetBoundPort() const
{
    nw_endpoint_t endpoint = nw_parameters_copy_local_endpoint(mParameters);
    return nw_endpoint_get_port(endpoint);
}

CHIP_ERROR UDPEndPointImplNetworkFramework::ListenImpl()
{
    return StartListener();
}

CHIP_ERROR UDPEndPointImplNetworkFramework::SendMsgImpl(const IPPacketInfo * pktInfo, System::PacketBufferHandle && msg)
{
    dispatch_data_t content;

    // Ensure the destination address type is compatible with the endpoint address type.
    VerifyOrReturnError(mAddrType == pktInfo->DestAddress.Type(), CHIP_ERROR_INVALID_ARGUMENT);

    // For now the entire message must fit within a single buffer.
    VerifyOrReturnError(msg->Next() == nullptr, CHIP_ERROR_MESSAGE_TOO_LONG);

    ReturnErrorOnFailure(GetConnection(pktInfo));

    // Send a message, and wait for it to be dispatched.
    content = dispatch_data_create(msg->Start(), msg->DataLength(), mDispatchQueue, DISPATCH_DATA_DESTRUCTOR_DEFAULT);

    // If there is a current message pending and the state of the network connection change (e.g switch to a
    // different network) the connection will enter a nw_connection_state_failed state and the completion handler
    // will never be called. In such cases a signal is sent from the connection state change handler to release
    // the semaphore. In this case the CHIP_ERROR will not update with the result of the completion handler.
    // To make sure caller knows that sending a message has failed the following code consider there is an error
    // _unless_ the completion handler says otherwise.
    __block CHIP_ERROR res = CHIP_ERROR_UNEXPECTED_EVENT;
    nw_connection_send(mConnection, content, NW_CONNECTION_DEFAULT_MESSAGE_CONTEXT, true, ^(nw_error_t error) {
        if (error)
        {
            res = CHIP_ERROR_POSIX(nw_error_get_error_code(error));
        }
        else
        {
            res = CHIP_NO_ERROR;
        }
        dispatch_semaphore_signal(mSendSemaphore);
    });
    dispatch_release(content);

    dispatch_semaphore_wait(mSendSemaphore, DISPATCH_TIME_FOREVER);

    return res;
}

void UDPEndPointImplNetworkFramework::CloseImpl()
{
    ReleaseAll();
}

void UDPEndPointImplNetworkFramework::ReleaseAll()
{

    OnMessageReceived = nullptr;
    OnReceiveError    = nullptr;

    ReleaseConnection();
    ReleaseListener();

    if (mParameters)
    {
        nw_release(mParameters);
        mParameters = nullptr;
    }

    if (mDispatchQueue)
    {
        dispatch_suspend(mDispatchQueue);
        dispatch_release(mDispatchQueue);
        mDispatchQueue = nullptr;
    }

    if (mConnectionSemaphore)
    {
        dispatch_release(mConnectionSemaphore);
        mConnectionSemaphore = nullptr;
    }

    if (mListenerQueue)
    {
        dispatch_suspend(mListenerQueue);
        dispatch_release(mListenerQueue);
        mListenerQueue = nullptr;
    }

    if (mListenerSemaphore)
    {
        dispatch_release(mListenerSemaphore);
        mListenerSemaphore = nullptr;
    }

    if (mSendSemaphore)
    {
        dispatch_release(mSendSemaphore);
        mSendSemaphore = nullptr;
    }
}

void UDPEndPointImplNetworkFramework::Free()
{
    Close();
    Release();
}

CHIP_ERROR UDPEndPointImplNetworkFramework::SetMulticastLoopback(IPVersion aIPVersion, bool aLoopback)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

#if INET_CONFIG_ENABLE_IPV4
CHIP_ERROR UDPEndPointImplNetworkFramework::IPv4JoinLeaveMulticastGroupImpl(InterfaceId aInterfaceId, const IPAddress & aAddress,
                                                                            bool join)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}
#endif // INET_CONFIG_ENABLE_IPV4

CHIP_ERROR UDPEndPointImplNetworkFramework::IPv6JoinLeaveMulticastGroupImpl(InterfaceId aInterfaceId, const IPAddress & aAddress,
                                                                            bool join)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR UDPEndPointImplNetworkFramework::ConfigureProtocol(IPAddressType aAddressType, const nw_parameters_t & aParameters)
{
    CHIP_ERROR res = CHIP_NO_ERROR;

    nw_protocol_stack_t protocolStack = nw_parameters_copy_default_protocol_stack(aParameters);
    nw_protocol_options_t ipOptions   = nw_protocol_stack_copy_internet_protocol(protocolStack);

    switch (aAddressType)
    {

    case IPAddressType::kIPv6:
        nw_ip_options_set_version(ipOptions, nw_ip_version_6);
        break;

#if INET_CONFIG_ENABLE_IPV4
    case IPAddressType::kIPv4:
        nw_ip_options_set_version(ipOptions, nw_ip_version_4);
        break;
#endif // INET_CONFIG_ENABLE_IPV4

    default:
        res = INET_ERROR_WRONG_ADDRESS_TYPE;
        break;
    }
    nw_release(ipOptions);
    nw_release(protocolStack);

    return res;
}

void UDPEndPointImplNetworkFramework::GetPacketInfo(const nw_connection_t & aConnection, IPPacketInfo & aPacketInfo)
{
    nw_path_t path              = nw_connection_copy_current_path(aConnection);
    nw_endpoint_t dest_endpoint = nw_path_copy_effective_local_endpoint(path);
    nw_endpoint_t src_endpoint  = nw_path_copy_effective_remote_endpoint(path);

    aPacketInfo.Clear();
    aPacketInfo.SrcAddress  = IPAddress::FromSockAddr(*nw_endpoint_get_address(src_endpoint));
    aPacketInfo.DestAddress = IPAddress::FromSockAddr(*nw_endpoint_get_address(dest_endpoint));
    aPacketInfo.SrcPort     = nw_endpoint_get_port(src_endpoint);
    aPacketInfo.DestPort    = nw_endpoint_get_port(dest_endpoint);
}

CHIP_ERROR UDPEndPointImplNetworkFramework::GetEndPoint(nw_endpoint_t & aEndPoint, const IPAddressType aAddressType,
                                                        const IPAddress & aAddress, uint16_t aPort)
{
    char addrStr[INET6_ADDRSTRLEN];
    char portStr[INET_PORTSTRLEN];

    // Note: aAddress.ToString will return the IPv6 Any address if the address type is Any, but that's not what
    // we want if the locale endpoint is IPv4.
    if (aAddressType == IPAddressType::kIPv4 && aAddress.Type() == IPAddressType::kAny)
    {
        const IPAddress anyAddr = IPAddress(aAddress.ToIPv4());
        anyAddr.ToString(addrStr, sizeof(addrStr));
    }
    else
    {
        aAddress.ToString(addrStr, sizeof(addrStr));
    }

    snprintf(portStr, sizeof(portStr), "%u", aPort);

    aEndPoint = nw_endpoint_create_host(addrStr, portStr);
    VerifyOrReturnError(aEndPoint != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    return CHIP_NO_ERROR;
}

CHIP_ERROR UDPEndPointImplNetworkFramework::GetConnection(const IPPacketInfo * aPktInfo)
{
    VerifyOrReturnError(mParameters != nullptr, CHIP_ERROR_INCORRECT_STATE);

    nw_endpoint_t endpoint     = nullptr;
    nw_connection_t connection = nullptr;

    if (mConnection)
    {
        nw_path_t path                 = nw_connection_copy_current_path(mConnection);
        nw_endpoint_t remote_endpoint  = nw_path_copy_effective_remote_endpoint(path);
        const IPAddress remote_address = IPAddress::FromSockAddr(*nw_endpoint_get_address(remote_endpoint));
        const uint16_t remote_port     = nw_endpoint_get_port(remote_endpoint);
        const bool isDifferentEndPoint = aPktInfo->DestPort != remote_port || aPktInfo->DestAddress != remote_address;
        VerifyOrReturnError(isDifferentEndPoint, CHIP_NO_ERROR);

        ReturnErrorOnFailure(ReleaseConnection());
    }

    ReturnErrorOnFailure(GetEndPoint(endpoint, mAddrType, aPktInfo->DestAddress, aPktInfo->DestPort));

    connection = nw_connection_create(endpoint, mParameters);
    nw_release(endpoint);

    VerifyOrReturnError(connection != nullptr, CHIP_ERROR_INCORRECT_STATE);

    return StartConnection(connection);
}

CHIP_ERROR UDPEndPointImplNetworkFramework::StartListener()
{
    __block CHIP_ERROR res = CHIP_NO_ERROR;
    nw_listener_t listener;

    VerifyOrReturnError(mListener == nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mListenerSemaphore == nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mListenerQueue == nullptr, CHIP_ERROR_INCORRECT_STATE);

    listener = nw_listener_create(mParameters);
    VerifyOrReturnError(listener != nullptr, CHIP_ERROR_INCORRECT_STATE);

    mListenerSemaphore = dispatch_semaphore_create(0);
    VerifyOrReturnError(mListenerSemaphore != nullptr, CHIP_ERROR_NO_MEMORY);
    dispatch_retain(mListenerSemaphore);

    mListenerQueue = dispatch_queue_create("inet_dispatch_listener", DISPATCH_QUEUE_CONCURRENT);
    VerifyOrReturnError(mListenerQueue != nullptr, CHIP_ERROR_NO_MEMORY);
    dispatch_retain(mListenerQueue);

    nw_listener_set_queue(listener, mListenerQueue);

    nw_listener_set_new_connection_handler(listener, ^(nw_connection_t connection) {
        ReleaseConnection();
        StartConnection(connection);
    });

    nw_listener_set_state_changed_handler(listener, ^(nw_listener_state_t state, nw_error_t error) {
        switch (state)
        {

        case nw_listener_state_invalid:
            ChipLogDetail(Inet, "Listener: Invalid");
            res = CHIP_ERROR_INCORRECT_STATE;
            nw_listener_cancel(listener);
            break;

        case nw_listener_state_waiting:
            ChipLogDetail(Inet, "Listener: Waiting");
            break;

        case nw_listener_state_failed:
            ChipLogDetail(Inet, "Listener: Failed");
            res = CHIP_ERROR_POSIX(nw_error_get_error_code(error));
            break;

        case nw_listener_state_ready:
            ChipLogDetail(Inet, "Listener: Ready");
            res = CHIP_NO_ERROR;
            dispatch_semaphore_signal(mListenerSemaphore);
            break;

        case nw_listener_state_cancelled:
            ChipLogDetail(Inet, "Listener: Cancelled");
            if (res == CHIP_NO_ERROR)
            {
                res = CHIP_ERROR_CONNECTION_ABORTED;
            }

            dispatch_semaphore_signal(mListenerSemaphore);
            break;
        }
    });

    nw_listener_start(listener);
    dispatch_semaphore_wait(mListenerSemaphore, DISPATCH_TIME_FOREVER);
    ReturnErrorOnFailure(res);

    mListener = listener;
    nw_retain(mListener);
    return res;
}

CHIP_ERROR UDPEndPointImplNetworkFramework::StartConnection(nw_connection_t & aConnection)
{
    __block CHIP_ERROR res = CHIP_NO_ERROR;

    nw_connection_set_queue(aConnection, mDispatchQueue);

    nw_connection_set_state_changed_handler(aConnection, ^(nw_connection_state_t state, nw_error_t error) {
        switch (state)
        {

        case nw_connection_state_invalid:
            ChipLogDetail(Inet, "Connection: Invalid");
            res = CHIP_ERROR_INCORRECT_STATE;
            nw_connection_cancel(aConnection);
            break;

        case nw_connection_state_preparing:
            ChipLogDetail(Inet, "Connection: Preparing");
            res = CHIP_ERROR_INCORRECT_STATE;
            break;

        case nw_connection_state_waiting:
            ChipLogDetail(Inet, "Connection: Waiting");
            nw_connection_cancel(aConnection);
            break;

        case nw_connection_state_failed:
            ChipLogDetail(Inet, "Connection: Failed");
            res = CHIP_ERROR_POSIX(nw_error_get_error_code(error));
            break;

        case nw_connection_state_ready:
            ChipLogDetail(Inet, "Connection: Ready");
            res = CHIP_NO_ERROR;
            dispatch_semaphore_signal(mConnectionSemaphore);
            break;

        case nw_connection_state_cancelled:
            ChipLogDetail(Inet, "Connection: Cancelled");
            if (res == CHIP_NO_ERROR)
            {
                res = CHIP_ERROR_CONNECTION_ABORTED;
            }

            dispatch_semaphore_signal(mConnectionSemaphore);
            break;
        }
    });

    nw_connection_start(aConnection);
    dispatch_semaphore_wait(mConnectionSemaphore, DISPATCH_TIME_FOREVER);
    SuccessOrExit(res);

    mConnection = aConnection;
    nw_retain(mConnection);
    HandleDataReceived(mConnection);

    return res;
}

void UDPEndPointImplNetworkFramework::HandleDataReceived(const nw_connection_t & aConnection)
{
    nw_connection_receive_completion_t handler =
        ^(dispatch_data_t content, nw_content_context_t context, bool is_complete, nw_error_t receive_error) {
            dispatch_block_t schedule_next_receive = ^{
                if (receive_error == nullptr)
                {
                    HandleDataReceived(aConnection);
                }
                else if (OnReceiveError != nullptr)
                {
                    nw_error_domain_t error_domain = nw_error_get_error_domain(receive_error);
                    errno                          = nw_error_get_error_code(receive_error);
                    if (!(error_domain == nw_error_domain_posix && errno == ECANCELED))
                    {
                        CHIP_ERROR error = CHIP_ERROR_POSIX(errno);
                        IPPacketInfo packetInfo;
                        GetPacketInfo(aConnection, packetInfo);
                        dispatch_async(mDispatchQueue, ^{
                            OnReceiveError((UDPEndPoint *) this, error, &packetInfo);
                        });
                    }
                }
            };

            if (content != nullptr && OnMessageReceived != nullptr)
            {
                size_t count                              = dispatch_data_get_size(content);
                System::PacketBufferHandle * packetBuffer = System::PacketBufferHandle::New(count);
                dispatch_data_apply(content, ^(dispatch_data_t data, size_t offset, const void * buffer, size_t size) {
                    memmove(packetBuffer->Start() + offset, buffer, size);
                    return true;
                });
                packetBuffer->SetDataLength(count);

                IPPacketInfo packetInfo;
                GetPacketInfo(aConnection, packetInfo);
                dispatch_async(mDispatchQueue, ^{
                    OnMessageReceived((UDPEndPoint *) this, packetBuffer, &packetInfo);
                });
            }

            schedule_next_receive();
        };

    nw_connection_receive_message(aConnection, handler);
}

CHIP_ERROR UDPEndPointImplNetworkFramework::ReleaseListener()
{
    VerifyOrReturnError(mListener, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mDispatchQueue, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mConnectionSemaphore, CHIP_ERROR_INCORRECT_STATE);

    nw_listener_cancel(mListener);
    dispatch_semaphore_wait(mListenerSemaphore, DISPATCH_TIME_FOREVER);
    nw_release(mListener);
    mListener = nullptr;

    return CHIP_NO_ERROR;
}

CHIP_ERROR UDPEndPointImplNetworkFramework::ReleaseConnection()
{
    VerifyOrReturnError(mConnection, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mDispatchQueue, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mConnectionSemaphore, CHIP_ERROR_INCORRECT_STATE);

    nw_connection_cancel(mConnection);
    dispatch_semaphore_wait(mConnectionSemaphore, DISPATCH_TIME_FOREVER);
    nw_release(mConnection);
    mConnection = nullptr;

    return CHIP_NO_ERROR;
}

#endif // CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK

CHIP_ERROR UDPEndPoint::Bind(IPAddressType addrType, const IPAddress & addr, uint16_t port, InterfaceId intfId)
{
    if (mState != State::kReady && mState != State::kBound)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    if ((addr != IPAddress::Any) && (addr.Type() != IPAddressType::kAny) && (addr.Type() != addrType))
    {
        return INET_ERROR_WRONG_ADDRESS_TYPE;
    }

    ReturnErrorOnFailure(BindImpl(addrType, addr, port, intfId));

    mState = State::kBound;

    return CHIP_NO_ERROR;
}

CHIP_ERROR UDPEndPoint::BindInterface(IPAddressType addrType, InterfaceId intfId)
{
    if (mState != State::kReady && mState != State::kBound)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    ReturnErrorOnFailure(BindInterfaceImpl(addrType, intfId));

    mState = State::kBound;

    return CHIP_NO_ERROR;
}

CHIP_ERROR UDPEndPoint::Listen(OnMessageReceivedFunct onMessageReceived, OnReceiveErrorFunct onReceiveError, void * appState)
{
    if (mState == State::kListening)
    {
        return CHIP_NO_ERROR;
    }

    if (mState != State::kBound)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    OnMessageReceived = onMessageReceived;
    OnReceiveError    = onReceiveError;
    mAppState         = appState;

    ReturnErrorOnFailure(ListenImpl());

    mState = State::kListening;

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
    if (mState != State::kClosed)
    {
        CloseImpl();
        mState = State::kClosed;
    }
}

CHIP_ERROR UDPEndPoint::JoinMulticastGroup(InterfaceId aInterfaceId, const IPAddress & aAddress)
{
    CHIP_ERROR lRetval = CHIP_ERROR_NOT_IMPLEMENTED;

    const IPAddressType lAddrType = aAddress.Type();
    lRetval                       = CheckMulticastGroupArgs(aInterfaceId, aAddress);
    SuccessOrExit(lRetval);

    switch (lAddrType)
    {

#if INET_CONFIG_ENABLE_IPV4
    case IPAddressType::kIPv4: {
        return IPv4JoinLeaveMulticastGroupImpl(aInterfaceId, aAddress, true);
    }
    break;
#endif // INET_CONFIG_ENABLE_IPV4

    case IPAddressType::kIPv6: {
        return IPv6JoinLeaveMulticastGroupImpl(aInterfaceId, aAddress, true);
    }
    break;

    default:
        lRetval = INET_ERROR_WRONG_ADDRESS_TYPE;
        break;
    }

exit:
    return (lRetval);
}

CHIP_ERROR UDPEndPoint::LeaveMulticastGroup(InterfaceId aInterfaceId, const IPAddress & aAddress)
{
    CHIP_ERROR lRetval = CHIP_ERROR_NOT_IMPLEMENTED;

    const IPAddressType lAddrType = aAddress.Type();
    lRetval                       = CheckMulticastGroupArgs(aInterfaceId, aAddress);
    SuccessOrExit(lRetval);

    switch (lAddrType)
    {

#if INET_CONFIG_ENABLE_IPV4
    case IPAddressType::kIPv4: {
        return IPv4JoinLeaveMulticastGroupImpl(aInterfaceId, aAddress, false);
    }
    break;
#endif // INET_CONFIG_ENABLE_IPV4

    case IPAddressType::kIPv6: {
        return IPv6JoinLeaveMulticastGroupImpl(aInterfaceId, aAddress, false);
    }
    break;

    default:
        lRetval = INET_ERROR_WRONG_ADDRESS_TYPE;
        break;
    }

exit:
    return (lRetval);
}

} // namespace Inet
} // namespace chip
