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

CHIP_ERROR MessageCounterManager::QueueReceivedMessageAndStartSync(const PacketHeader & packetHeader, SecureSessionHandle session,
                                                                   Transport::PeerConnectionState * state,
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

CHIP_ERROR MessageCounterManager::OnMessageReceived(Messaging::ExchangeContext * exchangeContext, const PacketHeader & packetHeader,
                                                    const PayloadHeader & payloadHeader, System::PacketBufferHandle && msgBuf)
{
    if (payloadHeader.HasMessageType(Protocols::SecureChannel::MsgType::MsgCounterSyncReq))
    {
        return HandleMsgCounterSyncReq(exchangeContext, packetHeader, std::move(msgBuf));
    }
    else if (payloadHeader.HasMessageType(Protocols::SecureChannel::MsgType::MsgCounterSyncRsp))
    {
        return HandleMsgCounterSyncResp(exchangeContext, packetHeader, std::move(msgBuf));
    }
    return CHIP_NO_ERROR;
}

void MessageCounterManager::OnResponseTimeout(Messaging::ExchangeContext * exchangeContext)
{
    Transport::PeerConnectionState * state =
        mExchangeMgr->GetSessionMgr()->GetPeerConnectionState(exchangeContext->GetSecureSession());

    if (state != nullptr)
    {
        state->GetSessionMessageCounter().GetPeerMessageCounter().SyncFailed();
    }
    else
    {
        ChipLogError(SecureChannel, "Timed out! Failed to clear message counter synchronization status.");
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
    auto * secureSessionMgr = mExchangeMgr->GetSessionMgr();

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
                secureSessionMgr->OnMessageReceived(entry.peerAddress, std::move(entry.msgBuf));

                // Explicitly free any buffer owned by this handle.
                entry.msgBuf = nullptr;
            }
        }
    }
}

CHIP_ERROR MessageCounterManager::SendMsgCounterSyncReq(SecureSessionHandle session, Transport::PeerConnectionState * state)
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

    // Arm a timer to enforce that a MsgCounterSyncRsp is received before kSyncTimeoutMs.
    exchangeContext->SetResponseTimeout(kSyncTimeoutMs);

    // Send the message counter synchronization request in a Secure Channel Protocol::MsgCounterSyncReq message.
    SuccessOrExit(
        err = exchangeContext->SendMessage(Protocols::SecureChannel::MsgType::MsgCounterSyncReq, std::move(msgBuf), sendFlags));

exit:
    if (err != CHIP_NO_ERROR)
    {
        state->GetSessionMessageCounter().GetPeerMessageCounter().SyncFailed();
        ChipLogError(SecureChannel, "Failed to send message counter synchronization request with error:%s", ErrorStr(err));
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
    msgBuf = MessagePacketBuffer::New(kSyncRespMsgSize);
    VerifyOrExit(!msgBuf.IsNull(), err = CHIP_ERROR_NO_MEMORY);

    msg = msgBuf->Start();

    {
        Encoding::LittleEndian::BufferWriter bbuf(msg, kSyncRespMsgSize);
        bbuf.Put32(state->GetSessionMessageCounter().GetLocalMessageCounter().Value());
        bbuf.Put(challenge.data(), kChallengeSize);
        VerifyOrExit(bbuf.Fit(), err = CHIP_ERROR_NO_MEMORY);
    }

    msgBuf->SetDataLength(kSyncRespMsgSize);

    err = exchangeContext->SendMessage(Protocols::SecureChannel::MsgType::MsgCounterSyncRsp, std::move(msgBuf),
                                       Messaging::SendFlags(Messaging::SendMessageFlags::kNoAutoRequestAck));

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SecureChannel, "Failed to send message counter synchronization response with error:%s", ErrorStr(err));
    }

    return err;
}

CHIP_ERROR MessageCounterManager::HandleMsgCounterSyncReq(Messaging::ExchangeContext * exchangeContext,
                                                          const PacketHeader & packetHeader, System::PacketBufferHandle && msgBuf)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    uint8_t * req = msgBuf->Start();
    size_t reqlen = msgBuf->DataLength();

    ChipLogDetail(SecureChannel, "Received MsgCounterSyncReq request");

    VerifyOrExit(packetHeader.GetSourceNodeId().HasValue(), err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(req != nullptr, err = CHIP_ERROR_MESSAGE_INCOMPLETE);
    VerifyOrExit(reqlen == kChallengeSize, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    // Respond with MsgCounterSyncResp
    err = SendMsgCounterSyncResp(exchangeContext, FixedByteSpan<kChallengeSize>(req));

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SecureChannel, "Failed to handle MsgCounterSyncReq message with error:%s", ErrorStr(err));
    }

    return err;
}

CHIP_ERROR MessageCounterManager::HandleMsgCounterSyncResp(Messaging::ExchangeContext * exchangeContext,
                                                           const PacketHeader & packetHeader, System::PacketBufferHandle && msgBuf)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    Transport::PeerConnectionState * state = nullptr;
    NodeId peerNodeId                      = 0;
    uint32_t syncCounter                   = 0;

    const uint8_t * resp = msgBuf->Start();
    size_t resplen       = msgBuf->DataLength();

    ChipLogDetail(SecureChannel, "Received MsgCounterSyncResp response");

    // Find an active connection to the specified peer node
    state = mExchangeMgr->GetSessionMgr()->GetPeerConnectionState(exchangeContext->GetSecureSession());
    VerifyOrExit(state != nullptr, err = CHIP_ERROR_NOT_CONNECTED);

    VerifyOrExit(msgBuf->DataLength() == kSyncRespMsgSize, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    VerifyOrExit(resp != nullptr, err = CHIP_ERROR_MESSAGE_INCOMPLETE);
    VerifyOrExit(resplen == kSyncRespMsgSize, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    syncCounter = chip::Encoding::LittleEndian::Read32(resp);
    VerifyOrExit(syncCounter != 0, err = CHIP_ERROR_READ_FAILED);

    // Verify that the response field matches the expected Challenge field for the exchange.
    err =
        state->GetSessionMessageCounter().GetPeerMessageCounter().VerifyChallenge(syncCounter, FixedByteSpan<kChallengeSize>(resp));
    SuccessOrExit(err);

    VerifyOrExit(packetHeader.GetSourceNodeId().HasValue(), err = CHIP_ERROR_INVALID_ARGUMENT);
    peerNodeId = packetHeader.GetSourceNodeId().Value();

    // Process all queued incoming messages after message counter synchronization is completed.
    ProcessPendingMessages(peerNodeId);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SecureChannel, "Failed to handle MsgCounterSyncResp message with error:%s", ErrorStr(err));
    }

    return err;
}

} // namespace secure_channel
} // namespace chip
