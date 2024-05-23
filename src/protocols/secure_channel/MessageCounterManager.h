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
 *      This file defines types and objects for CHIP message counter messages in secure channel protocol.
 *
 */

#pragma once

#include <messaging/ExchangeDelegate.h>
#include <messaging/ExchangeMgr.h>
#include <protocols/Protocols.h>
#include <system/SystemPacketBuffer.h>
#include <transport/MessageCounterManagerInterface.h>

namespace chip {
namespace secure_channel {

class MessageCounterManager : public Messaging::UnsolicitedMessageHandler,
                              public Messaging::ExchangeDelegate,
                              public Transport::MessageCounterManagerInterface
{
public:
    static constexpr uint16_t kChallengeSize             = Transport::PeerMessageCounter::kChallengeSize;
    static constexpr uint16_t kCounterSize               = 4;
    static constexpr uint16_t kSyncRespMsgSize           = kChallengeSize + kCounterSize;
    static constexpr System::Clock::Timeout kSyncTimeout = System::Clock::Milliseconds32(500);

    MessageCounterManager() : mExchangeMgr(nullptr) {}
    ~MessageCounterManager() override {}

    CHIP_ERROR Init(Messaging::ExchangeManager * exchangeMgr);
    void Shutdown();

    // Implement MessageCounterManagerInterface
    CHIP_ERROR StartSync(const SessionHandle & session, Transport::SecureSession * state) override;
    CHIP_ERROR QueueReceivedMessageAndStartSync(const PacketHeader & packetHeader, const SessionHandle & session,
                                                Transport::SecureSession * state, const Transport::PeerAddress & peerAddress,
                                                System::PacketBufferHandle && msgBuf) override;

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
    CHIP_ERROR SendMsgCounterSyncReq(const SessionHandle & session, Transport::SecureSession * state);

    /**
     *  Add a CHIP message into the cache table to queue the incoming messages that trigger message counter synchronization
     * protocol for re-processing.
     *
     *  @param[in]    msgBuf           A handle to the packet buffer holding the received message.
     *
     *  @retval  #CHIP_ERROR_NO_MEMORY If there is no empty slot left in the table for addition.
     *  @retval  #CHIP_NO_ERROR On success.
     */
    CHIP_ERROR AddToReceiveTable(const PacketHeader & packetHeader, const Transport::PeerAddress & peerAddress,
                                 System::PacketBufferHandle && msgBuf);

private:
    /**
     *  @class ReceiveTableEntry
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
        Transport::PeerAddress peerAddress; /**< The peer address for the message*/
        System::PacketBufferHandle msgBuf;  /**< A handle to the PacketBuffer object holding the message data. */
    };

    Messaging::ExchangeManager * mExchangeMgr; // [READ ONLY] Associated Exchange Manager object.

    // MessageCounterManager cache table to queue the incoming messages that trigger message counter synchronization protocol.
    ReceiveTableEntry mReceiveTable[CHIP_CONFIG_MCSP_RECEIVE_TABLE_SIZE];

    void ProcessPendingMessages(NodeId peerNodeId);

    CHIP_ERROR SendMsgCounterSyncResp(Messaging::ExchangeContext * exchangeContext, FixedByteSpan<kChallengeSize> challenge);
    CHIP_ERROR HandleMsgCounterSyncReq(Messaging::ExchangeContext * exchangeContext, System::PacketBufferHandle && msgBuf);
    CHIP_ERROR HandleMsgCounterSyncResp(Messaging::ExchangeContext * exchangeContext, System::PacketBufferHandle && msgBuf);

    CHIP_ERROR OnUnsolicitedMessageReceived(const PayloadHeader & payloadHeader, ExchangeDelegate *& newDelegate) override;
    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * exchangeContext, const PayloadHeader & payloadHeader,
                                 System::PacketBufferHandle && payload) override;

    void OnResponseTimeout(Messaging::ExchangeContext * exchangeContext) override;
};

} // namespace secure_channel
} // namespace chip
