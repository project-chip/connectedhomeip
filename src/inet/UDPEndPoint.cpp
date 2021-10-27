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
#include <lib/support/SafeInt.h>
#include <system/SystemFaultInjection.h>

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
#endif                             // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif // HAVE_SYS_SOCKET_H

#include <errno.h>
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

#include "arpa-inet-compatibility.h"

#include <string.h>
#include <utility>

namespace chip {
namespace Inet {

chip::System::ObjectPool<UDPEndPoint, INET_CONFIG_NUM_UDP_ENDPOINTS> UDPEndPoint::sPool;

#if CHIP_SYSTEM_CONFIG_USE_LWIP || CHIP_SYSTEM_CONFIG_USE_SOCKETS

static CHIP_ERROR CheckMulticastGroupArgs(InterfaceId aInterfaceId, const IPAddress & aAddress)
{
    VerifyOrReturnError(aInterfaceId.IsPresent(), INET_ERROR_UNKNOWN_INTERFACE);
    VerifyOrReturnError(aAddress.IsMulticast(), INET_ERROR_WRONG_ADDRESS_TYPE);
    return CHIP_NO_ERROR;
}
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP || CHIP_SYSTEM_CONFIG_USE_SOCKETS

#if CHIP_SYSTEM_CONFIG_USE_LWIP

namespace {

/*
 * Note that for LwIP InterfaceId is already defined to be 'struct
 * netif'; consequently, some of the checking performed here could
 * conceivably be optimized out and the HAVE_LWIP_UDP_BIND_NETIF case
 * could simply be:
 *
 *   udp_bind_netif(aUDP, intfId);
 *
 */
CHIP_ERROR LwIPBindInterface(struct udp_pcb * aUDP, InterfaceId intfId)
{
    CHIP_ERROR res = CHIP_NO_ERROR;

#if HAVE_LWIP_UDP_BIND_NETIF
    if (!intfId.IsPresent())
        udp_bind_netif(aUDP, NULL);
    else
    {
        struct netif * netifp = UDPEndPoint::FindNetifFromInterfaceId(intfId);

        if (netifp == NULL)
            res = INET_ERROR_UNKNOWN_INTERFACE;
        else
            udp_bind_netif(aUDP, netifp);
    }
#else
    if (!intfId.IsPresent())
        aUDP->intf_filter = NULL;
    else
    {
        struct netif * netifp = FindNetifFromInterfaceId(intfId);

        if (netifp == NULL)
            res = INET_ERROR_UNKNOWN_INTERFACE;
        else
            aUDP->intf_filter = netifp;
    }
#endif // HAVE_LWIP_UDP_BIND_NETIF

    return res;
}

} // anonymous namespace

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
        if ((addr.Type() == IPAddressType::kAny) && (addrType == IPAddressType::kIPv6))
        {
            ipAddr = *IP6_ADDR_ANY;
        }

        res = chip::System::MapErrorLwIP(udp_bind(mUDP, &ipAddr, port));
#else // LWIP_VERSION_MAJOR <= 1 && LWIP_VERSION_MINOR < 5
        if (addrType == IPAddressType::kIPv6)
        {
            ip6_addr_t ipv6Addr = addr.ToIPv6();
            res                 = chip::System::MapErrorLwIP(udp_bind_ip6(mUDP, &ipv6Addr, port));
        }
#if INET_CONFIG_ENABLE_IPV4
        else if (addrType == IPAddressType::kIPv4)
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
    return InterfaceId(netif_get_by_index(mUDP->netif_idx));
#else
    return InterfaceId(mUDP->intf_filter);
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
        lwipErr = udp_sendto_if(mUDP, System::LwIPPacketBufferView::UnsafeGetLwIPpbuf(msg), &lwipDestAddr, destPort,
                                intfId.GetPlatformInterface());
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

        if (intfId.IsPresent())
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

        if (intfId.IsPresent())
            lwipErr = udp_sendto_if(mUDP, System::LwIPPacketBufferView::UnsafeGetLwIPpbuf(msg), &lwipDestAddr, destPort, intfId);
        else
            lwipErr = udp_sendto(mUDP, System::LwIPPacketBufferView::UnsafeGetLwIPpbuf(msg), &lwipDestAddr, destPort);
    }

    ipX_addr_copy(mUDP->local_ip, boundAddr);

#endif // INET_CONFIG_ENABLE_IPV4
#endif // LWIP_VERSION_MAJOR <= 1 || LWIP_VERSION_MINOR >= 5

    // Unlock LwIP stack
    UNLOCK_TCPIP_CORE();

    if (lwipErr != ERR_OK)
        res = chip::System::MapErrorLwIP(lwipErr);

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
    IpHandleDataReceived(std::move(msg));
}

void UDPEndPoint::IpHandleDataReceived(System::PacketBufferHandle && aBuffer)
{
    if ((mState == State::kListening) && (OnMessageReceived != NULL))
    {
        const IPPacketInfo * pktInfo = GetPacketInfo(aBuffer);

        if (pktInfo != NULL)
        {
            const IPPacketInfo pktInfoCopy = *pktInfo; // copy the address info so that the app can free the
                                                       // PacketBuffer without affecting access to address info.
            OnMessageReceived(this, std::move(aBuffer), &pktInfoCopy);
        }
        else
        {
            if (OnReceiveError != NULL)
                OnReceiveError(this, CHIP_ERROR_INBOUND_MESSAGE_TOO_BIG, NULL);
        }
    }
}

CHIP_ERROR UDPEndPoint::GetPCB(IPAddressType addrType)
{
    // IMPORTANT: This method MUST be called with the LwIP stack LOCKED!

    // If a PCB hasn't been allocated yet...
    if (mUDP == NULL)
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
        pktInfo->SrcAddress  = IPAddress(*addr);
        pktInfo->DestAddress = IPAddress(*ip_current_dest_addr());
#else // LWIP_VERSION_MAJOR <= 1
        if (PCB_ISIPV6(pcb))
        {
            pktInfo->SrcAddress = IPAddress(*(ip6_addr_t *) addr);
            pktInfo->DestAddress = IPAddress(*ip6_current_dest_addr());
        }
#if INET_CONFIG_ENABLE_IPV4
        else
        {
            pktInfo->SrcAddress = IPAddress(*addr);
            pktInfo->DestAddress = IPAddress(*ip_current_dest_addr());
        }
#endif // INET_CONFIG_ENABLE_IPV4
#endif // LWIP_VERSION_MAJOR <= 1

        pktInfo->Interface = InterfaceId(ip_current_netif());
        pktInfo->SrcPort   = port;
        pktInfo->DestPort  = pcb->local_port;
    }

    const CHIP_ERROR error =
        lSystemLayer->PostEvent(*ep, kInetEvent_UDPDataReceived, (uintptr_t) System::LwIPPacketBufferView::UnsafeGetLwIPpbuf(buf));
    if (error == CHIP_NO_ERROR)
    {
        // If PostEvent() succeeded, it has ownership of the buffer, so we need to release it (without freeing it).
        static_cast<void>(std::move(buf).UnsafeRelease());
    }
}

CHIP_ERROR UDPEndPoint::SetMulticastLoopback(IPVersion aIPVersion, bool aLoopback)
{
    CHIP_ERROR lRetval = CHIP_ERROR_NOT_IMPLEMENTED;

#if !HAVE_LWIP_MULTICAST_LOOP
    lRetval = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#else
    if (aLoopback)
    {
        switch (mLwIPEndPointType)
        {

#if INET_CONFIG_ENABLE_UDP_ENDPOINT
        case LwIPEndPointType::UDP:
            udp_set_flags(mUDP, UDP_FLAGS_MULTICAST_LOOP);
            break;
#endif // INET_CONFIG_ENABLE_UDP_ENDPOINT

        default:
            lRetval = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
            break;
        }
    }
    else
    {
        switch (mLwIPEndPointType)
        {

#if INET_CONFIG_ENABLE_UDP_ENDPOINT
        case LwIPEndPointType::UDP:
            udp_clear_flags(mUDP, UDP_FLAGS_MULTICAST_LOOP);
            break;
#endif // INET_CONFIG_ENABLE_UDP_ENDPOINT

        default:
            lRetval = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
            break;
        }
    }

    lRetval = CHIP_NO_ERROR;
#endif // !HAVE_LWIP_MULTICAST_LOOP
    return (lRetval);
}

void UDPEndPoint::InitImpl() {}

#if INET_CONFIG_ENABLE_IPV4
#if LWIP_IPV4 && LWIP_IGMP
static CHIP_ERROR LwIPIPv4JoinLeaveMulticastGroup(InterfaceId aInterfaceId, const IPAddress & aAddress,
                                                  err_t (*aMethod)(struct netif *, const ip4_addr_t *))
{
    struct netif * const lNetif = UDPEndPoint::FindNetifFromInterfaceId(aInterfaceId);
    VerifyOrReturnError(lNetif != nullptr, INET_ERROR_UNKNOWN_INTERFACE);

    const ip4_addr_t lIPv4Address = aAddress.ToIPv4();
    const err_t lStatus           = aMethod(lNetif, &lIPv4Address);

    if (lStatus == ERR_MEM)
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    return chip::System::MapErrorLwIP(lStatus);
}
#endif // LWIP_IPV4 && LWIP_IGMP
#endif // INET_CONFIG_ENABLE_IPV4

#ifdef HAVE_IPV6_MULTICAST
static CHIP_ERROR LwIPIPv6JoinLeaveMulticastGroup(InterfaceId aInterfaceId, const IPAddress & aAddress,
                                                  err_t (*aMethod)(struct netif *, const ip6_addr_t *))
{
    struct netif * const lNetif = UDPEndPoint::FindNetifFromInterfaceId(aInterfaceId);
    VerifyOrReturnError(lNetif != nullptr, INET_ERROR_UNKNOWN_INTERFACE);

    const ip6_addr_t lIPv6Address = aAddress.ToIPv6();
    const err_t lStatus           = aMethod(lNetif, &lIPv6Address);

    if (lStatus == ERR_MEM)
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    return chip::System::MapErrorLwIP(lStatus);
}
#endif // LWIP_IPV6_MLD && LWIP_IPV6_ND && LWIP_IPV6

#if INET_CONFIG_ENABLE_IPV4
CHIP_ERROR UDPEndPoint::IPv4JoinLeaveMulticastGroupImpl(InterfaceId aInterfaceId, const IPAddress & aAddress, bool join)
{
#if LWIP_IPV4 && LWIP_IGMP
    const auto method = join ? igmp_joingroup_netif : igmp_leavegroup_netif;
    return LwIPIPv4JoinLeaveMulticastGroup(aInterfaceId, aAddress, method);
#else  // LWIP_IPV4 && LWIP_IGMP
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif // LWIP_IPV4 && LWIP_IGMP
}
#endif // INET_CONFIG_ENABLE_IPV4

CHIP_ERROR UDPEndPoint::IPv6JoinLeaveMulticastGroupImpl(InterfaceId aInterfaceId, const IPAddress & aAddress, bool join)
{
#ifdef HAVE_IPV6_MULTICAST
    const auto method = join ? mld6_joingroup_netif : mld6_leavegroup_netif;
    return LwIPIPv6JoinLeaveMulticastGroup(aInterfaceId, aAddress, method);
#else  // HAVE_IPV6_MULTICAST
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif // HAVE_IPV6_MULTICAST
}

struct netif * UDPEndPoint::FindNetifFromInterfaceId(InterfaceId aInterfaceId)
{
    struct netif * lRetval = NULL;

#if LWIP_VERSION_MAJOR >= 2 && LWIP_VERSION_MINOR >= 0 && defined(NETIF_FOREACH)
    NETIF_FOREACH(lRetval)
    {
        if (lRetval == aInterfaceId.GetPlatformInterface())
            break;
    }
#else  // LWIP_VERSION_MAJOR < 2 || !defined(NETIF_FOREACH)
    for (lRetval = netif_list; lRetval != NULL && lRetval != aInterfaceId.GetPlatformInterface(); lRetval = lRetval->next)
        ;
#endif // LWIP_VERSION_MAJOR >= 2 && LWIP_VERSION_MINOR >= 0 && defined(NETIF_FOREACH)

    return (lRetval);
}

IPPacketInfo * UDPEndPoint::GetPacketInfo(const System::PacketBufferHandle & aBuffer)
{
    uintptr_t lStart;
    uintptr_t lPacketInfoStart;
    IPPacketInfo * lPacketInfo = NULL;

    if (!aBuffer->EnsureReservedSize(sizeof(IPPacketInfo) + 3))
        goto done;

    lStart           = (uintptr_t) aBuffer->Start();
    lPacketInfoStart = lStart - sizeof(IPPacketInfo);

    // Align to a 4-byte boundary

    lPacketInfo = reinterpret_cast<IPPacketInfo *>(lPacketInfoStart & ~(sizeof(uint32_t) - 1));

done:
    return (lPacketInfo);
}

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS

#if CHIP_SYSTEM_CONFIG_USE_PLATFORM_MULTICAST_API
UDPEndPoint::MulticastGroupHandler UDPEndPoint::sJoinMulticastGroupHandler;
UDPEndPoint::MulticastGroupHandler UDPEndPoint::sLeaveMulticastGroupHandler;
#endif // CHIP_SYSTEM_CONFIG_USE_PLATFORM_MULTICAST_API

CHIP_ERROR UDPEndPoint::BindImpl(IPAddressType addrType, const IPAddress & addr, uint16_t port, InterfaceId intfId)
{
    // Make sure we have the appropriate type of socket.
    ReturnErrorOnFailure(GetSocket(addrType));
    ReturnErrorOnFailure(IpBind(addrType, addr, port, intfId));

    mBoundPort   = port;
    mBoundIntfId = intfId;

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

CHIP_ERROR UDPEndPoint::IpBind(IPAddressType aAddressType, const IPAddress & aAddress, uint16_t aPort, InterfaceId aInterfaceId)
{
    CHIP_ERROR lRetval = CHIP_NO_ERROR;

    if (aAddressType == IPAddressType::kIPv6)
    {
        struct sockaddr_in6 sa;

        memset(&sa, 0, sizeof(sa));

        sa.sin6_family                        = AF_INET6;
        sa.sin6_port                          = htons(aPort);
        sa.sin6_addr                          = aAddress.ToIPv6();
        InterfaceId::PlatformType interfaceId = aInterfaceId.GetPlatformInterface();
        if (!CanCastTo<decltype(sa.sin6_scope_id)>(interfaceId))
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }
        sa.sin6_scope_id = static_cast<decltype(sa.sin6_scope_id)>(interfaceId);

        if (bind(mSocket, reinterpret_cast<const sockaddr *>(&sa), static_cast<unsigned>(sizeof(sa))) != 0)
            lRetval = CHIP_ERROR_POSIX(errno);

            // Instruct the kernel that any messages to multicast destinations should be
            // sent down the interface specified by the caller.
#ifdef IPV6_MULTICAST_IF
        if (lRetval == CHIP_NO_ERROR)
            setsockopt(mSocket, IPPROTO_IPV6, IPV6_MULTICAST_IF, &aInterfaceId, sizeof(aInterfaceId));
#endif // defined(IPV6_MULTICAST_IF)

            // Instruct the kernel that any messages to multicast destinations should be
            // set with the configured hop limit value.
#ifdef IPV6_MULTICAST_HOPS
        int hops = INET_CONFIG_IP_MULTICAST_HOP_LIMIT;
        setsockopt(mSocket, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &hops, sizeof(hops));
#endif // defined(IPV6_MULTICAST_HOPS)
    }
#if INET_CONFIG_ENABLE_IPV4
    else if (aAddressType == IPAddressType::kIPv4)
    {
        struct sockaddr_in sa;
        int enable = 1;

        memset(&sa, 0, sizeof(sa));

        sa.sin_family = AF_INET;
        sa.sin_port   = htons(aPort);
        sa.sin_addr   = aAddress.ToIPv4();

        if (bind(mSocket, reinterpret_cast<const sockaddr *>(&sa), static_cast<unsigned>(sizeof(sa))) != 0)
            lRetval = CHIP_ERROR_POSIX(errno);

            // Instruct the kernel that any messages to multicast destinations should be
            // sent down the interface to which the specified IPv4 address is bound.
#ifdef IP_MULTICAST_IF
        if (lRetval == CHIP_NO_ERROR)
            setsockopt(mSocket, IPPROTO_IP, IP_MULTICAST_IF, &sa, sizeof(sa));
#endif // defined(IP_MULTICAST_IF)

            // Instruct the kernel that any messages to multicast destinations should be
            // set with the configured hop limit value.
#ifdef IP_MULTICAST_TTL
        int ttl = INET_CONFIG_IP_MULTICAST_HOP_LIMIT;
        setsockopt(mSocket, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));
#endif // defined(IP_MULTICAST_TTL)

        // Allow socket transmitting broadcast packets.
        if (lRetval == CHIP_NO_ERROR)
            setsockopt(mSocket, SOL_SOCKET, SO_BROADCAST, &enable, sizeof(enable));
    }
#endif // INET_CONFIG_ENABLE_IPV4
    else
        lRetval = INET_ERROR_WRONG_ADDRESS_TYPE;

    return (lRetval);
}

CHIP_ERROR UDPEndPoint::BindInterfaceImpl(IPAddressType addrType, InterfaceId intfId)
{
    // Make sure we have the appropriate type of socket.
    ReturnErrorOnFailure(GetSocket(addrType));
    return IpBindInterface(addrType, intfId);
}

CHIP_ERROR UDPEndPoint::IpBindInterface(IPAddressType aAddressType, InterfaceId aInterfaceId)
{
    CHIP_ERROR lRetval = CHIP_NO_ERROR;

#if HAVE_SO_BINDTODEVICE
    if (!aInterfaceId.IsPresent())
    {
        // Stop interface-based filtering.
        if (setsockopt(mSocket, SOL_SOCKET, SO_BINDTODEVICE, "", 0) == -1)
        {
            lRetval = CHIP_ERROR_POSIX(errno);
        }
    }
    else
    {
        // Start filtering on the passed interface.
        char lInterfaceName[IF_NAMESIZE];

        if (if_indextoname(aInterfaceId.GetPlatformInterface(), lInterfaceName) == NULL)
        {
            lRetval = CHIP_ERROR_POSIX(errno);
        }

        if (lRetval == CHIP_NO_ERROR &&
            setsockopt(mSocket, SOL_SOCKET, SO_BINDTODEVICE, lInterfaceName, socklen_t(strlen(lInterfaceName))) == -1)
        {
            lRetval = CHIP_ERROR_POSIX(errno);
        }
    }

    if (lRetval == CHIP_NO_ERROR)
        mBoundIntfId = aInterfaceId;

#else  // !HAVE_SO_BINDTODEVICE
    lRetval = CHIP_ERROR_NOT_IMPLEMENTED;
#endif // HAVE_SO_BINDTODEVICE

    return (lRetval);
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

    return IpSendMsg(pktInfo, std::move(msg));
}

CHIP_ERROR UDPEndPoint::IpSendMsg(const IPPacketInfo * aPktInfo, chip::System::PacketBufferHandle && aBuffer)
{
    // Ensure the destination address type is compatible with the endpoint address type.
    VerifyOrReturnError(mAddrType == aPktInfo->DestAddress.Type(), CHIP_ERROR_INVALID_ARGUMENT);

    // For now the entire message must fit within a single buffer.
    VerifyOrReturnError(!aBuffer->HasChainedBuffer(), CHIP_ERROR_MESSAGE_TOO_LONG);

    struct iovec msgIOV;
    msgIOV.iov_base = aBuffer->Start();
    msgIOV.iov_len  = aBuffer->DataLength();

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
        intf = mBoundIntfId;

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

            struct in_pktinfo * pktInfo = reinterpret_cast<struct in_pktinfo *> CMSG_DATA(controlHdr);
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

            struct in6_pktinfo * pktInfo = reinterpret_cast<struct in6_pktinfo *> CMSG_DATA(controlHdr);
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
        return CHIP_ERROR_POSIX(errno);
    if (lenSent != aBuffer->DataLength())
        return CHIP_ERROR_OUTBOUND_MESSAGE_TOO_BIG;
    return CHIP_NO_ERROR;
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
    constexpr int lType     = (SOCK_DGRAM | SOCK_CLOEXEC);
    constexpr int lProtocol = 0;

    return IpGetSocket(aAddressType, lType, lProtocol);
}

CHIP_ERROR UDPEndPoint::IpGetSocket(IPAddressType aAddressType, int aType, int aProtocol)
{
    if (mSocket == kInvalidSocketFd)
    {
        const int one = 1;
        int family;

        switch (aAddressType)
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

        mSocket = ::socket(family, aType, aProtocol);
        if (mSocket == -1)
            return CHIP_ERROR_POSIX(errno);
        ReturnErrorOnFailure(static_cast<System::LayerSockets *>(Layer().SystemLayer())->StartWatchingSocket(mSocket, &mWatch));

        mAddrType = aAddressType;

        // NOTE WELL: the errors returned by setsockopt() here are not
        // returned as Inet layer CHIP_ERROR_POSIX(errno)
        // codes because they are normally expected to fail on some
        // platforms where the socket option code is defined in the
        // header files but not [yet] implemented. Certainly, there is
        // room to improve this by connecting the build configuration
        // logic up to check for implementations of these options and
        // to provide appropriate HAVE_xxxxx definitions accordingly.

        int res = setsockopt(mSocket, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
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
        if (aAddressType == IPAddressType::kIPv6)
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
        if (aAddressType == IPAddressType::kIPv4)
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
        if (aAddressType == IPAddressType::kIPv6)
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
    else if (mAddrType != aAddressType)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    return CHIP_NO_ERROR;
}

// static
void UDPEndPoint::HandlePendingIO(System::SocketEvents events, intptr_t data)
{
    reinterpret_cast<UDPEndPoint *>(data)->HandlePendingIO(events);
}

void UDPEndPoint::HandlePendingIO(System::SocketEvents events)
{
    if (mState == State::kListening && OnMessageReceived != nullptr && events.Has(System::SocketEventFlags::kRead))
    {
        const uint16_t lPort = mBoundPort;

        IpHandlePendingIO(lPort);
    }
}

void UDPEndPoint::IpHandlePendingIO(uint16_t aPort)
{
    CHIP_ERROR lStatus = CHIP_NO_ERROR;
    IPPacketInfo lPacketInfo;
    System::PacketBufferHandle lBuffer;

    lPacketInfo.Clear();
    lPacketInfo.DestPort = aPort;

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
                    struct in_pktinfo * inPktInfo = reinterpret_cast<struct in_pktinfo *> CMSG_DATA(controlHdr);
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
                    struct in6_pktinfo * in6PktInfo = reinterpret_cast<struct in6_pktinfo *> CMSG_DATA(controlHdr);
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
    const unsigned int lValue = aLoopback;
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

CHIP_ERROR UDPEndPoint::SetMulticastLoopback(IPVersion aIPVersion, bool aLoopback)
{
    CHIP_ERROR lRetval = CHIP_ERROR_NOT_IMPLEMENTED;

    lRetval = SocketsSetMulticastLoopback(mSocket, aIPVersion, aLoopback);
    SuccessOrExit(lRetval);

exit:
    return (lRetval);
}

void UDPEndPoint::InitImpl()
{
    mBoundIntfId = InterfaceId::Null();
}

#if INET_CONFIG_ENABLE_IPV4

static CHIP_ERROR SocketsIPv4JoinLeaveMulticastGroup(int aSocket, InterfaceId aInterfaceId, const IPAddress & aAddress,
                                                     int aCommand)
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

    if (setsockopt(aSocket, IPPROTO_IP, aCommand, &lMulticastRequest, sizeof(lMulticastRequest)) != 0)
    {
        return CHIP_ERROR_POSIX(errno);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR UDPEndPoint::IPv4JoinLeaveMulticastGroupImpl(InterfaceId aInterfaceId, const IPAddress & aAddress, bool join)
{
    return SocketsIPv4JoinLeaveMulticastGroup(mSocket, aInterfaceId, aAddress, join ? IP_ADD_MEMBERSHIP : IP_DROP_MEMBERSHIP);
}

#endif // INET_CONFIG_ENABLE_IPV4

#if INET_IPV6_ADD_MEMBERSHIP || INET_IPV6_DROP_MEMBERSHIP
static CHIP_ERROR SocketsIPv6JoinLeaveMulticastGroup(int aSocket, InterfaceId aInterfaceId, const IPAddress & aAddress,
                                                     int aCommand)
{
    const InterfaceId::PlatformType lIfIndex = aInterfaceId.GetPlatformInterface();

    struct ipv6_mreq lMulticastRequest;
    memset(&lMulticastRequest, 0, sizeof(lMulticastRequest));
    VerifyOrReturnError(CanCastTo<decltype(lMulticastRequest.ipv6mr_interface)>(lIfIndex), CHIP_ERROR_UNEXPECTED_EVENT);

    lMulticastRequest.ipv6mr_interface = static_cast<decltype(lMulticastRequest.ipv6mr_interface)>(lIfIndex);
    lMulticastRequest.ipv6mr_multiaddr = aAddress.ToIPv6();

    if (setsockopt(aSocket, IPPROTO_IPV6, aCommand, &lMulticastRequest, sizeof(lMulticastRequest)) != 0)
    {
        return CHIP_ERROR_POSIX(errno);
    }
    return CHIP_NO_ERROR;
}
#endif // INET_IPV6_ADD_MEMBERSHIP || INET_IPV6_DROP_MEMBERSHIP

CHIP_ERROR UDPEndPoint::IPv6JoinLeaveMulticastGroupImpl(InterfaceId aInterfaceId, const IPAddress & aAddress, bool join)
{
#if CHIP_SYSTEM_CONFIG_USE_PLATFORM_MULTICAST_API
    MulticastGroupHandler handler = join ? sJoinMulticastGroupHandler : sLeaveMulticastGroupHandler;
    if (handler != nullptr)
    {
        return handler(aInterfaceId, aAddress);
    }
#endif // CHIP_SYSTEM_CONFIG_USE_PLATFORM_MULTICAST_API
#if defined(INET_IPV6_ADD_MEMBERSHIP) && defined(INET_IPV6_DROP_MEMBERSHIP)
    return SocketsIPv6JoinLeaveMulticastGroup(mSocket, aInterfaceId, aAddress,
                                              join ? INET_IPV6_ADD_MEMBERSHIP : INET_IPV6_DROP_MEMBERSHIP);
#else  // defined(INET_IPV6_ADD_MEMBERSHIP) && defined(INET_IPV6_DROP_MEMBERSHIP)
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif // defined(INET_IPV6_ADD_MEMBERSHIP) && defined(INET_IPV6_DROP_MEMBERSHIP)
}

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

#if CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK

CHIP_ERROR UDPEndPoint::BindImpl(IPAddressType addrType, const IPAddress & addr, uint16_t port, InterfaceId intfId)
{
    nw_parameters_configure_protocol_block_t configure_tls;
    nw_parameters_t parameters;

    if (intfId.IsPresent())
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    configure_tls = NW_PARAMETERS_DISABLE_PROTOCOL;
    parameters    = nw_parameters_create_secure_udp(configure_tls, NW_PARAMETERS_DEFAULT_CONFIGURATION);

    ReturnErrorOnFailure(IpBind(addrType, addr, port, parameters));

    mParameters = parameters;
    return CHIP_NO_ERROR;
}

CHIP_ERROR UDPEndPoint::IpBind(IPAddressType aAddressType, const IPAddress & aAddress, uint16_t aPort,
                                 const nw_parameters_t & aParameters)
{
    nw_endpoint_t endpoint = nullptr;

    VerifyOrReturnError(aParameters != NULL, CHIP_ERROR_INVALID_ARGUMENT);

    ReturnErrorOnFailure(ConfigureProtocol(aAddressType, aParameters));

    CHIP_ERROR res = GetEndPoint(endpoint, aAddressType, aAddress, aPort);
    nw_parameters_set_local_endpoint(aParameters, endpoint);
    nw_release(endpoint);
    ReturnErrorOnFailure(res);

    mDispatchQueue = dispatch_queue_create("inet_dispatch_global", DISPATCH_QUEUE_CONCURRENT);
    VerifyOrReturnError(mDispatchQueue != NULL, CHIP_ERROR_NO_MEMORY);
    dispatch_retain(mDispatchQueue);

    mConnectionSemaphore = dispatch_semaphore_create(0);
    VerifyOrReturnError(mConnectionSemaphore != NULL, CHIP_ERROR_NO_MEMORY);
    dispatch_retain(mConnectionSemaphore);

    mSendSemaphore = dispatch_semaphore_create(0);
    VerifyOrReturnError(mSendSemaphore != NULL, CHIP_ERROR_NO_MEMORY);
    dispatch_retain(mSendSemaphore);

    mAddrType   = aAddressType;
    mConnection = NULL;

    return CHIP_NO_ERROR;
}

CHIP_ERROR UDPEndPoint::BindInterfaceImpl(IPAddressType addrType, InterfaceId intfId)
{
    return INET_ERROR_UNKNOWN_INTERFACE;
}

InterfaceId UDPEndPoint::GetBoundInterface()
{
    return InterfaceId::Null();
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
    return IpSendMsg(pktInfo, std::move(msg));
}

CHIP_ERROR UDPEndPoint::IpSendMsg(const IPPacketInfo * aPktInfo, chip::System::PacketBufferHandle && aBuffer)
{
    dispatch_data_t content;

    // Ensure the destination address type is compatible with the endpoint address type.
    VerifyOrReturnError(mAddrType == aPktInfo->DestAddress.Type(), CHIP_ERROR_INVALID_ARGUMENT);

    // For now the entire message must fit within a single buffer.
    VerifyOrReturnError(aBuffer->Next() == NULL, CHIP_ERROR_MESSAGE_TOO_LONG);

    ReturnErrorOnFailure(GetConnection(aPktInfo));

    // Send a message, and wait for it to be dispatched.
    content = dispatch_data_create(aBuffer->Start(), aBuffer->DataLength(), mDispatchQueue, DISPATCH_DATA_DESTRUCTOR_DEFAULT);

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

void UDPEndPoint::CloseImpl()
{
    ReleaseAll();
}

void UDPEndPoint::ReleaseAll()
{

    OnMessageReceived = NULL;
    OnReceiveError    = NULL;

    ReleaseConnection();
    ReleaseListener();

    if (mParameters)
    {
        nw_release(mParameters);
        mParameters = NULL;
    }

    if (mDispatchQueue)
    {
        dispatch_suspend(mDispatchQueue);
        dispatch_release(mDispatchQueue);
        mDispatchQueue = NULL;
    }

    if (mConnectionSemaphore)
    {
        dispatch_release(mConnectionSemaphore);
        mConnectionSemaphore = NULL;
    }

    if (mListenerQueue)
    {
        dispatch_suspend(mListenerQueue);
        dispatch_release(mListenerQueue);
        mListenerQueue = NULL;
    }

    if (mListenerSemaphore)
    {
        dispatch_release(mListenerSemaphore);
        mListenerSemaphore = NULL;
    }

    if (mSendSemaphore)
    {
        dispatch_release(mSendSemaphore);
        mSendSemaphore = NULL;
    }
}

void UDPEndPoint::Free()
{
    Close();
    Release();
}

CHIP_ERROR UDPEndPoint::SetMulticastLoopback(IPVersion aIPVersion, bool aLoopback)
{
    CHIP_ERROR lRetval = CHIP_ERROR_NOT_IMPLEMENTED;
    return (lRetval);
}

#if INET_CONFIG_ENABLE_IPV4
CHIP_ERROR UDPEndPoint::IPv4JoinLeaveMulticastGroupImpl(InterfaceId aInterfaceId, const IPAddress & aAddress, bool join)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}
#endif // INET_CONFIG_ENABLE_IPV4

CHIP_ERROR UDPEndPoint::IPv6JoinLeaveMulticastGroupImpl(InterfaceId aInterfaceId, const IPAddress & aAddress, bool join)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR UDPEndPoint::ConfigureProtocol(IPAddressType aAddressType, const nw_parameters_t & aParameters)
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

void UDPEndPoint::GetPacketInfo(const nw_connection_t & aConnection, IPPacketInfo & aPacketInfo)
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

CHIP_ERROR UDPEndPoint::GetEndPoint(nw_endpoint_t & aEndPoint, const IPAddressType aAddressType, const IPAddress & aAddress,
                                        uint16_t aPort)
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
    VerifyOrReturnError(aEndPoint != NULL, CHIP_ERROR_INVALID_ARGUMENT);

    return CHIP_NO_ERROR;
}

CHIP_ERROR UDPEndPoint::GetConnection(const IPPacketInfo * aPktInfo)
{
    VerifyOrReturnError(mParameters != NULL, CHIP_ERROR_INCORRECT_STATE);

    nw_endpoint_t endpoint     = NULL;
    nw_connection_t connection = NULL;

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

    VerifyOrReturnError(connection != NULL, CHIP_ERROR_INCORRECT_STATE);

    return StartConnection(connection);
}

CHIP_ERROR UDPEndPoint::StartListener()
{
    __block CHIP_ERROR res = CHIP_NO_ERROR;
    nw_listener_t listener;

    VerifyOrReturnError(mListener == NULL, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mListenerSemaphore == NULL, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mListenerQueue == NULL, CHIP_ERROR_INCORRECT_STATE);

    listener = nw_listener_create(mParameters);
    VerifyOrReturnError(listener != NULL, CHIP_ERROR_INCORRECT_STATE);

    mListenerSemaphore = dispatch_semaphore_create(0);
    VerifyOrReturnError(mListenerSemaphore != NULL, CHIP_ERROR_NO_MEMORY);
    dispatch_retain(mListenerSemaphore);

    mListenerQueue = dispatch_queue_create("inet_dispatch_listener", DISPATCH_QUEUE_CONCURRENT);
    VerifyOrReturnError(mListenerQueue != NULL, CHIP_ERROR_NO_MEMORY);
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
                res = CHIP_ERROR_CONNECTION_ABORTED;

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

CHIP_ERROR UDPEndPoint::StartConnection(nw_connection_t & aConnection)
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
                res = CHIP_ERROR_CONNECTION_ABORTED;

            dispatch_semaphore_signal(mConnectionSemaphore);
            break;
        }
    });

    nw_connection_start(aConnection);
    dispatch_semaphore_wait(mConnectionSemaphore, DISPATCH_TIME_FOREVER);
    SuccessOrExit(res);

    mConnection = aConnection;
    nw_retain(mConnection);
    IpHandleDataReceived(mConnection);

    return res;
}

void UDPEndPoint::IpHandleDataReceived(const nw_connection_t & aConnection)
{
    nw_connection_receive_completion_t handler =
        ^(dispatch_data_t content, nw_content_context_t context, bool is_complete, nw_error_t receive_error) {
            dispatch_block_t schedule_next_receive = ^{
                if (receive_error == NULL)
                {
                    IpHandleDataReceived(aConnection);
                }
                else if (OnReceiveError != NULL)
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

            if (content != NULL && OnMessageReceived != NULL)
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

CHIP_ERROR UDPEndPoint::ReleaseListener()
{
    VerifyOrReturnError(mListener, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mDispatchQueue, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mConnectionSemaphore, CHIP_ERROR_INCORRECT_STATE);

    nw_listener_cancel(mListener);
    dispatch_semaphore_wait(mListenerSemaphore, DISPATCH_TIME_FOREVER);
    nw_release(mListener);
    mListener = NULL;

    return CHIP_NO_ERROR;
}

CHIP_ERROR UDPEndPoint::ReleaseConnection()
{
    VerifyOrReturnError(mConnection, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mDispatchQueue, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mConnectionSemaphore, CHIP_ERROR_INCORRECT_STATE);

    nw_connection_cancel(mConnection);
    dispatch_semaphore_wait(mConnectionSemaphore, DISPATCH_TIME_FOREVER);
    nw_release(mConnection);
    mConnection = NULL;

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
    AppState          = appState;

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

void UDPEndPoint::Init(InetLayer * inetLayer)
{
    IpInit(inetLayer);
}

void UDPEndPoint::IpInit(InetLayer * aInetLayer)
{
    InitEndPointBasis(*aInetLayer);
    InitImpl();
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
