/*
 *
 * SPDX-FileCopyrightText: 2020-2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2018 Google LLC
 * SPDX-FileCopyrightText: 2013-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * This file declares an implementation of Inet::UDPEndPoint using LwIP.
 */

#pragma once

#include <inet/EndPointStateLwIP.h>
#include <inet/EndpointQueueFilter.h>
#include <inet/UDPEndPoint.h>

namespace chip {
namespace Inet {

class UDPEndPointImplLwIP : public UDPEndPoint, public EndPointStateLwIP
{
public:
    UDPEndPointImplLwIP(EndPointManager<UDPEndPoint> & endPointManager) : UDPEndPoint(endPointManager), mUDP(nullptr) {}

    // UDPEndPoint overrides.
    CHIP_ERROR SetMulticastLoopback(IPVersion aIPVersion, bool aLoopback) override;
    InterfaceId GetBoundInterface() const override;
    uint16_t GetBoundPort() const override;
    void Free() override;

    /**
     * @brief Set the queue filter for all UDP endpoints
     *
     * Responsibility is on the caller to avoid changing the filter while packets are being
     * processed. Setting the queue filter to `nullptr` disables the filtering.
     *
     * NOTE: There is only one EndpointQueueFilter instance settable. However it's possible
     *       to create an instance of EndpointQueueFilter that combines several other
     *       EndpointQueueFilter by composition to achieve the effect of multiple filters.
     *
     * @param queueFilter - queue filter instance to set, owned by caller
     */
    static void SetQueueFilter(EndpointQueueFilter * queueFilter) { sQueueFilter = queueFilter; }

private:
    // UDPEndPoint overrides.
#if INET_CONFIG_ENABLE_IPV4
    CHIP_ERROR IPv4JoinLeaveMulticastGroupImpl(InterfaceId aInterfaceId, const IPAddress & aAddress, bool join) override;
#endif // INET_CONFIG_ENABLE_IPV4
    CHIP_ERROR IPv6JoinLeaveMulticastGroupImpl(InterfaceId aInterfaceId, const IPAddress & aAddress, bool join) override;
    CHIP_ERROR BindImpl(IPAddressType addressType, const IPAddress & address, uint16_t port, InterfaceId interfaceId) override;
    CHIP_ERROR BindInterfaceImpl(IPAddressType addressType, InterfaceId interfaceId) override;
    CHIP_ERROR ListenImpl() override;
    CHIP_ERROR SendMsgImpl(const IPPacketInfo * pktInfo, chip::System::PacketBufferHandle && msg) override;
    void CloseImpl() override;

    static struct netif * FindNetifFromInterfaceId(InterfaceId aInterfaceId);
    static CHIP_ERROR LwIPBindInterface(struct udp_pcb * aUDP, InterfaceId intfId);

    void HandleDataReceived(System::PacketBufferHandle && msg, IPPacketInfo * pktInfo);

    CHIP_ERROR GetPCB(IPAddressType addrType4);
    static void LwIPReceiveUDPMessage(void * arg, struct udp_pcb * pcb, struct pbuf * p, const ip_addr_t * addr, u16_t port);

    udp_pcb * mUDP; // LwIP User datagram protocol (UDP) control block.
    std::atomic_int mDelayReleaseCount{ 0 };

    static EndpointQueueFilter * sQueueFilter;
};

using UDPEndPointImpl = UDPEndPointImplLwIP;

} // namespace Inet
} // namespace chip
