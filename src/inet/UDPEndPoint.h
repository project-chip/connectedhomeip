/*
 *
 *    Copyright (c) 2018 Google LLC
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
 *    All rights reserved.
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
 *      This header file defines the <tt>nl::Inet::UDPEndPoint</tt>
 *      class, where the Nest Inet Layer encapsulates methods for
 *      interacting with UDP transport endpoints (SOCK_DGRAM sockets
 *      on Linux and BSD-derived systems) or LwIP UDP protocol
 *      control blocks, as the system is configured accordingly.
 */

#ifndef UDPENDPOINT_H
#define UDPENDPOINT_H

#include <InetLayer/IPEndPointBasis.h>
#include <InetLayer/IPAddress.h>

#include <SystemLayer/SystemPacketBuffer.h>

namespace nl {
namespace Inet {

class InetLayer;
class IPPacketInfo;

/**
 * @brief   Objects of this class represent UDP transport endpoints.
 *
 * @details
 *  Nest Inet Layer encapsulates methods for interacting with UDP transport
 *  endpoints (SOCK_DGRAM sockets on Linux and BSD-derived systems) or LwIP
 *  UDP protocol control blocks, as the system is configured accordingly.
 */
 class NL_DLL_EXPORT UDPEndPoint : public IPEndPointBasis
{
    friend class InetLayer;

public:
    INET_ERROR Bind(IPAddressType addrType, IPAddress addr, uint16_t port, InterfaceId intfId = INET_NULL_INTERFACEID);
    INET_ERROR BindInterface(IPAddressType addrType, InterfaceId intf);
    InterfaceId GetBoundInterface(void);
    uint16_t GetBoundPort(void);
    INET_ERROR Listen(void);
    INET_ERROR SendTo(IPAddress addr, uint16_t port, Weave::System::PacketBuffer *msg, uint16_t sendFlags = 0);
    INET_ERROR SendTo(IPAddress addr, uint16_t port, InterfaceId intfId, Weave::System::PacketBuffer *msg, uint16_t sendFlags = 0);
    INET_ERROR SendMsg(const IPPacketInfo *pktInfo, Weave::System::PacketBuffer *msg, uint16_t sendFlags = 0);
    void Close(void);
    void Free(void);

private:
    UDPEndPoint(void);                                  // not defined
    UDPEndPoint(const UDPEndPoint&);                    // not defined
    ~UDPEndPoint(void);                                 // not defined

    static Weave::System::ObjectPool<UDPEndPoint, INET_CONFIG_NUM_UDP_ENDPOINTS> sPool;

    void Init(InetLayer *inetLayer);

#if WEAVE_SYSTEM_CONFIG_USE_LWIP
    void HandleDataReceived(Weave::System::PacketBuffer *msg);
    INET_ERROR GetPCB(IPAddressType addrType4);
#if LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
    static void LwIPReceiveUDPMessage(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);
#else // LWIP_VERSION_MAJOR <= 1 && LWIP_VERSION_MINOR < 5
    static void LwIPReceiveUDPMessage(void *arg, struct udp_pcb *pcb, struct pbuf *p, ip_addr_t *addr, u16_t port);
#endif // LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP

#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS
    uint16_t mBoundPort;

    INET_ERROR GetSocket(IPAddressType addrType);
    SocketEvents PrepareIO(void);
    void HandlePendingIO(void);
#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS
};

} // namespace Inet
} // namespace nl

#endif // !defined(UDPENDPOINT_H)
