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

#include <support/CodeUtils.h>
#include <support/ErrorStr.h>
#include <transport/BLE.h>
#include <transport/RendezvousSession.h>

static const size_t kMax_SecureSDU_Length          = 1024;
static constexpr uint32_t kSpake2p_Iteration_Count = 50000;
static const char * kSpake2pKeyExchangeSalt        = "SPAKE2P Key Exchange Salt";

namespace chip {

CHIP_ERROR RendezvousSession::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mDelegate != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mParams.HasLocalNodeId(), err = CHIP_ERROR_INCORRECT_STATE);

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
    CHIP_ERROR err = CHIP_NO_ERROR;
    err            = mPairingInProgress ? SendPairingMessage(msgBuf) : SendSecureMessage(msgBuf);
    if (err != CHIP_NO_ERROR)
    {
        mPairingInProgress ? OnPairingError(err) : OnRendezvousError(err);
    }

    return err;
}

CHIP_ERROR RendezvousSession::SendPairingMessage(System::PacketBuffer * msgBuf)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    MessageHeader header;
    size_t headerSize = 0;

    VerifyOrExit(msgBuf != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(msgBuf->Next() == nullptr, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    err = header.Decode(msgBuf->Start(), msgBuf->DataLength(), &headerSize);
    SuccessOrExit(err);

    msgBuf->ConsumeHead(headerSize);
    err = mTransport->SendMessage(header, Transport::PeerAddress::BLE(), msgBuf);
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR RendezvousSession::SendSecureMessage(System::PacketBuffer * msgBuf)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    MessageHeader header;
    const size_t headerSize = header.EncryptedHeaderSizeBytes();
    size_t actualEncodedHeaderSize;
    uint8_t * data  = nullptr;
    size_t totalLen = 0;
    size_t taglen   = 0;

    VerifyOrExit(msgBuf != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(msgBuf->Next() == nullptr, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);
    VerifyOrExit(msgBuf->TotalLength() < kMax_SecureSDU_Length, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    header
        .SetSourceNodeId(mParams.GetLocalNodeId())           //
        .SetMessageId(mSecureMessageIndex)                   //
        .SetEncryptionKeyID(mPairingSession.GetLocalKeyId()) //
        .SetPayloadLength(headerSize + msgBuf->TotalLength());

    VerifyOrExit(msgBuf->EnsureReservedSize(headerSize), err = CHIP_ERROR_NO_MEMORY);

    msgBuf->SetStart(msgBuf->Start() - headerSize);
    data     = msgBuf->Start();
    totalLen = msgBuf->TotalLength();

    err = header.EncodeEncryptedHeader(data, totalLen, &actualEncodedHeaderSize);
    SuccessOrExit(err);

    err = mSecureSession.Encrypt(data, totalLen, data, header);
    SuccessOrExit(err);

    err = header.EncodeMACTag(&data[totalLen], kMaxTagLen, &taglen);
    SuccessOrExit(err);

    msgBuf->SetDataLength(totalLen + taglen);

    err = mTransport->SendMessage(header, Transport::PeerAddress::BLE(), msgBuf);
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
}

void RendezvousSession::OnPairingComplete()
{
    mPairingInProgress = false;

    CHIP_ERROR err = mPairingSession.DeriveSecureSession((const unsigned char *) kSpake2pI2RSessionInfo,
                                                         strlen(kSpake2pI2RSessionInfo), mSecureSession);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Ble, "Failed to initialize a secure session: %s", ErrorStr(err));
        return;
    }

    mDelegate->OnRendezvousConnectionOpened();
}

void RendezvousSession::OnRendezvousConnectionOpened()
{
    if (mParams.HasDiscriminator())
    {
        CHIP_ERROR err = Pair(mParams.GetLocalNodeId(), mParams.GetSetupPINCode());
        if (err != CHIP_NO_ERROR)
        {
            OnPairingError(err);
        }
    }
}

void RendezvousSession::OnRendezvousConnectionClosed()
{
    mDelegate->OnRendezvousConnectionClosed();

    if (!mParams.HasDiscriminator())
    {
        CHIP_ERROR err = WaitForPairing(mParams.GetLocalNodeId(), mParams.GetSetupPINCode());
        if (err != CHIP_NO_ERROR)
        {
            OnPairingError(err);
        }
    }
}

void RendezvousSession::OnRendezvousError(CHIP_ERROR err)
{
    mDelegate->OnRendezvousError(err);
}

void RendezvousSession::OnRendezvousMessageReceived(PacketBuffer * msgBuf)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = mPairingInProgress ? HandlePairingMessage(msgBuf) : HandleSecureMessage(msgBuf);
    if (err != CHIP_NO_ERROR)
    {
        mPairingInProgress ? OnPairingError(err) : OnRendezvousError(err);
    }
}

CHIP_ERROR RendezvousSession::HandlePairingMessage(PacketBuffer * msgBuf)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    MessageHeader header;
    size_t headerSize = 0;

    err = header.Decode(msgBuf->Start(), msgBuf->DataLength(), &headerSize);
    SuccessOrExit(err);

    msgBuf->ConsumeHead(headerSize);

    err = mPairingSession.HandlePeerMessage(header, msgBuf);
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR RendezvousSession::HandleSecureMessage(PacketBuffer * msgBuf)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    MessageHeader header;
    size_t headerSize              = 0;
    uint8_t * data                 = nullptr;
    uint8_t * plainText            = nullptr;
    uint16_t len                   = 0;
    size_t decodedSize             = 0;
    size_t taglen                  = 0;
    System::PacketBuffer * origMsg = nullptr;

    err = header.Decode(msgBuf->Start(), msgBuf->DataLength(), &headerSize);
    SuccessOrExit(err);
    msgBuf->ConsumeHead(headerSize);

    headerSize = header.EncryptedHeaderSizeBytes();
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

    err = header.DecodeMACTag(&data[header.GetPayloadLength()], kMaxTagLen, &taglen);
    SuccessOrExit(err);

    len -= taglen;
    msgBuf->SetDataLength(len);

    err = mSecureSession.Decrypt(data, len, plainText, header);
    SuccessOrExit(err);

    err = header.DecodeEncryptedHeader(plainText, headerSize, &decodedSize);
    SuccessOrExit(err);
    VerifyOrExit(headerSize == decodedSize, err = CHIP_ERROR_INCORRECT_STATE);

    msgBuf->ConsumeHead(headerSize);

    mDelegate->OnRendezvousMessageReceived(msgBuf);
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

} // namespace chip
