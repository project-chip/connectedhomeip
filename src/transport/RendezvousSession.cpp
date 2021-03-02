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
#include <transport/RendezvousSession.h>

#include <core/CHIPEncoding.h>
#include <core/CHIPSafeCasts.h>
#include <platform/internal/DeviceNetworkInfo.h>
#include <support/CHIPMem.h>
#include <support/CodeUtils.h>
#include <support/ErrorStr.h>
#include <support/ReturnMacros.h>
#include <support/SafeInt.h>
#include <transport/RendezvousSession.h>
#include <transport/SecureMessageCodec.h>
#include <transport/SecureSessionMgr.h>
#include <transport/TransportMgr.h>
#include <transport/raw/PeerAddress.h>

#if CONFIG_NETWORK_LAYER_BLE
#include <transport/BLE.h>
#endif // CONFIG_NETWORK_LAYER_BLE

static constexpr uint32_t kSpake2p_Iteration_Count = 100;
static const char * kSpake2pKeyExchangeSalt        = "SPAKE2P Key Salt";

using namespace chip::Inet;
using namespace chip::System;
using namespace chip::Transport;

namespace chip {

CHIP_ERROR RendezvousSession::Init(const RendezvousParameters & params, TransportMgrBase * transportMgr,
                                   SecureSessionMgr * sessionMgr, Transport::AdminPairingInfo * admin)
{
    mParams       = params;
    mTransportMgr = transportMgr;
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(sessionMgr != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(admin != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mParams.HasSetupPINCode() || mParams.HasPASEVerifier(), CHIP_ERROR_INVALID_ARGUMENT);

    mSecureSessionMgr = sessionMgr;
    mAdmin            = admin;

    // TODO: BLE Should be a transport, in that case, RendezvousSession and BLE should decouple
    if (params.GetPeerAddress().GetTransportType() == Transport::Type::kBle)
#if CONFIG_NETWORK_LAYER_BLE
    {
        ReturnErrorOnFailure(mParams.GetAdvertisementDelegate()->StartAdvertisement());
        Transport::BLE * transport = chip::Platform::New<Transport::BLE>();
        mTransport                 = transport;

        ReturnErrorOnFailure(transport->Init(this, mParams));
    }
#else
    {
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }
#endif // CONFIG_NETWORK_LAYER_BLE

    if (!mParams.IsController())
    {
        if (mParams.HasPASEVerifier())
        {
            ReturnErrorOnFailure(WaitForPairing(mParams.GetPASEVerifier()));
        }
        else
        {
            ReturnErrorOnFailure(WaitForPairing(mParams.GetSetupPINCode()));
        }
    }

    mNetworkProvision.Init(this);
    // TODO: We should assume mTransportMgr not null for IP rendezvous.
    if (mTransportMgr != nullptr)
    {
        mTransportMgr->SetRendezvousSession(this);
    }

    return CHIP_NO_ERROR;
}

RendezvousSession::~RendezvousSession()
{
    ReleasePairingSessionHandle();

    if (mTransport)
    {
        chip::Platform::Delete(mTransport);
        mTransport = nullptr;
    }

    mDelegate = nullptr;
}

CHIP_ERROR RendezvousSession::SendSessionEstablishmentMessage(const PacketHeader & header,
                                                              const Transport::PeerAddress & peerAddress,
                                                              System::PacketBufferHandle msgIn)
{
    if (mCurrentState != State::kSecurePairing)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    // TODO: Admin information and node ID shouold be set during operation credential configuration
    // This setting of header properties is a hack to transmit the local node id to our peer
    // so that admin configurations are preserved. Generally PASE sesions should not need to send
    // node-ids as the peers may not be part of a fabric.
    PacketHeader headerWithNodeIds = header;

    if (mParams.HasLocalNodeId())
    {
        headerWithNodeIds.SetSourceNodeId(mParams.GetLocalNodeId().Value());
    }

    if (mParams.HasRemoteNodeId())
    {
        headerWithNodeIds.SetDestinationNodeId(mParams.GetRemoteNodeId());
    }

    if (peerAddress.GetTransportType() == Transport::Type::kBle)
    {
        return mTransport->SendMessage(headerWithNodeIds, peerAddress, std::move(msgIn));
    }
    else if (mTransportMgr != nullptr)
    {
        return mTransportMgr->SendMessage(headerWithNodeIds, peerAddress, std::move(msgIn));
    }
    else
    {
        ChipLogError(Ble, "SendSessionEstablishmentMessage dropped since no transport mgr for IP rendezvous");
        return CHIP_ERROR_INVALID_ADDRESS;
    }
}

CHIP_ERROR RendezvousSession::SendSecureMessage(Protocols::CHIPProtocolId protocol, uint8_t msgType,
                                                System::PacketBufferHandle msgBuf)
{
    VerifyOrReturnError(mPairingSessionHandle != nullptr, CHIP_ERROR_INCORRECT_STATE);

    PayloadHeader payloadHeader;
    payloadHeader.SetMessageType(static_cast<uint16_t>(protocol), msgType);

    return mSecureSessionMgr->SendMessage(*mPairingSessionHandle, payloadHeader, std::move(msgBuf));
}

void RendezvousSession::OnSessionEstablishmentError(CHIP_ERROR err)
{
    OnRendezvousError(err);
}

void RendezvousSession::OnSessionEstablished()
{
    SecureSessionMgr::PairingDirection direction = SecureSessionMgr::PairingDirection::kInitiator;
    if (!mParams.IsController())
    {
        direction = SecureSessionMgr::PairingDirection::kResponder;
    }

    // TODO: Once Operational credentials are implemented, node id assignment should be done during opcreds configuration.
    // - can use internal node ids (0xFFFF_FFFE_xxxx_xxx - spec still being defined) if a temporary
    //   node id is required for indexing
    // - should only assign a final node id as part of setting operational credentials
    if (!mParams.GetRemoteNodeId().HasValue())
    {
        ChipLogError(Ble, "Missing node id in rendezvous parameters. Node ID is required until opcerts are implemented");
    }

    const auto defaultPeerNodeId = mParams.IsController() ? kTestDeviceNodeId : kTestControllerNodeId;
    mPairingSession.PeerConnection().SetPeerNodeId(mParams.GetRemoteNodeId().ValueOr(defaultPeerNodeId));

    CHIP_ERROR err = mSecureSessionMgr->NewPairing(
        Optional<Transport::PeerAddress>::Value(mPairingSession.PeerConnection().GetPeerAddress()),
        mPairingSession.PeerConnection().GetPeerNodeId(), &mPairingSession, direction, mAdmin->GetAdminId(), mTransport);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Ble, "Failed in setting up secure channel: err %s", ErrorStr(err));
        return;
    }

    InitPairingSessionHandle();

    // TODO: This check of BLE transport should be removed in the future, after we have network provisioning cluster and ble becomes
    // a transport.
    if (mParams.GetPeerAddress().GetTransportType() != Transport::Type::kBle || // For rendezvous initializer
        mPeerAddress.GetTransportType() != Transport::Type::kBle)               // For rendezvous target
    {
        UpdateState(State::kRendezvousComplete);
        if (!mParams.IsController())
        {
            OnRendezvousConnectionClosed();
        }
    }
    else
    {
        UpdateState(State::kNetworkProvisioning);
    }
}

void RendezvousSession::OnNetworkProvisioningError(CHIP_ERROR err)
{
    OnRendezvousError(err);
}

void RendezvousSession::OnNetworkProvisioningComplete()
{
    UpdateState(State::kRendezvousComplete);
}

void RendezvousSession::OnRendezvousConnectionOpened()
{
    if (!mParams.IsController())
    {
        return;
    }

    CHIP_ERROR err = Pair(mParams.GetSetupPINCode());
    if (err != CHIP_NO_ERROR)
    {
        OnSessionEstablishmentError(err);
    }
}

void RendezvousSession::OnRendezvousConnectionClosed()
{
    UpdateState(State::kInit, CHIP_NO_ERROR);
}

void RendezvousSession::OnRendezvousError(CHIP_ERROR err)
{
    if (mDelegate != nullptr)
    {
        switch (mCurrentState)
        {
        case State::kSecurePairing:
            mDelegate->OnRendezvousStatusUpdate(RendezvousSessionDelegate::SecurePairingFailed, err);
            break;

        case State::kNetworkProvisioning:
            mDelegate->OnRendezvousStatusUpdate(RendezvousSessionDelegate::NetworkProvisioningFailed, err);
            break;

        default:
            break;
        };
        mDelegate->OnRendezvousError(err);
    }
    UpdateState(State::kInit, err);
    if (mAdmin != nullptr)
    {
        mAdmin->Reset();
    }
}

void RendezvousSession::UpdateState(RendezvousSession::State newState, CHIP_ERROR err)
{
    if (mDelegate != nullptr)
    {
        switch (mCurrentState)
        {
        case State::kSecurePairing:
            if (newState != State::kInit)
            {
                mDelegate->OnRendezvousStatusUpdate(RendezvousSessionDelegate::SecurePairingSuccess, err);
            }
            else
            {
                mDelegate->OnRendezvousStatusUpdate(RendezvousSessionDelegate::SecurePairingFailed, err);
            }
            break;

        case State::kNetworkProvisioning:
            if (newState != State::kInit)
            {
                mDelegate->OnRendezvousStatusUpdate(RendezvousSessionDelegate::NetworkProvisioningSuccess, err);
            }
            else
            {
                mDelegate->OnRendezvousStatusUpdate(RendezvousSessionDelegate::NetworkProvisioningFailed, err);
            }
            break;

        default:
            break;
        };
    }
    mCurrentState = newState;

    switch (mCurrentState)
    {
    case State::kRendezvousComplete:
        if (mDelegate != nullptr)
        {
            mDelegate->OnRendezvousComplete();
        }

        mParams.GetAdvertisementDelegate()->RendezvousComplete();

        // Release the admin, as the rendezvous is complete.
        mAdmin = nullptr;
        break;

    case State::kSecurePairing:
        // Release the previous session handle
        ReleasePairingSessionHandle();
        break;

    case State::kInit:
        // Release the previous session handle
        ReleasePairingSessionHandle();

        // Disable rendezvous advertisement
        mParams.GetAdvertisementDelegate()->StopAdvertisement();
        if (mTransport)
        {
            // Free the transport
            chip::Platform::Delete(mTransport);
            mTransport = nullptr;
        }
        break;

    default:
        break;
    };
}

void RendezvousSession::OnRendezvousMessageReceived(const PacketHeader & packetHeader, const PeerAddress & peerAddress,
                                                    PacketBufferHandle msgBuf)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    mPeerAddress   = peerAddress;
    // TODO: RendezvousSession should handle SecurePairing messages only

    switch (mCurrentState)
    {
    case State::kSecurePairing:
        // TODO: when operational certificates are in use, Rendezvous should not rely on destination node id.
        //   This marks internal key settings to identify the underlying key by the value that the
        // remote node has sent.
        //   Generally such things should be saved as part of a CASE session, where operational credentials
        // would be authenticated and matched against a remote node id.
        //   Also unclear why 'destination node id' is used here - it seems to be better ot use
        // the source node id, which would identify the peer (rather than 'self' - rendezvous should
        // already know the local node id).
        if (packetHeader.GetDestinationNodeId().HasValue())
        {
            ChipLogProgress(Ble, "Received pairing message for %llu", packetHeader.GetDestinationNodeId().Value());
            mAdmin->SetNodeId(packetHeader.GetDestinationNodeId().Value());
        }

        err = HandlePairingMessage(packetHeader, peerAddress, std::move(msgBuf));
        break;

    case State::kNetworkProvisioning:
        err = HandleSecureMessage(packetHeader, peerAddress, std::move(msgBuf));
        break;

    default:
        err = CHIP_ERROR_INCORRECT_STATE;
        break;
    };

    if (err != CHIP_NO_ERROR)
    {
        OnRendezvousError(err);
    }
}

void RendezvousSession::OnMessageReceived(const PacketHeader & header, const Transport::PeerAddress & source,
                                          System::PacketBufferHandle msgBuf)
{
    // TODO: OnRendezvousMessageReceived can be renamed to OnMessageReceived after BLE becomes a transport.
    this->OnRendezvousMessageReceived(header, source, std::move(msgBuf));
}

CHIP_ERROR RendezvousSession::HandlePairingMessage(const PacketHeader & packetHeader, const PeerAddress & peerAddress,
                                                   PacketBufferHandle msgBuf)
{
    return mPairingSession.HandlePeerMessage(packetHeader, peerAddress, std::move(msgBuf));
}

CHIP_ERROR RendezvousSession::HandleSecureMessage(const PacketHeader & packetHeader, const PeerAddress & peerAddress,
                                                  PacketBufferHandle msgBuf)
{
    ReturnErrorCodeIf(mPairingSessionHandle == nullptr, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorCodeIf(msgBuf.IsNull(), CHIP_ERROR_INVALID_ARGUMENT);

    // Check if the source and destination node IDs match with what we already know
    if (packetHeader.GetDestinationNodeId().HasValue() && mParams.HasLocalNodeId())
    {
        VerifyOrReturnError(packetHeader.GetDestinationNodeId().Value() == mParams.GetLocalNodeId().Value(),
                            CHIP_ERROR_WRONG_NODE_ID);
    }

    if (packetHeader.GetSourceNodeId().HasValue() && mParams.HasRemoteNodeId())
    {
        VerifyOrReturnError(packetHeader.GetSourceNodeId().Value() == mParams.GetRemoteNodeId().Value(), CHIP_ERROR_WRONG_NODE_ID);
    }

    PeerConnectionState * state = mSecureSessionMgr->GetPeerConnectionState(*mPairingSessionHandle);
    ReturnErrorCodeIf(state == nullptr, CHIP_ERROR_KEY_NOT_FOUND_FROM_PEER);

    PayloadHeader payloadHeader;
    ReturnErrorOnFailure(SecureMessageCodec::Decode(state, payloadHeader, packetHeader, msgBuf));

    // Use the node IDs from the packet header only after it's successfully decrypted
    if (packetHeader.GetDestinationNodeId().HasValue() && !mParams.HasLocalNodeId())
    {
        ChipLogProgress(Ble, "Received rendezvous message for %llu", packetHeader.GetDestinationNodeId().Value());
        mAdmin->SetNodeId(packetHeader.GetDestinationNodeId().Value());
        mParams.SetLocalNodeId(packetHeader.GetDestinationNodeId().Value());
        mSecureSessionMgr->SetLocalNodeID(packetHeader.GetDestinationNodeId().Value());
    }

    if (packetHeader.GetSourceNodeId().HasValue() && !mParams.HasRemoteNodeId())
    {
        ChipLogProgress(Ble, "Received rendezvous message from %llu", packetHeader.GetSourceNodeId().Value());
        mParams.SetRemoteNodeId(packetHeader.GetSourceNodeId().Value());
    }

    if (payloadHeader.GetProtocolID() == Protocols::kProtocol_NetworkProvisioning)
    {
        ReturnErrorOnFailure(mNetworkProvision.HandleNetworkProvisioningMessage(payloadHeader.GetMessageType(), msgBuf));
    }

    return CHIP_NO_ERROR;
}

void RendezvousSession::InitPairingSessionHandle()
{
    ReleasePairingSessionHandle();
    mPairingSessionHandle = chip::Platform::New<SecureSessionHandle>(mPairingSession.PeerConnection().GetPeerNodeId(),
                                                                     mPairingSession.PeerConnection().GetPeerKeyID(),
                                                                     mPairingSession.PeerConnection().GetAdminId());
}

void RendezvousSession::ReleasePairingSessionHandle()
{
    if (mPairingSessionHandle != nullptr)
    {
        Transport::PeerConnectionState * state = mSecureSessionMgr->GetPeerConnectionState(*mPairingSessionHandle);
        if (state != nullptr)
        {
            // Reset the transport and peer address in the active secure channel
            // This will allow the regular transport (e.g. UDP) to take over the existing secure channel
            PeerAddress addr;
            state->SetTransport(nullptr);
            state->SetPeerAddress(addr);
        }
        chip::Platform::Delete(mPairingSessionHandle);
        mPairingSessionHandle = nullptr;
    }
}

CHIP_ERROR RendezvousSession::WaitForPairing(uint32_t setupPINCode)
{
    UpdateState(State::kSecurePairing);
    return mPairingSession.WaitForPairing(setupPINCode, kSpake2p_Iteration_Count,
                                          reinterpret_cast<const unsigned char *>(kSpake2pKeyExchangeSalt),
                                          strlen(kSpake2pKeyExchangeSalt), mNextKeyId++, this);
}

CHIP_ERROR RendezvousSession::WaitForPairing(const PASEVerifier & verifier)
{
    UpdateState(State::kSecurePairing);
    return mPairingSession.WaitForPairing(verifier, mNextKeyId++, this);
}

CHIP_ERROR RendezvousSession::Pair(uint32_t setupPINCode)
{
    UpdateState(State::kSecurePairing);
    return mPairingSession.Pair(mParams.GetPeerAddress(), setupPINCode, mNextKeyId++, this);
}

CHIP_ERROR RendezvousSession::Pair(const PASEVerifier & verifier)
{
    UpdateState(State::kSecurePairing);
    return mPairingSession.Pair(mParams.GetPeerAddress(), verifier, mNextKeyId++, this);
}

void RendezvousSession::SendNetworkCredentials(const char * ssid, const char * passwd)
{
    mNetworkProvision.SendNetworkCredentials(ssid, passwd);
}

void RendezvousSession::SendThreadCredentials(const DeviceLayer::Internal::DeviceNetworkInfo & threadData)
{
    mNetworkProvision.SendThreadCredentials(threadData);
}

void RendezvousSession::SendOperationalCredentials() {}
} // namespace chip
