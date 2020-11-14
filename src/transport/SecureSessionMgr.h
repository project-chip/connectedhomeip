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

#pragma once

#include <utility>

#include <core/CHIPCore.h>
#include <inet/IPAddress.h>
#include <inet/IPEndPointBasis.h>
#include <lib/mdns/DiscoveryManager.h>
#include <support/CodeUtils.h>
#include <support/DLLUtil.h>
#include <transport/PeerConnections.h>
#include <transport/SecurePairingSession.h>
#include <transport/SecureSession.h>
#include <transport/raw/Base.h>
#include <transport/raw/Tuple.h>

namespace chip {

class SecureSessionMgrBase;

/**
 * @brief
 *   This class provides a skeleton for the callback functions. The functions will be
 *   called by SecureSssionMgrBase object on specific events. If the user of SecureSessionMgrBase
 *   is interested in receiving these callbacks, they can specialize this class and handle
 *   each trigger in their implementation of this class.
 */
class DLL_EXPORT SecureSessionMgrDelegate
{
public:
    /**
     * @brief
     *   Called when a new message is received. The function must internally release the
     *   msgBuf after processing it.
     *
     * @param packetHeader  The message header
     * @param payloadHeader The payload header
     * @param state         The connection state
     * @param msgBuf        The received message
     * @param mgr           A pointer to the SecureSessionMgr
     */
    virtual void OnMessageReceived(const PacketHeader & packetHeader, const PayloadHeader & payloadHeader,
                                   Transport::PeerConnectionState * state, System::PacketBuffer * msgBuf,
                                   SecureSessionMgrBase * mgr)
    {}

    /**
     * @brief
     *   Called when received message processing resulted in error
     *
     * @param error   error code
     * @param source  network entity that sent the message
     * @param mgr     A pointer to the SecureSessionMgr
     */
    virtual void OnReceiveError(CHIP_ERROR error, const Transport::PeerAddress & source, SecureSessionMgrBase * mgr) {}

    /**
     * @brief
     *   Called when a new pairing is being established
     *
     * @param state   connection state
     * @param mgr     A pointer to the SecureSessionMgr
     */
    virtual void OnNewConnection(Transport::PeerConnectionState * state, SecureSessionMgrBase * mgr) {}

    /**
     * @brief
     *   Called when the peer address is resolved from NodeID.
     *
     * @param error   The resolution resolve error code
     * @param nodeId  The node ID resolved, 0 on error
     * @param mgr     A pointer to the SecureSessionMgr
     */
    virtual void OnAddressResolved(CHIP_ERROR error, NodeId nodeId, SecureSessionMgrBase * mgr) {}

    virtual ~SecureSessionMgrDelegate() {}
};

class DLL_EXPORT SecureSessionMgrBase : public Mdns::ResolveDelegate
{
public:
    /**
     * @brief
     *   Send a message to a currently connected peer
     *
     * @details
     *   This method calls <tt>chip::System::PacketBuffer::Free</tt> on
     *   behalf of the caller regardless of the return status.
     */
    CHIP_ERROR SendMessage(NodeId peerNodeId, System::PacketBuffer * msgBuf);
    CHIP_ERROR SendMessage(PayloadHeader & payloadHeader, NodeId peerNodeId, System::PacketBuffer * msgBuf);
    SecureSessionMgrBase();
    ~SecureSessionMgrBase() override;

    /**
     * @brief
     *   Set the callback object.
     *
     * @details
     *   Release if there was an existing callback object
     */
    void SetDelegate(SecureSessionMgrDelegate * cb) { mCB = cb; }

    /**
     * @brief
     *   Establish a new pairing with a peer node
     *
     * @details
     *   This method sets up a new pairing with the peer node. It also
     *   establishes the security keys for secure communication with the
     *   peer node.
     */
    CHIP_ERROR NewPairing(const Optional<Transport::PeerAddress> & peerAddr, NodeId peerNodeId, SecurePairingSession * pairing);

    /**
     * @brief
     *   Return the System Layer pointer used by current SecureSessionMgr.
     */
    System::Layer * SystemLayer() { return mSystemLayer; }

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

    SecureSessionMgrDelegate * mCB = nullptr;

    /** Schedules a new oneshot timer for checking connection expiry. */
    void ScheduleExpiryTimer();

    /** Cancels any active timers for connection expiry checks. */
    void CancelExpiryTimer();

    static void HandleDataReceived(const PacketHeader & header, const Transport::PeerAddress & source,
                                   System::PacketBuffer * msgBuf, SecureSessionMgrBase * transport);

    /**
     * Called when a specific connection expires.
     */
    static void HandleConnectionExpired(const Transport::PeerConnectionState & state, SecureSessionMgrBase * mgr);

    /**
     * Callback for timer expiry check
     */
    static void ExpiryTimerCallback(System::Layer * layer, void * param, System::Error error);

    void HandleNodeIdResolve(CHIP_ERROR error, NodeId nodeId, const Mdns::MdnsService & service) override;
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

    template <typename... Args>
    CHIP_ERROR ResetTransport(Args &&... transportInitArgs)
    {
        return mTransport.Init(std::forward<Args>(transportInitArgs)...);
    }

private:
    Transport::Tuple<TransportTypes...> mTransport;
};

} // namespace chip
