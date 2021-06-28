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
 *      This header file defines the <tt>Inet::RawEndPoint</tt>
 *      class, where the CHIP Inet Layer encapsulates methods for
 *      interacting with IP network endpoints (SOCK_RAW sockets
 *      on Linux and BSD-derived systems) or LwIP raw protocol
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
 * @brief   Objects of this class represent raw IP network endpoints.
 *
 * @details
 *  CHIP Inet Layer encapsulates methods for interacting with IP network
 *  endpoints (SOCK_RAW sockets on Linux and BSD-derived systems) or LwIP
 *  raw protocol control blocks, as the system is configured accordingly.
 */
class DLL_EXPORT RawEndPoint : public IPEndPointBasis
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
    IPVersion IPVer; // This data member is read-only

    /**
     * @brief   version of the Internet Control Message Protocol (ICMP)
     *
     * @details
     *  While this field is a mutable class variable, it is an invariant of the
     *  class that it not be modified.
     */
    IPProtocol IPProto; // This data member is read-only

    INET_ERROR Bind(IPAddressType addrType, const IPAddress & addr, InterfaceId intfId = INET_NULL_INTERFACEID);
    INET_ERROR BindIPv6LinkLocal(InterfaceId intfId, const IPAddress & addr);
    INET_ERROR BindInterface(IPAddressType addrType, InterfaceId intfId);
    InterfaceId GetBoundInterface();
    INET_ERROR Listen(IPEndPointBasis::OnMessageReceivedFunct onMessageReceived,
                      IPEndPointBasis::OnReceiveErrorFunct onReceiveError, void * appState = nullptr);
    INET_ERROR SendTo(const IPAddress & addr, chip::System::PacketBufferHandle && msg, uint16_t sendFlags = 0);
    INET_ERROR SendTo(const IPAddress & addr, InterfaceId intfId, chip::System::PacketBufferHandle && msg, uint16_t sendFlags = 0);
    INET_ERROR SendMsg(const IPPacketInfo * pktInfo, chip::System::PacketBufferHandle && msg, uint16_t sendFlags = 0);
    INET_ERROR SetICMPFilter(uint8_t numICMPTypes, const uint8_t * aICMPTypes);
    void Close();
    void Free();

private:
    RawEndPoint()                    = delete;
    RawEndPoint(const RawEndPoint &) = delete;
    ~RawEndPoint()                   = delete;

    static chip::System::ObjectPool<RawEndPoint, INET_CONFIG_NUM_RAW_ENDPOINTS> sPool;

    void Init(InetLayer * inetLayer, IPVersion ipVer, IPProtocol ipProto);

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    uint8_t NumICMPTypes;
    const uint8_t * ICMPTypes;

    void HandleDataReceived(chip::System::PacketBufferHandle && msg);
    INET_ERROR GetPCB(IPAddressType addrType);

#if LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
    static u8_t LwIPReceiveRawMessage(void * arg, struct raw_pcb * pcb, struct pbuf * p, const ip_addr_t * addr);
#else  // LWIP_VERSION_MAJOR <= 1 && LWIP_VERSION_MINOR < 5
    static u8_t LwIPReceiveRawMessage(void * arg, struct raw_pcb * pcb, struct pbuf * p, ip_addr_t * addr);
#endif // LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    INET_ERROR GetSocket(IPAddressType addrType);
    void HandlePendingIO();
    static void HandlePendingIO(System::WatchableSocket & socket);

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
    dispatch_source_t mReadableSource = nullptr;
#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS
};

} // namespace Inet
} // namespace chip
