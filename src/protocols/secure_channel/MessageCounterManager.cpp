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
 *      This file implements the CHIP message counter messages in secure channel protocol.
 *
 */

#include <protocols/secure_channel/MessageCounterManager.h>

#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/core/CHIPKeyIds.h>
#include <lib/support/BufferWriter.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <protocols/Protocols.h>
#include <protocols/secure_channel/Constants.h>

namespace chip {
namespace secure_channel {

constexpr System::Clock::Timeout MessageCounterManager::kSyncTimeout;

CHIP_ERROR MessageCounterManager::Init(Messaging::ExchangeManager * exchangeMgr)
{
    VerifyOrReturnError(exchangeMgr != nullptr, CHIP_ERROR_INCORRECT_STATE);
    mExchangeMgr = exchangeMgr;

    ReturnErrorOnFailure(
        mExchangeMgr->RegisterUnsolicitedMessageHandlerForType(Protocols::SecureChannel::MsgType::MsgCounterSyncReq, this));

    return CHIP_NO_ERROR;
}

void MessageCounterManager::Shutdown()
{
    if (mExchangeMgr != nullptr)
    {
        mExchangeMgr->UnregisterUnsolicitedMessageHandlerForType(Protocols::SecureChannel::MsgType::MsgCounterSyncReq);
        mExchangeMgr->CloseAllContextsForDelegate(this);
        mExchangeMgr = nullptr;
    }
}

CHIP_ERROR MessageCounterManager::StartSync(const SessionHandle & session, Transport::SecureSession * state)
{
    // Initiate message counter synchronization if no message counter synchronization is in progress.
    Transport::PeerMessageCounter & counter = state->GetSessionMessageCounter().GetPeerMessageCounter();
    if (!counter.IsSynchronizing() && !counter.IsSynchronized())
    {
        ReturnErrorOnFailure(SendMsgCounterSyncReq(session, state));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR MessageCounterManager::QueueReceivedMessageAndStartSync(const PacketHeader & packetHeader, const SessionHandle & session,
                                                                   Transport::SecureSession * state,
                                                                   const Transport::PeerAddress & peerAddress,
                                                                   System::PacketBufferHandle && msgBuf)
{
    // Queue the message to be reprocessed when sync completes.
    ReturnErrorOnFailure(AddToReceiveTable(packetHeader, peerAddress, std::move(msgBuf)));
    ReturnErrorOnFailure(StartSync(session, state));

    // After the message that triggers message counter synchronization is stored, and a message counter
    // synchronization exchange is initiated, we need to return immediately and re-process the original message
    // when the synchronization is completed.

    return CHIP_NO_ERROR;
}

CHIP_ERROR MessageCounterManager::OnUnsolicitedMessageReceived(const PayloadHeader & payloadHeader, ExchangeDelegate *& newDelegate)
{
    // MessageCounterManager do not use an extra context to handle messages
    newDelegate = this;
    return CHIP_NO_ERROR;
}

CHIP_ERROR MessageCounterManager::OnMessageReceived(Messaging::ExchangeContext * exchangeContext,
                                                    const PayloadHeader & payloadHeader, System::PacketBufferHandle && msgBuf)
{
    if (payloadHeader.HasMessageType(Protocols::SecureChannel::MsgType::MsgCounterSyncReq))
    {
        return HandleMsgCounterSyncReq(exchangeContext, std::move(msgBuf));
    }
    if (payloadHeader.HasMessageType(Protocols::SecureChannel::MsgType::MsgCounterSyncRsp))
    {
        return HandleMsgCounterSyncResp(exchangeContext, std::move(msgBuf));
    }
    return CHIP_NO_ERROR;
}

void MessageCounterManager::OnResponseTimeout(Messaging::ExchangeContext * exchangeContext)
{
    if (exchangeContext->HasSessionHandle())
    {
        exchangeContext->GetSessionHandle()->AsSecureSession()->GetSessionMessageCounter().GetPeerMessageCounter().SyncFailed();
    }
    else
    {
        ChipLogError(SecureChannel, "MCSP Timeout! On a already released session.");
    }
}

CHIP_ERROR MessageCounterManager::AddToReceiveTable(const PacketHeader & packetHeader, const Transport::PeerAddress & peerAddress,
                                                    System::PacketBufferHandle && msgBuf)
{
    ReturnErrorOnFailure(packetHeader.EncodeBeforeData(msgBuf));

    for (ReceiveTableEntry & entry : mReceiveTable)
    {
        if (entry.msgBuf.IsNull())
        {
            entry.peerAddress = peerAddress;
            entry.msgBuf      = std::move(msgBuf);

            return CHIP_NO_ERROR;
        }
    }

    ChipLogError(SecureChannel, "MCSP ReceiveTable Already Full");
    return CHIP_ERROR_NO_MEMORY;
}

/**
 *  Reprocess all pending messages that were encrypted with application
 *  group key and were addressed to the specified node id.
 *
 *  @param[in] peerNodeId    Node ID of the destination node.
 *
 */
void MessageCounterManager::ProcessPendingMessages(NodeId peerNodeId)
{
    auto * sessionManager = mExchangeMgr->GetSessionManager();

    // Find all receive entries matching peerNodeId.  Note that everything in
    // this table was using an application group key; that's why it was added.
    for (ReceiveTableEntry & entry : mReceiveTable)
    {
        if (!entry.msgBuf.IsNull())
        {
            PacketHeader packetHeader;
            uint16_t headerSize = 0;

            if (packetHeader.Decode((entry.msgBuf)->Start(), (entry.msgBuf)->DataLength(), &headerSize) != CHIP_NO_ERROR)
            {
                ChipLogError(SecureChannel, "MessageCounterManager::ProcessPendingMessages: Failed to decode PacketHeader");
                entry.msgBuf = nullptr;
                continue;
            }

            if (packetHeader.GetSourceNodeId().HasValue() && packetHeader.GetSourceNodeId().Value() == peerNodeId)
            {
                // Reprocess message.
                sessionManager->OnMessageReceived(entry.peerAddress, std::move(entry.msgBuf));

                // Explicitly free any buffer owned by this handle.
                entry.msgBuf = nullptr;
            }
        }
    }
}

CHIP_ERROR MessageCounterManager::SendMsgCounterSyncReq(const SessionHandle & session, Transport::SecureSession * state)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    Messaging::ExchangeContext * exchangeContext = nullptr;
    System::PacketBufferHandle msgBuf;
    Messaging::SendFlags sendFlags;

    exchangeContext = mExchangeMgr->NewContext(session, this);
    VerifyOrExit(exchangeContext != nullptr, err = CHIP_ERROR_NO_MEMORY);

    msgBuf = MessagePacketBuffer::New(kChallengeSize);
    VerifyOrExit(!msgBuf.IsNull(), err = CHIP_ERROR_NO_MEMORY);

    // Generate a 64-bit random number to uniquely identify the request.
    SuccessOrExit(err = Crypto::DRBG_get_bytes(msgBuf->Start(), kChallengeSize));

    msgBuf->SetDataLength(kChallengeSize);

    // Store generated Challenge value to message counter context to resolve synchronization response.
    state->GetSessionMessageCounter().GetPeerMessageCounter().SyncStarting(FixedByteSpan<kChallengeSize>(msgBuf->Start()));

    sendFlags.Set(Messaging::SendMessageFlags::kNoAutoRequestAck).Set(Messaging::SendMessageFlags::kExpectResponse);

    // Arm a timer to enforce that a MsgCounterSyncRsp is received before kSyncTimeout.
    exchangeContext->SetResponseTimeout(kSyncTimeout);

    // Send the message counter synchronization request in a Secure Channel Protocol::MsgCounterSyncReq message.
    SuccessOrExit(
        err = exchangeContext->SendMessage(Protocols::SecureChannel::MsgType::MsgCounterSyncReq, std::move(msgBuf), sendFlags));

exit:
    if (err != CHIP_NO_ERROR)
    {
        if (exchangeContext != nullptr)
        {
            exchangeContext->Close();
        }
        state->GetSessionMessageCounter().GetPeerMessageCounter().SyncFailed();
        ChipLogError(SecureChannel, "Failed to send message counter synchronization request with error:%" CHIP_ERROR_FORMAT,
                     err.Format());
    }

    return err;
}

CHIP_ERROR MessageCounterManager::SendMsgCounterSyncResp(Messaging::ExchangeContext * exchangeContext,
                                                         FixedByteSpan<kChallengeSize> challenge)
{
    System::PacketBufferHandle msgBuf;
    VerifyOrDie(exchangeContext->HasSessionHandle());

    VerifyOrReturnError(exchangeContext->GetSessionHandle()->IsGroupSession(), CHIP_ERROR_INVALID_ARGUMENT);

    // NOTE: not currently implemented. When implementing, the following should be done:
    //    - allocate a new buffer: MessagePacketBuffer::New
    //    - setup payload and place the local message counter + challange in it
    //    - exchangeContext->SendMessage(Protocols::SecureChannel::MsgType::MsgCounterSyncRsp, ...)
    //
    // You can view the history of this file for a partial implementation that got
    // removed due to it using non-group sessions.

    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR MessageCounterManager::HandleMsgCounterSyncReq(Messaging::ExchangeContext * exchangeContext,
                                                          System::PacketBufferHandle && msgBuf)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    uint8_t * req = msgBuf->Start();
    size_t reqlen = msgBuf->DataLength();

    ChipLogDetail(SecureChannel, "Received MsgCounterSyncReq request");

    VerifyOrExit(req != nullptr, err = CHIP_ERROR_MESSAGE_INCOMPLETE);
    VerifyOrExit(reqlen == kChallengeSize, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    // Respond with MsgCounterSyncResp
    err = SendMsgCounterSyncResp(exchangeContext, FixedByteSpan<kChallengeSize>(req));

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SecureChannel, "Failed to handle MsgCounterSyncReq message with error:%" CHIP_ERROR_FORMAT, err.Format());
    }

    return err;
}

CHIP_ERROR MessageCounterManager::HandleMsgCounterSyncResp(Messaging::ExchangeContext * exchangeContext,
                                                           System::PacketBufferHandle && msgBuf)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    uint32_t syncCounter = 0;

    const uint8_t * resp = msgBuf->Start();
    size_t resplen       = msgBuf->DataLength();

    ChipLogDetail(SecureChannel, "Received MsgCounterSyncResp response");

    VerifyOrDie(exchangeContext->HasSessionHandle());

    VerifyOrExit(msgBuf->DataLength() == kSyncRespMsgSize, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    VerifyOrExit(resp != nullptr, err = CHIP_ERROR_MESSAGE_INCOMPLETE);
    VerifyOrExit(resplen == kSyncRespMsgSize, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    syncCounter = chip::Encoding::LittleEndian::Read32(resp);
    VerifyOrExit(syncCounter != 0, err = CHIP_ERROR_READ_FAILED);

    // Verify that the response field matches the expected Challenge field for the exchange.
    err =
        exchangeContext->GetSessionHandle()->AsSecureSession()->GetSessionMessageCounter().GetPeerMessageCounter().VerifyChallenge(
            syncCounter, FixedByteSpan<kChallengeSize>(resp));
    SuccessOrExit(err);

    // Process all queued incoming messages after message counter synchronization is completed.
    ProcessPendingMessages(exchangeContext->GetSessionHandle()->AsSecureSession()->GetPeerNodeId());

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SecureChannel, "Failed to handle MsgCounterSyncResp message with error:%" CHIP_ERROR_FORMAT, err.Format());
    }

    return err;
}

} // namespace secure_channel
} // namespace chip
