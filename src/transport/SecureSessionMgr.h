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
#include <transport/Base.h>
#include <transport/PeerConnections.h>
#include <transport/SecureSession.h>
#include <transport/Tuple.h>

namespace chip {

using namespace System;

class SecureSessionMgrBase;

/**
 * @brief
 *   This class provides a skeleton for the callback functions. The functions will be
 *   called by SecureSssionMgrBase object on specific events. If the user of SecureSessionMgrBase
 *   is interested in receiving these callbacks, they can specialize this class and handle
 *   each trigger in their implementation of this class.
 */
class DLL_EXPORT SecureSessionMgrCallback : public ReferenceCounted<SecureSessionMgrCallback>
{
public:
    /**
     * @brief
     *   Called when a new message is received. The function must internally release the
     *   msgBuf after processing it.
     *
     * @param header  messageheader
     * @param state connection state
     * @param msgBuf received message
     */
    virtual void OnMessageReceived(const MessageHeader & header, Transport::PeerConnectionState * state,
                                   System::PacketBuffer * msgBuf, SecureSessionMgrBase * mgr)
    {}

    /**
     * @brief
     *   Called when received message processing resulted in error
     *
     * @param error error code
     * @param source network entity that sent the message
     */
    virtual void OnReceiveError(CHIP_ERROR error, const Transport::PeerAddress & source, SecureSessionMgrBase * mgr) {}

    /**
     * @brief
     *   Called when a new connection is being established
     *
     * @param state connection state
     */
    virtual void OnNewConnection(Transport::PeerConnectionState * state, SecureSessionMgrBase * mgr) {}

    virtual ~SecureSessionMgrCallback() {}
};

class DLL_EXPORT SecureSessionMgrBase : public ReferenceCounted<SecureSessionMgrBase>
{
public:
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

    SecureSessionMgrBase();
    virtual ~SecureSessionMgrBase();

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

protected:
    /**
     * @brief
     *   Initialize a Secure Session Manager
     *
     * @param localNodeId    Node id for the current node
     * @param systemLayer    System, layer to use
     * @param transport Underlying Transport to use
     */
    CHIP_ERROR InitInternal(NodeId localNodeId, System::Layer * systemLayer, Transport::Base * transport);

private:
    /**
     *    The State of a secure transport object.
     */
    enum class State
    {
        kNotReady,    /**< State before initialization. */
        kInitialized, /**< State when the object is ready connect to other peers. */
    };

    Transport::Base * mTransport = nullptr;
    System::Layer * mSystemLayer = nullptr;
    NodeId mLocalNodeId;                                                                // < Id of the current node
    Transport::PeerConnections<CHIP_CONFIG_PEER_CONNECTION_POOL_SIZE> mPeerConnections; // < Active connections to other peers
    State mState;                                                                       // < Initialization state of the object

    SecureSessionMgrCallback * mCB = nullptr;

    /** Schedules a new oneshot timer for checking connection expiry. */
    void ScheduleExpiryTimer(void);

    /** Cancels any active timers for connection expiry checks. */
    void CancelExpiryTimer(void);

    /**
     * Allocates a new connection for the given source.
     *
     * @param header The header that was received when the connection was established
     * @param address Where the connection originated from
     * @param state [out] the newly allocated connection state for the connection
     */
    CHIP_ERROR AllocateNewConnection(const MessageHeader & header, const Transport::PeerAddress & address,
                                     Transport::PeerConnectionState ** state);

    static void HandleDataReceived(MessageHeader & header, const Transport::PeerAddress & source, System::PacketBuffer * msgBuf,
                                   SecureSessionMgrBase * transport);

    /**
     * Called when a specific connection expires.
     */
    static void HandleConnectionExpired(const Transport::PeerConnectionState & state, SecureSessionMgrBase * mgr);

    /**
     * Callback for timer expiry check
     */
    static void ExpiryTimerCallback(System::Layer * layer, void * param, System::Error error);
};

/**
 * A secure session manager that includes required underlying transports.
 */
template <typename... TransportTypes>
class SecureSessionMgr : public SecureSessionMgrBase
{
public:
    /**
     * @brief
     *   Initialize a Secure Session Manager
     *
     * @param localNodeId    Node id for the current node
     * @param systemLayer    System, layer to use
     * @param transportInitArgs Arguments to initialize the underlying transport
     */
    template <typename... Args>
    CHIP_ERROR Init(NodeId localNodeId, System::Layer * systemLayer, Args &&... transportInitArgs)
    {
        CHIP_ERROR err = CHIP_NO_ERROR;

        err = mTransport.Init(std::forward<Args>(transportInitArgs)...);
        SuccessOrExit(err);

        err = InitInternal(localNodeId, systemLayer, &mTransport);
        SuccessOrExit(err);

    exit:
        return err;
    }

private:
    Transport::Tuple<TransportTypes...> mTransport;
};

} // namespace chip

#endif // __SECURESESSIONMGR_H__
