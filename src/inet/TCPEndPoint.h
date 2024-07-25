/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 *      This header file defines the <tt>Inet::TCPEndPoint</tt>
 *      class, where the CHIP Inet Layer encapsulates methods for
 *      interacting with TCP transport endpoints (SOCK_DGRAM sockets
 *      on Linux and BSD-derived systems) or LwIP TCP protocol
 *      control blocks, as the system is configured accordingly.
 */

#pragma once

#include <inet/EndPointBasis.h>
#include <inet/IPAddress.h>
#include <inet/InetInterface.h>
#include <inet/InetLayer.h>
#include <system/SystemLayer.h>
#include <system/SystemPacketBuffer.h>

#include <utility>

namespace chip {

namespace Inet {

class TCPTest;

/**
 * @brief   Objects of this class represent TCP transport endpoints.
 *
 * @details
 *  CHIP Inet Layer encapsulates methods for interacting with TCP transport
 *  endpoints (SOCK_STREAM sockets on Linux and BSD-derived systems) or LwIP
 *  TCP protocol control blocks, as the system is configured accordingly.
 */
class DLL_EXPORT TCPEndPoint : public EndPointBasis<TCPEndPoint>
{
public:
    /**
     * @brief   Bind the endpoint to an interface IP address.
     *
     * @param[in]   addrType    the protocol version of the IP address
     * @param[in]   addr        the IP address (must be an interface address)
     * @param[in]   port        the TCP port
     * @param[in]   reuseAddr   option to share binding with other endpoints
     *
     * @retval  CHIP_NO_ERROR               success: endpoint bound to address
     * @retval  CHIP_ERROR_INCORRECT_STATE  endpoint has been bound previously
     * @retval  CHIP_ERROR_NO_MEMORY        insufficient memory for endpoint
     *
     * @retval  INET_ERROR_WRONG_PROTOCOL_TYPE
     *      \c addrType does not match \c IPVer.
     *
     * @retval  INET_ERROR_WRONG_ADDRESS_TYPE
     *      \c addrType is \c IPAddressType::kAny, or the type of \c addr is not
     *      equal to \c addrType.
     *
     * @retval  other                   another system or platform error
     *
     * @details
     *  Binds the endpoint to the specified network interface IP address.
     *
     *  On LwIP, this method must not be called with the LwIP stack lock
     *  already acquired.
     */
    CHIP_ERROR Bind(IPAddressType addrType, const IPAddress & addr, uint16_t port, bool reuseAddr = false);

    /**
     * @brief   Prepare the endpoint to receive TCP messages.
     *
     * @param[in]   backlog     maximum depth of connection acceptance queue
     *
     * @retval  CHIP_NO_ERROR   success: endpoint ready to receive messages.
     * @retval  CHIP_ERROR_INCORRECT_STATE  endpoint is already listening.
     *
     * @details
     *  If \c mState is already \c State::kListening, then no operation is
     *  performed, otherwise the \c mState is set to \c State::kListening and
     *  the endpoint is prepared to received TCP messages, according to the
     *  semantics of the platform.
     *
     *  On some platforms, the \c backlog argument is not used (the depth of
     *  the queue is fixed; only one connection may be accepted at a time).
     *
     *  On LwIP systems, this method must not be called with the LwIP stack
     *  lock already acquired
     */
    CHIP_ERROR Listen(uint16_t backlog);

    /**
     * @brief   Initiate a TCP connection.
     *
     * @param[in]   addr        the destination IP address
     * @param[in]   port        the destination TCP port
     * @param[in]   intfId      an optional network interface indicator
     *
     * @retval  CHIP_NO_ERROR       success: \c msg is queued for transmit.
     * @retval  CHIP_ERROR_NOT_IMPLEMENTED  system implementation not complete.
     *
     * @retval  INET_ERROR_WRONG_ADDRESS_TYPE
     *      the destination address and the bound interface address do not
     *      have matching protocol versions or address type, or the destination
     *      address is an IPv6 link-local address and \c intfId is not specified.
     *
     * @retval  other                   another system or platform error
     *
     * @details
     *      If possible, then this method initiates a TCP connection to the
     *      destination \c addr (with \c intfId used as the scope
     *      identifier for IPv6 link-local destinations) and \c port.
     */
    CHIP_ERROR Connect(const IPAddress & addr, uint16_t port, InterfaceId intfId = InterfaceId::Null());

    /**
     * @brief   Extract IP address and TCP port of remote endpoint.
     *
     * @param[out]  retAddr     IP address of remote endpoint.
     * @param[out]  retPort     TCP port of remote endpoint.
     *
     * @retval  CHIP_NO_ERROR           success: address and port extracted.
     * @retval  CHIP_ERROR_INCORRECT_STATE  TCP connection not established.
     * @retval  CHIP_ERROR_CONNECTION_ABORTED   TCP connection no longer open.
     *
     * @details
     *  Do not use \c nullptr for either argument.
     */
    virtual CHIP_ERROR GetPeerInfo(IPAddress * retAddr, uint16_t * retPort) const = 0;

    /**
     * @brief   Extract IP address and TCP port of local endpoint.
     *
     * @param[out]  retAddr     IP address of local endpoint.
     * @param[out]  retPort     TCP port of local endpoint.
     *
     * @retval  CHIP_NO_ERROR           success: address and port extracted.
     * @retval  CHIP_ERROR_INCORRECT_STATE  TCP connection not established.
     * @retval  CHIP_ERROR_CONNECTION_ABORTED   TCP connection no longer open.
     *
     * @details
     *  Do not use \c nullptr for either argument.
     */
    virtual CHIP_ERROR GetLocalInfo(IPAddress * retAddr, uint16_t * retPort) const = 0;

    /**
     * @brief   Extract the interface id of the TCP endpoint.
     *
     * @param[out]  retInterface  The interface id.
     *
     * @retval  CHIP_NO_ERROR           success: address and port extracted.
     * @retval  CHIP_ERROR_INCORRECT_STATE  TCP connection not established.
     * @retval  CHIP_ERROR_CONNECTION_ABORTED   TCP connection no longer open.
     */
    virtual CHIP_ERROR GetInterfaceId(InterfaceId * retInterface) = 0;

    /**
     * @brief   Send message text on TCP connection.
     *
     * @param[out]  data    Message text to send.
     * @param[out]  push    If \c true, then send immediately, otherwise queue.
     *
     * @retval  CHIP_NO_ERROR           success: address and port extracted.
     * @retval  CHIP_ERROR_INCORRECT_STATE  TCP connection not established.
     */
    CHIP_ERROR Send(chip::System::PacketBufferHandle && data, bool push = true);

    /**
     * Disable reception.
     *
     *  Disable all event handlers. Data sent to an endpoint that disables
     *  reception will be acknowledged until the receive window is exhausted.
     */
    void DisableReceive() { mReceiveEnabled = false; }

    /**
     * Enable reception.
     *
     *  Enable all event handlers. Data sent to an endpoint that disables
     *  reception will be acknowledged until the receive window is exhausted.
     */
    void EnableReceive()
    {
        mReceiveEnabled = true;
        DriveReceiving();
    }

    /**
     * Switch off Nagle buffering algorithm.
     */
    virtual CHIP_ERROR EnableNoDelay() = 0;

    /**
     * @brief
     *    Enable TCP keepalive probes on the associated TCP connection.
     *
     *  @param[in] interval
     *    The interval (in seconds) between keepalive probes.  This value also controls
     *    the time between last data packet sent and the transmission of the first keepalive
     *    probe.
     *
     *  @param[in] timeoutCount
     *    The maximum number of unacknowledged probes before the connection will be deemed
     *    to have failed.
     *
     * @retval  CHIP_NO_ERROR           success: address and port extracted.
     * @retval  CHIP_ERROR_INCORRECT_STATE  TCP connection not established.
     * @retval  CHIP_ERROR_CONNECTION_ABORTED   TCP connection no longer open.
     * @retval  CHIP_ERROR_NOT_IMPLEMENTED  system implementation not complete.
     *
     * @retval  other                   another system or platform error
     *
     *  @note
     *    This method can only be called when the endpoint is in one of the connected states.
     *
     *    This method can be called multiple times to adjust the keepalive interval or timeout
     *    count.
     *
     * @details
     *  Start automatically  transmitting TCP "keep-alive" probe segments every
     *  \c interval seconds. The connection will abort automatically after
     *  receiving a negative response, or after sending \c timeoutCount
     *  probe segments without receiving a positive response.
     *
     *  See RFC 1122, section 4.2.3.6 for specification details.
     */
    virtual CHIP_ERROR EnableKeepAlive(uint16_t interval, uint16_t timeoutCount) = 0;

    /**
     * @brief   Disable the TCP "keep-alive" option.
     *
     *    This method can only be called when the endpoint is in one of the connected states.
     *    This method does nothing if keepalives have not been enabled on the endpoint.
     *
     * @retval  CHIP_NO_ERROR           success: address and port extracted.
     * @retval  CHIP_ERROR_INCORRECT_STATE  TCP connection not established.
     * @retval  CHIP_ERROR_CONNECTION_ABORTED   TCP connection no longer open.
     * @retval  CHIP_ERROR_NOT_IMPLEMENTED  system implementation not complete.
     *
     * @retval  other                   another system or platform error
     */
    virtual CHIP_ERROR DisableKeepAlive() = 0;

    /**
     * @brief   Acknowledge receipt of message text.
     *
     * @param[in]   len     number of bytes to acknowledge.
     *
     * @retval  CHIP_NO_ERROR           success: reception acknowledged.
     * @retval  CHIP_ERROR_INCORRECT_STATE  TCP connection not established.
     * @retval  CHIP_ERROR_CONNECTION_ABORTED   TCP connection no longer open.
     *
     * @details
     *  Use this method to acknowledge reception of all or part of the data
     *  received. The operational semantics are undefined if \c len is larger
     *  than the total outstanding unacknowledged received data.
     */
    virtual CHIP_ERROR AckReceive(size_t len) = 0;

    /**
     * @brief   Set the receive queue, for testing.
     *
     * @param[out]  data    Message text to push.
     *
     * @retval  CHIP_NO_ERROR           success: reception acknowledged.
     * @retval  CHIP_ERROR_INCORRECT_STATE  TCP connection not established.
     *
     * @details
     *  This method may only be called by data reception event handlers to
     *  put data on the receive queue for unit test purposes.
     */
    CHIP_ERROR SetReceivedDataForTesting(chip::System::PacketBufferHandle && data);

    /**
     * @brief   Extract the length of the data awaiting first transmit.
     *
     * @return  Number of untransmitted bytes in the transmit queue.
     */
    size_t PendingSendLength();

    /**
     * @brief   Extract the length of the unacknowledged receive data.
     *
     * @return  Number of bytes in the receive queue that have not yet been
     *      acknowledged with <tt>AckReceive(uint16_t len)</tt>.
     */
    size_t PendingReceiveLength();

    /**
     * @brief   Initiate TCP half close, in other words, finished with sending.
     */
    void Shutdown();

    /**
     * @brief   Initiate TCP full close, in other words, finished with both send and
     *  receive.
     */
    void Close();

    /**
     * @brief   Abortively close the endpoint, in other words, send RST packets.
     */
    void Abort();

    /**
     * @brief   Initiate (or continue) TCP full close, ignoring errors.
     *
     * @details
     *  The object is returned to the free pool, and all remaining user
     *  references are subsequently invalid.
     */
    void Free();

    /**
     * @brief   Extract whether TCP connection is established.
     */
    bool IsConnected() const { return IsConnected(mState); }

    /**
     * Set timeout for Connect to succeed or return an error.
     */
    void SetConnectTimeout(const uint32_t connTimeoutMsecs) { mConnectTimeoutMsecs = connTimeoutMsecs; }

#if INET_TCP_IDLE_CHECK_INTERVAL > 0
    /**
     * @brief   Set timer event for idle activity.
     *
     * @param[in]   timeoutMS The timeout in milliseconds
     *
     * @details
     *  Set the idle timer interval to \c timeoutMS milliseconds. A zero
     *  time interval implies the idle timer is disabled.
     */
    void SetIdleTimeout(uint32_t timeoutMS);
#endif // INET_TCP_IDLE_CHECK_INTERVAL > 0

    /**
     * @brief   Note activity, in other words, reset the idle timer.
     *
     * @details
     *  Reset the idle timer to zero.
     */
    void MarkActive()
    {
#if INET_TCP_IDLE_CHECK_INTERVAL > 0
        mRemainingIdleTime = mIdleTimeout;
#endif // INET_TCP_IDLE_CHECK_INTERVAL > 0
    }

    /**
     * @brief   Set the TCP TCP_USER_TIMEOUT socket option.
     *
     * @param[in]   userTimeoutMillis    Tcp user timeout value in milliseconds.
     *
     * @retval  CHIP_NO_ERROR           success: address and port extracted.
     * @retval  CHIP_ERROR_NOT_IMPLEMENTED  system implementation not complete.
     *
     * @retval  other                   another system or platform error
     *
     * @details
     *  When the value is greater than 0, it specifies the maximum amount of
     *  time in milliseconds that transmitted data may remain
     *  unacknowledged before TCP will forcibly close the
     *  corresponding connection. If the option value is specified as 0,
     *  TCP will to use the system default.
     *  See RFC 5482, for further details.
     *
     *  @note
     *    This method can only be called when the endpoint is in one of the connected states.
     *
     *    This method can be called multiple times to adjust the keepalive interval or timeout
     *    count.
     */
    CHIP_ERROR SetUserTimeout(uint32_t userTimeoutMillis);

    /**
     * @brief   Type of connection establishment event handling function.
     *
     * @param[in]   endPoint    The TCP endpoint associated with the event.
     * @param[in]   err         \c CHIP_NO_ERROR if success, else another code.
     *
     * @details
     *  Provide a function of this type to the \c OnConnectComplete delegate
     *  member to process connection establishment events on \c endPoint. The
     *  \c err argument distinguishes successful connections from failures.
     */
    typedef void (*OnConnectCompleteFunct)(TCPEndPoint * endPoint, CHIP_ERROR err);

    /**
     * The endpoint's connection establishment event handling function
     * delegate.
     */
    OnConnectCompleteFunct OnConnectComplete;

    /**
     * @brief   Type of data reception event handling function.
     *
     * @param[in]   endPoint        The TCP endpoint associated with the event.
     * @param[in]   data            The data received.
     *
     * @retval      CHIP_NO_ERROR   If the received data can be handled by higher layers.
     * @retval      other           If the received data can not be used, and higher layers will not see it.
     *
     * @details
     *  Provide a function of this type to the \c OnDataReceived delegate
     *  member to process data reception events on \c endPoint where \c data
     *  is the message text received.
     *
     *  If this function returns an error, the connection will be closed, since higher layers
     *  are not able to process the data for a better response.
     */
    typedef CHIP_ERROR (*OnDataReceivedFunct)(TCPEndPoint * endPoint, chip::System::PacketBufferHandle && data);

    /**
     * The endpoint's message text reception event handling function delegate.
     */
    OnDataReceivedFunct OnDataReceived;

    /**
     * @brief   Type of data transmission event handling function.
     *
     * @param[in]   endPoint    The TCP endpoint associated with the event.
     * @param[in]   len         Number of bytes added to the transmit window.
     *
     * @details
     *  Provide a function of this type to the \c OnDataSent delegate
     *  member to process data transmission events on \c endPoint where \c len
     *  is the length of the message text added to the TCP transmit window,
     *  which are eligible for sending by the underlying network stack.
     */
    typedef void (*OnDataSentFunct)(TCPEndPoint * endPoint, size_t len);

    /**
     * The endpoint's message text transmission event handling function
     * delegate.
     */
    OnDataSentFunct OnDataSent;

    /**
     * @brief   Type of connection establishment event handling function.
     *
     * @param[in]   endPoint    The TCP endpoint associated with the event.
     * @param[in]   err         \c CHIP_NO_ERROR if success, else another code.
     *
     * @details
     *  Provide a function of this type to the \c OnConnectionClosed delegate
     *  member to process connection termination events on \c endPoint. The
     *  \c err argument distinguishes successful terminations from failures.
     */
    typedef void (*OnConnectionClosedFunct)(TCPEndPoint * endPoint, CHIP_ERROR err);

    /** The endpoint's close event handling function delegate. */
    OnConnectionClosedFunct OnConnectionClosed;

    /**
     * @brief   Type of half-close reception event handling function.
     *
     * @param[in]   endPoint    The TCP endpoint associated with the event.
     *
     * @details
     *  Provide a function of this type to the \c OnPeerClose delegate member
     *  to process connection termination events on \c endPoint.
     */
    typedef void (*OnPeerCloseFunct)(TCPEndPoint * endPoint);

    /** The endpoint's half-close receive event handling function delegate. */
    OnPeerCloseFunct OnPeerClose;

    /**
     * @brief   Type of connection received event handling function.
     *
     * @param[in]   listeningEndPoint   The listening TCP endpoint.
     * @param[in]   conEndPoint         The newly received TCP endpoint.
     * @param[in]   peerAddr            The IP address of the remote peer.
     * @param[in]   peerPort            The TCP port of the remote peer.
     *
     * @details
     *  Provide a function of this type to the \c OnConnectionReceived delegate
     *  member to process connection reception events on \c listeningEndPoint.
     *  The newly received endpoint \c conEndPoint is located at IP address
     *  \c peerAddr and TCP port \c peerPort.
     */
    typedef void (*OnConnectionReceivedFunct)(TCPEndPoint * listeningEndPoint, TCPEndPoint * conEndPoint,
                                              const IPAddress & peerAddr, uint16_t peerPort);

    /** The endpoint's connection receive event handling function delegate. */
    OnConnectionReceivedFunct OnConnectionReceived;

    /**
     * @brief   Type of connection acceptance error event handling function.
     *
     * @param[in]   endPoint    The TCP endpoint associated with the event.
     * @param[in]   err         The reason for the error.
     *
     * @details
     *  Provide a function of this type to the \c OnAcceptError delegate
     *  member to process connection acceptance error events on \c endPoint. The
     *  \c err argument provides specific detail about the type of the error.
     */
    typedef void (*OnAcceptErrorFunct)(TCPEndPoint * endPoint, CHIP_ERROR err);

    /**
     * The endpoint's connection acceptance event handling function delegate.
     */
    OnAcceptErrorFunct OnAcceptError;

    /**
     * Size of the largest TCP packet that can be received.
     */
    constexpr static size_t kMaxReceiveMessageSize = System::PacketBuffer::kMaxAllocSize;

protected:
    friend class TCPTest;

    TCPEndPoint(EndPointManager<TCPEndPoint> & endPointManager) :
        EndPointBasis(endPointManager), OnConnectComplete(nullptr), OnDataReceived(nullptr), OnDataSent(nullptr),
        OnConnectionClosed(nullptr), OnPeerClose(nullptr), OnConnectionReceived(nullptr), OnAcceptError(nullptr),
        mState(State::kReady), mReceiveEnabled(true), mConnectTimeoutMsecs(0) // Initialize to zero for using system defaults.
#if INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT
        ,
        mUserTimeoutMillis(INET_CONFIG_DEFAULT_TCP_USER_TIMEOUT_MSEC), mUserTimeoutTimerRunning(false)
#endif // INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT
    {}

    virtual ~TCPEndPoint() = default;

    /**
     * Basic dynamic state of the underlying endpoint.
     *
     *  Objects are initialized in the "ready" state, proceed to subsequent
     *  states corresponding to a simplification of the states of the TCP
     *  transport state machine.
     */
    enum class State : uint8_t
    {
        kReady           = 0, /**< Endpoint initialized, but not bound. */
        kBound           = 1, /**< Endpoint bound, but not listening. */
        kListening       = 2, /**< Endpoint receiving connections. */
        kConnecting      = 3, /**< Endpoint attempting to connect. */
        kConnected       = 4, /**< Endpoint connected, ready for tx/rx. */
        kSendShutdown    = 5, /**< Endpoint initiated its half-close. */
        kReceiveShutdown = 6, /**< Endpoint responded to half-close. */
        kClosing         = 7, /**< Endpoint closing bidirectionally. */
        kClosed          = 8  /**< Endpoint closed, ready for release. */
    } mState;

    /** Control switch indicating whether the application is receiving data. */
    bool mReceiveEnabled;

    chip::System::PacketBufferHandle mRcvQueue;
    chip::System::PacketBufferHandle mSendQueue;
#if INET_TCP_IDLE_CHECK_INTERVAL > 0
    static void HandleIdleTimer(System::Layer * aSystemLayer, void * aAppState);
    static bool IsIdleTimerRunning(EndPointManager<TCPEndPoint> & endPointManager);
    uint16_t mIdleTimeout;       // in units of INET_TCP_IDLE_CHECK_INTERVAL; zero means no timeout
    uint16_t mRemainingIdleTime; // in units of INET_TCP_IDLE_CHECK_INTERVAL
#endif                           // INET_TCP_IDLE_CHECK_INTERVAL > 0

    uint32_t mConnectTimeoutMsecs; // This is the timeout to wait for a Connect call to succeed or
                                   // return an error; zero means use system defaults.

#if INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT
    uint32_t mUserTimeoutMillis;   // The configured TCP user timeout value in milliseconds.
                                   // If 0, assume not set.
    bool mUserTimeoutTimerRunning; // Indicates whether the TCP UserTimeout timer has been started.

    static void TCPUserTimeoutHandler(chip::System::Layer * aSystemLayer, void * aAppState);
    virtual void TCPUserTimeoutHandler() = 0;

    void StartTCPUserTimeoutTimer();
    void StopTCPUserTimeoutTimer();
    void RestartTCPUserTimeoutTimer();
    void ScheduleNextTCPUserTimeoutPoll(uint32_t aTimeOut);
#endif // INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT

    TCPEndPoint(const TCPEndPoint &) = delete;

    CHIP_ERROR DriveSending();
    void DriveReceiving();
    void HandleConnectComplete(CHIP_ERROR err);
    void HandleAcceptError(CHIP_ERROR err);
    void DoClose(CHIP_ERROR err, bool suppressCallback);
    static bool IsConnected(State state);

    static void TCPConnectTimeoutHandler(chip::System::Layer * aSystemLayer, void * aAppState);

    void StartConnectTimerIfSet();
    void StopConnectTimer();

    friend class TCPEndPointDeletor;

    /*
     * Implementation helpers for shared methods.
     */
    virtual CHIP_ERROR BindImpl(IPAddressType addrType, const IPAddress & addr, uint16_t port, bool reuseAddr) = 0;
    virtual CHIP_ERROR ListenImpl(uint16_t backlog)                                                            = 0;
    virtual CHIP_ERROR ConnectImpl(const IPAddress & addr, uint16_t port, InterfaceId intfId)                  = 0;
    virtual CHIP_ERROR SendQueuedImpl(bool queueWasEmpty)                                                      = 0;
    virtual CHIP_ERROR SetUserTimeoutImpl(uint32_t userTimeoutMillis)                                          = 0;
    virtual CHIP_ERROR DriveSendingImpl()                                                                      = 0;
    virtual void HandleConnectCompleteImpl()                                                                   = 0;
    virtual void DoCloseImpl(CHIP_ERROR err, State oldState)                                                   = 0;
};

template <>
struct EndPointProperties<TCPEndPoint>
{
    static constexpr char kName[]         = "TCP";
    static constexpr size_t kNumEndPoints = INET_CONFIG_NUM_TCP_ENDPOINTS;
    static constexpr int kSystemStatsKey  = System::Stats::kInetLayer_NumTCPEps;
};

} // namespace Inet
} // namespace chip
