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
 *      This file provides implementation of ExchangeTransport class.
 */

#include <messaging/ExchangeTransport.h>
#include <messaging/ReliableMessageContext.h>
#include <messaging/ReliableMessageMgr.h>
#include <support/CodeUtils.h>

namespace chip {
namespace Messaging {

CHIP_ERROR ExchangeTransport::SendMessage(SecureSessionHandle session, ExchangeTransport::ExchangeInfo & exchangeInfo,
                                          ReliableMessageContext & rmCtxt, bool isReliableTransmission, Protocols::Id protocol,
                                          uint8_t type, System::PacketBufferHandle message)
{
    ReturnErrorCodeIf(!MessagePermitted(protocol.GetProtocolId(), type), CHIP_ERROR_INVALID_ARGUMENT);

    PayloadHeader payloadHeader;
    payloadHeader.SetExchangeID(exchangeInfo.mExchangeId).SetMessageType(protocol, type).SetInitiator(exchangeInfo.mInitiator);

    // If there is a pending acknowledgment piggyback it on this message.
    if (rmCtxt.HasPeerRequestedAck())
    {
        payloadHeader.SetAckId(rmCtxt.mPendingPeerAckId);

        // Set AckPending flag to false since current outgoing message is going to serve as the ack on this exchange.
        rmCtxt.SetAckPending(false);
    }

    CHIP_ERROR err = CHIP_NO_ERROR;
    if (!IsChannelReliable() && rmCtxt.AutoRequestAck() && mReliableMessageMgr != nullptr && isReliableTransmission)
    {
        payloadHeader.SetNeedsAck(true);

        ReliableMessageMgr::RetransTableEntry * entry = nullptr;

        // Add to Table for subsequent sending
        ReturnErrorOnFailure(mReliableMessageMgr->AddToRetransTable(&rmCtxt, &entry));

        err = SendMessageImpl(session, payloadHeader, std::move(message), &entry->retainedBuf);
        if (err != CHIP_NO_ERROR)
        {
            // Remove from table
            mReliableMessageMgr->ClearRetransTable(*entry);
        }
        else
        {
            mReliableMessageMgr->StartRetransmision(entry);
        }
    }
    else
    {
        // If the channel itself is providing reliability, let's not request CRMP acks
        payloadHeader.SetNeedsAck(false);
        ChipLogProgress(ExchangeManager, "Calling transport sendmessage");
        err = SendMessageImpl(session, payloadHeader, std::move(message), nullptr);
    }

    return err;
}

CHIP_ERROR ExchangeTransport::OnMessageReceived(uint16_t protocol, uint8_t type, ReliableMessageContext & rmCtxt,
                                                MessageReliabilityInfo & rmInfo)
{
    ReturnErrorCodeIf(!MessagePermitted(protocol, type), CHIP_ERROR_INVALID_ARGUMENT);

    if (!IsChannelReliable() && mReliableMessageMgr != nullptr)
    {
        if (rmInfo.mHasAck)
        {
            ReturnErrorOnFailure(rmCtxt.HandleRcvdAck(rmInfo.mAckId));
        }

        if (rmInfo.mNeedsAck)
        {
            MessageFlags msgFlags;

            // An acknowledgment needs to be sent back to the peer for this message on this exchange,
            // Set the flag in message header indicating an ack requested by peer;
            msgFlags.Set(MessageFlagValues::kPeerRequestedAck);

            // Also set the flag in the exchange context indicating an ack requested;
            rmCtxt.SetPeerRequestedAck(true);

            ReturnErrorOnFailure(rmCtxt.HandleNeedsAck(rmInfo.mMessageId, msgFlags));
        }
    }

    return CHIP_NO_ERROR;
}

} // namespace Messaging
} // namespace chip
