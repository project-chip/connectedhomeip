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

#ifndef __CHIPCONNECTION_H__
#define __CHIPCONNECTION_H__

#include <core/CHIPCore.h>
#include <inet/IPAddress.h>
#include <inet/IPEndPointBasis.h>

namespace chip {

using namespace System;

class DLL_EXPORT ChipConnection
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
        kState_ReadyToConnect = 0, /**< State after initialization of the CHIP connection. */
        kState_Connected      = 1, /**< State when the connection has been established. */
        kState_Closed         = 2  /**< State when the connection is closed. */
    };

    void * AppState; /**< A pointer to the application-specific state object. */

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
     * @param peerNodeId    Currently unused; a NodeId to identify this peer
     * @param peerAddr      The <tt>chip::Inet::IPAddress</tt> of the requested peer
     * @param peerPort      The port of the requested peer
     * @return CHIP_ERROR   The connection result
     */
    CHIP_ERROR Connect(uint64_t peerNodeId, const IPAddress & peerAddr, uint16_t peerPort = 0);

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
     *   Close an existing connection. This allows reusing this Connection object.
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
     *  @param[in]    con           A pointer to the ChipConnection object.
     *
     *  @param[in]    msgBuf        A pointer to the PacketBuffer object holding the message.
     *
     *  @param[in]    pktInfo       A pointer to the IPPacketInfo object carrying sender details.
     *
     */
    typedef void (*MessageReceiveHandler)(ChipConnection * con, PacketBuffer * msgBuf, const IPPacketInfo * pktInfo);
    MessageReceiveHandler OnMessageReceived;

    /**
     *  This function is the application callback invoked upon encountering an error when receiving
     *  a Chip message.
     *
     *  @param[in]     con            A pointer to the ChipConnection object.
     *
     *  @param[in]     err            The CHIP_ERROR encountered when receiving data over the connection.
     *
     *  @param[in]    pktInfo         A pointer to the IPPacketInfo object carrying sender details.
     *
     */
    typedef void (*ReceiveErrorHandler)(ChipConnection * con, CHIP_ERROR err, const IPPacketInfo * pktInfo);
    ReceiveErrorHandler OnReceiveError;

private:
    Inet::InetLayer * mInetLayer;
    UDPEndPoint * mUDPEndPoint;
    uint64_t mPeerNodeId;
    IPAddress mPeerAddr;
    uint16_t mPeerPort;
    uint8_t mState;
    uint8_t mRefCount;

    CHIP_ERROR DoConnect();
    void DoClose(CHIP_ERROR err);
    bool StateAllowsSend(void) const { return mState == kState_Connected; }
    bool StateAllowsReceive(void) const { return mState == kState_Connected; }

    static void HandleDataReceived(IPEndPointBasis * endPoint, chip::System::PacketBuffer * msg, const IPPacketInfo * pktInfo);
    static void HandleReceiveError(IPEndPointBasis * endPoint, INET_ERROR err, const IPPacketInfo * pktInfo);
};

} // namespace chip

#endif // __CHIPCONNECTION_H__