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

#include <array>
#include <stdint.h>

#include <messaging/ExchangeContext.h>
#include <messaging/ReliableMessageProtocolConfig.h>

#include <core/CHIPError.h>
#include <support/BitFlags.h>
#include <support/Pool.h>
#include <system/SystemLayer.h>
#include <system/SystemPacketBuffer.h>
#include <system/SystemTimer.h>
#include <transport/raw/MessageHeader.h>

namespace chip {
namespace Messaging {

enum class SendMessageFlags : uint16_t;
class ReliableMessageContext;

class ReliableMessageMgr
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

        ReliableMessageContext * rc;             /**< The context for the stored CHIP message. */
        EncryptedPacketBufferHandle retainedBuf; /**< The packet buffer holding the CHIP message. */
        uint16_t nextRetransTimeTick;            /**< A counter representing the next retransmission time for the message. */
        uint8_t sendCount;                       /**< A counter representing the number of times the message has been sent. */
    };

public:
    ReliableMessageMgr(BitMapObjectPool<ExchangeContext, CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS> & contextPool);
    ~ReliableMessageMgr();

    void Init(chip::System::Layer * systemLayer, SecureSessionMgr * sessionMgr);
    void Shutdown();

    /**
     * Return a tick counter value given a time period.
     *
     * @param[in]  period        Timestamp value of in milliseconds.
     *
     * @return Tick count for the time period.
     */
    uint64_t GetTickCounterFromTimePeriod(uint64_t period);

    /**
     * Return a tick counter value between the given time and the stored time.
     *
     * @param[in]  newTime        Timestamp value of in milliseconds.
     *
     * @return Tick count of the difference between the given time and the stored time.
     */
    uint64_t GetTickCounterFromTimeDelta(uint64_t newTime);

    /**
     * Iterate through active exchange contexts and retrans table entries.  If an
     * action needs to be triggered by ReliableMessageProtocol time facilities,
     * execute that action.
     */
    void ExecuteActions();

    /**
     * Handle physical wakeup of system due to ReliableMessageProtocol wakeup.
     *
     */
    static void Timeout(System::Layer * aSystemLayer, void * aAppState, CHIP_ERROR aError);

    /**
     *  Add a CHIP message into the retransmission table to be subsequently resent if a corresponding acknowledgment
     *  is not received within the retransmission timeout.
     *
     *  @param[in]    rc        A pointer to the ExchangeContext object.
     *
     *  @param[out]   rEntry    A pointer to a pointer of a retransmission table entry added into the table.
     *
     *  @retval  #CHIP_ERROR_RETRANS_TABLE_FULL If there is no empty slot left in the table for addition.
     *  @retval  #CHIP_NO_ERROR On success.
     */
    CHIP_ERROR AddToRetransTable(ReliableMessageContext * rc, RetransTableEntry ** rEntry);

    /**
     *  Start retranmisttion of cached encryped packet for current entry.
     *
     *  @param[in]   entry    A pointer to a retransmission table entry added into the table.
     *
     *  @retval  #CHIP_NO_ERROR On success.
     */
    void StartRetransmision(RetransTableEntry * entry);

    /**
     *  Pause retranmisttion of current exchange for specified period.
     *
     *  @param[in]    rc                A pointer to the ExchangeContext object.
     *
     *  @param[in]    PauseTimeMillis   Pause period in milliseconds.
     *
     *  @retval  #CHIP_NO_ERROR On success.
     */
    void PauseRetransmision(ReliableMessageContext * rc, uint32_t PauseTimeMillis);

    /**
     *  Re-start retranmisttion of cached encryped packets for the given ReliableMessageContext.
     *
     *  @param[in] rc The ReliableMessageContext to resume retransmission for.
     *
     *  @retval  #CHIP_NO_ERROR On success.
     */
    void ResumeRetransmision(ReliableMessageContext * rc);

    /**
     *  Iterate through active exchange contexts and retrans table entries. Clear the entry matching
     *  the specified ExchangeContext and the message ID from the retransmision table.
     *
     *  @param[in]    rc        A pointer to the ExchangeContext object.
     *
     *  @param[in]    msgId     message ID which has been acked.
     *
     *  @retval  #CHIP_NO_ERROR On success.
     */
    bool CheckAndRemRetransTable(ReliableMessageContext * rc, uint32_t msgId);

    /**
     *  Send the specified entry from the retransmission table.
     *
     *  @param[in]    entry     A pointer to a retransmission table entry object that needs to be sent.
     *
     *  @return  #CHIP_NO_ERROR On success, else corresponding CHIP_ERROR returned from SendMessage.
     */
    CHIP_ERROR SendFromRetransTable(RetransTableEntry * entry);

    /**
     *  Clear entries matching a specified ExchangeContext.
     *
     *  @param[in]    rc    A pointer to the ExchangeContext object.
     *
     */
    void ClearRetransTable(ReliableMessageContext * rc);

    /**
     *  Clear an entry in the retransmission table.
     *
     *  @param[in]    rEntry   A reference to the RetransTableEntry object.
     *
     */
    void ClearRetransTable(RetransTableEntry & rEntry);

    /**
     *  Fail entries matching a specified ExchangeContext.
     *
     *  @param[in]    rc    A pointer to the ExchangeContext object.
     *
     *  @param[in]    err   The error for failing table entries.
     *
     */
    void FailRetransTableEntries(ReliableMessageContext * rc, CHIP_ERROR err);

    /**
     * Iterate through active exchange contexts and retrans table entries.
     * Determine how many ReliableMessageProtocol ticks we need to sleep before we
     * need to physically wake the CPU to perform an action.  Set a timer to go off
     * when we next need to wake the system.
     *
     */
    void StartTimer();

    /**
     * Stop the timer for retransmistion on current node.
     *
     */
    void StopTimer();

    /**
     * Calculate number of virtual ReliableMessageProtocol ticks that have expired
     * since we last called this function. Iterate through active exchange contexts
     * and retrans table entries, subtracting expired virtual ticks to synchronize
     * wakeup times with the current system time. Do not perform any actions beyond
     * updating tick counts, actions will be performed by the physical
     * ReliableMessageProtocol timer tick expiry.
     *
     */
    void ExpireTicks();

#if CHIP_CONFIG_TEST
    // Functions for testing
    int TestGetCountRetransTable();
    void TestSetIntervalShift(uint16_t value) { mTimerIntervalShift = value; }
#endif // CHIP_CONFIG_TEST

private:
    BitMapObjectPool<ExchangeContext, CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS> & mContextPool;
    chip::System::Layer * mSystemLayer;
    SecureSessionMgr * mSessionMgr;
    uint64_t mTimeStampBase; // ReliableMessageProtocol timer base value to add offsets to evaluate timeouts
    System::Clock::MonotonicMilliseconds mCurrentTimerExpiry; // Tracks when the ReliableMessageProtocol timer will next expire
    uint16_t mTimerIntervalShift;                             // ReliableMessageProtocol Timer tick period shift

    /* Placeholder function to run a function for all exchanges */
    template <typename Function>
    void ExecuteForAllContext(Function function)
    {
        mContextPool.ForEachActiveObject([&](auto * ec) {
            function(ec->GetReliableMessageContext());
            return true;
        });
    }

    void TicklessDebugDumpRetransTable(const char * log);

    // ReliableMessageProtocol Global tables for timer context
    RetransTableEntry mRetransTable[CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE];
};

} // namespace Messaging
} // namespace chip
