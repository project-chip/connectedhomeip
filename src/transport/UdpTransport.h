/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    All rights reserved.
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
 *      This file defines the CHIP Connection object that maintains a UDP connection.
 *      It binds to any avaiable local addr and port and begins listening.
 *      TODO This class should be extended to support TCP as well...
 *
 */

#ifndef __UDPTRANSPORT_H__
#define __UDPTRANSPORT_H__

#include <utility>

#include <core/CHIPCore.h>
#include <inet/IPAddress.h>
#include <inet/IPEndPointBasis.h>

namespace chip {

using namespace System;

class DLL_EXPORT UdpTransport
{
public:
    /**
     *  @enum State
     *
     *  @brief
     *    The State of the CHIP connection object.
     *
     */
    enum State
    {
        // TODO need more modes when TCP support is added
        kState_NotReady       = 0, /**< State before initialization. */
        kState_ReadyToConnect = 1, /**< State after initialization of the CHIP connection. */
        kState_Connected      = 2, /**< State when the connection has been established. */
        kState_Closed         = 3  /**< State when the connection is closed. */
    };

    /**
     * @brief
     *   Initialize a CHIP Connection
     *
     * @param inetLayer     A pointer to the <tt>chip::Inet::InetLayer</tt>
     */
    void Init(Inet::InetLayer * inetLayer);

    /**
     * @brief
     *   Attempt to establish a connection to the given peer
     *
     * @param peerAddr      The <tt>chip::Inet::IPAddress</tt> of the requested peer
     * @param peerPort      The port of the requested peer
     * @return CHIP_ERROR   The connection result
     */
    CHIP_ERROR Connect(const IPAddress & peerAddr, uint16_t peerPort = 0);

    /**
     * @brief
     *   Send a message to the currently connected peer
     *
     * @param msgBuf        A PacketBuffer containing the message to be sent
     * @return CHIP_ERROR   The send result
     *
     * @details
     *   This method calls <tt>chip::System::PacketBuffer::Free</tt> on
     *   behalf of the caller regardless of the return status.
     */
    CHIP_ERROR SendMessage(PacketBuffer * msgBuf);

    /**
     * @brief
     *   Close an existing connection. Once close is called, the UdpTransport object can no longer be used
     *
     * @return CHIP_ERROR   The close result
     */
    CHIP_ERROR Close(void);

    void Retain(void);
    void Release(void);

    /**
     *  This function is the application callback that is invoked when a message is received over a
     *  Chip connection.
     *
     *  @param[in]    con           A pointer to the UdpTransport object.
     *
     *  @param[in]    msgBuf        A pointer to the PacketBuffer object holding the message.
     *
     *  @param[in]    pktInfo       A pointer to the IPPacketInfo object carrying sender details.
     *
     */
    typedef void (*MessageReceiveHandler)(UdpTransport * con, PacketBuffer * msgBuf, const IPPacketInfo * pktInfo);
    MessageReceiveHandler OnMessageReceived;

    /**
     *  This function is the application callback invoked upon encountering an error when receiving
     *  a Chip message.
     *
     *  @param[in]     con            A pointer to the UdpTransport object.
     *
     *  @param[in]     err            The CHIP_ERROR encountered when receiving data over the connection.
     *
     *  @param[in]    pktInfo         A pointer to the IPPacketInfo object carrying sender details.
     *
     */
    typedef void (*ReceiveErrorHandler)(UdpTransport * con, CHIP_ERROR err, const IPPacketInfo * pktInfo);
    ReceiveErrorHandler OnReceiveError;

    UdpTransport();
    virtual ~UdpTransport() {}

private:
    Inet::InetLayer * mInetLayer;
    UDPEndPoint * mUDPEndPoint;
    IPAddress mPeerAddr;
    uint16_t mPeerPort;
    State mState;
    uint8_t mRefCount;

    CHIP_ERROR DoConnect();
    void DoClose(CHIP_ERROR err);
    bool StateAllowsSend(void) const { return mState == kState_Connected; }
    bool StateAllowsReceive(void) const { return mState == kState_Connected; }

    static void HandleDataReceived(IPEndPointBasis * endPoint, chip::System::PacketBuffer * msg, const IPPacketInfo * pktInfo);
    static void HandleReceiveError(IPEndPointBasis * endPoint, INET_ERROR err, const IPPacketInfo * pktInfo);
};

/// Associates a UDP transport with a state at creation time
template <typename StateType>
class StatefulUdpTransport : public UdpTransport
{
public:
    StatefulUdpTransport(const StateType & state) : mState(state) {}
    StatefulUdpTransport(StateType && state) : mState(std::move(state)) {}

    StateType & State(void) { return mState; }
    const StateType & State(void) const { return mState; }

    /// Typesafe equivalent of UdpTransport::MessageReceivehandler
    typedef void (*StatefulMessageReceiveHandler)(StatefulUdpTransport * con, PacketBuffer * msgBuf, const IPPacketInfo * pktInfo);

    /// Typesafe equivalent of UdpTransport::ReceiveErrorHandler
    typedef void (*StatefulReceiveErrorHandler)(StatefulUdpTransport * con, CHIP_ERROR err, const IPPacketInfo * pktInfo);

    /// Sets the OnMessageReceived callback using a stateful callback
    void SetMessageReceiveHandler(StatefulMessageReceiveHandler handler)
    {
        OnMessageReceived = reinterpret_cast<MessageReceiveHandler>(handler);
    }

    /// Sets the OnMessageReceived callback using a stateful callback
    void SetReceiveErrorHandler(StatefulReceiveErrorHandler handler)
    {
        OnReceiveError = reinterpret_cast<ReceiveErrorHandler>(handler);
    }

private:
    StateType mState; ///< State for this transport. Often a pointer.
};

} // namespace chip

#endif // __UDPTRANSPORT_H__
