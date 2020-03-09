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
 *      This header file defines the <tt>nl::Inet::RawEndPoint</tt>
 *      class, where the Nest Inet Layer encapsulates methods for
 *      interacting with IP network endpoints (SOCK_RAW sockets
 *      on Linux and BSD-derived systems) or LwIP raw protocol
 *      control blocks, as the system is configured accordingly.
 */

#ifndef RAWENDPOINT_H
#define RAWENDPOINT_H

#include <InetLayer/IPEndPointBasis.h>
#include <InetLayer/IPAddress.h>

#include <SystemLayer/SystemPacketBuffer.h>

namespace nl {
namespace Inet {

class InetLayer;
class IPPacketInfo;

/**
 * @brief   Objects of this class represent raw IP network endpoints.
 *
 * @details
 *  Nest Inet Layer encapsulates methods for interacting with IP network
 *  endpoints (SOCK_RAW sockets on Linux and BSD-derived systems) or LwIP
 *  raw protocol control blocks, as the system is configured accordingly.
 */
class NL_DLL_EXPORT RawEndPoint : public IPEndPointBasis
{
    friend class InetLayer;

public:
    /**
     * @brief   Version of the Internet protocol
     *
     * @details
     *  While this field is a mutable class variable, it is an invariant of the
     *  class that it not be modified.
     */
    IPVersion IPVer;   // This data member is read-only

    /**
     * @brief   version of the Internet Control Message Protocol (ICMP)
     *
     * @details
     *  While this field is a mutable class variable, it is an invariant of the
     *  class that it not be modified.
     */
    IPProtocol IPProto; // This data member is read-only

    INET_ERROR Bind(IPAddressType addrType, IPAddress addr, InterfaceId intfId = INET_NULL_INTERFACEID);
    INET_ERROR BindIPv6LinkLocal(InterfaceId intf, IPAddress addr);
    INET_ERROR BindInterface(IPAddressType addrType, InterfaceId intf);
    InterfaceId GetBoundInterface(void);
    INET_ERROR Listen(void);
    INET_ERROR SendTo(IPAddress addr, Weave::System::PacketBuffer *msg, uint16_t sendFlags = 0);
    INET_ERROR SendTo(IPAddress addr, InterfaceId intfId, Weave::System::PacketBuffer *msg, uint16_t sendFlags = 0);
    INET_ERROR SendMsg(const IPPacketInfo *pktInfo, Weave::System::PacketBuffer *msg, uint16_t sendFlags = 0);
    INET_ERROR SetICMPFilter(uint8_t numICMPTypes, const uint8_t * aICMPTypes);
    void Close(void);
    void Free(void);

private:
    RawEndPoint(void);                          // not defined
    RawEndPoint(const RawEndPoint&);            // not defined
    ~RawEndPoint(void);                         // not defined

    static Weave::System::ObjectPool<RawEndPoint, INET_CONFIG_NUM_RAW_ENDPOINTS> sPool;

    void Init(InetLayer *inetLayer, IPVersion ipVer, IPProtocol ipProto);

#if WEAVE_SYSTEM_CONFIG_USE_LWIP
    uint8_t NumICMPTypes;
    const uint8_t *ICMPTypes;

    void HandleDataReceived(Weave::System::PacketBuffer *msg);
    INET_ERROR GetPCB(IPAddressType addrType);

#if LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
    static u8_t LwIPReceiveRawMessage(void *arg, struct raw_pcb *pcb, struct pbuf *p, const ip_addr_t *addr);
#else // LWIP_VERSION_MAJOR <= 1 && LWIP_VERSION_MINOR < 5
    static u8_t LwIPReceiveRawMessage(void *arg, struct raw_pcb *pcb, struct pbuf *p, ip_addr_t *addr);
#endif // LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
#endif // WEAVE_SYSTEM_CONFIG_USE_LWIP

#if WEAVE_SYSTEM_CONFIG_USE_SOCKETS
    INET_ERROR GetSocket(IPAddressType addrType);
    SocketEvents PrepareIO(void);
    void HandlePendingIO(void);
#endif // WEAVE_SYSTEM_CONFIG_USE_SOCKETS
};

} // namespace Inet
} // namespace nl

#endif // !defined(RAWENDPOINT_H)
