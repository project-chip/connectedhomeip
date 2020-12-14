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
#include <transport/TransportMgr.h>
#include <transport/raw/Base.h>
#include <transport/raw/Tuple.h>

namespace chip {

class SecureSessionMgr;

class SecureSessionHandle
{
public:
    SecureSessionHandle() : mPeerNodeId(kAnyNodeId), mPeerKeyId(0) {}
    SecureSessionHandle(NodeId peerNodeId, uint16_t peerKeyId) : mPeerNodeId(peerNodeId), mPeerKeyId(peerKeyId) {}

    bool operator==(const SecureSessionHandle & that) const
    {
        return mPeerNodeId == that.mPeerNodeId && mPeerKeyId == that.mPeerKeyId;
    }

private:
    friend class SecureSessionMgr;
    NodeId mPeerNodeId;
    uint16_t mPeerKeyId;
};

/**
 * @brief
 *   This class provides a skeleton for the callback functions. The functions will be
 *   called by SecureSssionMgrBase object on specific events. If the user of SecureSessionMgr
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
     * @param session       The handle to the secure session
     * @param msgBuf        The received message
     * @param mgr           A pointer to the SecureSessionMgr
     */
    virtual void OnMessageReceived(const PacketHeader & packetHeader, const PayloadHeader & payloadHeader,
                                   SecureSessionHandle session, System::PacketBufferHandle msgBuf, SecureSessionMgr * mgr)
    {}

    /**
     * @brief
     *   Called when received message processing resulted in error
     *
     * @param error   error code
     * @param source  network entity that sent the message
     * @param mgr     A pointer to the SecureSessionMgr
     */
    virtual void OnReceiveError(CHIP_ERROR error, const Transport::PeerAddress & source, SecureSessionMgr * mgr) {}

    /**
     * @brief
     *   Called when a new pairing is being established
     *
     * @param session The handle to the secure session
     * @param mgr     A pointer to the SecureSessionMgr
     */
    virtual void OnNewConnection(SecureSessionHandle session, SecureSessionMgr * mgr) {}

    /**
     * @brief
     *   Called when a new connection is closing
     *
     * @param session The handle to the secure session
     * @param mgr     A pointer to the SecureSessionMgr
     */
    virtual void OnConnectionExpired(SecureSessionHandle session, SecureSessionMgr * mgr) {}

    /**
     * @brief
     *   Called when the peer address is resolved from NodeID.
     *
     * @param error   The resolution resolve error code
     * @param nodeId  The node ID resolved, 0 on error
     * @param mgr     A pointer to the SecureSessionMgr
     */
    virtual void OnAddressResolved(CHIP_ERROR error, NodeId nodeId, SecureSessionMgr * mgr) {}

    virtual ~SecureSessionMgrDelegate() {}
};

class DLL_EXPORT SecureSessionMgr : public Mdns::ResolveDelegate, public TransportMgrDelegate
{
public:
    /**
     * @brief
     *   Send a message to a currently connected peer
     */
    CHIP_ERROR SendMessage(SecureSessionHandle session, System::PacketBufferHandle msgBuf);
    CHIP_ERROR SendMessage(SecureSessionHandle session, PayloadHeader & payloadHeader, System::PacketBufferHandle msgBuf);
    Transport::PeerConnectionState * GetPeerConnectionState(SecureSessionHandle session);
    SecureSessionMgr();
    ~SecureSessionMgr() override;

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

    /**
     * @brief
     *   Initialize a Secure Session Manager
     *
     * @param localNodeId    Node id for the current node
     * @param systemLayer    System, layer to use
     * @param transportMgr   Transport to use
     */
    CHIP_ERROR Init(NodeId localNodeId, System::Layer * systemLayer, TransportMgrBase * transportMgr);

protected:
    /**
     * @brief
     *   Handle received secure message. Implements TransportMgrDelegate
     *
     * @param header    the received message header
     * @param source    the source address of the package
     * @param msgBuf    the buffer of (encrypted) payload
     */
    void OnMessageReceived(const PacketHeader & header, const Transport::PeerAddress & source,
                           System::PacketBufferHandle msgBuf) override;

private:
    /**
     *    The State of a secure transport object.
     */
    enum class State
    {
        kNotReady,    /**< State before initialization. */
        kInitialized, /**< State when the object is ready connect to other peers. */
    };

    System::Layer * mSystemLayer = nullptr;
    NodeId mLocalNodeId;                                                                // < Id of the current node
    Transport::PeerConnections<CHIP_CONFIG_PEER_CONNECTION_POOL_SIZE> mPeerConnections; // < Active connections to other peers
    State mState;                                                                       // < Initialization state of the object

    SecureSessionMgrDelegate * mCB   = nullptr;
    TransportMgrBase * mTransportMgr = nullptr;

    /** Schedules a new oneshot timer for checking connection expiry. */
    void ScheduleExpiryTimer();

    /** Cancels any active timers for connection expiry checks. */
    void CancelExpiryTimer();

    /**
     * Called when a specific connection expires.
     */
    void HandleConnectionExpired(const Transport::PeerConnectionState & state);

    /**
     * Callback for timer expiry check
     */
    static void ExpiryTimerCallback(System::Layer * layer, void * param, System::Error error);

    void HandleNodeIdResolve(CHIP_ERROR error, NodeId nodeId, const Mdns::MdnsService & service) override;
};

} // namespace chip
