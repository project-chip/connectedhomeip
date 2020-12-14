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
#include <transport/SecureSessionMgr.h>
#include <transport/TransportMgr.h>
#include <transport/raw/PeerAddress.h>

#if CONFIG_NETWORK_LAYER_BLE
#include <transport/BLE.h>
#endif // CONFIG_NETWORK_LAYER_BLE

static const size_t kMax_SecureSDU_Length          = 1024;
static constexpr uint32_t kSpake2p_Iteration_Count = 100;
static const char * kSpake2pKeyExchangeSalt        = "SPAKE2P Key Exchange Salt";

using namespace chip::Inet;
using namespace chip::System;
using namespace chip::Transport;

namespace chip {

CHIP_ERROR RendezvousSession::Init(const RendezvousParameters & params, TransportMgrBase * transportMgr)
{
    mParams       = params;
    mTransportMgr = transportMgr;
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mParams.HasSetupPINCode(), CHIP_ERROR_INVALID_ARGUMENT);

    // TODO: BLE Should be a transport, in that case, RendezvousSession and BLE should decouple
    if (params.GetPeerAddress().GetTransportType() == Transport::Type::kBle)
#if CONFIG_NETWORK_LAYER_BLE
    {
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
        ReturnErrorOnFailure(WaitForPairing(mParams.GetLocalNodeId(), mParams.GetSetupPINCode()));
    }

    mNetworkProvision.Init(this);
    // TODO: We should assmue mTransportMgr not null for IP rendezvous.
    if (mTransportMgr != nullptr)
    {
        mTransportMgr->SetRendezvousSession(this);
    }

    return CHIP_NO_ERROR;
}

RendezvousSession::~RendezvousSession()
{
    if (mTransport)
    {
        chip::Platform::Delete(mTransport);
        mTransport = nullptr;
    }

    mDelegate = nullptr;
}

CHIP_ERROR RendezvousSession::SendPairingMessage(const PacketHeader & header, const Transport::PeerAddress & peerAddress,
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
        ChipLogError(Ble, "SendPairingMessage dropped since no transport mgr for IP rendezvous");
        return CHIP_ERROR_INVALID_ADDRESS;
    }
}

CHIP_ERROR RendezvousSession::SendSecureMessage(Protocols::CHIPProtocolId protocol, uint8_t msgType,
                                                System::PacketBufferHandle msgBuf)
{
    PayloadHeader payloadHeader;
    payloadHeader.SetProtocolID(static_cast<uint16_t>(protocol)).SetMessageType(msgType);

    const uint16_t headerSize = payloadHeader.EncodeSizeBytes();

    VerifyOrReturnError(!msgBuf.IsNull(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(!msgBuf->HasChainedBuffer(), CHIP_ERROR_INVALID_MESSAGE_LENGTH);
    VerifyOrReturnError(msgBuf->TotalLength() < kMax_SecureSDU_Length, CHIP_ERROR_INVALID_MESSAGE_LENGTH);
    VerifyOrReturnError(CanCastTo<uint16_t>(headerSize + msgBuf->TotalLength()), CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    PacketHeader packetHeader;
    packetHeader
        .SetSourceNodeId(mParams.GetLocalNodeId())           //
        .SetDestinationNodeId(mParams.GetRemoteNodeId())     //
        .SetMessageId(mSecureMessageIndex)                   //
        .SetEncryptionKeyID(mPairingSession.GetLocalKeyId()) //
        .SetPayloadLength(static_cast<uint16_t>(headerSize + msgBuf->TotalLength()));

    VerifyOrReturnError(msgBuf->EnsureReservedSize(headerSize), CHIP_ERROR_NO_MEMORY);

    msgBuf->SetStart(msgBuf->Start() - headerSize);

    MessageAuthenticationCode mac;
    uint16_t actualEncodedHeaderSize = 0;
    uint8_t * data                   = msgBuf->Start();
    uint16_t totalLen                = msgBuf->TotalLength();

    ReturnErrorOnFailure(payloadHeader.Encode(data, totalLen, &actualEncodedHeaderSize));
    ReturnErrorOnFailure(mSecureSession.Encrypt(data, totalLen, data, packetHeader, mac));

    uint16_t taglen = 0;
    ReturnErrorOnFailure(mac.Encode(packetHeader, &data[totalLen], kMaxTagLen, &taglen));

    VerifyOrReturnError(CanCastTo<uint16_t>(totalLen + taglen), CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    msgBuf->SetDataLength(static_cast<uint16_t>(totalLen + taglen));

    ReturnErrorOnFailure(mTransport->SendMessage(packetHeader, Transport::PeerAddress::BLE(), std::move(msgBuf)));

    mSecureMessageIndex++;

    return CHIP_NO_ERROR;
}

void RendezvousSession::OnPairingError(CHIP_ERROR err)
{
    OnRendezvousError(err);
}

void RendezvousSession::OnPairingComplete()
{
    CHIP_ERROR err = mPairingSession.DeriveSecureSession(reinterpret_cast<const unsigned char *>(kSpake2pI2RSessionInfo),
                                                         strlen(kSpake2pI2RSessionInfo), mSecureSession);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Ble, "Failed to initialize a secure session: %s", ErrorStr(err));
        return;
    }

    // TODO: This check of BLE transport should be removed in the future, after we have network provisioning cluster and ble becomes
    // a transport.
    if (mParams.GetPeerAddress().GetTransportType() != Transport::Type::kBle || // For rendezvous initializer
        mPeerAddress.GetTransportType() != Transport::Type::kBle)               // For rendezvous target
    {
        if (mRendezvousRemoteNodeId.HasValue() && !mParams.HasRemoteNodeId())
        {
            ChipLogProgress(Ble, "Completed rendezvous with %llu", mRendezvousRemoteNodeId.Value());
            mParams.SetRemoteNodeId(mRendezvousRemoteNodeId.Value());
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
        OnPairingError(err);
    }
}

void RendezvousSession::OnRendezvousConnectionClosed()
{
    if (mParams.IsController())
    {
        return;
    }

    mSecureSession.Reset();
    mRendezvousRemoteNodeId.ClearValue();

    CHIP_ERROR err = WaitForPairing(mParams.GetLocalNodeId(), mParams.GetSetupPINCode());
    if (err != CHIP_NO_ERROR)
    {
        OnPairingError(err);
    }
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

    if (newState == State::kRendezvousComplete && mDelegate != nullptr)
    {
        mDelegate->OnRendezvousComplete();
    }
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
            ChipLogProgress(Ble, "Received rendezvous message from %llu", packetHeader.GetSourceNodeId().Value());
            mRendezvousRemoteNodeId.SetValue(packetHeader.GetSourceNodeId().Value());
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
    uint16_t headerSize = 0;
    uint8_t * plainText = nullptr;
    uint16_t taglen     = 0;
    uint16_t payloadlen = 0;

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
    headerSize = payloadHeader.EncodeSizeBytes();

    uint8_t * data = msgBuf->Start();
    uint16_t len   = msgBuf->TotalLength();

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    /* This is a workaround for the case where PacketBuffer payload is not
       allocated as an inline buffer to PacketBuffer structure */
    System::PacketBufferHandle origMsg;

    origMsg = std::move(msgBuf);
    msgBuf  = PacketBuffer::NewWithAvailableSize(len);

    ReturnErrorCodeIf(msgBuf.IsNull(), CHIP_ERROR_NO_MEMORY);

    msgBuf->SetDataLength(len);
#endif
    plainText = msgBuf->Start();

    payloadlen = packetHeader.GetPayloadLength();
    VerifyOrReturnError(payloadlen <= len, CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    MessageAuthenticationCode mac;
    ReturnErrorOnFailure(mac.Decode(packetHeader, &data[payloadlen], static_cast<uint16_t>(len - payloadlen), &taglen));

    len = static_cast<uint16_t>(len - taglen);
    msgBuf->SetDataLength(len);

    ReturnErrorOnFailure(mSecureSession.Decrypt(data, len, plainText, packetHeader, mac));

    // Use the node IDs from the packet header only after it's successfully decrypted
    if (packetHeader.GetDestinationNodeId().HasValue() && !mParams.HasLocalNodeId())
    {
        ChipLogProgress(Ble, "Received rendezvous message for %llu", packetHeader.GetDestinationNodeId().Value());
        mParams.SetLocalNodeId(packetHeader.GetDestinationNodeId().Value());
    }

    if (packetHeader.GetSourceNodeId().HasValue() && !mParams.HasRemoteNodeId())
    {
        ChipLogProgress(Ble, "Received rendezvous message from %llu", packetHeader.GetSourceNodeId().Value());
        mParams.SetRemoteNodeId(packetHeader.GetSourceNodeId().Value());
    }

    uint16_t decodedSize = 0;
    ReturnErrorOnFailure(payloadHeader.Decode(plainText, len, &decodedSize));

    ReturnErrorCodeIf(headerSize != decodedSize, CHIP_ERROR_INCORRECT_STATE);

    msgBuf->ConsumeHead(headerSize);

    if (payloadHeader.GetProtocolID() == Protocols::kProtocol_NetworkProvisioning)
    {
        ReturnErrorOnFailure(mNetworkProvision.HandleNetworkProvisioningMessage(payloadHeader.GetMessageType(), msgBuf));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR RendezvousSession::WaitForPairing(Optional<NodeId> nodeId, uint32_t setupPINCode)
{
    UpdateState(State::kSecurePairing);
    return mPairingSession.WaitForPairing(setupPINCode, kSpake2p_Iteration_Count,
                                          reinterpret_cast<const unsigned char *>(kSpake2pKeyExchangeSalt),
                                          strlen(kSpake2pKeyExchangeSalt), nodeId, 0, this);
}

CHIP_ERROR RendezvousSession::Pair(Optional<NodeId> nodeId, uint32_t setupPINCode)
{
    UpdateState(State::kSecurePairing);
    return mPairingSession.Pair(mParams.GetPeerAddress(), setupPINCode, nodeId, mNextKeyId++, this);
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
