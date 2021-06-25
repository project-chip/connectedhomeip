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

CHIP_ERROR SessionEstablishmentExchangeDispatch::PrepareMessage(SecureSessionHandle session, PayloadHeader & payloadHeader,
                                                                System::PacketBufferHandle && message,
                                                                EncryptedPacketBufferHandle & preparedMessage)
{
    PacketHeader packetHeader;
    ReturnErrorOnFailure(payloadHeader.EncodeBeforeData(message));
    ReturnErrorOnFailure(packetHeader.EncodeBeforeData(message));

    preparedMessage = EncryptedPacketBufferHandle::MarkEncrypted(std::move(message));
    return CHIP_NO_ERROR;
}

CHIP_ERROR SessionEstablishmentExchangeDispatch::SendPreparedMessage(SecureSessionHandle session,
                                                                     const EncryptedPacketBufferHandle & preparedMessage) const
{
    ReturnErrorCodeIf(mTransportMgr == nullptr, CHIP_ERROR_INCORRECT_STATE);
    return mTransportMgr->SendMessage(mPeerAddress, preparedMessage.CastToWritable());
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
