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

#include <errno.h>
#include <inttypes.h>
#include <memory>

#include <lib/support/CodeUtils.h>
#include <messaging/ExchangeMessageDispatch.h>
#include <messaging/ReliableMessageContext.h>
#include <messaging/ReliableMessageMgr.h>
#include <protocols/secure_channel/Constants.h>

namespace chip {
namespace Messaging {

CHIP_ERROR ExchangeMessageDispatch::SendMessage(SessionManager * sessionManager, const SessionHandle & session, uint16_t exchangeId,
                                                bool isInitiator, ReliableMessageContext * reliableMessageContext,
                                                bool isReliableTransmission, Protocols::Id protocol, uint8_t type,
                                                System::PacketBufferHandle && message)
{
    ReturnErrorCodeIf(!MessagePermitted(protocol.GetProtocolId(), type), CHIP_ERROR_INVALID_ARGUMENT);

    PayloadHeader payloadHeader;
    payloadHeader.SetExchangeID(exchangeId).SetMessageType(protocol, type).SetInitiator(isInitiator);

    // If there is a pending acknowledgment piggyback it on this message.
    if (reliableMessageContext->HasPiggybackAckPending())
    {
        payloadHeader.SetAckMessageCounter(reliableMessageContext->TakePendingPeerAckMessageCounter());

#if !defined(NDEBUG)
        if (!payloadHeader.HasMessageType(Protocols::SecureChannel::MsgType::StandaloneAck))
        {
            ChipLogDetail(ExchangeManager,
                          "Piggybacking Ack for MessageCounter:" ChipLogFormatMessageCounter
                          " on exchange: " ChipLogFormatExchangeId,
                          payloadHeader.GetAckMessageCounter().Value(), ChipLogValueExchangeId(exchangeId, isInitiator));
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

        ReturnErrorOnFailure(sessionManager->PrepareMessage(session, payloadHeader, std::move(message), entryOwner->retainedBuf));
        CHIP_ERROR err = sessionManager->SendPreparedMessage(session, entryOwner->retainedBuf);
        if (err == CHIP_ERROR_POSIX(ENOBUFS))
        {
            // sendmsg on BSD-based systems never blocks, no matter how the
            // socket is configured, and will return ENOBUFS in situation in
            // which Linux, for example, blocks.
            //
            // This is typically a transient situation, so we pretend like this
            // packet drop happened somewhere on the network instead of inside
            // sendmsg and will just resend it in the normal MRP way later.
            ChipLogError(ExchangeManager, "Ignoring ENOBUFS: %" CHIP_ERROR_FORMAT " on exchange " ChipLogFormatExchangeId,
                         err.Format(), ChipLogValueExchangeId(exchangeId, isInitiator));
            err = CHIP_NO_ERROR;
        }
        ReturnErrorOnFailure(err);
        reliableMessageMgr->StartRetransmision(entryOwner.release());
    }
    else
    {
        // If the channel itself is providing reliability, let's not request MRP acks
        payloadHeader.SetNeedsAck(false);
        EncryptedPacketBufferHandle preparedMessage;
        ReturnErrorOnFailure(sessionManager->PrepareMessage(session, payloadHeader, std::move(message), preparedMessage));
        ReturnErrorOnFailure(sessionManager->SendPreparedMessage(session, preparedMessage));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ExchangeMessageDispatch::OnMessageReceived(uint32_t messageCounter, const PayloadHeader & payloadHeader,
                                                      MessageFlags msgFlags, ReliableMessageContext * reliableMessageContext)
{
    ReturnErrorCodeIf(!MessagePermitted(payloadHeader.GetProtocolID().GetProtocolId(), payloadHeader.GetMessageType()),
                      CHIP_ERROR_INVALID_ARGUMENT);

    if (IsReliableTransmissionAllowed() && !reliableMessageContext->GetExchangeContext()->IsGroupExchangeContext())
    {
        if (!msgFlags.Has(MessageFlagValues::kDuplicateMessage) && payloadHeader.IsAckMsg() &&
            payloadHeader.GetAckMessageCounter().HasValue())
        {
            reliableMessageContext->HandleRcvdAck(payloadHeader.GetAckMessageCounter().Value());
        }

        if (payloadHeader.NeedsAck())
        {
            // An acknowledgment needs to be sent back to the peer for this message on this exchange,

            ReturnErrorOnFailure(reliableMessageContext->HandleNeedsAck(messageCounter, msgFlags));
        }
    }

    return CHIP_NO_ERROR;
}

} // namespace Messaging
} // namespace chip
