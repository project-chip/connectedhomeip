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

    /**
     * Bind the endpoint to an interface IP address.
     *
     *  Binds the endpoint to the specified network interface IP address.
     *
     *  On LwIP, this method must not be called with the LwIP stack lock already acquired.
     *
     * @param[in]   addrType    Protocol version of the IP address.
     * @param[in]   addr        IP address (must be an interface address).
     * @param[in]   port        UDP port.
     * @param[in]   intfId      An optional network interface indicator.
     *
     * @retval  CHIP_NO_ERROR                   Success: endpoint bound to address.
     * @retval  CHIP_ERROR_INCORRECT_STATE      Endpoint has been bound previously.
     * @retval  CHIP_ERROR_NO_MEMORY            Insufficient memory for endpoint.
     * @retval  INET_ERROR_UNKNOWN_INTERFACE    The optionally specified interface is not present.
     * @retval  INET_ERROR_WRONG_PROTOCOL_TYPE  \c addrType does not match \c IPVer.
     * @retval  INET_ERROR_WRONG_ADDRESS_TYPE   \c addrType is \c IPAddressType::kAny, or not equal to the type of \c addr.
     * @retval  other                           Another system or platform error
     */
    CHIP_ERROR Bind(IPAddressType addrType, const IPAddress & addr, uint16_t port, InterfaceId intfId = InterfaceId());

    /**
     * Bind the endpoint to a network interface.
     *
     *  Binds the endpoint to the specified network interface IP address.
     *
     *  On LwIP, this method must not be called with the LwIP stack lock already acquired.
     *
     * @param[in]   addrType    Protocol version of the IP address.
     * @param[in]   intfId      Network interface.
     *
     * @retval  CHIP_NO_ERROR                   Success: endpoint bound to address.
     * @retval  CHIP_ERROR_NO_MEMORY            Insufficient memory for endpoint.
     * @retval  CHIP_ERROR_NOT_IMPLEMENTED      System implementation not complete.
     * @retval  INET_ERROR_UNKNOWN_INTERFACE    The interface is not present.
     * @retval  other                           Another system or platform error.
     */
    CHIP_ERROR BindInterface(IPAddressType addrType, InterfaceId intfId);

    /**
     * Get the bound interface on this endpoint.
     *
     * @return InterfaceId   The bound interface id.
     */
    InterfaceId GetBoundInterface();

    uint16_t GetBoundPort();

    /**
     * Prepare the endpoint to receive UDP messages.
     *
     *  If \c mState is already \c State::kListening, then no operation is
     *  performed, otherwise the \c mState is set to \c State::kListening and
     *  the endpoint is prepared to received UDP messages, according to the
     *  semantics of the platform.
     *
     *  On LwIP, this method must not be called with the LwIP stack lock already acquired.
     *
     * @param[in]  onMessageReceived   The endpoint's message reception event handling function delegate.
     * @param[in]  onReceiveError      The endpoint's receive error event handling function delegate.
     * @param[in]  appState            Application state pointer.
     *
     * @retval  CHIP_NO_ERROR               Success: endpoint ready to receive messages.
     * @retval  CHIP_ERROR_INCORRECT_STATE  Endpoint is already listening.
     */
    CHIP_ERROR Listen(OnMessageReceivedFunct onMessageReceived, OnReceiveErrorFunct onReceiveError, void * appState = nullptr);

    /**
     * Send a UDP message to the specified destination address.
     *
     *  If possible, then this method sends the UDP message \c msg to the destination \c addr
     *  (with \c intfId used as the scope identifier for IPv6 link-local destinations) and \c port.
     *
     * @param[in]   addr        Destination IP address.
     * @param[in]   port        Destination UDP port.
     * @param[in]   msg         Packet buffer containing the UDP message.
     * @param[in]   intfId      Optional network interface indicator.
     *
     * @retval  CHIP_NO_ERROR                       Success: \c msg is queued for transmit.
     * @retval  CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE The system does not support the requested operation.
     * @retval  INET_ERROR_WRONG_ADDRESS_TYPE       The destination address and the bound interface address do not have
     *                                              matching protocol versions or address type.
     * @retval  CHIP_ERROR_MESSAGE_TOO_LONG         \c msg does not contain the whole UDP message.
     * @retval  CHIP_ERROR_OUTBOUND_MESSAGE_TOO_BIG Only a truncated portion of \c msg was queued for transmit.
     * @retval  other                               Another system or platform error.
     */
    CHIP_ERROR SendTo(const IPAddress & addr, uint16_t port, chip::System::PacketBufferHandle && msg,
                      InterfaceId intfId = InterfaceId());

    /**
     * Send a UDP message to a specified destination.
     *
     *  Send the UDP message in \c msg to the destination address and port given in \c pktInfo.
     *  If \c pktInfo contains an interface id, the message will be sent over the specified interface.
     *  If \c pktInfo contains a source address, the given address will be used as the source of the UDP message.
     *
     * @param[in]   pktInfo     Source and destination information for the UDP message.
     * @param[in]   msg         Packet buffer containing the UDP message.
     *
     * @retval  CHIP_NO_ERROR                       Success: \c msg is queued for transmit.
     * @retval  CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE The system does not support the requested operation.
     * @retval  INET_ERROR_WRONG_ADDRESS_TYPE       The destination address and the bound interface address do not have
     *                                              matching protocol versions or address type.
     * @retval  CHIP_ERROR_MESSAGE_TOO_LONG         \c msg does not contain the whole UDP message.
     * @retval  CHIP_ERROR_OUTBOUND_MESSAGE_TOO_BIG Only a truncated portion of \c msg was queued for transmit.
     * @retval  other                               Another system or platform error.
     */
    CHIP_ERROR SendMsg(const IPPacketInfo * pktInfo, chip::System::PacketBufferHandle && msg);

    /**
     * Close the endpoint.
     *
     *  If <tt>mState != State::kClosed</tt>, then closes the endpoint, removing
     *  it from the set of endpoints eligible for communication events.
     *
     *  On LwIP systems, this method must not be called with the LwIP stack lock already acquired.
     */
    void Close();

    /**
     * Close the endpoint and recycle its memory.
     *
     *  Invokes the \c Close method, then invokes the <tt>InetLayerBasis::Release</tt> method to return the object to its
     *  memory pool.
     *
     *  On LwIP systems, this method must not be called with the LwIP stack lock already acquired.
     */
    void Free();

private:
    UDPEndPoint(const UDPEndPoint &) = delete;

    static chip::System::ObjectPool<UDPEndPoint, INET_CONFIG_NUM_UDP_ENDPOINTS> sPool;

    CHIP_ERROR BindImpl(IPAddressType addrType, const IPAddress & addr, uint16_t port, InterfaceId intfId);
    CHIP_ERROR BindInterfaceImpl(IPAddressType addrType, InterfaceId intfId);
    CHIP_ERROR ListenImpl();
    CHIP_ERROR SendMsgImpl(const IPPacketInfo * pktInfo, chip::System::PacketBufferHandle && msg);
    void CloseImpl();

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
