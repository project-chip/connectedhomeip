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

#include <lib/core/CHIPError.h>
#include <lib/core/Optional.h>
#include <lib/support/BitFlags.h>
#include <lib/support/Pool.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ReliableMessageAnalyticsDelegate.h>
#include <messaging/ReliableMessageProtocolConfig.h>
#include <system/SystemLayer.h>
#include <system/SystemPacketBuffer.h>
#include <transport/SessionUpdateDelegate.h>
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
        RetransTableEntry(ReliableMessageContext * rc);
        ~RetransTableEntry();

        ExchangeHandle ec;                        /**< The context for the stored CHIP message. */
        EncryptedPacketBufferHandle retainedBuf;  /**< The packet buffer holding the CHIP message. */
        System::Clock::Timestamp nextRetransTime; /**< A counter representing the next retransmission time for the message. */
        uint8_t sendCount;                        /**< The number of times we have tried to send this entry,
                                                       including both successfully and failure send. */
    };

    ReliableMessageMgr(ObjectPool<ExchangeContext, CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS> & contextPool);
    ~ReliableMessageMgr();

    void Init(chip::System::Layer * systemLayer);
    void Shutdown();

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
    static void Timeout(System::Layer * aSystemLayer, void * aAppState);

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
     *  Calculate the backoff timer for the retransmission.
     *
     *  @param[in]   baseInterval         The base interval to use for the backoff calculation, either the active or idle interval.
     *  @param[in]   sendCount            Count of how many times this message
     *                                    has been retransmitted so far (0 if it has
     *                                    been sent only once with no retransmits,
     *                                    1 if it has been sent twice, etc).
     *  @param[in]   computeMaxPossible   Disable randomness such that the maximum value is used instead.
     *
     *  @retval  The backoff time value, including jitter.
     */
    static System::Clock::Timeout GetBackoff(System::Clock::Timeout baseInterval, uint8_t sendCount,
                                             bool computeMaxPossible = false);

    /**
     *  Start retranmisttion of cached encryped packet for current entry.
     *
     *  @param[in]   entry    A pointer to a retransmission table entry added into the table.
     *
     *  @retval  #CHIP_NO_ERROR On success.
     */
    void StartRetransmision(RetransTableEntry * entry);

    /**
     *  Iterate through active exchange contexts and retrans table entries. Clear the entry matching
     *  the specified ExchangeContext and the message ID from the retransmision table.
     *
     *  @param[in]    rc                 A pointer to the ExchangeContext object.
     *  @param[in]    ackMessageCounter  The acknowledged message counter of the received packet.
     *
     *  @retval  #CHIP_NO_ERROR On success.
     */
    bool CheckAndRemRetransTable(ReliableMessageContext * rc, uint32_t ackMessageCounter);

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
     *  Registers a delegate to perform an address lookup and update all active sessions.
     *
     *  @param[in] sessionUpdateDelegate - Pointer to delegate to perform address lookup
     *             that will update all active session. A null pointer is allowed if you
     *             no longer have a valid delegate.
     *
     */
    void RegisterSessionUpdateDelegate(SessionUpdateDelegate * sessionUpdateDelegate);

#if CHIP_CONFIG_MRP_ANALYTICS_ENABLED
    /**
     *  Registers a delegate interested in analytic information
     *
     *  @param[in] analyticsDelegate - Pointer to delegate for reporting analytic
     */
    void RegisterAnalyticsDelegate(ReliableMessageAnalyticsDelegate * analyticsDelegate);
#endif // CHIP_CONFIG_MRP_ANALYTICS_ENABLED

    /**
     * Map a send error code to the error code we should actually use for
     * success checks.  This maps some error codes to CHIP_NO_ERROR as
     * appropriate.
     */
    static CHIP_ERROR MapSendError(CHIP_ERROR error, uint16_t exchangeId, bool isInitiator);

#if CHIP_CONFIG_TEST
    // Functions for testing
    int TestGetCountRetransTable();

    // Enumerate the retransmission table.  Clearing an entry while enumerating
    // that entry is allowed.  F must take a RetransTableEntry as an argument
    // and return Loop::Continue or Loop::Break.
    template <typename F>
    void EnumerateRetransTable(F && functor)
    {
        mRetransTable.ForEachActiveObject(std::forward<F>(functor));
    }
#endif // CHIP_CONFIG_TEST

    /**
     * Set the value to add to the MRP backoff time we compute.  This is meant to
     * account for high network latency on the sending side (us) that can't be
     * known to the message recipient and hence is not captured in the MRP
     * parameters the message recipient communicates to us.
     *
     * If set to NullOptional falls back to the compile-time
     * CHIP_CONFIG_MRP_RETRY_INTERVAL_SENDER_BOOST.
     *
     * This is a static, not a regular member, because API consumers may need to
     * set this before actually bringing up the stack and having access to a
     * ReliableMessageMgr.
     */
    static void SetAdditionalMRPBackoffTime(const Optional<System::Clock::Timeout> & additionalTime);

private:
    /**
     * Calculates the next retransmission time for the entry
     * Function sets the nextRetransTime of the entry
     *
     * @param[in,out] entry RetransTableEntry for which we need to calculate the nextRetransTime
     */
    void CalculateNextRetransTime(RetransTableEntry & entry);

    ObjectPool<ExchangeContext, CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS> & mContextPool;
    chip::System::Layer * mSystemLayer;

    /* Placeholder function to run a function for all exchanges */
    template <typename Function>
    void ExecuteForAllContext(Function function)
    {
        mContextPool.ForEachActiveObject([&](auto * ec) {
            function(ec->GetReliableMessageContext());
            return Loop::Continue;
        });
    }

    void TicklessDebugDumpRetransTable(const char * log);

#if CHIP_CONFIG_MRP_ANALYTICS_ENABLED
    void NotifyMessageSendAnalytics(const RetransTableEntry & entry, const SessionHandle & sessionHandle,
                                    const ReliableMessageAnalyticsDelegate::EventType & eventType);
#endif // CHIP_CONFIG_MRP_ANALYTICS_ENABLED

    // ReliableMessageProtocol Global tables for timer context
    ObjectPool<RetransTableEntry, CHIP_CONFIG_RMP_RETRANS_TABLE_SIZE> mRetransTable;

    SessionUpdateDelegate * mSessionUpdateDelegate = nullptr;
#if CHIP_CONFIG_MRP_ANALYTICS_ENABLED
    ReliableMessageAnalyticsDelegate * mAnalyticsDelegate = nullptr;
#endif // CHIP_CONFIG_MRP_ANALYTICS_ENABLED

    static System::Clock::Timeout sAdditionalMRPBackoffTime;
};

} // namespace Messaging
} // namespace chip
