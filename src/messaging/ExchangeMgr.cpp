/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 *      This file implements the ExchangeManager class.
 *
 */

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <cstring>
#include <inttypes.h>
#include <stddef.h>

#include <crypto/RandUtils.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/support/CHIPFaultInjection.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <protocols/Protocols.h>

using namespace chip::Encoding;
using namespace chip::Inet;
using namespace chip::System;

namespace chip {
namespace Messaging {

/**
 *  Constructor for the ExchangeManager class.
 *  It sets the state to kState_NotInitialized.
 *
 *  @note
 *    The class must be initialized via ExchangeManager::Init()
 *    prior to use.
 *
 */
ExchangeManager::ExchangeManager() : mReliableMessageMgr(mContextPool)
{
    mState = State::kState_NotInitialized;
}

CHIP_ERROR ExchangeManager::Init(SessionManager * sessionManager)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrReturnError(mState == State::kState_NotInitialized, err = CHIP_ERROR_INCORRECT_STATE);

    mSessionManager = sessionManager;

    mNextExchangeId = chip::Crypto::GetRandU16();
    mNextKeyId      = 0;

    for (auto & handler : UMHandlerPool)
    {
        // Mark all handlers as unallocated.  This handles both initial
        // initialization and the case when the consumer shuts us down and
        // then re-initializes without removing registered handlers.
        handler.Reset();
    }

    sessionManager->SetMessageDelegate(this);

    mReliableMessageMgr.Init(sessionManager->SystemLayer());

    mState = State::kState_Initialized;

    return err;
}

void ExchangeManager::Shutdown()
{
    VerifyOrReturn(mState != State::kState_NotInitialized);

    mReliableMessageMgr.Shutdown();

    if (mSessionManager != nullptr)
    {
        mSessionManager->SetMessageDelegate(nullptr);
        mSessionManager = nullptr;
    }

    mState = State::kState_NotInitialized;
}

ExchangeContext * ExchangeManager::NewContext(const SessionHandle & session, ExchangeDelegate * delegate, bool isInitiator)
{
    if (!session->IsActiveSession())
    {
        // Disallow creating exchange on an inactive session
        ChipLogError(ExchangeManager, "NewContext failed: session inactive");
        return nullptr;
    }
    return mContextPool.CreateObject(this, mNextExchangeId++, session, isInitiator, delegate);
}

CHIP_ERROR ExchangeManager::RegisterUnsolicitedMessageHandlerForProtocol(Protocols::Id protocolId,
                                                                         UnsolicitedMessageHandler * handler)
{
    return RegisterUMH(protocolId, kAnyMessageType, handler);
}

CHIP_ERROR ExchangeManager::RegisterUnsolicitedMessageHandlerForType(Protocols::Id protocolId, uint8_t msgType,
                                                                     UnsolicitedMessageHandler * handler)
{
    return RegisterUMH(protocolId, static_cast<int16_t>(msgType), handler);
}

CHIP_ERROR ExchangeManager::UnregisterUnsolicitedMessageHandlerForProtocol(Protocols::Id protocolId)
{
    return UnregisterUMH(protocolId, kAnyMessageType);
}

CHIP_ERROR ExchangeManager::UnregisterUnsolicitedMessageHandlerForType(Protocols::Id protocolId, uint8_t msgType)
{
    return UnregisterUMH(protocolId, static_cast<int16_t>(msgType));
}

CHIP_ERROR ExchangeManager::RegisterUMH(Protocols::Id protocolId, int16_t msgType, UnsolicitedMessageHandler * handler)
{
    UnsolicitedMessageHandlerSlot * selected = nullptr;

    for (auto & umh : UMHandlerPool)
    {
        if (!umh.IsInUse())
        {
            if (selected == nullptr)
                selected = &umh;
        }
        else if (umh.Matches(protocolId, msgType))
        {
            umh.Handler = handler;
            return CHIP_NO_ERROR;
        }
    }

    if (selected == nullptr)
        return CHIP_ERROR_TOO_MANY_UNSOLICITED_MESSAGE_HANDLERS;

    selected->Handler     = handler;
    selected->ProtocolId  = protocolId;
    selected->MessageType = msgType;

    SYSTEM_STATS_INCREMENT(chip::System::Stats::kExchangeMgr_NumUMHandlers);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ExchangeManager::UnregisterUMH(Protocols::Id protocolId, int16_t msgType)
{
    for (auto & umh : UMHandlerPool)
    {
        if (umh.IsInUse() && umh.Matches(protocolId, msgType))
        {
            umh.Reset();
            SYSTEM_STATS_DECREMENT(chip::System::Stats::kExchangeMgr_NumUMHandlers);
            return CHIP_NO_ERROR;
        }
    }

    return CHIP_ERROR_NO_UNSOLICITED_MESSAGE_HANDLER;
}

void ExchangeManager::OnMessageReceived(const PacketHeader & packetHeader, const PayloadHeader & payloadHeader,
                                        const SessionHandle & session, DuplicateMessage isDuplicate,
                                        System::PacketBufferHandle && msgBuf)
{
    UnsolicitedMessageHandlerSlot * matchingUMH = nullptr;

#if CHIP_PROGRESS_LOGGING
    auto * protocolName = Protocols::GetProtocolName(payloadHeader.GetProtocolID());
    auto * msgTypeName  = Protocols::GetMessageTypeName(payloadHeader.GetProtocolID(), payloadHeader.GetMessageType());

    //
    // 32-bit value maximum = 10 chars + text preamble (6) + trailer (1) + null (1) + 2 buffer = 20
    //
    char ackBuf[20];
    ackBuf[0] = '\0';
    if (payloadHeader.GetAckMessageCounter().HasValue())
    {
        snprintf(ackBuf, sizeof(ackBuf), " (Ack:" ChipLogFormatMessageCounter ")", payloadHeader.GetAckMessageCounter().Value());
    }

    CompressedFabricId compressedFabricId = 0;
    if (session->IsSecureSession() && mSessionManager->GetFabricTable() != nullptr)
    {
        auto fabricInfo = mSessionManager->GetFabricTable()->FindFabricWithIndex(session->AsSecureSession()->GetFabricIndex());
        if (fabricInfo)
        {
            compressedFabricId = fabricInfo->GetCompressedFabricId();
        }
    }

    //
    // Legend that can be used to decode this log line can be found in README.md
    //
    ChipLogProgress(ExchangeManager,
                    ">>> [E:" ChipLogFormatExchangeId " M:" ChipLogFormatMessageCounter "%s] (%s) Msg RX from %u:" ChipLogFormatX64
                    " [%04X] --- Type %04x:%02x (%s:%s)",
                    ChipLogValueExchangeIdFromReceivedHeader(payloadHeader), packetHeader.GetMessageCounter(), ackBuf,
                    Transport::GetSessionTypeString(session), session->GetFabricIndex(),
                    ChipLogValueX64(session->GetPeer().GetNodeId()), static_cast<uint16_t>(compressedFabricId),
                    payloadHeader.GetProtocolID().GetProtocolId(), payloadHeader.GetMessageType(), protocolName, msgTypeName);
#endif

    MessageFlags msgFlags;
    if (isDuplicate == DuplicateMessage::Yes)
    {
        msgFlags.Set(MessageFlagValues::kDuplicateMessage);
    }

    // Skip retrieval of exchange for group message since no exchange is stored
    // for group msg (optimization)
    if (!packetHeader.IsGroupSession())
    {
        // Search for an existing exchange that the message applies to. If a match is found...
        bool found = false;
        mContextPool.ForEachActiveObject([&](auto * ec) {
            if (ec->MatchExchange(session, packetHeader, payloadHeader))
            {
                ChipLogDetail(ExchangeManager, "Found matching exchange: " ChipLogFormatExchange ", Delegate: %p",
                              ChipLogValueExchange(ec), ec->GetDelegate());

                // Matched ExchangeContext; send to message handler.
                ec->HandleMessage(packetHeader.GetMessageCounter(), payloadHeader, msgFlags, std::move(msgBuf));
                found = true;
                return Loop::Break;
            }
            return Loop::Continue;
        });

        if (found)
        {
            return;
        }
    }
    else
    {
        ChipLogProgress(ExchangeManager, "Received Groupcast Message with GroupId 0x%04X (%d)",
                        packetHeader.GetDestinationGroupId().Value(), packetHeader.GetDestinationGroupId().Value());
    }

    // Do not handle messages that don't match an existing exchange on an
    // inactive session, since we should not be creating new exchanges there.
    if (!session->IsActiveSession())
    {
        ChipLogProgress(ExchangeManager, "Dropping message on inactive session that does not match an existing exchange");
        return;
    }

    // If it's not a duplicate message, search for an unsolicited message handler if it is marked as being sent by an initiator.
    // Since we didn't find an existing exchange that matches the message, it must be an unsolicited message. However all
    // unsolicited messages must be marked as being from an initiator.
    if (!msgFlags.Has(MessageFlagValues::kDuplicateMessage) && payloadHeader.IsInitiator())
    {
        // Search for an unsolicited message handler that can handle the message. Prefer handlers that can explicitly
        // handle the message type over handlers that handle all messages for a profile.
        matchingUMH = nullptr;

        for (auto & umh : UMHandlerPool)
        {
            if (umh.IsInUse() && payloadHeader.HasProtocol(umh.ProtocolId))
            {
                if (umh.MessageType == payloadHeader.GetMessageType())
                {
                    matchingUMH = &umh;
                    break;
                }

                if (umh.MessageType == kAnyMessageType)
                    matchingUMH = &umh;
            }
        }
    }
    // Discard the message if it isn't marked as being sent by an initiator and the message does not need to send
    // an ack to the peer.
    else if (!payloadHeader.NeedsAck())
    {
        // Using same error message for all errors to reduce code size.
        ChipLogError(ExchangeManager, "OnMessageReceived failed, err = %" CHIP_ERROR_FORMAT,
                     CHIP_ERROR_UNSOLICITED_MSG_NO_ORIGINATOR.Format());
        return;
    }

    // If we found a handler, create an exchange to handle the message.
    if (matchingUMH != nullptr)
    {
        ExchangeDelegate * delegate = nullptr;

        // Fetch delegate from the handler
        CHIP_ERROR err = matchingUMH->Handler->OnUnsolicitedMessageReceived(payloadHeader, delegate);
        if (err != CHIP_NO_ERROR)
        {
            // Using same error message for all errors to reduce code size.
            ChipLogError(ExchangeManager, "OnMessageReceived failed, err = %" CHIP_ERROR_FORMAT, err.Format());
            SendStandaloneAckIfNeeded(packetHeader, payloadHeader, session, msgFlags, std::move(msgBuf));
            return;
        }

        ExchangeContext * ec = mContextPool.CreateObject(this, payloadHeader.GetExchangeID(), session, false, delegate);

        if (ec == nullptr)
        {
            if (delegate != nullptr)
            {
                matchingUMH->Handler->OnExchangeCreationFailed(delegate);
            }

            // Using same error message for all errors to reduce code size.
            ChipLogError(ExchangeManager, "OnMessageReceived failed, err = %" CHIP_ERROR_FORMAT, CHIP_ERROR_NO_MEMORY.Format());
            // No resource for creating new exchange, SendStandaloneAckIfNeeded probably also fails, so do not try it here
            return;
        }

        ChipLogDetail(ExchangeManager, "Handling via exchange: " ChipLogFormatExchange ", Delegate: %p", ChipLogValueExchange(ec),
                      ec->GetDelegate());

        if (ec->IsEncryptionRequired() != packetHeader.IsEncrypted())
        {
            ChipLogError(ExchangeManager, "OnMessageReceived failed, err = %" CHIP_ERROR_FORMAT,
                         CHIP_ERROR_INVALID_MESSAGE_TYPE.Format());
            ec->Close();
            SendStandaloneAckIfNeeded(packetHeader, payloadHeader, session, msgFlags, std::move(msgBuf));
            return;
        }

        err = ec->HandleMessage(packetHeader.GetMessageCounter(), payloadHeader, msgFlags, std::move(msgBuf));
        if (err != CHIP_NO_ERROR)
        {
            // Using same error message for all errors to reduce code size.
            ChipLogError(ExchangeManager, "OnMessageReceived failed, err = %" CHIP_ERROR_FORMAT, err.Format());
        }
        return;
    }

    SendStandaloneAckIfNeeded(packetHeader, payloadHeader, session, msgFlags, std::move(msgBuf));
    return;
}

void ExchangeManager::SendStandaloneAckIfNeeded(const PacketHeader & packetHeader, const PayloadHeader & payloadHeader,
                                                const SessionHandle & session, MessageFlags msgFlags,
                                                System::PacketBufferHandle && msgBuf)
{
    // If we need to send a StandaloneAck, create a EphemeralExchange for the purpose to send the StandaloneAck
    if (!payloadHeader.NeedsAck())
        return;

    // If rcvd msg is from initiator then this exchange is created as not Initiator.
    // If rcvd msg is not from initiator then this exchange is created as Initiator.
    // Create a EphemeralExchange to generate a StandaloneAck
    ExchangeContext * ec = mContextPool.CreateObject(this, payloadHeader.GetExchangeID(), session, !payloadHeader.IsInitiator(),
                                                     nullptr, true /* IsEphemeralExchange */);

    if (ec == nullptr)
    {
        // Using same error message for all errors to reduce code size.
        ChipLogError(ExchangeManager, "OnMessageReceived failed, err = %" CHIP_ERROR_FORMAT, CHIP_ERROR_NO_MEMORY.Format());
        return;
    }

    ChipLogDetail(ExchangeManager, "Generating StandaloneAck via exchange: " ChipLogFormatExchange, ChipLogValueExchange(ec));

    // No need to verify packet encryption type, the EphemeralExchange can handle both secure and insecure messages.

    CHIP_ERROR err = ec->HandleMessage(packetHeader.GetMessageCounter(), payloadHeader, msgFlags, std::move(msgBuf));
    if (err != CHIP_NO_ERROR)
    {
        // Using same error message for all errors to reduce code size.
        ChipLogError(ExchangeManager, "OnMessageReceived failed, err = %" CHIP_ERROR_FORMAT, err.Format());
    }

    // The exchange should be closed inside HandleMessage function. So don't bother close it here.
    return;
}

void ExchangeManager::CloseAllContextsForDelegate(const ExchangeDelegate * delegate)
{
    mContextPool.ForEachActiveObject([&](auto * ec) {
        if (ec->GetDelegate() == delegate)
        {
            // Make sure to null out the delegate before closing the context, so
            // we don't notify the delegate that the context is closing.  We
            // have to do this, because the delegate might be partially
            // destroyed by this point.
            ec->SetDelegate(nullptr);
            ec->Close();
        }
        return Loop::Continue;
    });
}

} // namespace Messaging
} // namespace chip
