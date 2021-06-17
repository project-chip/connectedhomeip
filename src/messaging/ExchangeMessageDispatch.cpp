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
 *      This file provides implementation of ExchangeMessageDispatch class.
 */

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <inttypes.h>
#include <memory>

#include <messaging/ExchangeMessageDispatch.h>
#include <messaging/ReliableMessageContext.h>
#include <messaging/ReliableMessageMgr.h>
#include <protocols/secure_channel/Constants.h>
#include <support/CodeUtils.h>

namespace chip {
namespace Messaging {

CHIP_ERROR ExchangeMessageDispatch::SendMessage(SecureSessionHandle session, uint16_t exchangeId, bool isInitiator,
                                                ReliableMessageContext * reliableMessageContext, bool isReliableTransmission,
                                                Protocols::Id protocol, uint8_t type, System::PacketBufferHandle && message)
{
    ReturnErrorCodeIf(!MessagePermitted(protocol.GetProtocolId(), type), CHIP_ERROR_INVALID_ARGUMENT);

    PayloadHeader payloadHeader;
    payloadHeader.SetExchangeID(exchangeId).SetMessageType(protocol, type).SetInitiator(isInitiator);

    // If there is a pending acknowledgment piggyback it on this message.
    if (reliableMessageContext->HasPeerRequestedAck())
    {
        payloadHeader.SetAckId(reliableMessageContext->GetPendingPeerAckId());

        // Set AckPending flag to false since current outgoing message is going to serve as the ack on this exchange.
        reliableMessageContext->SetAckPending(false);

#if !defined(NDEBUG)
        if (!payloadHeader.HasMessageType(Protocols::SecureChannel::MsgType::StandaloneAck))
        {
            ChipLogDetail(ExchangeManager, "Piggybacking Ack for MsgId:%08" PRIX32 " with msg",
                          reliableMessageContext->GetPendingPeerAckId());
        }
#endif
    }

    if (IsReliableTransmissionAllowed() && reliableMessageContext->AutoRequestAck() &&
        reliableMessageContext->GetReliableMessageMgr() != nullptr && isReliableTransmission)
    {
        auto * reliableMessageMgr = reliableMessageContext->GetReliableMessageMgr();

        payloadHeader.SetNeedsAck(true);

        ReliableMessageMgr::RetransTableEntry * entry = nullptr;

        // Add to Table for subsequent sending
        ReturnErrorOnFailure(reliableMessageMgr->AddToRetransTable(reliableMessageContext, &entry));
        auto deleter = [reliableMessageMgr](ReliableMessageMgr::RetransTableEntry * e) {
            reliableMessageMgr->ClearRetransTable(*e);
        };
        std::unique_ptr<ReliableMessageMgr::RetransTableEntry, decltype(deleter)> entryOwner(entry, deleter);

        ReturnErrorOnFailure(PrepareMessage(session, payloadHeader, std::move(message), entryOwner->retainedBuf));
        ReturnErrorOnFailure(SendPreparedMessage(session, entryOwner->retainedBuf));
        reliableMessageMgr->StartRetransmision(entryOwner.release());
    }
    else
    {
        // If the channel itself is providing reliability, let's not request MRP acks
        payloadHeader.SetNeedsAck(false);
        EncryptedPacketBufferHandle preparedMessage;
        ReturnErrorOnFailure(PrepareMessage(session, payloadHeader, std::move(message), preparedMessage));
        ReturnErrorOnFailure(SendPreparedMessage(session, preparedMessage));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ExchangeMessageDispatch::OnMessageReceived(const PayloadHeader & payloadHeader, uint32_t messageId,
                                                      const Transport::PeerAddress & peerAddress,
                                                      ReliableMessageContext * reliableMessageContext)
{
    ReturnErrorCodeIf(!MessagePermitted(payloadHeader.GetProtocolID().GetProtocolId(), payloadHeader.GetMessageType()),
                      CHIP_ERROR_INVALID_ARGUMENT);

    if (IsReliableTransmissionAllowed())
    {
        if (payloadHeader.IsAckMsg() && payloadHeader.GetAckId().HasValue())
        {
            ReturnErrorOnFailure(reliableMessageContext->HandleRcvdAck(payloadHeader.GetAckId().Value()));
        }

        if (payloadHeader.NeedsAck())
        {
            MessageFlags msgFlags;

            // An acknowledgment needs to be sent back to the peer for this message on this exchange,
            // Set the flag in message header indicating an ack requested by peer;
            msgFlags.Set(MessageFlagValues::kPeerRequestedAck);

            // Also set the flag in the exchange context indicating an ack requested;
            reliableMessageContext->SetPeerRequestedAck(true);

            ReturnErrorOnFailure(reliableMessageContext->HandleNeedsAck(messageId, msgFlags));
        }
    }

    return CHIP_NO_ERROR;
}

} // namespace Messaging
} // namespace chip
