/*
 *    Copyright (c) 2020 Project CHIP Authors
 *    All rights reserved.
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
 *      This file defines the classes corresponding to CHIP reliable message
 *      protocol.
 */

#pragma once

#include <stdint.h>

#include <messaging/ExchangeContext.h>
#include <messaging/ReliableMessageProtocolConfig.h>

#include <core/CHIPError.h>
#include <support/BitFlags.h>
#include <system/SystemLayer.h>
#include <system/SystemPacketBuffer.h>
#include <system/SystemTimer.h>
#include <transport/raw/MessageHeader.h>

namespace chip {
namespace Messaging {

enum class SendMessageFlags : uint16_t;
class ReliableMessageContext;

class ReliableMessageManager
{
public:
    /**
     *  @class RetransTableEntry
     *
     *  @brief
     *    This class is part of the CHIP Reliable Messaging Protocol and is used
     *    to keep track of CHIP messages that have been sent and are expecting an
     *    acknowledgment back. If the acknowledgment is not received within a
     *    specific timeout, the message would be retransmitted from this table.
     *
     */
    struct RetransTableEntry
    {
        RetransTableEntry();

        ReliableMessageContext * rc;   /**< The context for the stored CHIP message. */
        System::PacketBuffer * msgBuf; /**< A pointer to the PacketBuffer object holding the CHIP message. */
        PayloadHeader payloadHeader;   /**< Exchange Header for the stored CHIP message. */
        uint64_t peerNodeId;           /**< Node ID of peer node. */
        uint32_t msgId;                /**< The message identifier of the CHIP message awaiting acknowledgment. */
        uint32_t payloadLen;           /**< The payload length of the CHIP message awaiting acknowledgment. */
        uint16_t nextRetransTimeTick;  /**< A counter representing the next retransmission time for the message. */
        uint8_t sendCount;             /**< A counter representing the number of times the message has been sent. */
    };

public:
    ReliableMessageManager();
    ~ReliableMessageManager();

    void Init(chip::System::Layer * systemLayer, SecureSessionMgr * sessionMgr, ExchangeContext * contextPool);
    void Shutdown();

    uint64_t GetTickCounterFromTimePeriod(uint64_t period);
    uint64_t GetTickCounterFromTimeDelta(uint64_t newTime);

    void ExecuteActions();
    void ProcessDelayedDeliveryMessage(uint32_t PauseTimeMillis, uint64_t DelayedNodeId);
    static void Timeout(System::Layer * aSystemLayer, void * aAppState, System::Error aError);

    CHIP_ERROR AddToRetransTable(ReliableMessageContext * rc, PayloadHeader & payloadHeader, NodeId nodeId,
                                 System::PacketBuffer * msgBuf, RetransTableEntry ** rEntry);
    void PauseRetransTable(ReliableMessageContext * rc, uint32_t PauseTimeMillis);
    void ResumeRetransTable(ReliableMessageContext * rc);
    bool CheckAndRemRetransTable(ReliableMessageContext * rc, uint32_t msgId);
    CHIP_ERROR SendFromRetransTable(RetransTableEntry * entry, bool isResend);
    void ClearRetransmitTable(ReliableMessageContext * rc);
    void ClearRetransmitTable(RetransTableEntry & rEntry);
    void FailRetransmitTableEntries(ReliableMessageContext * rc, CHIP_ERROR err);

    void StartTimer();
    void StopTimer();
    void ExpireTicks();

    // Functions for testing
    int TestGetCountRetransTable();
    void TestSetIntervalShift(uint16_t value) { mTimerIntervalShift = value; }

private:
    chip::System::Layer * mSystemLayer;
    ExchangeContext * mContextPool;
    SecureSessionMgr * mSessionMgr;
    uint64_t mTimeStampBase;                  // ReliableMessageProtocol timer base value to add offsets to evaluate timeouts
    System::Timer::Epoch mCurrentTimerExpiry; // Tracks when the ReliableMessageProtocol timer will next expire
    uint16_t mTimerIntervalShift;             // ReliableMessageProtocol Timer tick period shift

    /* Placeholder function to run a function for all exchanges */
    template <typename Function>
    void ExecuteForAllContext(Function function)
    {
        ExchangeContext * ec = mContextPool;

        if (ec != nullptr)
        {
            for (int i = 0; i < CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS; i++, ec++)
            {
                function(ec->GetReliableMessageContext());
            }
        }
    }

    void TicklessDebugDumpRetransTable(const char * log);

    // ReliableMessageProtocol Global tables for timer context
    RetransTableEntry RetransTable[CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE];
};

} // namespace Messaging
} // namespace chip
