/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2018 Google LLC
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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
 * This file declares an implementation of Inet::UDPEndPoint using LwIP.
 */

#pragma once

#include <inet/EndPointStateLwIP.h>
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

    void HandleDataReceived(chip::System::PacketBufferHandle && aBuffer);

    /**
     *  Get LwIP IP layer source and destination addressing information.
     *
     *  @param[in]   aBuffer    The packet buffer containing the IP message.
     *
     *  @returns  a pointer to the address information on success; otherwise,
     *            nullptr if there is insufficient space in the packet for
     *            the address information.
     *
     *  When using LwIP information about the packet is 'hidden' in the reserved space before the start of the
     *  data in the packet buffer. This is necessary because the system layer events only have two arguments,
     *  which in this case are used to convey the pointer to the end point and the pointer to the buffer.
     *
     *  In most cases this trick of storing information before the data works because the first buffer in an
     *  LwIP IP message contains the space that was used for the Ethernet/IP/UDP headers. However, given the
     *  current size of the IPPacketInfo structure (40 bytes), it is possible for there to not be enough room
     *  to store the structure along with the payload in a single packet buffer. In practice, this should only
     *  happen for extremely large IPv4 packets that arrive without an Ethernet header.
     */
    static IPPacketInfo * GetPacketInfo(const chip::System::PacketBufferHandle & aBuffer);

    CHIP_ERROR GetPCB(IPAddressType addrType4);
    static void LwIPReceiveUDPMessage(void * arg, struct udp_pcb * pcb, struct pbuf * p, const ip_addr_t * addr, u16_t port);

    udp_pcb * mUDP; // LwIP User datagram protocol (UDP) control block.
};

using UDPEndPointImpl = UDPEndPointImplLwIP;

} // namespace Inet
} // namespace chip
