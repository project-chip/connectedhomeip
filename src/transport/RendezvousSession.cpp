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
static const char * kSpake2pKeyExchangeSalt        = "SPAKE2P Key Exchange Salt";

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
            ReturnErrorOnFailure(WaitForPairing(mParams.GetLocalNodeId(), mParams.GetPASEVerifier()));
        }
        else
        {
            ReturnErrorOnFailure(WaitForPairing(mParams.GetLocalNodeId(), mParams.GetSetupPINCode()));
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

    if (peerAddress.GetTransportType() == Transport::Type::kBle)
    {
        return mTransport->SendMessage(header, peerAddress, std::move(msgIn));
    }
    else if (mTransportMgr != nullptr)
    {
        return mTransportMgr->SendMessage(header, peerAddress, std::move(msgIn));
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
    CHIP_ERROR err = mSecureSessionMgr->NewPairing(
        Optional<Transport::PeerAddress>::Value(mPairingSession.PeerConnection().GetPeerAddress()),
        mPairingSession.PeerConnection().GetPeerNodeId(), &mPairingSession, mAdmin->GetAdminId(), mTransport);
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
        if (!mParams.HasRemoteNodeId())
        {
            ChipLogProgress(Ble, "Completed rendezvous with %llu", mPairingSession.GetPeerNodeId());
            mParams.SetRemoteNodeId(mPairingSession.GetPeerNodeId());
        }
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

    CHIP_ERROR err = Pair(mParams.GetLocalNodeId(), mParams.GetSetupPINCode());
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
        if (packetHeader.GetSourceNodeId().HasValue())
        {
            ChipLogProgress(Ble, "Received pairing message from %llu", packetHeader.GetSourceNodeId().Value());
        }
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

    PayloadHeader payloadHeader;
    ReturnErrorOnFailure(SecureMessageCodec::Decode(&mPairingSession.PeerConnection(), payloadHeader, packetHeader, msgBuf));

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
                                                                     mPairingSession.PeerConnection().GetPeerKeyID());
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

CHIP_ERROR RendezvousSession::WaitForPairing(Optional<NodeId> nodeId, uint32_t setupPINCode)
{
    UpdateState(State::kSecurePairing);
    return mPairingSession.WaitForPairing(setupPINCode, kSpake2p_Iteration_Count,
                                          reinterpret_cast<const unsigned char *>(kSpake2pKeyExchangeSalt),
                                          strlen(kSpake2pKeyExchangeSalt), nodeId, 0, this);
}

CHIP_ERROR RendezvousSession::WaitForPairing(Optional<NodeId> nodeId, const PASEVerifier & verifier)
{
    UpdateState(State::kSecurePairing);
    return mPairingSession.WaitForPairing(verifier, nodeId, 0, this);
}

CHIP_ERROR RendezvousSession::Pair(Optional<NodeId> nodeId, uint32_t setupPINCode)
{
    UpdateState(State::kSecurePairing);
    return mPairingSession.Pair(mParams.GetPeerAddress(), setupPINCode, nodeId, mParams.GetRemoteNodeId().ValueOr(kUndefinedNodeId),
                                mNextKeyId++, this);
}

CHIP_ERROR RendezvousSession::Pair(Optional<NodeId> nodeId, const PASEVerifier & verifier)
{
    UpdateState(State::kSecurePairing);
    return mPairingSession.Pair(mParams.GetPeerAddress(), verifier, nodeId, mParams.GetRemoteNodeId().ValueOr(kUndefinedNodeId),
                                mNextKeyId++, this);
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
