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

#include <messaging/CHIPReliableMessageConfig.h>

#include <core/CHIPError.h>
#include <system/SystemLayer.h>
#include <system/SystemPacketBuffer.h>
#include <system/SystemTimer.h>

namespace chip {
namespace messaging {

class CHIPReliableMessageContext;

class CHIPReliableMessageManager
{
private:
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
    class RetransTableEntry
    {
    public:
        RetransTableEntry();

        CHIPReliableMessageContext * rc; /**< The context for the stored CHIP message. */
        System::PacketBuffer * msgBuf;   /**< A pointer to the PacketBuffer object holding the CHIP message. */
        uint32_t msgId;                  /**< The message identifier of the CHIP message awaiting acknowledgment. */
        uint16_t msgSendFlags;
        uint16_t nextRetransTime; /**< A counter representing the next retransmission time for the message. */
        uint8_t sendCount;        /**< A counter representing the number of times the message has been sent. */
    };

public:
    CHIPReliableMessageManager();
    ~CHIPReliableMessageManager();

    uint32_t GetTickCounterFromTimeDelta(uint64_t newTime);

    void RMPExecuteActions();
    void RMPProcessDDMessage(uint32_t PauseTimeMillis, uint64_t DelayedNodeId);
    static void RMPTimeout(System::Layer * aSystemLayer, void * aAppState, System::Error aError);

    CHIP_ERROR AddToRetransTable(CHIPReliableMessageContext * rc, System::PacketBuffer * msgBuf, uint32_t messageId,
                                 uint16_t msgSendFlags, RetransTableEntry ** rEntry);
    void RMPPauseRetransTable(CHIPReliableMessageContext * rc, uint32_t PauseTimeMillis);
    void RMPResumeRetransTable(CHIPReliableMessageContext * rc);
    bool RMPCheckAndRemRetransTable(CHIPReliableMessageContext * rc, uint32_t msgId);
    CHIP_ERROR SendFromRetransTable(RetransTableEntry * entry);
    void ClearRetransmitTable(CHIPReliableMessageContext * rc);
    void ClearRetransmitTable(RetransTableEntry & rEntry);
    void FailRetransmitTableEntries(CHIPReliableMessageContext * rc, CHIP_ERROR err);

    void RMPStartTimer();
    void RMPStopTimer();
    void RMPExpireTicks();

    RMPConfig mRMPConfig; /**< RMP configuration. */

private:
    chip::System::Layer * mSystemLayer;
    uint64_t mRMPTimeStampBase;                  // RMP timer base value to add offsets to evaluate timeouts
    System::Timer::Epoch mRMPCurrentTimerExpiry; // Tracks when the RMP timer will next expire
    uint16_t mRMPTimerInterval;                  // RMP Timer tick period

    /* Placeholder function to run a function for all exchanges */
    template <typename Function>
    void ExecuteForAllExchange(Function function)
    {}

    void TicklessDebugDumpRetransTable(const char * log);

    // RMP Global tables for timer context
    RetransTableEntry RetransTable[CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE];
};

} // namespace messaging
} // namespace chip
