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
#include <messaging/MessageCounterSync.h>
#include <protocols/Protocols.h>
#include <support/BufferWriter.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

namespace chip {
namespace Messaging {

CHIP_ERROR MessageCounterSyncMgr::Init(Messaging::ExchangeManager * exchangeMgr)
{
    VerifyOrReturnError(exchangeMgr != nullptr, CHIP_ERROR_INCORRECT_STATE);
    mExchangeMgr = exchangeMgr;

    // Register to receive unsolicited Message Counter Synchronization Request messages from the exchange manager.
    return mExchangeMgr->RegisterUnsolicitedMessageHandlerForType(Protocols::SecureChannel::MsgType::MsgCounterSyncReq, this);
}

void MessageCounterSyncMgr::Shutdown()
{
    if (mExchangeMgr != nullptr)
    {
        mExchangeMgr->UnregisterUnsolicitedMessageHandlerForType(Protocols::SecureChannel::MsgType::MsgCounterSyncReq);
        mExchangeMgr = nullptr;
    }
}

void MessageCounterSyncMgr::OnMessageReceived(Messaging::ExchangeContext * exchangeContext, const PacketHeader & packetHeader,
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

void MessageCounterSyncMgr::OnResponseTimeout(Messaging::ExchangeContext * exchangeContext)
{
    if (mExchangeMgr == nullptr)
    {
        if (exchangeContext != nullptr)
        {
            exchangeContext->Close();
        }
        return;
    }

    Transport::PeerConnectionState * state =
        mExchangeMgr->GetSessionMgr()->GetPeerConnectionState(exchangeContext->GetSecureSessionHandle());

    if (state != nullptr)
    {
        state->SetMsgCounterSyncInProgress(false);
    }
    else
    {
        ChipLogError(ExchangeManager, "Timed out! Failed to clear message counter synchronization status.");
    }

    // Close the exchange if MsgCounterSyncRsp is not received before kMsgCounterSyncTimeout.
    if (exchangeContext != nullptr)
        exchangeContext->Close();
}

CHIP_ERROR MessageCounterSyncMgr::AddToRetransmissionTable(Protocols::Id protocolId, uint8_t msgType, const SendFlags & sendFlags,
                                                           System::PacketBufferHandle msgBuf,
                                                           Messaging::ExchangeContext * exchangeContext)
{
    bool added     = false;
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrReturnError(exchangeContext != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    for (RetransTableEntry & entry : mRetransTable)
    {
        // Entries are in use if they have an exchangeContext.
        if (entry.exchangeContext == nullptr)
        {
            entry.protocolId      = protocolId;
            entry.msgType         = msgType;
            entry.msgBuf          = std::move(msgBuf);
            entry.exchangeContext = exchangeContext;
            entry.exchangeContext->Retain();
            added = true;

            break;
        }
    }

    if (!added)
    {
        ChipLogError(ExchangeManager, "MCSP RetransTable Already Full");
        err = CHIP_ERROR_NO_MEMORY;
    }

    return err;
}

/**
 *  Retransmit all pending messages that were encrypted with application
 *  group key and were addressed to the specified node.
 *
 *  @param[in] peerNodeId    Node ID of the destination node.
 *
 */
void MessageCounterSyncMgr::RetransPendingGroupMsgs(NodeId peerNodeId)
{
    // Find all retransmit entries matching peerNodeId.  Note that everything in
    // this table was using an application group key; that's why it was added.
    for (RetransTableEntry & entry : mRetransTable)
    {
        if (entry.exchangeContext != nullptr && entry.exchangeContext->GetSecureSession().GetPeerNodeId() == peerNodeId)
        {
            // Retramsmit message.
            CHIP_ERROR err =
                entry.exchangeContext->SendMessage(entry.protocolId, entry.msgType, std::move(entry.msgBuf), entry.sendFlags);

            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(ExchangeManager, "Failed to resend cached group message to node: %d with error:%s", peerNodeId,
                             ErrorStr(err));
            }

            entry.exchangeContext->Release();
            entry.exchangeContext = nullptr;
        }
    }
}

CHIP_ERROR MessageCounterSyncMgr::AddToReceiveTable(System::PacketBufferHandle msgBuf)
{
    bool added     = false;
    CHIP_ERROR err = CHIP_NO_ERROR;

    for (ReceiveTableEntry & entry : mReceiveTable)
    {
        // Entries are in use if they have a message buffer.
        if (entry.msgBuf.IsNull())
        {
            entry.msgBuf = std::move(msgBuf);
            added        = true;
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
void MessageCounterSyncMgr::ProcessPendingGroupMsgs(NodeId peerNodeId)
{
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
                ChipLogError(ExchangeManager, "ProcessPendingGroupMsgs::Failed to decode PacketHeader");
                break;
            }

            if (packetHeader.GetSourceNodeId().HasValue() && packetHeader.GetSourceNodeId().Value() == peerNodeId)
            {
                (entry.msgBuf)->ConsumeHead(headerSize);

                // Reprocess message.
                mExchangeMgr->GetSessionMgr()->HandleGroupMessageReceived(packetHeader, std::move(entry.msgBuf));

                // Explicitly free any buffer owned by this handle.  The
                // HandleGroupMessageReceived() call should really handle this, but
                // just in case it messes up we don't want to get confused about
                // wheter the entry is in use.
                entry.msgBuf = nullptr;
            }
        }
    }
}

// Create and initialize new exchange for the message counter synchronization request/response messages.
CHIP_ERROR MessageCounterSyncMgr::NewMsgCounterSyncExchange(SecureSessionHandle session,
                                                            Messaging::ExchangeContext *& exchangeContext)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Message counter synchronization protocol is only applicable for application group keys.
    VerifyOrReturnError(ChipKeyId::IsAppGroupKey(session.GetPeerKeyId()), err = CHIP_ERROR_INVALID_ARGUMENT);

    // Create new exchange context.
    exchangeContext = mExchangeMgr->NewContext(session, this);
    VerifyOrReturnError(exchangeContext != nullptr, err = CHIP_ERROR_NO_MEMORY);

    return err;
}

CHIP_ERROR MessageCounterSyncMgr::SendMsgCounterSyncReq(SecureSessionHandle session)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    Messaging::ExchangeContext * exchangeContext = nullptr;
    Transport::PeerConnectionState * state       = nullptr;
    System::PacketBufferHandle msgBuf;
    Messaging::SendFlags sendFlags;
    uint8_t challenge[kMsgCounterChallengeSize];

    state = mExchangeMgr->GetSessionMgr()->GetPeerConnectionState(session);
    VerifyOrExit(state != nullptr, err = CHIP_ERROR_NOT_CONNECTED);

    // Create and initialize new exchange.
    err = NewMsgCounterSyncExchange(session, exchangeContext);
    SuccessOrExit(err);

    // Allocate a buffer for the null message.
    msgBuf = MessagePacketBuffer::New(kMsgCounterChallengeSize);
    VerifyOrExit(!msgBuf.IsNull(), err = CHIP_ERROR_NO_MEMORY);

    // Generate a 64-bit random number to uniquely identify the request.
    err = DRBG_get_bytes(challenge, kMsgCounterChallengeSize);
    SuccessOrExit(err);

    // Store generated Challenge value to ExchangeContext to resolve synchronization response.
    exchangeContext->SetChallenge(challenge);

    memcpy(msgBuf->Start(), challenge, kMsgCounterChallengeSize);
    msgBuf->SetDataLength(kMsgCounterChallengeSize);

    sendFlags.Set(Messaging::SendMessageFlags::kNoAutoRequestAck, true).Set(Messaging::SendMessageFlags::kExpectResponse, true);

    // Arm a timer to enforce that a MsgCounterSyncRsp is received before kMsgCounterSyncTimeout.
    exchangeContext->SetResponseTimeout(kMsgCounterSyncTimeout);

    // Send the message counter synchronization request in a Secure Channel Protocol::MsgCounterSyncReq message.
    err = exchangeContext->SendMessageImpl(Protocols::SecureChannel::Id,
                                           static_cast<uint8_t>(Protocols::SecureChannel::MsgType::MsgCounterSyncReq),
                                           std::move(msgBuf), sendFlags);
    SuccessOrExit(err);

    state->SetMsgCounterSyncInProgress(true);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(ExchangeManager, "Failed to send message counter synchronization request with error:%s", ErrorStr(err));
    }

    return err;
}

CHIP_ERROR MessageCounterSyncMgr::SendMsgCounterSyncResp(Messaging::ExchangeContext * exchangeContext, SecureSessionHandle session)
{
    CHIP_ERROR err                         = CHIP_NO_ERROR;
    Transport::PeerConnectionState * state = nullptr;
    System::PacketBufferHandle msgBuf;
    uint8_t * msg = nullptr;

    state = mExchangeMgr->GetSessionMgr()->GetPeerConnectionState(session);
    VerifyOrExit(state != nullptr, err = CHIP_ERROR_NOT_CONNECTED);

    // Allocate new buffer.
    msgBuf = System::PacketBufferHandle::New(kMsgCounterSyncRespMsgSize);
    VerifyOrExit(!msgBuf.IsNull(), err = CHIP_ERROR_NO_MEMORY);

    msg = msgBuf->Start();

    // Let's construct the message using BufBound
    {
        Encoding::LittleEndian::BufferWriter bbuf(msg, kMsgCounterSyncRespMsgSize);

        // Write the message id (counter) field.
        bbuf.Put32(state->GetSendMessageIndex());

        // Fill in the random value
        bbuf.Put(exchangeContext->GetChallenge(), kMsgCounterChallengeSize);

        VerifyOrExit(bbuf.Fit(), err = CHIP_ERROR_NO_MEMORY);
    }

    // Set message length.
    msgBuf->SetDataLength(kMsgCounterSyncRespMsgSize);

    // Send message counter synchronization response message.
    err = exchangeContext->SendMessageImpl(Protocols::SecureChannel::Id,
                                           static_cast<uint8_t>(Protocols::SecureChannel::MsgType::MsgCounterSyncRsp),
                                           std::move(msgBuf), Messaging::SendFlags(Messaging::SendMessageFlags::kNoAutoRequestAck));

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(ExchangeManager, "Failed to send message counter synchronization response with error:%s", ErrorStr(err));
    }

    return err;
}

void MessageCounterSyncMgr::HandleMsgCounterSyncReq(Messaging::ExchangeContext * exchangeContext, const PacketHeader & packetHeader,
                                                    System::PacketBufferHandle msgBuf)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    const uint8_t * req = msgBuf->Start();
    size_t reqlen       = msgBuf->DataLength();

    ChipLogDetail(ExchangeManager, "Received MsgCounterSyncReq request");

    VerifyOrExit(packetHeader.GetSourceNodeId().HasValue(), err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(ChipKeyId::IsAppGroupKey(packetHeader.GetEncryptionKeyID()), err = CHIP_ERROR_WRONG_KEY_TYPE);
    VerifyOrExit(req != nullptr, err = CHIP_ERROR_MESSAGE_INCOMPLETE);
    VerifyOrExit(reqlen == kMsgCounterChallengeSize, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    // Store the 64-bit value sent in the Challenge filed of the MsgCounterSyncReq.
    exchangeContext->SetChallenge(req);

    // Respond with MsgCounterSyncResp
    err = SendMsgCounterSyncResp(exchangeContext, { packetHeader.GetSourceNodeId().Value(), packetHeader.GetEncryptionKeyID(), 0 });

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(ExchangeManager, "Failed to handle MsgCounterSyncReq message with error:%s", ErrorStr(err));
    }

    if (exchangeContext != nullptr)
        exchangeContext->Close();

    return;
}

void MessageCounterSyncMgr::HandleMsgCounterSyncResp(Messaging::ExchangeContext * exchangeContext,
                                                     const PacketHeader & packetHeader, System::PacketBufferHandle msgBuf)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    Transport::PeerConnectionState * state = nullptr;
    NodeId peerNodeId                      = 0;
    uint32_t syncCounter                   = 0;
    uint8_t challenge[kMsgCounterChallengeSize];

    const uint8_t * resp = msgBuf->Start();
    size_t resplen       = msgBuf->DataLength();

    ChipLogDetail(ExchangeManager, "Received MsgCounterSyncResp response");

    // Find an active connection to the specified peer node
    state = mExchangeMgr->GetSessionMgr()->GetPeerConnectionState(exchangeContext->GetSecureSessionHandle());
    VerifyOrExit(state != nullptr, err = CHIP_ERROR_NOT_CONNECTED);

    state->SetMsgCounterSyncInProgress(false);

    VerifyOrExit(msgBuf->DataLength() == kMsgCounterSyncRespMsgSize, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);
    VerifyOrExit(ChipKeyId::IsAppGroupKey(packetHeader.GetEncryptionKeyID()), err = CHIP_ERROR_WRONG_KEY_TYPE);

    // Store the 64-bit value sent in the Challenge filed of the MsgCounterSyncReq.
    VerifyOrExit(resp != nullptr, err = CHIP_ERROR_MESSAGE_INCOMPLETE);
    VerifyOrExit(resplen == kMsgCounterSyncRespMsgSize, err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    syncCounter = chip::Encoding::LittleEndian::Read32(resp);
    VerifyOrExit(syncCounter != 0, err = CHIP_ERROR_READ_FAILED);

    memcpy(challenge, resp, kMsgCounterChallengeSize);

    // Verify that the response field matches the expected Challenge field for the exchange.
    VerifyOrExit(memcmp(exchangeContext->GetChallenge(), challenge, kMsgCounterChallengeSize) == 0,
                 err = CHIP_ERROR_INVALID_SIGNATURE);

    VerifyOrExit(packetHeader.GetSourceNodeId().HasValue(), err = CHIP_ERROR_INVALID_ARGUMENT);
    peerNodeId = packetHeader.GetSourceNodeId().Value();

    // Process all queued ougoing and incomming group messages after message counter synchronization is completed.
    RetransPendingGroupMsgs(peerNodeId);
    ProcessPendingGroupMsgs(peerNodeId);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(ExchangeManager, "Failed to handle MsgCounterSyncResp message with error:%s", ErrorStr(err));
    }

    if (exchangeContext != nullptr)
        exchangeContext->Close();

    return;
}

} // namespace Messaging
} // namespace chip
