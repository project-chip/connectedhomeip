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
 *      This file defines types and objects for CHIP Secure Channel protocol.
 *
 */

#pragma once

#include <protocols/Protocols.h>
#include <system/SystemPacketBuffer.h>

namespace chip {
namespace Messaging {

constexpr uint16_t kMsgCounterChallengeSize   = 8;   // The size of the message counter synchronization request message.
constexpr uint16_t kMsgCounterSyncRespMsgSize = 12;  // The size of the message counter synchronization response message.
constexpr uint32_t kMsgCounterSyncTimeout     = 500; // The amount of time(in milliseconds) which a peer is given to respond
                                                     // to a message counter synchronization request.

class ExchangeManager;

class MessageCounterSyncMgr : public Messaging::ExchangeDelegate
{
public:
    MessageCounterSyncMgr() : mExchangeMgr(nullptr) {}

    CHIP_ERROR Init(Messaging::ExchangeManager * exchangeMgr);
    void Shutdown();

    /**
     * Send peer message counter synchronization request.
     * This function is called while processing a message encrypted with an application key from a peer whose message counter is not
     * synchronized. This message is sent on a newly created exchange, which is closed immediately after.
     *
     * @param[in]  session  The secure session handle of the received message.
     *
     * @retval  #CHIP_ERROR_NO_MEMORY         If memory could not be allocated for the new
     *                                         exchange context or new message buffer.
     * @retval  #CHIP_NO_ERROR                On success.
     *
     */
    CHIP_ERROR SendMsgCounterSyncReq(SecureSessionHandle session);

    /**
     *  Add a CHIP message into the cache table to queue the outgoing messages that trigger message counter synchronization protocol
     *  for retransmission.
     *
     *  @param[in]    protocolId       The protocol identifier of the CHIP message to be sent.
     *
     *  @param[in]    msgType          The message type of the corresponding protocol.
     *
     *  @param[in]    sendFlags        Flags set by the application for the CHIP message being sent.
     *
     *  @param[in]    msgBuf           A handle to the packet buffer holding the CHIP message.
     *
     *  @param[in]    exchangeContext  A pointer to the exchange context object associated with the message being sent.
     *
     *  @retval  #CHIP_ERROR_NO_MEMORY If there is no empty slot left in the table for addition.
     *  @retval  #CHIP_NO_ERROR On success.
     */
    CHIP_ERROR AddToRetransmissionTable(Protocols::Id protocolId, uint8_t msgType, const SendFlags & sendFlags,
                                        System::PacketBufferHandle msgBuf, Messaging::ExchangeContext * exchangeContext);

    /**
     *  Add a CHIP message into the cache table to queue the incoming messages that trigger message counter synchronization
     * protocol for re-processing.
     *
     *  @param[in]    msgBuf           A handle to the packet buffer holding the received message.
     *
     *  @retval  #CHIP_ERROR_NO_MEMORY If there is no empty slot left in the table for addition.
     *  @retval  #CHIP_NO_ERROR On success.
     */
    CHIP_ERROR AddToReceiveTable(System::PacketBufferHandle msgBuf);

private:
    /**
     *  @class RetransTableEntry
     *
     *  @brief
     *    This class is part of the CHIP Message Counter Synchronization Protocol and is used
     *    to keep track of a CHIP messages to be transmitted to a destination node whose message
     *    counter is unknown. The message would be retransmitted from this table after message
     *    counter synchronization is completed.
     *
     */
    struct RetransTableEntry
    {
        RetransTableEntry() : protocolId(Protocols::NotSpecified) {}
        ExchangeContext * exchangeContext; /**< The ExchangeContext for the stored CHIP message.
                                                Non-null if and only if this entry is in use. */
        System::PacketBufferHandle msgBuf; /**< A handle to the PacketBuffer object holding the CHIP message. */
        SendFlags sendFlags;               /**< Flags set by the application for the CHIP message being sent. */
        Protocols::Id protocolId;          /**< The protocol identifier of the CHIP message to be sent. */
        uint8_t msgType;                   /**< The message type of the CHIP message to be sent. */
    };

    /**
     *  @class RetransTableEntry
     *
     *  @brief
     *    This class is part of the CHIP Message Counter Synchronization Protocol and is used
     *    to keep track of a CHIP messages to be reprocessed whose source's
     *    message counter is unknown. The message is reprocessed after message
     *    counter synchronization is completed.
     *
     */
    struct ReceiveTableEntry
    {
        System::PacketBufferHandle msgBuf; /**< A handle to the PacketBuffer object holding
                                                the message data. This is non-null if and only
                                                if this entry is in use. */
    };

    Messaging::ExchangeManager * mExchangeMgr; // [READ ONLY] Associated Exchange Manager object.

    // MessageCounterSyncProtocol cache table to queue the outgoing messages that trigger message counter
    // synchronization protocol. Reserve two extra exchanges, one for MCSP messages and another one for
    // temporary exchange for ack.
    RetransTableEntry mRetransTable[CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS - 2];

    // MessageCounterSyncProtocol cache table to queue the incoming messages that trigger message counter
    // synchronization protocol. Reserve two extra exchanges, one for MCSP messages and another one for
    // temporary exchange for ack.
    ReceiveTableEntry mReceiveTable[CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS - 2];

    void RetransPendingGroupMsgs(NodeId peerNodeId);

    void ProcessPendingGroupMsgs(NodeId peerNodeId);

    CHIP_ERROR NewMsgCounterSyncExchange(SecureSessionHandle session, Messaging::ExchangeContext *& exchangeContext);

    CHIP_ERROR SendMsgCounterSyncResp(Messaging::ExchangeContext * exchangeContext, SecureSessionHandle session);

    void HandleMsgCounterSyncReq(Messaging::ExchangeContext * exchangeContext, const PacketHeader & packetHeader,
                                 System::PacketBufferHandle msgBuf);

    void HandleMsgCounterSyncResp(Messaging::ExchangeContext * exchangeContext, const PacketHeader & packetHeader,
                                  System::PacketBufferHandle msgBuf);

    void OnMessageReceived(Messaging::ExchangeContext * exchangeContext, const PacketHeader & packetHeader,
                           const PayloadHeader & payloadHeader, System::PacketBufferHandle payload) override;

    void OnResponseTimeout(Messaging::ExchangeContext * exchangeContext) override;
};

} // namespace Messaging
} // namespace chip
