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
        uint32_t msgId;                /**< The message identifier of the CHIP message awaiting acknowledgment. */
        uint16_t msgSendFlags;
        uint16_t nextRetransTimeTick; /**< A counter representing the next retransmission time for the message. */
        uint8_t sendCount;            /**< A counter representing the number of times the message has been sent. */
    };

public:
    ReliableMessageManager();
    ~ReliableMessageManager();

    void Init(chip::System::Layer & system) { mSystemLayer = &system; }
    void Shutdown() {}

    void FreeContext(ReliableMessageContext * rc);

    uint64_t GetTickCounterFromTimePeriod(uint64_t period);
    uint64_t GetTickCounterFromTimeDelta(uint64_t newTime);

    void ExecuteActions();
    void ProcessDelayedDeliveryMessage(ReliableMessageContext * rc, uint32_t PauseTimeMillis);
    static void Timeout(System::Layer * aSystemLayer, void * aAppState, System::Error aError);

    CHIP_ERROR AddToRetransTable(ReliableMessageContext * rc, System::PacketBuffer * msgBuf, uint32_t messageId,
                                 uint16_t msgSendFlags, RetransTableEntry ** rEntry);
    void PauseRetransTable(ReliableMessageContext * rc, uint32_t PauseTimeMillis);
    void ResumeRetransTable(ReliableMessageContext * rc);
    bool CheckAndRemRetransTable(ReliableMessageContext * rc, uint32_t msgId);
    CHIP_ERROR SendFromRetransTable(RetransTableEntry * entry);
    void ClearRetransmitTable(ReliableMessageContext * rc);
    void ClearRetransmitTable(RetransTableEntry & rEntry);
    void FailRetransmitTableEntries(ReliableMessageContext * rc, CHIP_ERROR err);

    void StartTimer();
    void StopTimer();
    void ExpireTicks();

    // Functions for testing
    int TestGetCountRetransTable();
    void TestSetIntervalShift(uint16_t value) { mTimerIntervalShift = value; }

public:
    // public functions for ReliableMessageProtocol internal usage
    CHIP_ERROR SendMessage(ReliableMessageContext * context, System::PacketBuffer * msgBuf, uint16_t sendFlags);
    CHIP_ERROR SendMessage(ReliableMessageContext * context, uint32_t profileId, uint8_t msgType, System::PacketBuffer * msgBuf,
                           BitFlags<uint16_t, SendMessageFlags> sendFlags);

private:
    chip::System::Layer * mSystemLayer;
    uint64_t mTimeStampBase;                  // ReliableMessageProtocol timer base value to add offsets to evaluate timeouts
    System::Timer::Epoch mCurrentTimerExpiry; // Tracks when the ReliableMessageProtocol timer will next expire
    uint16_t mTimerIntervalShift;             // ReliableMessageProtocol Timer tick period shift

    /* Placeholder function to run a function for all exchanges */
    template <typename Function>
    void ExecuteForAllContext(Function function)
    {}

    void TicklessDebugDumpRetransTable(const char * log);

    // ReliableMessageProtocol Global tables for timer context
    RetransTableEntry RetransTable[CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE];
};

} // namespace Messaging
} // namespace chip
