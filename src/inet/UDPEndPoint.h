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

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
#include <dispatch/dispatch.h>
#endif

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
    UDPEndPoint() = default;

    CHIP_ERROR Bind(IPAddressType addrType, const IPAddress & addr, uint16_t port, InterfaceId intfId = INET_NULL_INTERFACEID);
    CHIP_ERROR BindInterface(IPAddressType addrType, InterfaceId intfId);
    InterfaceId GetBoundInterface();
    uint16_t GetBoundPort();
    CHIP_ERROR Listen(OnMessageReceivedFunct onMessageReceived, OnReceiveErrorFunct onReceiveError, void * appState = nullptr);
    CHIP_ERROR SendTo(const IPAddress & addr, uint16_t port, chip::System::PacketBufferHandle && msg, uint16_t sendFlags = 0);
    CHIP_ERROR SendTo(const IPAddress & addr, uint16_t port, InterfaceId intfId, chip::System::PacketBufferHandle && msg,
                      uint16_t sendFlags = 0);
    CHIP_ERROR SendMsg(const IPPacketInfo * pktInfo, chip::System::PacketBufferHandle && msg, uint16_t sendFlags = 0);
    void Close();
    void Free();

private:
    UDPEndPoint(const UDPEndPoint &) = delete;

    static chip::System::ObjectPool<UDPEndPoint, INET_CONFIG_NUM_UDP_ENDPOINTS> sPool;

    void Init(InetLayer * inetLayer);

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    void HandleDataReceived(chip::System::PacketBufferHandle && msg);
    CHIP_ERROR GetPCB(IPAddressType addrType4);
#if LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
    static void LwIPReceiveUDPMessage(void * arg, struct udp_pcb * pcb, struct pbuf * p, const ip_addr_t * addr, u16_t port);
#else  // LWIP_VERSION_MAJOR <= 1 && LWIP_VERSION_MINOR < 5
    static void LwIPReceiveUDPMessage(void * arg, struct udp_pcb * pcb, struct pbuf * p, ip_addr_t * addr, u16_t port);
#endif // LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    uint16_t mBoundPort;

    CHIP_ERROR GetSocket(IPAddressType addrType);
    void HandlePendingIO(System::SocketEvents events);
    static void HandlePendingIO(System::SocketEvents events, intptr_t data);

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
    dispatch_source_t mReadableSource = nullptr;
#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS
};

} // namespace Inet
} // namespace chip
