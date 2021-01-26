/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      This file defines the CHIP RendezvousSession object that maintains a Rendezvous session.
 *
 */
#pragma once

#include <core/CHIPCore.h>
#include <protocols/Protocols.h>
#include <support/BufBound.h>
#include <transport/NetworkProvisioning.h>
#include <transport/PASESession.h>
#include <transport/RendezvousParameters.h>
#include <transport/RendezvousSessionDelegate.h>
#include <transport/TransportMgr.h>
#include <transport/raw/MessageHeader.h>
#include <transport/raw/PeerAddress.h>
namespace chip {

namespace DeviceLayer {
class CHIPDeviceEvent;
}

class SecureSessionMgr;
class SecureSessionHandle;

/**
 * RendezvousSession establishes and maintains the first connection between
 * a commissioner and a device. This connection is used in order to
 * provide the necessary infos for a device to participate to the CHIP
 * ecosystem.
 *
 * All the information transmitted over the underlying transport are
 * encrypted upon establishment of an initial secure pairing session.
 *
 * In order to securely transmit the informations, RendezvousSession
 * requires a setupPINCode to be shared between both ends. The
 * setupPINCode can be configured using RendezvousParameters
 *
 * @dotfile dots/Rendezvous/RendezvousSessionGeneral.dot
 *
 * The state of the secure pairing session setup can be observed by passing a
 * RendezvousSessionDelegate object to RendezvousSession.
 * Both the commissioner and the device needs to bootstrap RendezvousSession
 * using RendezvousParameters.
 *
 * @dotfile dots/Rendezvous/RendezvousSessionInit.dot
 */
class RendezvousSession : public SessionEstablishmentDelegate,
                          public RendezvousSessionDelegate,
                          public RendezvousDeviceCredentialsDelegate,
                          public NetworkProvisioningDelegate,
                          public TransportMgrDelegate
{
public:
    enum State : uint8_t
    {
        kInit = 0,
        kSecurePairing,
        kNetworkProvisioning,
        kRendezvousComplete,
    };

    RendezvousSession(RendezvousSessionDelegate * delegate) : mDelegate(delegate) {}
    ~RendezvousSession() override;

    /**
     * @brief
     *  Initialize the underlying transport using the RendezvousParameters passed in the constructor.
     *
     * @param params       The RendezvousParameters
     * @param transportMgr The transport to use
     * @param sessionMgr   Pointer to secure session manager
     * @ return CHIP_ERROR  The result of the initialization
     */
    CHIP_ERROR Init(const RendezvousParameters & params, TransportMgrBase * transportMgr, SecureSessionMgr * sessionMgr);

    /**
     * @brief
     *  Return the associated pairing session.
     *
     * @return PASESession The associated pairing session
     */
    PASESession & GetPairingSession() { return mPairingSession; }

    Optional<NodeId> GetLocalNodeId() const { return mParams.GetLocalNodeId(); }
    Optional<NodeId> GetRemoteNodeId() const { return mParams.GetRemoteNodeId(); }

    //////////// SessionEstablishmentDelegate Implementation ///////////////
    CHIP_ERROR SendSessionEstablishmentMessage(const PacketHeader & header, const Transport::PeerAddress & peerAddress,
                                               System::PacketBufferHandle msgBuf) override;
    void OnSessionEstablishmentError(CHIP_ERROR err) override;
    void OnSessionEstablished() override;

    //////////// RendezvousSessionDelegate Implementation ///////////////
    void OnRendezvousConnectionOpened() override;
    void OnRendezvousConnectionClosed() override;
    void OnRendezvousError(CHIP_ERROR err) override;
    void OnRendezvousMessageReceived(const PacketHeader & packetHeader, const Transport::PeerAddress & peerAddress,
                                     System::PacketBufferHandle buffer) override;

    //////////// RendezvousDeviceCredentialsDelegate Implementation ///////////////
    void SendNetworkCredentials(const char * ssid, const char * passwd) override;
    void SendThreadCredentials(const DeviceLayer::Internal::DeviceNetworkInfo & threadData) override;
    void SendOperationalCredentials() override;

    //////////// NetworkProvisioningDelegate Implementation ///////////////
    CHIP_ERROR SendSecureMessage(Protocols::CHIPProtocolId protocol, uint8_t msgType, System::PacketBufferHandle msgBug) override;
    void OnNetworkProvisioningError(CHIP_ERROR error) override;
    void OnNetworkProvisioningComplete() override;

    //////////// TransportMgrDelegate Implementation ///////////////
    void OnMessageReceived(const PacketHeader & header, const Transport::PeerAddress & source,
                           System::PacketBufferHandle msgBuf) override;

    /**
     * @brief
     *  Get the IP address assigned to the device during network provisioning
     *  process.
     *
     * @return The IP address of the device
     */
    const Inet::IPAddress & GetIPAddress() const { return mNetworkProvision.GetIPAddress(); }

private:
    CHIP_ERROR HandlePairingMessage(const PacketHeader & packetHeader, const Transport::PeerAddress & peerAddress,
                                    System::PacketBufferHandle msgBuf);
    CHIP_ERROR Pair(Optional<NodeId> nodeId, uint32_t setupPINCode);
    CHIP_ERROR WaitForPairing(Optional<NodeId> nodeId, uint32_t setupPINCode);

    CHIP_ERROR HandleSecureMessage(const PacketHeader & packetHeader, const Transport::PeerAddress & peerAddress,
                                   System::PacketBufferHandle msgBuf);
    Transport::Base * mTransport          = nullptr; ///< Underlying transport
    RendezvousSessionDelegate * mDelegate = nullptr; ///< Underlying transport events
    RendezvousParameters mParams;                    ///< Rendezvous configuration

    PASESession mPairingSession;
    NetworkProvisioning mNetworkProvision;
    Transport::PeerAddress mPeerAddress; // Current peer address we are doing rendezvous with.
    TransportMgrBase * mTransportMgr;
    uint16_t mNextKeyId                         = 0;
    SecureSessionMgr * mSecureSessionMgr        = nullptr;
    SecureSessionHandle * mPairingSessionHandle = nullptr;

    RendezvousSession::State mCurrentState = State::kInit;
    void UpdateState(RendezvousSession::State newState, CHIP_ERROR err = CHIP_NO_ERROR);

    void InitPairingSessionHandle();
    void ReleasePairingSessionHandle();
};

} // namespace chip
