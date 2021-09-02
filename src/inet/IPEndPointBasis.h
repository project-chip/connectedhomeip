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
    friend class InetLayer;

public:
    /**
     * @brief   Basic dynamic state of the underlying endpoint.
     *
     * @details
     *  Objects are initialized in the "ready" state, proceed to the "bound"
     *  state after binding to a local interface address, then proceed to the
     *  "listening" state when they have continuations registered for handling
     *  events for reception of ICMP messages.
     *
     * @note
     *  The \c kBasisState_Closed state enumeration is mapped to \c
     *  kState_Ready for historical binary-compatibility reasons. The
     *  existing \c kState_Closed exists to identify separately the
     *  distinction between "not opened yet" and "previously opened
     *  now closed" that existed previously in the \c kState_Ready and
     *  \c kState_Closed states.
     */
    enum
    {
        kState_Ready     = kBasisState_Closed, /**< Endpoint initialized, but not open. */
        kState_Bound     = 1,                  /**< Endpoint bound, but not listening. */
        kState_Listening = 2,                  /**< Endpoint receiving datagrams. */
        kState_Closed    = 3                   /**< Endpoint closed, ready for release. */
    } mState;

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
    CHIP_ERROR SetMulticastLoopback(IPVersion aIPVersion, bool aLoopback);
    CHIP_ERROR JoinMulticastGroup(InterfaceId aInterfaceId, const IPAddress & aAddress);
    CHIP_ERROR LeaveMulticastGroup(InterfaceId aInterfaceId, const IPAddress & aAddress);

protected:
    void Init(InetLayer * aInetLayer);

    /** The endpoint's message reception event handling function delegate. */
    OnMessageReceivedFunct OnMessageReceived;

    /** The endpoint's receive error event handling function delegate. */
    OnReceiveErrorFunct OnReceiveError;

#if CHIP_SYSTEM_CONFIG_USE_LWIP
public:
    static struct netif * FindNetifFromInterfaceId(InterfaceId aInterfaceId);
    static CHIP_ERROR PostPacketBufferEvent(chip::System::Layer * aLayer, System::Object & aTarget, System::EventType aEventType,
                                            System::PacketBufferHandle && aBuffer);

protected:
    void HandleDataReceived(chip::System::PacketBufferHandle && aBuffer);

    static IPPacketInfo * GetPacketInfo(const chip::System::PacketBufferHandle & aBuffer);
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    InterfaceId mBoundIntfId;

    CHIP_ERROR Bind(IPAddressType aAddressType, const IPAddress & aAddress, uint16_t aPort, InterfaceId aInterfaceId);
    CHIP_ERROR BindInterface(IPAddressType aAddressType, InterfaceId aInterfaceId);
    CHIP_ERROR SendMsg(const IPPacketInfo * aPktInfo, chip::System::PacketBufferHandle && aBuffer, uint16_t aSendFlags);
    CHIP_ERROR GetSocket(IPAddressType aAddressType, int aType, int aProtocol);
    void HandlePendingIO(uint16_t aPort);
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
    CHIP_ERROR SendMsg(const IPPacketInfo * aPktInfo, chip::System::PacketBufferHandle && aBuffer, uint16_t aSendFlags);
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

#if CHIP_SYSTEM_CONFIG_USE_PLATFORM_MULTICAST_API
public:
    using JoinMulticastGroupHandler  = CHIP_ERROR (*)(InterfaceId, const IPAddress &);
    using LeaveMulticastGroupHandler = CHIP_ERROR (*)(InterfaceId, const IPAddress &);
    static void SetJoinMulticastGroupHandler(JoinMulticastGroupHandler handler) { sJoinMulticastGroupHandler = handler; }
    static void SetLeaveMulticastGroupHandler(LeaveMulticastGroupHandler handler) { sLeaveMulticastGroupHandler = handler; }

private:
    static JoinMulticastGroupHandler sJoinMulticastGroupHandler;
    static LeaveMulticastGroupHandler sLeaveMulticastGroupHandler;
#endif // CHIP_SYSTEM_CONFIG_USE_PLATFORM_MULTICAST_API

private:
    IPEndPointBasis(const IPEndPointBasis &) = delete;
};

#if CHIP_SYSTEM_CONFIG_USE_LWIP

inline struct netif * IPEndPointBasis::FindNetifFromInterfaceId(InterfaceId aInterfaceId)
{
    struct netif * lRetval = NULL;

#if LWIP_VERSION_MAJOR >= 2 && LWIP_VERSION_MINOR >= 0 && defined(NETIF_FOREACH)
    NETIF_FOREACH(lRetval)
    {
        if (lRetval == aInterfaceId)
            break;
    }
#else  // LWIP_VERSION_MAJOR < 2 || !defined(NETIF_FOREACH)
    for (lRetval = netif_list; lRetval != NULL && lRetval != aInterfaceId; lRetval = lRetval->next)
        ;
#endif // LWIP_VERSION_MAJOR >= 2 && LWIP_VERSION_MINOR >= 0 && defined(NETIF_FOREACH)

    return (lRetval);
}

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

} // namespace Inet
} // namespace chip
