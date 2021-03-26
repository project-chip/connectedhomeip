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
#include <protocols/secure_channel/SessionEstablishmentTransport.h>

#include <transport/BLE.h>

namespace chip {
namespace Messaging {

CHIP_ERROR SessionEstablishmentTransport::SendMessageImpl(SecureSessionHandle session, PayloadHeader & payloadHeader,
                                                          System::PacketBufferHandle && message,
                                                          EncryptedPacketBufferHandle * retainedMessage)
{
    ChipLogProgress(Ble, "SessionEstablishmentTransport::SendMessageImpl mBLETransport %p, mTransportMgr %p", mBLETransport,
                    mTransportMgr);
    ReturnErrorOnFailure(payloadHeader.EncodeBeforeData(message));
    if (mBLETransport != nullptr)
    {
        ChipLogProgress(Ble, "Sending message using BLE transport");
        return mBLETransport->SendMessage(PacketHeader(), Transport::PeerAddress::BLE(), std::move(message));
    }
    else if (mTransportMgr != nullptr)
    {
        return mTransportMgr->SendMessage(PacketHeader(), mPeerAddress, std::move(message));
    }

    return CHIP_ERROR_INCORRECT_STATE;
}

bool SessionEstablishmentTransport::MessagePermitted(uint16_t protocol, uint8_t type)
{
    switch (protocol)
    {
    case Protocols::SecureChannel::Id.GetProtocolId():
        switch (static_cast<Protocols::SecureChannel::MsgType>(type))
        {
        case Protocols::SecureChannel::MsgType::PBKDFParamRequest:
        case Protocols::SecureChannel::MsgType::PBKDFParamResponse:
        case Protocols::SecureChannel::MsgType::PASE_Spake2p1:
        case Protocols::SecureChannel::MsgType::PASE_Spake2p2:
        case Protocols::SecureChannel::MsgType::PASE_Spake2p3:
        case Protocols::SecureChannel::MsgType::PASE_Spake2pError:
        case Protocols::SecureChannel::MsgType::CASE_SigmaR1:
        case Protocols::SecureChannel::MsgType::CASE_SigmaR2:
        case Protocols::SecureChannel::MsgType::CASE_SigmaR3:
        case Protocols::SecureChannel::MsgType::CASE_SigmaErr:
            return true;

        default:
            break;
        }
    }
    return false;
}

} // namespace Messaging
} // namespace chip
