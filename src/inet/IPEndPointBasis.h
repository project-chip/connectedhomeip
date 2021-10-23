/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Google LLC.
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
 *      This header file defines the <tt>Inet::IPEndPointBasis</tt>
 *      class, an intermediate, non-instantiable basis class
 *      supporting other IP-based end points.
 *
 */

#pragma once

#include <inet/EndPointBasis.h>

#include <inet/InetInterface.h>
#include <inet/InetLayerEvents.h>
#include <system/SystemPacketBuffer.h>

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#include <lwip/init.h>
#include <lwip/netif.h>
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

namespace chip {
namespace Inet {

class InetLayer;
class IPPacketInfo;

/**
 * @class IPEndPointBasis
 *
 * @brief Objects of this class represent non-instantiable IP protocol
 *        endpoints.
 *
 */
class DLL_EXPORT IPEndPointBasis : public EndPointBasis
{
public:
    /**
     * @brief   Type of message text reception event handling function.
     *
     * @param[in]   endPoint    The endpoint associated with the event.
     * @param[in]   msg         The message text received.
     * @param[in]   pktInfo     The packet's IP information.
     *
     * @details
     *  Provide a function of this type to the \c OnMessageReceived delegate
     *  member to process message text reception events on \c endPoint where
     *  \c msg is the message text received from the sender at \c senderAddr.
     */
    typedef void (*OnMessageReceivedFunct)(IPEndPointBasis * endPoint, chip::System::PacketBufferHandle && msg,
                                           const IPPacketInfo * pktInfo);

    /**
     * @brief   Type of reception error event handling function.
     *
     * @param[in]   endPoint    The endpoint associated with the event.
     * @param[in]   err         The reason for the error.
     *
     * @details
     *  Provide a function of this type to the \c OnReceiveError delegate
     *  member to process reception error events on \c endPoint. The \c err
     *  argument provides specific detail about the type of the error.
     */
    typedef void (*OnReceiveErrorFunct)(IPEndPointBasis * endPoint, CHIP_ERROR err, const IPPacketInfo * pktInfo);

    IPEndPointBasis() = default;

    /**
     * Set whether IP multicast traffic should be looped back.
     */
    CHIP_ERROR SetMulticastLoopback(IPVersion aIPVersion, bool aLoopback);

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

protected:
    friend class InetLayer;

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

    void Init(InetLayer * aInetLayer);

    /** The endpoint's message reception event handling function delegate. */
    OnMessageReceivedFunct OnMessageReceived;

    /** The endpoint's receive error event handling function delegate. */
    OnReceiveErrorFunct OnReceiveError;

private:
    IPEndPointBasis(const IPEndPointBasis &) = delete;

    void InitImpl();
    CHIP_ERROR IPv4JoinLeaveMulticastGroupImpl(InterfaceId aInterfaceId, const IPAddress & aAddress, bool join);
    CHIP_ERROR IPv6JoinLeaveMulticastGroupImpl(InterfaceId aInterfaceId, const IPAddress & aAddress, bool join);

#if CHIP_SYSTEM_CONFIG_USE_LWIP
public:
    static struct netif * FindNetifFromInterfaceId(InterfaceId aInterfaceId);

protected:
    void HandleDataReceived(chip::System::PacketBufferHandle && aBuffer);

    static IPPacketInfo * GetPacketInfo(const chip::System::PacketBufferHandle & aBuffer);
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
protected:
    InterfaceId mBoundIntfId;

    CHIP_ERROR Bind(IPAddressType aAddressType, const IPAddress & aAddress, uint16_t aPort, InterfaceId aInterfaceId);
    CHIP_ERROR BindInterface(IPAddressType aAddressType, InterfaceId aInterfaceId);
    CHIP_ERROR SendMsg(const IPPacketInfo * aPktInfo, chip::System::PacketBufferHandle && aBuffer);
    CHIP_ERROR GetSocket(IPAddressType aAddressType, int aType, int aProtocol);
    void HandlePendingIO(uint16_t aPort);

#if CHIP_SYSTEM_CONFIG_USE_PLATFORM_MULTICAST_API
public:
    using MulticastGroupHandler = CHIP_ERROR (*)(InterfaceId, const IPAddress &);
    static void SetJoinMulticastGroupHandler(MulticastGroupHandler handler) { sJoinMulticastGroupHandler = handler; }
    static void SetLeaveMulticastGroupHandler(MulticastGroupHandler handler) { sLeaveMulticastGroupHandler = handler; }

private:
    static MulticastGroupHandler sJoinMulticastGroupHandler;
    static MulticastGroupHandler sLeaveMulticastGroupHandler;
#endif // CHIP_SYSTEM_CONFIG_USE_PLATFORM_MULTICAST_API

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

#if CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK
protected:
    nw_listener_t mListener;
    dispatch_semaphore_t mListenerSemaphore;
    dispatch_queue_t mListenerQueue;
    nw_connection_t mConnection;
    dispatch_semaphore_t mConnectionSemaphore;
    dispatch_queue_t mDispatchQueue;
    dispatch_semaphore_t mSendSemaphore;

    CHIP_ERROR Bind(IPAddressType aAddressType, const IPAddress & aAddress, uint16_t aPort, const nw_parameters_t & aParameters);
    CHIP_ERROR ConfigureProtocol(IPAddressType aAddressType, const nw_parameters_t & aParameters);
    CHIP_ERROR SendMsg(const IPPacketInfo * aPktInfo, chip::System::PacketBufferHandle && aBuffer);
    CHIP_ERROR StartListener();
    CHIP_ERROR GetConnection(const IPPacketInfo * aPktInfo);
    CHIP_ERROR GetEndPoint(nw_endpoint_t & aEndpoint, const IPAddressType aAddressType, const IPAddress & aAddress, uint16_t aPort);
    CHIP_ERROR StartConnection(nw_connection_t & aConnection);
    void GetPacketInfo(const nw_connection_t & aConnection, IPPacketInfo & aPacketInfo);
    void HandleDataReceived(const nw_connection_t & aConnection);
    CHIP_ERROR ReleaseListener();
    CHIP_ERROR ReleaseConnection();
    void ReleaseAll();
#endif // CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK
};

} // namespace Inet
} // namespace chip
