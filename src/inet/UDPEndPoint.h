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

#include <inet/InetConfig.h>

#include <inet/EndPointBasis.h>
#include <inet/IPAddress.h>
#include <inet/IPPacketInfo.h>
#include <inet/InetInterface.h>
#include <inet/InetLayer.h>
#include <system/SystemPacketBuffer.h>

struct otInstance;

namespace chip {
namespace Inet {

/**
 * @brief   Objects of this class represent UDP transport endpoints.
 *
 * @details
 *  CHIP Inet Layer encapsulates methods for interacting with UDP transport
 *  endpoints (SOCK_DGRAM sockets on Linux and BSD-derived systems) or LwIP
 *  UDP protocol control blocks, as the system is configured accordingly.
 */
class DLL_EXPORT UDPEndPoint : public EndPointBasis<UDPEndPoint>
{
public:
    UDPEndPoint(const UDPEndPoint &) = delete;
    UDPEndPoint(UDPEndPoint &&)      = delete;
    UDPEndPoint & operator=(const UDPEndPoint &) = delete;
    UDPEndPoint & operator=(UDPEndPoint &&) = delete;

    /**
     * Type of message text reception event handling function.
     *
     * @param[in]   endPoint    The endpoint associated with the event.
     * @param[in]   msg         The message text received.
     * @param[in]   pktInfo     The packet's IP information.
     *
     *  Provide a function of this type to the \c OnMessageReceived delegate
     *  member to process message text reception events on \c endPoint where
     *  \c msg is the message text received from the sender at \c senderAddr.
     */
    using OnMessageReceivedFunct = void (*)(UDPEndPoint * endPoint, chip::System::PacketBufferHandle && msg,
                                            const IPPacketInfo * pktInfo);

    /**
     * Type of reception error event handling function.
     *
     * @param[in]   endPoint    The endpoint associated with the event.
     * @param[in]   err         The reason for the error.
     * @param[in]   pktInfo     The packet's IP information.
     *
     *  Provide a function of this type to the \c OnReceiveError delegate
     *  member to process reception error events on \c endPoint. The \c err
     *  argument provides specific detail about the type of the error.
     */
    using OnReceiveErrorFunct = void (*)(UDPEndPoint * endPoint, CHIP_ERROR err, const IPPacketInfo * pktInfo);

    /**
     * Set whether IP multicast traffic should be looped back.
     */
    virtual CHIP_ERROR SetMulticastLoopback(IPVersion aIPVersion, bool aLoopback) = 0;

    /**
     * Join an IP multicast group.
     *
     *  @param[in]   aInterfaceId   The indicator of the network interface to add to the multicast group.
     *  @param[in]   aAddress       The multicast group to add the interface to.
     *
     *  @retval  CHIP_NO_ERROR                  Success: multicast group removed.
     *  @retval  INET_ERROR_UNKNOWN_INTERFACE   Unknown network interface, \c aInterfaceId.
     *  @retval  INET_ERROR_WRONG_ADDRESS_TYPE  \c aAddress is not \c kIPv4 or \c kIPv6 or is not multicast.
     *  @retval  other                          Another system or platform error.
     */
    CHIP_ERROR JoinMulticastGroup(InterfaceId aInterfaceId, const IPAddress & aAddress);

    /**
     * Leave an IP multicast group.
     *
     *  @param[in]   aInterfaceId   The indicator of the network interface to remove from the multicast group.
     *  @param[in]   aAddress       The multicast group to remove the interface from.
     *
     *  @retval  CHIP_NO_ERROR                  Success: multicast group removed
     *  @retval  INET_ERROR_UNKNOWN_INTERFACE   Unknown network interface, \c aInterfaceId
     *  @retval  INET_ERROR_WRONG_ADDRESS_TYPE  \c aAddress is not \c kIPv4 or \c kIPv6 or is not multicast.
     *  @retval  other                          Another system or platform error
     */
    CHIP_ERROR LeaveMulticastGroup(InterfaceId aInterfaceId, const IPAddress & aAddress);

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
    CHIP_ERROR Bind(IPAddressType addrType, const IPAddress & addr, uint16_t port, InterfaceId intfId = InterfaceId::Null());

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
     */
    virtual InterfaceId GetBoundInterface() const = 0;

    /**
     * Get the bound port on this endpoint.
     */
    virtual uint16_t GetBoundPort() const = 0;

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
                      InterfaceId intfId = InterfaceId::Null());

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
     *  Invokes the \c Close method, then invokes the <tt>EndPointBasis::Release</tt> method to return the object to its
     *  memory pool.
     *
     *  On LwIP systems, this method must not be called with the LwIP stack lock already acquired.
     */
    virtual void Free() = 0;

    /**
     * Set Network Native Parameters (optional)
     *
     * Some networking stack requires additionnal parameters
     */
    virtual inline void SetNativeParams(void * params) { (void) params; }

protected:
    UDPEndPoint(EndPointManager<UDPEndPoint> & endPointManager) :
        EndPointBasis(endPointManager), mState(State::kReady), OnMessageReceived(nullptr), OnReceiveError(nullptr)
    {}

    virtual ~UDPEndPoint() = default;

    /**
     * Basic dynamic state of the underlying endpoint.
     *
     *  Objects are initialized in the "ready" state, proceed to the "bound"
     *  state after binding to a local interface address, then proceed to the
     *  "listening" state when they have continuations registered for handling
     *  events for reception of ICMP messages.
     */
    enum class State : uint8_t
    {
        kReady     = 0, /**< Endpoint initialized, but not open. */
        kBound     = 1, /**< Endpoint bound, but not listening. */
        kListening = 2, /**< Endpoint receiving datagrams. */
        kClosed    = 3  /**< Endpoint closed, ready for release. */
    } mState;

    /** The endpoint's message reception event handling function delegate. */
    OnMessageReceivedFunct OnMessageReceived;

    /** The endpoint's receive error event handling function delegate. */
    OnReceiveErrorFunct OnReceiveError;

    /*
     * Implementation helpers for shared methods.
     */
#if INET_CONFIG_ENABLE_IPV4
    virtual CHIP_ERROR IPv4JoinLeaveMulticastGroupImpl(InterfaceId aInterfaceId, const IPAddress & aAddress, bool join) = 0;
#endif // INET_CONFIG_ENABLE_IPV4
    virtual CHIP_ERROR IPv6JoinLeaveMulticastGroupImpl(InterfaceId aInterfaceId, const IPAddress & aAddress, bool join) = 0;

    virtual CHIP_ERROR BindImpl(IPAddressType addressType, const IPAddress & address, uint16_t port, InterfaceId interfaceId) = 0;
    virtual CHIP_ERROR BindInterfaceImpl(IPAddressType addressType, InterfaceId interfaceId)                                  = 0;
    virtual CHIP_ERROR ListenImpl()                                                                                           = 0;
    virtual CHIP_ERROR SendMsgImpl(const IPPacketInfo * pktInfo, chip::System::PacketBufferHandle && msg)                     = 0;
    virtual void CloseImpl()                                                                                                  = 0;
};

template <>
struct EndPointProperties<UDPEndPoint>
{
    static constexpr const char * kName   = "UDP";
    static constexpr size_t kNumEndPoints = INET_CONFIG_NUM_UDP_ENDPOINTS;
    static constexpr int kSystemStatsKey  = System::Stats::kInetLayer_NumUDPEps;
};

} // namespace Inet
} // namespace chip
