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

#include <system/SystemPacketBuffer.h>
#include <transport/MessageCounterManagerInterface.h>

namespace chip {
namespace mcsp {

class ExchangeManager;

class MessageCounterManager : public Messaging::ExchangeDelegate, public Transport::MessageCounterManagerInterface
{
public:
    static constexpr uint16_t kChallengeSize = Transport::PeerMessageCounter::kChallengeSize;
    static constexpr uint16_t kCounterSize   = 4;
    static constexpr uint16_t kSyncRespMsgSize =
        kChallengeSize + kCounterSize; // The size of the message counter synchronization response message.
    static constexpr uint32_t kSyncTimeout =
        500; // The amount of time(in milliseconds) which a peer is given to respond to a message counter synchronization request.

    MessageCounterManager() : mExchangeMgr(nullptr) {}
    ~MessageCounterManager() override {}

    CHIP_ERROR Init(Messaging::ExchangeManager * exchangeMgr);
    void Shutdown();

    // Implement MessageCounterManagerInterface
    MessageCounter & GetGlobalUnsecureCounter() override;
    MessageCounter & GetGlobalSecureCounter() override;
    MessageCounter & GetLocalSessionCounter(Transport::PeerConnectionState * state) override;

    bool IsSyncCompleted(Transport::PeerConnectionState * state) override;
    CHIP_ERROR VerifyCounter(Transport::PeerConnectionState * state, const PacketHeader & packetHeader) override;
    void CommitCounter(Transport::PeerConnectionState * state, const PacketHeader & packetHeader) override;

    CHIP_ERROR StartSync(SecureSessionHandle session, Transport::PeerConnectionState * state) override;
    CHIP_ERROR QueueSendMessageAndStartSync(SecureSessionHandle session, Transport::PeerConnectionState * state,
                                            PayloadHeader & payloadHeader, System::PacketBufferHandle msgBuf) override;
    CHIP_ERROR QueueReceivedMessageAndStartSync(SecureSessionHandle session, Transport::PeerConnectionState * state,
                                                const PacketHeader & packetHeader, const Transport::PeerAddress & peerAddress,
                                                System::PacketBufferHandle msgBuf) override;

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
    CHIP_ERROR SendMsgCounterSyncReq(SecureSessionHandle session, Transport::PeerConnectionState * state);

    /**
     *  Add a CHIP message into the cache table to queue the outging messages that trigger message counter synchronization protocol
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
    CHIP_ERROR AddToRetransmissionTable(SecureSessionHandle session, NodeId peerNodeId, PayloadHeader & payloadHeader,
                                        System::PacketBufferHandle msgBuf);

    /**
     *  Add a CHIP message into the cache table to queue the incoming messages that trigger message counter synchronization
     * protocol for re-processing.
     *
     *  @param[in]    packetHeader     The message header for the received message.
     *  @param[in]    payloadHeader    The payload header for the received message.
     *  @param[in]    session          The handle to the secure session.
     *  @param[in]    msgBuf           A handle to the packet buffer holding the received message.
     *
     *  @retval  #CHIP_ERROR_NO_MEMORY If there is no empty slot left in the table for addition.
     *  @retval  #CHIP_NO_ERROR On success.
     */
    CHIP_ERROR AddToReceiveTable(NodeId peerNodeId, const PacketHeader & packetHeader, const Transport::PeerAddress & peerAddress,
                                 System::PacketBufferHandle msgBuf);

private:
    GlobalUnencryptedMessageCounter mGlobalUnencryptedMessageCounter;
    GlobalEncryptedMessageCounter mGlobalEncryptedMessageCounter;

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
        RetransTableEntry() : peerNodeId(kUndefinedNodeId) {}

        NodeId peerNodeId;                 /**< The peerNodeId of the message. kUndefinedNodeId if is not in use. */
        SecureSessionHandle session;       /**< The session to send the message. */
        PayloadHeader payloadHeader;       /**< The payload header for the message. */
        System::PacketBufferHandle msgBuf; /**< A handle to the PacketBuffer object holding the CHIP message. */
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
        ReceiveTableEntry() : peerNodeId(kUndefinedNodeId) {}

        NodeId peerNodeId;                  /**< The peerNodeId of the message. kUndefinedNodeId if is not in use. */
        PacketHeader packetHeader;          /**< The packet header for the message. */
        Transport::PeerAddress peerAddress; /**< The peer address for the message*/
        System::PacketBufferHandle msgBuf;  /**< A handle to the PacketBuffer object holding the message data. */
    };

    Messaging::ExchangeManager * mExchangeMgr; // [READ ONLY] Associated Exchange Manager object.

    // MessageCounterManager cache table to queue the outging messages that trigger message counter synchronization protocol.
    RetransTableEntry mRetransTable[CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS];

    // MessageCounterManager cache table to queue the incoming messages that trigger message counter synchronization protocol.
    ReceiveTableEntry mReceiveTable[CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS];

    void RetransPendingMessages(NodeId peerNodeId);

    void ProcessPendingMessages(NodeId peerNodeId);

    CHIP_ERROR NewMsgCounterSyncExchange(SecureSessionHandle session, Messaging::ExchangeContext *& exchangeContext);

    CHIP_ERROR SendMsgCounterSyncResp(Messaging::ExchangeContext * exchangeContext, SecureSessionHandle session,
                                      std::array<uint8_t, kChallengeSize> challenge);

    void HandleMsgCounterSyncReq(Messaging::ExchangeContext * exchangeContext, const PacketHeader & packetHeader,
                                 System::PacketBufferHandle msgBuf);

    void HandleMsgCounterSyncResp(Messaging::ExchangeContext * exchangeContext, const PacketHeader & packetHeader,
                                  System::PacketBufferHandle msgBuf);

    void OnMessageReceived(Messaging::ExchangeContext * exchangeContext, const PacketHeader & packetHeader,
                           const PayloadHeader & payloadHeader, System::PacketBufferHandle payload) override;

    void OnResponseTimeout(Messaging::ExchangeContext * exchangeContext) override;
};

} // namespace mcsp
} // namespace chip
