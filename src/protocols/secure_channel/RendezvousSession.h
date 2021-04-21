/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
#include <messaging/ExchangeMgr.h>
#include <protocols/Protocols.h>
#include <protocols/secure_channel/NetworkProvisioning.h>
#include <protocols/secure_channel/PASESession.h>
#include <protocols/secure_channel/RendezvousParameters.h>
#include <protocols/secure_channel/SessionEstablishmentExchangeDispatch.h>
#include <support/BufferWriter.h>
#include <transport/AdminPairingTable.h>
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
                          public NetworkProvisioningDelegate,
                          public TransportMgrDelegate
{
public:
    RendezvousSession(RendezvousSessionDelegate * delegate) : mDelegate(delegate) {}
    ~RendezvousSession() override;

    /**
     * @brief
     *  Initialize the underlying transport using the RendezvousParameters passed in the constructor.
     *
     * @param params          The RendezvousParameters
     * @param exchangeManager The instance of exchange manager to create exchange contexts
     * @param transportMgr    The transport to use
     * @param sessionMgr      Pointer to secure session manager
     * @param admin           Pointer to a device administrator info that will be filled up on successful pairing
     * @ return CHIP_ERROR  The result of the initialization
     */
    CHIP_ERROR Init(const RendezvousParameters & params, Messaging::ExchangeManager * exchangeManager,
                    TransportMgrBase * transportMgr, SecureSessionMgr * sessionMgr, Transport::AdminPairingInfo * admin);

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
    void OnSessionEstablished() override;

    //////////// RendezvousSessionDelegate Implementation ///////////////
    void OnRendezvousConnectionOpened() override;
    void OnRendezvousConnectionClosed() override;
    void OnRendezvousError(CHIP_ERROR err) override;

    //////////// NetworkProvisioningDelegate Implementation ///////////////
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

    Transport::AdminId GetAdminId() const { return (mAdmin != nullptr) ? mAdmin->GetAdminId() : Transport::kUndefinedAdminId; }

    uint16_t GetNextKeyId() const { return mNextKeyId; }
    void SetNextKeyId(uint16_t id) { mNextKeyId = id; }

private:
    CHIP_ERROR Pair(uint32_t setupPINCode);
    CHIP_ERROR WaitForPairing(uint32_t setupPINCode);
    CHIP_ERROR WaitForPairing(const PASEVerifier & verifier);

    RendezvousSessionDelegate * mDelegate = nullptr; ///< Underlying transport events
    RendezvousParameters mParams;                    ///< Rendezvous configuration

    PASESession mPairingSession;
    NetworkProvisioning mNetworkProvision;
    Messaging::ExchangeManager * mExchangeManager = nullptr;
    TransportMgrBase * mTransportMgr;
    uint16_t mNextKeyId                  = 0;
    SecureSessionMgr * mSecureSessionMgr = nullptr;

    Transport::AdminPairingInfo * mAdmin = nullptr;

    void Cleanup();
};

} // namespace chip
