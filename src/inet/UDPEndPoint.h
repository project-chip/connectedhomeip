/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *    @file
 *      This header file defines the <tt>Inet::UDPEndPoint</tt>
 *      class, where the CHIP Inet Layer encapsulates methods for
 *      interacting with UDP transport endpoints (SOCK_DGRAM sockets
 *      on Linux and BSD-derived systems) or LwIP UDP protocol
 *      control blocks, as the system is configured accordingly.
 */

#pragma once

#include "inet/IPEndPointBasis.h"
#include <inet/IPAddress.h>

#include <system/SystemPacketBuffer.h>

namespace chip {
namespace Inet {

class InetLayer;
class IPPacketInfo;

/**
 * @brief   Objects of this class represent UDP transport endpoints.
 *
 * @details
 *  CHIP Inet Layer encapsulates methods for interacting with UDP transport
 *  endpoints (SOCK_DGRAM sockets on Linux and BSD-derived systems) or LwIP
 *  UDP protocol control blocks, as the system is configured accordingly.
 */
class DLL_EXPORT UDPEndPoint : public IPEndPointBasis
{
    friend class InetLayer;

public:
    INET_ERROR Bind(IPAddressType addrType, const IPAddress & addr, uint16_t port, InterfaceId intfId = INET_NULL_INTERFACEID);
    INET_ERROR BindInterface(IPAddressType addrType, InterfaceId intfId);
    InterfaceId GetBoundInterface();
    uint16_t GetBoundPort();
    INET_ERROR Listen();
    INET_ERROR SendTo(const IPAddress & addr, uint16_t port, chip::System::PacketBuffer * msg, uint16_t sendFlags = 0);
    INET_ERROR SendTo(const IPAddress & addr, uint16_t port, InterfaceId intfId, chip::System::PacketBuffer * msg,
                      uint16_t sendFlags = 0);
    INET_ERROR SendMsg(const IPPacketInfo * pktInfo, chip::System::PacketBuffer * msg, uint16_t sendFlags = 0);
    void Close();
    void Free();

private:
    UDPEndPoint()                    = delete;
    UDPEndPoint(const UDPEndPoint &) = delete;
    ~UDPEndPoint()                   = delete;

    static chip::System::ObjectPool<UDPEndPoint, INET_CONFIG_NUM_UDP_ENDPOINTS> sPool;

    void Init(InetLayer * inetLayer);

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    void HandleDataReceived(chip::System::PacketBufferHandle msg);
    INET_ERROR GetPCB(IPAddressType addrType4);
#if LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
    static void LwIPReceiveUDPMessage(void * arg, struct udp_pcb * pcb, struct pbuf * p, const ip_addr_t * addr, u16_t port);
#else  // LWIP_VERSION_MAJOR <= 1 && LWIP_VERSION_MINOR < 5
    static void LwIPReceiveUDPMessage(void * arg, struct udp_pcb * pcb, struct pbuf * p, ip_addr_t * addr, u16_t port);
#endif // LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    uint16_t mBoundPort;

    INET_ERROR GetSocket(IPAddressType addrType);
    SocketEvents PrepareIO();
    void HandlePendingIO();
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS
};

} // namespace Inet
} // namespace chip
