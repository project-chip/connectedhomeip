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
#include <support/SafeInt.h>

#if CONFIG_NETWORK_LAYER_BLE
#include <transport/BLE.h>
#endif // CONFIG_NETWORK_LAYER_BLE

static const size_t kMax_SecureSDU_Length          = 1024;
static constexpr uint32_t kSpake2p_Iteration_Count = 100;
static const char * kSpake2pKeyExchangeSalt        = "SPAKE2P Key Exchange Salt";

using namespace chip::Inet;
using namespace chip::System;

namespace chip {

CHIP_ERROR RendezvousSession::Init(const RendezvousParameters & params)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    mParams = params;
    VerifyOrExit(mDelegate != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mParams.HasSetupPINCode(), err = CHIP_ERROR_INVALID_ARGUMENT);

    err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#if CONFIG_NETWORK_LAYER_BLE
    {
        Transport::BLE * transport = chip::Platform::New<Transport::BLE>();
        err                        = transport->Init(this, mParams);
        mTransport                 = transport;
    }
#endif // CONFIG_NETWORK_LAYER_BLE
    SuccessOrExit(err);

    if (!mParams.IsController())
    {
        err = WaitForPairing(mParams.GetLocalNodeId(), mParams.GetSetupPINCode());
        SuccessOrExit(err);
    }

    mNetworkProvision.Init(this);

exit:
    return err;
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

CHIP_ERROR RendezvousSession::SendPairingMessage(const PacketHeader & header, Header::Flags payloadFlags,
                                                 System::PacketBuffer * msgIn)
{
    if (mCurrentState != State::kSecurePairing)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    return mTransport->SendMessage(header, payloadFlags, Transport::PeerAddress::BLE(), msgIn);
}

CHIP_ERROR RendezvousSession::SendSecureMessage(Protocols::CHIPProtocolId protocol, uint8_t msgType, System::PacketBuffer * msgIn)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    PacketHeader packetHeader;
    PayloadHeader payloadHeader;
    MessageAuthenticationCode mac;
    System::PacketBufferHandle msgBuf;
    const uint16_t headerSize = payloadHeader.EncodeSizeBytes();
    uint16_t actualEncodedHeaderSize;
    uint8_t * data    = nullptr;
    uint16_t totalLen = 0;
    uint16_t taglen   = 0;

    msgBuf.Adopt(msgIn);
    VerifyOrExit(!msgBuf.IsNull(), err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(msgBuf->Next() == nullptr, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);
    VerifyOrExit(msgBuf->TotalLength() < kMax_SecureSDU_Length, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);
    VerifyOrExit(CanCastTo<uint16_t>(headerSize + msgBuf->TotalLength()), err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    packetHeader
        .SetSourceNodeId(mParams.GetLocalNodeId())           //
        .SetDestinationNodeId(mParams.GetRemoteNodeId())     //
        .SetMessageId(mSecureMessageIndex)                   //
        .SetEncryptionKeyID(mPairingSession.GetLocalKeyId()) //
        .SetPayloadLength(static_cast<uint16_t>(headerSize + msgBuf->TotalLength()));

    payloadHeader.SetProtocolID(static_cast<uint16_t>(protocol)).SetMessageType(msgType);

    VerifyOrExit(msgBuf->EnsureReservedSize(headerSize), err = CHIP_ERROR_NO_MEMORY);

    msgBuf->SetStart(msgBuf->Start() - headerSize);
    data     = msgBuf->Start();
    totalLen = msgBuf->TotalLength();

    err = payloadHeader.Encode(data, totalLen, &actualEncodedHeaderSize);
    SuccessOrExit(err);

    err = mSecureSession.Encrypt(data, totalLen, data, packetHeader, payloadHeader.GetEncodePacketFlags(), mac);
    SuccessOrExit(err);

    err = mac.Encode(packetHeader, &data[totalLen], kMaxTagLen, &taglen);
    SuccessOrExit(err);

    VerifyOrExit(CanCastTo<uint16_t>(totalLen + taglen), err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);
    msgBuf->SetDataLength(static_cast<uint16_t>(totalLen + taglen));

    err = mTransport->SendMessage(packetHeader, payloadHeader.GetEncodePacketFlags(), Transport::PeerAddress::BLE(),
                                  msgBuf.Release_ForNow());
    SuccessOrExit(err);

    mSecureMessageIndex++;

exit:
    return err;
}

void RendezvousSession::OnPairingError(CHIP_ERROR err)
{
    OnRendezvousError(err);
}

void RendezvousSession::OnPairingComplete()
{
    CHIP_ERROR err = mPairingSession.DeriveSecureSession(reinterpret_cast<const unsigned char *>(kSpake2pI2RSessionInfo),
                                                         strlen(kSpake2pI2RSessionInfo), mSecureSession);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Ble, "Failed to initialize a secure session: %s", ErrorStr(err)));

    UpdateState(State::kNetworkProvisioning);
exit:
    return;
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
    if (mParams.IsController())
    {
        CHIP_ERROR err = Pair(mParams.GetLocalNodeId(), mParams.GetSetupPINCode());
        VerifyOrExit(err == CHIP_NO_ERROR, OnPairingError(err));
    }

exit:
    return;
}

void RendezvousSession::OnRendezvousConnectionClosed()
{
    if (!mParams.IsController())
    {
        mSecureSession.Reset();
        CHIP_ERROR err = WaitForPairing(mParams.GetLocalNodeId(), mParams.GetSetupPINCode());
        VerifyOrExit(err == CHIP_NO_ERROR, OnPairingError(err));
    }

exit:
    return;
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

void RendezvousSession::OnRendezvousMessageReceived(PacketBuffer * msgBuf)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    switch (mCurrentState)
    {
    case State::kSecurePairing:
        err = HandlePairingMessage(msgBuf);
        break;

    case State::kNetworkProvisioning:
        err = HandleSecureMessage(msgBuf);
        break;

    default:
        err = CHIP_ERROR_INCORRECT_STATE;
        break;
    };

    SuccessOrExit(err);

exit:
    if (err != CHIP_NO_ERROR)
    {
        OnRendezvousError(err);
    }
}

CHIP_ERROR RendezvousSession::HandlePairingMessage(PacketBuffer * msgBuf)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    PacketHeader packetHeader;
    uint16_t headerSize = 0;

    err = packetHeader.Decode(msgBuf->Start(), msgBuf->DataLength(), &headerSize);
    SuccessOrExit(err);

    msgBuf->ConsumeHead(headerSize);

    err = mPairingSession.HandlePeerMessage(packetHeader, msgBuf);
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR RendezvousSession::HandleSecureMessage(PacketBuffer * msgIn)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    PacketHeader packetHeader;
    PayloadHeader payloadHeader;
    MessageAuthenticationCode mac;
    uint16_t headerSize  = 0;
    uint8_t * data       = nullptr;
    uint8_t * plainText  = nullptr;
    uint16_t len         = 0;
    uint16_t decodedSize = 0;
    uint16_t taglen      = 0;
    uint16_t payloadlen  = 0;

    System::PacketBufferHandle msgBuf;
    System::PacketBufferHandle origMsg;

    msgBuf.Adopt(msgIn);
    VerifyOrExit(!msgBuf.IsNull(), err = CHIP_ERROR_INVALID_ARGUMENT);

    err = packetHeader.Decode(msgBuf->Start(), msgBuf->DataLength(), &headerSize);
    SuccessOrExit(err);
    msgBuf->ConsumeHead(headerSize);

    // Check if the source and destination node IDs match with what we already know
    if (packetHeader.GetDestinationNodeId().HasValue() && mParams.HasLocalNodeId())
    {
        VerifyOrExit(packetHeader.GetDestinationNodeId().Value() == mParams.GetLocalNodeId().Value(),
                     err = CHIP_ERROR_WRONG_NODE_ID);
    }

    if (packetHeader.GetSourceNodeId().HasValue() && mParams.HasRemoteNodeId())
    {
        VerifyOrExit(packetHeader.GetSourceNodeId().Value() == mParams.GetRemoteNodeId().Value(), err = CHIP_ERROR_WRONG_NODE_ID);
    }

    headerSize = payloadHeader.EncodeSizeBytes();
    data       = msgBuf->Start();
    len        = msgBuf->TotalLength();

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    /* This is a workaround for the case where PacketBuffer payload is not
       allocated as an inline buffer to PacketBuffer structure */
    origMsg = std::move(msgBuf);
    msgBuf  = PacketBuffer::NewWithAvailableSize(len);
    VerifyOrExit(!msgBuf.IsNull(), err = CHIP_ERROR_NO_MEMORY);

    msgBuf->SetDataLength(len);
#endif
    plainText = msgBuf->Start();

    payloadlen = packetHeader.GetPayloadLength();
    VerifyOrExit(payloadlen <= len, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);
    err = mac.Decode(packetHeader, &data[payloadlen], static_cast<uint16_t>(len - payloadlen), &taglen);
    SuccessOrExit(err);

    len = static_cast<uint16_t>(len - taglen);
    msgBuf->SetDataLength(len);

    err = mSecureSession.Decrypt(data, len, plainText, packetHeader, payloadHeader.GetEncodePacketFlags(), mac);
    SuccessOrExit(err);

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

    err = payloadHeader.Decode(packetHeader.GetFlags(), plainText, len, &decodedSize);
    SuccessOrExit(err);
    VerifyOrExit(headerSize == decodedSize, err = CHIP_ERROR_INCORRECT_STATE);

    msgBuf->ConsumeHead(headerSize);

    if (payloadHeader.GetProtocolID() == Protocols::kProtocol_NetworkProvisioning)
    {
        err = mNetworkProvision.HandleNetworkProvisioningMessage(payloadHeader.GetMessageType(), msgBuf.Get_ForNow());
        SuccessOrExit(err);
    }

exit:
    return err;
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
    return mPairingSession.Pair(setupPINCode, kSpake2p_Iteration_Count,
                                reinterpret_cast<const unsigned char *>(kSpake2pKeyExchangeSalt), strlen(kSpake2pKeyExchangeSalt),
                                nodeId, mNextKeyId++, this);
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
