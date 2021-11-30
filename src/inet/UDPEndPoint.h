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

#include <inet/EndPointBasis.h>
#include <inet/IPAddress.h>
#include <inet/IPPacketInfo.h>
#include <inet/InetInterface.h>
#include <inet/InetLayer.h>
#include <lib/support/Pool.h>
#include <system/SystemPacketBuffer.h>

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#include <inet/EndPointStateLwIP.h>
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP
#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
#include <inet/EndPointStateSockets.h>
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS
#if CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK
#include <inet/EndPointStateNetworkFramework.h>
#endif // CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
#include <dispatch/dispatch.h>
#endif

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

#if CHIP_SYSTEM_CONFIG_USE_LWIP

class UDPEndPointImplLwIP : public UDPEndPoint, public EndPointStateLwIP
{
public:
    UDPEndPointImplLwIP(EndPointManager<UDPEndPoint> & endPointManager) : UDPEndPoint(endPointManager) {}

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
#if LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
    static void LwIPReceiveUDPMessage(void * arg, struct udp_pcb * pcb, struct pbuf * p, const ip_addr_t * addr, u16_t port);
#else  // LWIP_VERSION_MAJOR <= 1 && LWIP_VERSION_MINOR < 5
    static void LwIPReceiveUDPMessage(void * arg, struct udp_pcb * pcb, struct pbuf * p, ip_addr_t * addr, u16_t port);
#endif // LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
};

using UDPEndPointImpl = UDPEndPointImplLwIP;

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS

class UDPEndPointImplSockets : public UDPEndPoint, public EndPointStateSockets
{
public:
    UDPEndPointImplSockets(EndPointManager<UDPEndPoint> & endPointManager) :
        UDPEndPoint(endPointManager), mBoundIntfId(InterfaceId::Null())
    {}

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

    CHIP_ERROR GetSocket(IPAddressType addressType);
    void HandlePendingIO(System::SocketEvents events);
    static void HandlePendingIO(System::SocketEvents events, intptr_t data);

    InterfaceId mBoundIntfId;
    uint16_t mBoundPort;

#if CHIP_SYSTEM_CONFIG_USE_DISPATCH
    dispatch_source_t mReadableSource = nullptr;
#endif // CHIP_SYSTEM_CONFIG_USE_DISPATCH

#if CHIP_SYSTEM_CONFIG_USE_PLATFORM_MULTICAST_API
public:
    using MulticastGroupHandler = CHIP_ERROR (*)(InterfaceId, const IPAddress &);
    static void SetJoinMulticastGroupHandler(MulticastGroupHandler handler) { sJoinMulticastGroupHandler = handler; }
    static void SetLeaveMulticastGroupHandler(MulticastGroupHandler handler) { sLeaveMulticastGroupHandler = handler; }

private:
    static MulticastGroupHandler sJoinMulticastGroupHandler;
    static MulticastGroupHandler sLeaveMulticastGroupHandler;
#endif // CHIP_SYSTEM_CONFIG_USE_PLATFORM_MULTICAST_API
};

using UDPEndPointImpl = UDPEndPointImplSockets;

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

#if CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK

class UDPEndPointImplNetworkFramework : public UDPEndPoint, public EndPointStateNetworkFramework
{
public:
    UDPEndPointImplNetworkFramework(EndPointManager<UDPEndPoint> & endPointManager) : UDPEndPoint(endPointManager) {}

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

    nw_listener_t mListener;
    dispatch_semaphore_t mListenerSemaphore;
    dispatch_queue_t mListenerQueue;
    nw_connection_t mConnection;
    dispatch_semaphore_t mConnectionSemaphore;
    dispatch_queue_t mDispatchQueue;
    dispatch_semaphore_t mSendSemaphore;

    CHIP_ERROR ConfigureProtocol(IPAddressType aAddressType, const nw_parameters_t & aParameters);
    CHIP_ERROR StartListener();
    CHIP_ERROR GetConnection(const IPPacketInfo * aPktInfo);
    CHIP_ERROR GetEndPoint(nw_endpoint_t & aEndpoint, const IPAddressType aAddressType, const IPAddress & aAddress, uint16_t aPort);
    CHIP_ERROR StartConnection(nw_connection_t & aConnection);
    void GetPacketInfo(const nw_connection_t & aConnection, IPPacketInfo & aPacketInfo);
    void HandleDataReceived(const nw_connection_t & aConnection);
    CHIP_ERROR ReleaseListener();
    CHIP_ERROR ReleaseConnection();
    void ReleaseAll();
};

using UDPEndPointImpl = UDPEndPointImplNetworkFramework;

#endif // CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK

using UDPEndPointManagerImpl = EndPointManagerImplPool<UDPEndPointImpl, INET_CONFIG_NUM_UDP_ENDPOINTS>;

template <>
struct EndPointProperties<UDPEndPoint>
{
    static constexpr const char * Name  = "UDP";
    static constexpr int SystemStatsKey = System::Stats::kInetLayer_NumUDPEps;
};

} // namespace Inet
} // namespace chip
