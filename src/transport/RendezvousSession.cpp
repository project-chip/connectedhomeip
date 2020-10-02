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

#include <core/CHIPSafeCasts.h>
#include <support/CodeUtils.h>
#include <support/ErrorStr.h>
#include <support/SafeInt.h>
#include <transport/RendezvousSession.h>

#if CONFIG_DEVICE_LAYER
#include <platform/CHIPDeviceLayer.h>
#endif

#if CONFIG_NETWORK_LAYER_BLE
#include <transport/BLE.h>
#endif // CONFIG_NETWORK_LAYER_BLE

static const size_t kMax_SecureSDU_Length          = 1024;
static constexpr uint32_t kSpake2p_Iteration_Count = 50000;
static const char * kSpake2pKeyExchangeSalt        = "SPAKE2P Key Exchange Salt";

using namespace chip::Inet;
using namespace chip::System;

namespace chip {

enum NetworkProvisioningMsgTypes : uint8_t
{
    kWiFiAssociationRequest = 0,
    kIPAddressAssigned      = 1,
};

CHIP_ERROR RendezvousSession::Init(const RendezvousParameters & params)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    mParams = params;
    VerifyOrExit(mDelegate != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mParams.HasLocalNodeId(), err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(mParams.HasSetupPINCode(), err = CHIP_ERROR_INVALID_ARGUMENT);

    err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#if CONFIG_NETWORK_LAYER_BLE
    {
        Transport::BLE * transport = new Transport::BLE();
        err                        = transport->Init(this, mParams);
        mTransport                 = transport;
        mTransport->Retain();
        transport->Release();
    }
#endif // CONFIG_NETWORK_LAYER_BLE
    SuccessOrExit(err);

    if (mParams.HasDiscriminator() == false)
    {
        err = WaitForPairing(mParams.GetLocalNodeId(), mParams.GetSetupPINCode());
#if CONFIG_DEVICE_LAYER
        DeviceLayer::PlatformMgr().AddEventHandler(ConnectivityHandler, reinterpret_cast<intptr_t>(this));
#endif
    }

exit:
    return err;
}

RendezvousSession::~RendezvousSession()
{
    if (mTransport)
    {
        mTransport->Release();
        mTransport = nullptr;
    }

    mDelegate = nullptr;
}

CHIP_ERROR RendezvousSession::SendMessage(System::PacketBuffer * msgBuf)
{
    CHIP_ERROR err = mPairingInProgress ? SendPairingMessage(msgBuf)
                                        : SendSecureMessage(Protocols::kChipProtocol_NetworkProvisioning,
                                                            NetworkProvisioningMsgTypes::kWiFiAssociationRequest, msgBuf);
    SuccessOrExit(err);

exit:
    if (err != CHIP_NO_ERROR)
    {
        mPairingInProgress ? OnPairingError(err) : OnRendezvousError(err);
    }
    return err;
}
CHIP_ERROR RendezvousSession::SendPairingMessage(System::PacketBuffer * msgBuf)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    PacketHeader header;
    uint16_t headerSize = 0;

    VerifyOrExit(msgBuf != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(msgBuf->Next() == nullptr, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    err = header.Decode(msgBuf->Start(), msgBuf->DataLength(), &headerSize);
    SuccessOrExit(err);

    msgBuf->ConsumeHead(headerSize);
    err = mTransport->SendMessage(header, Header::Flags::None(), Transport::PeerAddress::BLE(), msgBuf);
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR RendezvousSession::SendSecureMessage(Protocols::CHIPProtocolId protocol, uint8_t msgType, System::PacketBuffer * msgBuf)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    PacketHeader packetHeader;
    PayloadHeader payloadHeader;
    MessageAuthenticationCode mac;
    const uint16_t headerSize = payloadHeader.EncodeSizeBytes();
    uint16_t actualEncodedHeaderSize;
    uint8_t * data    = nullptr;
    uint16_t totalLen = 0;
    uint16_t taglen   = 0;

    VerifyOrExit(msgBuf != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(msgBuf->Next() == nullptr, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);
    VerifyOrExit(msgBuf->TotalLength() < kMax_SecureSDU_Length, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);
    VerifyOrExit(CanCastTo<uint16_t>(headerSize + msgBuf->TotalLength()), err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    packetHeader
        .SetSourceNodeId(mParams.GetLocalNodeId())           //
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

    err = mTransport->SendMessage(packetHeader, payloadHeader.GetEncodePacketFlags(), Transport::PeerAddress::BLE(), msgBuf);
    SuccessOrExit(err);

    mSecureMessageIndex++;
    msgBuf = nullptr;

exit:
    return err;
}

void RendezvousSession::OnPairingError(CHIP_ERROR err)
{
    mPairingInProgress = false;
    mDelegate->OnRendezvousError(err);
    mDelegate->OnRendezvousStatusUpdate(RendezvousSessionDelegate::SecurePairingFailed);
}

void RendezvousSession::OnPairingComplete()
{
    mPairingInProgress = false;

    CHIP_ERROR err = mPairingSession.DeriveSecureSession((const unsigned char *) kSpake2pI2RSessionInfo,
                                                         strlen(kSpake2pI2RSessionInfo), mSecureSession);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(Ble, "Failed to initialize a secure session: %s", ErrorStr(err)));

    mDelegate->OnRendezvousStatusUpdate(RendezvousSessionDelegate::SecurePairingSuccess);
    mDelegate->OnRendezvousConnectionOpened();

exit:
    return;
}

void RendezvousSession::OnRendezvousConnectionOpened()
{
    if (mParams.HasDiscriminator())
    {
        CHIP_ERROR err = Pair(mParams.GetLocalNodeId(), mParams.GetSetupPINCode());
        VerifyOrExit(err == CHIP_NO_ERROR, OnPairingError(err));
    }

exit:
    return;
}

void RendezvousSession::OnRendezvousConnectionClosed()
{
    mDelegate->OnRendezvousConnectionClosed();

    if (!mParams.HasDiscriminator() && !mParams.HasConnectionObject())
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
    mDelegate->OnRendezvousError(err);
}

void RendezvousSession::OnRendezvousMessageReceived(PacketBuffer * msgBuf)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = mPairingInProgress ? HandlePairingMessage(msgBuf) : HandleSecureMessage(msgBuf);
    SuccessOrExit(err);

exit:
    if (err != CHIP_NO_ERROR)
    {
        mPairingInProgress ? OnPairingError(err) : OnRendezvousError(err);
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

CHIP_ERROR RendezvousSession::HandleSecureMessage(PacketBuffer * msgBuf)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    PacketHeader packetHeader;
    PayloadHeader payloadHeader;
    MessageAuthenticationCode mac;
    uint16_t headerSize            = 0;
    uint8_t * data                 = nullptr;
    uint8_t * plainText            = nullptr;
    uint16_t len                   = 0;
    uint16_t decodedSize           = 0;
    uint16_t taglen                = 0;
    System::PacketBuffer * origMsg = nullptr;

    err = packetHeader.Decode(msgBuf->Start(), msgBuf->DataLength(), &headerSize);
    SuccessOrExit(err);
    msgBuf->ConsumeHead(headerSize);

    headerSize = payloadHeader.EncodeSizeBytes();
    data       = msgBuf->Start();
    len        = msgBuf->TotalLength();

#if CHIP_SYSTEM_CONFIG_USE_LWIP
    /* This is a workaround for the case where PacketBuffer payload is not
       allocated as an inline buffer to PacketBuffer structure */
    origMsg = msgBuf;
    msgBuf  = PacketBuffer::NewWithAvailableSize(len);
    msgBuf->SetDataLength(len, msgBuf);
#endif
    plainText = msgBuf->Start();

    // TODO: We need length checks here!  https://github.com/project-chip/connectedhomeip/issues/2928
    err = mac.Decode(packetHeader, &data[packetHeader.GetPayloadLength()], kMaxTagLen, &taglen);
    SuccessOrExit(err);

    len = static_cast<uint16_t>(len - taglen);
    msgBuf->SetDataLength(len);

    err = mSecureSession.Decrypt(data, len, plainText, packetHeader, payloadHeader.GetEncodePacketFlags(), mac);
    SuccessOrExit(err);

    err = payloadHeader.Decode(packetHeader.GetFlags(), plainText, headerSize, &decodedSize);
    SuccessOrExit(err);
    VerifyOrExit(headerSize == decodedSize, err = CHIP_ERROR_INCORRECT_STATE);

    msgBuf->ConsumeHead(headerSize);

    if (payloadHeader.GetProtocolID() == Protocols::kChipProtocol_NetworkProvisioning &&
        payloadHeader.GetMessageType() == NetworkProvisioningMsgTypes::kIPAddressAssigned)
    {
        if (!IPAddress::FromString(Uint8::to_const_char(msgBuf->Start()), msgBuf->DataLength(), mDeviceAddress))
        {
            mDelegate->OnRendezvousStatusUpdate(RendezvousSessionDelegate::NetworkProvisioningFailed);
            ExitNow(err = CHIP_ERROR_INVALID_ADDRESS);
        }
        mDelegate->OnRendezvousStatusUpdate(RendezvousSessionDelegate::NetworkProvisioningSuccess);
    }
    // else .. TBD once application dependency on this message has been removed, enable the else condition
    {
        mDelegate->OnRendezvousMessageReceived(msgBuf);
    }

    msgBuf = nullptr;

exit:
    if (origMsg != nullptr)
    {
        PacketBuffer::Free(origMsg);
    }

    return err;
}

CHIP_ERROR RendezvousSession::WaitForPairing(Optional<NodeId> nodeId, uint32_t setupPINCode)
{
    mPairingInProgress = true;
    return mPairingSession.WaitForPairing(setupPINCode, kSpake2p_Iteration_Count, (const unsigned char *) kSpake2pKeyExchangeSalt,
                                          strlen(kSpake2pKeyExchangeSalt), nodeId, 0, this);
}

CHIP_ERROR RendezvousSession::Pair(Optional<NodeId> nodeId, uint32_t setupPINCode)
{
    mPairingInProgress = true;
    return mPairingSession.Pair(setupPINCode, kSpake2p_Iteration_Count, (const unsigned char *) kSpake2pKeyExchangeSalt,
                                strlen(kSpake2pKeyExchangeSalt), nodeId, mNextKeyId++, this);
}

void RendezvousSession::SendIPAddress(const IPAddress & addr)
{
    CHIP_ERROR err                = CHIP_NO_ERROR;
    System::PacketBuffer * buffer = System::PacketBuffer::New();
    char * addrStr                = addr.ToString(Uint8::to_char(buffer->Start()), buffer->AvailableDataLength());
    size_t addrLen                = 0;

    VerifyOrExit(addrStr != nullptr, err = CHIP_ERROR_INVALID_ADDRESS);
    VerifyOrExit(mPairingInProgress == false, err = CHIP_ERROR_INCORRECT_STATE);

    addrLen = strlen(addrStr) + 1;

    VerifyOrExit(CanCastTo<uint16_t>(addrLen), err = CHIP_ERROR_INVALID_ARGUMENT);
    buffer->SetDataLength(static_cast<uint16_t>(addrLen));

    err = SendSecureMessage(Protocols::kChipProtocol_NetworkProvisioning, NetworkProvisioningMsgTypes::kIPAddressAssigned, buffer);
    SuccessOrExit(err);

exit:
    if (CHIP_NO_ERROR != err)
    {
        OnRendezvousError(err);
        PacketBuffer::Free(buffer);
    }
}

#if CONFIG_DEVICE_LAYER
void RendezvousSession::ConnectivityHandler(const DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    RendezvousSession * session = reinterpret_cast<RendezvousSession *>(arg);

    VerifyOrExit(session != nullptr, /**/);
    VerifyOrExit(event->Type == DeviceLayer::DeviceEventType::kInternetConnectivityChange, /**/);
    VerifyOrExit(event->InternetConnectivityChange.IPv4 == DeviceLayer::kConnectivity_Established, /**/);

    IPAddress addr;
    IPAddress::FromString(event->InternetConnectivityChange.address, addr);
    session->SendIPAddress(addr);

exit:
    return;
}
#endif

} // namespace chip
