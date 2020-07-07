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

#ifndef IPENDPOINTBASIS_H
#define IPENDPOINTBASIS_H

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
     * @brief   Transmit option flags for the \c SendMsg method.
     */
    enum
    {
        /** Do not destructively queue the message directly. Queue a copy. */
        kSendFlag_RetainBuffer = 0x0040
    };

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
    typedef void (*OnMessageReceivedFunct)(IPEndPointBasis * endPoint, chip::System::PacketBuffer * msg,
                                           const IPPacketInfo * pktInfo);

    /** The endpoint's message reception event handling function delegate. */
    OnMessageReceivedFunct OnMessageReceived;

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
    typedef void (*OnReceiveErrorFunct)(IPEndPointBasis * endPoint, INET_ERROR err, const IPPacketInfo * pktInfo);

    /** The endpoint's receive error event handling function delegate. */
    OnReceiveErrorFunct OnReceiveError;

    INET_ERROR SetMulticastLoopback(IPVersion aIPVersion, bool aLoopback);
    INET_ERROR JoinMulticastGroup(InterfaceId aInterfaceId, const IPAddress & aAddress);
    INET_ERROR LeaveMulticastGroup(InterfaceId aInterfaceId, const IPAddress & aAddress);

protected:
    void Init(InetLayer * aInetLayer);

#if CHIP_SYSTEM_CONFIG_USE_LWIP
public:
    static struct netif * FindNetifFromInterfaceId(InterfaceId aInterfaceId);

protected:
    void HandleDataReceived(chip::System::PacketBuffer * aBuffer);

    static IPPacketInfo * GetPacketInfo(chip::System::PacketBuffer * buf);
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
protected:
    InterfaceId mBoundIntfId;

    INET_ERROR Bind(IPAddressType aAddressType, IPAddress aAddress, uint16_t aPort, InterfaceId aInterfaceId);
    INET_ERROR BindInterface(IPAddressType aAddressType, InterfaceId aInterfaceId);
    INET_ERROR SendMsg(const IPPacketInfo * aPktInfo, chip::System::PacketBuffer * aBuffer, uint16_t aSendFlags);
    INET_ERROR GetSocket(IPAddressType aAddressType, int aType, int aProtocol);
    SocketEvents PrepareIO(void);
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

    INET_ERROR Bind(IPAddressType aAddressType, const IPAddress & aAddress, uint16_t aPort, const nw_parameters_t & aParameters);
    INET_ERROR ConfigureProtocol(IPAddressType aAddressType, const nw_parameters_t & aParameters);
    INET_ERROR SendMsg(const IPPacketInfo * aPktInfo, chip::System::PacketBuffer * aBuffer, uint16_t aSendFlags);
    INET_ERROR StartListener();
    INET_ERROR GetConnection(const IPPacketInfo * aPktInfo);
    INET_ERROR GetEndPoint(nw_endpoint_t & aEndpoint, const IPAddressType aAddressType, const IPAddress & aAddress, uint16_t aPort);
    INET_ERROR StartConnection(nw_connection_t & aConnection);
    void GetPacketInfo(const nw_connection_t & aConnection, IPPacketInfo & aPacketInfo);
    void HandleDataReceived(const nw_connection_t & aConnection);
    INET_ERROR ReleaseListener();
    INET_ERROR ReleaseConnection();
    void ReleaseAll();
#endif // CHIP_SYSTEM_CONFIG_USE_NETWORK_FRAMEWORK

private:
    IPEndPointBasis(void);                    // not defined
    IPEndPointBasis(const IPEndPointBasis &); // not defined
    ~IPEndPointBasis(void);                   // not defined
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

#endif // !defined(IPENDPOINT_H)
