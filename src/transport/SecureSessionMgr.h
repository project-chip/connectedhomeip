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
 * @file
 *   This file defines a secure transport layer which adds encryption to data
 *   sent over a transport.
 *
 */

#ifndef __SECURESESSIONMGR_H__
#define __SECURESESSIONMGR_H__

#include <utility>

#include <core/CHIPCore.h>
#include <core/ReferenceCounted.h>
#include <inet/IPAddress.h>
#include <inet/IPEndPointBasis.h>
#include <transport/PeerConnections.h>
#include <transport/SecureSession.h>
#include <transport/UDP.h>

namespace chip {

using namespace System;

class DLL_EXPORT SecureSessionMgr : public ReferenceCounted<SecureSessionMgr>
{
public:
    /**
     *    The State of a secure transport object.
     */
    enum class State
    {
        kNotReady,    /**< State before initialization. */
        kInitialized, /**< State when the object is ready connect to other peers. */
    };

    /**
     * @brief
     *   Initialize a Secure Transport
     *
     * @param inet  Inet layer to use
     * @param listenParams  Listen settings for the transport
     *
     * @note This is not a final API as it is UDP specific. Class will be updated to support
     * separate Transports (UDP, BLE, TCP, optional ipv4 for testing etc.). This API is currently
     * UDP-specific and that will change.
     */
    CHIP_ERROR Init(NodeId localNodeId, Inet::InetLayer * inet, const Transport::UdpListenParameters & listenParams);

    /**
     * Establishes a connection to the given peer node.
     *
     * A connection needs to be established before SendMessage can be called.
     */
    CHIP_ERROR Connect(NodeId peerNodeId, const Transport::PeerAddress & peerAddress);

    /**
     * @brief
     *   Send a message to a currently connected peer
     *
     * @details
     *   This method calls <tt>chip::System::PacketBuffer::Free</tt> on
     *   behalf of the caller regardless of the return status.
     */
    CHIP_ERROR SendMessage(NodeId peerNodeId, System::PacketBuffer * msgBuf);

    SecureSessionMgr();
    virtual ~SecureSessionMgr() {}

    /**
     * Sets the message receive handler and associated argument
     *
     * @param[in] handler The callback to call when a message is received
     * @param[in] param   The argument to pass in to the handler function
     *
     */
    template <class T>
    void SetMessageReceiveHandler(void (*handler)(const MessageHeader &, Transport::PeerConnectionState *, System::PacketBuffer *,
                                                  T *),
                                  T * param)
    {
        mMessageReceivedArgument = param;
        OnMessageReceived        = reinterpret_cast<MessageReceiveHandler>(handler);
    }

    /**
     * Sets the receive error handler and associated argument
     *
     * @param[in] handler The callback to call on receive error
     *
     */
    void SetReceiveErrorHandler(void (*handler)(CHIP_ERROR, const Inet::IPPacketInfo &))
    {
        OnReceiveError = reinterpret_cast<ReceiveErrorHandler>(handler);
    }

    /**
     * Sets the new connection handler and associated argument
     *
     * @param[in] handler The callback to call when a message is received
     * @param[in] param   The argument to pass in to the handler function
     *
     */
    template <class T>
    void SetNewConnectionHandler(void (*handler)(Transport::PeerConnectionState *, T *), T * param)
    {
        mNewConnectionArgument = param;
        OnNewConnection        = reinterpret_cast<NewConnectionHandler>(handler);
    }

private:
    // TODO: add support for multiple transports (TCP, BLE to be added)
    Transport::UDP mTransport;

    NodeId mLocalNodeId;                                                                //< Id of the current node
    Transport::PeerConnections<CHIP_CONFIG_PEER_CONNECTION_POOL_SIZE> mPeerConnections; //< Active connections to other peers
    State mState;                                                                       //< Initialization state of the object

    /**
     * This function is the application callback that is invoked when a message is received over a
     * Chip connection.
     *
     * @param[in]    msgBuf        A pointer to the PacketBuffer object holding the message.
     */
    typedef void (*MessageReceiveHandler)(const MessageHeader & header, Transport::PeerConnectionState * state,
                                          System::PacketBuffer * msgBuf, void * param);

    MessageReceiveHandler OnMessageReceived = nullptr; ///< Callback on message receiving
    void * mMessageReceivedArgument         = nullptr; ///< Argument for callback

    typedef void (*ReceiveErrorHandler)(CHIP_ERROR error, const Inet::IPPacketInfo & source);

    ReceiveErrorHandler OnReceiveError = nullptr; ///< Callback on error in message receiving

    typedef void (*NewConnectionHandler)(Transport::PeerConnectionState * state, void * param);

    NewConnectionHandler OnNewConnection = nullptr; ///< Callback for new connection received
    void * mNewConnectionArgument        = nullptr; ///< Argument for callback

    /**
     * Allocates a new connection for the given source.
     *
     * @param header The header that was received when the connection was established
     * @param address Where the connection originated from
     * @param state [out] the newly allocated connection state for the connection
     */
    CHIP_ERROR AllocateNewConnection(const MessageHeader & header, const Transport::PeerAddress & address,
                                     Transport::PeerConnectionState ** state);

    /**
     * Handle UDP data receiving. Each transport has separate data receiving as active sessions
     * follow data receiving channels.
     *
     */
    static void HandleUdpDataReceived(const MessageHeader & header, const Inet::IPPacketInfo & source,
                                      System::PacketBuffer * msgBuf, SecureSessionMgr * transport);
};

} // namespace chip

#endif // __SECURESESSIONMGR_H__
