/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *      This file implements an object for a Matter User Directed Commissioning unsolicited
 *      initiator (client).
 *
 */

#include "UserDirectedCommissioning.h"

#ifdef __ZEPHYR__
#include <zephyr/kernel.h>
#endif // __ZEPHYR__

#include <unistd.h>

namespace chip {
namespace Protocols {
namespace UserDirectedCommissioning {

CHIP_ERROR UserDirectedCommissioningClient::SendUDCMessage(TransportMgrBase * transportMgr, System::PacketBufferHandle && payload,
                                                           chip::Transport::PeerAddress peerAddress)
{
    ReturnErrorOnFailure(EncodeUDCMessage(payload));

    ChipLogProgress(Inet, "Sending UDC msg");

    // send UDC message 5 times per spec (no ACK on this message)
    for (unsigned int i = 0; i < 5; i++)
    {
        auto msgCopy = payload.CloneData();
        VerifyOrReturnError(!msgCopy.IsNull(), CHIP_ERROR_NO_MEMORY);

        auto err = transportMgr->SendMessage(peerAddress, std::move(msgCopy));
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "UDC SendMessage failed: %" CHIP_ERROR_FORMAT, err.Format());
            return err;
        }
        // Zephyr doesn't provide usleep implementation.
#ifdef __ZEPHYR__
        k_usleep(100 * 1000); // 100ms
#else
        usleep(100 * 1000); // 100ms
#endif // __ZEPHYR__
    }

    ChipLogProgress(Inet, "UDC msg sent");
    return CHIP_NO_ERROR;
}

CHIP_ERROR UserDirectedCommissioningClient::EncodeUDCMessage(const System::PacketBufferHandle & payload)
{
    PayloadHeader payloadHeader;
    PacketHeader packetHeader;

    payloadHeader.SetMessageType(MsgType::IdentificationDeclaration).SetInitiator(true).SetNeedsAck(false);

    VerifyOrReturnError(!payload.IsNull(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(!payload->HasChainedBuffer(), CHIP_ERROR_INVALID_MESSAGE_LENGTH);
    VerifyOrReturnError(payload->TotalLength() <= kMaxAppMessageLen, CHIP_ERROR_MESSAGE_TOO_LONG);

    ReturnErrorOnFailure(payloadHeader.EncodeBeforeData(payload));

    ReturnErrorOnFailure(packetHeader.EncodeBeforeData(payload));

    return CHIP_NO_ERROR;
}

} // namespace UserDirectedCommissioning
} // namespace Protocols
} // namespace chip
