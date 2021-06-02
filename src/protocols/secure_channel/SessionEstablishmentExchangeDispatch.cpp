/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *      This file provides implementation of Application Channel class.
 */

#include <core/CHIPError.h>
#include <protocols/secure_channel/Constants.h>
#include <protocols/secure_channel/SessionEstablishmentExchangeDispatch.h>

namespace chip {

using namespace Messaging;

CHIP_ERROR SessionEstablishmentExchangeDispatch::SendMessageImpl(SecureSessionHandle session, PayloadHeader & payloadHeader,
                                                                 System::PacketBufferHandle && message,
                                                                 EncryptedPacketBufferHandle * retainedMessage)
{
    ReturnErrorCodeIf(mTransportMgr == nullptr, CHIP_ERROR_INCORRECT_STATE);
    PacketHeader packetHeader;

    ReturnErrorOnFailure(payloadHeader.EncodeBeforeData(message));
    ReturnErrorOnFailure(packetHeader.EncodeBeforeData(message));

    if (retainedMessage != nullptr)
    {
        *retainedMessage = EncryptedPacketBufferHandle::MarkEncrypted(message.Retain());
        ChipLogError(Inet, "RETAINED IN SESS: %p %d", retainedMessage, (*retainedMessage).IsNull());
    }
    return mTransportMgr->SendMessage(mPeerAddress, std::move(message));
}

CHIP_ERROR SessionEstablishmentExchangeDispatch::ResendMessage(SecureSessionHandle session, EncryptedPacketBufferHandle && message,
                                                               EncryptedPacketBufferHandle * retainedMessage) const
{
    ReturnErrorCodeIf(mTransportMgr == nullptr, CHIP_ERROR_INCORRECT_STATE);

    // Our send path needs a (writable) PacketBuffer, so get that from the
    // EncryptedPacketBufferHandle.  Note that we have to do this before we set
    // *retainedMessage, because 'message' and '*retainedMessage' might be the
    // same memory location and we have to guarantee that we move out of
    // 'message' before we write to *retainedMessage.
    System::PacketBufferHandle writableBuf(std::move(message).CastToWritable());
    if (retainedMessage != nullptr)
    {
        *retainedMessage = EncryptedPacketBufferHandle::MarkEncrypted(writableBuf.Retain());
    }
    return mTransportMgr->SendMessage(mPeerAddress, std::move(writableBuf));
}

CHIP_ERROR SessionEstablishmentExchangeDispatch::OnMessageReceived(const PayloadHeader & payloadHeader, uint32_t messageId,
                                                                   const Transport::PeerAddress & peerAddress,
                                                                   ReliableMessageContext * reliableMessageContext)
{
    mPeerAddress = peerAddress;
    return ExchangeMessageDispatch::OnMessageReceived(payloadHeader, messageId, peerAddress, reliableMessageContext);
}

bool SessionEstablishmentExchangeDispatch::MessagePermitted(uint16_t protocol, uint8_t type)
{
    switch (protocol)
    {
    case Protocols::SecureChannel::Id.GetProtocolId():
        switch (type)
        {
        case static_cast<uint8_t>(Protocols::SecureChannel::MsgType::StandaloneAck):
        case static_cast<uint8_t>(Protocols::SecureChannel::MsgType::PBKDFParamRequest):
        case static_cast<uint8_t>(Protocols::SecureChannel::MsgType::PBKDFParamResponse):
        case static_cast<uint8_t>(Protocols::SecureChannel::MsgType::PASE_Spake2p1):
        case static_cast<uint8_t>(Protocols::SecureChannel::MsgType::PASE_Spake2p2):
        case static_cast<uint8_t>(Protocols::SecureChannel::MsgType::PASE_Spake2p3):
        case static_cast<uint8_t>(Protocols::SecureChannel::MsgType::PASE_Spake2pError):
        case static_cast<uint8_t>(Protocols::SecureChannel::MsgType::CASE_SigmaR1):
        case static_cast<uint8_t>(Protocols::SecureChannel::MsgType::CASE_SigmaR2):
        case static_cast<uint8_t>(Protocols::SecureChannel::MsgType::CASE_SigmaR3):
        case static_cast<uint8_t>(Protocols::SecureChannel::MsgType::CASE_SigmaErr):
            return true;

        default:
            break;
        }
        break;

    default:
        break;
    }
    return false;
}

} // namespace chip
