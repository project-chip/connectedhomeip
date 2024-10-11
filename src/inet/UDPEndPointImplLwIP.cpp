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
 * This file implements Inet::UDPEndPoint using LwIP.
 */

#include <inet/UDPEndPointImplLwIP.h>

#if INET_CONFIG_ENABLE_IPV4
#include <lwip/igmp.h>
#endif // INET_CONFIG_ENABLE_IPV4

#include <lwip/init.h>
#include <lwip/ip.h>
#include <lwip/mld6.h>
#include <lwip/netif.h>
#include <lwip/raw.h>
#include <lwip/udp.h>

static_assert(LWIP_VERSION_MAJOR > 1, "CHIP requires LwIP 2.0 or later");

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

namespace chip {
namespace Inet {

EndpointQueueFilter * UDPEndPointImplLwIP::sQueueFilter = nullptr;

CHIP_ERROR UDPEndPointImplLwIP::BindImpl(IPAddressType addressType, const IPAddress & address, uint16_t port,
                                         InterfaceId interfaceId)
{
    // Make sure we have the appropriate type of PCB.
    CHIP_ERROR res = GetPCB(addressType);

    // Bind the PCB to the specified address/port.
    ip_addr_t ipAddr;
    if (res == CHIP_NO_ERROR)
    {
        res = address.ToLwIPAddr(addressType, ipAddr);
    }

    if (res == CHIP_NO_ERROR)
    {
        res = chip::System::MapErrorLwIP(RunOnTCPIPRet([this, &ipAddr, port]() { return udp_bind(mUDP, &ipAddr, port); }));
    }

    if (res == CHIP_NO_ERROR)
    {
        res = LwIPBindInterface(mUDP, interfaceId);
    }

    return res;
}

CHIP_ERROR UDPEndPointImplLwIP::BindInterfaceImpl(IPAddressType addrType, InterfaceId intfId)
{
    // NOTE: this only supports LwIP interfaces whose number is no bigger than 9.

    // Make sure we have the appropriate type of PCB.
    CHIP_ERROR err = GetPCB(addrType);

    if (err == CHIP_NO_ERROR)
    {
        err = LwIPBindInterface(mUDP, intfId);
    }
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

    RunOnTCPIP([aUDP, netifp]() { udp_bind_netif(aUDP, netifp); });
    return CHIP_NO_ERROR;
}

InterfaceId UDPEndPointImplLwIP::GetBoundInterface() const
{
    struct netif * netif;
    RunOnTCPIP([this, &netif]() { netif = netif_get_by_index(mUDP->netif_idx); });
    return InterfaceId(netif);
}

uint16_t UDPEndPointImplLwIP::GetBoundPort() const
{
    return mUDP->local_port;
}

CHIP_ERROR UDPEndPointImplLwIP::ListenImpl()
{
    RunOnTCPIP([this]() { udp_recv(mUDP, LwIPReceiveUDPMessage, this); });
    return CHIP_NO_ERROR;
}

CHIP_ERROR UDPEndPointImplLwIP::SendMsgImpl(const IPPacketInfo * pktInfo, System::PacketBufferHandle && msg)
{
    assertChipStackLockedByCurrentThread();

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

    CHIP_ERROR res = CHIP_NO_ERROR;
    err_t lwipErr  = ERR_VAL;

    // Make sure we have the appropriate type of PCB based on the destination address.
    res = GetPCB(destAddr.Type());
    if (res != CHIP_NO_ERROR)
    {
        return res;
    }

    // Send the message to the specified address/port.
    // If an outbound interface has been specified, call a specific version of the UDP sendto()
    // function that accepts the target interface.
    // If a source address has been specified, temporarily override the local_ip of the PCB.
    // This results in LwIP using the given address being as the source address for the generated
    // packet, as if the PCB had been bound to that address.
    const IPAddress & srcAddr  = pktInfo->SrcAddress;
    const uint16_t & destPort  = pktInfo->DestPort;
    const InterfaceId & intfId = pktInfo->Interface;

    ip_addr_t lwipSrcAddr  = srcAddr.ToLwIPAddr();
    ip_addr_t lwipDestAddr = destAddr.ToLwIPAddr();

    ip_addr_t boundAddr;
    ip_addr_copy(boundAddr, mUDP->local_ip);

    if (!ip_addr_isany(&lwipSrcAddr))
    {
        ip_addr_copy(mUDP->local_ip, lwipSrcAddr);
    }

    lwipErr = RunOnTCPIPRet([this, &intfId, &msg, &lwipDestAddr, destPort]() {
        if (intfId.IsPresent())
        {
            return udp_sendto_if(mUDP, System::LwIPPacketBufferView::UnsafeGetLwIPpbuf(msg), &lwipDestAddr, destPort,
                                 intfId.GetPlatformInterface());
        }
        return udp_sendto(mUDP, System::LwIPPacketBufferView::UnsafeGetLwIPpbuf(msg), &lwipDestAddr, destPort);
    });

    ip_addr_copy(mUDP->local_ip, boundAddr);

    if (lwipErr != ERR_OK)
    {
        res = chip::System::MapErrorLwIP(lwipErr);
    }

    return res;
}

void UDPEndPointImplLwIP::CloseImpl()
{
    assertChipStackLockedByCurrentThread();

    // Since UDP PCB is released synchronously here, but UDP endpoint itself might have to wait
    // for destruction asynchronously, there could be more allocated UDP endpoints than UDP PCBs.
    if (mUDP == nullptr)
    {
        return;
    }
    RunOnTCPIP([this]() { udp_remove(mUDP); });
    mUDP              = nullptr;
    mLwIPEndPointType = LwIPEndPointType::Unknown;

    // If there is a UDPEndPointImplLwIP::LwIPReceiveUDPMessage
    // event pending in the event queue (SystemLayer::ScheduleLambda), we
    // schedule a release call to the end of the queue, to ensure that the
    // queued pointer to UDPEndPointImplLwIP is not dangling.
    if (mDelayReleaseCount != 0)
    {
        Retain();
        CHIP_ERROR err = GetSystemLayer().ScheduleLambda([this] { Release(); });
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Inet, "Unable to schedule lambda: %" CHIP_ERROR_FORMAT, err.Format());
            // There is nothing we can do here, accept the chance of racing
            Release();
        }
    }
}

void UDPEndPointImplLwIP::Free()
{
    Close();
    Release();
}

void UDPEndPointImplLwIP::HandleDataReceived(System::PacketBufferHandle && msg, IPPacketInfo * pktInfo)
{
    // Process packet filter if needed. May cause packet to get dropped before processing.
    bool dropPacket = false;
    if ((pktInfo != nullptr) && (sQueueFilter != nullptr))
    {
        auto outcome = sQueueFilter->FilterAfterDequeue(this, *pktInfo, msg);
        dropPacket   = (outcome == EndpointQueueFilter::FilterOutcome::kDropPacket);
    }

    // Process actual packet if allowed
    if ((mState == State::kListening) && (OnMessageReceived != nullptr) && !dropPacket)
    {
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
    Platform::Delete(pktInfo);
}

CHIP_ERROR UDPEndPointImplLwIP::GetPCB(IPAddressType addrType)
{
    assertChipStackLockedByCurrentThread();

    // If a PCB hasn't been allocated yet...
    if (mUDP == nullptr)
    {
        // Allocate a PCB of the appropriate type.
        if (addrType == IPAddressType::kIPv6)
        {
            RunOnTCPIP([this]() { mUDP = udp_new_ip_type(IPADDR_TYPE_V6); });
        }
#if INET_CONFIG_ENABLE_IPV4
        else if (addrType == IPAddressType::kIPv4)
        {
            RunOnTCPIP([this]() { mUDP = udp_new_ip_type(IPADDR_TYPE_V4); });
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

        // Fail if the existing PCB is not the correct type.
        VerifyOrReturnError(addrType == pcbAddrType, INET_ERROR_WRONG_ADDRESS_TYPE);
    }

    return CHIP_NO_ERROR;
}

void UDPEndPointImplLwIP::LwIPReceiveUDPMessage(void * arg, struct udp_pcb * pcb, struct pbuf * p, const ip_addr_t * addr,
                                                u16_t port)
{
    UDPEndPointImplLwIP * ep = static_cast<UDPEndPointImplLwIP *>(arg);
    if (ep->mState == State::kClosed)
    {
        return;
    }

    auto pktInfo = Platform::MakeUnique<IPPacketInfo>();
    if (pktInfo.get() == nullptr)
    {
        ChipLogError(Inet, "Cannot allocate packet info");
        return;
    }

    System::PacketBufferHandle buf = System::PacketBufferHandle::Adopt(p);
    if (buf->HasChainedBuffer())
    {
        buf->CompactHead();
    }
    if (buf->HasChainedBuffer())
    {
        // Have to allocate a new big-enough buffer and copy.
        size_t messageSize              = buf->TotalLength();
        System::PacketBufferHandle copy = System::PacketBufferHandle::New(messageSize, 0);
        if (copy.IsNull() || buf->Read(copy->Start(), messageSize) != CHIP_NO_ERROR)
        {
            ChipLogError(Inet, "No memory to flatten incoming packet buffer chain of size %u", buf->TotalLength());
            return;
        }
        buf = std::move(copy);
    }

    pktInfo->SrcAddress  = IPAddress(*addr);
    pktInfo->DestAddress = IPAddress(*ip_current_dest_addr());
    pktInfo->Interface   = InterfaceId(ip_current_netif());
    pktInfo->SrcPort     = port;
    pktInfo->DestPort    = pcb->local_port;

    auto filterOutcome = EndpointQueueFilter::FilterOutcome::kAllowPacket;
    if (sQueueFilter != nullptr)
    {
        filterOutcome = sQueueFilter->FilterBeforeEnqueue(ep, *(pktInfo.get()), buf);
    }

    if (filterOutcome != EndpointQueueFilter::FilterOutcome::kAllowPacket)
    {
        // Logging, if any, should be at the choice of the filter impl at time of filtering.
        return;
    }

    // Increase mDelayReleaseCount to delay release of this UDP EndPoint while the HandleDataReceived call is
    // pending on it.
    ep->mDelayReleaseCount++;

    CHIP_ERROR err = ep->GetSystemLayer().ScheduleLambda(
        [ep, p = System::LwIPPacketBufferView::UnsafeGetLwIPpbuf(buf), pktInfo = pktInfo.get()] {
            ep->mDelayReleaseCount--;

            auto handle = System::PacketBufferHandle::Adopt(p);
            ep->HandleDataReceived(std::move(handle), pktInfo);
        });

    if (err == CHIP_NO_ERROR)
    {
        // If ScheduleLambda() succeeded, it has ownership of the buffer, so we need to release it (without freeing it).
        static_cast<void>(std::move(buf).UnsafeRelease());
        // Similarly, ScheduleLambda now has ownership of pktInfo.
        pktInfo.release();
    }
    else
    {
        // On failure to enqueue the processing, we have to tell the filter that
        // the packet is basically dequeued, if it tries to keep track of the lifecycle.
        if (sQueueFilter != nullptr)
        {
            (void) sQueueFilter->FilterAfterDequeue(ep, *(pktInfo.get()), buf);
            ChipLogError(Inet, "Dequeue ERROR err = %" CHIP_ERROR_FORMAT, err.Format());
        }

        ep->mDelayReleaseCount--;
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
    const ip4_addr_t lIPv4Address = aAddress.ToIPv4();
    struct netif * lNetif         = nullptr;
    if (aInterfaceId.IsPresent())
    {
        lNetif = FindNetifFromInterfaceId(aInterfaceId);
        VerifyOrReturnError(lNetif != nullptr, INET_ERROR_UNKNOWN_INTERFACE);
    }

    err_t lStatus = RunOnTCPIPRet([lNetif, &lIPv4Address, join]() {
        if (lNetif != nullptr)
        {
            return join ? igmp_joingroup_netif(lNetif, &lIPv4Address) //
                        : igmp_leavegroup_netif(lNetif, &lIPv4Address);
        }
        return join ? igmp_joingroup(IP4_ADDR_ANY4, &lIPv4Address) //
                    : igmp_leavegroup(IP4_ADDR_ANY4, &lIPv4Address);
    });

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
    const ip6_addr_t lIPv6Address = aAddress.ToIPv6();
    struct netif * lNetif         = nullptr;
    if (aInterfaceId.IsPresent())
    {
        lNetif = FindNetifFromInterfaceId(aInterfaceId);
        VerifyOrReturnError(lNetif != nullptr, INET_ERROR_UNKNOWN_INTERFACE);
    }

    err_t lStatus = RunOnTCPIPRet([lNetif, &lIPv6Address, join]() {
        if (lNetif != nullptr)
        {
            return join ? mld6_joingroup_netif(lNetif, &lIPv6Address) //
                        : mld6_leavegroup_netif(lNetif, &lIPv6Address);
        }
        return join ? mld6_joingroup(IP6_ADDR_ANY6, &lIPv6Address) //
                    : mld6_leavegroup(IP6_ADDR_ANY6, &lIPv6Address);
    });

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

    RunOnTCPIP([aInterfaceId, &lRetval]() {
#if defined(NETIF_FOREACH)
        NETIF_FOREACH(lRetval)
        {
            if (lRetval == aInterfaceId.GetPlatformInterface())
            {
                break;
            }
        }
#else  // defined(NETIF_FOREACH)
        for (lRetval = netif_list; lRetval != nullptr && lRetval != aInterfaceId.GetPlatformInterface(); lRetval = lRetval->next)
            ;
#endif // defined(NETIF_FOREACH)
    });

    return (lRetval);
}

} // namespace Inet
} // namespace chip
