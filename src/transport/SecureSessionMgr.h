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

class DLL_EXPORT SecureSessionMgrCallback : public ReferenceCounted<SecureSessionMgrCallback>
{
public:
    /**
     * @brief
     *   Called when a new message is received
     *
     * @param header  messageheader
     * @param state connection state
     * @param msgBuf received message
     */
    virtual void OnMessageReceived(const MessageHeader & header, Transport::PeerConnectionState * state,
                                   System::PacketBuffer * msgBuf)
    {}

    /**
     * @brief
     *   Called when received message processing resulted in error
     *
     * @param error error code
     * @param source network entity that sent the message
     */
    virtual void OnReceiveError(CHIP_ERROR error, const Inet::IPPacketInfo & source) {}

    /**
     * @brief
     *   Called when a new connection is being established
     *
     * @param state connection state
     */
    virtual void OnNewConnection(Transport::PeerConnectionState * state) {}
};

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
     * @brief
     *   Set the callback object.
     *
     * @details
     *   Release if there was an existing callback object
     */
    void SetDelegate(SecureSessionMgrCallback * cb)
    {
        if (mCB != nullptr)
        {
            mCB->Release();
        }
        mCB = cb->Retain();
    }

private:
    // TODO: add support for multiple transports (TCP, BLE to be added)
    Transport::UDP mTransport;

    NodeId mLocalNodeId;                                                                //< Id of the current node
    Transport::PeerConnections<CHIP_CONFIG_PEER_CONNECTION_POOL_SIZE> mPeerConnections; //< Active connections to other peers
    State mState;                                                                       //< Initialization state of the object

    SecureSessionMgrCallback * mCB = nullptr;

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
