/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    All rights reserved.
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
 *      This file implements an object for a CHIP Echo unsolicitied
 *      initiator (client).
 *
 */

#include "UserDirectedCommissioning.h"

namespace chip {
namespace Protocols {
namespace UserDirectedCommissioning {

CHIP_ERROR UserDirectedCommissioningClient::SendUDCMessage(TransportMgrBase * transportMgr, System::PacketBufferHandle && payload,
                                                           chip::Inet::IPAddress commissioner, uint16_t port)
{
    CHIP_ERROR err;

    PayloadHeader payloadHeader;
    PacketHeader packetHeader;

    payloadHeader.SetMessageType(Protocols::UserDirectedCommissioning::Id, (uint8_t) MsgType::IdentificationDeclaration)
        .SetInitiator(true);
    payloadHeader.SetNeedsAck(false);

    VerifyOrReturnError(!payload.IsNull(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(!payload->HasChainedBuffer(), CHIP_ERROR_INVALID_MESSAGE_LENGTH);
    VerifyOrReturnError(payload->TotalLength() <= kMaxAppMessageLen, CHIP_ERROR_MESSAGE_TOO_LONG);

    ReturnErrorOnFailure(payloadHeader.EncodeBeforeData(payload));

    packetHeader.SetEncryptionType(Header::EncryptionType::kEncryptionTypeNone);

    uint16_t totalLen = payload->TotalLength();
    uint16_t taglen   = 0;
    ChipLogDetail(Inet, "SendUDCMessage totalLen=%d taglen=%d", totalLen, taglen);

    payload->SetDataLength(static_cast<uint16_t>(totalLen + taglen));

    ReturnErrorOnFailure(packetHeader.EncodeBeforeData(payload));

    chip::Transport::PeerAddress peerAddress;
    peerAddress = chip::Transport::PeerAddress::UDP(commissioner, port, INET_NULL_INTERFACEID);

    ChipLogProgress(Inet, "Sending secure msg on generic transport");
    err = transportMgr->SendMessage(peerAddress, std::move(payload));

    ChipLogProgress(Inet, "Secure msg send status %s", ErrorStr(err));
    return err;
}

} // namespace UserDirectedCommissioning
} // namespace Protocols
} // namespace chip
