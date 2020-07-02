/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#ifndef TCPENDPOINT_H
#define TCPENDPOINT_H

#include <inet/EndPointBasis.h>
#include <inet/IPAddress.h>

#include <system/SystemPacketBuffer.h>

namespace chip {
namespace Inet {

class InetLayer;

/**
 * @brief   Objects of this class represent TCP transport endpoints.
 *
 * @details
 *  CHIP Inet Layer encapsulates methods for interacting with TCP transport
 *  endpoints (SOCK_STREAM sockets on Linux and BSD-derived systems) or LwIP
 *  TCP protocol control blocks, as the system is configured accordingly.
 */
class DLL_EXPORT TCPEndPoint : public EndPointBasis
{
    friend class InetLayer;

public:
    /** Control switch indicating whether the application is receiving data. */
    bool ReceiveEnabled;

    /**
     * @brief   Basic dynamic state of the underlying endpoint.
     *
     * @details
     *  Objects are initialized in the "ready" state, proceed to subsequent
     *  states corresponding to a simplification of the states of the TCP
     *  transport state machine.
     *
     * @note
     *  The \c kBasisState_Closed state enumeration is mapped to \c kState_Ready for historical binary-compatibility reasons. The
     *  existing \c kState_Closed exists to identify separately the distinction between "not opened yet" and "previously opened now
     *  closed" that existed previously in the \c kState_Ready and \c kState_Closed states.
     */
    enum
    {
        kState_Ready           = kBasisState_Closed, /**< Endpoint initialized, but not bound. */
        kState_Bound           = 1,                  /**< Endpoint bound, but not listening. */
        kState_Listening       = 2,                  /**< Endpoint receiving connections. */
        kState_Connecting      = 3,                  /**< Endpoint attempting to connect. */
        kState_Connected       = 4,                  /**< Endpoint connected, ready for tx/rx. */
        kState_SendShutdown    = 5,                  /**< Endpoint initiated its half-close. */
        kState_ReceiveShutdown = 6,                  /**< Endpoint responded to half-close. */
        kState_Closing         = 7,                  /**< Endpoint closing bidirectionally. */
        kState_Closed          = 8                   /**< Endpoint closed, ready for release. */
    } State;

    /**
     * @brief   Bind the endpoint to an interface IP address.
     *
     * @param[in]   addrType    the protocol version of the IP address
     * @param[in]   addr        the IP address (must be an interface address)
     * @param[in]   port        the TCP port
     * @param[in]   reuseAddr   option to share binding with other endpoints
     *
     * @retval  INET_NO_ERROR               success: endpoint bound to address
     * @retval  INET_ERROR_INCORRECT_STATE  endpoint has been bound previously
     * @retval  INET_NO_MEMORY              insufficient memory for endpoint
     *
     * @retval  INET_ERROR_WRONG_PROTOCOL_TYPE
     *      \c addrType does not match \c IPVer.
     *
     * @retval  INET_ERROR_WRONG_ADDRESS_TYPE
     *      \c addrType is \c kIPAddressType_Any, or the type of \c addr is not
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
    INET_ERROR Bind(IPAddressType addrType, IPAddress addr, uint16_t port, bool reuseAddr = false);

    /**
     * @brief   Prepare the endpoint to receive TCP messages.
     *
     * @param[in]   backlog     maximum depth of connection acceptance queue
     *
     * @retval  INET_NO_ERROR   success: endpoint ready to receive messages.
     * @retval  INET_ERROR_INCORRECT_STATE  endpoint is already listening.
     *
     * @details
     *  If \c State is already \c kState_Listening, then no operation is
     *  performed, otherwise the \c State is set to \c kState_Listening and
     *  the endpoint is prepared to received TCP messages, according to the
     *  semantics of the platform.
     *
     *  On some platforms, the \c backlog argument is not used (the depth of
     *  the queue is fixed; only one connection may be accepted at a time).
     *
     *  On LwIP systems, this method must not be called with the LwIP stack
     *  lock already acquired
     */
    INET_ERROR Listen(uint16_t backlog);

    /**
     * @brief   Initiate a TCP connection.
     *
     * @param[in]   addr        the destination IP address
     * @param[in]   port        the destination TCP port
     * @param[in]   intf        an optional network interface indicator
     *
     * @retval  INET_NO_ERROR       success: \c msg is queued for transmit.
     * @retval  INET_ERROR_NOT_IMPLEMENTED  system implementation not complete.
     *
     * @retval  INET_ERROR_WRONG_ADDRESS_TYPE
     *      the destination address and the bound interface address do not
     *      have matching protocol versions or address type, or the destination
     *      address is an IPv6 link-local address and \c intf is not specified.
     *
     * @retval  other                   another system or platform error
     *
     * @details
     *      If possible, then this method initiates a TCP connection to the
     *      destination \c addr (with \c intf used as the scope
     *      identifier for IPv6 link-local destinations) and \c port.
     */
    INET_ERROR Connect(IPAddress addr, uint16_t port, InterfaceId intf = INET_NULL_INTERFACEID);

    /**
     * @brief   Extract IP address and TCP port of remote endpoint.
     *
     * @param[out]  retAddr     IP address of remote endpoint.
     * @param[out]  retPort     TCP port of remote endpoint.
     *
     * @retval  INET_NO_ERROR           success: address and port extracted.
     * @retval  INET_ERROR_INCORRECT_STATE  TCP connection not established.
     * @retval  INET_ERROR_CONNECTION_ABORTED   TCP connection no longer open.
     *
     * @details
     *  Do not use \c NULL pointer values for either argument.
     */
    INET_ERROR GetPeerInfo(IPAddress * retAddr, uint16_t * retPort) const;

    /**
     * @brief   Extract IP address and TCP port of local endpoint.
     *
     * @param[out]  retAddr     IP address of local endpoint.
     * @param[out]  retPort     TCP port of local endpoint.
     *
     * @retval  INET_NO_ERROR           success: address and port extracted.
     * @retval  INET_ERROR_INCORRECT_STATE  TCP connection not established.
     * @retval  INET_ERROR_CONNECTION_ABORTED   TCP connection no longer open.
     *
     * @details
     *  Do not use \c NULL pointer values for either argument.
     */
    INET_ERROR GetLocalInfo(IPAddress * retAddr, uint16_t * retPort);

    /**
     * @brief   Send message text on TCP connection.
     *
     * @param[out]  data    Message text to send.
     * @param[out]  push    If \c true, then send immediately, otherwise queue.
     *
     * @retval  INET_NO_ERROR           success: address and port extracted.
     * @retval  INET_ERROR_INCORRECT_STATE  TCP connection not established.
     *
     * @details
     *  The <tt>chip::System::PacketBuffer::Free</tt> method is called on the \c data argument
     *  regardless of whether the transmission is successful or failed.
     */
    INET_ERROR Send(chip::System::PacketBuffer * data, bool push = true);

    /**
     * @brief   Disable reception.
     *
     * @details
     *  Disable all event handlers. Data sent to an endpoint that disables
     *  reception will be acknowledged until the receive window is exhausted.
     */
    void DisableReceive(void);

    /**
     * @brief   Enable reception.
     *
     * @details
     *  Enable all event handlers. Data sent to an endpoint that disables
     *  reception will be acknowledged until the receive window is exhausted.
     */
    void EnableReceive(void);

    /**
     *  @brief EnableNoDelay
     */
    INET_ERROR EnableNoDelay(void);

    /**
     * @brief   Enable the TCP "keep-alive" option.
     *
     * @param[in]   interval        time in seconds between probe requests.
     * @param[in]   timeoutCount    number of probes to send before timeout.
     *
     * @retval  INET_NO_ERROR           success: address and port extracted.
     * @retval  INET_ERROR_INCORRECT_STATE  TCP connection not established.
     * @retval  INET_ERROR_CONNECTION_ABORTED   TCP connection no longer open.
     * @retval  INET_ERROR_NOT_IMPLEMENTED  system implementation not complete.
     *
     * @retval  other                   another system or platform error
     *
     * @details
     *  Start automatically  transmitting TCP "keep-alive" probe segments every
     *  \c interval seconds. The connection will abort automatically after
     *  receiving a negative response, or after sending \c timeoutCount
     *  probe segments without receiving a positive response.
     *
     *  See RFC 1122, section 4.2.3.6 for specification details.
     */
    INET_ERROR EnableKeepAlive(uint16_t interval, uint16_t timeoutCount);

    /**
     * @brief   Disable the TCP "keep-alive" option.
     *
     * @retval  INET_NO_ERROR           success: address and port extracted.
     * @retval  INET_ERROR_INCORRECT_STATE  TCP connection not established.
     * @retval  INET_ERROR_CONNECTION_ABORTED   TCP connection no longer open.
     * @retval  INET_ERROR_NOT_IMPLEMENTED  system implementation not complete.
     *
     * @retval  other                   another system or platform error
     */
    INET_ERROR DisableKeepAlive(void);

    /**
     * @brief   Set the TCP TCP_USER_TIMEOUT socket option.
     *
     * @param[in]   userTimeoutMillis    Tcp user timeout value in milliseconds.
     *
     * @retval  INET_NO_ERROR           success: address and port extracted.
     * @retval  INET_ERROR_NOT_IMPLEMENTED  system implementation not complete.
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
     */
    INET_ERROR SetUserTimeout(uint32_t userTimeoutMillis);

    /**
     * @brief   Acknowledge receipt of message text.
     *
     * @param[in]   len     number of bytes to acknowledge.
     *
     * @retval  INET_NO_ERROR           success: reception acknowledged.
     * @retval  INET_ERROR_INCORRECT_STATE  TCP connection not established.
     * @retval  INET_ERROR_CONNECTION_ABORTED   TCP connection no longer open.
     *
     * @details
     *  Use this method to acknowledge reception of all or part of the data
     *  received. The operational semantics are undefined if \c len is larger
     *  than the total outstanding unacknowledged received data.
     */
    INET_ERROR AckReceive(uint16_t len);

    /**
     * @brief   Push message text back to the head of the receive queue.
     *
     * @param[out]  data    Message text to push.
     *
     * @retval  INET_NO_ERROR           success: reception acknowledged.
     * @retval  INET_ERROR_INCORRECT_STATE  TCP connection not established.
     *
     * @details
     *  This method may only be called by data reception event handlers to
     *  put an unacknowledged portion of data back on the receive queue. The
     *  operational semantics are undefined if the caller is outside the scope
     *  of a data reception event handler, \c data is not the \c chip::System::PacketBuffer
     *  provided to the handler, or \c data does not contain the unacknowledged
     *  portion remaining after the bytes acknowledged by a prior call to the
     *  <tt>AckReceive(uint16_t len)</tt> method.
     */
    INET_ERROR PutBackReceivedData(chip::System::PacketBuffer * data);

    /**
     * @brief   Extract the length of the data awaiting first transmit.
     *
     * @return  Number of untransmitted bytes in the transmit queue.
     */
    uint32_t PendingSendLength(void);

    /**
     * @brief   Extract the length of the unacknowledged receive data.
     *
     * @return  Number of bytes in the receive queue that have not yet been
     *      acknowledged with <tt>AckReceive(uint16_t len)</tt>.
     */
    uint32_t PendingReceiveLength(void);

    /**
     * @brief   Initiate TCP half close, in other words, finished with sending.
     *
     * @retval  INET_NO_ERROR           success: address and port extracted.
     * @retval  INET_ERROR_INCORRECT_STATE  TCP connection not established.
     *
     * @retval  other                   another system or platform error
     */
    INET_ERROR Shutdown(void);

    /**
     * @brief   Initiate TCP full close, in other words, finished with both send and
     *  receive.
     *
     * @retval  INET_NO_ERROR           success: address and port extracted.
     * @retval  INET_ERROR_INCORRECT_STATE  TCP connection not established.
     *
     * @retval  other                   another system or platform error
     */
    INET_ERROR Close(void);

    /**
     * @brief   Abortively close the endpoint, in other words, send RST packets.
     */
    void Abort(void);

    /**
     * @brief   Initiate (or continue) TCP full close, ignoring errors.
     *
     * @details
     *  The object is returned to the free pool, and all remaining user
     *  references are subsequently invalid.
     */
    void Free(void);

    /**
     * @brief   Extract whether TCP connection is established.
     */
    bool IsConnected(void) const;

    void SetConnectTimeout(const uint32_t connTimeoutMsecs);

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
    void MarkActive(void);

    /**
     * @brief   Obtain an identifier for the endpoint.
     *
     * @return  Returns an opaque unique identifier for use logs.
     */
    uint16_t LogId(void);

    /**
     * @brief   Type of connection establishment event handling function.
     *
     * @param[in]   endPoint    The TCP endpoint associated with the event.
     * @param[in]   err         \c INET_NO_ERROR if success, else another code.
     *
     * @details
     *  Provide a function of this type to the \c OnConnectComplete delegate
     *  member to process connection establishment events on \c endPoint. The
     *  \c err argument distinguishes successful connections from failures.
     */
    typedef void (*OnConnectCompleteFunct)(TCPEndPoint * endPoint, INET_ERROR err);

    /**
     * The endpoint's connection establishment event handling function
     * delegate.
     */
    OnConnectCompleteFunct OnConnectComplete;

    /**
     * @brief   Type of data reception event handling function.
     *
     * @param[in]   endPoint    The TCP endpoint associated with the event.
     * @param[in]   data        The data received.
     *
     * @details
     *  Provide a function of this type to the \c OnDataReceived delegate
     *  member to process data reception events on \c endPoint where \c data
     *  is the message text received.
     *
     *  A data reception event handler must acknowledge data processed using
     *  the \c AckReceive method. The \c Free method on the data buffer must
     *  also be invoked unless the \c PutBackReceivedData is used instead.
     */
    typedef void (*OnDataReceivedFunct)(TCPEndPoint * endPoint, chip::System::PacketBuffer * data);

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
    typedef void (*OnDataSentFunct)(TCPEndPoint * endPoint, uint16_t len);

    /**
     * The endpoint's message text transmission event handling function
     * delegate.
     */
    OnDataSentFunct OnDataSent;

    /**
     * @brief   Type of connection establishment event handling function.
     *
     * @param[in]   endPoint    The TCP endpoint associated with the event.
     * @param[in]   err         \c INET_NO_ERROR if success, else another code.
     *
     * @details
     *  Provide a function of this type to the \c OnConnectionClosed delegate
     *  member to process connection termination events on \c endPoint. The
     *  \c err argument distinguishes successful terminations from failures.
     */
    typedef void (*OnConnectionClosedFunct)(TCPEndPoint * endPoint, INET_ERROR err);

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
    typedef void (*OnAcceptErrorFunct)(TCPEndPoint * endPoint, INET_ERROR err);

    /**
     * The endpoint's connection acceptance event handling function delegate.
     */
    OnAcceptErrorFunct OnAcceptError;

#if INET_CONFIG_ENABLE_TCP_SEND_IDLE_CALLBACKS
    /**
     * @brief   Type of TCP SendIdle changed signal handling function.
     *
     * @param[in]   endPoint    The TCP endpoint associated with the event.
     *
     * @param[in]   isIdle      True if the send channel of the TCP endpoint
     *                          is Idle, otherwise false.
     * @details
     *  Provide a function of this type to the \c OnTCPSendIdleChanged delegate
     *  member to process the event of the send channel of the TCPEndPoint
     *  changing state between being idle and not idle.
     */
    typedef void (*OnTCPSendIdleChangedFunct)(TCPEndPoint * endPoint, bool isIdle);

    /** The event handling function delegate of the endpoint signaling when the
     *  idleness of the TCP connection's send channel changes. This is utilized
     *  by upper layers to take appropriate actions based on whether sent data
     *  has been reliably delivered to the peer. */
    OnTCPSendIdleChangedFunct OnTCPSendIdleChanged;
#endif // INET_CONFIG_ENABLE_TCP_SEND_IDLE_CALLBACKS

private:
    static chip::System::ObjectPool<TCPEndPoint, INET_CONFIG_NUM_TCP_ENDPOINTS> sPool;

    chip::System::PacketBuffer * mRcvQueue;
    chip::System::PacketBuffer * mSendQueue;
#if INET_TCP_IDLE_CHECK_INTERVAL > 0
    uint16_t mIdleTimeout;       // in units of INET_TCP_IDLE_CHECK_INTERVAL; zero means no timeout
    uint16_t mRemainingIdleTime; // in units of INET_TCP_IDLE_CHECK_INTERVAL
#endif                           // INET_TCP_IDLE_CHECK_INTERVAL > 0

    uint32_t mConnectTimeoutMsecs; // This is the timeout to wait for a Connect call to succeed or
                                   // return an error; zero means use system defaults.

#if INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT
    uint32_t mUserTimeoutMillis; // The configured TCP user timeout value in milliseconds.
                                 // If 0, assume not set.
#if INET_CONFIG_ENABLE_TCP_SEND_IDLE_CALLBACKS
    bool mIsTCPSendIdle; // Indicates whether the send channel of the TCPEndPoint is Idle.

    uint16_t mTCPSendQueueRemainingPollCount; // The current remaining number of TCP SendQueue polls before
                                              // the TCP User timeout period is reached.

    uint32_t mTCPSendQueuePollPeriodMillis; // The configured period of active polling of the TCP
                                            // SendQueue. If 0, assume not set.
    void SetTCPSendIdleAndNotifyChange(bool aIsSendIdle);

#endif // INET_CONFIG_ENABLE_TCP_SEND_IDLE_CALLBACKS

    bool mUserTimeoutTimerRunning; // Indicates whether the TCP UserTimeout timer has been started.

    static void TCPUserTimeoutHandler(chip::System::Layer * aSystemLayer, void * aAppState, chip::System::Error aError);

    void StartTCPUserTimeoutTimer(void);

    void StopTCPUserTimeoutTimer(void);

    void RestartTCPUserTimeoutTimer(void);

    void ScheduleNextTCPUserTimeoutPoll(uint32_t aTimeOut);

#if INET_CONFIG_ENABLE_TCP_SEND_IDLE_CALLBACKS
    uint16_t MaxTCPSendQueuePolls(void);
#endif // INET_CONFIG_ENABLE_TCP_SEND_IDLE_CALLBACKS

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    uint32_t mBytesWrittenSinceLastProbe; // This counts the number of bytes written on the TCP socket since the
                                          // last probe into the TCP outqueue was made.

    uint32_t mLastTCPKernelSendQueueLen; // This is the measured size(in bytes) of the kernel TCP send queue
                                         // at the end of the last user timeout window.
    INET_ERROR CheckConnectionProgress(bool & IsProgressing);
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS

#endif // INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT

    TCPEndPoint(void);                // not defined
    TCPEndPoint(const TCPEndPoint &); // not defined
    ~TCPEndPoint(void);               // not defined

    void Init(InetLayer * inetLayer);
    INET_ERROR DriveSending(void);
    void DriveReceiving(void);
    void HandleConnectComplete(INET_ERROR err);
    void HandleAcceptError(INET_ERROR err);
    INET_ERROR DoClose(INET_ERROR err, bool suppressCallback);
    static bool IsConnected(int state);

    static void TCPConnectTimeoutHandler(chip::System::Layer * aSystemLayer, void * aAppState, chip::System::Error aError);

    void StartConnectTimerIfSet(void);
    void StopConnectTimer(void);

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    chip::System::PacketBuffer * mUnsentQueue;
    uint16_t mUnsentOffset;

    INET_ERROR GetPCB(IPAddressType addrType);
    void HandleDataSent(uint16_t len);
    void HandleDataReceived(chip::System::PacketBuffer * buf);
    void HandleIncomingConnection(TCPEndPoint * pcb);
    void HandleError(INET_ERROR err);

    static err_t LwIPHandleConnectComplete(void * arg, struct tcp_pcb * tpcb, err_t lwipErr);
    static err_t LwIPHandleIncomingConnection(void * arg, struct tcp_pcb * tcpConPCB, err_t lwipErr);
    static err_t LwIPHandleDataReceived(void * arg, struct tcp_pcb * tpcb, struct pbuf * p, err_t err);
    static err_t LwIPHandleDataSent(void * arg, struct tcp_pcb * tpcb, u16_t len);
    static void LwIPHandleError(void * arg, err_t err);

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS
    INET_ERROR GetSocket(IPAddressType addrType);
    SocketEvents PrepareIO(void);
    void HandlePendingIO(void);
    void ReceiveData(void);
    void HandleIncomingConnection(void);
    INET_ERROR BindSrcAddrFromIntf(IPAddressType addrType, InterfaceId intf);
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS
};

#if INET_CONFIG_ENABLE_TCP_SEND_IDLE_CALLBACKS && INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT
inline uint16_t TCPEndPoint::MaxTCPSendQueuePolls(void)
{
    // If the UserTimeout is configured less than or equal to the poll interval,
    // return 1 to poll at least once instead of returning zero and timing out
    // immediately.
    return (mUserTimeoutMillis > mTCPSendQueuePollPeriodMillis) ? (mUserTimeoutMillis / mTCPSendQueuePollPeriodMillis) : 1;
}
#endif // INET_CONFIG_ENABLE_TCP_SEND_IDLE_CALLBACKS && INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT

inline bool TCPEndPoint::IsConnected(void) const
{
    return IsConnected(State);
}

inline uint16_t TCPEndPoint::LogId(void)
{
    return static_cast<uint16_t>(reinterpret_cast<intptr_t>(this));
}

inline void TCPEndPoint::MarkActive(void)
{
#if INET_TCP_IDLE_CHECK_INTERVAL > 0
    mRemainingIdleTime = mIdleTimeout;
#endif // INET_TCP_IDLE_CHECK_INTERVAL > 0
}

} // namespace Inet
} // namespace chip

#endif // !defined(TCPENDPOINT_H)
