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
 *      This file implements the CHIP Secure Channel protocol.
 *
 */

#include <core/CHIPCore.h>
#include <core/CHIPEncoding.h>
#include <core/CHIPKeyIds.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <protocols/Protocols.h>
#include <protocols/secure_channel/Constants.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <support/BufferWriter.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

namespace chip {
namespace secure_channel {

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
        mExchangeMgr = nullptr;
    }
}

CHIP_ERROR MessageCounterManager::StartSync(SecureSessionHandle session, Transport::PeerConnectionState * state)
{
    // Initiate message counter synchronization if no message counter synchronization is in progress.
    Transport::PeerMessageCounter & counter = state->GetSessionMessageCounter().GetPeerMessageCounter();
    if (!counter.IsSynchronizing() && !counter.IsSynchronized())
    {
        ReturnErrorOnFailure(SendMsgCounterSyncReq(session, state));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR MessageCounterManager::QueueReceivedMessageAndStartSync(SecureSessionHandle session,
                                                                   Transport::PeerConnectionState * state,
                                                                   const PacketHeader & packetHeader,
                                                                   const Transport::PeerAddress & peerAddress,
                                                                   System::PacketBufferHandle msgBuf)
{
    // Queue the message to be reprocessed when sync completes.
    ReturnErrorOnFailure(AddToReceiveTable(state->GetPeerNodeId(), packetHeader, peerAddress, std::move(msgBuf)));
    ReturnErrorOnFailure(StartSync(session, state));

    // After the message that triggers message counter synchronization is stored, and a message counter
    // synchronization exchange is initiated, we need to return immediately and re-process the original message
    // when the synchronization is completed.

    return CHIP_NO_ERROR;
}

void MessageCounterManager::OnMessageReceived(Messaging::ExchangeContext * exchangeContext, const PacketHeader & packetHeader,
                                              const PayloadHeader & payloadHeader, System::PacketBufferHandle msgBuf)
{
    if (payloadHeader.HasMessageType(Protocols::SecureChannel::MsgType::MsgCounterSyncReq))
    {
        HandleMsgCounterSyncReq(exchangeContext, packetHeader, std::move(msgBuf));
    }
    else if (payloadHeader.HasMessageType(Protocols::SecureChannel::MsgType::MsgCounterSyncRsp))
    {
        HandleMsgCounterSyncResp(exchangeContext, packetHeader, std::move(msgBuf));
    }
}

void MessageCounterManager::OnResponseTimeout(Messaging::ExchangeContext * exchangeContext)
{
    Transport::PeerConnectionState * state =
        mExchangeMgr->GetSessionMgr()->GetPeerConnectionState(exchangeContext->GetSecureSessionHandle());

    if (state != nullptr)
    {
        state->GetSessionMessageCounter().GetPeerMessageCounter().SyncFailed();
    }
    else
    {
        ChipLogError(ExchangeManager, "Timed out! Failed to clear message counter synchronization status.");
    }

    // Close the exchange if MsgCounterSyncRsp is not received before kSyncTimeoutMs.
    if (exchangeContext != nullptr)
    {
        exchangeContext->Close();
    }
}

CHIP_ERROR MessageCounterManager::AddToReceiveTable(NodeId peerNodeId, const PacketHeader & packetHeader,
                                                    const Transport::PeerAddress & peerAddress, System::PacketBufferHandle msgBuf)
{
    bool added     = false;
    CHIP_ERROR err = CHIP_NO_ERROR;

    for (ReceiveTableEntry & entry : mReceiveTable)
    {
        // Entries are in use if they have a message buffer.
        if (entry.peerNodeId == kUndefinedNodeId)
        {
            entry.peerNodeId   = peerNodeId;
            entry.packetHeader = packetHeader;
            entry.peerAddress  = peerAddress;
            entry.msgBuf       = std::move(msgBuf);
            added              = true;

            break;
        }
    }

    if (!added)
    {
        ChipLogError(ExchangeManager, "MCSP ReceiveTable Already Full");
        err = CHIP_ERROR_NO_MEMORY;
    }

    return err;
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
    auto * secureSessionMgr = mExchangeMgr->GetSessionMgr();

    // Find all receive entries matching peerNodeId.  Note that everything in
    // this table was using an application group key; that's why it was added.
    for (ReceiveTableEntry & entry : mReceiveTable)
    {
        if (entry.peerNodeId == peerNodeId)
        {
            // Reprocess message.
            secureSessionMgr->OnMessageReceived(entry.packetHeader, entry.peerAddress, std::move(entry.msgBuf));

            // Explicitly free any buffer owned by this handle.  The
            // HandleGroupMessageReceived() call should really handle this, but
            // just in case it messes up we don't want to get confused about
            // whether the entry is in use.
            entry.msgBuf     = nullptr;
            entry.peerNodeId = kUndefinedNodeId;
        }
    }
}

CHIP_ERROR MessageCounterManager::SendMsgCounterSyncReq(SecureSessionHandle session, Transport::PeerConnectionState * state)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    Messaging::ExchangeContext * exchangeContext = nullptr;
    System::PacketBufferHandle msgBuf;
    Messaging::SendFlags sendFlags;

    // Create and initialize new exchange.
    exchangeContext = mExchangeMgr->NewContext(session, this);
    VerifyOrExit(exchangeContext != nullptr, err = CHIP_ERROR_NO_MEMORY);

    // Allocate a buffer for the reply message.
    msgBuf = MessagePacketBuffer::New(kChallengeSize + MessagePacketBuffer::kMaxFooterSize);
    VerifyOrExit(!msgBuf.IsNull(), err = CHIP_ERROR_NO_MEMORY);

    // Generate a 64-bit random number to uniquely identify the request.
    SuccessOrExit(err = Crypto::DRBG_get_bytes(msgBuf->Start(), kChallengeSize));

    msgBuf->SetDataLength(kChallengeSize);

    // Store generated Challenge value to message counter context to resolve synchronization response.
    state->GetSessionMessageCounter().GetPeerMessageCounter().SyncStarting(FixedByteSpan<kChallengeSize>(msgBuf->Start()));

    sendFlags.Set(Messaging::SendMessageFlags::kNoAutoRequestAck, true).Set(Messaging::SendMessageFlags::kExpectResponse, true);

    // Arm a timer to enforce that a MsgCounterSyncRsp is received before kSyncTimeoutMs.
    exchangeContext->SetResponseTimeout(kSyncTimeoutMs);

    // Send the message counter synchronization request in a Secure Channel Protocol::MsgCounterSyncReq message.
    SuccessOrExit(
        err = exchangeContext->SendMessage(Protocols::SecureChannel::MsgType::MsgCounterSyncReq, std::move(msgBuf), sendFlags));

exit:
    if (err != CHIP_NO_ERROR)
    {
        state->GetSessionMessageCounter().GetPeerMessageCounter().SyncFailed();
        ChipLogError(ExchangeManager, "Failed to send message counter synchronization request with error:%s", ErrorStr(err));
    }

    return err;
}

CHIP_ERROR MessageCounterManager::SendMsgCounterSyncResp(Messaging::ExchangeContext * exchangeContext,
                                                         FixedByteSpan<kChallengeSize> challenge)
{
    CHIP_ERROR err                         = CHIP_NO_ERROR;
    Transport::PeerConnectionState * state = nullptr;
    System::PacketBufferHandle msgBuf;
    uint8_t * msg = nullptr;

    state = mExchangeMgr->GetSessionMgr()->GetPeerConnectionState(exchangeContext->GetSecureSession());
    VerifyOrExit(state != nullptr, err = CHIP_ERROR_NOT_CONNECTED);

    // Allocate new buffer.
    msgBuf = MessagePacketBuffer::New(kSyncRespMsgSize + MessagePacketBuffer::kMaxFooterSize);
    VerifyOrExit(!msgBuf.IsNull(), err = CHIP_ERROR_NO_MEMORY);

    msg = msgBuf->Start();

    // Let's construct the message using BufBound
    {
        Encoding::LittleEndian::BufferWriter bbuf(msg, kSyncRespMsgSize);

        // Write the message id (counter) field.
        bbuf.Put32(state->GetSessionMessageCounter().GetLocalMessageCounter().Value());

        // Fill in the random value
        bbuf.Put(challenge.data(), kChallengeSize);

        VerifyOrExit(bbuf.Fit(), err = CHIP_ERROR_NO_MEMORY);
    }

    // Set message length.
    msgBuf->SetDataLength(kSyncRespMsgSize);

    // Send message counter synchronization response message.
    err = exchangeContext->SendMessage(Protocols::SecureChannel::MsgType::MsgCounterSyncRsp, std::move(msgBuf),
                                       Messaging::SendFlags(Messaging::SendMessageFlags::kNoAutoRequestAck));

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(ExchangeManager, "Failed to send message counter synchronization response with error:%s", ErrorStr(err));
    }

    return err;
}

void MessageCounterManager::HandleMsgCounterSyncReq(Messaging::ExchangeContext * exchangeContext, const PacketHeader & packetHeader,
                                                    System::PacketBufferHandle msgBuf)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    uint8_t * req = msgBuf->Start();
    size_t reqlen = msgBuf->DataLength();

    ChipLogDetail(ExchangeManager, "Received MsgCounterSyncReq request");

    VerifyOrExit(packetHeader.GetSourceNodeId().HasValue(), err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(req != nullptr, err = CHIP_ERROR_MESSAGE_INCOMPLETE);
    VerifyOrExit(reqlen == kChallengeSize, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    // Respond with MsgCounterSyncResp
    err = SendMsgCounterSyncResp(exchangeContext, FixedByteSpan<kChallengeSize>(req));

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(ExchangeManager, "Failed to handle MsgCounterSyncReq message with error:%s", ErrorStr(err));
    }

    exchangeContext->Close();
    return;
}

void MessageCounterManager::HandleMsgCounterSyncResp(Messaging::ExchangeContext * exchangeContext,
                                                     const PacketHeader & packetHeader, System::PacketBufferHandle msgBuf)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    Transport::PeerConnectionState * state = nullptr;
    NodeId peerNodeId                      = 0;
    uint32_t syncCounter                   = 0;

    const uint8_t * resp = msgBuf->Start();
    size_t resplen       = msgBuf->DataLength();

    ChipLogDetail(ExchangeManager, "Received MsgCounterSyncResp response");

    // Find an active connection to the specified peer node
    state = mExchangeMgr->GetSessionMgr()->GetPeerConnectionState(exchangeContext->GetSecureSessionHandle());
    VerifyOrExit(state != nullptr, err = CHIP_ERROR_NOT_CONNECTED);

    VerifyOrExit(msgBuf->DataLength() == kSyncRespMsgSize, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    // Store the 64-bit value sent in the Challenge filed of the MsgCounterSyncReq.
    VerifyOrExit(resp != nullptr, err = CHIP_ERROR_MESSAGE_INCOMPLETE);
    VerifyOrExit(resplen == kSyncRespMsgSize, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    syncCounter = chip::Encoding::LittleEndian::Read32(resp);
    VerifyOrExit(syncCounter != 0, err = CHIP_ERROR_READ_FAILED);

    // Verify that the response field matches the expected Challenge field for the exchange.
    err = state->GetSessionMessageCounter().GetPeerMessageCounter().VerifyChallenge(syncCounter, FixedByteSpan<kChallengeSize>(resp));
    SuccessOrExit(err);

    VerifyOrExit(packetHeader.GetSourceNodeId().HasValue(), err = CHIP_ERROR_INVALID_ARGUMENT);
    peerNodeId = packetHeader.GetSourceNodeId().Value();

    // Process all queued ougoing and incomming group messages after message counter synchronization is completed.
    // TODO: Notify ReliableMessageMgr to retrans all messages.
    ProcessPendingMessages(peerNodeId);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(ExchangeManager, "Failed to handle MsgCounterSyncResp message with error:%s", ErrorStr(err));
    }

    exchangeContext->Close();
    return;
}

} // namespace secure_channel
} // namespace chip
